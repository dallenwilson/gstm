#include <gnome.h>

void
on_maindialog_remove                   (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data);

void
on_btn_close_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

gboolean
on_logo_button_release_event           (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_logo_key_release_event              (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

gboolean
on_logo_button_press_event             (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_eventbox1_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

void
on_aboutdialog_close                   (GtkDialog       *dialog,
                                        gpointer         user_data);

void
on_aboutdialog_remove                  (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data);

gboolean
on_aboutdialog_destroy_event           (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_aboutdialog_close                   (GtkDialog       *dialog,
                                        gpointer         user_data);

void
on_aboutdialog_response                (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data);

gboolean
on_tunnellist_unselect_all             (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_tunnellist_row_activated            (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_tunnellist_cursor_changed           (GtkTreeView     *treeview,
                                        gpointer         user_data);

void
on_btn_properties_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_add_clicked                     (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_tunnellist_row_activated            (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data);

void
on_btn_delete_clicked                  (GtkButton       *button,
                                        gpointer         user_data);

void
on_tunnellist_cursor_changed           (GtkTreeView     *treeview,
                                        gpointer         user_data);

gboolean
on_tunnellist_select_cursor_row        (GtkTreeView     *treeview,
                                        gboolean         start_editing,
                                        gpointer         user_data);

gboolean
on_tunnellist_move_cursor              (GtkTreeView     *treeview,
                                        GtkMovementStep  step,
                                        gint             count,
                                        gpointer         user_data);

gboolean
on_tunnellist_select_cursor_parent     (GtkTreeView     *treeview,
                                        gpointer         user_data);

gboolean
on_tunnellist_selection_clear_event    (GtkWidget       *widget,
                                        GdkEventSelection *event,
                                        gpointer         user_data);

gboolean
on_tunnellist_button_release_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data);

gboolean
on_tunnellist_key_release_event        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data);

void
on_btn_redir_add_clicked               (GtkButton       *button,
                                        gpointer         user_data);

void
on_btn_redir_delete_clicked            (GtkButton       *button,
                                        gpointer         user_data);


void
on_btn_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data);

void
on_check_auto_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data);

void
on_choice_type_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data);

gboolean
on_maindialog_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data);

void
on_btn_redir_edit_clicked              (GtkButton       *button,
                                        gpointer         user_data);

void
on_dockletmenu_quit_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_dockletmenu_toggle_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);
void
on_dockletmenu_tunnel_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data);

void
on_btn_copy_clicked                  (GtkButton       *button,
                                        gpointer         user_data);
