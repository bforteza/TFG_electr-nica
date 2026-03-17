#include "users_view_stack.h"
#include "globals.h"

UsersViewStack::UsersViewStack(const Glib::RefPtr<Gtk::Builder>& builder) {
    builder->get_widget("AddKeyToUserButton", add_key_button_);
    if (!add_key_button_)
        throw std::runtime_error("No \"AddKeyToUserButton\" object in MainWindow.glade");
    add_key_button_->hide();
    add_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::OnAddKeyButtonClicked));

    builder->get_widget("RemoveKeyToUserButton", remove_key_button_);
    if (!remove_key_button_)
        throw std::runtime_error("No \"RemoveKeyToUserButton\" object in MainWindow.glade");
    remove_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::OnRemoveKeyButtonClicked));

    builder->get_widget("SelectUserButton", select_user_button_);
    if (!select_user_button_)
        throw std::runtime_error("No \"SelectUserButton\" object in MainWindow.glade");
    select_user_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::OnSelectUserButtonClicked));

    builder->get_widget("UserEditButton", edit_button_);
    if (!edit_button_)
        throw std::runtime_error("No \"UserEditButton\" object in MainWindow.glade");
    edit_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::OnEditButtonClicked));

    builder->get_widget("ViewKeysOfUserButton", view_keys_button_);
    if (!view_keys_button_)
        throw std::runtime_error("No \"ViewKeysOfUserButton\" object in MainWindow.glade");
    view_keys_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::OnViewKeysButtonClicked));

    builder->get_widget("HistoryPersonButton", history_person_button_);
    if (!history_person_button_)
        throw std::runtime_error("No \"HistoryPersonButton\" object in MainWindow.glade");
    history_person_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::OnHistoryPersonButtonClicked));

    builder->get_widget("ViewUsersTree", users_tree_view_);
    if (!users_tree_view_)
        throw std::runtime_error("No \"ViewUsersTree\" object in MainWindow.glade");

    tree_model_ = Gtk::ListStore::create(columns_);
    users_tree_view_->set_model(tree_model_);

    // Las cadenas de cabecera se establecen en RefreshLabels(); aquí solo se añaden las columnas.
    users_tree_view_->append_column("Id",  columns_.id_col);
    users_tree_view_->append_column("",    columns_.name_col);
    users_tree_view_->append_column("",    columns_.password_col);
    users_tree_view_->append_column("",    columns_.uid_col);

    id_column_       = users_tree_view_->get_column(0);
    name_column_     = users_tree_view_->get_column(1);
    password_column_ = users_tree_view_->get_column(2);
    uid_column_      = users_tree_view_->get_column(3);

    id_column_->set_visible(false);

    // Suscribe RefreshLabels al cambio de idioma global.
    language_changed.connect(sigc::mem_fun(*this, &UsersViewStack::RefreshLabels));
    RefreshLabels();
}

void UsersViewStack::RefreshLabels() {
    add_key_button_->set_label(Tr().users_view.btn_add_key);
    remove_key_button_->set_label(Tr().users_view.btn_remove_key);
    select_user_button_->set_label(Tr().users_view.btn_select);
    edit_button_->set_label(Tr().users_view.btn_edit);
    view_keys_button_->set_label(Tr().users_view.btn_view_keys);
    history_person_button_->set_label(Tr().history_view.btn_history_person);

    name_column_->set_title(Tr().users_view.col_name);
    password_column_->set_title(Tr().users_view.col_password);
    uid_column_->set_title(Tr().users_view.col_uid);
}

// --- Iniciadores públicos ---

void UsersViewStack::view(std::vector<kdb::Person> users, int access) {
    toggle_conn_.disconnect();
    users_tree_view_->get_selection()->set_mode(Gtk::SELECTION_SINGLE);
    password_column_->set_visible(access >= 2);
    uid_column_->set_visible(access >= 2);
    access_ = access;
    add_key_button_->show();
    remove_key_button_->show();
    view_keys_button_->show();
    select_user_button_->hide();

    // Editar usuario e historial son solo para administradores.
    if (access_ >= 2) {
        edit_button_->show();
        history_person_button_->show();
    } else {
        edit_button_->hide();
        history_person_button_->hide();
    }

    current_users_ = users;
    Refresh();
}

void UsersViewStack::select(std::vector<kdb::Person> users) {
    users_tree_view_->get_selection()->set_mode(Gtk::SELECTION_MULTIPLE);
    toggle_conn_ = users_tree_view_->signal_button_press_event().connect([this](GdkEventButton* ev) -> bool {
        Gtk::TreePath path;
        Gtk::TreeViewColumn* col;
        int cx, cy;
        if (users_tree_view_->get_path_at_pos((int)ev->x, (int)ev->y, path, col, cx, cy)) {
            auto sel = users_tree_view_->get_selection();
            if (sel->is_selected(path)) sel->unselect(path);
            else                        sel->select(path);
        }
        return true;
    }, false);
    password_column_->set_visible(false);
    uid_column_->set_visible(false);
    select_user_button_->show();
    add_key_button_->hide();
    remove_key_button_->hide();
    edit_button_->hide();
    view_keys_button_->hide();
    history_person_button_->hide();

    current_users_ = users;
    Refresh();
}

// --- Auxiliares internos ---

int UsersViewStack::GetSelectionId() {
    auto sel = users_tree_view_->get_selection();
    if (auto iter = sel->get_selected())
        return (*iter)[columns_.id_col];
    return 0;
}

std::shared_ptr<kdb::Person> UsersViewStack::GetSelectedPerson() {
    auto sel = users_tree_view_->get_selection();
    if (auto iter = sel->get_selected())
        return std::make_shared<kdb::Person>(
            litesql::select<kdb::Person>(*db, kdb::Person::Id == (*iter)[columns_.id_col]).one());
    return nullptr;
}

std::vector<std::shared_ptr<kdb::Person>> UsersViewStack::GetSelectedPersons() {
    std::vector<std::shared_ptr<kdb::Person>> result;
    for (auto& path : users_tree_view_->get_selection()->get_selected_rows()) {
        auto iter = tree_model_->get_iter(path);
        if (iter) {
            try {
                result.push_back(std::make_shared<kdb::Person>(
                    litesql::select<kdb::Person>(*db, kdb::Person::Id == (*iter)[columns_.id_col]).one()));
            } catch (...) {}
        }
    }
    return result;
}

void UsersViewStack::Refresh() {
    tree_model_->clear();
    for (auto& person : current_users_) {
        Gtk::TreeModel::Row row = *(tree_model_->append());
        row[columns_.id_col]        = (int)person.id;
        row[columns_.name_col]      = person.name;
        row[columns_.password_col]  = person.password;
        row[columns_.uid_col]       = person.uid;
    }
}

// --- Manejadores de botones ---

void UsersViewStack::OnAddKeyButtonClicked() {
    int id = GetSelectionId();
    if (id)
        user_link.emit(
            std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db, kdb::Person::Id == id).one()));
}

void UsersViewStack::OnRemoveKeyButtonClicked() {
    int id = GetSelectionId();
    if (id)
        user_unlink.emit(
            std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db, kdb::Person::Id == id).one()));
}

void UsersViewStack::OnViewKeysButtonClicked() {
    int id = GetSelectionId();
    if (id)
        keys_view.emit(
            litesql::select<kdb::Person>(*db, kdb::Person::Id == id).one().keys().get().all());
}

void UsersViewStack::OnEditButtonClicked() {
    auto person = GetSelectedPerson();
    if (person)
        user_edit.emit(person);
}

void UsersViewStack::OnSelectUserButtonClicked() {
    auto persons = GetSelectedPersons();
    if (!persons.empty())
        user_selected.emit(persons);
}

void UsersViewStack::OnHistoryPersonButtonClicked() {
    auto person = GetSelectedPerson();
    if (person)
        person_history.emit(person);
}
