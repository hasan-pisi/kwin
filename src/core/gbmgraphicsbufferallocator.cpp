/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "core/gbmgraphicsbufferallocator.h"
#include "backends/drm/gbm_dmabuf.h" // FIXME: move dmaBufAttributesForBo() elsewhere

#include <drm_fourcc.h>
#include <gbm.h>

namespace KWin
{

GbmGraphicsBufferAllocator::GbmGraphicsBufferAllocator(gbm_device *device)
    : m_gbmDevice(device)
{
}

GbmGraphicsBufferAllocator::~GbmGraphicsBufferAllocator()
{
}

GbmGraphicsBuffer *GbmGraphicsBufferAllocator::allocate(const QSize &size, uint32_t format, const QVector<uint64_t> &modifiers)
{
    gbm_bo *bo = nullptr;

    if (!modifiers.isEmpty() && !(modifiers.size() == 1 && modifiers.first() == DRM_FORMAT_MOD_INVALID)) {
        bo = gbm_bo_create_with_modifiers(m_gbmDevice,
                                          size.width(),
                                          size.height(),
                                          format,
                                          modifiers.constData(),
                                          modifiers.size());
    }

    if (!bo) {
        bo = gbm_bo_create(m_gbmDevice,
                           size.width(),
                           size.height(),
                           format,
                           GBM_BO_USE_SCANOUT | GBM_BO_USE_RENDERING);
    }

    if (!bo) {
        return nullptr;
    }

    return new GbmGraphicsBuffer(bo, size, format);
}

GbmGraphicsBuffer::GbmGraphicsBuffer(gbm_bo *handle, const QSize &size, uint32_t format)
    : m_bo(handle)
    , m_dmabufAttributes(dmaBufAttributesForBo(handle))
    , m_size(size)
    , m_hasAlphaChannel(alphaChannelFromDrmFormat(format))
{
}

GbmGraphicsBuffer::~GbmGraphicsBuffer()
{
    gbm_bo_destroy(m_bo);
}

QSize GbmGraphicsBuffer::size() const
{
    return m_size;
}

bool GbmGraphicsBuffer::hasAlphaChannel() const
{
    return m_hasAlphaChannel;
}

GraphicsBuffer::Origin GbmGraphicsBuffer::origin() const
{
    return Origin::TopLeft;
}

const DmaBufAttributes &GbmGraphicsBuffer::dmabufAttributes() const
{
    return m_dmabufAttributes;
}

} // namespace KWin
