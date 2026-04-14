#!/bin/bash
# =============================================================================
#  kiosk-session.sh — Armario de Llaves Inteligente
#
#  Wrapper de arranque para la app GTK. La lógica de reintento es simple:
#
#    - Salida limpia (exit 0):  ESC → get_application()->quit() → vuelve al
#      escritorio LXDE para mantenimiento local. NO relanza la app.
#
#    - Crash (exit != 0):  error inesperado → espera 2 s y relanza.
#
#  __INSTALL_DIR__ se sustituye por install.sh al instalarlo en el sistema.
# =============================================================================

APP_DIR="__INSTALL_DIR__/bin/Debug"

while true; do
    # cd al directorio del binario: la app busca ./keyboard/keyboard.py relativo
    # al directorio de trabajo, por lo que hay que ejecutarla desde bin/Debug/
    cd "$APP_DIR" && ./Pruebas2
    [ $? -eq 0 ] && break   # Salida intencional (ESC) → salir al escritorio
    sleep 2                  # Crash → esperar y relanzar automáticamente
done
