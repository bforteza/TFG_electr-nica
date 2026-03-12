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
- `SolenoidPanel` → panel de solenoides con tres modos: PICKUP, ADMIN, SELECT
- `Window` → ventana principal que contiene todos los stacks

## Modelos de Base de Datos (LiteSQL, MariaDB)
**Person**: id, type, name, password, uid (NFC UID), a1 (bool), a2 (bool)
**Key**: id, type, name, ubi (ubicación física), commentary, pos (int, hueco armario), active (bool), uid (NFC UID)
**Relaciones**:
- `Key_Person_Acces` (M:N): llaves a las que tiene acceso una persona → `person.keys()` / `key.owners()`
- `Key_Person_Keep` (1:N, Key1_ UNIQUE): quién tiene actualmente cada llave → `person.keepkeys()` / `key.keeper()`

**Niveles de acceso** (calculado como `(int)a1 + 2*(int)a2`):
- 0: solo ver sus llaves asignadas
- 1 (a1=true): ver+gestionar llaves (crear, editar, vincular usuarios)
- 2 (a2=true): administrador completo (+ gestión usuarios, crear usuarios)

## Componentes Clave
- `NfcManager` (nfc_manager.h/cpp): hilo separado, detecta tags ISO14443A, guarda UID en `uid`, emite `Glib::Dispatcher dispatcher` para comunicar con GTK thread-safe. `StopPolling()` usa `detach()`.
- `db_schema.hpp` + `db_schema.cpp`: modelos LiteSQL (autogenerados por LiteSQL tool)
- `globals.h`: instancias globales `db` (DbSchema*) y `nfcman` (unique_ptr<NfcManager>), `PosToString(int pos)` y `PosFromString(string)` para conversión posición↔string (A1-D8)
- `models.h`: `ModelColumns` (usuarios) y `KeyModelColumns` (llaves) para Gtk::TreeView
- `sound_manager.h/cpp`: clase estática SoundManager (libcanberra-gtk3), ver sección Sonido

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

## Sistema de Sonido (SoundManager)
Clase estática en `include/sound_manager.h` + `src/sound_manager.cpp`.
- `SoundManager::Init()`: obtiene contexto canberra, pre-cachea todos los sonidos con `CA_PROP_CANBERRA_CACHE_CONTROL "permanent"` para eliminar latencia de primera reproducción.
- `SoundManager::Play(SoundEvent)`: reproduce con IDs únicos (`next_id_++`) para evitar cancelación de sonido anterior.
- `SoundManager::ConnectToAllButtons(Gtk::Container*)`: recorre recursivamente el árbol de widgets conectando `kClick` a cada `Gtk::Button`.

Mapeo de eventos:
- `kClick` → `"message"`
- `kLoginOk` → `"complete"`
- `kLoginError` → `"dialog-error"`
- `kKeyReturn` → `"complete"`

Integración: `Window` llama `SoundManager::Init()` y `ConnectToAllButtons(this)` al final del constructor. `LoginStack` llama `Play()` en los callbacks de login. Los botones dinámicos deben llamar `SoundManager::Play(kClick)` al crearse.

Dependencia Makefile: `pkg-config gtkmm-3.0 libcanberra-gtk3` en CXXFLAGS y LDFLAGS.

## Dialog "Llave ya en uso" (HomeStack::OnKeyKept)
```cpp
if (key->keeper().get().count() > 0) {
    auto* dlg = new Gtk::MessageDialog(
        *window_, Tr().key_view.err_key_in_use,
        false, Gtk::MESSAGE_WARNING, Gtk::BUTTONS_OK, true);
    auto closed = std::make_shared<bool>(false);
    dlg->signal_response().connect([dlg, closed](int) {
        *closed = true; dlg->hide(); delete dlg;
    });
    Glib::signal_timeout().connect_once([dlg, closed]() {
        if (!*closed) dlg->response(Gtk::RESPONSE_OK);
    }, 1000);
    dlg->show();
    return;
}
```
`shared_ptr<bool> closed`: evita doble-delete si el usuario pulsa OK antes del timeout.
El timeout llama a `dlg->response()` en lugar de `delete` para que todo pase por `signal_response`.

## Modo Kiosco
Window lee la variable de entorno `KIOSK` para activar `fullscreen()`. Si no está definida, ventana normal (útil durante desarrollo). ESC cierra la aplicación.
- Producción: `KIOSK=1 ./bin/Debug/Pruebas2`
- Desarrollo: ejecutar sin la variable.

## Convenciones de Nomenclatura
- Métodos: PascalCase (`PersonLogged`, `OnBackButtonClicked`, `RefreshLabels`)
- Miembros privados: snake_case con trailing underscore (`window_`, `logged_person_`)
- Señales públicas: snake_case sin underscore (`signal_open_solenoid`, `signal_position_selected`)
- Overrides GTK: mantener nombre original (`on_activate`, `on_startup`)

## Feedback del Usuario (importante para futuras sesiones)
- **Compilar después de cada adición**: el usuario lo ha pedido explícitamente para no programar a ciegas y detectar errores inmediatamente.
- **Discutir antes de escribir**: para funcionalidades nuevas, primero discutir diseño e implementación, luego escribir.
- Los métodos que Window llama directamente deben ser `public` en el header.

## Hardware del Sistema
- **Raspberry Pi 4** (2GB RAM) como unidad central
- **PN532** lector NFC por I2C
- **XL9535** expansor de puertos I2C + módulo de 16 relés con optoacopladores
- **Armario**: 4 filas × 8 columnas = 32 posiciones
- **Circuito solenoides**: matriz con diodos — controla 32 solenoides con 4+8=12 relés
  - Solenoide desactivado = pasador cae = llave bloqueada (lógica inversa)
  - Para abrir: activar relé de fila LUEGO relé de columna; para cerrar: desactivar fila primero
- **Pantalla táctil** conectada por USB
- **Teclado numérico** (usuario normal): passwords deben ser numéricas

## Requisitos Funcionales del TFG (estado)
- RF-01: Identificación NFC/password ✓ implementado
- RF-02: Verificación de permisos ✓ implementado
- RF-03: Control hardware (solenoides) — SolenoidPanel implementado en UI, falta I2C real
- RF-04: Historial de acciones ✗ pendiente
- RF-05: Interfaz de gestión remota ✗ pendiente
- RF-06: Múltiples idiomas ✓ implementado — Catalán, Español, Inglés completos en `translations.h`. Variable global `current_language`, función `Tr()` para acceso. Cambio en caliente con `RefreshLabels()`.

## Partes Pendientes de Implementar
1. **Historial (RF-04)**: tabla `History` via SQL directo, `HistoryViewStack` UI, conectar `history_button_` en HomeStack. Registrar acciones en LoginStack, SolenoidPanel, HomeStack (CRUD).
2. **Devolución de llaves por NFC**: modo RETURN definido en SolenoidPanel y `OnKeyLogged` en Window con comentario de desvinculación, pero flujo no probado (sin hardware NFC disponible actualmente).
3. **Control I2C real**: SolenoidPanel activa/desactiva solenoides en UI, pero la escritura a XL9535 por I2C no está implementada.

## Posibles Bugs/Issues Conocidos
*(Vacío — reportar aquí los que se detecten)*
