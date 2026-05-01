import QtQuick 2.15
import QtQuick.Layouts 1.15
import org.kde.plasma.core 2.0 as PlasmaCore
import org.kde.plasma.plasmoid 2.0
import org.kde.plasma.components 3.0 as PlasmaComponents3
import org.kde.kirigami 2.20 as Kirigami

Item {
    id: root
    
    property int updateCount: 0
    property var packages: []
    property string lastCheck: ""
    property bool hasUpdates: updateCount > 0
    
    Plasmoid.preferredRepresentation: Plasmoid.compactRepresentation
    Plasmoid.icon: hasUpdates ? "system-software-update" : "update-none"
    Plasmoid.toolTipMainText: hasUpdates ? 
        (updateCount === 1 ? "1 update available" : updateCount + " updates available") :
        "System is up to date"
    Plasmoid.toolTipSubText: lastCheck ? "Last checked: " + lastCheck : "Checking for updates..."
    
    // Compact representation (taskbar icon)
    Plasmoid.compactRepresentation: Item {
        Layout.minimumWidth: Kirigami.Units.iconSizes.small
        Layout.minimumHeight: Kirigami.Units.iconSizes.small
        
        Kirigami.Icon {
            id: icon
            anchors.fill: parent
            source: hasUpdates ? "system-software-update" : "update-none"
            active: mouseArea.containsMouse
            
            // Pulsing animation when updates available
            SequentialAnimation on opacity {
                running: hasUpdates
                loops: Animation.Infinite
                NumberAnimation { from: 1.0; to: 0.5; duration: 1000 }
                NumberAnimation { from: 0.5; to: 1.0; duration: 1000 }
            }
        }
        
        // Badge with update count
        Rectangle {
            visible: hasUpdates
            anchors.right: parent.right
            anchors.top: parent.top
            width: Math.max(badgeText.width + 8, 20)
            height: 20
            radius: 10
            color: "#00d9ff"
            border.color: "#00a8cc"
            border.width: 1
            
            Text {
                id: badgeText
                anchors.centerIn: parent
                text: updateCount > 99 ? "99+" : updateCount
                color: "#1a1a2e"
                font.pixelSize: 10
                font.bold: true
            }
        }
        
        MouseArea {
            id: mouseArea
            anchors.fill: parent
            hoverEnabled: true
            onClicked: {
                if (hasUpdates) {
                    // Open Discover
                    executable.exec("plasma-discover --mode Update")
                } else {
                    // Show full representation
                    plasmoid.expanded = !plasmoid.expanded
                }
            }
        }
    }
    
    // Full representation (popup)
    Plasmoid.fullRepresentation: Item {
        Layout.minimumWidth: 350
        Layout.minimumHeight: hasUpdates ? 400 : 200
        Layout.preferredWidth: 400
        Layout.preferredHeight: hasUpdates ? 500 : 250
        
        ColumnLayout {
            anchors.fill: parent
            anchors.margins: Kirigami.Units.smallSpacing
            spacing: Kirigami.Units.smallSpacing
            
            // Header
            RowLayout {
                Layout.fillWidth: true
                
                Kirigami.Icon {
                    source: hasUpdates ? "system-software-update" : "update-none"
                    Layout.preferredWidth: 32
                    Layout.preferredHeight: 32
                }
                
                PlasmaComponents3.Label {
                    text: hasUpdates ? 
                        (updateCount === 1 ? "1 Update Available" : updateCount + " Updates Available") :
                        "System Up to Date"
                    font.pixelSize: 18
                    font.bold: true
                    Layout.fillWidth: true
                }
                
                PlasmaComponents3.Button {
                    icon.name: "view-refresh"
                    text: "Check"
                    onClicked: checkUpdates()
                }
            }
            
            Kirigami.Separator {
                Layout.fillWidth: true
            }
            
            // Last check time
            PlasmaComponents3.Label {
                text: lastCheck ? "Last checked: " + lastCheck : "Checking..."
                opacity: 0.7
                font.pixelSize: 11
            }
            
            // Update list
            ScrollView {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: hasUpdates
                
                ListView {
                    id: updateList
                    model: packages
                    clip: true
                    
                    delegate: Item {
                        width: updateList.width
                        height: 60
                        
                        Rectangle {
                            anchors.fill: parent
                            anchors.margins: 2
                            color: delegateMouseArea.containsMouse ? "#2d2d30" : "transparent"
                            radius: 4
                            
                            RowLayout {
                                anchors.fill: parent
                                anchors.margins: 8
                                spacing: 8
                                
                                Kirigami.Icon {
                                    source: "package-x-generic"
                                    Layout.preferredWidth: 32
                                    Layout.preferredHeight: 32
                                }
                                
                                ColumnLayout {
                                    Layout.fillWidth: true
                                    spacing: 2
                                    
                                    PlasmaComponents3.Label {
                                        text: modelData.name
                                        font.bold: true
                                        Layout.fillWidth: true
                                    }
                                    
                                    PlasmaComponents3.Label {
                                        text: modelData.old_version + " → " + modelData.new_version
                                        opacity: 0.7
                                        font.pixelSize: 10
                                        Layout.fillWidth: true
                                    }
                                }
                            }
                            
                            MouseArea {
                                id: delegateMouseArea
                                anchors.fill: parent
                                hoverEnabled: true
                            }
                        }
                    }
                }
            }
            
            // No updates message
            Item {
                Layout.fillWidth: true
                Layout.fillHeight: true
                visible: !hasUpdates
                
                ColumnLayout {
                    anchors.centerIn: parent
                    spacing: Kirigami.Units.largeSpacing
                    
                    Kirigami.Icon {
                        source: "checkmark"
                        Layout.preferredWidth: 64
                        Layout.preferredHeight: 64
                        Layout.alignment: Qt.AlignHCenter
                        color: "#00d9ff"
                    }
                    
                    PlasmaComponents3.Label {
                        text: "Your system is up to date!"
                        font.pixelSize: 14
                        Layout.alignment: Qt.AlignHCenter
                    }
                }
            }
            
            Kirigami.Separator {
                Layout.fillWidth: true
                visible: hasUpdates
            }
            
            // Action buttons
            RowLayout {
                Layout.fillWidth: true
                visible: hasUpdates
                
                PlasmaComponents3.Button {
                    text: "Update All"
                    icon.name: "system-software-update"
                    Layout.fillWidth: true
                    onClicked: {
                        executable.exec("plasma-discover --mode Update")
                        plasmoid.expanded = false
                    }
                }
                
                PlasmaComponents3.Button {
                    text: "Details"
                    icon.name: "documentinfo"
                    onClicked: {
                        executable.exec("konsole -e 'checkupdates; read -p \"Press Enter to close...\"'")
                    }
                }
            }
        }
    }
    
    // Data source to read state file
    PlasmaCore.DataSource {
        id: executable
        engine: "executable"
        connectedSources: []
        
        function exec(cmd) {
            executable.connectSource(cmd)
        }
    }
    
    // Timer to refresh data
    Timer {
        interval: 5000 // 5 seconds
        running: true
        repeat: true
        onTriggered: loadState()
    }
    
    // Load state from file
    function loadState() {
        var xhr = new XMLHttpRequest()
        xhr.open("GET", "file:///var/lib/genesi-updater/state.json")
        xhr.onreadystatechange = function() {
            if (xhr.readyState === XMLHttpRequest.DONE) {
                if (xhr.status === 200) {
                    try {
                        var data = JSON.parse(xhr.responseText)
                        updateCount = data.updates_available || 0
                        packages = data.packages || []
                        lastCheck = data.last_check ? formatDate(data.last_check) : ""
                    } catch (e) {
                        console.log("Error parsing state:", e)
                    }
                }
            }
        }
        xhr.send()
    }
    
    // Format ISO date to readable format
    function formatDate(isoDate) {
        var date = new Date(isoDate)
        var now = new Date()
        var diff = Math.floor((now - date) / 1000) // seconds
        
        if (diff < 60) return "just now"
        if (diff < 3600) return Math.floor(diff / 60) + " minutes ago"
        if (diff < 86400) return Math.floor(diff / 3600) + " hours ago"
        return Math.floor(diff / 86400) + " days ago"
    }
    
    // Check for updates manually
    function checkUpdates() {
        executable.exec("/usr/local/bin/genesi-updater")
        // Reload after 3 seconds
        Qt.callLater(function() {
            loadStateTimer.start()
        })
    }
    
    Timer {
        id: loadStateTimer
        interval: 3000
        onTriggered: loadState()
    }
    
    // Load state on startup
    Component.onCompleted: {
        loadState()
    }
}
