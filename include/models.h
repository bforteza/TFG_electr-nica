#ifndef MODELS_H
#define MODELS_H

#include <gtkmm/treemodel.h>
#include <glibmm/ustring.h>
#include <string>

// Columnas del TreeView de usuarios (UsersViewStack).
class ModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    ModelColumns() {
        add(name_col);
        add(password_col);
        add(id_col);
        add(uid_col);
    }

    Gtk::TreeModelColumn<std::string> name_col;
    Gtk::TreeModelColumn<std::string> password_col;
    Gtk::TreeModelColumn<int>         id_col;
    Gtk::TreeModelColumn<std::string> uid_col;
};

// Columnas del TreeView de llaves (KeyViewStack).
class KeyModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    KeyModelColumns() {
        add(name_col);
        add(ubi_col);
        add(keeper_col);
        add(commentary_col);
        add(pos_col);
        add(active_col);
        add(id_col);
    }

    Gtk::TreeModelColumn<Glib::ustring> name_col;
    Gtk::TreeModelColumn<std::string>   ubi_col;
    Gtk::TreeModelColumn<std::string>   keeper_col;
    Gtk::TreeModelColumn<std::string>   commentary_col;
    Gtk::TreeModelColumn<std::string>   pos_col;
    Gtk::TreeModelColumn<bool>          active_col;
    Gtk::TreeModelColumn<int>           id_col;
};

// Columnas del TreeView del historial (HistoryViewStack).
class HistoryModelColumns : public Gtk::TreeModel::ColumnRecord {
public:
    HistoryModelColumns() {
        add(timestamp_col);
        add(event_col);
        add(person_col);
        add(key_col);
        add(pos_col);
        add(id_col);
    }

    Gtk::TreeModelColumn<std::string> timestamp_col;
    Gtk::TreeModelColumn<std::string> event_col;
    Gtk::TreeModelColumn<std::string> person_col;
    Gtk::TreeModelColumn<std::string> key_col;
    Gtk::TreeModelColumn<std::string> pos_col;
    Gtk::TreeModelColumn<int>         id_col;
};

#endif // MODELS_H
