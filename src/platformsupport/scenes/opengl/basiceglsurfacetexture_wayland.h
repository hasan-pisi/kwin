/*
    SPDX-FileCopyrightText: 2021 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include "openglsurfacetexture_wayland.h"

namespace KWaylandServer
{
class ShmClientBuffer;
class LinuxDmaBufV1ClientBuffer;
}

namespace KWin
{

class AbstractEglBackend;

class KWIN_EXPORT BasicEGLSurfaceTextureWayland : public OpenGLSurfaceTextureWayland
{
public:
    BasicEGLSurfaceTextureWayland(OpenGLBackend *backend, SurfacePixmapWayland *pixmap);
    ~BasicEGLSurfaceTextureWayland() override;

    AbstractEglBackend *backend() const;

    bool create() override;
    void update(const QRegion &region) override;

private:
    bool loadShmTexture(KWaylandServer::ShmClientBuffer *buffer);
    void updateShmTexture(KWaylandServer::ShmClientBuffer *buffer, const QRegion &region);
    bool loadDmabufTexture(KWaylandServer::LinuxDmaBufV1ClientBuffer *buffer);
    void updateDmabufTexture(KWaylandServer::LinuxDmaBufV1ClientBuffer *buffer);
    void destroy();

    enum class BufferType {
        None,
        Shm,
        DmaBuf,
    };

    BufferType m_bufferType = BufferType::None;
};

} // namespace KWin
