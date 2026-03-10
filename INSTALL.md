# Guía de instalación — Armario de Llaves

Válida para **Debian 12 (Bookworm)** en x86_64 (PC de desarrollo) y aarch64 (Raspberry Pi).

---

## 1. Dependencias del sistema

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
CREATE USER IF NOT EXISTS 'usuario'@'localhost' IDENTIFIED BY 'CAMBIAR';
GRANT ALL PRIVILEGES ON miBaseDeDatos.* TO 'usuario'@'localhost';
FLUSH PRIVILEGES;
"
```

> **TODO**: La contraseña está en `main.cpp`. Moverla a un fichero de configuración externo antes de desplegar en producción.

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

---

## Notas

- Si no hay lector NFC conectado, la app arranca igualmente y el login NFC queda desactivado.
- La base de datos se crea automáticamente al primer arranque (`db->needsUpgrade()` / `db->upgrade()`).
- El archivo `kdb.db` (SQLite antiguo) está obsoleto; la app usa MariaDB.
