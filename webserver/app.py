from flask import Flask, render_template, request, redirect, url_for, flash
import pymysql.cursors
from config import DB_CONFIG

app = Flask(__name__)
app.secret_key = "armario-llaves-secret"

ETYPE_NAMES = {
    0: "Recogida",
    1: "Devolución",
    2: "Admin abre",
    3: "Llave creada",
    4: "Llave desactivada",
}

ALL_POSITIONS = [
    f"{chr(ord('A') + r)}{c}" for r in range(4) for c in range(1, 9)
]  # ["A1", "A2", ..., "D8"]


def get_db():
    return pymysql.connect(**DB_CONFIG, cursorclass=pymysql.cursors.DictCursor)


def pos_to_string(pos):
    if pos is None or pos < 0 or pos > 31:
        return "-"
    return f"{chr(ord('A') + pos // 8)}{pos % 8 + 1}"


def pos_from_string(s):
    if not s or len(s) < 2:
        return -1
    try:
        return (ord(s[0].upper()) - ord("A")) * 8 + int(s[1:]) - 1
    except (ValueError, IndexError):
        return -1


app.jinja_env.globals.update(
    pos_to_string=pos_to_string,
    etype_name=lambda e: ETYPE_NAMES.get(int(e), str(e)),
    ALL_POSITIONS=ALL_POSITIONS,
)


# ─── USERS ────────────────────────────────────────────────────────────────────

@app.route("/")
def index():
    return redirect(url_for("users_list"))


@app.route("/users")
def users_list():
    db = get_db()
    with db.cursor() as cur:
        cur.execute(
            "SELECT id_, name_, password_, uid_, level_ FROM Person_ ORDER BY name_"
        )
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
                "UPDATE Person_ SET name_=%s, password_=%s, uid_=%s, level_=%s "
                "WHERE id_=%s",
                (name, password, uid, level, user_id),
            )
        db.commit()
        db.close()
        flash("Usuario actualizado.", "success")
        return redirect(url_for("users_list"))
    with db.cursor() as cur:
        cur.execute(
            "SELECT id_, name_, password_, uid_, level_ FROM Person_ WHERE id_=%s",
            (user_id,),
        )
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


# ─── KEYS ─────────────────────────────────────────────────────────────────────

@app.route("/keys")
def keys_list():
    db = get_db()
    with db.cursor() as cur:
        cur.execute(
            "SELECT id_, name_, ubi_, commentary_, pos_, active_, uid_, pub_ "
            "FROM Key_ ORDER BY pos_"
        )
        keys = cur.fetchall()
        for k in keys:
            cur.execute(
                "SELECT p.name_ FROM Person_ p "
                "JOIN Key_Person_Keep kk ON p.id_ = kk.Person2_ "
                "WHERE kk.Key1_ = %s",
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
            return render_template(
                "keys/form.html", key=None, persons=persons,
                authorized_ids=[], taken_pos=taken_pos, action="new"
            )
        with db.cursor() as cur:
            cur.execute(
                "INSERT INTO Key_ (type_, name_, ubi_, commentary_, pos_, active_, uid_, pub_) "
                "VALUES ('Key', %s, %s, %s, %s, %s, %s, %s)",
                (name, ubi, commentary, pos, active, uid, pub),
            )
            key_id = cur.lastrowid
            for pid in authorized_ids:
                cur.execute(
                    "INSERT INTO Key_Person_Acces (Key1_, Person2_) VALUES (%s, %s)",
                    (key_id, pid),
                )
        db.commit()
        db.close()
        flash(f'Llave "{name}" creada.', "success")
        return redirect(url_for("keys_list"))

    db.close()
    return render_template(
        "keys/form.html", key=None, persons=persons,
        authorized_ids=[], taken_pos=taken_pos, action="new"
    )


@app.route("/keys/<int:key_id>/edit", methods=["GET", "POST"])
def keys_edit(key_id):
    db = get_db()
    with db.cursor() as cur:
        cur.execute(
            "SELECT id_, name_, ubi_, commentary_, pos_, active_, uid_, pub_ "
            "FROM Key_ WHERE id_=%s",
            (key_id,),
        )
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
                "UPDATE Key_ SET name_=%s, ubi_=%s, commentary_=%s, pos_=%s, "
                "active_=%s, uid_=%s, pub_=%s WHERE id_=%s",
                (name, ubi, commentary, pos, active, uid, pub, key_id),
            )
            cur.execute("DELETE FROM Key_Person_Acces WHERE Key1_=%s", (key_id,))
            for pid in authorized_ids:
                cur.execute(
                    "INSERT INTO Key_Person_Acces (Key1_, Person2_) VALUES (%s, %s)",
                    (key_id, pid),
                )
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

    return render_template(
        "keys/form.html", key=key, persons=persons,
        authorized_ids=authorized_ids, taken_pos=taken_pos, action="edit"
    )


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


# ─── HISTORY ──────────────────────────────────────────────────────────────────

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

    return render_template(
        "history/list.html",
        events=events, keys=keys, persons=persons,
        filter_key=key_id, filter_person=person_id, filter_etype=etype,
        etype_names=ETYPE_NAMES,
    )


if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000, debug=False)
