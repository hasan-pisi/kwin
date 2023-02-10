/*
    SPDX-FileCopyrightText: 2013 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#pragma once

#include <kwinquickeffect.h>

#include <KConfigPropertyMap>

#include <QTimer>

class KConfigLoader;
class KConfigPropertyMap;

namespace KWin
{

class SceneEffectItem : public QuickSceneEffect
{
    Q_OBJECT
    Q_PROPERTY(QQmlListProperty<QObject> data READ data)
    Q_PROPERTY(KConfigPropertyMap *configuration READ configuration NOTIFY configurationChanged)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible NOTIFY visibleChanged)

    Q_CLASSINFO("DefaultProperty", "data")

public:
    explicit SceneEffectItem();
    ~SceneEffectItem() override;

    void setMetaData(const KPluginMetaData &metaData);

    int requestedEffectChainPosition() const override;

    bool isVisible() const;
    void setVisible(bool visible);

    QQmlListProperty<QObject> data();
    KConfigPropertyMap *configuration() const;

    static void data_append(QQmlListProperty<QObject> *objects, QObject *object);
    static qsizetype data_count(QQmlListProperty<QObject> *objects);
    static QObject *data_at(QQmlListProperty<QObject> *objects, qsizetype index);
    static void data_clear(QQmlListProperty<QObject> *objects);

Q_SIGNALS:
    void visibleChanged();
    void configurationChanged();

private:
    KConfigLoader *m_configLoader = nullptr;
    KConfigPropertyMap *m_configuration = nullptr;
    QObjectList m_children;
    QTimer m_visibleTimer;
    bool m_isVisible = false;
    int m_requestedEffectChainPosition = 0;
};

} // namespace KWin
