/*
    KWin - the KDE window manager
    This file is part of the KDE project.

    SPDX-FileCopyrightText: 2016 Roman Gilg <subdiff@gmail.com>

    SPDX-License-Identifier: GPL-2.0-or-later
*/
#include "drm_object.h"

#include <errno.h>

#include "drm_atomic_commit.h"
#include "drm_gpu.h"
#include "drm_logging.h"
#include "drm_pointer.h"

namespace KWin
{

DrmObject::DrmObject(DrmGpu *gpu, uint32_t objectId, const QVector<PropertyDefinition> &&vector, uint32_t objectType)
    : m_gpu(gpu)
    , m_id(objectId)
    , m_objectType(objectType)
    , m_propertyDefinitions(vector)
{
    m_props.resize(m_propertyDefinitions.count());
}

bool DrmObject::updateProperties()
{
    DrmUniquePtr<drmModeObjectProperties> properties(drmModeObjectGetProperties(m_gpu->fd(), m_id, m_objectType));
    if (!properties) {
        qCWarning(KWIN_DRM) << "Failed to get properties for object" << m_id;
        return false;
    }
    for (int propIndex = 0; propIndex < m_propertyDefinitions.count(); propIndex++) {
        const PropertyDefinition &def = m_propertyDefinitions[propIndex];
        bool found = false;
        for (uint32_t drmPropIndex = 0; drmPropIndex < properties->count_props; drmPropIndex++) {
            DrmUniquePtr<drmModePropertyRes> prop(drmModeGetProperty(m_gpu->fd(), properties->props[drmPropIndex]));
            if (!prop) {
                qCWarning(KWIN_DRM, "Getting property %d of object %d failed!", drmPropIndex, m_id);
                continue;
            }
            if (def.name == prop->name) {
                if (m_props[propIndex]) {
                    m_props[propIndex]->setCurrent(properties->prop_values[drmPropIndex]);
                } else {
                    m_props[propIndex] = std::make_unique<DrmProperty>(this, prop.get(), properties->prop_values[drmPropIndex], def.enumNames);
                }
                found = true;
                break;
            }
        }
        if (!found) {
            m_props[propIndex].reset();
        }
    }
    for (int i = 0; i < m_propertyDefinitions.count(); i++) {
        bool required = m_gpu->atomicModeSetting() ? m_propertyDefinitions[i].requirement == Requirement::Required
                                                   : m_propertyDefinitions[i].requirement == Requirement::RequiredForLegacy;
        if (!m_props[i] && required) {
            qCWarning(KWIN_DRM, "Required property %s for object %d not found!", qPrintable(m_propertyDefinitions[i].name), m_id);
            return false;
        }
    }
    return true;
}

uint32_t DrmObject::id() const
{
    return m_id;
}

DrmGpu *DrmObject::gpu() const
{
    return m_gpu;
}

uint32_t DrmObject::type() const
{
    return m_objectType;
}

QString DrmObject::typeName() const
{
    switch (m_objectType) {
    case DRM_MODE_OBJECT_CONNECTOR:
        return QStringLiteral("connector");
    case DRM_MODE_OBJECT_CRTC:
        return QStringLiteral("crtc");
    case DRM_MODE_OBJECT_PLANE:
        return QStringLiteral("plane");
    default:
        return QStringLiteral("unknown?");
    }
}
}

QDebug operator<<(QDebug s, const KWin::DrmObject *obj)
{
    QDebugStateSaver saver(s);
    if (obj) {
        s.nospace() << "DrmObject(id=" << obj->id() << ", gpu=" << obj->gpu() << ')';
    } else {
        s << "DrmObject(0x0)";
    }
    return s;
}
