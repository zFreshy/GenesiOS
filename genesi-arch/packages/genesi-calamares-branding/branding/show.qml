/* === Genesi OS Installer Slideshow ===
 *
 *   SPDX-FileCopyrightText: 2026 Genesi OS Team
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.15
import calamares.slideshow 1.0

Presentation {
    id: presentation

    function nextSlide() {
        presentation.goToNextSlide()
    }

    Timer {
        interval: 5000
        running: presentation.activatedInCalamares
        repeat: true
        onTriggered: nextSlide()
    }

    // Slide 1: Welcome to Genesi OS
    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0f0d"
            
            Column {
                anchors.centerIn: parent
                spacing: 30
                
                Image {
                    source: "logo.png"
                    width: 200
                    height: 200
                    fillMode: Image.PreserveAspectFit
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Bem-vindo ao Genesi OS"
                    font.pixelSize: 32
                    font.bold: true
                    color: "#00ff9f"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "A primeira distribuição Linux otimizada para IA local"
                    font.pixelSize: 18
                    color: "#00ff9f"
                    opacity: 0.8
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    // Slide 2: AI Mode
    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0f0d"
            
            Column {
                anchors.centerIn: parent
                spacing: 30
                width: parent.width * 0.8
                
                Text {
                    text: "🤖 AI Mode"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#00ff9f"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Otimização automática quando você roda modelos de IA"
                    font.pixelSize: 20
                    color: "#00ff9f"
                    opacity: 0.9
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Column {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    
                    Text {
                        text: "✓ Detecta Ollama, llama.cpp, vLLM automaticamente"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Otimiza CPU governor para performance"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Gerenciamento inteligente de memória"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ 15-25% mais rápido em inferência"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                }
            }
        }
    }

    // Slide 3: Auto-Updates
    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0f0d"
            
            Column {
                anchors.centerIn: parent
                spacing: 30
                width: parent.width * 0.8
                
                Text {
                    text: "🔄 Sistema de Atualizações"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#00ff9f"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Mantenha seu sistema sempre atualizado"
                    font.pixelSize: 20
                    color: "#00ff9f"
                    opacity: 0.9
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Column {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    
                    Text {
                        text: "✓ Verificação automática de updates"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Notificações no desktop"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Widget na taskbar"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Integração com KDE Discover"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                }
            }
        }
    }

    // Slide 4: Developer Focused
    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0f0d"
            
            Column {
                anchors.centerIn: parent
                spacing: 30
                width: parent.width * 0.8
                
                Text {
                    text: "💻 Feito para Desenvolvedores"
                    font.pixelSize: 36
                    font.bold: true
                    color: "#00ff9f"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Tudo que você precisa, sem configuração"
                    font.pixelSize: 20
                    color: "#00ff9f"
                    opacity: 0.9
                    wrapMode: Text.WordWrap
                    horizontalAlignment: Text.AlignHCenter
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Column {
                    spacing: 15
                    anchors.horizontalCenter: parent.horizontalCenter
                    
                    Text {
                        text: "✓ Baseado em CachyOS (kernel otimizado)"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ KDE Plasma com tema customizado"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Ferramentas de desenvolvimento pré-instaladas"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                    
                    Text {
                        text: "✓ Zero configuração necessária"
                        font.pixelSize: 16
                        color: "#00ff9f"
                        opacity: 0.7
                    }
                }
            }
        }
    }

    // Slide 5: Installing
    Slide {
        Rectangle {
            anchors.fill: parent
            color: "#0a0f0d"
            
            Column {
                anchors.centerIn: parent
                spacing: 40
                
                Image {
                    source: "logo.png"
                    width: 150
                    height: 150
                    fillMode: Image.PreserveAspectFit
                    anchors.horizontalCenter: parent.horizontalCenter
                    
                    RotationAnimation on rotation {
                        loops: Animation.Infinite
                        from: 0
                        to: 360
                        duration: 3000
                    }
                }
                
                Text {
                    text: "Instalando Genesi OS..."
                    font.pixelSize: 28
                    font.bold: true
                    color: "#00ff9f"
                    anchors.horizontalCenter: parent.horizontalCenter
                }
                
                Text {
                    text: "Em alguns minutos você terá o melhor ambiente para IA local"
                    font.pixelSize: 16
                    color: "#00ff9f"
                    opacity: 0.7
                    anchors.horizontalCenter: parent.horizontalCenter
                }
            }
        }
    }

    function onActivate() {
        presentation.currentSlide = 0
    }

    function onLeave() {
    }
}
