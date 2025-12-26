#include "database.hpp"
namespace kdb {
using namespace litesql;
KeyPersonRelationOwnership::Row::Row(const litesql::Database& db, const litesql::Record& rec)
         : person(KeyPersonRelationOwnership::Person), key(KeyPersonRelationOwnership::Key) {
    switch(rec.size()) {
    case 2:
        person = rec[1];
    case 1:
        key = rec[0];
    }
}
const std::string KeyPersonRelationOwnership::table__("Key_Person_Ownership");
const litesql::FieldType KeyPersonRelationOwnership::Key("Key1_",A_field_type_integer,table__);
const litesql::FieldType KeyPersonRelationOwnership::Person("Person2_",A_field_type_integer,table__);
void KeyPersonRelationOwnership::link(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1) {
    Record values;
    Split fields;
    fields.push_back(Key.name());
    values.push_back(o0.id);
    fields.push_back(Person.name());
    values.push_back(o1.id);
    db.insert(table__, values, fields);
}
void KeyPersonRelationOwnership::unlink(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1) {
    db.delete_(table__, (Key == o0.id && Person == o1.id));
}
void KeyPersonRelationOwnership::del(const litesql::Database& db, const litesql::Expr& expr) {
    db.delete_(table__, expr);
}
litesql::DataSource<KeyPersonRelationOwnership::Row> KeyPersonRelationOwnership::getRows(const litesql::Database& db, const litesql::Expr& expr) {
    SelectQuery sel;
    sel.result(Key.fullName());
    sel.result(Person.fullName());
    sel.source(table__);
    sel.where(expr);
    return DataSource<KeyPersonRelationOwnership::Row>(db, sel);
}
template <> litesql::DataSource<kdb::Key> KeyPersonRelationOwnership::get(const litesql::Database& db, const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    SelectQuery sel;
    sel.source(table__);
    sel.result(Key.fullName());
    sel.where(srcExpr);
    return DataSource<kdb::Key>(db, kdb::Key::Id.in(sel) && expr);
}
template <> litesql::DataSource<kdb::Person> KeyPersonRelationOwnership::get(const litesql::Database& db, const litesql::Expr& expr, const litesql::Expr& srcExpr) {
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
    KeyPersonRelationOwnership::link(owner->getDatabase(), o0, *owner);
}
void Person::KeysHandle::unlink(const Key& o0) {
    KeyPersonRelationOwnership::unlink(owner->getDatabase(), o0, *owner);
}
void Person::KeysHandle::del(const litesql::Expr& expr) {
    KeyPersonRelationOwnership::del(owner->getDatabase(), expr && KeyPersonRelationOwnership::Person == owner->id);
}
litesql::DataSource<Key> Person::KeysHandle::get(const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    return KeyPersonRelationOwnership::get<Key>(owner->getDatabase(), expr, (KeyPersonRelationOwnership::Person == owner->id) && srcExpr);
}
litesql::DataSource<KeyPersonRelationOwnership::Row> Person::KeysHandle::getRows(const litesql::Expr& expr) {
    return KeyPersonRelationOwnership::getRows(owner->getDatabase(), expr && (KeyPersonRelationOwnership::Person == owner->id));
}
const std::string Person::type__("Person");
const std::string Person::table__("Person_");
const std::string Person::sequence__("Person_seq");
const litesql::FieldType Person::Id("id_",A_field_type_integer,table__);
const litesql::FieldType Person::Type("type_",A_field_type_string,table__);
const litesql::FieldType Person::Name("name_",A_field_type_string,table__);
const litesql::FieldType Person::Password("password_",A_field_type_string,table__);
void Person::initValues() {
}
void Person::defaults() {
    id = 0;
}
Person::Person(const litesql::Database& db)
     : litesql::Persistent(db), id(Id), type(Type), name(Name), password(Password) {
    defaults();
}
Person::Person(const litesql::Database& db, const litesql::Record& rec)
     : litesql::Persistent(db, rec), id(Id), type(Type), name(Name), password(Password) {
    defaults();
    size_t size = (rec.size() > 4) ? 4 : rec.size();
    switch(size) {
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
     : litesql::Persistent(obj), id(obj.id), type(obj.type), name(obj.name), password(obj.password) {
}
const Person& Person::operator=(const Person& obj) {
    if (this != &obj) {
        id = obj.id;
        type = obj.type;
        name = obj.name;
        password = obj.password;
    }
    litesql::Persistent::operator=(obj);
    return *this;
}
Person::KeysHandle Person::keys() {
    return Person::KeysHandle(*this);
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
}
void Person::addIDUpdates(Updates& updates) {
}
void Person::getFieldTypes(std::vector<litesql::FieldType>& ftypes) {
    ftypes.push_back(Id);
    ftypes.push_back(Type);
    ftypes.push_back(Name);
    ftypes.push_back(Password);
}
void Person::delRecord() {
    deleteFromTable(table__, id);
}
void Person::delRelations() {
    KeyPersonRelationOwnership::del(*db, (KeyPersonRelationOwnership::Person == id));
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
        std::unique_ptr<Person> p(upcastCopy());
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
std::unique_ptr<Person> Person::upcast() const {
    return unique_ptr<Person>(new Person(*this));
}
std::unique_ptr<Person> Person::upcastCopy() const {
    Person* np = new Person(*this);
    np->id = id;
    np->type = type;
    np->name = name;
    np->password = password;
    np->inDatabase = inDatabase;
    return unique_ptr<Person>(np);
}
std::ostream & operator<<(std::ostream& os, Person o) {
    os << "-------------------------------------" << std::endl;
    os << o.id.name() << " = " << o.id << std::endl;
    os << o.type.name() << " = " << o.type << std::endl;
    os << o.name.name() << " = " << o.name << std::endl;
    os << o.password.name() << " = " << o.password << std::endl;
    os << "-------------------------------------" << std::endl;
    return os;
}
const litesql::FieldType Key::Own::Id("id_",A_field_type_integer,"Key_");
Key::OwnersHandle::OwnersHandle(const Key& owner)
         : litesql::RelationHandle<Key>(owner) {
}
void Key::OwnersHandle::link(const Person& o0) {
    KeyPersonRelationOwnership::link(owner->getDatabase(), *owner, o0);
}
void Key::OwnersHandle::unlink(const Person& o0) {
    KeyPersonRelationOwnership::unlink(owner->getDatabase(), *owner, o0);
}
void Key::OwnersHandle::del(const litesql::Expr& expr) {
    KeyPersonRelationOwnership::del(owner->getDatabase(), expr && KeyPersonRelationOwnership::Key == owner->id);
}
litesql::DataSource<Person> Key::OwnersHandle::get(const litesql::Expr& expr, const litesql::Expr& srcExpr) {
    return KeyPersonRelationOwnership::get<Person>(owner->getDatabase(), expr, (KeyPersonRelationOwnership::Key == owner->id) && srcExpr);
}
litesql::DataSource<KeyPersonRelationOwnership::Row> Key::OwnersHandle::getRows(const litesql::Expr& expr) {
    return KeyPersonRelationOwnership::getRows(owner->getDatabase(), expr && (KeyPersonRelationOwnership::Key == owner->id));
}
const std::string Key::type__("Key");
const std::string Key::table__("Key_");
const std::string Key::sequence__("Key_seq");
const litesql::FieldType Key::Id("id_",A_field_type_integer,table__);
const litesql::FieldType Key::Type("type_",A_field_type_string,table__);
const litesql::FieldType Key::Name("name_",A_field_type_string,table__);
const litesql::FieldType Key::Ubi("ubi_",A_field_type_string,table__);
const litesql::FieldType Key::Pos("pos_",A_field_type_integer,table__);
void Key::initValues() {
}
void Key::defaults() {
    id = 0;
    pos = 0;
}
Key::Key(const litesql::Database& db)
     : litesql::Persistent(db), id(Id), type(Type), name(Name), ubi(Ubi), pos(Pos) {
    defaults();
}
Key::Key(const litesql::Database& db, const litesql::Record& rec)
     : litesql::Persistent(db, rec), id(Id), type(Type), name(Name), ubi(Ubi), pos(Pos) {
    defaults();
    size_t size = (rec.size() > 5) ? 5 : rec.size();
    switch(size) {
    case 5: pos = convert<const std::string&, int>(rec[4]);
        pos.setModified(false);
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
     : litesql::Persistent(obj), id(obj.id), type(obj.type), name(obj.name), ubi(obj.ubi), pos(obj.pos) {
}
const Key& Key::operator=(const Key& obj) {
    if (this != &obj) {
        id = obj.id;
        type = obj.type;
        name = obj.name;
        ubi = obj.ubi;
        pos = obj.pos;
    }
    litesql::Persistent::operator=(obj);
    return *this;
}
Key::OwnersHandle Key::owners() {
    return Key::OwnersHandle(*this);
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
    fields.push_back(pos.name());
    values.push_back(pos);
    pos.setModified(false);
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
    updateField(updates, table__, pos);
}
void Key::addIDUpdates(Updates& updates) {
}
void Key::getFieldTypes(std::vector<litesql::FieldType>& ftypes) {
    ftypes.push_back(Id);
    ftypes.push_back(Type);
    ftypes.push_back(Name);
    ftypes.push_back(Ubi);
    ftypes.push_back(Pos);
}
void Key::delRecord() {
    deleteFromTable(table__, id);
}
void Key::delRelations() {
    KeyPersonRelationOwnership::del(*db, (KeyPersonRelationOwnership::Key == id));
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
        std::unique_ptr<Key> p(upcastCopy());
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
std::unique_ptr<Key> Key::upcast() const {
    return unique_ptr<Key>(new Key(*this));
}
std::unique_ptr<Key> Key::upcastCopy() const {
    Key* np = new Key(*this);
    np->id = id;
    np->type = type;
    np->name = name;
    np->ubi = ubi;
    np->pos = pos;
    np->inDatabase = inDatabase;
    return unique_ptr<Key>(np);
}
std::ostream & operator<<(std::ostream& os, Key o) {
    os << "-------------------------------------" << std::endl;
    os << o.id.name() << " = " << o.id << std::endl;
    os << o.type.name() << " = " << o.type << std::endl;
    os << o.name.name() << " = " << o.name << std::endl;
    os << o.ubi.name() << " = " << o.ubi << std::endl;
    os << o.pos.name() << " = " << o.pos << std::endl;
    os << "-------------------------------------" << std::endl;
    return os;
}
Database::Database(std::string backendType, std::string connInfo)
     : litesql::Database(backendType, connInfo) {
    initialize();
}
std::vector<litesql::Database::SchemaItem> Database::getSchema() const {
    vector<Database::SchemaItem> res;
    string TEXT = backend->getSQLType(A_field_type_string);
    string rowIdType = backend->getRowIDType();
    res.push_back(Database::SchemaItem("schema_","table","CREATE TABLE schema_ (name_ "+TEXT+", type_ "+TEXT+", sql_ "+TEXT+")"));
    if (backend->supportsSequences()) {
        res.push_back(Database::SchemaItem("Person_seq","sequence",backend->getCreateSequenceSQL("Person_seq")));
        res.push_back(Database::SchemaItem("Key_seq","sequence",backend->getCreateSequenceSQL("Key_seq")));
    }
    res.push_back(Database::SchemaItem("Person_","table","CREATE TABLE Person_ (id_ " + rowIdType + ",type_ " + backend->getSQLType(A_field_type_string,"") + "" +",name_ " + backend->getSQLType(A_field_type_string,"") + "" +",password_ " + backend->getSQLType(A_field_type_string,"") + "" +")"));
    res.push_back(Database::SchemaItem("Key_","table","CREATE TABLE Key_ (id_ " + rowIdType + ",type_ " + backend->getSQLType(A_field_type_string,"") + "" +",name_ " + backend->getSQLType(A_field_type_string,"") + "" +",ubi_ " + backend->getSQLType(A_field_type_string,"") + "" +",pos_ " + backend->getSQLType(A_field_type_integer,"") + "" +")"));
    res.push_back(Database::SchemaItem("Key_Person_Ownership","table","CREATE TABLE Key_Person_Ownership (Key1_ " + backend->getSQLType(A_field_type_integer,"") + "" +",Person2_ " + backend->getSQLType(A_field_type_integer,"") + "" +")"));
    res.push_back(Database::SchemaItem("Person_id_idx","index","CREATE INDEX Person_id_idx ON Person_ (id_)"));
    res.push_back(Database::SchemaItem("Key_id_idx","index","CREATE INDEX Key_id_idx ON Key_ (id_)"));
    res.push_back(Database::SchemaItem("Key_Person_OwnershipKey1_idx","index","CREATE INDEX Key_Person_OwnershipKey1_idx ON Key_Person_Ownership (Key1_)"));
    res.push_back(Database::SchemaItem("O0e5616a468e032e837c0f9f87324f","index","CREATE INDEX O0e5616a468e032e837c0f9f87324f ON Key_Person_Ownership (Person2_)"));
    res.push_back(Database::SchemaItem("Key_Person_Ownership_all_idx","index","CREATE INDEX Key_Person_Ownership_all_idx ON Key_Person_Ownership (Key1_,Person2_)"));
    return res;
}
void Database::initialize() {
    static bool initialized = false;
    if (initialized)
        return;
    initialized = true;
}
}
