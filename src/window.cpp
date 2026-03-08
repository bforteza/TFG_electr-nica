#include "window.h"

// Ruta al archivo de interfaz gráfica, relativa al directorio de trabajo.
static constexpr const char* kGladePath = "./ui/MainWindow.glade";

Window::Window(Gtk::ApplicationWindow::BaseObjectType* cobject,
               const Glib::RefPtr<Gtk::Builder>& builder)
    : Gtk::ApplicationWindow(cobject),
      builder_(builder),
      home_stack_(builder, this),
      login_stack_(builder)
{
    builder->get_widget("PantallasStack", main_stack_);
    if (!main_stack_) {
        throw std::runtime_error("No \"PantallasStack\" object in MainWindow.glade");
    }

    // Conecta las señales del login con los manejadores de esta ventana.
    login_stack_.user_logged.connect(sigc::mem_fun(*this, &Window::OnUserLogged));
    login_stack_.key_logged.connect(sigc::mem_fun(*this, &Window::OnKeyLogged));

    login_stack_.Start();
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
    home_stack_.PersonLogg(person);
}

// TODO: implementar devolución de llave por NFC (navegar a SolenoidPanel).
void Window::OnKeyLogged(std::shared_ptr<kdb::Key> key) {
    (void)key;
}
