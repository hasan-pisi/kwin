#include "extdrag_v1_interface.h"

#include <qwayland-server-ext-drag-v1.h>

#include "dataoffer_interface.h"
#include "datasource_interface.h"
#include "datasource_interface_p.h"

#include "display.h"
#include "surface_interface.h"
#include "utils.h"

#include <memory>

namespace KWaylandServer
{
constexpr int version = 1;

class ExtDragSourceV1InterfacePrivate : public QtWaylandServer::zcr_extended_drag_source_v1
{
public:
    ExtDragSourceV1InterfacePrivate(wl_resource *resource, ExtDragSourceV1Interface *q)
        : zcr_extended_drag_source_v1(resource)
        , q(q)
    {
    }
    ExtDragSourceV1Interface *q;
    QPointer<DataSourceInterface> dataSource;
    QFlags<ExtDragSourceV1Interface::Option> options;
    SurfaceInterface *surface = nullptr;
    QPoint pos;

private:
    void zcr_extended_drag_source_v1_drag(Resource *resource, wl_resource *surface, int32_t x_offset, int32_t y_offset) override
    {
        auto surfaceInterface = SurfaceInterface::get(surface);
        this->surface = surfaceInterface;
        pos = {x_offset, y_offset};
    }
    void zcr_extended_drag_source_v1_destroy_resource(Resource *resource) override
    {
        delete q;
    }
    void zcr_extended_drag_source_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
};

ExtDragSourceV1Interface::ExtDragSourceV1Interface(wl_resource *resource, DataSourceInterface *dataSource, QFlags<Option> options)
    : d(std::make_unique<ExtDragSourceV1InterfacePrivate>(resource, this))
{
    d->dataSource = dataSource;
    d->options = options;
    DataSourceInterfacePrivate::get(dataSource)->extendedDragSource = this;
}

void ExtDragSourceV1Interface::sendSwallow(const QString &mimeType)
{
    d->send_swallow(mimeType);
}

void ExtDragSourceV1Interface::sendUnswallow(const QString &mimeType, const QPoint &offset)
{
    d->send_unswallow(mimeType, offset.x(), offset.y());
}

SurfaceInterface *ExtDragSourceV1Interface::surface() const
{
    return d->surface;
}

QPoint ExtDragSourceV1Interface::offset() const
{
    return d->pos;
}

QFlags<ExtDragSourceV1Interface::Option> ExtDragSourceV1Interface::options() const
{
    return d->options;
}

ExtDragSourceV1Interface::~ExtDragSourceV1Interface() = default;
/*
class ExtDragOfferV1InterfacePrivate : public QtWaylandServer::zcr_extended_drag_offer_v1
{
public:
    ExtDragOfferV1InterfacePrivate(wl_resource *resource, ExtDragOfferV1Interface *q)
        : zcr_extended_drag_offer_v1(resource)
        , q(q)
    {
    }
    ExtDragOfferV1Interface *q;
    DataOfferInterface *dataOffer;

private:
    void zcr_extended_drag_offer_v1_swallow(Resource *resource, uint32_t serial, const QString &mime_type) override
    {
        Q_EMIT q->swallowRequested(serial, mime_type);
    }
    void zcr_extended_drag_offer_v1_unswallow(Resource *resource, uint32_t serial, const QString &mime_type, int32_t x_offset, int32_t y_offset) override
    {
        Q_EMIT q->unswallowRequested(serial, mime_type, {x_offset, y_offset});
    }
    void zcr_extended_drag_offer_v1_destroy_resource(Resource *resource) override
    {
        delete q;
    }
    void zcr_extended_drag_offer_v1_destroy(Resource *resource) override
    {
        wl_resource_destroy(resource->handle);
    }
};

ExtDragOfferV1Interface::ExtDragOfferV1Interface(wl_resource *resource, DataOfferInterface *dataOffer)
    : d(std::make_unique<ExtDragOfferV1InterfacePrivate>(resource, this))
{
    d->dataOffer = dataOffer;
}

ExtDragOfferV1Interface::~ExtDragOfferV1Interface() = default;*/

class ExtDragV1InterfacePrivate : public QtWaylandServer::zcr_extended_drag_v1
{
public:
    ExtDragV1InterfacePrivate(ExtDragV1Interface *q, Display *display)
        : zcr_extended_drag_v1(*display, version)
        , q(q)
    {
    }

private:
    void zcr_extended_drag_v1_get_extended_drag_source(Resource *resource, uint32_t id, wl_resource *data_source, uint32_t options) override
    {
        auto dataSource = DataSourceInterface::get(data_source);
        wl_resource *extendedSourceResource = wl_resource_create(resource->client(), &zcr_extended_drag_source_v1_interface, resource->version(), id);
        if (!extendedSourceResource) {
            wl_resource_post_no_memory(resource->handle);
            return;
        }
        auto extendedSource = new ExtDragSourceV1Interface(extendedSourceResource, dataSource, QFlags<ExtDragSourceV1Interface::Option>(options));
        Q_EMIT q->extendedDragSourceCreated(extendedSource);
    }
    void zcr_extended_drag_v1_get_extended_drag_offer(Resource *resource, uint32_t id, struct ::wl_resource *data_offer) override
    {
        // auto dataOffer = DataOfferInterface::get(data_offer);
        // wl_resource *extendedOfferResource = wl_resource_create(resource->client(), &zcr_extended_drag_offer_v1_interface, resource->version(), id);
        // if (!extendedOfferResource) {
        //     wl_resource_post_no_memory(resource->handle);
        //     return;
        // }
        // auto extendedOffer = new ExtDragOfferV1Interface(extendedOfferResource, dataOffer);
        // Q_EMIT q->extendedDragOfferCreated(extendedOffer);
    }
    ExtDragV1Interface *q;
};

ExtDragV1Interface::ExtDragV1Interface(KWaylandServer::Display *display, QObject *parent)
    : QObject(parent)
    , d(std::make_unique<ExtDragV1InterfacePrivate>(this, display))
{
}

ExtDragV1Interface::~ExtDragV1Interface() = default;

}
