#include "sound_manager.h"

ca_context* SoundManager::context_ = nullptr;
int         SoundManager::next_id_ = 1;

void SoundManager::Init() {
    // ca_gtk_context_get() devuelve el contexto canberra asociado a la app GTK,
    // creándolo si todavía no existe. No hay que liberar el puntero manualmente.
    context_ = ca_gtk_context_get();

    // Pre-carga todos los sonidos en memoria al arranque para eliminar el delay
    // de inicialización del backend de audio en la primera pulsación.
    static const char* kAllEvents[] = {
        "message",
        "complete",
        "dialog-error",
    };
    for (const char* id : kAllEvents) {
        ca_context_cache(context_,
            CA_PROP_EVENT_ID,              id,
            CA_PROP_MEDIA_ROLE,            "event",
            CA_PROP_CANBERRA_CACHE_CONTROL, "permanent",
            nullptr);
    }
}

void SoundManager::Play(SoundEvent event) {
    if (!context_) return;

    const char* event_id;
    switch (event) {
        case SoundEvent::kClick:      event_id = "message"; break;
        case SoundEvent::kLoginOk:    event_id = "complete";       break;
        case SoundEvent::kLoginError: event_id = "dialog-error";   break;
        case SoundEvent::kKeyReturn:  event_id = "complete";       break;
        default: return;
    }

    // Cada llamada usa un ID único para que los sonidos no se cancelen entre sí.
    ca_context_play(context_, next_id_++,
        CA_PROP_EVENT_ID,               event_id,
        CA_PROP_MEDIA_ROLE,             "event",
        CA_PROP_CANBERRA_CACHE_CONTROL, "permanent",
        nullptr);
}

void SoundManager::ConnectToAllButtons(Gtk::Container* container) {
    for (Gtk::Widget* child : container->get_children()) {
        if (auto* btn = dynamic_cast<Gtk::Button*>(child)) {
            btn->signal_clicked().connect([]() {
                SoundManager::Play(SoundEvent::kClick);
            });
        }
        // Gtk::Button también es Gtk::Container (Gtk::Bin), así que se entra
        // igualmente para cubrir posibles widgets anidados dentro del botón.
        if (auto* cnt = dynamic_cast<Gtk::Container*>(child)) {
            ConnectToAllButtons(cnt);
        }
    }
}
