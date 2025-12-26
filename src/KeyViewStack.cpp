#include "KeyViewStack.h"
#include "Globals.h"
KeyViewStack::KeyViewStack(const Glib::RefPtr<Gtk::Builder>& builder)
{
    //Buttons
    builder->get_widget("AddUserToKeyButton", AddUserToKeyButton);
    if (!AddUserToKeyButton) {
        throw std::runtime_error("No \"AddUserToKeyButton\" object in INI.glade" );
    }
    AddUserToKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_AddUserToKeyButton_clicked));


    builder->get_widget("RemoveUserToKeyButton", RemoveUserToKeyButton);
    if (!RemoveUserToKeyButton) {
        throw std::runtime_error("No \"RemoveUserToKeyButton\" object in INI.glade" );
    }
    RemoveUserToKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_RemoveUserToKeyButton_clicked));

    builder->get_widget("SelectKeyButton", SelectKeyButton);
    if (!SelectKeyButton) {
        throw std::runtime_error("No \"SelectKeyButton\" object in INI.glade" );
    }

    SelectKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_SelectKeyButton_clicked));

    builder->get_widget("EditKeyButton", EditKeyButton);
    if (!EditKeyButton) {
        throw std::runtime_error("No \"EditKeyButton\" object in INI.glade" );
    }
    EditKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_EditKeyButton_clicked));

    builder->get_widget("KeepKeyButton", KeepKeyButton);
    if (!KeepKeyButton) {
        throw std::runtime_error("No \"KeepKeyButton\" object in INI.glade" );
    }
    KeepKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_KeepKeyButton_clicked));

    builder->get_widget("ViewUsersOfKeyButton", ViewUsersOfKeyButton);
    if (!ViewUsersOfKeyButton) {
        throw std::runtime_error("No \"ViewUsersOfKeyButton\" object in INI.glade" );
    }
    ViewUsersOfKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_ViewUsersOfKeyButton_clicked));

    builder->get_widget("DeleteKeyButton", DeleteKeyButton);
    if (!DeleteKeyButton) {
        throw std::runtime_error("No \"DeleteKeyButton\" object in INI.glade" );
    }
    DeleteKeyButton->signal_clicked().connect(sigc::mem_fun(*this, &KeyViewStack::on_DeleteKeyButton_clicked));



    //view
    builder->get_widget("ViewKeysTree", ViewKeysTree);
    if (!ViewKeysTree) {
        throw std::runtime_error("No \"ViewKeysTree\" object in INI.glade" );
    }

    m_refTreeModel = Gtk::ListStore::create(m_Columns);
 // ViewUsersTree->set_model(m_refTreeModel);

  ViewKeysTree->set_model(m_refTreeModel);
  //Fill the TreeView's model

  ViewKeysTree->append_column("Id", m_Columns.IdCol);
  ViewKeysTree->append_column("Nom", m_Columns.NameCol);
  ViewKeysTree->append_column("Ubicació", m_Columns.UbiCol);
  ViewKeysTree->append_column("Comentaris", m_Columns.CommentaryCol);
  ViewKeysTree->append_column("Posició", m_Columns.PosCol);
  ViewKeysTree->append_column("Activa", m_Columns.ActiveCol);
  ViewKeysTree->append_column("Agafada", m_Columns.KeeperCol);

  IdColumn = ViewKeysTree->get_column(0);
  NameColumn = ViewKeysTree->get_column(1);
  UbiColumn = ViewKeysTree->get_column(2);
  CommentaryColumn = ViewKeysTree->get_column(3);
  PosColumn = ViewKeysTree->get_column(4);
  ActiveColumn = ViewKeysTree->get_column(5);
  KeeperColumn = ViewKeysTree->get_column(6);

  ViewKeysTree->set_enable_search(true);
  ViewKeysTree->set_search_column(m_Columns.NameCol);

}

// Public initiators

void KeyViewStack::view(std::shared_ptr<kdb::Person> InPerson){
    SelectKeyButton->hide();
    acces = (int) InPerson->a1 + 2 * (int)InPerson->a2;
    configure();
    if (acces == 0){
    }
     if (acces==1){
        actual = InPerson->keys().get().all();
    }
     if (acces==2){
        actual = litesql::select<kdb::Key>(*db).all();
    }
    actualizar();
}

void KeyViewStack::view(std::vector<kdb::Key> i){
    SelectKeyButton->hide();
    configure();

    actual = i;
    actualizar();
}

void KeyViewStack::select(std::vector<kdb::Key> i){
    SelectKeyButton->show();

    AddUserToKeyButton->hide();
    RemoveUserToKeyButton->hide();
    EditKeyButton->hide();


    actual = i;
    actualizar();
}


// Auxiliar functions

int KeyViewStack::get_selection(){
    auto sel = ViewKeysTree->get_selection();
    if(auto iter = sel->get_selected())
        return (*iter)[m_Columns.IdCol];
    return 0;
}

std::shared_ptr<kdb::Key> KeyViewStack::get_selection_ptr(){
    auto sel = ViewKeysTree->get_selection();
    if(auto iter = sel->get_selected())
        return std::make_shared<kdb::Key>(litesql::select<kdb::Key>(*db,kdb::Key::Id ==  (*iter)[m_Columns.IdCol]).one());
    return nullptr;
}

void KeyViewStack::actualizar(){
    m_refTreeModel->clear();
    for(auto iter : actual ){
        Gtk::TreeModel::Row row = *(m_refTreeModel->append());
         row[m_Columns.NameCol]= (Glib::ustring) iter.name;
         row[m_Columns.IdCol]= iter.id;
         row[m_Columns.UbiCol] = iter.ubi;
         row[m_Columns.CommentaryCol] = iter.commentary;
         row[m_Columns.PosCol] = to_string(iter.pos);
         row[m_Columns.ActiveCol] = iter.active;
         try{
            row[m_Columns.KeeperCol] = (Glib::ustring) iter.keeper().get().one().name;
         }catch(...){}

      }

}

//          Button actions

void KeyViewStack::on_SelectKeyButton_clicked(){
    int aux = get_selection();
    if (aux){
        KeySelected.emit(std::make_shared<kdb::Key>(litesql::select<kdb::Key>(*db,kdb::Key::Id == aux).one()));
    }
}

void KeyViewStack::on_EditKeyButton_clicked(){
    std::shared_ptr<kdb::Key> aux = get_selection_ptr();
    if (aux != nullptr){
        KeyEdit.emit(aux);
    }
}

void KeyViewStack::on_AddUserToKeyButton_clicked(){
    std::shared_ptr<kdb::Key> aux = get_selection_ptr();
    if (aux != nullptr){
        KeyLink.emit(aux);
    }
}

void KeyViewStack::on_RemoveUserToKeyButton_clicked(){
    std::shared_ptr<kdb::Key> aux = get_selection_ptr();
    if (aux != nullptr){
        KeyUnLink.emit(aux);
    }
}

void KeyViewStack::on_ViewUsersOfKeyButton_clicked(){
    std::shared_ptr<kdb::Key> aux = get_selection_ptr();
    if (aux != nullptr){
        UsersView.emit(aux->owners().get().all());
    }
}

void KeyViewStack::on_DeleteKeyButton_clicked(){

}

void KeyViewStack::on_KeepKeyButton_clicked(){
    std::shared_ptr<kdb::Key> aux = get_selection_ptr();
    if (aux != nullptr){
        KeyKeeped.emit(aux);
    }
}

bool KeyViewStack::configure(){
   if (acces >= 0){
        KeepKeyButton->show();
        IdColumn->set_visible(false);
        ActiveColumn->set_visible(false);
    }
     if (acces>=1){
        AddUserToKeyButton->show();
        RemoveUserToKeyButton->show();
        EditKeyButton->show();
        DeleteKeyButton->show();
        ViewUsersOfKeyButton->show();
    }
     if (acces>=2){

    }
        ViewKeysTree->columns_autosize();
}
