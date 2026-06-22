/*
 * Genesi UI kit — tiny runtime i18n helper, shared by every Genesi Qt6/QML app.
 *
 * English is the DEFAULT; Portuguese (pt-BR) is the alternate. The choice is
 * persisted (QtCore Settings) and switching is LIVE: every `i18n.t("key")`
 * binding re-evaluates when `lang` changes, because t() reads `lang` during
 * evaluation and QML captures that as a dependency. No .ts/.qm build step.
 *
 * Usage (mirror the Theme pattern — instantiate once per window root):
 *     I18n { id: i18n }
 *     QQC2.Label { text: i18n.t("nav.dashboard") }
 *     // toggle from a button: i18n.toggle()
 *
 * Add a key to BOTH dictionaries below. Missing keys fall back to English, then
 * to the raw key, so a half-translated string is never blank.
 *
 * Canonical source: genesi-arch/packages/genesi-ui-kit/components/. Bundled into
 * each app at build (see README) — NOT a published package.
 */
import QtQuick
import QtCore

Item {
    id: i18n
    visible: false
    width: 0; height: 0

    // "en" (default) or "pt". Persisted across launches.
    property string lang: "en"
    function toggle() { lang = (lang === "en" ? "pt" : "en") }
    // Short label for a switch button ("EN" / "PT").
    readonly property string code: lang === "pt" ? "PT" : "EN"

    Settings {
        id: _store
        category: "Genesi/i18n"
        property alias lang: i18n.lang
    }

    function t(key) {
        var table = _d[lang] || _d["en"]
        if (table && table[key] !== undefined) return table[key]
        if (_d["en"][key] !== undefined) return _d["en"][key]
        return key
    }

    readonly property var _d: ({
        "en": {
            // nav / shell
            "nav.dashboard": "Dashboard",
            "nav.chat": "AI Chat",
            "nav.models": "Models",
            "nav.settings": "Settings",
            "lang.tooltip": "Switch language (English / Português)",
            // mode segmented control
            "mode.on": "Force ON",
            "mode.auto": "Auto",
            "mode.off": "Force OFF",
            // profile segmented control
            "prof.max": "Maximum",
            "prof.balanced": "Balanced",
            "prof.battery": "Battery",
            "prof.auto": "Auto",
            // hero card
            "hero.off": "AI Mode OFF",
            "hero.onMax": "AI Mode ON · maximum",
            "hero.onBalanced": "AI Mode ON · balanced",
            "hero.onBattery": "AI Mode ON · battery",
            "hero.onEconomy": "AI Mode ON · economy",
            "hero.generating": "● generating",
            "hero.warm": "○ model warm · idle",
            "hero.standby": "○ standing by",
            "hero.optReal": "Optimizations applied in real time",
            "hero.noTweaks": "No tweaks applied",
            // metric cards
            "card.cpu": "CPU",
            "card.memory": "MEMORY",
            "card.inference": "INFERENCE",
            "u.cores": "cores",
            "u.threads": "threads",
            "u.inUse": "MB in use",
            "u.noModel": "no model",
            "u.active": "Active",
            // turbo card
            "turbo.title": "Turbo Mode",
            "turbo.speculative": "⚡ speculative",
            "turbo.fullOffload": "full offload",
            "turbo.installBackend": "Install Backend",
            "turbo.backend": "Backend: CUDA / Vulkan",
            "turbo.recommendedGpu": "Recommended for your GPU:",
            "turbo.descSpec": "Advanced mode: ⚡ speculative decoding + dynamic draft + persistent KV cache on disk.",
            "turbo.descFull": "Full GPU offload (stable). Flip ⚡ for the advanced stack.",
            // optimizations card
            "opt.applied": "Optimizations applied",
            "opt.inactive": "Inactive — no tweaks applied",
            "opt.hint": "Start a local model (or use Force ON) to see live tuning here.",
            // benchmark card
            "bench.title": "Performance benchmark",
            "bench.run": "Run benchmark",
            "bench.measuring": "Measuring…",
            "bench.gain": "% generation gain with AI Mode ON",
            "bench.vmNote": "  ·  in a VM the governor is a no-op; run on bare metal for the real gain"
        },
        "pt": {
            // nav / shell
            "nav.dashboard": "Painel",
            "nav.chat": "Chat IA",
            "nav.models": "Modelos",
            "nav.settings": "Ajustes",
            "lang.tooltip": "Trocar idioma (English / Português)",
            // mode segmented control
            "mode.on": "Forçar ON",
            "mode.auto": "Auto",
            "mode.off": "Forçar OFF",
            // profile segmented control
            "prof.max": "Máximo",
            "prof.balanced": "Equilibrado",
            "prof.battery": "Bateria",
            "prof.auto": "Auto",
            // hero card
            "hero.off": "AI Mode desligado",
            "hero.onMax": "AI Mode ligado · máximo",
            "hero.onBalanced": "AI Mode ligado · equilibrado",
            "hero.onBattery": "AI Mode ligado · bateria",
            "hero.onEconomy": "AI Mode ligado · economia",
            "hero.generating": "● gerando",
            "hero.warm": "○ modelo aquecido · ocioso",
            "hero.standby": "○ em espera",
            "hero.optReal": "Otimizações aplicadas em tempo real",
            "hero.noTweaks": "Nenhum ajuste aplicado",
            // metric cards
            "card.cpu": "CPU",
            "card.memory": "MEMÓRIA",
            "card.inference": "INFERÊNCIA",
            "u.cores": "núcleos",
            "u.threads": "threads",
            "u.inUse": "MB em uso",
            "u.noModel": "sem modelo",
            "u.active": "Ativo",
            // turbo card
            "turbo.title": "Modo Turbo",
            "turbo.speculative": "⚡ especulativo",
            "turbo.fullOffload": "offload total",
            "turbo.installBackend": "Instalar backend",
            "turbo.backend": "Backend: CUDA / Vulkan",
            "turbo.recommendedGpu": "Recomendado pra sua GPU:",
            "turbo.descSpec": "Modo avançado: ⚡ decodificação especulativa + draft dinâmico + cache KV persistente em disco.",
            "turbo.descFull": "Offload total na GPU (estável). Ative o ⚡ pro modo avançado.",
            // optimizations card
            "opt.applied": "Otimizações aplicadas",
            "opt.inactive": "Inativo — nenhum ajuste aplicado",
            "opt.hint": "Inicie um modelo local (ou use Forçar ON) pra ver os ajustes ao vivo aqui.",
            // benchmark card
            "bench.title": "Benchmark de desempenho",
            "bench.run": "Rodar benchmark",
            "bench.measuring": "Medindo…",
            "bench.gain": "% de ganho na geração com o AI Mode ligado",
            "bench.vmNote": "  ·  numa VM o governor não faz efeito; rode em bare metal pro ganho real"
        }
    })
}
