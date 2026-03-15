#include "history_view_stack.h"
#include "globals.h"
#include "history_logger.h"
#include <litesql/selectquery.hpp>
#include <algorithm>

HistoryViewStack::HistoryViewStack(const Glib::RefPtr<Gtk::Builder>& builder) {
    builder->get_widget("HistoryTreeView", history_tree_view_);
    if (!history_tree_view_)
        throw std::runtime_error("No \"HistoryTreeView\" object in MainWindow.glade");

    builder->get_widget("HistoryFilterLabel", filter_label_);
    if (!filter_label_)
        throw std::runtime_error("No \"HistoryFilterLabel\" object in MainWindow.glade");

    tree_model_ = Gtk::ListStore::create(columns_);
    history_tree_view_->set_model(tree_model_);

    // Las cabeceras se establecen en RefreshLabels(); aquí solo se añaden las columnas.
    history_tree_view_->append_column("", columns_.timestamp_col);
    history_tree_view_->append_column("", columns_.event_col);
    history_tree_view_->append_column("", columns_.person_col);
    history_tree_view_->append_column("", columns_.key_col);
    history_tree_view_->append_column("", columns_.pos_col);

    timestamp_column_ = history_tree_view_->get_column(0);
    event_column_     = history_tree_view_->get_column(1);
    person_column_    = history_tree_view_->get_column(2);
    key_column_       = history_tree_view_->get_column(3);
    pos_column_       = history_tree_view_->get_column(4);

    history_tree_view_->set_enable_search(false);

    language_changed.connect(sigc::mem_fun(*this, &HistoryViewStack::RefreshLabels));
    RefreshLabels();
}

void HistoryViewStack::RefreshLabels() {
    timestamp_column_->set_title(Tr().history_view.col_timestamp);
    event_column_    ->set_title(Tr().history_view.col_event);
    person_column_   ->set_title(Tr().history_view.col_person);
    key_column_      ->set_title(Tr().history_view.col_key);
    pos_column_      ->set_title(Tr().history_view.col_position);
}

void HistoryViewStack::view() {
    filter_label_->set_text("");
    Populate(litesql::select<kdb::HistoryEvent>(*db).all());
}

void HistoryViewStack::view(std::shared_ptr<kdb::Key> key) {
    filter_label_->set_text((std::string)key->name);
    Populate(litesql::select<kdb::HistoryEvent>(
        *db, kdb::HistoryEvent::Keyid == (int)key->id).all());
}

void HistoryViewStack::view(std::shared_ptr<kdb::Person> person) {
    filter_label_->set_text((std::string)person->name);
    Populate(litesql::select<kdb::HistoryEvent>(
        *db, kdb::HistoryEvent::Personid == (int)person->id).all());
}

std::string HistoryViewStack::EventTypeToString(int etype) const {
    switch (static_cast<HistoryEventType>(etype)) {
        case HistoryEventType::PICKUP:          return Tr().history_view.evt_pickup;
        case HistoryEventType::RETURN:          return Tr().history_view.evt_return;
        case HistoryEventType::ADMIN_OPEN:      return Tr().history_view.evt_admin_open;
        case HistoryEventType::KEY_CREATED:     return Tr().history_view.evt_key_created;
        case HistoryEventType::KEY_DEACTIVATED: return Tr().history_view.evt_key_deactivated;
        default:                                return "?";
    }
}

std::string HistoryViewStack::PosLabel(int pos) const {
    return (pos == 0) ? "" : PosToString(pos);
}

void HistoryViewStack::Populate(const std::vector<kdb::HistoryEvent>& events) {
    tree_model_->clear();

    std::vector<kdb::HistoryEvent> sorted = events;
    std::sort(sorted.begin(), sorted.end(),
        [](const kdb::HistoryEvent& a, const kdb::HistoryEvent& b) {
            return (std::string)a.timestamp > (std::string)b.timestamp;
        });

    for (const auto& ev : sorted) {
        auto row = *(tree_model_->append());
        row[columns_.timestamp_col] = (std::string)ev.timestamp;
        row[columns_.event_col]     = EventTypeToString((int)ev.etype);
        row[columns_.person_col]    = (std::string)ev.personname;
        row[columns_.key_col]       = (std::string)ev.keyname;
        row[columns_.pos_col]       = PosLabel((int)ev.pos);
        row[columns_.id_col]        = (int)ev.id;
    }
}
