#pragma once

#include <gtkmm.h>
#include <canberra-gtk.h>

// Eventos de audio semánticos de la aplicación.
enum class SoundEvent {
    kClick,       // Feedback táctil genérico de botón
    kLoginOk,     // Identificación correcta
    kLoginError,  // Identificación incorrecta
    kKeyReturn,   // Devolución de llave
    kTimeout,     // Tiempo de espera agotado
};

// Gestión de audio mediante libcanberra-gtk3.
// Clase puramente estática; no se instancia.
class SoundManager {
public:
    SoundManager() = delete;

    // Inicializa el contexto de audio. Llamar una vez al arranque.
    static void Init();

    // Reproduce un evento de audio.
    static void Play(SoundEvent event);

    // Recorre el árbol de widgets y conecta Play(kClick) a cada Gtk::Button.
    static void ConnectToAllButtons(Gtk::Container* container);

private:
    static ca_context* context_;
    static int         next_id_;   // ID único por sonido (evita cancelaciones)
};
