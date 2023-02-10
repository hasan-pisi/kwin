import QtQuick 2.15

import org.kde.kwin 3.0

SceneEffect {
    id: effect

    delegate: Rectangle {
        color: SceneView.effect.configuration.BackgroundColor

        Text {
            anchors.centerIn: parent
            text: SceneView.screen.name
        }

        MouseArea {
            anchors.fill: parent
            onClicked: SceneView.effect.visible = false
        }
    }

    ScreenEdgeItem {
        enabled: true
        edge: ScreenEdgeItem.TopEdge
        onActivated: effect.visible = !effect.visible
    }

    ShortcutHandler {
        name: "Toggle Quick Effect"
        text: "Toggle Quick Effect"
        sequence: "Meta+Ctrl+Q"
        onActivated: effect.visible = !effect.visible
    }

    PinchGestureHandler {
        direction: PinchGestureHandler.Direction.Contracting
        fingerCount: 3
        onActivated: effect.visible = !effect.visible
    }
}
