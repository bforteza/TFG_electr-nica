#include "window.h"
#include "sound_manager.h"

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
        main_stack_->set_visible_child("AdminView");
    });
    solenoid_panel_.signal_logout.connect(
        sigc::mem_fun(*this, &Window::OnBackButtonClicked));

    login_stack_.Start();

    // Inicializa el sistema de audio y conecta sonido de click a todos los botones.
    SoundManager::Init();
    SoundManager::ConnectToAllButtons(this);
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

void Window::OnBackButtonClicked() {
    main_stack_->set_visible_child("INI");
    login_stack_.Start();
}

// Navega al panel de administración y lo inicializa con el usuario identificado.
void Window::OnUserLogged(std::shared_ptr<kdb::Person> person) {
    main_stack_->set_visible_child("AdminView");
    home_stack_.PersonLogged(person);
}

// Desvincula al portador actual de la llave y navega al SolenoidPanel (RETURN).
void Window::OnKeyLogged(std::shared_ptr<kdb::Key> key) {
    // Desvincula al portador actual antes de abrir el solenoide de retorno.
    for (auto& keeper : key->keeper().get().all())
        key->keeper().unlink(keeper);

    OnOpenSolenoid(key, SolenoidPanel::Mode::RETURN);
}

void Window::OnOpenSolenoid(std::shared_ptr<kdb::Key> key, SolenoidPanel::Mode mode) {
    solenoid_panel_.Setup(key, mode);
    main_stack_->set_visible_child("KeySelect");
}
