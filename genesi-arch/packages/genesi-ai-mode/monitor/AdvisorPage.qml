/*
 * Genesi AI Mode Monitor — Model Advisor page.
 * Shows `genesi-ai-mode advise` (which model fits 100% on this GPU/CPU) — the
 * single biggest speed lever is picking a model that doesn't spill to CPU.
 */
import QtQuick
import QtQuick.Controls as QQC2
import org.kde.kirigami as Kirigami

Kirigami.ScrollablePage {
    id: page
    title: "Modelos recomendados"

    actions: [
        Kirigami.Action {
            text: "Recarregar"
            icon.name: "view-refresh"
            onTriggered: page.reload()
        }
    ]

    function reload() { area.text = backend.advise() }
    Component.onCompleted: reload()

    QQC2.TextArea {
        id: area
        readOnly: true
        wrapMode: TextEdit.NoWrap
        textFormat: TextEdit.PlainText
        font.family: "monospace"
        background: null
    }
}
