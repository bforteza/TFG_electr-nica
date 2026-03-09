# TFG Electrónica - Armario de Llaves (Raspberry Pi)

## Descripción del Proyecto
Sistema de gestión de llaves físicas para una empresa, corriendo en Raspberry Pi.
- Lenguaje: C++, UI: gtkmm (GTK), BD: LiteSQL (SQLite), NFC: libnfc
- Convención de código: Google C++ Style Guide (snake_case para archivos y variables)
- IDE: Code::Blocks. UI en Glade (MainWindow.glade en ./ui/)

## Arquitectura de Pantallas (GTK Stack)
- `LoginStack` → pantalla inicial; login por NFC o texto. Emite señal `UserLogg` o `KeyLogg`
- `HomeStack` → panel admin con subvistas (AdminStacks inner stack):
  - `UsersViewStack` → lista de usuarios
  - `UserCreateStack` → crear/editar usuario
  - `KeyViewStack` → lista de llaves
  - `KeyCreateStack` → crear/editar llave
- `SolenoidPanel` → panel de apertura de llaves (casi vacío, pendiente de implementar)
- `Window` → ventana principal que contiene todos los stacks

## Modelos de Base de Datos (LiteSQL, SQLite en kdb.db)
**Person**: id, type, name, password, uid (NFC UID), a1 (bool), a2 (bool)
**Key**: id, type, name, ubi (ubicación física), commentary, pos (int, hueco armario), active (bool), uid (NFC UID)
**Relaciones**:
- `Key_Person_Acces` (M:N): llaves a las que tiene acceso una persona → `person.keys()` / `key.owners()`
- `Key_Person_Keep` (1:N, Key1_ UNIQUE): quién tiene actualmente cada llave → `person.keepkeys()` / `key.keeper()`

**Niveles de acceso** (calculado como `(int)a1 + 2*(int)a2`):
- 0: solo ver sus llaves asignadas
- 1 (a1=true): ver+gestionar llaves (crear, editar, vincular usuarios)
- 2 (a2=true): administrador completo (+ gestión usuarios, crear usuarios)
- Nota: acces==3 (ambos true) no muestra gestión de usuarios (bug potencial: usa `==2` no `>=2`)

## Componentes Clave
- `NfcManager` (nfc_manager.h/cpp): hilo separado, detecta tags ISO14443A, guarda UID en `uid`, emite `Glib::Dispatcher dispatcher` para comunicar con GTK thread-safe. `StopPolling()` usa `detach()`.
- `db_schema.hpp` + `db_schema.cpp`: modelos LiteSQL (autogenerados por LiteSQL tool)
- `globals.h`: instancias globales `db` (DbSchema*) y `nfcman` (unique_ptr<NfcManager>), `PosToString(int pos)` y `PosFromString(string)` para conversión posición↔string (A1-D8)
- `models.h`: `ModelColumns` (usuarios) y `KeyModelColumns` (llaves) para Gtk::TreeView — columnas en snake_case (`name_col`, `id_col`, `pos_col`, etc.)

## Estado del Refactor — COMPLETADO
Checklist aplicado: nomenclatura Google, accesos correctos, comentarios, traducción vía Tr(), RefreshLabels() suscrito a language_changed.

| Clase | .h | .cpp | Glade labels |
|---|---|---|---|
| `Window` | ✓ | ✓ | — |
| `LoginStack` | ✓ | ✓ | — |
| `HomeStack` | ✓ | ✓ | — |
| `KeyViewStack` | ✓ | ✓ | — (columnas vía RefreshLabels) |
| `KeyCreateStack` | ✓ | ✓ | ✓ IDs añadidos |
| `UsersViewStack` | ✓ | ✓ | — (columnas vía RefreshLabels) |
| `UserCreateStack` | ✓ | ✓ | ✓ IDs añadidos |
| `SolenoidPanel` | ✓ | ✓ | — |
| `Application` | ✓ | ✓ | — |
| `globals.h` | ✓ | — | — |
| `NfcManager` | ✓ | ✓ | — |

- `nfc_utils.h`: código C-style para escritura NDEF en tarjetas Mifare (uso futuro, no incluido aún)
- `main.cpp`: limpiado, password de desarrollo sustituida por `CAMBIAR` con TODO

## Convenciones de Nomenclatura Aplicadas
- Métodos: PascalCase (`PersonLogged`, `OnBackButtonClicked`, `RefreshLabels`)
- Miembros privados: snake_case con trailing underscore (`window_`, `inner_stack_`, `logged_person_`)
- Señales públicas: snake_case sin underscore (`user_selected`, `key_link`, `keys_view`)
- Overrides GTK: mantener nombre original (`on_activate`, `on_startup`)
- Mensajes de error: siempre `"MainWindow.glade"` (nunca `"INI.glade"`)
- Labels anónimas en Glade: añadir ID con prefijo del stack (`KeyCreate*`, `UserCreate*`)

## Próximo Paso — Entorno de Pruebas
Probar en **VirtualBox + Raspberry Pi OS Desktop (64-bit, imagen .iso para PC)**.
- Carpeta compartida VirtualBox para editar en Windows y compilar en la VM
- NfcManager arranca sin lanzar excepción si no hay hardware (device_ queda nullptr, solo cerr)
- MariaDB se instala en la propia VM
- Toda la UI y flujo por contraseña funcionarán sin hardware real

## Partes Pendientes de Implementar
1. **SolenoidPanel**: abrir solenoides por I2C con expansor GPIO y relés. Solo tiene botón "atrás". Arquitecturalmente pertenece a HomeStack (sub-vista de HomeInnerStack), no a Window.
2. **Devolución de llaves por NFC**: `key_logged` en LoginStack conectado a stub `OnKeyLogged` en Window (pendiente navegar a KeyLoginStack).
3. **Historial**: botón `history_button_` en HomeStack existe, señal comentada, sin implementar.

## Flujo Principal
1. App arranca → LoginStack activo → NfcManager polling
2. Usuario acerca tarjeta NFC → NfcDetected() → busca en BD (Person o Key activa) → emite señal
3. Si es Person: Window → "AdminView" → HomeStack.PersonLogg() (muestra botones según nivel acceso)
4. Si es Key activa: KeyLogg emitido (pendiente de conectar en Window)
5. Login manual por password en campo texto también disponible
6. Botón atrás → vuelve a "INI" (LoginStack) y reinicia NFC polling

## Hardware del Sistema (del documento TFG)
- **Raspberry Pi 4** (2GB RAM) como unidad central
- **PN532** lector NFC por I2C
- **XL9535** expansor de puertos I2C + módulo de 16 relés con optoacopladores (aislamiento galvánico)
- **Armario**: 4 filas × 8 columnas = 32 posiciones, fabricado por herrero local
- **Circuito solenoides**: matriz con diodos — controla 32 solenoides con solo 4+8=12 relés (n² elementos con 2n relés)
  - Solenoide desactivado = pasador cae = llave bloqueada (lógica inversa)
  - Diodos evitan caminos alternativos en la matriz; diodo de descarga de bobina por fila (no por solenoide)
  - Para abrir: activar relé de fila LUEGO relé de columna; para cerrar: desactivar relé fila primero (da tiempo a descarga)
- **Pantalla táctil** conectada por USB (interfaz principal de interacción)
- **Teclado numérico** (usuario normal): solo dígitos 0-9 — passwords deben ser numéricas
- **Teclado completo** (solo administrador): disponible para formularios de texto (nombres, ubicaciones, etc.)

## Requisitos Funcionales del TFG (estado)
- RF-01: Identificación NFC/password ✓ implementado
- RF-02: Verificación de permisos ✓ implementado
- RF-03: Control hardware (solenoides) ✗ pendiente
- RF-04: Historial de acciones ✗ pendiente
- RF-05: Interfaz de gestión remota ✗ pendiente
- RF-06: Múltiples idiomas ✗ pendiente (UI actualmente en catalán)

## Base de Datos
- Backend: MariaDB corriendo en la Raspberry Pi, accesible por red local
- LiteSQL abstrae el acceso; solo cambia la cadena de conexión al instanciar DbSchema
- Empezó con SQLite (kdb.db) pero se migró a MariaDB para acceso en red — kdb.db eliminado y obsoleto

## Posibles Bugs/Issues Conocidos
- `acces==2` en lugar de `>=2` en HomeStack: usuario con a1=true y a2=true (acces=3) no ve gestión de usuarios
- `StopPolling()` usa `detach()` en lugar de `join()`: el hilo NFC queda desvinculado pero no hay garantía de que pare antes de que se destruya NfcManager
- `KeyLogg` signal no conectado en Window: devolver llave por NFC no hace nada actualmente
