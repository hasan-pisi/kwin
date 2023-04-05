#pragma once

#include <qwayland-server-wayland.h>

#include "datadevicemanager_interface.h"

namespace KWaylandServer
{
class DataSourceInterface;
class XdgToplevelDragV1Interface;

class DataSourceInterfacePrivate : public QtWaylandServer::wl_data_source
{
public:
    DataSourceInterfacePrivate(DataSourceInterface *_q, ::wl_resource *resource);

    static DataSourceInterfacePrivate *get(DataSourceInterface *dataSource);

    DataSourceInterface *q;
    QStringList mimeTypes;
    DataDeviceManagerInterface::DnDActions supportedDnDActions = DataDeviceManagerInterface::DnDAction::None;
    DataDeviceManagerInterface::DnDAction selectedDndAction = DataDeviceManagerInterface::DnDAction::None;
    bool isAccepted = false;
    XdgToplevelDragV1Interface *xdgToplevelDrag = nullptr;

protected:
    void data_source_destroy_resource(Resource *resource) override;
    void data_source_offer(Resource *resource, const QString &mime_type) override;
    void data_source_destroy(Resource *resource) override;
    void data_source_set_actions(Resource *resource, uint32_t dnd_actions) override;

private:
    void offer(const QString &mimeType);
};
}
