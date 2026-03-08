#ifndef MODELS_H
#define MODELS_H

#include <gtkmm/treemodel.h>
#include <string>

class ModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    ModelColumns()
    { add(m_col_name); add(m_col_password);add(IdCol); add(UidCol);}

    Gtk::TreeModelColumn<std::string> m_col_name;
    Gtk::TreeModelColumn<std::string> m_col_password;
    Gtk::TreeModelColumn<int> IdCol;
    Gtk::TreeModelColumn<std::string> UidCol;
  };


class KeyModelColumns : public Gtk::TreeModel::ColumnRecord
  {
  public:

    KeyModelColumns()
    { add(NameCol); add(UbiCol); add(KeeperCol);  add(CommentaryCol); add(PosCol); add(ActiveCol);add(IdCol);}

    Gtk::TreeModelColumn<Glib::ustring> NameCol;
    Gtk::TreeModelColumn<std::string> UbiCol;
    Gtk::TreeModelColumn<std::string> KeeperCol;
    Gtk::TreeModelColumn<std::string> CommentaryCol;
    Gtk::TreeModelColumn<std::string> PosCol;
    Gtk::TreeModelColumn<bool> ActiveCol;
    Gtk::TreeModelColumn<int> IdCol;
  };

#endif // MODELS_H
