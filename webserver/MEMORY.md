# Webserver — Armario de Llaves Inteligente

## Propósito

Proceso Python independiente (separado del programa C++ GTK en la Raspberry Pi) que accede a la misma base de datos MariaDB. Ofrece:

1. **Panel HTML** en `/` → accesible desde cualquier navegador en la LAN.
2. **API REST JSON** en `/api/` → para que el programador de la empresa integre el sistema en su software (VB u otro lenguaje).
3. **Swagger UI** en `/openapi/` → documentación interactiva de la API.

Sin autenticación (diseño intencionado para red privada LAN). No controla el solenoide de forma remota.

---

## Stack técnico

| Componente | Detalle |
|---|---|
| Framework | `flask-openapi3[swagger]>=4.0` |
| Validación | Pydantic v2 (`BaseModel`, `Field`, `field_validator`) |
| Base de datos | MariaDB vía `pymysql>=1.1` (SQL directo, sin ORM) |
| UI HTML | Jinja2 + Bootstrap 5 + Bootstrap Icons |

**Archivos:**
```
webserver/
  app.py          ← todo el código (rutas HTML + API + modelos Pydantic)
  config.py       ← credenciales BD (ajustar en producción)
  requirements.txt
  templates/
    base.html
    users/list.html
    keys/list.html
    history/list.html
```

---

## Base de datos — convenciones críticas

**LiteSQL convention**: todos los nombres de columna tienen sufijo `_` (ej: `id_`, `name_`, `level_`).

### Tablas

**`Person_`** — usuarios
```
id_, type_='Person', name_, password_, uid_ (NFC), level_ (int 0/1/2)
```

**`Key_`** — llaves físicas
```
id_, type_='Key', name_, ubi_, commentary_, pos_ (int), active_, uid_, pub_
```

**`Key_Person_Acces`** — usuarios autorizados a coger una llave
```
Key1_ → Key_.id_,  Person2_ → Person_.id_
```

**`Key_Person_Keep`** — quién tiene cogida actualmente una llave
```
Key1_ (UNIQUE) → Key_.id_,  Person2_ → Person_.id_
```

**`HistoryEvent_`** — log de eventos
```
id_, type_='HistoryEvent', etype_, timestamp_, keyid_, keyname_, personid_, personname_, pos_
```

### Codificación de posiciones (`pos_`)

Armario 4 filas (A–D) × 8 columnas (1–8) = 32 posiciones.

| `pos_` | Posición |
|---|---|
| 0 | Sin asignar |
| 1 | A1 |
| 8 | A8 |
| 9 | B1 |
| 32 | D8 |

Fórmula 1-based: `pos = fila*8 + col` donde A=0, col 1-based.
`pos_to_string(pos)` → "A1", `pos_from_string("A1")` → 1.

### Tipos de evento (`etype_`)

| `etype_` | Nombre |
|---|---|
| 0 | Recogida |
| 1 | Devolución |
| 2 | Admin abre |
| 3 | Llave creada |
| 4 | Llave desactivada |

---

## Arquitectura de app.py

### Orden de definición (crítico)

```python
class ErrorResponse(BaseModel): ...      # 1. PRIMERO
def _validation_error_callback(e): ...  # 2. usa ErrorResponse
app = OpenAPI(__name__, info=info,
    validation_error_model=ErrorResponse,
    validation_error_callback=_validation_error_callback)  # 3. LUEGO
```

Si `ErrorResponse` se define después de `app = OpenAPI(...)`, falla en tiempo de carga.

### Helpers

```python
get_db()           # abre conexión PyMySQL con DictCursor
pos_to_string(pos) # int → "A1" (0 → "-")
pos_from_string(s) # "A1" → int ("" → 0)
log_history(db, etype, keyid, keyname, pos)
    # Escribe en HistoryEvent_ con personid_=0
    # y personname_="Webserver (<IP>)"
    # Llamado solo en creación (etype=3) y eliminación (etype=4) de llaves
```

### Patrón de cursor (importante)

Usar **cursores separados** para queries independientes dentro de la misma conexión:
```python
with db.cursor() as cur:
    cur.execute("SELECT ..."); row = cur.fetchone()
# ← abrir nuevo cursor para la siguiente query
with db.cursor() as cur:
    cur.execute("SELECT ..."); other = cur.fetchone()
```
Reutilizar el mismo cursor sin vaciarlo puede hacer que el segundo `fetchone()` devuelva `None`.

---

## Rutas HTML

Panel de **solo consulta** (sin formularios de creación/edición/eliminación).

| Método | Ruta | Descripción |
|---|---|---|
| GET | `/` | Redirige a `/users` |
| GET | `/users` | Lista todos los usuarios (ID, nombre, nivel, estado) |
| GET | `/keys` | Lista llaves activas (ID, pos, nombre, ubicación, acceso, posesión) |
| GET | `/history` | Historial (filtros: key_id, person_id, etype) |

### Columnas `/users`
ID · Nombre · Nivel de acceso (badge 0/1/2) · Estado (Activo / Inactivo)

### Columnas `/keys`
Solo muestra llaves con `active_=1`. Columnas: ID · Posición · Nombre · Ubicación · Acceso (badge "Pública" si `pub_=1`, o badges con nombres de usuarios autorizados) · En posesión de (nombre o "En armario").

**Archivos de plantilla:**
```
templates/
  base.html
  users/list.html
  keys/list.html
  history/list.html
```
(`users/form.html` y `keys/form.html` eliminados — ya no hay CRUD en HTML)

---

## API REST JSON

Base URL: `http://<IP>:5000/api/`

### Usuarios

| Método | Ruta | Respuestas |
|---|---|---|
| GET | `/api/users` | 200: `[UserOut]` |
| POST | `/api/users` | 201: `UserOut` · 409: nombre o contraseña duplicada · 422: validación |
| GET | `/api/users/<id>` | 200: `UserDetailOut` · 404 |
| PUT | `/api/users/<id>` | 200: `SuccessResponse` · 404 · 409 · 422 |

### Llaves

| Método | Ruta | Respuestas |
|---|---|---|
| GET | `/api/keys` | 200: `[KeyOut]` |
| POST | `/api/keys` | 201: `KeyOut` · 409: nombre duplicado · 422 |
| GET | `/api/keys/<id>` | 200: `KeyDetailOut` · 404 |
| PUT | `/api/keys/<id>` | 200: `SuccessResponse` · 404 · 409 · 422 |

### Historial

| Método | Ruta | Respuestas |
|---|---|---|
| GET | `/api/history` | 200: `[HistoryEventOut]` |

Query params opcionales: `key_id`, `person_id`, `date_from` (YYYY-MM-DD), `date_to`.
Máx. 1000 eventos, ordenados por fecha desc.

---

## Modelos Pydantic

```
UserIn:          name, password (≥3 dígitos, obligatoria), level (0-2), active (bool)
UserOut:         id, name, level, active
UserDetailOut:   hereda UserOut + password  ← usado por GET /api/users/<id>
KeyIn:           name, ubi, commentary, active, pub, authorized_ids
KeyOut:          id, name, ubi, commentary, pos (str), active, pub, keeper (str|None)
KeyDetailOut:    hereda KeyOut + authorized_users ([{id, name}])
HistoryEventOut: id, etype, etype_name, timestamp, key_id, key_name, person_id, person_name, pos
HistoryQuery:    key_id?, person_id?, date_from?, date_to?
SuccessResponse: success, message
ErrorResponse:   success=False, message
PathUserId:      user_id (int de la URL)
PathKeyId:       key_id  (int de la URL)
```

---

## Formato de errores

Todos los errores usan el mismo formato:
```json
{"success": false, "message": "Descripción del error"}
```
Incluye 404, 409 y 422 (normalizado por `_validation_error_callback`).

**Nota**: flask-openapi3 añade 422 en Swagger para todos los endpoints (incluso GETs sin body). Es una limitación de la librería, no un bug.

---

## Validaciones de negocio

- Contraseña: mínimo 3 dígitos, obligatoria; debe ser única en `Person_`
- Nombre de usuario: único en `Person_`
- Nombre de llave: único en `Key_`
- Posición: puede estar vacía (→ `pos_=0`); si `pub=True`, `authorized_ids` se ignora

## Lógica de activación/desactivación

### Usuarios (`PUT /api/users/<id>`)
- Al **reactivar** (`active 0→1`): se resetea `uid_=''` para forzar nuevo registro NFC
- No se pueden eliminar usuarios por API (DELETE eliminado)

### Llaves (`PUT /api/keys/<id>`)
- Al **desactivar** (`active 1→0`): registra `etype=4` (Llave desactivada) en `HistoryEvent_`
- Al **reactivar** (`active 0→1`): registra `etype=3` (Llave creada) en `HistoryEvent_` y resetea `uid_=''`
- No se pueden eliminar llaves por API (DELETE eliminado)

### Creación de llaves (`POST /api/keys`)
- Solo registra evento `etype=3` en historial si `active=True`; si se crea inactiva no genera evento

---

## Arrancar el servidor

```bash
cd webserver
source venv/bin/activate
python app.py        # escucha en 0.0.0.0:5000
```

Para parar: `Ctrl+C` o `kill <PID>`.

Swagger UI: `http://localhost:5000/openapi/`
