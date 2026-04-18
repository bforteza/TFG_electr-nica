from flask import render_template, request, redirect, url_for, flash, jsonify
from flask_openapi3 import OpenAPI, Info, Tag
import pymysql.cursors
from config import DB_CONFIG
from pydantic import BaseModel, Field, field_validator
from typing import Optional

# ─── APP ──────────────────────────────────────────────────────────────────────

class ErrorResponse(BaseModel):
    success: bool = False
    message: str


def _validation_error_callback(e):
    """Normaliza todos los errores de validación de Pydantic al formato estándar."""
    try:
        errors = e.errors()
        if errors:
            msg = errors[0].get("msg", "Datos de entrada incorrectos")
            msg = msg.removeprefix("Value error, ")
        else:
            msg = "Datos de entrada incorrectos"
    except Exception:
        msg = "Datos de entrada incorrectos"
    r = jsonify({"success": False, "message": msg})
    r.status_code = 422
    return r


info = Info(
    title="Armario de Llaves",
    version="1.0.0",
    description="API REST para gestión remota del armario de llaves inteligente.",
)
app = OpenAPI(
    __name__,
    info=info,
    validation_error_model=ErrorResponse,
    validation_error_callback=_validation_error_callback,
)
app.secret_key = "armario-llaves-secret"

tag_users   = Tag(name="Usuarios",  description="Gestión de usuarios del sistema")
tag_keys    = Tag(name="Llaves",    description="Gestión de llaves del armario")
tag_history = Tag(name="Historial", description="Consulta del historial de eventos")

ETYPE_NAMES = {
    0: "Recogida",
    1: "Devolución",
    2: "Admin abre",
    3: "Llave creada",
    4: "Llave desactivada",
}

ALL_POSITIONS = [
    f"{chr(ord('A') + r)}{c}" for r in range(4) for c in range(1, 9)
]


# ─── HELPERS ──────────────────────────────────────────────────────────────────

def get_db():
    return pymysql.connect(**DB_CONFIG, cursorclass=pymysql.cursors.DictCursor)


def pos_to_string(pos):
    if pos is None or pos == 0:
        return "-"
    if pos > 32:
        return "-"
    pos -= 1  # convertir a 0-based para el cálculo
    return f"{chr(ord('A') + pos // 8)}{pos % 8 + 1}"


def pos_from_string(s):
    if not s or len(s) < 2:
        return 0  # 0 = sin asignar
    try:
        row = ord(s[0].upper()) - ord("A")
        col = int(s[1:]) - 1
        return row * 8 + col + 1  # 1-based: A1=1, D8=32
    except (ValueError, IndexError):
        return 0


def log_history(db, etype, keyid=0, keyname="", pos=0):
    """Escribe un evento en HistoryEvent_ indicando que el origen es el webserver."""
    from datetime import datetime
    timestamp   = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
    client_ip   = request.remote_addr or "IP desconocida"
    person_name = f"Webserver ({client_ip})"
    with db.cursor() as cur:
        cur.execute(
            "INSERT INTO HistoryEvent_ (type_, etype_, timestamp_, keyid_, keyname_, personid_, personname_, pos_) "
            "VALUES ('HistoryEvent', %s, %s, %s, %s, %s, %s, %s)",
            (etype, timestamp, keyid, keyname, 0, person_name, pos),
        )


app.jinja_env.globals.update(
    pos_to_string=pos_to_string,
    etype_name=lambda e: ETYPE_NAMES.get(int(e), str(e)),
    ALL_POSITIONS=ALL_POSITIONS,
)


# ─── MODELOS PYDANTIC (usados por Swagger para documentar la API) ─────────────
#
# Cada clase describe la forma de los datos que entran o salen de un endpoint.
# Swagger los usa para mostrar ejemplos y validar peticiones automáticamente.

class UserOut(BaseModel):
    """Datos de un usuario devueltos por la API."""
    id:     int
    name:   str
    level:  int  = Field(description="0=básico · 1=gestión llaves · 2=admin")
    active: bool

class UserDetailOut(UserOut):
    """Datos completos de un usuario, incluyendo contraseña."""
    password: str

class UserIn(BaseModel):
    """Datos para crear o editar un usuario."""
    name:     str  = Field(...,   description="Nombre del usuario (mínimo 3 caracteres)")
    password: str  = Field(...,   description="Contraseña numérica (mínimo 3 dígitos, obligatoria)")
    level:    int  = Field(0, ge=0, le=2, description="Nivel de acceso (0, 1 o 2)")
    active:   bool = Field(True,  description="False = usuario desactivado")

    @field_validator("name")
    @classmethod
    def name_min_length(cls, v):
        if len(v) < 3:
            raise ValueError("El nombre debe tener al menos 3 caracteres")
        return v

    @field_validator("password")
    @classmethod
    def password_min_length(cls, v):
        if len(v) < 3:
            raise ValueError("La contraseña debe tener al menos 3 dígitos")
        return v

class KeyOut(BaseModel):
    """Datos de una llave devueltos por la API."""
    id:          int
    name:        str
    ubi:         str  = Field(description="Ubicación física")
    commentary:  str
    pos:         str  = Field(description="Posición en armario (ej: A1, C4). '-' si no asignada")
    active:      bool
    pub:         bool = Field(description="True = accesible por todos los usuarios")
    keeper:      Optional[str] = Field(None, description="Nombre de quien tiene la llave. None = en armario")

class KeyDetailOut(KeyOut):
    """Datos de una llave con lista de usuarios autorizados."""
    authorized_users: list[dict] = Field(description="[{id, name}, ...]")

class KeyIn(BaseModel):
    """Datos para crear o editar una llave."""
    name:           str       = Field(...,  description="Nombre de la llave (mínimo 3 caracteres)")
    ubi:            str       = Field("",   description="Ubicación física")
    commentary:     str       = Field("",   description="Comentario")
    active:         bool      = Field(True, description="True = llave activa")
    pub:            bool      = Field(False,description="True = pública, accesible por todos")
    authorized_ids: list[int] = Field([],   description="IDs de usuarios autorizados (ignorado si pub=True)")

    @field_validator("name")
    @classmethod
    def name_min_length(cls, v):
        if len(v) < 3:
            raise ValueError("El nombre debe tener al menos 3 caracteres")
        return v

class HistoryEventOut(BaseModel):
    """Un evento del historial."""
    id:          int
    etype:       int
    etype_name:  str = Field(description="Nombre legible del tipo de evento")
    timestamp:   str
    key_id:      int
    key_name:    str
    person_id:   int
    person_name: str
    pos:         str = Field(description="Posición en armario (ej: A1)")

class HistoryQuery(BaseModel):
    """Parámetros de filtro para el historial (todos opcionales)."""
    key_id:    Optional[int] = Field(None, description="Filtrar por ID de llave")
    person_id: Optional[int] = Field(None, description="Filtrar por ID de usuario")
    date_from: Optional[str] = Field(None, description="Fecha inicio (YYYY-MM-DD)")
    date_to:   Optional[str] = Field(None, description="Fecha fin   (YYYY-MM-DD)")

class SuccessResponse(BaseModel):
    success: bool
    message: str = ""

class PathUserId(BaseModel):
    user_id: int

class PathKeyId(BaseModel):
    key_id: int


# ─── WEB HTML ─────────────────────────────────────────────────────────────────
# Estas rutas sirven páginas HTML para gestión desde el navegador.
# No aparecen en Swagger porque no son parte de la API JSON.

@app.route("/")
def index():
    return redirect(url_for("users_list"))

@app.route("/users")
def users_list():
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, password_, uid_, level_ FROM Person_ ORDER BY name_")
        users = cur.fetchall()
    db.close()
    for u in users:
        u["access"] = int(u["level_"])
    return render_template("users/list.html", users=users)

@app.route("/users/new", methods=["GET", "POST"])
def users_new():
    if request.method == "POST":
        name     = request.form["name"].strip()
        password = request.form["password"].strip()
        uid      = request.form["uid"].strip()
        level    = int(request.form.get("level", 0))
        if not name:
            flash("El nombre es obligatorio.", "danger")
            return render_template("users/form.html", user=None, action="new")
        db = get_db()
        with db.cursor() as cur:
            cur.execute(
                "INSERT INTO Person_ (type_, name_, password_, uid_, level_) "
                "VALUES ('Person', %s, %s, %s, %s)",
                (name, password, uid, level),
            )
        db.commit()
        db.close()
        flash(f'Usuario "{name}" creado.', "success")
        return redirect(url_for("users_list"))
    return render_template("users/form.html", user=None, action="new")

@app.route("/users/<int:user_id>/edit", methods=["GET", "POST"])
def users_edit(user_id):
    db = get_db()
    if request.method == "POST":
        name     = request.form["name"].strip()
        password = request.form["password"].strip()
        uid      = request.form["uid"].strip()
        level    = int(request.form.get("level", 0))
        with db.cursor() as cur:
            cur.execute(
                "UPDATE Person_ SET name_=%s, password_=%s, uid_=%s, level_=%s WHERE id_=%s",
                (name, password, uid, level, user_id),
            )
        db.commit()
        db.close()
        flash("Usuario actualizado.", "success")
        return redirect(url_for("users_list"))
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, password_, uid_, level_ FROM Person_ WHERE id_=%s", (user_id,))
        user = cur.fetchone()
    db.close()
    if not user:
        flash("Usuario no encontrado.", "danger")
        return redirect(url_for("users_list"))
    return render_template("users/form.html", user=user, action="edit")

@app.route("/users/<int:user_id>/delete", methods=["POST"])
def users_delete(user_id):
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT name_ FROM Person_ WHERE id_=%s", (user_id,))
        user = cur.fetchone()
        if user:
            cur.execute("DELETE FROM Key_Person_Acces WHERE Person2_=%s", (user_id,))
            cur.execute("DELETE FROM Key_Person_Keep  WHERE Person2_=%s", (user_id,))
            cur.execute("DELETE FROM Person_ WHERE id_=%s", (user_id,))
            db.commit()
            flash(f'Usuario "{user["name_"]}" eliminado.', "success")
        else:
            flash("Usuario no encontrado.", "danger")
    db.close()
    return redirect(url_for("users_list"))

@app.route("/keys")
def keys_list():
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, ubi_, commentary_, pos_, active_, uid_, pub_ FROM Key_ ORDER BY pos_")
        keys = cur.fetchall()
        for k in keys:
            cur.execute(
                "SELECT p.name_ FROM Person_ p "
                "JOIN Key_Person_Keep kk ON p.id_ = kk.Person2_ WHERE kk.Key1_ = %s",
                (k["id_"],),
            )
            keeper = cur.fetchone()
            k["keeper"] = keeper["name_"] if keeper else None
    db.close()
    return render_template("keys/list.html", keys=keys)

@app.route("/keys/new", methods=["GET", "POST"])
def keys_new():
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_ FROM Person_ ORDER BY name_")
        persons = cur.fetchall()
        cur.execute("SELECT pos_ FROM Key_ WHERE pos_ >= 0")
        taken_pos = {row["pos_"] for row in cur.fetchall()}
    if request.method == "POST":
        name           = request.form["name"].strip()
        ubi            = request.form["ubi"].strip()
        commentary     = request.form["commentary"].strip()
        pos            = pos_from_string(request.form.get("pos", ""))
        active         = 1 if request.form.get("active") else 0
        pub            = 1 if request.form.get("pub") else 0
        uid            = request.form["uid"].strip()
        authorized_ids = [] if pub else request.form.getlist("authorized")
        if not name:
            flash("El nombre es obligatorio.", "danger")
            db.close()
            return render_template("keys/form.html", key=None, persons=persons, authorized_ids=[], taken_pos=taken_pos, action="new")
        with db.cursor() as cur:
            cur.execute(
                "INSERT INTO Key_ (type_, name_, ubi_, commentary_, pos_, active_, uid_, pub_) "
                "VALUES ('Key', %s, %s, %s, %s, %s, %s, %s)",
                (name, ubi, commentary, pos, active, uid, pub),
            )
            key_id = cur.lastrowid
            for pid in authorized_ids:
                cur.execute("INSERT INTO Key_Person_Acces (Key1_, Person2_) VALUES (%s, %s)", (key_id, pid))
        db.commit()
        db.close()
        flash(f'Llave "{name}" creada.', "success")
        return redirect(url_for("keys_list"))
    db.close()
    return render_template("keys/form.html", key=None, persons=persons, authorized_ids=[], taken_pos=taken_pos, action="new")

@app.route("/keys/<int:key_id>/edit", methods=["GET", "POST"])
def keys_edit(key_id):
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, ubi_, commentary_, pos_, active_, uid_, pub_ FROM Key_ WHERE id_=%s", (key_id,))
        key = cur.fetchone()
    if not key:
        db.close()
        flash("Llave no encontrada.", "danger")
        return redirect(url_for("keys_list"))
    if request.method == "POST":
        name           = request.form["name"].strip()
        ubi            = request.form["ubi"].strip()
        commentary     = request.form["commentary"].strip()
        pos            = pos_from_string(request.form.get("pos", ""))
        active         = 1 if request.form.get("active") else 0
        pub            = 1 if request.form.get("pub") else 0
        uid            = request.form["uid"].strip()
        authorized_ids = [] if pub else [int(x) for x in request.form.getlist("authorized")]
        with db.cursor() as cur:
            cur.execute(
                "UPDATE Key_ SET name_=%s, ubi_=%s, commentary_=%s, pos_=%s, active_=%s, uid_=%s, pub_=%s WHERE id_=%s",
                (name, ubi, commentary, pos, active, uid, pub, key_id),
            )
            cur.execute("DELETE FROM Key_Person_Acces WHERE Key1_=%s", (key_id,))
            for pid in authorized_ids:
                cur.execute("INSERT INTO Key_Person_Acces (Key1_, Person2_) VALUES (%s, %s)", (key_id, pid))
        db.commit()
        db.close()
        flash("Llave actualizada.", "success")
        return redirect(url_for("keys_list"))
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_ FROM Person_ ORDER BY name_")
        persons = cur.fetchall()
        cur.execute("SELECT Person2_ FROM Key_Person_Acces WHERE Key1_=%s", (key_id,))
        authorized_ids = [row["Person2_"] for row in cur.fetchall()]
        cur.execute("SELECT pos_ FROM Key_ WHERE pos_ >= 0 AND id_ != %s", (key_id,))
        taken_pos = {row["pos_"] for row in cur.fetchall()}
    db.close()
    return render_template("keys/form.html", key=key, persons=persons, authorized_ids=authorized_ids, taken_pos=taken_pos, action="edit")

@app.route("/keys/<int:key_id>/delete", methods=["POST"])
def keys_delete(key_id):
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT name_ FROM Key_ WHERE id_=%s", (key_id,))
        key = cur.fetchone()
        if key:
            cur.execute("DELETE FROM Key_Person_Acces WHERE Key1_=%s", (key_id,))
            cur.execute("DELETE FROM Key_Person_Keep  WHERE Key1_=%s", (key_id,))
            cur.execute("DELETE FROM Key_ WHERE id_=%s", (key_id,))
            db.commit()
            flash(f'Llave "{key["name_"]}" eliminada.', "success")
        else:
            flash("Llave no encontrada.", "danger")
    db.close()
    return redirect(url_for("keys_list"))

@app.route("/history")
def history_list():
    db = get_db()
    key_id    = request.args.get("key_id", "")
    person_id = request.args.get("person_id", "")
    etype     = request.args.get("etype", "")
    query  = "SELECT * FROM HistoryEvent_ WHERE 1=1"
    params = []
    if key_id:
        query += " AND keyid_=%s";    params.append(key_id)
    if person_id:
        query += " AND personid_=%s"; params.append(person_id)
    if etype != "":
        query += " AND etype_=%s";    params.append(etype)
    query += " ORDER BY timestamp_ DESC LIMIT 500"
    with db.cursor() as cur:
        cur.execute(query, params)
        events = cur.fetchall()
        cur.execute("SELECT id_, name_ FROM Key_    ORDER BY name_")
        keys = cur.fetchall()
        cur.execute("SELECT id_, name_ FROM Person_ ORDER BY name_")
        persons = cur.fetchall()
    db.close()
    return render_template("history/list.html", events=events, keys=keys, persons=persons,
                           filter_key=key_id, filter_person=person_id, filter_etype=etype,
                           etype_names=ETYPE_NAMES)


# ─── API JSON ─────────────────────────────────────────────────────────────────
# Estas rutas devuelven JSON puro. Son las que usa el programador VB (o cualquier
# otro lenguaje). Swagger las documenta automáticamente en /openapi/swagger.

# ── Usuarios ──────────────────────────────────────────────────────────────────

@app.get("/api/users", tags=[tag_users], summary="Listar todos los usuarios",
         responses={"200": UserOut})
def api_users_list():
    """Lista todos los usuarios con id, nombre, nivel de acceso y estado activo."""
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, level_, active_ FROM Person_ ORDER BY name_")
        rows = cur.fetchall()
    db.close()
    return [{"id": r["id_"], "name": r["name_"], "level": int(r["level_"]), "active": bool(r["active_"])} for r in rows]


@app.post("/api/users", tags=[tag_users], summary="Crear un usuario",
          responses={"201": UserOut, "409": ErrorResponse, "422": ErrorResponse})
def api_users_create(body: UserIn):
    """
    Crea un nuevo usuario. Devuelve el registro completo con el id asignado.

    - **name**: obligatorio, mínimo 3 caracteres, único
    - **password**: mínimo 3 dígitos, debe ser única en el sistema
    - **level**: 0 = básico · 1 = gestión de llaves · 2 = administrador
    - **active**: False = usuario desactivado (por defecto True)
    """
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_ FROM Person_ WHERE name_=%s", (body.name,))
        if cur.fetchone():
            db.close()
            return {"success": False, "message": "Ya existe un usuario con ese nombre"}, 409
        if body.password:
            cur.execute("SELECT id_ FROM Person_ WHERE password_=%s", (body.password,))
            if cur.fetchone():
                db.close()
                return {"success": False, "message": "La contraseña ya está en uso"}, 409
        cur.execute(
            "INSERT INTO Person_ (type_, name_, password_, uid_, level_) VALUES ('Person', %s, %s, '', %s)",
            (body.name, body.password, body.level),
        )
        new_id = cur.lastrowid
    db.commit()
    db.close()
    return {"id": new_id, "name": body.name, "level": body.level}, 201


@app.get("/api/users/<int:user_id>", tags=[tag_users], summary="Consultar un usuario",
         responses={"200": UserDetailOut, "404": ErrorResponse})
def api_users_get(path: PathUserId):
    """Devuelve todos los campos de un usuario: id, nombre, contraseña, nivel y estado."""
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, password_, level_, active_ FROM Person_ WHERE id_=%s", (path.user_id,))
        row = cur.fetchone()
    db.close()
    if not row:
        return {"success": False, "message": "Usuario no encontrado"}, 404
    return {"id": row["id_"], "name": row["name_"], "password": row["password_"], "level": int(row["level_"]), "active": bool(row["active_"])}


@app.put("/api/users/<int:user_id>", tags=[tag_users], summary="Editar un usuario",
         responses={"200": SuccessResponse, "404": ErrorResponse, "409": ErrorResponse, "422": ErrorResponse})
def api_users_update(path: PathUserId, body: UserIn):
    """
    Reemplaza todos los datos de un usuario existente.

    - **name**: obligatorio, mínimo 3 caracteres, único
    - **password**: mínimo 3 dígitos, debe ser única en el sistema
    - **level**: 0 = básico · 1 = gestión de llaves · 2 = administrador
    - **active**: False = desactiva el usuario
    """
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT active_ FROM Person_ WHERE id_=%s", (path.user_id,))
        existing = cur.fetchone()
        if not existing:
            db.close()
            return {"success": False, "message": "Usuario no encontrado"}, 404
    with db.cursor() as cur:
        cur.execute("SELECT id_ FROM Person_ WHERE name_=%s AND id_!=%s", (body.name, path.user_id))
        if cur.fetchone():
            db.close()
            return {"success": False, "message": "Ya existe un usuario con ese nombre"}, 409
    if body.password:
        with db.cursor() as cur:
            cur.execute("SELECT id_ FROM Person_ WHERE password_=%s AND id_!=%s", (body.password, path.user_id))
            if cur.fetchone():
                db.close()
                return {"success": False, "message": "La contraseña ya está en uso"}, 409
    reactivating = (not existing["active_"]) and body.active
    with db.cursor() as cur:
        if reactivating:
            cur.execute(
                "UPDATE Person_ SET name_=%s, password_=%s, level_=%s, active_=%s, uid_='' WHERE id_=%s",
                (body.name, body.password, body.level, int(body.active), path.user_id),
            )
        else:
            cur.execute(
                "UPDATE Person_ SET name_=%s, password_=%s, level_=%s, active_=%s WHERE id_=%s",
                (body.name, body.password, body.level, int(body.active), path.user_id),
            )
    db.commit()
    db.close()
    return {"success": True, "message": "Usuario actualizado"}


# ── Llaves ────────────────────────────────────────────────────────────────────

@app.get("/api/keys", tags=[tag_keys], summary="Listar todas las llaves",
         responses={"200": KeyOut})
def api_keys_list():
    """Lista todas las llaves con posición, estado activo, visibilidad y portador actual."""
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, ubi_, pos_, active_, pub_ FROM Key_ ORDER BY pos_")
        rows = cur.fetchall()
        result = []
        for r in rows:
            cur.execute(
                "SELECT p.name_ FROM Person_ p JOIN Key_Person_Keep kk ON p.id_=kk.Person2_ WHERE kk.Key1_=%s",
                (r["id_"],),
            )
            keeper = cur.fetchone()
            result.append({
                "id":     r["id_"],
                "name":   r["name_"],
                "ubi":    r["ubi_"] or "",
                "pos":    pos_to_string(r["pos_"]),
                "active": bool(r["active_"]),
                "pub":    bool(r["pub_"]),
                "keeper": keeper["name_"] if keeper else None,
            })
    db.close()
    return result


@app.post("/api/keys", tags=[tag_keys], summary="Crear una llave",
          responses={"201": KeyOut, "409": ErrorResponse, "422": ErrorResponse})
def api_keys_create(body: KeyIn):
    """
    Crea una nueva llave. Devuelve el registro creado con el id asignado.

    - **active**: False = llave creada como inactiva (no se registra evento en historial)
    - **pub**: True = accesible por todos los usuarios (authorized_ids se ignora)
    - **authorized_ids**: lista de ids de usuarios autorizados (solo si pub = False)
    """
    authorized_ids = [] if body.pub else body.authorized_ids
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_ FROM Key_ WHERE name_=%s", (body.name,))
        if cur.fetchone():
            db.close()
            return {"success": False, "message": "Ya existe una llave con ese nombre"}, 409
        cur.execute(
            "INSERT INTO Key_ (type_, name_, ubi_, commentary_, pos_, active_, uid_, pub_) "
            "VALUES ('Key', %s, %s, %s, 0, %s, '', %s)",
            (body.name, body.ubi, body.commentary, int(body.active), int(body.pub)),
        )
        new_id = cur.lastrowid
        for pid in authorized_ids:
            cur.execute("INSERT INTO Key_Person_Acces (Key1_, Person2_) VALUES (%s, %s)", (new_id, pid))
    if body.active:
        log_history(db, etype=3, keyid=new_id, keyname=body.name, pos=0)
    db.commit()
    db.close()
    return {"id": new_id, "name": body.name, "pos": "-", "active": body.active, "pub": body.pub}, 201


@app.get("/api/keys/<int:key_id>", tags=[tag_keys], summary="Consultar una llave",
         responses={"200": KeyDetailOut, "404": ErrorResponse})
def api_keys_get(path: PathKeyId):
    """Devuelve todos los campos de una llave, incluyendo portador y lista de usuarios autorizados."""
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT id_, name_, ubi_, commentary_, pos_, active_, pub_ FROM Key_ WHERE id_=%s", (path.key_id,))
        row = cur.fetchone()
    if not row:
        db.close()
        return {"success": False, "message": "Llave no encontrada"}, 404
    with db.cursor() as cur:
        cur.execute(
            "SELECT p.name_ FROM Person_ p JOIN Key_Person_Keep kk ON p.id_=kk.Person2_ WHERE kk.Key1_=%s",
            (path.key_id,),
        )
        keeper = cur.fetchone()
    with db.cursor() as cur:
        cur.execute(
            "SELECT p.id_, p.name_ FROM Person_ p JOIN Key_Person_Acces ka ON p.id_=ka.Person2_ WHERE ka.Key1_=%s",
            (path.key_id,),
        )
        authorized = [{"id": p["id_"], "name": p["name_"]} for p in cur.fetchall()]
    db.close()
    return {
        "id":               row["id_"],
        "name":             row["name_"],
        "ubi":              row["ubi_"] or "",
        "commentary":       row["commentary_"] or "",
        "pos":              pos_to_string(row["pos_"]),
        "active":           bool(row["active_"]),
        "pub":              bool(row["pub_"]),
        "keeper":           keeper["name_"] if keeper else None,
        "authorized_users": authorized,
    }


@app.put("/api/keys/<int:key_id>", tags=[tag_keys], summary="Editar una llave",
         responses={"200": SuccessResponse, "404": ErrorResponse, "409": ErrorResponse, "422": ErrorResponse})
def api_keys_update(path: PathKeyId, body: KeyIn):
    """
    Reemplaza todos los datos de una llave existente.

    - **active**: al desactivar (True→False) registra etype=4 en historial; al reactivar (False→True) registra etype=3 y resetea el uid NFC
    - **pub**: True = accesible por todos (authorized_ids se ignora)
    - **authorized_ids**: lista de ids de usuarios autorizados (solo si pub = False)
    """
    authorized_ids = [] if body.pub else body.authorized_ids
    db = get_db()
    with db.cursor() as cur:
        cur.execute("SELECT active_, pos_, name_ FROM Key_ WHERE id_=%s", (path.key_id,))
        existing = cur.fetchone()
    if not existing:
        db.close()
        return {"success": False, "message": "Llave no encontrada"}, 404
    with db.cursor() as cur:
        cur.execute("SELECT id_ FROM Key_ WHERE name_=%s AND id_!=%s", (body.name, path.key_id))
        if cur.fetchone():
            db.close()
            return {"success": False, "message": "Ya existe una llave con ese nombre"}, 409
    reactivating  = (not existing["active_"]) and body.active
    deactivating  = existing["active_"] and (not body.active)
    with db.cursor() as cur:
        if reactivating:
            cur.execute(
                "UPDATE Key_ SET name_=%s, ubi_=%s, commentary_=%s, active_=%s, pub_=%s, uid_='' WHERE id_=%s",
                (body.name, body.ubi, body.commentary, int(body.active), int(body.pub), path.key_id),
            )
        else:
            cur.execute(
                "UPDATE Key_ SET name_=%s, ubi_=%s, commentary_=%s, active_=%s, pub_=%s WHERE id_=%s",
                (body.name, body.ubi, body.commentary, int(body.active), int(body.pub), path.key_id),
            )
        cur.execute("DELETE FROM Key_Person_Acces WHERE Key1_=%s", (path.key_id,))
        for pid in authorized_ids:
            cur.execute("INSERT INTO Key_Person_Acces (Key1_, Person2_) VALUES (%s, %s)", (path.key_id, pid))
    if reactivating:
        log_history(db, etype=3, keyid=path.key_id, keyname=existing["name_"], pos=existing["pos_"] or 0)
    elif deactivating:
        log_history(db, etype=4, keyid=path.key_id, keyname=existing["name_"], pos=existing["pos_"] or 0)
    db.commit()
    db.close()
    return {"success": True, "message": "Llave actualizada"}


# ── Historial ─────────────────────────────────────────────────────────────────

@app.get("/api/history", tags=[tag_history], summary="Consultar el historial",
         responses={"200": HistoryEventOut})
def api_history(query: HistoryQuery):
    """
    Devuelve eventos del historial. Todos los filtros son opcionales y combinables.

    - **key_id**: solo eventos de esa llave
    - **person_id**: solo eventos de ese usuario
    - **date_from / date_to**: rango de fechas en formato YYYY-MM-DD
    """
    db = get_db()
    sql    = "SELECT * FROM HistoryEvent_ WHERE 1=1"
    params = []
    if query.key_id:
        sql += " AND keyid_=%s";      params.append(query.key_id)
    if query.person_id:
        sql += " AND personid_=%s";   params.append(query.person_id)
    if query.date_from:
        sql += " AND timestamp_>=%s"; params.append(query.date_from + " 00:00:00")
    if query.date_to:
        sql += " AND timestamp_<=%s"; params.append(query.date_to   + " 23:59:59")
    sql += " ORDER BY timestamp_ DESC LIMIT 1000"
    with db.cursor() as cur:
        cur.execute(sql, params)
        rows = cur.fetchall()
    db.close()
    return [
        {
            "id":          r["id_"],
            "etype":       r["etype_"],
            "etype_name":  ETYPE_NAMES.get(r["etype_"], str(r["etype_"])),
            "timestamp":   r["timestamp_"],
            "key_id":      r["keyid_"],
            "key_name":    r["keyname_"] or "",
            "person_id":   r["personid_"],
            "person_name": r["personname_"] or "",
            "pos":         pos_to_string(r["pos_"]),
        }
        for r in rows
    ]


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)
