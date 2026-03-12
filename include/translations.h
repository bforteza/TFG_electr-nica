#ifndef TRANSLATIONS_H
#define TRANSLATIONS_H

// Idiomas disponibles en la aplicación.
enum class Language { kCatalan = 0, kSpanish = 1, kEnglish = 2 };

// Idioma activo. Cambiar su valor y llamar a RefreshLabels() en cada stack
// para actualizar la interfaz sin reiniciar la aplicación.
inline Language current_language = Language::kCatalan;

// Todas las cadenas traducibles, agrupadas por clase para facilitar el mantenimiento.
struct Translations {

    // --- LoginStack ---
    struct Login {
        const char* title;
        const char* description;
        const char* error_device_not_found;
        const char* error_duplicate_uid;
        const char* error_invalid_credentials;
    } login;

    // --- HomeStack ---
    struct Home {
        const char* btn_solenoid_panel; // Abrir panel de control de solenoides
        const char* btn_create_key;
        const char* btn_create_user;
        const char* btn_view_keys;
        const char* btn_view_users;
        const char* btn_history;
    } home;

    // --- UsersViewStack ---
    struct UsersView {
        const char* btn_add_key;      // Vincular llave al usuario seleccionado
        const char* btn_remove_key;   // Desvincular llave del usuario seleccionado
        const char* btn_edit;
        const char* btn_view_keys;    // Ver llaves del usuario seleccionado
        const char* btn_select;       // Solo visible en modo selección de usuario
        const char* col_name;
        const char* col_password;
        const char* col_uid;
    } users_view;

    // --- UserCreateStack ---
    struct UserCreate {
        const char* lbl_username;
        const char* lbl_password;
        const char* lbl_repeat_password;
        const char* lbl_access_level;
        const char* btn_add_key;
        const char* btn_add_nfc;
        const char* radio_level_0;    // Sin acceso especial
        const char* radio_level_1;    // Acceso a gestión de llaves
        const char* radio_admin;      // Acceso de administrador
        const char* btn_create;
        const char* btn_edit;
        const char* prompt_scan_card; // Texto cuando NFC no detecta nada
        const char* error_username_exists;
        const char* error_password_mismatch;
        const char* error_password_exists;
        const char* error_username_too_short;
        const char* error_password_too_short;
        const char* error_card_in_use;
    } user_create;

    // --- KeyCreateStack ---
    struct KeyCreate {
        const char* lbl_key_name;
        const char* lbl_location;
        const char* lbl_comments;
        const char* lbl_position;
        const char* btn_add_users;
        const char* btn_add_nfc;
        const char* btn_create;
        const char* btn_edit;
        const char* prompt_scan_card; // Texto cuando NFC no detecta nada
        const char* error_name_too_short;
        const char* error_name_exists;
        const char* error_position_unavailable; // Prefijo; se añaden posiciones tomadas
        const char* error_position_invalid;
        const char* error_card_in_use;
        const char* btn_pick_position; // Botón para abrir el selector de posición
    } key_create;

    // --- SolenoidPanel ---
    struct Solenoid {
        const char* btn_repeat;        // Reactivar solenoide
        const char* btn_take_another;  // Volver a KeyView para coger otra llave
        const char* lbl_return;        // Prefijo "Devuelve en: "
        const char* lbl_activated;     // Prefijo "Activado: " (modo admin)
        const char* lbl_admin_title;   // Título panel admin
        const char* lbl_select_title;  // Título modo selección de posición
    } solenoid;

    // --- KeyViewStack ---
    struct KeyView {
        const char* btn_take;         // Registrar recogida/devolución de llave
        const char* btn_add_user;     // Dar acceso a un usuario
        const char* btn_remove_user;  // Quitar acceso a un usuario
        const char* btn_view_users;   // Ver usuarios con acceso a esta llave
        const char* btn_edit;
        const char* btn_select;       // Solo visible en modo selección de llave
        const char* btn_delete;
        const char* col_name;
        const char* col_location;
        const char* col_comments;
        const char* col_position;
        const char* col_active;
        const char* col_keeper;
        const char* err_key_in_use;
    } key_view;
};

inline const Translations kTranslations[] = {
    // ---- kCatalan ----
    {
        // login
        {
            "Introdueix credencials",
            "Introdueix les credencials mitjançant el teclat numèric o aproxima "
            "l'identificador nfc al lector. Podeu retornar una clau utilitzant "
            "també l'identificador nfc.",
            "Atenció: dispositiu no reconegut",
            "ERROR: dos dispositius amb el mateix uid, contacta amb l'administrador",
            "Atenció: credencials no identificades",
        },
        // home
        {
            "Controlar clauers",
            "Crear clau",
            "Crear usuari",
            "Veure claus",
            "Veure usuaris",
            "Historial",
        },
        // users_view
        {
            "Afegir clau",
            "Llevar clau",
            "Editar usuari",
            "Veure claus",
            "Seleccionar",
            "Nom", "Contrasenya", "Uid",
        },
        // user_create
        {
            "Nom usuari",
            "Contrasenya",
            "Repeteix contrasenya",
            "Nivell d'accés",
            "Agregar clau",
            "Agregar NFC",
            "A0", "A1", "Administrador",
            "Crear usuari",
            "Editar usuari",
            "Aproxima la tarjeta desitjada",
            "Atenció: nom de usuari existent",
            "Atenció: contrasenya no coincideix",
            "Atenció: contrasenya de usuari repetida",
            "Atenció: nom de usuari massa curt",
            "Atenció: contrasenya massa curta",
            "Atenció: tarjeta en us",
        },
        // key_create
        {
            "Nom de clau",
            "Ubicació",
            "Comentaris",
            "Posició",
            "Agregar usuaris",
            "Afegir NFC",
            "Crear clau",
            "Editar clau",
            "Aproxima la tarjeta desitjada",
            "Atenció: nom molt breu",
            "Atenció: nom de clau existent",
            "Posicions no disponibles:",
            "Atenció: posició no vàlida",
            "Atenció: tarjeta en us",
            "Triar posició",
        },
        // solenoid
        {
            "Repetir",
            "Agafar una altra clau",
            "Retorna a: ",
            "Activat: ",
            "Panel de control",
            "Tria una posició lliure",
        },
        // key_view
        {
            "Agafar clau",
            "Afegir usuari a clau",
            "Llevar usuari",
            "Veure usuaris",
            "Editar clau",
            "Seleccionar",
            "Eliminar",
            "Nom", "Ubicació", "Comentaris", "Posició", "Activa", "Agafada",
            "Atenció: la clau ja està en ús",
        },
    },
    // ---- kSpanish ----
    {
        // login
        {
            "Introduce credenciales",
            "Introduce tus credenciales mediante el teclado numérico o acerca el "
            "identificador NFC al lector. También puedes devolver una llave "
            "usando el identificador NFC.",
            "Atención: dispositivo no reconocido",
            "ERROR: dos dispositivos con el mismo uid, contacta con el administrador",
            "Atención: credenciales no identificadas",
        },
        // home
        {
            "Controlar cerraduras",
            "Crear llave",
            "Crear usuario",
            "Ver llaves",
            "Ver usuarios",
            "Historial",
        },
        // users_view
        {
            "Agregar llave",
            "Quitar llave",
            "Editar usuario",
            "Ver llaves",
            "Seleccionar",
            "Nombre", "Contraseña", "Uid",
        },
        // user_create
        {
            "Nombre de usuario",
            "Contraseña",
            "Repetir contraseña",
            "Nivel de acceso",
            "Agregar llave",
            "Agregar NFC",
            "A0", "A1", "Administrador",
            "Crear usuario",
            "Editar usuario",
            "Acerca la tarjeta deseada",
            "Atención: nombre de usuario existente",
            "Atención: las contraseñas no coinciden",
            "Atención: contraseña ya en uso",
            "Atención: nombre de usuario demasiado corto",
            "Atención: contraseña demasiado corta",
            "Atención: tarjeta en uso",
        },
        // key_create
        {
            "Nombre de llave",
            "Ubicación",
            "Comentarios",
            "Posición",
            "Agregar usuarios",
            "Agregar NFC",
            "Crear llave",
            "Editar llave",
            "Acerca la tarjeta deseada",
            "Atención: nombre demasiado corto",
            "Atención: nombre de llave existente",
            "Posiciones no disponibles:",
            "Atención: posición no válida",
            "Atención: tarjeta en uso",
            "Elegir posición",
        },
        // solenoid
        {
            "Repetir",
            "Coger otra llave",
            "Devuelve en: ",
            "Activado: ",
            "Panel de control",
            "Elige una posición libre",
        },
        // key_view
        {
            "Coger llave",
            "Agregar usuario a llave",
            "Quitar usuario",
            "Ver usuarios",
            "Editar llave",
            "Seleccionar",
            "Eliminar",
            "Nombre", "Ubicación", "Comentarios", "Posición", "Activa", "En posesión de",
            "Atención: la llave ya está en uso",
        },
    },
    // ---- kEnglish ----
    {
        // login
        {
            "Enter credentials",
            "Enter your credentials using the numeric keypad or bring the NFC "
            "identifier to the reader. You can also return a key using the NFC "
            "identifier.",
            "Warning: device not recognized",
            "ERROR: two devices share the same uid, contact the administrator",
            "Warning: invalid credentials",
        },
        // home
        {
            "Control panel",
            "Create key",
            "Create user",
            "View keys",
            "View users",
            "History",
        },
        // users_view
        {
            "Add key",
            "Remove key",
            "Edit user",
            "View keys",
            "Select",
            "Name", "Password", "Uid",
        },
        // user_create
        {
            "Username",
            "Password",
            "Repeat password",
            "Access level",
            "Add key",
            "Add NFC",
            "A0", "A1", "Administrator",
            "Create user",
            "Edit user",
            "Place the desired card on the reader",
            "Warning: username already exists",
            "Warning: passwords do not match",
            "Warning: password already in use",
            "Warning: username too short",
            "Warning: password too short",
            "Warning: card already in use",
        },
        // key_create
        {
            "Key name",
            "Location",
            "Comments",
            "Position",
            "Add users",
            "Add NFC",
            "Create key",
            "Edit key",
            "Place the desired card on the reader",
            "Warning: name too short",
            "Warning: key name already exists",
            "Unavailable positions:",
            "Warning: invalid position",
            "Warning: card already in use",
            "Pick position",
        },
        // solenoid
        {
            "Repeat",
            "Take another key",
            "Return to: ",
            "Activated: ",
            "Control panel",
            "Choose a free position",
        },
        // key_view
        {
            "Take key",
            "Add user to key",
            "Remove user",
            "View users",
            "Edit key",
            "Select",
            "Delete",
            "Name", "Location", "Comments", "Position", "Active", "Held by",
            "Warning: key already in use",
        },
    },
};

// Devuelve la estructura de traducciones del idioma actualmente seleccionado.
inline const Translations& Tr() {
    return kTranslations[static_cast<int>(current_language)];
}

#endif // TRANSLATIONS_H
