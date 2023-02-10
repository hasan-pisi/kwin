/*
    SPDX-FileCopyrightText: 2023 Vlad Zahorodnii <vlad.zahorodnii@kde.org>

    SPDX-License-Identifier: GPL-2.0-or-later
*/

#include "sceneeffectitem.h"
#include "main.h"

#include <KConfigGroup>
#include <KConfigLoader>

#include <QFile>

namespace KWin
{

SceneEffectItem::SceneEffectItem()
{
    m_visibleTimer.setSingleShot(true);
    connect(&m_visibleTimer, &QTimer::timeout, this, [this]() {
        setRunning(false);
    });
}

SceneEffectItem::~SceneEffectItem()
{
}

int SceneEffectItem::requestedEffectChainPosition() const
{
    return m_requestedEffectChainPosition;
}

void SceneEffectItem::setMetaData(const KPluginMetaData &metaData)
{
    m_requestedEffectChainPosition = metaData.value(QStringLiteral("X-KDE-Ordering"), 50);

    KConfigGroup cg = kwinApp()->config()->group(QStringLiteral("Effect-%1").arg(metaData.pluginId()));
    const QString configFilePath = QStandardPaths::locate(QStandardPaths::GenericDataLocation, QLatin1String("kwin/effects/") + metaData.pluginId() + QLatin1String("/contents/config/main.xml"));
    if (configFilePath.isNull()) {
        m_configLoader = new KConfigLoader(cg, nullptr, this);
    } else {
        QFile xmlFile(configFilePath);
        m_configLoader = new KConfigLoader(cg, &xmlFile, this);
        m_configLoader->load();
    }

    m_configuration = new KConfigPropertyMap(m_configLoader, this);
    connect(m_configLoader, &KConfigLoader::configChanged, this, &SceneEffectItem::configurationChanged);
}

bool SceneEffectItem::isVisible() const
{
    return m_isVisible;
}

void SceneEffectItem::setVisible(bool visible)
{
    if (m_isVisible == visible) {
        return;
    }
    m_isVisible = visible;

    if (m_isVisible) {
        m_visibleTimer.stop();
        setRunning(true);
    } else {
        m_visibleTimer.start();
    }

    Q_EMIT visibleChanged();
}

KConfigPropertyMap *SceneEffectItem::configuration() const
{
    return m_configuration;
}

QQmlListProperty<QObject> SceneEffectItem::data()
{
    return QQmlListProperty<QObject>(this, nullptr,
                                     data_append,
                                     data_count,
                                     data_at,
                                     data_clear);
}

void SceneEffectItem::data_append(QQmlListProperty<QObject> *objects, QObject *object)
{
    if (!object) {
        return;
    }

    SceneEffectItem *effect = static_cast<SceneEffectItem *>(objects->object);
    if (!effect->m_children.contains(object)) {
        object->setParent(effect);
        effect->m_children.append(object);
    }
}

qsizetype SceneEffectItem::data_count(QQmlListProperty<QObject> *objects)
{
    SceneEffectItem *effect = static_cast<SceneEffectItem *>(objects->object);
    return effect->m_children.count();
}

QObject *SceneEffectItem::data_at(QQmlListProperty<QObject> *objects, qsizetype index)
{
    SceneEffectItem *effect = static_cast<SceneEffectItem *>(objects->object);
    return effect->m_children.value(index);
}

void SceneEffectItem::data_clear(QQmlListProperty<QObject> *objects)
{
    SceneEffectItem *effect = static_cast<SceneEffectItem *>(objects->object);
    while (!effect->m_children.isEmpty()) {
        QObject *child = effect->m_children.takeLast();
        child->setParent(nullptr);
    }
}

} // namespace KWin
