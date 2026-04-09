#!/usr/bin/env bash
# =============================================================================
#  install.sh — Armario de Llaves Inteligente
#  Prepara una Raspberry Pi (Raspberry Pi OS 64-bit, Bookworm) desde cero.
#
#  Uso:
#    chmod +x install.sh && ./install.sh
# =============================================================================
set -euo pipefail

# ── Colores ──────────────────────────────────────────────────────────────────
RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'
log()  { echo -e "${GREEN}[✓]${NC} $*"; }
info() { echo -e "${CYAN}[→]${NC} $*"; }
warn() { echo -e "${YELLOW}[!]${NC} $*"; }
die()  { echo -e "${RED}[✗]${NC} $*" >&2; exit 1; }

echo -e "${CYAN}"
echo "  ╔══════════════════════════════════════════════╗"
echo "  ║   Armario de Llaves — Script de instalación  ║"
echo "  ╚══════════════════════════════════════════════╝"
echo -e "${NC}"

# ── Variables ─────────────────────────────────────────────────────────────────
INSTALL_DIR="$HOME/TFG_electr-nica"
REPO_URL="https://github.com/bforteza/TFG_electr-nica"

DB_NAME="miBaseDeDatos"
DB_USER="KEYSISTEM"
DB_PASS="KeySistem"

LITESQL_VERSION="0.3.18"
LITESQL_TAR="litesql-src-${LITESQL_VERSION}.tar.gz"
LITESQL_URL="https://sourceforge.net/projects/litesql/files/litesql/${LITESQL_VERSION}/${LITESQL_TAR}/download"
LITESQL_BUILD_DIR="/tmp/litesql-build"

# ── 0. Comprobaciones previas ─────────────────────────────────────────────────
if ! command -v sudo &>/dev/null; then
    die "sudo no está disponible. Ejecuta el script como usuario normal con sudo configurado."
fi

# Detectar SO
if [ -f /etc/os-release ]; then
    . /etc/os-release
    info "Sistema detectado: ${PRETTY_NAME:-desconocido}"
    CODENAME="${VERSION_CODENAME:-}"
    if [ "$CODENAME" != "bookworm" ]; then
        warn "Este script está probado en Debian 12 (bookworm). Detectado: '${CODENAME}'. Continúa bajo tu responsabilidad."
        read -r -p "¿Continuar de todas formas? [s/N] " resp
        [[ "$resp" =~ ^[sS]$ ]] || exit 0
    fi
fi

# Detectar arquitectura y ruta de libmysqlclient
ARCH=$(dpkg --print-architecture 2>/dev/null || uname -m)
case "$ARCH" in
    arm64|aarch64) MYSQL_LIB="/usr/lib/aarch64-linux-gnu/libmysqlclient.so" ;;
    armhf)         MYSQL_LIB="/usr/lib/arm-linux-gnueabihf/libmysqlclient.so" ;;
    amd64|x86_64)  MYSQL_LIB="/usr/lib/x86_64-linux-gnu/libmysqlclient.so" ;;
    *)             MYSQL_LIB=$(find /usr/lib -name "libmysqlclient.so" 2>/dev/null | head -1) ;;
esac

# ── 1. Dependencias del sistema ───────────────────────────────────────────────
info "Actualizando lista de paquetes..."
sudo apt-get update -qq

info "Instalando dependencias del sistema..."
sudo apt-get install -y \
    pkg-config \
    libgtkmm-3.0-dev \
    libnfc-dev \
    default-libmysqlclient-dev \
    cmake \
    libsqlite3-dev \
    mariadb-server \
    git \
    wget \
    codeblocks \
    python3-gi \
    python3-gi-cairo \
    gir1.2-gtk-3.0 \
    libgtk-3-0 \
    python3-venv

log "Dependencias instaladas"

# Verificar libmysqlclient
[ -f "$MYSQL_LIB" ] || die "No se encontró libmysqlclient.so en '$MYSQL_LIB'. Revisa la instalación de libmysqlclient-dev."
log "libmysqlclient encontrada: $MYSQL_LIB"

# Verificar PyGObject (teclado virtual)
python3 -c "import gi; gi.require_version('Gtk','3.0'); from gi.repository import Gtk" \
    && log "PyGObject OK" \
    || die "PyGObject no funciona correctamente"

# ── 2. Clonar el repositorio ──────────────────────────────────────────────────
if [ -d "$INSTALL_DIR/.git" ]; then
    warn "Repositorio ya existe en $INSTALL_DIR — actualizando con git pull..."
    git -C "$INSTALL_DIR" pull
else
    info "Clonando repositorio en $INSTALL_DIR..."
    git clone "$REPO_URL" "$INSTALL_DIR"
    log "Repositorio clonado"
fi

# ── 3. Compilar LiteSQL desde fuente ─────────────────────────────────────────
# Saltar si ya está instalado
if [ -f /usr/local/lib/liblitesql.so ] || [ -f /usr/local/lib/liblitesql.a ]; then
    warn "LiteSQL ya parece estar instalado en /usr/local/lib — saltando compilación."
    warn "Si quieres reinstalar, borra /usr/local/lib/liblitesql* y vuelve a ejecutar."
else
    info "Preparando LiteSQL ${LITESQL_VERSION}..."
    mkdir -p "$LITESQL_BUILD_DIR"
    cd "$LITESQL_BUILD_DIR"

    if [ ! -f "$LITESQL_TAR" ]; then
        info "Descargando LiteSQL desde SourceForge (puede tardar)..."
        wget --show-progress -O "$LITESQL_TAR" "$LITESQL_URL" \
            || die "Error al descargar LiteSQL. Comprueba la conexión a internet."
    else
        warn "Archivo $LITESQL_TAR ya existe — reutilizando."
    fi

    info "Extrayendo..."
    tar xzf "$LITESQL_TAR"
    cd "litesql-src-${LITESQL_VERSION}"

    # ── Parche 0: corregir BACKEND_LIBRARIES en el CMakeLists.txt raíz ──
    # El root CMakeLists.txt añade 'litesql_mysql' a BACKEND_LIBRARIES, pero nuestro
    # parche de la librería crea 'litesql_backend_mysql'. Hay que sincronizar el nombre.
    info "Aplicando parche 0 — corrigiendo BACKEND_LIBRARIES en CMakeLists.txt raíz..."
    sed -i 's/APPEND BACKEND_LIBRARIES litesql_mysql/APPEND BACKEND_LIBRARIES litesql_backend_mysql/' CMakeLists.txt

    # ── Parche 1: CMakeLists.txt del backend MySQL ──
    info "Aplicando parche 1 — CMakeLists.txt..."
    python3 - <<'PYEOF'
import pathlib, sys

cmake_path = pathlib.Path("src/library/CMakeLists.txt")
text = cmake_path.read_text()

MARKER_START = "if (LITESQL_WITH_MYSQL)"
MARKER_END   = "endif (LITESQL_WITH_MYSQL)"

start = text.find(MARKER_START)
if start == -1:
    sys.exit("ERROR: No se encontró 'if (LITESQL_WITH_MYSQL)' en CMakeLists.txt")
end = text.find(MARKER_END, start)
if end == -1:
    sys.exit("ERROR: No se encontró el cierre 'endif (LITESQL_WITH_MYSQL)'")
end += len(MARKER_END)

new_block = """\
if (LITESQL_WITH_MYSQL)
  add_library(  ${LIBNAME}_backend_mysql SHARED
                        mysql/mysql.cpp
                        mysql/mysql_backend_plugin.cpp
  )
  target_link_libraries(${LIBNAME}_backend_mysql PRIVATE litesql PRIVATE litesql-util PRIVATE ${MYSQL_LIBRARIES})
  install(TARGETS ${LIBNAME}_backend_mysql  EXPORT ${LIBNAME}_backend_mysql
            RUNTIME DESTINATION lib
            LIBRARY DESTINATION lib
            ARCHIVE DESTINATION lib/static
            COMPONENT devel)
  install(EXPORT ${LIBNAME}_backend_mysql DESTINATION lib)
if (MSVC)
  install(FILES ${CMAKE_CURRENT_BINARY_DIR}/debug/${LIBNAME}_backend_mysqld.lib DESTINATION lib/static COMPONENT devel)
endif(MSVC)

endif (LITESQL_WITH_MYSQL)"""

cmake_path.write_text(text[:start] + new_block + text[end:])
print("CMakeLists.txt parcheado correctamente")
PYEOF

    # ── Parche 2: mysql_backend_plugin.cpp ──
    info "Aplicando parche 2 — mysql_backend_plugin.cpp..."
    cat > src/library/mysql/mysql_backend_plugin.cpp << 'EOF'
#include "litesql/backend.hpp"
#include "mysql.hpp"

using namespace litesql;

extern "C" Backend* createBackend(const std::string& parameter)
{
  Backend* pBackend = nullptr;
  try {
    pBackend = new MySQL(parameter);
  } catch(const DatabaseError& /*ex*/) {
    pBackend = nullptr;
  }
  return pBackend;
}

extern "C" void deleteBackend(Backend* backend)
{
  if (backend)
  {
    delete backend;
  }
}
EOF

    # ── Compilar ──
    info "Compilando LiteSQL (esto puede tardar varios minutos en la Raspberry Pi)..."
    sudo rm -rf build && mkdir build && cd build
    cmake .. \
        -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX=/usr/local \
        -DLITESQL_WITH_MYSQL=ON \
        -DMYSQL_INCLUDE_DIR=/usr/include/mysql \
        "-DMYSQL_LIBRARIES=${MYSQL_LIB}" \
        -DCMAKE_POLICY_DEFAULT_CMP0167=OLD \
        -Wno-dev \
        -DLITESQL_WITH_DOCS=OFF \
        -DLITESQL_WITH_TESTS=OFF \
        -DLITESQL_WITH_EXAMPLES=OFF

    make -j"$(nproc)"
    sudo make install
    sudo /sbin/ldconfig
    log "LiteSQL ${LITESQL_VERSION} instalado en /usr/local"
fi

# ── 4. Configurar MariaDB ─────────────────────────────────────────────────────
info "Configurando MariaDB..."
sudo systemctl start mariadb
sudo systemctl enable mariadb

sudo mariadb -e "
CREATE DATABASE IF NOT EXISTS \`${DB_NAME}\` CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS '${DB_USER}'@'localhost' IDENTIFIED BY '${DB_PASS}';
GRANT ALL PRIVILEGES ON \`${DB_NAME}\`.* TO '${DB_USER}'@'localhost';
FLUSH PRIVILEGES;
"
log "Base de datos '${DB_NAME}' y usuario '${DB_USER}' configurados"

# ── 5. Configurar el servidor web ─────────────────────────────────────────────
info "Configurando el servidor web (venv + pip)..."
cd "$INSTALL_DIR/webserver"

python3 -m venv venv
source venv/bin/activate
pip install --quiet --upgrade pip
pip install --quiet -r requirements.txt
deactivate

# Escribir config.py con las credenciales correctas
cat > "$INSTALL_DIR/webserver/config.py" << PYEOF
DB_CONFIG = {
    "host":     "localhost",
    "user":     "${DB_USER}",
    "password": "${DB_PASS}",
    "database": "${DB_NAME}",
    "charset":  "utf8mb4",
}
PYEOF
log "Servidor web configurado (venv en webserver/venv/)"

# ── 6. Symlink del teclado virtual ────────────────────────────────────────────
info "Creando symlink del teclado virtual para Code::Blocks..."
mkdir -p "$INSTALL_DIR/bin/Debug"
ln -sfn "../../keyboard" "$INSTALL_DIR/bin/Debug/keyboard"
log "Symlink creado: bin/Debug/keyboard → ../../keyboard"

# ── 7. Resumen ────────────────────────────────────────────────────────────────
echo ""
echo -e "${GREEN}╔══════════════════════════════════════════════════════════╗${NC}"
echo -e "${GREEN}║          Instalación completada correctamente            ║${NC}"
echo -e "${GREEN}╚══════════════════════════════════════════════════════════╝${NC}"
echo ""
echo -e "  Repositorio clonado en:  ${CYAN}${INSTALL_DIR}${NC}"
echo -e "  Base de datos:           ${CYAN}${DB_NAME}${NC}  (user: ${DB_USER})"
echo ""
echo -e "  ${YELLOW}Para compilar el proyecto:${NC}"
echo    "    Abrir Code::Blocks con: ${INSTALL_DIR}/Pruebas2.cbp"
echo    "    Build → Build"
echo ""
echo -e "  ${YELLOW}Para arrancar el servidor web:${NC}"
echo    "    cd ${INSTALL_DIR}/webserver"
echo    "    source venv/bin/activate"
echo    "    python app.py"
echo    "    → Panel:   http://localhost:5000/"
echo    "    → Swagger: http://localhost:5000/openapi/"
echo ""
