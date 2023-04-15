/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2019 Roman Gilg <subdiff@gmail.com>
    SPDX-FileCopyrightText: 2013, 2015 Martin Gräßlin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "wayland_qpainter_backend.h"
#include "core/shmgraphicsbufferallocator.h"
#include "wayland_backend.h"
#include "wayland_display.h"
#include "wayland_logging.h"
#include "wayland_output.h"

#include <KWayland/Client/surface.h>

#include <cmath>
#include <drm_fourcc.h>
#include <sys/mman.h>

namespace KWin
{
namespace Wayland
{

static uint32_t drmFormatToShmFormat(uint32_t drmFormat)
{
    switch (drmFormat) {
    case DRM_FORMAT_ARGB8888:
        return WL_SHM_FORMAT_ARGB8888;
    case DRM_FORMAT_XRGB8888:
        return WL_SHM_FORMAT_XRGB8888;
    default:
        return static_cast<wl_shm_format>(drmFormat);
    }
}

static QImage::Format drmFormatToQImageFormat(uint32_t drmFormat)
{
    switch (drmFormat) {
    case DRM_FORMAT_ARGB8888:
        return QImage::Format_ARGB32;
    case DRM_FORMAT_XRGB8888:
        return QImage::Format_RGB32;
    default:
        Q_UNREACHABLE();
    }
}

WaylandQPainterBufferSlot::WaylandQPainterBufferSlot(WaylandDisplay *display, ShmGraphicsBuffer *graphicsBuffer)
    : graphicsBuffer(graphicsBuffer)
{
    const ShmAttributes &attributes = graphicsBuffer->shmAttributes();
    size = attributes.size.height() * attributes.stride;

    pool = wl_shm_create_pool(display->shm(), attributes.fd.get(), size);
    buffer = wl_shm_pool_create_buffer(pool,
                                       attributes.offset,
                                       attributes.size.width(),
                                       attributes.size.height(),
                                       attributes.stride,
                                       drmFormatToShmFormat(attributes.format));

    static const wl_buffer_listener listener = {
        .release = [](void *userData, wl_buffer *buffer) {
            WaylandQPainterBufferSlot *slot = static_cast<WaylandQPainterBufferSlot *>(userData);
            slot->used = false;
        },
    };
    wl_buffer_add_listener(buffer, &listener, this);

    data = mmap(nullptr, size, PROT_READ | PROT_WRITE, MAP_SHARED, attributes.fd.get(), 0);
    if (data == MAP_FAILED) {
        qCWarning(KWIN_WAYLAND_BACKEND) << "Failed to map a shared memory buffer";
        return;
    }

    image = QImage(static_cast<uchar *>(data), attributes.size.width(), attributes.size.height(), drmFormatToQImageFormat(attributes.format));
}

WaylandQPainterBufferSlot::~WaylandQPainterBufferSlot()
{
    if (data) {
        munmap(data, size);
    }

    wl_buffer_destroy(buffer);
    wl_shm_pool_destroy(pool);
    graphicsBuffer->drop();
}

WaylandQPainterSwapchain::WaylandQPainterSwapchain(WaylandOutput *output, const QSize &size, uint32_t format)
    : m_allocator(std::make_unique<ShmGraphicsBufferAllocator>())
    , m_output(output)
    , m_size(size)
    , m_format(format)
{
}

QSize WaylandQPainterSwapchain::size() const
{
    return m_size;
}

std::shared_ptr<WaylandQPainterBufferSlot> WaylandQPainterSwapchain::acquire()
{
    for (const auto &slot : m_slots) {
        if (!slot->used) {
            slot->used = true;
            return slot;
        }
    }

    ShmGraphicsBuffer *buffer = m_allocator->allocate(m_size, m_format);
    if (!buffer) {
        qCDebug(KWIN_WAYLAND_BACKEND) << "Did not get a new Buffer from Shm Pool";
        return nullptr;
    }

    auto slot = std::make_shared<WaylandQPainterBufferSlot>(m_output->backend()->display(), buffer);
    m_slots.push_back(slot);

    return slot;
}

void WaylandQPainterSwapchain::release(std::shared_ptr<WaylandQPainterBufferSlot> buffer)
{
    for (const auto &slot : m_slots) {
        if (slot == buffer) {
            slot->age = 1;
        } else if (slot->age > 0) {
            slot->age++;
        }
    }
}

WaylandQPainterPrimaryLayer::WaylandQPainterPrimaryLayer(WaylandOutput *output)
    : m_waylandOutput(output)
{
}

void WaylandQPainterPrimaryLayer::present()
{
    auto s = m_waylandOutput->surface();
    s->attachBuffer(m_back->buffer);
    s->damage(m_damageJournal.lastDamage());
    s->setScale(std::ceil(m_waylandOutput->scale()));
    s->commit();

    m_swapchain->release(m_back);
}

QRegion WaylandQPainterPrimaryLayer::accumulateDamage(int bufferAge) const
{
    return m_damageJournal.accumulate(bufferAge, infiniteRegion());
}

std::optional<OutputLayerBeginFrameInfo> WaylandQPainterPrimaryLayer::beginFrame()
{
    const QSize nativeSize(m_waylandOutput->pixelSize());
    if (!m_swapchain || m_swapchain->size() != nativeSize) {
        m_swapchain = std::make_unique<WaylandQPainterSwapchain>(m_waylandOutput, nativeSize, DRM_FORMAT_XRGB8888);
    }

    m_back = m_swapchain->acquire();
    return OutputLayerBeginFrameInfo{
        .renderTarget = RenderTarget(&m_back->image),
        .repaint = accumulateDamage(m_back->age),
    };
}

bool WaylandQPainterPrimaryLayer::endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    m_damageJournal.add(damagedRegion);
    return true;
}

quint32 WaylandQPainterPrimaryLayer::format() const
{
    return DRM_FORMAT_RGBA8888;
}

WaylandQPainterCursorLayer::WaylandQPainterCursorLayer(WaylandOutput *output)
    : m_output(output)
{
}

std::optional<OutputLayerBeginFrameInfo> WaylandQPainterCursorLayer::beginFrame()
{
    const auto tmp = size().expandedTo(QSize(64, 64));
    const QSize bufferSize(std::ceil(tmp.width()), std::ceil(tmp.height()));
    if (!m_swapchain || m_swapchain->size() != bufferSize) {
        m_swapchain = std::make_unique<WaylandQPainterSwapchain>(m_output, bufferSize, DRM_FORMAT_ARGB8888);
    }

    m_back = m_swapchain->acquire();
    return OutputLayerBeginFrameInfo{
        .renderTarget = RenderTarget(&m_back->image),
        .repaint = infiniteRegion(),
    };
}

bool WaylandQPainterCursorLayer::endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    m_output->cursor()->update(m_back->buffer, scale(), hotspot().toPoint());
    m_swapchain->release(m_back);
    return true;
}

quint32 WaylandQPainterCursorLayer::format() const
{
    return DRM_FORMAT_RGBA8888;
}

WaylandQPainterBackend::WaylandQPainterBackend(Wayland::WaylandBackend *b)
    : QPainterBackend()
    , m_backend(b)
{

    const auto waylandOutputs = m_backend->waylandOutputs();
    for (auto *output : waylandOutputs) {
        createOutput(output);
    }
    connect(m_backend, &WaylandBackend::outputAdded, this, &WaylandQPainterBackend::createOutput);
    connect(m_backend, &WaylandBackend::outputRemoved, this, [this](Output *waylandOutput) {
        m_outputs.erase(waylandOutput);
    });
}

WaylandQPainterBackend::~WaylandQPainterBackend()
{
}

void WaylandQPainterBackend::createOutput(Output *waylandOutput)
{
    m_outputs[waylandOutput] = Layers{
        .primaryLayer = std::make_unique<WaylandQPainterPrimaryLayer>(static_cast<WaylandOutput *>(waylandOutput)),
        .cursorLayer = std::make_unique<WaylandQPainterCursorLayer>(static_cast<WaylandOutput *>(waylandOutput)),
    };
}

void WaylandQPainterBackend::present(Output *output)
{
    m_outputs[output].primaryLayer->present();
}

OutputLayer *WaylandQPainterBackend::primaryLayer(Output *output)
{
    return m_outputs[output].primaryLayer.get();
}

OutputLayer *WaylandQPainterBackend::cursorLayer(Output *output)
{
    return m_outputs[output].cursorLayer.get();
}

}
}
