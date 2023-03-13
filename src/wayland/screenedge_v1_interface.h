/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#pragma once

#include "kwinglobals.h"

#include <QObject>
#include <memory>

struct wl_resource;

namespace KWaylandServer
{

class Display;
class ScreenEdgeManagerV1InterfacePrivate;
class ScreenEdgeV1Interface;
class ScreenEdgeV1InterfacePrivate;
class SurfaceInterface;

class KWIN_EXPORT ScreenEdgeManagerV1Interface : public QObject
{
    Q_OBJECT

public:
    explicit ScreenEdgeManagerV1Interface(Display *display, QObject *parent = nullptr);
    ~ScreenEdgeManagerV1Interface() override;

Q_SIGNALS:
    void edgeRequested(ScreenEdgeV1Interface *edge);

private:
    std::unique_ptr<ScreenEdgeManagerV1InterfacePrivate> d;
};

class KWIN_EXPORT ScreenEdgeV1Interface : public QObject
{
    Q_OBJECT

public:
    ScreenEdgeV1Interface(SurfaceInterface *surface, KWin::ElectricBorder border, wl_resource *resource);
    ~ScreenEdgeV1Interface() override;

    SurfaceInterface *surface() const;
    KWin::ElectricBorder border() const;

    void sendShown();
    void sendHidden();

Q_SIGNALS:
    void showRequested();
    void hideRequested();

private:
    std::unique_ptr<ScreenEdgeV1InterfacePrivate> d;
};

} // namespace KWaylandServer
