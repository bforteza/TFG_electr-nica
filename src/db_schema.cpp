#include "db_schema.hpp"
namespace kdb {
using namespace litesql;
KeyPersonRelationAcces::Row::Row(const litesql::Database& db, const litesql::Record& rec)
         : person(KeyPersonRelationAcces::Person), key(KeyPersonRelationAcces::Key) {
    switch(rec.size()) {
    case 2:
        person = rec[1];
    case 1:
        key = rec[0];
    }
}
const std::string KeyPersonRelationAcces::table__("Key_Person_Acces");
const litesql::FieldType KeyPersonRelationAcces::Key("Key1_",A_field_type_integer,table__);
const litesql::FieldType KeyPersonRelationAcces::Person("Person2_",A_field_type_integer,table__);
void KeyPersonRelationAcces::link(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1) {
    Record values;
    Split fields;
    fields.push_back(Key.name());
    values.push_back(o0.id);
    fields.push_back(Person.name());
    values.push_back(o1.id);
    db.insert(table__, values, fields);
}
void KeyPersonRelationAcces::unlink(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1) {
    db.delete_(table__, (Key == o0.id && Person == o1.id));
}
void KeyPersonRelationAcces::del(const litesql::Database& db, const litesql::Expr& expr) {
    db.delete_(table__, expr);
}
litesql::DataSource<KeyPersonRelationAcces::Row> KeyPersonRelationAcces::getRows(const litesql::Database& db, const litesql::Expr& expr) {
    SelectQuery sel;
    sel.result(Key.fullName());
    sel.result(Person.fullName());
    sel.source(table__);
    sel.where(expr);
    return DataSource<KeyPersonRelationAcces::Row>(db, sel);
}
template <> litesql::DataSource<kdb::Key> KeyPersonRelationAcces::get(const litesql::Database& db, const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    SelectQuery sel;
    sel.source(table__);
    sel.result(Key.fullName());
    sel.where(srcExpr);
    return DataSource<kdb::Key>(db, kdb::Key::Id.in(sel) && expr);
}
template <> litesql::DataSource<kdb::Person> KeyPersonRelationAcces::get(const litesql::Database& db, const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    SelectQuery sel;
    sel.source(table__);
    sel.result(Person.fullName());
    sel.where(srcExpr);
    return DataSource<kdb::Person>(db, kdb::Person::Id.in(sel) && expr);
}
KeyPersonRelationKeep::Row::Row(const litesql::Database& db, const litesql::Record& rec)
         : person(KeyPersonRelationKeep::Person), key(KeyPersonRelationKeep::Key) {
    switch(rec.size()) {
    case 2:
        person = rec[1];
    case 1:
        key = rec[0];
    }
}
const std::string KeyPersonRelationKeep::table__("Key_Person_Keep");
const litesql::FieldType KeyPersonRelationKeep::Key("Key1_",A_field_type_integer,table__);
const litesql::FieldType KeyPersonRelationKeep::Person("Person2_",A_field_type_integer,table__);
void KeyPersonRelationKeep::link(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1) {
    Record values;
    Split fields;
    fields.push_back(Key.name());
    values.push_back(o0.id);
    fields.push_back(Person.name());
    values.push_back(o1.id);
    db.insert(table__, values, fields);
}
void KeyPersonRelationKeep::unlink(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1) {
    db.delete_(table__, (Key == o0.id && Person == o1.id));
}
void KeyPersonRelationKeep::del(const litesql::Database& db, const litesql::Expr& expr) {
    db.delete_(table__, expr);
}
litesql::DataSource<KeyPersonRelationKeep::Row> KeyPersonRelationKeep::getRows(const litesql::Database& db, const litesql::Expr& expr) {
    SelectQuery sel;
    sel.result(Key.fullName());
    sel.result(Person.fullName());
    sel.source(table__);
    sel.where(expr);
    return DataSource<KeyPersonRelationKeep::Row>(db, sel);
}
template <> litesql::DataSource<kdb::Key> KeyPersonRelationKeep::get(const litesql::Database& db, const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    SelectQuery sel;
    sel.source(table__);
    sel.result(Key.fullName());
    sel.where(srcExpr);
    return DataSource<kdb::Key>(db, kdb::Key::Id.in(sel) && expr);
}
template <> litesql::DataSource<kdb::Person> KeyPersonRelationKeep::get(const litesql::Database& db, const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    SelectQuery sel;
    sel.source(table__);
    sel.result(Person.fullName());
    sel.where(srcExpr);
    return DataSource<kdb::Person>(db, kdb::Person::Id.in(sel) && expr);
}
const litesql::FieldType Person::Own::Id("id_",A_field_type_integer,"Person_");
Person::KeysHandle::KeysHandle(const Person& owner)
         : litesql::RelationHandle<Person>(owner) {
}
void Person::KeysHandle::link(const Key& o0) {
    KeyPersonRelationAcces::link(owner->getDatabase(), o0, *owner);
}
void Person::KeysHandle::unlink(const Key& o0) {
    KeyPersonRelationAcces::unlink(owner->getDatabase(), o0, *owner);
}
void Person::KeysHandle::del(const litesql::Expr& expr) {
    KeyPersonRelationAcces::del(owner->getDatabase(), expr && KeyPersonRelationAcces::Person == owner->id);
}
litesql::DataSource<Key> Person::KeysHandle::get(const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    return KeyPersonRelationAcces::get<Key>(owner->getDatabase(), expr, (KeyPersonRelationAcces::Person == owner->id) && srcExpr);
}
litesql::DataSource<KeyPersonRelationAcces::Row> Person::KeysHandle::getRows(const litesql::Expr& expr) {
    return KeyPersonRelationAcces::getRows(owner->getDatabase(), expr && (KeyPersonRelationAcces::Person == owner->id));
}
Person::KeepkeysHandle::KeepkeysHandle(const Person& owner)
         : litesql::RelationHandle<Person>(owner) {
}
void Person::KeepkeysHandle::link(const Key& o0) {
    KeyPersonRelationKeep::link(owner->getDatabase(), o0, *owner);
}
void Person::KeepkeysHandle::unlink(const Key& o0) {
    KeyPersonRelationKeep::unlink(owner->getDatabase(), o0, *owner);
}
void Person::KeepkeysHandle::del(const litesql::Expr& expr) {
    KeyPersonRelationKeep::del(owner->getDatabase(), expr && KeyPersonRelationKeep::Person == owner->id);
}
litesql::DataSource<Key> Person::KeepkeysHandle::get(const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    return KeyPersonRelationKeep::get<Key>(owner->getDatabase(), expr, (KeyPersonRelationKeep::Person == owner->id) && srcExpr);
}
litesql::DataSource<KeyPersonRelationKeep::Row> Person::KeepkeysHandle::getRows(const litesql::Expr& expr) {
    return KeyPersonRelationKeep::getRows(owner->getDatabase(), expr && (KeyPersonRelationKeep::Person == owner->id));
}
const std::string Person::type__("Person");
const std::string Person::table__("Person_");
const std::string Person::sequence__("Person_seq");
const litesql::FieldType Person::Id("id_",A_field_type_integer,table__);
const litesql::FieldType Person::Type("type_",A_field_type_string,table__);
const litesql::FieldType Person::Name("name_",A_field_type_string,table__);
const litesql::FieldType Person::Password("password_",A_field_type_string,table__);
const litesql::FieldType Person::Uid("uid_",A_field_type_string,table__);
const litesql::FieldType Person::A1("a1_",A_field_type_boolean,table__);
const litesql::FieldType Person::A2("a2_",A_field_type_boolean,table__);
void Person::initValues() {
}
void Person::defaults() {
    id = 0;
    a1 = 0;
    a2 = 0;
}
Person::Person(const litesql::Database& db)
     : litesql::Persistent(db), id(Id), type(Type), name(Name), password(Password), uid(Uid), a1(A1), a2(A2) {
    defaults();
}
Person::Person(const litesql::Database& db, const litesql::Record& rec)
     : litesql::Persistent(db, rec), id(Id), type(Type), name(Name), password(Password), uid(Uid), a1(A1), a2(A2) {
    defaults();
    size_t size = (rec.size() > 7) ? 7 : rec.size();
    switch(size) {
    case 7: a2 = convert<const std::string&, bool>(rec[6]);
        a2.setModified(false);
    case 6: a1 = convert<const std::string&, bool>(rec[5]);
        a1.setModified(false);
    case 5: uid = convert<const std::string&, std::string>(rec[4]);
        uid.setModified(false);
    case 4: password = convert<const std::string&, std::string>(rec[3]);
        password.setModified(false);
    case 3: name = convert<const std::string&, std::string>(rec[2]);
        name.setModified(false);
    case 2: type = convert<const std::string&, std::string>(rec[1]);
        type.setModified(false);
    case 1: id = convert<const std::string&, int>(rec[0]);
        id.setModified(false);
    }
}
Person::Person(const Person& obj)
     : litesql::Persistent(obj), id(obj.id), type(obj.type), name(obj.name), password(obj.password), uid(obj.uid), a1(obj.a1), a2(obj.a2) {
}
const Person& Person::operator=(const Person& obj) {
    if (this != &obj) {
        id = obj.id;
        type = obj.type;
        name = obj.name;
        password = obj.password;
        uid = obj.uid;
        a1 = obj.a1;
        a2 = obj.a2;
    }
    litesql::Persistent::operator=(obj);
    return *this;
}
Person::KeysHandle Person::keys() {
    return Person::KeysHandle(*this);
}
Person::KeepkeysHandle Person::keepkeys() {
    return Person::KeepkeysHandle(*this);
}
std::string Person::insert(litesql::Record& tables, litesql::Records& fieldRecs, litesql::Records& valueRecs) {
    tables.push_back(table__);
    litesql::Record fields;
    litesql::Record values;
    fields.push_back(id.name());
    values.push_back(id);
    id.setModified(false);
    fields.push_back(type.name());
    values.push_back(type);
    type.setModified(false);
    fields.push_back(name.name());
    values.push_back(name);
    name.setModified(false);
    fields.push_back(password.name());
    values.push_back(password);
    password.setModified(false);
    fields.push_back(uid.name());
    values.push_back(uid);
    uid.setModified(false);
    fields.push_back(a1.name());
    values.push_back(a1);
    a1.setModified(false);
    fields.push_back(a2.name());
    values.push_back(a2);
    a2.setModified(false);
    fieldRecs.push_back(fields);
    valueRecs.push_back(values);
    return litesql::Persistent::insert(tables, fieldRecs, valueRecs, sequence__);
}
void Person::create() {
    litesql::Record tables;
    litesql::Records fieldRecs;
    litesql::Records valueRecs;
    type = type__;
    std::string newID = insert(tables, fieldRecs, valueRecs);
    if (id == 0)
        id = newID;
}
void Person::addUpdates(Updates& updates) {
    prepareUpdate(updates, table__);
    updateField(updates, table__, id);
    updateField(updates, table__, type);
    updateField(updates, table__, name);
    updateField(updates, table__, password);
    updateField(updates, table__, uid);
    updateField(updates, table__, a1);
    updateField(updates, table__, a2);
}
void Person::addIDUpdates(Updates& updates) {
}
void Person::getFieldTypes(std::vector<litesql::FieldType>& ftypes) {
    ftypes.push_back(Id);
    ftypes.push_back(Type);
    ftypes.push_back(Name);
    ftypes.push_back(Password);
    ftypes.push_back(Uid);
    ftypes.push_back(A1);
    ftypes.push_back(A2);
}
void Person::delRecord() {
    deleteFromTable(table__, id);
}
void Person::delRelations() {
    KeyPersonRelationAcces::del(*db, (KeyPersonRelationAcces::Person == id));
    KeyPersonRelationKeep::del(*db, (KeyPersonRelationKeep::Person == id));
}
void Person::update() {
    if (!inDatabase) {
        create();
        return;
    }
    Updates updates;
    addUpdates(updates);
    if (id != oldKey) {
        if (!typeIsCorrect())
            upcastCopy()->addIDUpdates(updates);
    }
    litesql::Persistent::update(updates);
    oldKey = id;
}
void Person::del() {
    if (!typeIsCorrect()) {
        std::auto_ptr<Person> p(upcastCopy());
        p->delRelations();
        p->onDelete();
        p->delRecord();
    } else {
        delRelations();
        onDelete();
        delRecord();
    }
    inDatabase = false;
}
bool Person::typeIsCorrect() const {
    return type == type__;
}
std::auto_ptr<Person> Person::upcast() const {
    return auto_ptr<Person>(new Person(*this));
}
std::auto_ptr<Person> Person::upcastCopy() const {
    Person* np = new Person(*this);
    np->id = id;
    np->type = type;
    np->name = name;
    np->password = password;
    np->uid = uid;
    np->a1 = a1;
    np->a2 = a2;
    np->inDatabase = inDatabase;
    return auto_ptr<Person>(np);
}
std::ostream & operator<<(std::ostream& os, Person o) {
    os << "-------------------------------------" << std::endl;
    os << o.id.name() << " = " << o.id << std::endl;
    os << o.type.name() << " = " << o.type << std::endl;
    os << o.name.name() << " = " << o.name << std::endl;
    os << o.password.name() << " = " << o.password << std::endl;
    os << o.uid.name() << " = " << o.uid << std::endl;
    os << o.a1.name() << " = " << o.a1 << std::endl;
    os << o.a2.name() << " = " << o.a2 << std::endl;
    os << "-------------------------------------" << std::endl;
    return os;
}
const litesql::FieldType Key::Own::Id("id_",A_field_type_integer,"Key_");
Key::OwnersHandle::OwnersHandle(const Key& owner)
         : litesql::RelationHandle<Key>(owner) {
}
void Key::OwnersHandle::link(const Person& o0) {
    KeyPersonRelationAcces::link(owner->getDatabase(), *owner, o0);
}
void Key::OwnersHandle::unlink(const Person& o0) {
    KeyPersonRelationAcces::unlink(owner->getDatabase(), *owner, o0);
}
void Key::OwnersHandle::del(const litesql::Expr& expr) {
    KeyPersonRelationAcces::del(owner->getDatabase(), expr && KeyPersonRelationAcces::Key == owner->id);
}
litesql::DataSource<Person> Key::OwnersHandle::get(const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    return KeyPersonRelationAcces::get<Person>(owner->getDatabase(), expr, (KeyPersonRelationAcces::Key == owner->id) && srcExpr);
}
litesql::DataSource<KeyPersonRelationAcces::Row> Key::OwnersHandle::getRows(const litesql::Expr& expr) {
    return KeyPersonRelationAcces::getRows(owner->getDatabase(), expr && (KeyPersonRelationAcces::Key == owner->id));
}
Key::KeeperHandle::KeeperHandle(const Key& owner)
         : litesql::RelationHandle<Key>(owner) {
}
void Key::KeeperHandle::link(const Person& o0) {
    KeyPersonRelationKeep::link(owner->getDatabase(), *owner, o0);
}
void Key::KeeperHandle::unlink(const Person& o0) {
    KeyPersonRelationKeep::unlink(owner->getDatabase(), *owner, o0);
}
void Key::KeeperHandle::del(const litesql::Expr& expr) {
    KeyPersonRelationKeep::del(owner->getDatabase(), expr && KeyPersonRelationKeep::Key == owner->id);
}
litesql::DataSource<Person> Key::KeeperHandle::get(const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    return KeyPersonRelationKeep::get<Person>(owner->getDatabase(), expr, (KeyPersonRelationKeep::Key == owner->id) && srcExpr);
}
litesql::DataSource<KeyPersonRelationKeep::Row> Key::KeeperHandle::getRows(const litesql::Expr& expr) {
    return KeyPersonRelationKeep::getRows(owner->getDatabase(), expr && (KeyPersonRelationKeep::Key == owner->id));
}
const std::string Key::type__("Key");
const std::string Key::table__("Key_");
const std::string Key::sequence__("Key_seq");
const litesql::FieldType Key::Id("id_",A_field_type_integer,table__);
const litesql::FieldType Key::Type("type_",A_field_type_string,table__);
const litesql::FieldType Key::Name("name_",A_field_type_string,table__);
const litesql::FieldType Key::Ubi("ubi_",A_field_type_string,table__);
const litesql::FieldType Key::Commentary("commentary_",A_field_type_string,table__);
const litesql::FieldType Key::Pos("pos_",A_field_type_integer,table__);
const litesql::FieldType Key::Active("active_",A_field_type_boolean,table__);
const litesql::FieldType Key::Uid("uid_",A_field_type_string,table__);
void Key::initValues() {
}
void Key::defaults() {
    id = 0;
    pos = 0;
    active = 0;
}
Key::Key(const litesql::Database& db)
     : litesql::Persistent(db), id(Id), type(Type), name(Name), ubi(Ubi), commentary(Commentary), pos(Pos), active(Active), uid(Uid) {
    defaults();
}
Key::Key(const litesql::Database& db, const litesql::Record& rec)
     : litesql::Persistent(db, rec), id(Id), type(Type), name(Name), ubi(Ubi), commentary(Commentary), pos(Pos), active(Active), uid(Uid) {
    defaults();
    size_t size = (rec.size() > 8) ? 8 : rec.size();
    switch(size) {
    case 8: uid = convert<const std::string&, std::string>(rec[7]);
        uid.setModified(false);
    case 7: active = convert<const std::string&, bool>(rec[6]);
        active.setModified(false);
    case 6: pos = convert<const std::string&, int>(rec[5]);
        pos.setModified(false);
    case 5: commentary = convert<const std::string&, std::string>(rec[4]);
        commentary.setModified(false);
    case 4: ubi = convert<const std::string&, std::string>(rec[3]);
        ubi.setModified(false);
    case 3: name = convert<const std::string&, std::string>(rec[2]);
        name.setModified(false);
    case 2: type = convert<const std::string&, std::string>(rec[1]);
        type.setModified(false);
    case 1: id = convert<const std::string&, int>(rec[0]);
        id.setModified(false);
    }
}
Key::Key(const Key& obj)
     : litesql::Persistent(obj), id(obj.id), type(obj.type), name(obj.name), ubi(obj.ubi), commentary(obj.commentary), pos(obj.pos), active(obj.active), uid(obj.uid) {
}
const Key& Key::operator=(const Key& obj) {
    if (this != &obj) {
        id = obj.id;
        type = obj.type;
        name = obj.name;
        ubi = obj.ubi;
        commentary = obj.commentary;
        pos = obj.pos;
        active = obj.active;
        uid = obj.uid;
    }
    litesql::Persistent::operator=(obj);
    return *this;
}
Key::OwnersHandle Key::owners() {
    return Key::OwnersHandle(*this);
}
Key::KeeperHandle Key::keeper() {
    return Key::KeeperHandle(*this);
}
std::string Key::insert(litesql::Record& tables, litesql::Records& fieldRecs, litesql::Records& valueRecs) {
    tables.push_back(table__);
    litesql::Record fields;
    litesql::Record values;
    fields.push_back(id.name());
    values.push_back(id);
    id.setModified(false);
    fields.push_back(type.name());
    values.push_back(type);
    type.setModified(false);
    fields.push_back(name.name());
    values.push_back(name);
    name.setModified(false);
    fields.push_back(ubi.name());
    values.push_back(ubi);
    ubi.setModified(false);
    fields.push_back(commentary.name());
    values.push_back(commentary);
    commentary.setModified(false);
    fields.push_back(pos.name());
    values.push_back(pos);
    pos.setModified(false);
    fields.push_back(active.name());
    values.push_back(active);
    active.setModified(false);
    fields.push_back(uid.name());
    values.push_back(uid);
    uid.setModified(false);
    fieldRecs.push_back(fields);
    valueRecs.push_back(values);
    return litesql::Persistent::insert(tables, fieldRecs, valueRecs, sequence__);
}
void Key::create() {
    litesql::Record tables;
    litesql::Records fieldRecs;
    litesql::Records valueRecs;
    type = type__;
    std::string newID = insert(tables, fieldRecs, valueRecs);
    if (id == 0)
        id = newID;
}
void Key::addUpdates(Updates& updates) {
    prepareUpdate(updates, table__);
    updateField(updates, table__, id);
    updateField(updates, table__, type);
    updateField(updates, table__, name);
    updateField(updates, table__, ubi);
    updateField(updates, table__, commentary);
    updateField(updates, table__, pos);
    updateField(updates, table__, active);
    updateField(updates, table__, uid);
}
void Key::addIDUpdates(Updates& updates) {
}
void Key::getFieldTypes(std::vector<litesql::FieldType>& ftypes) {
    ftypes.push_back(Id);
    ftypes.push_back(Type);
    ftypes.push_back(Name);
    ftypes.push_back(Ubi);
    ftypes.push_back(Commentary);
    ftypes.push_back(Pos);
    ftypes.push_back(Active);
    ftypes.push_back(Uid);
}
void Key::delRecord() {
    deleteFromTable(table__, id);
}
void Key::delRelations() {
    KeyPersonRelationAcces::del(*db, (KeyPersonRelationAcces::Key == id));
    KeyPersonRelationKeep::del(*db, (KeyPersonRelationKeep::Key == id));
}
void Key::update() {
    if (!inDatabase) {
        create();
        return;
    }
    Updates updates;
    addUpdates(updates);
    if (id != oldKey) {
        if (!typeIsCorrect())
            upcastCopy()->addIDUpdates(updates);
    }
    litesql::Persistent::update(updates);
    oldKey = id;
}
void Key::del() {
    if (!typeIsCorrect()) {
        std::auto_ptr<Key> p(upcastCopy());
        p->delRelations();
        p->onDelete();
        p->delRecord();
    } else {
        delRelations();
        onDelete();
        delRecord();
    }
    inDatabase = false;
}
bool Key::typeIsCorrect() const {
    return type == type__;
}
std::auto_ptr<Key> Key::upcast() const {
    return auto_ptr<Key>(new Key(*this));
}
std::auto_ptr<Key> Key::upcastCopy() const {
    Key* np = new Key(*this);
    np->id = id;
    np->type = type;
    np->name = name;
    np->ubi = ubi;
    np->commentary = commentary;
    np->pos = pos;
    np->active = active;
    np->uid = uid;
    np->inDatabase = inDatabase;
    return auto_ptr<Key>(np);
}
std::ostream & operator<<(std::ostream& os, Key o) {
    os << "-------------------------------------" << std::endl;
    os << o.id.name() << " = " << o.id << std::endl;
    os << o.type.name() << " = " << o.type << std::endl;
    os << o.name.name() << " = " << o.name << std::endl;
    os << o.ubi.name() << " = " << o.ubi << std::endl;
    os << o.commentary.name() << " = " << o.commentary << std::endl;
    os << o.pos.name() << " = " << o.pos << std::endl;
    os << o.active.name() << " = " << o.active << std::endl;
    os << o.uid.name() << " = " << o.uid << std::endl;
    os << "-------------------------------------" << std::endl;
    return os;
}
DbSchema::DbSchema(std::string backendType, std::string connInfo)
     : litesql::Database(backendType, connInfo) {
    initialize();
}
std::vector<litesql::Database::SchemaItem> DbSchema::getSchema() const {
    vector<Database::SchemaItem> res;
    string TEXT = backend->getSQLType(A_field_type_string);
    string rowIdType = backend->getRowIDType();
    res.push_back(Database::SchemaItem("schema_","table","CREATE TABLE schema_ (name_ "+TEXT+", type_ "+TEXT+", sql_ "+TEXT+")"));
    if (backend->supportsSequences()) {
        res.push_back(Database::SchemaItem("Person_seq","sequence",backend->getCreateSequenceSQL("Person_seq")));
        res.push_back(Database::SchemaItem("Key_seq","sequence",backend->getCreateSequenceSQL("Key_seq")));
    }
    res.push_back(Database::SchemaItem("Person_","table","CREATE TABLE Person_ (id_ " + rowIdType + ",type_ " + backend->getSQLType(A_field_type_string,"") + "" +",name_ " + backend->getSQLType(A_field_type_string,"") + "" +",password_ " + backend->getSQLType(A_field_type_string,"") + "" +",uid_ " + backend->getSQLType(A_field_type_string,"") + "" +",a1_ " + backend->getSQLType(A_field_type_boolean,"") + "" +",a2_ " + backend->getSQLType(A_field_type_boolean,"") + "" +")"));
    res.push_back(Database::SchemaItem("Key_","table","CREATE TABLE Key_ (id_ " + rowIdType + ",type_ " + backend->getSQLType(A_field_type_string,"") + "" +",name_ " + backend->getSQLType(A_field_type_string,"") + "" +",ubi_ " + backend->getSQLType(A_field_type_string,"") + "" +",commentary_ " + backend->getSQLType(A_field_type_string,"") + "" +",pos_ " + backend->getSQLType(A_field_type_integer,"") + "" +",active_ " + backend->getSQLType(A_field_type_boolean,"") + "" +",uid_ " + backend->getSQLType(A_field_type_string,"") + "" +")"));
    res.push_back(Database::SchemaItem("Key_Person_Acces","table","CREATE TABLE Key_Person_Acces (Key1_ " + backend->getSQLType(A_field_type_integer,"") + "" +",Person2_ " + backend->getSQLType(A_field_type_integer,"") + "" +")"));
    res.push_back(Database::SchemaItem("Key_Person_Keep","table","CREATE TABLE Key_Person_Keep (Key1_ " + backend->getSQLType(A_field_type_integer,"") + " UNIQUE" +",Person2_ " + backend->getSQLType(A_field_type_integer,"") + "" +")"));
    res.push_back(Database::SchemaItem("Person_id_idx","index","CREATE INDEX Person_id_idx ON Person_ (id_)"));
    res.push_back(Database::SchemaItem("Key_id_idx","index","CREATE INDEX Key_id_idx ON Key_ (id_)"));
    res.push_back(Database::SchemaItem("Key_Person_AccesKey1_idx","index","CREATE INDEX Key_Person_AccesKey1_idx ON Key_Person_Acces (Key1_)"));
    res.push_back(Database::SchemaItem("Key_Person_AccesPerson2_idx","index","CREATE INDEX Key_Person_AccesPerson2_idx ON Key_Person_Acces (Person2_)"));
    res.push_back(Database::SchemaItem("Key_Person_Acces_all_idx","index","CREATE INDEX Key_Person_Acces_all_idx ON Key_Person_Acces (Key1_,Person2_)"));
    res.push_back(Database::SchemaItem("Key_Person_KeepKey1_idx","index","CREATE INDEX Key_Person_KeepKey1_idx ON Key_Person_Keep (Key1_)"));
    res.push_back(Database::SchemaItem("Key_Person_KeepPerson2_idx","index","CREATE INDEX Key_Person_KeepPerson2_idx ON Key_Person_Keep (Person2_)"));
    res.push_back(Database::SchemaItem("Key_Person_Keep_all_idx","index","CREATE INDEX Key_Person_Keep_all_idx ON Key_Person_Keep (Key1_,Person2_)"));
    return res;
}
void DbSchema::initialize() {
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;
}
}
