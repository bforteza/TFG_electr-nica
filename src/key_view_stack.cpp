#include "key_view_stack.h"
#include "globals.h"

KeyViewStack::KeyViewStack(const Glib::RefPtr<Gtk::Builder>& builder) {
    builder->get_widget("AddUserToKeyButton", add_user_button_);
    if (!add_user_button_)
        throw std::runtime_error("No \"AddUserToKeyButton\" object in MainWindow.glade");
    add_user_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnAddUserButtonClicked));

    builder->get_widget("RemoveUserToKeyButton", remove_user_button_);
    if (!remove_user_button_)
        throw std::runtime_error("No \"RemoveUserToKeyButton\" object in MainWindow.glade");
    remove_user_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnRemoveUserButtonClicked));

    builder->get_widget("SelectKeyButton", select_key_button_);
    if (!select_key_button_)
        throw std::runtime_error("No \"SelectKeyButton\" object in MainWindow.glade");
    select_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnSelectKeyButtonClicked));

    builder->get_widget("EditKeyButton", edit_key_button_);
    if (!edit_key_button_)
        throw std::runtime_error("No \"EditKeyButton\" object in MainWindow.glade");
    edit_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnEditKeyButtonClicked));

    builder->get_widget("KeepKeyButton", keep_key_button_);
    if (!keep_key_button_)
        throw std::runtime_error("No \"KeepKeyButton\" object in MainWindow.glade");
    keep_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnKeepKeyButtonClicked));

    builder->get_widget("ViewUsersOfKeyButton", view_users_button_);
    if (!view_users_button_)
        throw std::runtime_error("No \"ViewUsersOfKeyButton\" object in MainWindow.glade");
    view_users_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnViewUsersButtonClicked));

    builder->get_widget("DeleteKeyButton", delete_key_button_);
    if (!delete_key_button_)
        throw std::runtime_error("No \"DeleteKeyButton\" object in MainWindow.glade");
    delete_key_button_->signal_clicked().connect(
        sigc::mem_fun(*this, &KeyViewStack::OnDeleteKeyButtonClicked));

    builder->get_widget("ViewKeysTree", keys_tree_view_);
    if (!keys_tree_view_)
        throw std::runtime_error("No \"ViewKeysTree\" object in MainWindow.glade");

    tree_model_ = Gtk::ListStore::create(columns_);
    keys_tree_view_->set_model(tree_model_);

    // Las cadenas de cabecera se establecen en RefreshLabels(); aquí solo se añaden las columnas.
    keys_tree_view_->append_column("Id",          columns_.id_col);
    keys_tree_view_->append_column("",            columns_.name_col);
    keys_tree_view_->append_column("",            columns_.ubi_col);
    keys_tree_view_->append_column("",            columns_.commentary_col);
    keys_tree_view_->append_column("",            columns_.pos_col);
    keys_tree_view_->append_column("",            columns_.active_col);
    keys_tree_view_->append_column("",            columns_.keeper_col);

    id_column_          = keys_tree_view_->get_column(0);
    name_column_        = keys_tree_view_->get_column(1);
    ubi_column_         = keys_tree_view_->get_column(2);
    commentary_column_  = keys_tree_view_->get_column(3);
    pos_column_         = keys_tree_view_->get_column(4);
    active_column_      = keys_tree_view_->get_column(5);
    keeper_column_      = keys_tree_view_->get_column(6);

    keys_tree_view_->set_enable_search(true);
    keys_tree_view_->set_search_column(columns_.name_col);

    // Suscribe RefreshLabels al cambio de idioma global.
    language_changed.connect(sigc::mem_fun(*this, &KeyViewStack::RefreshLabels));
    RefreshLabels();
}

void KeyViewStack::RefreshLabels() {
    add_user_button_->set_label(Tr().key_view.btn_add_user);
    remove_user_button_->set_label(Tr().key_view.btn_remove_user);
    select_key_button_->set_label(Tr().key_view.btn_select);
    edit_key_button_->set_label(Tr().key_view.btn_edit);
    keep_key_button_->set_label(Tr().key_view.btn_take);
    view_users_button_->set_label(Tr().key_view.btn_view_users);
    delete_key_button_->set_label(Tr().key_view.btn_delete);

    name_column_->set_title(Tr().key_view.col_name);
    ubi_column_->set_title(Tr().key_view.col_location);
    commentary_column_->set_title(Tr().key_view.col_comments);
    pos_column_->set_title(Tr().key_view.col_position);
    active_column_->set_title(Tr().key_view.col_active);
    keeper_column_->set_title(Tr().key_view.col_keeper);
}

// --- Iniciadores públicos ---

void KeyViewStack::view(std::shared_ptr<kdb::Person> person) {
    select_key_button_->hide();
    access_ = (int)person->a1 + 2 * (int)person->a2;
    Configure();
    if (access_ >= 2)
        current_keys_ = litesql::select<kdb::Key>(*db).all();
    else
        current_keys_ = person->keys().get().all();
    Refresh();
}

void KeyViewStack::view(std::vector<kdb::Key> keys) {
    select_key_button_->hide();
    Configure();
    current_keys_ = keys;
    Refresh();
}

void KeyViewStack::select(std::vector<kdb::Key> keys) {
    select_key_button_->show();
    add_user_button_->hide();
    remove_user_button_->hide();
    edit_key_button_->hide();
    current_keys_ = keys;
    Refresh();
}

// --- Auxiliares internos ---

int KeyViewStack::GetSelectionId() {
    auto sel = keys_tree_view_->get_selection();
    if (auto iter = sel->get_selected())
        return (*iter)[columns_.id_col];
    return 0;
}

std::shared_ptr<kdb::Key> KeyViewStack::GetSelectedKey() {
    auto sel = keys_tree_view_->get_selection();
    if (auto iter = sel->get_selected())
        return std::make_shared<kdb::Key>(
            litesql::select<kdb::Key>(*db, kdb::Key::Id == (*iter)[columns_.id_col]).one());
    return nullptr;
}

void KeyViewStack::Refresh() {
    tree_model_->clear();
    for (auto& key : current_keys_) {
        Gtk::TreeModel::Row row = *(tree_model_->append());
        row[columns_.id_col]          = key.id;
        row[columns_.name_col]        = (Glib::ustring)key.name;
        row[columns_.ubi_col]         = key.ubi;
        row[columns_.commentary_col]  = key.commentary;
        row[columns_.pos_col]         = PosToString(key.pos);
        row[columns_.active_col]      = key.active;
        try {
            row[columns_.keeper_col] = (Glib::ustring)key.keeper().get().one().name;
        } catch (...) {}
    }
    ;
}

void KeyViewStack::Configure() {
    if (access_ >= 0) {
        keep_key_button_->show();
        id_column_->set_visible(false);
        active_column_->set_visible(false);
    }
    if (access_ >= 1) {
        add_user_button_->show();
        remove_user_button_->show();
        edit_key_button_->show();
        delete_key_button_->show();
        view_users_button_->show();
    }
    keys_tree_view_->columns_autosize();
}

// --- Manejadores de botones ---

void KeyViewStack::OnSelectKeyButtonClicked() {
    int id = GetSelectionId();
    if (id)
        key_selected.emit(
            std::make_shared<kdb::Key>(litesql::select<kdb::Key>(*db, kdb::Key::Id == id).one()));
}

void KeyViewStack::OnEditKeyButtonClicked() {
    auto key = GetSelectedKey();
    if (key)
        key_edit.emit(key);
}

void KeyViewStack::OnAddUserButtonClicked() {
    auto key = GetSelectedKey();
    if (key)
        key_link.emit(key);
}

void KeyViewStack::OnRemoveUserButtonClicked() {
    auto key = GetSelectedKey();
    if (key)
        key_unlink.emit(key);
}

void KeyViewStack::OnViewUsersButtonClicked() {
    auto key = GetSelectedKey();
    if (key)
        users_view.emit(key->owners().get().all());
}

void KeyViewStack::OnDeleteKeyButtonClicked() {
    // TODO: implementar eliminación de llave.
}

void KeyViewStack::OnKeepKeyButtonClicked() {
    auto key = GetSelectedKey();
    if (key)
        key_kept.emit(key);
}
