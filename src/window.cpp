#include "window.h"
#include "sound_manager.h"
#include "history_logger.h"
#include <gdk/gdkkeysyms.h>
#include <iostream>

// Ruta al archivo de interfaz gráfica, relativa al directorio de trabajo.
static constexpr const char* kGladePath = "./ui/MainWindow.glade";

Window::Window(Gtk::ApplicationWindow::BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject),
      builder_(builder),
      login_stack_(builder),
      home_stack_(builder, this),
      solenoid_panel_(builder, this)
{
    builder->get_widget("PantallasStack", main_stack_);
    if (!main_stack_) {
        throw std::runtime_error("No \"PantallasStack\" object in MainWindow.glade");
    }

    // Conecta las señales del login con los manejadores de esta ventana.
    login_stack_.user_logged.connect(sigc::mem_fun(*this, &Window::OnUserLogged));
    login_stack_.key_logged.connect(sigc::mem_fun(*this, &Window::OnKeyLogged));

    // Conecta las señales de HomeStack para abrir el SolenoidPanel.
    home_stack_.signal_open_solenoid.connect(
        sigc::mem_fun(*this, &Window::OnOpenSolenoid));

    // Conecta las señales del SolenoidPanel para la navegación de retorno.
    solenoid_panel_.signal_go_home.connect([this]() {
        main_stack_->set_visible_child("HomeView");
        ResetInactivityTimer();
    });
    solenoid_panel_.signal_logout.connect(
        sigc::mem_fun(*this, &Window::OnBackButtonClicked));
    solenoid_panel_.signal_position_selected.connect(
        sigc::mem_fun(*this, &Window::OnPositionSelected));
    solenoid_panel_.signal_slot_activated.connect(
        sigc::mem_fun(home_stack_, &HomeStack::OnSlotActivated));

    login_stack_.Start();

    // Inicializa el sistema de audio y conecta sonido de click a todos los botones.
    SoundManager::Init();
    SoundManager::ConnectToAllButtons(this);

    // Intercepta TODOS los eventos GDK antes de que se despachen a los widgets hijos.
    // on_button_press_event no funciona para GtkButton (consume el evento antes de que
    // suba al ApplicationWindow), así que usamos gdk_event_handler_set para capturarlo.
    gdk_event_handler_set([](GdkEvent* ev, gpointer data) {
        if (ev->type == GDK_BUTTON_PRESS || ev->type == GDK_TOUCH_BEGIN)
            static_cast<Window*>(data)->ResetInactivityTimer();
        gtk_main_do_event(ev);
    }, this, nullptr);

    if (getenv("KIOSK"))
        fullscreen();
}

Window::~Window() {}

Window* Window::create() {
    auto builder = Gtk::Builder::create_from_file(kGladePath);

    Window* window = nullptr;
    builder->get_widget_derived("MainWindow", window);
    if (!window) {
        throw std::runtime_error("No \"MainWindow\" object in MainWindow.glade");
    }
    return window;
}

// --- Timer de inactividad ---

void Window::ResetInactivityTimer() {
    inactivity_timer_conn_.disconnect();
    if (main_stack_->get_visible_child_name() != "HomeView") {
        std::cout << "[InactivityTimer] Stop (no en HomeView: "
                  << main_stack_->get_visible_child_name() << ")\n";
        return;
    }
    std::cout << "[InactivityTimer] Reset (" << kInactivitySeconds << "s)\n";
    inactivity_timer_conn_ = Glib::signal_timeout().connect_seconds([this]() {
        std::cout << "[InactivityTimer] Disparado — cerrando sesión\n";
        home_stack_.Logout();
        return false; // no repetir
    }, kInactivitySeconds);
}

void Window::StopInactivityTimer() {
    std::cout << "[InactivityTimer] Detenido explícitamente\n";
    inactivity_timer_conn_.disconnect();
}

// --- Eventos de usuario ---

bool Window::on_key_press_event(GdkEventKey* event) {
    ResetInactivityTimer();
    if (event->keyval == GDK_KEY_Escape) {
        get_application()->quit();
        return true;
    }
    return Gtk::ApplicationWindow::on_key_press_event(event);
}

void Window::OnBackButtonClicked() {
    StopInactivityTimer();
    main_stack_->set_visible_child("INI");
    login_stack_.Start();
}

// Navega al panel de administración y lo inicializa con el usuario identificado.
void Window::OnUserLogged(std::shared_ptr<kdb::Person> person) {
    main_stack_->set_visible_child("HomeView");
    home_stack_.PersonLogged(person);
    ResetInactivityTimer();
}

// Desvincula al portador actual de la llave y navega al SolenoidPanel (RETURN).
void Window::OnKeyLogged(std::shared_ptr<kdb::Key> key) {
    // Desvincula al portador actual antes de abrir el solenoide de retorno.
    for (auto& keeper : key->keeper().get().all())
        key->keeper().unlink(keeper);

    history::LogReturn(key);
    OnOpenSolenoid(key, SolenoidPanel::Mode::RETURN);
}

void Window::OnPositionSelected(int pos) {
    home_stack_.OnPositionSelected(pos);
}

void Window::OnOpenSolenoid(std::shared_ptr<kdb::Key> key, SolenoidPanel::Mode mode) {
    StopInactivityTimer();
    solenoid_panel_.Setup(key, mode);
    main_stack_->set_visible_child("KeySelect");
}
