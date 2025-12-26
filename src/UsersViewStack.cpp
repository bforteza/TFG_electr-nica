#include "UsersViewStack.h"
#include "Globals.h"



UsersViewStack::UsersViewStack(const Glib::RefPtr<Gtk::Builder>& builder)
{
    //Buttons
    builder->get_widget("AddKeyToUserButton", AddKeyToUserButton);
    if (!AddKeyToUserButton) {
        throw std::runtime_error("No \"AddKeyToUserButton\" object in INI.glade" );
    }
    AddKeyToUserButton->hide();
    AddKeyToUserButton->signal_clicked().connect(sigc::mem_fun(*this, &UsersViewStack::on_AddKeyToUserButton_clicked));


    builder->get_widget("RemoveKeyToUserButton", RemoveKeyToUserButton);
    if (!RemoveKeyToUserButton) {
        throw std::runtime_error("No \"RemoveKeyToUserButton\" object in INI.glade" );
    }
    RemoveKeyToUserButton->signal_clicked().connect(sigc::mem_fun(*this, &UsersViewStack::on_RemoveKeyToUserButton_clicked));

    builder->get_widget("SelectUserButton", SelectUserButton);
    if (!SelectUserButton) {
        throw std::runtime_error("No \"SelectUserButton\" object in INI.glade" );
    }
    SelectUserButton->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::on_SelectUserButton_clicked));

    builder->get_widget("UserEditButton", UserEditButton);
    if (!UserEditButton) {
        throw std::runtime_error("No \"UserEditButton\" object in INI.glade" );
    }

    UserEditButton->signal_clicked().connect(
        sigc::mem_fun(*this, &UsersViewStack::on_UserEditButton_clicked));

    builder->get_widget("ViewKeysOfUserButton", ViewKeysOfUserButton);
    if (!ViewKeysOfUserButton) {
        throw std::runtime_error("No \"ViewKeysOfUserButton\" object in INI.glade" );
    }
    ViewKeysOfUserButton->signal_clicked().connect(sigc::mem_fun(*this, &UsersViewStack::on_ViewKeysOfUserButton_clicked));

    //view
    builder->get_widget("ViewUsersTree", ViewUsersTree);
    if (!ViewUsersTree) {
        throw std::runtime_error("No \"ViewUsersTree\" object in INI.glade" );
    }

    m_refTreeModel = Gtk::ListStore::create(m_Columns);
 // ViewUsersTree->set_model(m_refTreeModel);

  ViewUsersTree->set_model(m_refTreeModel);
  //Fill the TreeView's model
  ViewUsersTree->append_column("Id",m_Columns.IdCol);
  ViewUsersTree->append_column("Name", m_Columns.m_col_name);
  ViewUsersTree->append_column("Password", m_Columns.m_col_password);
  ViewUsersTree->append_column("Uid", m_Columns.UidCol);





}

//Function initiators

void UsersViewStack::view(std::vector<kdb::Person> i){

    AddKeyToUserButton->show();
    RemoveKeyToUserButton->show();
    UserEditButton->show();
    ViewKeysOfUserButton->show();

    SelectUserButton->hide();

    actual = i;
    actualizar();
}

void UsersViewStack::select(std::vector<kdb::Person> i){
    SelectUserButton->show();

    AddKeyToUserButton->hide();
    RemoveKeyToUserButton->hide();
    UserEditButton->hide();
    ViewKeysOfUserButton->hide();

    actual = i;
    actualizar();
}

//Auxiliar function

int UsersViewStack::get_selection(){
    auto sel = ViewUsersTree->get_selection();
    if(auto iter = sel->get_selected())
        return (*iter)[m_Columns.IdCol];
    return 0;
}

std::shared_ptr<kdb::Person> UsersViewStack::get_selection_ptr(){
    auto sel = ViewUsersTree->get_selection();
    if(auto iter = sel->get_selected())
        return std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db,kdb::Person::Id ==  (*iter)[m_Columns.IdCol]).one());
    return nullptr;
}
//Refresh treeview with the data from vector actual
void UsersViewStack::actualizar(){
    m_refTreeModel->clear();
    for(auto iter : actual ){
        Gtk::TreeModel::Row row = *(m_refTreeModel->append());

        row[m_Columns.IdCol] = (int) iter.id;
        row[m_Columns.m_col_name]= iter.name;
        row[m_Columns.UidCol]= iter.uid;
        row[m_Columns.m_col_password] = iter.password;
      }
}

//Button actions

void UsersViewStack::on_AddKeyToUserButton_clicked(){
    int aux=get_selection();
    if (aux)
    UserLink.emit(std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db,kdb::Person::Id == aux).one()));
}

void UsersViewStack::on_RemoveKeyToUserButton_clicked(){
    int aux=get_selection();
    if (aux)
    UserUnLink.emit(std::make_shared<kdb::Person>(litesql::select<kdb::Person>(*db,kdb::Person::Id == aux).one()));
}

void UsersViewStack::on_ViewKeysOfUserButton_clicked(){
     int aux=get_selection();
    if (aux)
    KeysView.emit((litesql::select<kdb::Person>(*db,kdb::Person::Id == aux).one()).keys().get().all());
}

void UsersViewStack::on_UserEditButton_clicked(){
    std::shared_ptr<kdb::Person> aux = get_selection_ptr();
    if (aux != nullptr){
        UserEdit.emit(aux);
    }
}

void UsersViewStack::on_SelectUserButton_clicked(){
    std::shared_ptr<kdb::Person> aux = get_selection_ptr();
    if (aux != nullptr){
        UserSelected.emit(aux);
    }
}
