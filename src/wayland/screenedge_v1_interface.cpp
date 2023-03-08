/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "wayland/screenedge_v1_interface.h"
#include "wayland/display.h"
#include "wayland/surface_interface.h"

#include "qwayland-server-kde-screen-edge-v1.h"

using namespace KWin;

namespace KWaylandServer
{

static const int s_version = 1;

class ScreenEdgeManagerV1InterfacePrivate : public QtWaylandServer::kde_screen_edge_manager_v1
{
public:
    ScreenEdgeManagerV1InterfacePrivate(ScreenEdgeManagerV1Interface *q, Display *display);

    ScreenEdgeManagerV1Interface *q;

protected:
    void kde_screen_edge_manager_v1_destroy(Resource *resource) override;
    void kde_screen_edge_manager_v1_get_screen_edge(Resource *resource, uint32_t id, uint32_t border, struct ::wl_resource *surface) override;
};

ScreenEdgeManagerV1InterfacePrivate::ScreenEdgeManagerV1InterfacePrivate(ScreenEdgeManagerV1Interface *q, Display *display)
    : QtWaylandServer::kde_screen_edge_manager_v1(*display, s_version)
    , q(q)
{
}

void ScreenEdgeManagerV1InterfacePrivate::kde_screen_edge_manager_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void ScreenEdgeManagerV1InterfacePrivate::kde_screen_edge_manager_v1_get_screen_edge(Resource *resource, uint32_t id, uint32_t border, struct ::wl_resource *surface)
{
    ElectricBorder electricBorder;
    switch (border) {
    case border_top:
        electricBorder = ElectricTop;
        break;
    case border_bottom:
        electricBorder = ElectricBottom;
        break;
    case border_left:
        electricBorder = ElectricLeft;
        break;
    case border_right:
        electricBorder = ElectricRight;
        break;
    default:
        wl_resource_post_error(resource->handle, 0, "invalid border");
        return;
    }

    wl_resource *edgeResource = wl_resource_create(resource->client(), &kde_screen_edge_v1_interface, resource->version(), id);
    auto edge = new ScreenEdgeV1Interface(SurfaceInterface::get(surface), electricBorder, edgeResource);
    Q_EMIT q->edgeRequested(edge);
}

ScreenEdgeManagerV1Interface::ScreenEdgeManagerV1Interface(Display *display, QObject *parent)
    : d(new ScreenEdgeManagerV1InterfacePrivate(this, display))
{
}

ScreenEdgeManagerV1Interface::~ScreenEdgeManagerV1Interface()
{
}

class ScreenEdgeV1InterfacePrivate : public QtWaylandServer::kde_screen_edge_v1
{
public:
    ScreenEdgeV1InterfacePrivate(ScreenEdgeV1Interface *q, SurfaceInterface *surface, ElectricBorder border, wl_resource *resource);

    ScreenEdgeV1Interface *q;
    QPointer<SurfaceInterface> surface;
    ElectricBorder border;

protected:
    void kde_screen_edge_v1_destroy_resource(Resource *resource) override;
    void kde_screen_edge_v1_destroy(Resource *resource) override;
    void kde_screen_edge_v1_show(Resource *resource) override;
    void kde_screen_edge_v1_hide(Resource *resource) override;
};

ScreenEdgeV1InterfacePrivate::ScreenEdgeV1InterfacePrivate(ScreenEdgeV1Interface *q, SurfaceInterface *surface, ElectricBorder border, wl_resource *resource)
    : QtWaylandServer::kde_screen_edge_v1(resource)
    , q(q)
    , surface(surface)
    , border(border)
{
}

void ScreenEdgeV1InterfacePrivate::kde_screen_edge_v1_destroy_resource(Resource *resource)
{
    delete q;
}

void ScreenEdgeV1InterfacePrivate::kde_screen_edge_v1_destroy(Resource *resource)
{
    wl_resource_destroy(resource->handle);
}

void ScreenEdgeV1InterfacePrivate::kde_screen_edge_v1_show(Resource *resource)
{
    Q_EMIT q->showRequested();
}

void ScreenEdgeV1InterfacePrivate::kde_screen_edge_v1_hide(Resource *resource)
{
    Q_EMIT q->hideRequested();
}

ScreenEdgeV1Interface::ScreenEdgeV1Interface(SurfaceInterface *surface, ElectricBorder border, wl_resource *resource)
    : d(new ScreenEdgeV1InterfacePrivate(this, surface, border, resource))
{
}

ScreenEdgeV1Interface::~ScreenEdgeV1Interface()
{
}

SurfaceInterface *ScreenEdgeV1Interface::surface() const
{
    return d->surface;
}

ElectricBorder ScreenEdgeV1Interface::border() const
{
    return d->border;
}

void ScreenEdgeV1Interface::sendShown()
{
    d->send_shown();
}

void ScreenEdgeV1Interface::sendHidden()
{
    d->send_hidden();
}

} // namespace KWaylandServer
