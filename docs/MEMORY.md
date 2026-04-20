# TFG Electrónica - Armario de Llaves (Raspberry Pi)

## Descripción del Proyecto
Sistema de gestión de llaves físicas para una empresa, corriendo en Raspberry Pi.
- Lenguaje: C++17, UI: gtkmm (GTK3), BD: LiteSQL (MariaDB), NFC: libnfc, Sonido: libcanberra-gtk3
- Convención de código: Google C++ Style Guide (snake_case para archivos y variables)
- IDE: Code::Blocks. UI en Glade (MainWindow.glade en ./ui/)

## Arquitectura de Pantallas (GTK Stack)
- `LoginStack` → pantalla inicial; login por NFC o texto. Emite señal `UserLogg` o `KeyLogg`
- `HomeStack` → panel admin con subvistas (AdminStacks inner stack):
  - `UsersViewStack` → lista de usuarios
  - `UserCreateStack` → crear/editar usuario
  - `KeyViewStack` → lista de llaves
  - `KeyCreateStack` → crear/editar llave
  - `HistoryViewStack` → historial de acciones (filtrable por llave o usuario)
- `SolenoidPanel` → panel de solenoides con tres modos: PICKUP, ADMIN, SELECT
- `Window` → ventana principal que contiene todos los stacks

## Modelos de Base de Datos (LiteSQL, MariaDB)
**Person**: id, type, name, password, uid (NFC UID), a1 (bool), a2 (bool), **active (bool, default true)**
**Key**: id, type, name, ubi (ubicación física), commentary, pos (int, hueco armario), active (bool), uid (NFC UID), **pub (bool, default false)**
**HistoryEvent**: etype, timestamp, keyid, keyname, personid, personname, pos

**Relaciones**:
- `Key_Person_Acces` (M:N): llaves a las que tiene acceso una persona → `person.keys()` / `key.owners()`
- `Key_Person_Keep` (1:N, Key1_ UNIQUE): quién tiene actualmente cada llave → `person.keepkeys()` / `key.keeper()`

**Columnas en MariaDB** (LiteSQL añade `_` a todos los nombres): `id_`, `name_`, `pub_`, `active_`, etc.

**Niveles de acceso** (calculado como `(int)a1 + 2*(int)a2`):
- 0: solo ver sus llaves asignadas + llaves públicas
- 1 (a1=true): ver+gestionar llaves (crear, editar, vincular usuarios)
- 2 (a2=true): administrador completo (+ gestión usuarios, ver todas las llaves)

## Campo `pub` — Llaves Públicas
Una llave marcada como `pub=true` es accesible por **todos los usuarios** sin necesidad de estar en la relación `Key_Person_Acces`.
- **KeyCreateStack**: `GtkCheckButton` con id `KeyPublicCheckButton` activa/desactiva el flag.
- **HomeStack::OnViewKeysButtonClicked**: usuarios con `access_ < 2` ven `litesql::union_(person.keys, select<Key>(pub==true))`.
- **HomeStack::OnKeyLinkUser / OnKeyUnlinkUser**: bloquean con `ShowWarning(Tr().key_view.err_key_is_public)` si `key->pub`.
- Al crear una llave pública, el creador no queda vinculado en `Key_Person_Acces` (sí como `keeper`).
- En el webserver, si una llave es pública, la sección de "Usuarios autorizados" se oculta con JS y no se persisten enlaces.

## Componentes Clave
- `NfcManager` (nfc_manager.h/cpp): hilo separado, detecta tags ISO14443A, guarda UID en `uid`, emite `Glib::Dispatcher dispatcher` para comunicar con GTK thread-safe. `StopPolling()` usa `detach()`.
- `db_schema.hpp` + `db_schema.cpp`: modelos LiteSQL (autogenerados por LiteSQL tool). **Regenerar con `litesql-gen -t c++ --output-dir=. GestorBaseDatos.xml` y copiar los .hpp/.cpp generados a include/ y src/ respectivamente.**
- `globals.h`: instancias globales `db` (DbSchema*) y `nfcman` (unique_ptr<NfcManager>), `PosToString(int pos)` y `PosFromString(string)` para conversión posición↔string (A1-D8)
- `models.h`: `ModelColumns` (usuarios), `KeyModelColumns` (llaves) y `HistoryModelColumns` (historial) para Gtk::TreeView
- `sound_manager.h/cpp`: clase estática SoundManager (libcanberra-gtk3), ver sección Sonido
- `history_logger.h/cpp`: funciones estáticas `history::LogPickup`, `LogReturn`, `LogKeyCreated(creator, key)`, `LogKeyDeactivated`, `LogAdminOpen` etc. Escribe en tabla History vía SQL directo. `creator` puede ser `nullptr` (p.ej. en reactivación de llaves).

## SolenoidPanel — Cuatro Modos
- **PICKUP**: abre el solenoide de la llave `key` pasada, muestra cuenta atrás, vuelve a HomeStack al terminar
- **RETURN**: usuario devuelve una llave (sin autenticación, por NFC de la llave). Emite `signal_logout` al terminar.
- **ADMIN**: grid completo de posiciones, toggle (click activa/desactiva), solo admin. `active_admin_slot_` guarda el slot activo (-1 si ninguno). Al clic: desactiva el actual (siempre), luego activa el nuevo si es diferente.
- **SELECT**: grid solo de posiciones libres (ocupadas insensibles). Al pulsar una libre emite `signal_position_selected(int pos)` y navega atrás. Se usa desde KeyCreateStack para elegir posición de llave nueva.

Señales públicas de SolenoidPanel:
- `signal_go_home` → volver a HomeStack (PICKUP o ADMIN)
- `signal_logout` → cerrar sesión (RETURN o timeout de PICKUP)
- `signal_position_selected(int)` → posición elegida en modo SELECT

## Flujo Selección de Posición (KeyCreateStack → SolenoidPanel SELECT)
```
KeyCreateStack::signal_position_select_requested
  → HomeStack::OnPositionSelectRequested()
    → signal_open_solenoid(nullptr, Mode::SELECT)
      → Window::OnOpenSolenoid() → SolenoidPanel::Setup(SELECT)
        → SolenoidPanel::signal_position_selected(int pos)
          → Window (handler) → HomeStack::OnPositionSelected(int pos) [PUBLIC]
            → KeyCreateStack::SetPosition(pos) + navega de vuelta a KeyCreateStack
```

## Timer de Inactividad (Window)
- Se activa cuando `main_stack_` muestra "HomeView" (usuario logueado).
- 30 segundos sin interacción → `home_stack_.Logout()`.
- Implementado con `gdk_event_handler_set` en el constructor de Window: intercepta `GDK_BUTTON_PRESS` y `GDK_TOUCH_BEGIN` a nivel GDK (antes de que cualquier widget reciba el evento) y llama `ResetInactivityTimer()`. Debe llamar `gtk_main_do_event(ev)` para reenviar el evento al pipeline normal de GTK.
- `StopInactivityTimer()` se llama al entrar en vistas que no son HomeView.
- `on_key_press_event` sigue activo para manejar la tecla ESC.
- `db->verbose = false` en main.cpp para silenciar LiteSQL.

## Selección Múltiple Táctil (KeyViewStack / UsersViewStack)
Sin Ctrl disponible en pantalla táctil, se usa un `button-press-event` handler que simula Ctrl permanente:
- `select(vector, bool multiple=true)`:
  - `multiple=true`: `set_mode(SELECTION_MULTIPLE)` + conectar handler en `toggle_conn_` que por cada tap togglea la selección de la fila. `return true` consume el evento.
  - `multiple=false`: `set_mode(SELECTION_SINGLE)`, sin handler extra. Se usa en flujo de recuperación.
- En `view()`: `toggle_conn_.disconnect()` + `set_mode(SELECTION_SINGLE)`.
- `GetSelectedKeys()` / `GetSelectedPersons()` usan `get_selection()->get_selected_rows()`.

## Visibilidad de Columnas por Nivel de Acceso (UsersViewStack)
- **Modo `select()`**: siempre oculta `password_column_` y `uid_column_` (cualquier nivel de acceso).
- **Modo `view()`**: muestra `password_column_` y `uid_column_` solo si `access >= 2` (admin).

## Sistema de Sonido (SoundManager)
Clase estática en `include/sound_manager.h` + `src/sound_manager.cpp`.
- `SoundManager::Init()`: obtiene contexto canberra, pre-cachea todos los sonidos con `CA_PROP_CANBERRA_CACHE_CONTROL "permanent"` para eliminar latencia de primera reproducción.
- `SoundManager::Play(SoundEvent)`: reproduce con IDs únicos (`next_id_++`) para evitar cancelación de sonido anterior.
- `SoundManager::ConnectToAllButtons(Gtk::Container*)`: recorre recursivamente el árbol de widgets conectando `kClick` a cada `Gtk::Button`.

## Dialog "Llave ya en uso" (HomeStack::OnKeyKept)
```cpp
if (key->keeper().get().count() > 0) {
    auto* dlg = new Gtk::MessageDialog(...);
    auto closed = std::make_shared<bool>(false);
    dlg->signal_response().connect([dlg, closed](int) {
        *closed = true; dlg->hide(); delete dlg;
    });
    Glib::signal_timeout().connect_once([dlg, closed]() {
        if (!*closed) dlg->response(Gtk::RESPONSE_OK);
    }, 2000);
    dlg->show();
    return;
}
```
`shared_ptr<bool> closed`: evita doble-delete si el usuario pulsa OK antes del timeout.
`HomeStack::ShowWarning(const std::string& msg)` es el método genérico para este patrón de diálogo auto-cerrante (2 segundos).

## Webserver (RF-05 Gestión Remota) — Flask + PyMySQL
Ubicación: `./webserver/`
Stack: Python/Flask, PyMySQL (acceso SQL directo a MariaDB), Bootstrap 5 + Bootstrap Icons, Jinja2.
Ejecutar: `python app.py` (puerto 5000). Configuración de BD en `config.py`.

### Rutas implementadas
| Método | Ruta | Descripción |
|--------|------|-------------|
| GET | `/` | Redirige a `/users` |
| GET | `/users` | Lista todos los usuarios |
| GET/POST | `/users/new` | Crear usuario |
| GET/POST | `/users/<id>/edit` | Editar usuario |
| POST | `/users/<id>/delete` | Eliminar usuario |
| GET | `/keys` | Lista todas las llaves (con keeper y badge pública/privada) |
| GET/POST | `/keys/new` | Crear llave (incluye campo pub, selector de posición, usuarios autorizados) |
| GET/POST | `/keys/<id>/edit` | Editar llave |
| POST | `/keys/<id>/delete` | Eliminar llave (borra relaciones Acces y Keep) |
| GET | `/history` | Historial filtrable por llave, persona o tipo de evento |

### Convenciones del webserver
- SQL directo con PyMySQL (no ORM). Las columnas de MariaDB usan el sufijo `_` de LiteSQL: `name_`, `pub_`, `active_`, etc.
- `pos_to_string(pos)` y `pos_from_string(s)` replican la lógica de `globals.h` en Python.
- Las llaves públicas (`pub_=1`) no tienen usuarios autorizados: el form oculta la sección con JS (`togglePublic()`) y el backend ignora los `authorized` del POST si `pub=1`.
- Templates en `templates/` organizados por entidad: `users/`, `keys/`, `history/`. Base en `base.html` con sidebar Bootstrap.

### Acceso a la BD desde el webserver
LiteSQL crea tablas con el sufijo `_` y campo de tipo `type_`:
- `Person_` (columnas: `id_`, `type_`, `name_`, `password_`, `uid_`, `a1_`, `a2_`)
- `Key_` (columnas: `id_`, `type_`, `name_`, `ubi_`, `commentary_`, `pos_`, `active_`, `uid_`, `pub_`)
- `Key_Person_Acces` (columnas: `Key1_`, `Person2_`)
- `Key_Person_Keep` (columnas: `Key1_`, `Person2_`)
- `HistoryEvent_` (columnas: `id_`, `etype_`, `timestamp_`, `keyid_`, `keyname_`, `personid_`, `personname_`, `pos_`)

Al insertar en `Person_` o `Key_` siempre incluir `type_='Person'` / `type_='Key'` (requerido por LiteSQL).

## Convenciones de Nomenclatura
- Métodos: PascalCase (`PersonLogged`, `OnBackButtonClicked`, `RefreshLabels`)
- Miembros privados: snake_case con trailing underscore (`window_`, `logged_person_`)
- Señales públicas: snake_case sin underscore (`signal_open_solenoid`, `signal_position_selected`)
- Overrides GTK: mantener nombre original (`on_activate`, `on_startup`)

## Feedback del Usuario (importante para futuras sesiones)
- **Compilar después de cada adición**: el usuario lo ha pedido explícitamente para no programar a ciegas y detectar errores inmediatamente.
- **Discutir antes de escribir**: para funcionalidades nuevas, primero discutir diseño e implementación, luego escribir.
- Los métodos que Window llama directamente deben ser `public` en el header.
- **No revertir con git** sin confirmar: puede haber cambios funcionales mezclados con los que se quieren deshacer.

## Hardware del Sistema
- **Raspberry Pi 4** (2GB RAM) como unidad central
- **PN532** lector NFC por I2C
- **XL9535** expansor de puertos I2C + módulo de 16 relés con optoacopladores (compatible PCA9535, mismos registros)
- **Armario**: 4 filas × 8 columnas = 32 posiciones
- **Circuito solenoides**: matriz con diodos — controla 32 solenoides con 4+8=12 relés + 1 relé cerradura de acceso
  - Solenoide desactivado = pasador cae = llave bloqueada (lógica inversa)
  - Para abrir: activar fila + columna simultáneamente; para cerrar: desactivar fila primero → delay → desactivar columna (diodo flyback)
  - Restricción hardware: solo un solenoide activo simultáneamente (`active_pos_` en I2cController)
- **Pantalla táctil** conectada por USB (sin teclado físico para usuario normal)
- **Teclado numérico** (usuario normal): passwords deben ser numéricas

## Control Hardware I2C (I2cController)
Archivos: `include/i2c_controller.h`, `src/i2c_controller.cpp`, `include/hardware_config.h`

- Accede al XL9535 vía Linux i2c-dev (`/dev/i2c-1`, ioctl + write). Sin dependencias extra.
- **Secuencia segura de Init()**: primero escribe 0x00 en registros output (0x02/0x03), luego 0x00 en config (0x06/0x07) → evita activación involuntaria de relés al arrancar.
- **WriteState(uint16_t)**: escribe los 16 bits en una sola transacción I2C: `{0x02, port0, port1}` (auto-incremento de registro).
- **Activate(Position)**: calcula row_pin=kRowPins[(pos-1)/8], col_pin=kColPins[(pos-1)%8] → activa ambos a la vez. Ignorado si ya hay posición activa.
- **Deactivate()**: fila off → sleep(kRelayOffDelayMs) → columna off → active_pos_=N0.
- **OpenDoor() / CloseDoor()**: relé independiente (kDoorPin). Se llama desde SolenoidPanel al entrar en PICKUP/RETURN/ADMIN, con cierre automático a los 2s via `Glib::signal_timeout().connect_once`.
- **RunRelayTest(callback)**: cicla los 16 pines uno a uno para verificar GPIOs.
- Configuración centralizada en `hardware_config.h`: kI2cBus, kI2cAddress, kRelayOffDelayMs, kRowPins, kColPins, kDoorPin.
- Global: `inline std::unique_ptr<I2cController> hw_ctrl` declarado en `i2c_controller.h`. Inicializado en `Application::on_startup()`.

## Requisitos Funcionales del TFG (estado)
- RF-01: Identificación NFC/password ✓ implementado
- RF-02: Verificación de permisos ✓ implementado
- RF-03: Control hardware (solenoides) ✓ implementado — I2cController vía Linux i2c-dev, XL9535, SolenoidPanel integrado
- RF-04: Historial de acciones ✓ implementado (history_logger + HistoryViewStack)
- RF-05: Interfaz de gestión remota ✓ implementado (webserver Flask en ./webserver/)
- RF-06: Múltiples idiomas ✓ implementado — Catalán, Español, Inglés completos en `translations.h`

## Borrado Lógico y Recuperación (soft-delete)
Tanto llaves como usuarios usan `active` para "borrado lógico" (nunca se eliminan de la BD).

### Usuarios
- `Person.active` (bool, default true). `OnViewUsersButtonClicked` filtra `kdb::Person::Active == true`.
- **Desactivar**: `UsersViewStack` emite `user_delete` → `HomeStack::OnUserDelete`: `person->active = false; update(); reload`.
- Botón con diálogo de confirmación (`Gtk::MessageDialog`) antes de emitir la señal.

### Llaves
- `Key.active` ya existía. Desactivación ya implementada vía `OnKeyDelete`.

### Flujo de Recuperación (reactivar entidad inactiva)
Botón "Recuperar" en `KeyViewStack` / `UsersViewStack` emite `key_recover` / `user_recover`.
`HomeStack` gestiona el flujo en dos pasos:

```
1. OnKeyRecoverRequested() / OnUserRecoverRequested()
   → consulta inactivos (Active == false)
   → llama select(inactive, false)  ← selección única
   → guarda back_widget_, conecta señal key_selected/user_selected
   → navega a ViewKeyStack / ViewUsersStack

2. Lambda en key_selected / user_selected:
   → ResetSelectionState()
   → key_create_stack_.RecoverKey(key) / user_create_stack_.RecoverUser(person)
   → navega a KeyCreateStack / UserCreateStack
```

`RecoverKey(key)` / `RecoverUser(person)`: llama a `KeyEdit`/`UserEdit` (hace Reset, carga datos) y luego activa `recover_mode_ = true`.
Al confirmar en `OnGenerateButtonClicked`: si `recover_mode_`, pone `active = true` antes de `update()`. Para llaves también registra `history::LogKeyCreated(nullptr, key)`.

### Corrección Back desde flujo de recuperación
`OnBackButtonClicked` detecta si el visible child no cambia tras navegar (caso recuperación: `back_widget_` apunta a la vista actual). En ese caso recarga la vista en modo normal (`OnViewKeysButtonClicked` / `OnViewUsersButtonClicked`).

## Feedback Visual del Botón Confirmar (KeyCreateStack / UserCreateStack)
El botón `generate_button_` cambia de color según el resultado de la última acción:
- **Rojo** (`btn-error`): validación fallida.
- **Verde** (`btn-success`): guardado con éxito.
- **Neutro**: al entrar a la pantalla (Reset() elimina ambas clases).

Las clases CSS están definidas en `ui/style.css`:
```css
button.btn-success { background-image: none; background-color: #4CAF50; color: white; }
button.btn-error   { background-image: none; background-color: #F44336; color: white; }
```

## Validación de Posición Ocupada (KeyCreateStack)
En `OnGenerateButtonClicked`, además de comprobar `position_ == 0`, se verifica que no haya otra llave activa en esa posición (en **todos los modos**: creación, edición y recuperación):
```cpp
else if (litesql::select<kdb::Key>(*db, kdb::Key::Pos == position_
                                        && kdb::Key::Active == true
                                        && kdb::Key::Id != exclude_id).count())
```
`exclude_id = edited_key_ ? (int)edited_key_->id : 0`. En creación, `exclude_id=0` (ninguna llave excluida).

## Layout de Botones UsersViewStack (dos filas)
Igual que `KeyViewStack`: `GtkBox` vertical con dos `GtkBox` horizontales (`homogeneous=true`, `spacing=2`):
- Fila 1: AddKeyToUserButton, RemoveKeyToUserButton, ViewKeysOfUserButton, SelectUserButton
- Fila 2: UserEditButton, DeleteUserButton, HistoryPersonButton, RecoverUserButton

## Sistema de Log Persistente (AppLogger)

Archivos: `include/app_logger.h`, `src/app_logger.cpp`, `webserver/logger.py`
Log en: `~/TFG_electr-nica/logs/armario.log` (relativo al ejecutable: `../../logs/armario.log`)

### C++ — AppLogger (clase estática, thread-safe)
- `AppLogger::Init()` — llamar al inicio de `Application::on_startup()`. Crea el directorio `logs/` si no existe, abre el archivo en modo append, registra `APPLICATION START`. Si la última línea no contiene `APPLICATION STOP`, registra aviso de crash previo.
- `AppLogger::Shutdown(ShutdownReason)` — registra `APPLICATION STOP: <motivo>` y cierra el stream. Razones: `kEsc` (ESC en `window.cpp`), `kShutdown` (`login_stack.cpp:OnShutdownClicked`), `kReboot` (`login_stack.cpp:OnRebootClicked`).
- `AppLogger::Info(source, msg)` / `AppLogger::Error(source, msg)` — thread-safe via mutex. Fuentes: `"APP"`, `"DB"`, `"I2C"`, `"NFC"`.
- **Rotación**: cuando `armario.log` supera 5 MB, se renombra a `armario.log.bak` y se abre uno nuevo. Solo hay un backup.

### Python — logger.py (webserver)
- `logger.setup()` — llamar antes de `app.run()`. Configura `WatchedFileHandler` (detecta rotaciones del C++) sobre el mismo `armario.log`. Comprueba estado de red al arrancar, lanza hilo daemon `net-monitor`.
- `logger.info(source, msg)` / `logger.error(source, msg)` — escribe al mismo archivo. Fuente: `"WEB"`.
- **Hilo net-monitor**: cada 30 s lee `/sys/class/net/<iface>/operstate`. Solo loguea en transiciones UP→DOWN o DOWN→UP.

### Formato de línea
```
YYYY-MM-DD HH:MM:SS [LEVEL] [SOURCE ] mensaje
```
`LEVEL`: `INFO ` / `ERROR` / `WARN ` (5 chars). `SOURCE`: 7 chars con padding. Ver `docs/log_reference.md` para catálogo completo de mensajes.

## Sistema de Copia de Seguridad

### Estrategia
- **Hora**: 12:00 (mediodía) — la Pi está encendida durante la jornada laboral. No se programa de noche porque el sistema puede estar apagado.
- **Destinos**: dos destinos independientes ejecutados en orden:
  1. **Local** — `~/TFG_electr-nica/backups/` (siempre se intenta)
  2. **Servidor** — carpeta SMB/SSH de la empresa (solo si la red está disponible)
- **Contenido**: volcado comprimido de MariaDB (`mysqldump` → `.sql.gz`) + copia del log `armario.log`. Dump completo de todas las tablas (incluyendo `HistoryEvent_`) — el tamaño total es ~150 KB comprimido, no justifica hacerlo incremental.
- **Condición**: la Pi debe estar encendida a las 12:00. Si está apagada, ese día no se realiza la copia.

### Gestión de espacio (rotación automática)
Misma lógica aplicada a ambos destinos independientemente. Espacio reservado: **200 MB en cada destino**.

1. Antes de copiar, comprueba si el nuevo fichero cabe en el espacio reservado (`BACKUP_MAX_MB=200` en el script).
2. Si no cabe, borra la copia **más antigua** y repite la comprobación.
3. Si no quedan copias antiguas y aún no hay espacio → `[WARN] LOCAL BACKUP space full` / `[WARN] REMOTE BACKUP space full` y **realiza la copia igualmente** (se sobrepasa el límite).
4. Tras cada backup loguea el espacio libre restante.

Con backups de ~150 KB/día, 200 MB dan para ~1.333 copias (~3,5 años) en cada destino.

### Implementación pendiente
- Script bash `scripts/backup.sh` que ejecuta `mysqldump`, comprime, aplica la rotación en ambos destinos y escribe en el log con la fuente `[BAK]` (ver `docs/log_reference.md`).
- Entrada en `crontab` del usuario pi: `0 12 * * * /home/pi/TFG_electr-nica/scripts/backup.sh`.
- Añadir `~/TFG_electr-nica/backups/` a `.gitignore`.
- Añadir creación del cron y del directorio `backups/` en `install.sh` una vez confirmado el método de conexión con el informático.

### Preguntas pendientes al informático
1. ¿Tiene el servidor Windows una carpeta compartida (SMB) accesible desde la red local?
2. ¿Tiene el servidor OpenSSH instalado (disponible en Windows Server 2019+)?
3. ¿Qué espacio puede asignar para las copias?

### Opciones de conexión Pi → Windows Server
| Opción | Requisito en el servidor | Comando en la Pi |
|--------|--------------------------|------------------|
| SMB/CIFS | Carpeta compartida con usuario/contraseña | `mount -t cifs //server/share /mnt/backup` |
| SSH/rsync | OpenSSH + clave pública de la Pi autorizada | `rsync -az archivo user@server:/ruta/` |

La opción SSH es más segura (sin contraseña en texto plano). La SMB es más sencilla si el informático ya tiene carpetas compartidas configuradas.

## Partes Pendientes de Implementar
1. **Devolución de llaves por NFC**: modo RETURN definido en SolenoidPanel y `OnKeyLogged` en Window, pero flujo no probado (sin hardware NFC disponible actualmente).
2. **Ajuste pin mapping**: `kRowPins` y `kColPins` en `hardware_config.h` deben verificarse contra el cableado real de la PCB antes del despliegue en Raspberry Pi.

## Posibles Bugs/Issues Conocidos
*(Vacío — reportar aquí los que se detecten)*
