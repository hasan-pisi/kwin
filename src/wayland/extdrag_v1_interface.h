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
class ExtDragV1InterfacePrivate;
class ExtDragOfferV1InterfacePrivate;
class ExtDragSourceV1InterfacePrivate;
class SurfaceInterface;

class ExtDragSourceV1Interface : public QObject
{
    Q_OBJECT
public:
    enum class Option {
        AllowSwallow = 1,
        AllowDropNoTarget,
        LockCursor,
    };
    Q_FLAGS(Option)

    ~ExtDragSourceV1Interface() override;

    DataSourceInterface *dataSource();
    void sendSwallow(const QString &mimeType);
    void sendUnswallow(const QString &mimeType, const QPoint &offset);
    SurfaceInterface *surface() const;
    QPoint offset() const;
    QFlags<Option> options() const;

private:
    ExtDragSourceV1Interface(wl_resource *resource, DataSourceInterface *dataSource, QFlags<Option> options);
    std::unique_ptr<ExtDragSourceV1InterfacePrivate> d;
    friend class ExtDragV1InterfacePrivate;
};

// class ExtDragOfferV1Interface : public QObject
// {
//     Q_OBJECT
// public:
//     ~ExtDragOfferV1Interface() override;
// Q_SIGNALS:
//     void swallowRequested(int serial, const QString &mimeType);
//     void unswallowRequested(int serial, const QString &mimeType, const QPoint &offset);
//
// private:
//     ExtDragOfferV1Interface(wl_resource *resource, DataOfferInterface *dataOffer);
//     std::unique_ptr<ExtDragOfferV1InterfacePrivate> d;
//     friend class ExtDragV1InterfacePrivate;
// };

class ExtDragV1Interface : public QObject
{
    Q_OBJECT
public:
    ExtDragV1Interface(Display *display, QObject *parent = nullptr);
    ~ExtDragV1Interface() override;
Q_SIGNALS:
    void extendedDragSourceCreated(ExtDragSourceV1Interface *extDragSource);
    // void extendedDragOfferCreated(ExtDragOfferV1Interface *extDragOffer);

private:
    std::unique_ptr<ExtDragV1InterfacePrivate> d;
};
}
