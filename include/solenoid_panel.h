#ifndef SOLENOID_PANEL_H
#define SOLENOID_PANEL_H

#include <array>
#include <memory>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/box.h>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <glibmm/main.h>
#include "dbmanager.hpp"

class Window;

// Panel de apertura de solenoides. Sirve como guía para el usuario
// al recoger/devolver una llave, y como herramienta de control para el
// administrador. Se muestra en PantallasStack bajo la página "KeySelect".
class SolenoidPanel {
public:
    // PICKUP: usuario recoge una llave asignada.
    // RETURN: usuario devuelve una llave (sin autenticación, por NFC de la llave).
    // ADMIN : administrador activa solenoides manualmente.
    enum class Mode { PICKUP, RETURN, ADMIN, SELECT };

    SolenoidPanel(const Glib::RefPtr<Gtk::Builder>& builder, Window* window);

    // Configura el panel antes de navegar a él. Para ADMIN, key puede ser nullptr.
    void Setup(std::shared_ptr<kdb::Key> key, Mode mode);

    // Emitida cuando el panel solicita volver al HomeStack (PICKUP o ADMIN).
    sigc::signal<void> signal_go_home;

    // Emitida cuando el panel solicita cerrar sesión (RETURN o timeout de PICKUP).
    sigc::signal<void> signal_logout;

    // Emitida en modo SELECT cuando el usuario elige una posición libre (1-based).
    sigc::signal<void, int> signal_position_selected;

    // Emitida en modo ADMIN cuando el administrador activa un slot (1-based).
    // Parámetros: posición (1-based), llave en ese slot o nullptr si está vacío.
    sigc::signal<void, int, std::shared_ptr<kdb::Key>> signal_slot_activated;

private:
    static constexpr int kRows           = 4;
    static constexpr int kCols           = 8;
    static constexpr int kTimeoutSeconds = 20;

    Window* window_;
    Mode    current_mode_;
    std::shared_ptr<kdb::Key> current_key_;

    // Widgets de Glade
    Gtk::Box*    panel_box_;
    Gtk::Button* back_button_;

    // Widgets creados programáticamente
    Gtk::Grid*   grid_;
    Gtk::Box*    info_bar_;
    Gtk::Label*  key_info_label_;
    Gtk::Label*  countdown_label_;
    Gtk::Button* repeat_button_;
    Gtk::Button* action_button_;   // "Coger otra llave" (solo PICKUP)

    // Los 32 botones de posición (A1-D8).
    std::array<Gtk::Button*, kRows * kCols> slot_buttons_;

    int             countdown_;
    int             active_admin_slot_ = -1;   // -1 = ninguno activo
    sigc::connection timer_conn_;

    // Crea el grid y la barra inferior y los añade a panel_box_.
    // Solo se llama una vez desde el constructor.
    void BuildWidgets();

    // Actualiza los colores CSS de los slots según el modo y las posiciones
    // registradas en la BD.
    void UpdateGrid();

    // Activa/para el temporizador de cuenta atrás.
    void StartTimer();
    void StopTimer();
    bool OnTimerTick();   // Devuelve false para detener el timer.

    void OnBackButtonClicked();
    void OnRepeatButtonClicked();
    void OnActionButtonClicked();         // "Coger otra llave"
    void OnSlotButtonClicked(int idx);    // Solo en modo ADMIN
};

#endif // SOLENOID_PANEL_H
