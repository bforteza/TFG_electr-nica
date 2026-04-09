#ifndef KEYBOARD_HELPER_H
#define KEYBOARD_HELPER_H

#include <cstdio>
#include <string>
#include <algorithm>
#include <glibmm/main.h>
#include "globals.h"

// Ruta al script del teclado virtual, relativa al directorio de trabajo del ejecutable.
static constexpr const char* kKeyboardScript = "./keyboard/keyboard.py";

// Guard global: evita abrir el teclado dos veces si focus-in se dispara de nuevo
// mientras el teclado ya está abierto o justo al cerrarse.
inline bool keyboard_running = false;

// Lanza el teclado virtual como subproceso y devuelve el texto introducido por
// el usuario. Bloquea hasta que el teclado se cierra.
//
// title   — texto que se muestra sobre el campo de entrada (etiqueta del campo)
// initial — texto inicial para editar un valor existente (vacío si es nuevo)
//
// Devuelve la cadena escrita por el usuario, o "" si canceló (cerró sin pulsar Intro).
inline std::string OpenKeyboard(const std::string& title, const std::string& initial = "") {
    if (keyboard_running) return "";
    keyboard_running = true;

    // Notifica a Window para que detenga el timer de inactividad.
    keyboard_open_changed.emit(true);

    // Elimina comillas simples del texto inicial para evitar problemas en el shell.
    // El título proviene de traducciones (strings constantes), no necesita sanitización.
    std::string safe_initial = initial;
    safe_initial.erase(
        std::remove(safe_initial.begin(), safe_initial.end(), '\''),
        safe_initial.end());

    std::string cmd = "python3 " + std::string(kKeyboardScript)
                    + " --title '" + title + "'"
                    + " --text '"  + safe_initial + "' 2>/dev/null";

    FILE* pipe = popen(cmd.c_str(), "r");
    std::string result;
    if (pipe) {
        char buffer[512] = {};
        if (fgets(buffer, sizeof(buffer), pipe))
            result = buffer;
        pclose(pipe);
    }

    // Quita el salto de línea final que agrega Python al imprimir.
    if (!result.empty() && result.back() == '\n')
        result.pop_back();

    // Notifica a Window para que reanude el timer de inactividad.
    keyboard_open_changed.emit(false);

    // Rearma el guard con un pequeño retardo para absorber cualquier evento
    // focus-in pendiente en la cola de GTK que llegue al cerrarse el teclado.
    Glib::signal_timeout().connect_once([]() {
        keyboard_running = false;
    }, 200);

    return result;
}

#endif // KEYBOARD_HELPER_H
