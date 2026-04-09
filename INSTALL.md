# Guía de instalación — Armario de Llaves

Válida para **Debian 12 (Bookworm)** en x86_64 (PC de desarrollo) y aarch64 (Raspberry Pi).

---

## 1. Dependencias del sistema

### Aplicación principal (C++ / GTK)

```bash
sudo apt-get update
sudo apt-get install -y \
    pkg-config \
    libgtkmm-3.0-dev \
    libnfc-dev \
    default-libmysqlclient-dev \
    cmake \
    libsqlite3-dev \
    mariadb-server \
    git
```

### Teclado virtual (`keyboard/keyboard.py`)

```bash
sudo apt-get install -y \
    python3-gi \
    python3-gi-cairo \
    gir1.2-gtk-3.0 \
    libgtk-3-0
```

Verificar la instalación:
```bash
python3 -c "import gi; gi.require_version('Gtk','3.0'); from gi.repository import Gtk; print('OK')"
```

### Servidor web (`webserver/`)

```bash
sudo apt-get install -y python3-venv
```

---

## 2. Compilar e instalar LiteSQL desde fuente

El paquete `.deb` oficial de LiteSQL **no es compatible** con el ABI de GCC moderno. Hay que compilarlo desde fuente con parches.

### 2.1 Descargar el fuente

Descargar `litesql-src-0.3.18.tar.gz` desde:
```
https://sourceforge.net/projects/litesql/files/litesql/0.3.18/
```

Extraer:
```bash
tar xzf litesql-src-0.3.18.tar.gz
cd litesql-src-0.3.18
```

### 2.2 Aplicar parches

El backend MySQL de LiteSQL 0.3.18 tiene dos bugs: nombre incorrecto de la librería y falta el símbolo de entrada del plugin. Hay que corregirlos manualmente.

**Parche 1 — `src/library/CMakeLists.txt`**

Buscar el bloque `if (LITESQL_WITH_MYSQL)` (línea ~129) y sustituirlo por:

```cmake
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

endif (LITESQL_WITH_MYSQL)
```

**Parche 2 — crear `src/library/mysql/mysql_backend_plugin.cpp`** (archivo nuevo):

```cpp
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
```

### 2.3 Compilar e instalar

```bash
mkdir build && cd build
cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr/local \
    -DLITESQL_WITH_MYSQL=ON \
    -DMYSQL_INCLUDE_DIR=/usr/include/mysql \
    -DMYSQL_LIBRARIES=/usr/lib/x86_64-linux-gnu/libmysqlclient.so

make -j$(nproc)
sudo make install
sudo /sbin/ldconfig
```

> **En Raspberry Pi (aarch64)** cambiar la ruta de la librería:
> ```
> -DMYSQL_LIBRARIES=/usr/lib/aarch64-linux-gnu/libmysqlclient.so
> ```

---

## 3. Configurar MariaDB

```bash
sudo systemctl start mariadb
sudo systemctl enable mariadb

sudo mariadb -e "
CREATE DATABASE IF NOT EXISTS miBaseDeDatos CHARACTER SET utf8mb4 COLLATE utf8mb4_unicode_ci;
CREATE USER IF NOT EXISTS 'KEYSISTEM'@'localhost' IDENTIFIED BY 'KeySistem';
GRANT ALL PRIVILEGES ON miBaseDeDatos.* TO 'KEYSISTEM'@'localhost';
FLUSH PRIVILEGES;
"
```

> Las credenciales están en `src/application.cpp`. Cambiarlas antes de desplegar en producción.

---

## 4. Compilar el proyecto

### En PC de desarrollo (Debian x86_64)

Usar el `Makefile` incluido (no está en git, es local):
```bash
make
./bin/Debug/Pruebas2
```

### En Raspberry Pi (con Code::Blocks)

Abrir `Pruebas2.cbp` con Code::Blocks y compilar desde ahí (Build → Build).

Verificar que las rutas del `.cbp` apunten a las librerías instaladas:
- `/usr/local/lib/liblitesql.a` (o `.so`)
- `/usr/local/lib/liblitesql-util.a`
- `/usr/lib/aarch64-linux-gnu/libmysqlclient.so`
- `/lib/libnfc.so` o `/usr/lib/aarch64-linux-gnu/libnfc.so`

### Despliegue del teclado virtual junto al binario

La aplicación lanza el teclado virtual como subproceso buscando `./keyboard/keyboard.py`
**relativo al directorio desde donde se ejecuta el binario**.

**En PC (Makefile):** el symlink se crea automáticamente con cada `make`:
```
bin/Debug/keyboard → ../../keyboard   (creado por el Makefile)
```
No hay que hacer nada extra.

**En Raspberry Pi (Code::Blocks):** Code::Blocks no ejecuta pasos post-build,
así que hay que hacer el enlace (o la copia) una sola vez a mano:

```bash
# Opción A — symlink (recomendado en desarrollo: los cambios en keyboard/ se reflejan al instante)
ln -sfn ../../keyboard bin/Debug/keyboard

# Opción B — copia directa (más simple para despliegue final)
cp -r keyboard bin/Debug/keyboard
```

> Si se usa la copia y se modifica `keyboard.py` o `keyboard.css`,
> hay que repetir la copia manualmente.

---

## 5. Configurar el servidor web

El servidor web es un proceso Python independiente que accede a la misma base de datos MariaDB y expone un panel HTML y una API REST en el puerto 5000.

### 5.1 Crear el entorno virtual e instalar dependencias

```bash
cd webserver
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

`requirements.txt` instala:
- `flask-openapi3[swagger]>=4.0` — framework web + Swagger UI en `/openapi/`
- `pymysql>=1.1` — conector MariaDB

### 5.2 Configurar credenciales

Editar `webserver/config.py` y ajustar los datos de conexión a MariaDB:

```python
DB_HOST = "localhost"
DB_USER = "usuario"
DB_PASSWORD = "CAMBIAR"
DB_NAME = "miBaseDeDatos"
```

### 5.3 Arrancar el servidor

```bash
cd webserver
source venv/bin/activate
python app.py        # escucha en 0.0.0.0:5000
```

Interfaces disponibles una vez arrancado:
- Panel HTML: `http://<IP>:5000/`
- API REST JSON: `http://<IP>:5000/api/`
- Swagger UI: `http://<IP>:5000/openapi/`

Para parar: `Ctrl+C` o `kill <PID>`.

---

## Notas

- Si no hay lector NFC conectado, la app arranca igualmente y el login NFC queda desactivado.
- La base de datos se crea automáticamente al primer arranque (`db->needsUpgrade()` / `db->upgrade()`).
- El archivo `kdb.db` (SQLite antiguo) está obsoleto; la app usa MariaDB.
