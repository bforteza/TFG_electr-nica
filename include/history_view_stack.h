#ifndef HISTORY_VIEW_STACK_H
#define HISTORY_VIEW_STACK_H

#include <gtkmm/builder.h>
#include <gtkmm/treeview.h>
#include <gtkmm/treemodelsort.h>
#include <gtkmm/liststore.h>
#include <gtkmm/label.h>
#include "dbmanager.hpp"
#include "models.h"
#include "translations.h"

// Vista de solo lectura que muestra el historial de eventos.
// Soporta tres modos de consulta: todo el historial, filtrado por llave,
// o filtrado por persona. Los eventos se presentan ordenados de más
// reciente a más antiguo.
class HistoryViewStack {
public:
    HistoryViewStack(const Glib::RefPtr<Gtk::Builder>& builder);

    // Muestra todos los eventos del historial.
    void view();

    // Muestra solo los eventos relacionados con la llave dada.
    void view(std::shared_ptr<kdb::Key> key);

    // Muestra solo los eventos relacionados con la persona dada.
    void view(std::shared_ptr<kdb::Person> person);

private:
    // Widget de lista para mostrar los eventos.
    Gtk::TreeView* history_tree_view_;

    // Etiqueta que describe el filtro activo (e.g. "Historial: Llave A").
    Gtk::Label* filter_label_;

    // Columnas del TreeView.
    Gtk::TreeViewColumn* timestamp_column_;
    Gtk::TreeViewColumn* event_column_;
    Gtk::TreeViewColumn* person_column_;
    Gtk::TreeViewColumn* key_column_;
    Gtk::TreeViewColumn* pos_column_;

    // Modelo de datos enlazado al TreeView.
    Glib::RefPtr<Gtk::ListStore>      tree_model_;
    Glib::RefPtr<Gtk::TreeModelSort>  sort_model_;

    // Definición de columnas del modelo.
    HistoryModelColumns columns_;

    // Actualiza los textos de cabeceras de columnas al idioma activo.
    void RefreshLabels();

    // Convierte el valor entero de etype en su cadena traducida.
    std::string EventTypeToString(int etype) const;

    // Convierte un número de posición en su cadena legible (usa PosToString).
    // Devuelve "" si pos == 0.
    std::string PosLabel(int pos) const;

    // Carga en el modelo los eventos proporcionados, ordenados por timestamp DESC.
    void Populate(const std::vector<kdb::HistoryEvent>& events);
};

#endif // HISTORY_VIEW_STACK_H
