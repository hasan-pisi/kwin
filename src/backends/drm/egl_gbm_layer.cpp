/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2022 Xaver Hugl <xaver.hugl@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "egl_gbm_layer.h"
#include "drm_abstract_output.h"
#include "drm_backend.h"
#include "drm_buffer_gbm.h"
#include "drm_gpu.h"
#include "drm_output.h"
#include "drm_pipeline.h"
#include "egl_gbm_backend.h"
#include "logging.h"
#include "surfaceitem_wayland.h"

#include <KWaylandServer/linuxdmabufv1clientbuffer.h>
#include <KWaylandServer/surface_interface.h>

#include <QRegion>
#include <drm_fourcc.h>
#include <errno.h>
#include <gbm.h>
#include <unistd.h>

namespace KWin
{

EglGbmLayer::EglGbmLayer(EglGbmBackend *eglBackend, DrmPipeline *pipeline)
    : m_surface(pipeline->gpu(), eglBackend)
    , m_dmabufFeedback(pipeline->gpu(), eglBackend)
    , m_pipeline(pipeline)
{
    connect(eglBackend, &EglGbmBackend::aboutToBeDestroyed, this, &EglGbmLayer::destroyResources);
}

EglGbmLayer::~EglGbmLayer()
{
    destroyResources();
}

void EglGbmLayer::destroyResources()
{
    m_surface.destroyResources();
}

std::optional<QRegion> EglGbmLayer::beginFrame(const QRect &geometry)
{
    m_scanoutBuffer.reset();
    m_dmabufFeedback.renderingSurface();

    return m_surface.startRendering(m_pipeline->sourceSize(), geometry, m_pipeline->pending.bufferTransformation, m_pipeline->pending.sourceTransformation, m_pipeline->supportedFormats());
}

void EglGbmLayer::aboutToStartPainting(const QRegion &damagedRegion)
{
    m_surface.aboutToStartPainting(m_pipeline->output(), damagedRegion);
}

void EglGbmLayer::endFrame(const QRegion &renderedRegion, const QRegion &damagedRegion)
{
    Q_UNUSED(renderedRegion)
    const auto ret = m_surface.endRendering(m_pipeline->pending.bufferTransformation, damagedRegion);
    if (ret.has_value()) {
        std::tie(m_currentBuffer, m_currentDamage) = ret.value();
    }
}

QRegion EglGbmLayer::currentDamage() const
{
    return m_currentDamage;
}

bool EglGbmLayer::checkTestBuffer()
{
    if (!m_surface.doesSurfaceFit(m_pipeline->sourceSize(), m_pipeline->supportedFormats())) {
        beginFrame(m_pipeline->output()->geometry());
        glClear(GL_COLOR_BUFFER_BIT);
        endFrame(m_pipeline->output()->geometry(), m_pipeline->output()->geometry());
    }
    return !m_currentBuffer.isNull();
}

QSharedPointer<GLTexture> EglGbmLayer::texture() const
{
    if (m_scanoutBuffer) {
        return m_scanoutBuffer->createTexture(m_surface.eglBackend()->eglDisplay());
    } else {
        return m_surface.texture();
    }
}

bool EglGbmLayer::scanout(SurfaceItem *surfaceItem)
{
    static bool valid;
    static const bool directScanoutDisabled = qEnvironmentVariableIntValue("KWIN_DRM_NO_DIRECT_SCANOUT", &valid) == 1 && valid;
    if (directScanoutDisabled) {
        return false;
    }

    SurfaceItemWayland *item = qobject_cast<SurfaceItemWayland *>(surfaceItem);
    if (!item || !item->surface()) {
        return false;
    }
    const auto surface = item->surface();
    const auto buffer = qobject_cast<KWaylandServer::LinuxDmaBufV1ClientBuffer *>(surface->buffer());
    if (!buffer || buffer->planes().isEmpty() || buffer->size() != m_pipeline->sourceSize()) {
        return false;
    }
    const auto formats = m_pipeline->supportedFormats();
    if (!formats.contains(buffer->format())) {
        m_dmabufFeedback.scanoutFailed(surface, formats);
        return false;
    }
    m_scanoutBuffer = QSharedPointer<DrmGbmBuffer>::create(m_pipeline->gpu(), buffer);
    if (!m_scanoutBuffer || !m_scanoutBuffer->bufferId()) {
        m_dmabufFeedback.scanoutFailed(surface, formats);
        m_scanoutBuffer.reset();
        return false;
    }
    // damage tracking for screen casting
    QRegion damage;
    if (m_dmabufFeedback.currentSurface() == surface) {
        damage = surfaceItem->damage().translated(m_pipeline->output()->geometry().topLeft());
        surfaceItem->resetDamage();
    } else {
        damage = m_pipeline->output()->geometry();
    }
    if (m_pipeline->testScanout()) {
        m_dmabufFeedback.scanoutSuccessful(surface);
        m_currentBuffer = m_scanoutBuffer;
        m_currentDamage = damage;
        return true;
    } else {
        m_dmabufFeedback.scanoutFailed(surface, formats);
        m_scanoutBuffer.reset();
        return false;
    }
}

QSharedPointer<DrmBuffer> EglGbmLayer::currentBuffer() const
{
    return m_scanoutBuffer ? m_scanoutBuffer : m_currentBuffer;
}

bool EglGbmLayer::hasDirectScanoutBuffer() const
{
    return m_scanoutBuffer != nullptr;
}

QRect EglGbmLayer::geometry() const
{
    return m_pipeline->output()->geometry();
}

void EglGbmLayer::pageFlipped()
{
    m_scanoutBuffer.reset();
    // don't release m_currentBuffer yet, it may be needed for atomic tests
}
}
