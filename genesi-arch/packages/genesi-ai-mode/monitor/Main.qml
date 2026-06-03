/*
 * Genesi AI Mode Monitor — Kirigami dashboard.
 * Polls backend.state() (the daemon's state.json) and drives backend.setMode().
 */
import QtQuick
import QtQuick.Layouts
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ApplicationWindow {
    id: win
    title: "Genesi AI Mode Monitor"
    width: Kirigami.Units.gridUnit * 34
    height: Kirigami.Units.gridUnit * 30
    minimumWidth: Kirigami.Units.gridUnit * 26
    minimumHeight: Kirigami.Units.gridUnit * 22

    readonly property color genesiGreen: "#1D9E75"

    property var st: ({})
    property bool active: false
    property string forceMode: "auto"
    property var tpsHistory: []

    function num(v, suffix) { return v === undefined || v === null ? "—" : v + (suffix || "") }
    function metrics() { return st.metrics || ({}) }
    function gpus() { return (st.metrics && st.metrics.gpus) || [] }
    function hw() { return st.hardware || ({}) }

    function poll() {
        var txt = backend.state()
        try {
            st = JSON.parse(txt)
        } catch (e) {
            st = ({})
        }
        active = st.ai_mode_active || false
        forceMode = st.force_mode || "auto"
        var tps = st.tokens_per_second || 0
        if (active && tps > 0) {
            var h = tpsHistory.slice()
            h.push(tps)
            if (h.length > 60) h.shift()
            tpsHistory = h
            spark.requestPaint()
        }
    }

    Timer { interval: 2000; running: true; repeat: true; triggeredOnStart: true; onTriggered: win.poll() }

    pageStack.initialPage: Kirigami.ScrollablePage {
        title: "AI Mode"
        actions: [
            Kirigami.Action {
                text: "Chat com a IA"; icon.name: "dialog-messages"
                onTriggered: win.pageStack.push(Qt.resolvedUrl("ChatPage.qml"))
            },
            Kirigami.Action {
                text: "Force ON"; icon.name: "run-build"
                checked: win.forceMode === "on"
                onTriggered: backend.setMode("on")
            },
            Kirigami.Action {
                text: "Auto"; icon.name: "view-refresh"
                checked: win.forceMode === "auto"
                onTriggered: backend.setMode("auto")
            },
            Kirigami.Action {
                text: "Force OFF"; icon.name: "dialog-cancel"
                checked: win.forceMode === "off"
                onTriggered: backend.setMode("off")
            }
        ]

        ColumnLayout {
            spacing: Kirigami.Units.largeSpacing

            // ── status banner ─────────────────────────────────────────────
            Kirigami.AbstractCard {
                Layout.fillWidth: true
                contentItem: RowLayout {
                    spacing: Kirigami.Units.largeSpacing
                    Kirigami.Icon {
                        source: "cpu"
                        Layout.preferredWidth: Kirigami.Units.iconSizes.huge
                        Layout.preferredHeight: Kirigami.Units.iconSizes.huge
                        color: win.active ? win.genesiGreen : Kirigami.Theme.disabledTextColor
                    }
                    ColumnLayout {
                        Layout.fillWidth: true
                        spacing: 0
                        Kirigami.Heading {
                            level: 1
                            text: win.active
                                  ? (win.forceMode === "on" ? "AI Mode ON (forced)"
                                     : st.aggressive ? "AI Mode ON (max)" : "AI Mode ON (battery-safe)")
                                  : (win.forceMode === "off" ? "AI Mode OFF (forced)" : "AI Mode OFF")
                            color: win.active ? win.genesiGreen : Kirigami.Theme.textColor
                        }
                        QQC2.Label {
                            opacity: 0.7
                            text: {
                                var h = win.hw()
                                var g = win.gpus()
                                var parts = []
                                if (h.cpu_vendor) parts.push(h.cpu_vendor + " " + (h.physical_cores||"?") + "c/" + (h.logical_cores||"?") + "t")
                                if (h.ram_mb) parts.push(Math.round(h.ram_mb/1024) + "GB RAM")
                                if (h.chassis) parts.push(h.chassis + (h.virtualized ? " (VM)" : ""))
                                return parts.join("  •  ")
                            }
                        }
                    }
                    ColumnLayout {
                        visible: !!st.tokens_per_second
                        spacing: 0
                        Kirigami.Heading {
                            level: 1
                            text: win.num(st.tokens_per_second)
                            color: win.genesiGreen
                            horizontalAlignment: Text.AlignRight
                            Layout.alignment: Qt.AlignRight
                        }
                        QQC2.Label { text: "tokens/s"; opacity: 0.7; Layout.alignment: Qt.AlignRight }
                    }
                }
            }

            // ── tokens/s sparkline ────────────────────────────────────────
            Kirigami.AbstractCard {
                Layout.fillWidth: true
                visible: win.tpsHistory.length > 1
                contentItem: ColumnLayout {
                    QQC2.Label { text: "Generation rate (last ~2 min)"; font.bold: true }
                    Canvas {
                        id: spark
                        Layout.fillWidth: true
                        Layout.preferredHeight: Kirigami.Units.gridUnit * 5
                        onPaint: {
                            var ctx = getContext("2d")
                            ctx.reset()
                            var h = win.tpsHistory
                            if (h.length < 2) return
                            var max = Math.max.apply(null, h) * 1.15 || 1
                            var dx = width / (h.length - 1)
                            ctx.strokeStyle = win.genesiGreen
                            ctx.lineWidth = 2
                            ctx.beginPath()
                            for (var i = 0; i < h.length; i++) {
                                var x = i * dx
                                var y = height - (h[i] / max) * height
                                if (i === 0) ctx.moveTo(x, y); else ctx.lineTo(x, y)
                            }
                            ctx.stroke()
                        }
                    }
                }
            }

            // ── live metrics cards ────────────────────────────────────────
            Kirigami.CardsLayout {
                Layout.fillWidth: true
                Kirigami.AbstractCard {
                    contentItem: ColumnLayout {
                        QQC2.Label { text: "CPU"; opacity: 0.7 }
                        Kirigami.Heading { level: 2; text: win.num(win.metrics().cpu_percent, "%") }
                        QQC2.Label {
                            visible: win.metrics().cpu_temp_c != null
                            text: win.num(win.metrics().cpu_temp_c, "°C"); opacity: 0.7
                        }
                    }
                }
                Kirigami.AbstractCard {
                    contentItem: ColumnLayout {
                        QQC2.Label { text: "RAM"; opacity: 0.7 }
                        Kirigami.Heading {
                            level: 2
                            text: win.metrics().ram_total_mb
                                  ? Math.round((win.metrics().ram_used_mb||0)/1024*10)/10 + " / "
                                    + Math.round(win.metrics().ram_total_mb/1024) + " GB"
                                  : "—"
                        }
                    }
                }
                Repeater {
                    model: win.gpus()
                    Kirigami.AbstractCard {
                        contentItem: ColumnLayout {
                            QQC2.Label { text: "GPU " + (modelData.vendor || "?"); opacity: 0.7 }
                            Kirigami.Heading { level: 2; text: win.num(modelData.util, "%") }
                            QQC2.Label {
                                visible: modelData.vram_total_mb != null
                                text: (modelData.vram_used_mb||0) + "/" + (modelData.vram_total_mb||0) + " MB"
                                      + (modelData.temp_c != null ? "  •  " + modelData.temp_c + "°C" : "")
                                opacity: 0.7
                            }
                        }
                    }
                }
            }

            // ── loaded models ─────────────────────────────────────────────
            Kirigami.AbstractCard {
                Layout.fillWidth: true
                visible: (st.ollama || []).length > 0
                contentItem: ColumnLayout {
                    QQC2.Label { text: "Loaded models"; font.bold: true }
                    Repeater {
                        model: st.ollama || []
                        RowLayout {
                            Layout.fillWidth: true
                            Kirigami.Icon { source: "applications-science"; Layout.preferredWidth: Kirigami.Units.iconSizes.small; Layout.preferredHeight: Kirigami.Units.iconSizes.small }
                            QQC2.Label { Layout.fillWidth: true; elide: Text.ElideRight; text: modelData.name }
                            QQC2.Label { opacity: 0.7; text: (modelData.size_mb ? modelData.size_mb + "MB  " : "") + (modelData.where||"") }
                        }
                    }
                }
            }

            // ── applied optimizations ─────────────────────────────────────
            Kirigami.AbstractCard {
                Layout.fillWidth: true
                contentItem: ColumnLayout {
                    QQC2.Label {
                        text: win.active ? "Applied optimizations" : "Idle — nothing applied"
                        font.bold: true
                    }
                    Repeater {
                        model: st.applied || []
                        RowLayout {
                            Layout.fillWidth: true
                            Kirigami.Icon { source: "dialog-ok"; color: win.genesiGreen; Layout.preferredWidth: Kirigami.Units.iconSizes.small; Layout.preferredHeight: Kirigami.Units.iconSizes.small }
                            QQC2.Label { Layout.fillWidth: true; wrapMode: Text.WordWrap; text: modelData }
                        }
                    }
                    QQC2.Label {
                        visible: !win.active
                        opacity: 0.6
                        text: "Start a local model (or Force ON) to see live tuning here."
                    }
                }
            }
        }
    }
}
