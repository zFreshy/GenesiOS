#!/bin/bash
# Add AI Mode widget to Plasma panel automatically
# Runs once per user on first login

MARKER_FILE="$HOME/.config/genesi-aimode-widget-added"

# Check if already added
if [ -f "$MARKER_FILE" ]; then
    exit 0
fi

# Wait for Plasma to be ready
sleep 5

# Add widget to panel using qdbus
qdbus org.kde.plasmashell /PlasmaShell org.kde.PlasmaShell.evaluateScript '
var panel = panelById(panelIds[0]);
if (panel) {
    panel.addWidget("org.genesi.aimode");
}
' 2>/dev/null

# Create marker file
touch "$MARKER_FILE"
