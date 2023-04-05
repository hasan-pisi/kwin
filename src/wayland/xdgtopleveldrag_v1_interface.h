#pragma once

#include <QObject>

#include <memory>

#include "datadevice_interface.h"

struct wl_resource;

namespace KWaylandServer
{

class DataOfferInterface;
class DataSourceInterface;
class Display;
class XdgToplevelDragV1InterfacePrivate;
class XdgToplevelDragManagerV1InterfacePrivate;
class XdgToplevelInterface;

class XdgToplevelDragV1Interface : public QObject
{
    Q_OBJECT
public:
    ~XdgToplevelDragV1Interface() override;

    DataSourceInterface *dataSource();
    XdgToplevelInterface *toplevel() const;
    QPoint offset() const;

Q_SIGNALS:
    void toplevelChanged();

private:
    XdgToplevelDragV1Interface(wl_resource *resource, DataSourceInterface *dataSource);
    std::unique_ptr<XdgToplevelDragV1InterfacePrivate> d;
    friend class XdgToplevelDragManagerV1InterfacePrivate;
};

class XdgToplevelDragManagerV1Interface : public QObject
{
    Q_OBJECT
public:
    XdgToplevelDragManagerV1Interface(Display *display, QObject *parent = nullptr);
    ~XdgToplevelDragManagerV1Interface() override;

private:
    std::unique_ptr<XdgToplevelDragManagerV1InterfacePrivate> d;
};
}
