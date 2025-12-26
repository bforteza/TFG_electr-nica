#ifndef datos_hpp
#define datos_hpp
#include "litesql.hpp"
namespace kdb {
class Person;
class Key;
class KeyPersonRelationAcces {
public:
    class Row {
    public:
        litesql::Field<int> person;
        litesql::Field<int> key;
        Row(const litesql::Database& db, const litesql::Record& rec=litesql::Record());
    };
    static const std::string table__;
    static const litesql::FieldType Key;
    static const litesql::FieldType Person;
    static void link(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1);
    static void unlink(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1);
    static void del(const litesql::Database& db, const litesql::Expr& expr=litesql::Expr());
    static litesql::DataSource<KeyPersonRelationAcces::Row> getRows(const litesql::Database& db, const litesql::Expr& expr=litesql::Expr());
    template <class T> static litesql::DataSource<T> get(const litesql::Database& db, const litesql::Expr& expr=litesql::Expr(), const litesql::Expr& srcExpr=litesql::Expr());
;
;
};
class KeyPersonRelationKeep {
public:
    class Row {
    public:
        litesql::Field<int> person;
        litesql::Field<int> key;
        Row(const litesql::Database& db, const litesql::Record& rec=litesql::Record());
    };
    static const std::string table__;
    static const litesql::FieldType Key;
    static const litesql::FieldType Person;
    static void link(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1);
    static void unlink(const litesql::Database& db, const kdb::Key& o0, const kdb::Person& o1);
    static void del(const litesql::Database& db, const litesql::Expr& expr=litesql::Expr());
    static litesql::DataSource<KeyPersonRelationKeep::Row> getRows(const litesql::Database& db, const litesql::Expr& expr=litesql::Expr());
    template <class T> static litesql::DataSource<T> get(const litesql::Database& db, const litesql::Expr& expr=litesql::Expr(), const litesql::Expr& srcExpr=litesql::Expr());
;
;
};
class Person : public litesql::Persistent {
public:
    class Own {
    public:
        static const litesql::FieldType Id;
    };
    class KeysHandle : public litesql::RelationHandle<Person> {
    public:
        KeysHandle(const Person& owner);
        void link(const Key& o0);
        void unlink(const Key& o0);
        void del(const litesql::Expr& expr=litesql::Expr());
        litesql::DataSource<Key> get(const litesql::Expr& expr=litesql::Expr(), const litesql::Expr& srcExpr=litesql::Expr());
        litesql::DataSource<KeyPersonRelationAcces::Row> getRows(const litesql::Expr& expr=litesql::Expr());
    };
    class KeepkeysHandle : public litesql::RelationHandle<Person> {
    public:
        KeepkeysHandle(const Person& owner);
        void link(const Key& o0);
        void unlink(const Key& o0);
        void del(const litesql::Expr& expr=litesql::Expr());
        litesql::DataSource<Key> get(const litesql::Expr& expr=litesql::Expr(), const litesql::Expr& srcExpr=litesql::Expr());
        litesql::DataSource<KeyPersonRelationKeep::Row> getRows(const litesql::Expr& expr=litesql::Expr());
    };
    static const std::string type__;
    static const std::string table__;
    static const std::string sequence__;
    static const litesql::FieldType Id;
    litesql::Field<int> id;
    static const litesql::FieldType Type;
    litesql::Field<std::string> type;
    static const litesql::FieldType Name;
    litesql::Field<std::string> name;
    static const litesql::FieldType Password;
    litesql::Field<std::string> password;
    static const litesql::FieldType Uid;
    litesql::Field<std::string> uid;
    static const litesql::FieldType A1;
    litesql::Field<bool> a1;
    static const litesql::FieldType A2;
    litesql::Field<bool> a2;
    static void initValues();
protected:
    void defaults();
public:
    Person(const litesql::Database& db);
    Person(const litesql::Database& db, const litesql::Record& rec);
    Person(const Person& obj);
    const Person& operator=(const Person& obj);
    Person::KeysHandle keys();
    Person::KeepkeysHandle keepkeys();
protected:
    std::string insert(litesql::Record& tables, litesql::Records& fieldRecs, litesql::Records& valueRecs);
    void create();
    virtual void addUpdates(Updates& updates);
    virtual void addIDUpdates(Updates& updates);
public:
    static void getFieldTypes(std::vector<litesql::FieldType>& ftypes);
protected:
    virtual void delRecord();
    virtual void delRelations();
public:
    virtual void update();
    virtual void del();
    virtual bool typeIsCorrect() const;
    std::auto_ptr<Person> upcast() const;
    std::auto_ptr<Person> upcastCopy() const;
};
std::ostream & operator<<(std::ostream& os, Person o);
class Key : public litesql::Persistent {
public:
    class Own {
    public:
        static const litesql::FieldType Id;
    };
    class OwnersHandle : public litesql::RelationHandle<Key> {
    public:
        OwnersHandle(const Key& owner);
        void link(const Person& o0);
        void unlink(const Person& o0);
        void del(const litesql::Expr& expr=litesql::Expr());
        litesql::DataSource<Person> get(const litesql::Expr& expr=litesql::Expr(), const litesql::Expr& srcExpr=litesql::Expr());
        litesql::DataSource<KeyPersonRelationAcces::Row> getRows(const litesql::Expr& expr=litesql::Expr());
    };
    class KeeperHandle : public litesql::RelationHandle<Key> {
    public:
        KeeperHandle(const Key& owner);
        void link(const Person& o0);
        void unlink(const Person& o0);
        void del(const litesql::Expr& expr=litesql::Expr());
        litesql::DataSource<Person> get(const litesql::Expr& expr=litesql::Expr(), const litesql::Expr& srcExpr=litesql::Expr());
        litesql::DataSource<KeyPersonRelationKeep::Row> getRows(const litesql::Expr& expr=litesql::Expr());
    };
    static const std::string type__;
    static const std::string table__;
    static const std::string sequence__;
    static const litesql::FieldType Id;
    litesql::Field<int> id;
    static const litesql::FieldType Type;
    litesql::Field<std::string> type;
    static const litesql::FieldType Name;
    litesql::Field<std::string> name;
    static const litesql::FieldType Ubi;
    litesql::Field<std::string> ubi;
    static const litesql::FieldType Commentary;
    litesql::Field<std::string> commentary;
    static const litesql::FieldType Pos;
    litesql::Field<int> pos;
    static const litesql::FieldType Active;
    litesql::Field<bool> active;
    static const litesql::FieldType Uid;
    litesql::Field<std::string> uid;
    static void initValues();
protected:
    void defaults();
public:
    Key(const litesql::Database& db);
    Key(const litesql::Database& db, const litesql::Record& rec);
    Key(const Key& obj);
    const Key& operator=(const Key& obj);
    Key::OwnersHandle owners();
    Key::KeeperHandle keeper();
protected:
    std::string insert(litesql::Record& tables, litesql::Records& fieldRecs, litesql::Records& valueRecs);
    void create();
    virtual void addUpdates(Updates& updates);
    virtual void addIDUpdates(Updates& updates);
public:
    static void getFieldTypes(std::vector<litesql::FieldType>& ftypes);
protected:
    virtual void delRecord();
    virtual void delRelations();
public:
    virtual void update();
    virtual void del();
    virtual bool typeIsCorrect() const;
    std::auto_ptr<Key> upcast() const;
    std::auto_ptr<Key> upcastCopy() const;
};
std::ostream & operator<<(std::ostream& os, Key o);
class datos : public litesql::Database {
public:
    datos(std::string backendType, std::string connInfo);
protected:
    virtual std::vector<litesql::Database::SchemaItem> getSchema() const;
    static void initialize();
};
}
#endif
