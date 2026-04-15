#!/bin/bash
# =============================================================================
#  setup-splash.sh — Sustituir el logo de Raspberry Pi por imagen propia
#
#  Uso: sudo ./setup-splash.sh /ruta/a/tu/imagen.png
#
#  La imagen debe ser PNG a la resolución exacta de la pantalla.
#  Para convertir desde SVG:
#    inkscape imagen.svg --export-type=png --export-width=1920 --export-height=1080 -o splash.png
# =============================================================================

set -e

if [ -z "$1" ]; then
    echo "Uso: sudo $0 /ruta/a/tu/imagen.png"
    exit 1
fi

IMAGE="$1"

if [ ! -f "$IMAGE" ]; then
    echo "Error: no se encuentra el fichero '$IMAGE'"
    exit 1
fi

if [[ "$IMAGE" != *.png ]]; then
    echo "Error: la imagen debe ser PNG"
    exit 1
fi

# --- Sustituir imagen del logo ------------------------------------------------
echo "[1/2] Sustituyendo logo de Raspberry Pi..."
cp "$IMAGE" /usr/share/plymouth/themes/pix/splash.png
echo "  -> /usr/share/plymouth/themes/pix/splash.png"

# --- Regenerar initramfs ------------------------------------------------------
echo "[2/2] Aplicando cambios (puede tardar un momento)..."
update-initramfs -u

echo ""
echo "Listo. Reinicia para ver los cambios: sudo reboot"
