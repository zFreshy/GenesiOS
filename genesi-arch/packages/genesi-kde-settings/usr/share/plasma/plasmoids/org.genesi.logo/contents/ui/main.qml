import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.core as PlasmaCore
import org.kde.kirigami as Kirigami

PlasmoidItem {
    id: root

    preferredRepresentation: fullRepresentation
    Layout.preferredWidth: logoRow.implicitWidth + Kirigami.Units.smallSpacing * 4
    Layout.preferredHeight: Kirigami.Units.iconSizes.medium

    fullRepresentation: MouseArea {
        id: mouseArea
        hoverEnabled: true
        Layout.fillHeight: true
        Layout.preferredWidth: logoRow.implicitWidth + Kirigami.Units.smallSpacing * 4

        onClicked: {
            // Open the app launcher using DBus
            var executable = "/usr/bin/qdbus"
            var args = ["org.kde.plasmashell", "/PlasmaShell", "activateLauncherMenu"]
            var p = Qt.createQmlObject('import QtQuick 2.0; QtObject { function run() { var process = Qt.createQmlObject("import org.kde.plasma.core 2.0 as PlasmaCore; PlasmaCore.DataSource { engine: \\"executable\\"; connectedSources: [\\"qdbus org.kde.plasmashell /PlasmaShell activateLauncherMenu\\"] }", parent, "dynamic_dbus"); } }', parent, "dynamic_dbus_call")
            // Wait, using executable data source is better
            var dataSource = Qt.createQmlObject('import org.kde.plasma.core 2.0 as PlasmaCore; PlasmaCore.DataSource { id: executable; engine: "executable"; connectedSources: []; onNewData: function(sourceName, data) { disconnectSource(sourceName); } }', root, "dynamicExec")
            dataSource.connectSource("qdbus org.kde.plasmashell /PlasmaShell activateLauncherMenu")
        }

        Row {
            id: logoRow
            anchors.centerIn: parent
            spacing: Kirigami.Units.smallSpacing

            Image {
                id: logoImage
                source: "/usr/share/icons/hicolor/64x64/apps/genesi.png"
                width: Kirigami.Units.iconSizes.smallMedium
                height: Kirigami.Units.iconSizes.smallMedium
                anchors.verticalCenter: parent.verticalCenter
                smooth: true
                mipmap: true

                scale: mouseArea.containsMouse ? 1.1 : 1.0
                Behavior on scale {
                    NumberAnimation { duration: 150; easing.type: Easing.OutCubic }
                }
            }

            Text {
                id: logoText
                text: "GenesiOS"
                color: "#E1F5EE"
                font.pixelSize: 14
                font.weight: Font.Bold
                font.family: "Noto Sans"
                anchors.verticalCenter: parent.verticalCenter

                opacity: mouseArea.containsMouse ? 1.0 : 0.85
                Behavior on opacity {
                    NumberAnimation { duration: 150 }
                }
            }
        }

        // Subtle glow effect on hover
        Rectangle {
            anchors.fill: parent
            radius: 8
            color: mouseArea.containsMouse ? "#101D9E75" : "transparent"
            Behavior on color {
                ColorAnimation { duration: 200 }
            }
        }
    }
}
