/*
    SPDX-FileCopyrightText: 2016 Martin Graesslin <mgraesslin@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

import QtQuick
import QtQuick.Layouts
import QtQuick.Window

import org.kde.plasma.core as PlasmaCore
import org.kde.plasma.components 3.0 as PlasmaComponents3

PlasmaCore.Dialog {
    location: PlasmaCore.Types.Floating
    visible: osd.visible
    flags: Qt.FramelessWindowHint
    type: PlasmaCore.Dialog.OnScreenDisplay
    outputOnly: true

    mainItem: RowLayout {
        spacing: PlasmaCore.Units.smallSpacing
        PlasmaCore.IconItem {
            implicitWidth: PlasmaCore.Units.iconSizes.medium
            implicitHeight: implicitWidth
            source: osd.iconName
            visible: osd.iconName !== ""
        }
        PlasmaComponents3.Label {
            text: osd.message
        }
    }
}
