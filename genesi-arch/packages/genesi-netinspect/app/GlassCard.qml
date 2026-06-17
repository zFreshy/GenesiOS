/*
 * Genesi API Inspector — reusable "glass" surface card.
 * Dark branded panel with a subtle top sheen and an animated hover/active border.
 */
import QtQuick

Rectangle {
    id: card

    property color accent: "#1E382E"
    property bool active: false

    radius: 16
    color: "#0F1D18"
    border.width: 1
    border.color: active ? accent : (hov.hovered ? "#2A463B" : "#1E382E")
    Behavior on border.color { ColorAnimation { duration: 180 } }

    Rectangle {
        anchors.fill: parent
        radius: parent.radius
        z: 0
        gradient: Gradient {
            GradientStop { position: 0.0;  color: Qt.rgba(1, 1, 1, 0.05) }
            GradientStop { position: 0.18; color: Qt.rgba(1, 1, 1, 0.0) }
        }
    }

    HoverHandler { id: hov }
}
