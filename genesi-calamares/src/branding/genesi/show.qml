/* === Genesi OS Installer Slideshow ===
 *
 *   SPDX-FileCopyrightText: 2026 Genesi OS Team
 *   SPDX-License-Identifier: GPL-3.0-or-later
 */

import QtQuick 2.15;
import calamares.slideshow 1.0;

Presentation
{
    id: presentation

    function nextSlide() {
        presentation.goToNextSlide();
    }

    Timer {
        id: advanceTimer
        interval: 8000
        running: presentation.activatedInCalamares
        repeat: true
        onTriggered: nextSlide()
    }

    // Slide 1
    Slide {
        Image {
            anchors.fill: parent
            source: "slide1.png"
            fillMode: Image.PreserveAspectCrop
        }
    }

    // Slide 2
    Slide {
        Image {
            anchors.fill: parent
            source: "slide2.png"
            fillMode: Image.PreserveAspectCrop
        }
    }
}
