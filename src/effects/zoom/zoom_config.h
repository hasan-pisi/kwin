/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2007 Rivo Laks <rivolaks@hot.ee>
    SPDX-FileCopyrightText: 2010 Sebastian Sauer <sebsauer@kdab.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <kcmodule.h>

#include "ui_zoom_config.h"

namespace KWin
{

class ZoomEffectConfigForm : public QWidget, public Ui::ZoomEffectConfigForm
{
    Q_OBJECT
public:
    explicit ZoomEffectConfigForm(QWidget *parent = nullptr);
};

class ZoomEffectConfig : public KCModule
{
    Q_OBJECT
public:
    explicit ZoomEffectConfig(QObject *parent, const KPluginMetaData &data, const QVariantList &args);
    ~ZoomEffectConfig() override;

public Q_SLOTS:
    void save() override;

private:
    ZoomEffectConfigForm *m_ui;
    enum MouseTracking {
        MouseCentred = 0,
        MouseProportional = 1,
        MouseDisabled = 2,
    };
};

} // namespace
