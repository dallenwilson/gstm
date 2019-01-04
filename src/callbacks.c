#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#include "common.h"
#include "fniface.h"
#include "conffile.h"

extern void gstm_quit(void);
extern void docklet_x11_create(void);
extern void docklet_x11_position_menu(GtkMenu *menu, int *x, int *y, gboolean *push_in, gpointer user_data);

/* quit handlers */
void
on_maindialog_remove                   (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
	//gstm_quit();
}

void gstm_terminate()
{
	extern GtkWidget *maindialog;
	GtkWidget *confirm = gtk_message_dialog_new_with_markup (
		GTK_WINDOW (maindialog),
		0,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_NONE,
		"<b><big>Really quit?</big></b>\n\n"
		"All active tunnels will be stopped.");
		
	gtk_dialog_add_buttons (GTK_DIALOG (confirm),
		GTK_STOCK_CANCEL, GTK_RESPONSE_NO,
		GTK_STOCK_QUIT, GTK_RESPONSE_YES,
		NULL);
	
	if (activeCount==0 || gtk_dialog_run(GTK_DIALOG(confirm))==GTK_RESPONSE_YES)
		gstm_quit();

	gtk_widget_destroy(confirm);
	gtk_window_set_focus(GTK_WINDOW(maindialog),lookup_widget(maindialog,"tunnellist"));

}

gboolean
on_maindialog_delete_event             (GtkWidget       *widget,
                                        GdkEvent        *event,
                                        gpointer         user_data)
{
	gstm_terminate();
}

void
on_btn_close_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gstm_terminate();
}

/* show about dialog, disable main */
gboolean
on_eventbox1_button_press_event        (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	GtkWidget *about;
	about = create_aboutdialog();
	gtk_window_set_skip_taskbar_hint(GTK_WINDOW(about),TRUE);
	gstm_interface_enable_maindialog(FALSE);
	gtk_widget_show(about);
  return FALSE;
}

/* re-enable main dialog */
void
on_aboutdialog_remove                  (GtkContainer    *container,
                                        GtkWidget       *widget,
                                        gpointer         user_data)
{
	gstm_interface_enable_maindialog(TRUE);
	gtk_widget_destroy(widget);
}
void
on_aboutdialog_response                (GtkDialog       *dialog,
                                        gint             response_id,
                                        gpointer         user_data)
{
	gstm_interface_enable_maindialog(TRUE);
	gtk_widget_destroy(GTK_WIDGET(dialog));
}

void
on_btn_properties_clicked              (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkTreeSelection *s;
	int id;
	if (s=gstm_interface_get_selected_tunnel()) {
		id = gstm_interface_selection2id(s,COL_ID);
		gstm_interface_properties(id);
	}
}


void
on_btn_add_clicked                     (GtkButton       *button,
                                        gpointer         user_data)
{
	extern GtkWidget *maindialog;
	int id=-1;
	gchar *newname=NULL;
	if (gstm_interface_asknewname(&newname) == GTK_RESPONSE_OK) {
		id = gstm_tunnel_add(newname);
		if (id>-1) gstm_interface_properties(id);
	}
	free(newname);
	gtk_window_set_focus(GTK_WINDOW(maindialog),lookup_widget(maindialog,"tunnellist"));
}

void
on_btn_delete_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	extern GtkWidget *maindialog;
	GtkTreeSelection *s;
	GtkWidget *del = gtk_message_dialog_new (
		GTK_WINDOW (maindialog),
		0,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_YES_NO,
		"");
	gint id;

	if (s=gstm_interface_get_selected_tunnel()) {
		id = gstm_interface_selection2id(s,COL_ID);	
		gchar *messagemarkup = g_strdup_printf (
			"<b><big>Delete tunnel?</big></b>\n\nAre you sure you want to delete "
			"the tunnel '%s'?", gSTMtunnels[id]->name);
		gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (del), messagemarkup);
		if (gtk_dialog_run(GTK_DIALOG(del))==GTK_RESPONSE_YES) {
			gstm_tunnel_del(gstm_interface_selection2id(s,COL_ID));
		}
		g_free (messagemarkup);
	} else {
		gstm_interface_error("oops, selected tunnel not found!");
	}

	gtk_widget_destroy(del);
	gtk_window_set_focus(GTK_WINDOW(maindialog), lookup_widget(maindialog, "tunnellist"));
	gstm_interface_rowactivity();
}

/* start or stop a tunnel */
void
on_btn_start_clicked                   (GtkButton       *button,
                                        gpointer         user_data)
{
	gstm_interface_rowaction();
}
void
on_tunnellist_row_activated            (GtkTreeView     *treeview,
                                        GtkTreePath     *path,
                                        GtkTreeViewColumn *column,
                                        gpointer         user_data)
{
	gstm_interface_rowaction();
}

/* possible rowselection change */
gboolean
on_tunnellist_button_release_event     (GtkWidget       *widget,
                                        GdkEventButton  *event,
                                        gpointer         user_data)
{
	gstm_interface_rowactivity();
  return FALSE;
}
gboolean
on_tunnellist_key_release_event        (GtkWidget       *widget,
                                        GdkEventKey     *event,
                                        gpointer         user_data)
{
	gstm_interface_rowactivity();
  return FALSE;
}


void redir_addedit(GtkButton *button, GtkWidget *tun, gint editid) {
	GtkWidget *t,*p1,*h,*p2,*w;
	GtkTreeIter iter;
	int ok = 0;
	struct portredir p;
	while (!ok){
		ok=1;
		if (gtk_dialog_run(GTK_DIALOG(tun))==GTK_RESPONSE_OK) {
			if (t=lookup_widget(GTK_WIDGET(tun),"choice_type")) {
				p.type = gtk_combo_box_get_active_text(GTK_COMBO_BOX(t));
			}
			if (t && p.type && strlen(p.type)>0) {
				if (p1=lookup_widget(GTK_WIDGET(tun),"input_port1")) {
					p.port1=(gchar *)gtk_entry_get_text(GTK_ENTRY(p1));
					if (!p.port1 || strlen(p.port1)==0) {
						gstm_interface_error("Port entry is empty");
						ok=0;
					}
				}
				if (ok) {
					if (strcmp(p.type,"dynamic")==0) {
						p.host = malloc(4);
						strcpy(p.host,"n/a\0");
						p.port2 = malloc(4);
						strcpy(p.port2,"n/a\0");
					} else {
						if (h=lookup_widget(GTK_WIDGET(tun),"input_host")) {
							p.host=(gchar *)gtk_entry_get_text(GTK_ENTRY(h));
							if (!p.host || strlen(p.host)==0) {
								gstm_interface_error("Host entry is empty");
								ok=0;
							}
						}
						if (ok) {
							if (p2=lookup_widget(GTK_WIDGET(tun),"input_port2")) {
								p.port2=(gchar *)gtk_entry_get_text(GTK_ENTRY(p2));
								if (!p.port2 || strlen(p.port2)==0) {
									gstm_interface_error("Port2 entry is empty");
									ok=0;
								}
							}
						}
					}
					if (ok) {
						if (w=lookup_widget(GTK_WIDGET(button),"redirlist")) {
							if (editid>-1) {
								/* edit existing */
								gstm_interface_redirlist_edit(&p, editid, GTK_TREE_VIEW(w));
							} else {
								/* add as new */
								gstm_interface_redirlist_add(&p, (int)random(), GTK_TREE_VIEW(w));
							}
						}
					}
				}
			} else {
				ok=0;
				gstm_interface_error("No tunnel type selected");
			}
		}
	}
}

void
on_btn_redir_add_clicked               (GtkButton       *button,
                                        gpointer         user_data)
{
	extern GtkWidget *prop;
	GtkWidget *t;
	GtkWidget *tun = create_tundialog();
	/* some dialog init */
	gtk_window_set_transient_for(GTK_WINDOW(tun),GTK_WINDOW(prop));
	if (t=lookup_widget(GTK_WIDGET(tun),"choice_type")) {
		gtk_combo_box_set_active(GTK_COMBO_BOX(t),0);
	}
	/* run the dialog */
	redir_addedit(button,tun,-1);
	gtk_widget_destroy(tun);
}

void
on_btn_redir_delete_clicked            (GtkButton       *button,
                                        gpointer         user_data)
{
	GtkWidget *w;
	GtkTreeSelection *s;
	if (w = lookup_widget(GTK_WIDGET(button),"redirlist")) {
		s = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
		if (gtk_tree_selection_count_selected_rows(s)>0) {
			gstm_interface_redirlist_del(GTK_TREE_VIEW(w),gstm_interface_selection2id(s,COL_RID));
		}
	}
}



void
on_check_auto_toggled                  (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{

}


void
on_choice_type_changed                 (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	GtkWidget *port1, *host, *port2;
	char *cbtext;
	port1 = lookup_widget(GTK_WIDGET(combobox),"input_port1");
	host = lookup_widget(GTK_WIDGET(combobox),"input_host");
	port2 = lookup_widget(GTK_WIDGET(combobox),"input_port2");
	cbtext = gtk_combo_box_get_active_text(combobox);
	if (port1 && host && port2 && cbtext!=NULL) {
		if (strcmp(cbtext,"dynamic")==0) {
			//gtk_entry_set_text(GTK_ENTRY(port1),"");
			gtk_entry_set_text(GTK_ENTRY(host),"n/a");
			gtk_entry_set_text(GTK_ENTRY(port2),"n/a");
			gtk_editable_set_editable(GTK_EDITABLE(host),FALSE);
			gtk_editable_set_editable(GTK_EDITABLE(port2),FALSE);
		} else {
			if (strcmp(gtk_entry_get_text(GTK_ENTRY(host)),"n/a")==0) {
				//gtk_entry_set_text(GTK_ENTRY(port1),"");
				gtk_entry_set_text(GTK_ENTRY(host),"");
				gtk_entry_set_text(GTK_ENTRY(port2),"");
				gtk_editable_set_editable(GTK_EDITABLE(host),TRUE);
				gtk_editable_set_editable(GTK_EDITABLE(port2),TRUE);
			}
		}
	}
}

void docklet_clicked(int buttontype) {
	extern GtkWidget *maindialog;
	static GtkWidget *menu = NULL;
	
	if (menu) {
		gtk_widget_destroy(menu);
	}
	
	switch (buttontype) {
		case 1:
			//left mouse click
			if (GTK_WIDGET_VISIBLE(maindialog)) {
				//hide the main dialog
				gtk_widget_hide(maindialog);
			} else {
				//show the dialog again
				gtk_widget_show(maindialog);
			}
			break;
		case 2:
			//middle mouse click
			break;
		case 3:
			//right mouse click
			menu = gstm_create_dockletmenu();
			gtk_widget_show_all(menu);
			gtk_menu_popup(GTK_MENU(menu), NULL, NULL,
							docklet_x11_position_menu,
							NULL, 0, gtk_get_current_event_time());
			break;
	}
}

void
on_btn_redir_edit_clicked              (GtkButton       *button,                                        gpointer         user_data)
{
	GtkWidget *w;
	GtkTreeSelection *s;
	gint selid=-1,typeid=0;
	extern GtkWidget *prop;
	GtkWidget *t;
	GtkWidget *tun;
	GtkTreeModel *m;
	GtkTreeIter i;
	gchar *type,*port1,*host,*port2;
	if (w = lookup_widget(GTK_WIDGET(button),"redirlist")) {
		s = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
		if (gtk_tree_selection_count_selected_rows(s)>0) {
			selid=gstm_interface_selection2id(s,COL_RID);
		}
	}
	if (selid>-1 && gtk_tree_selection_get_selected(s,&m,&i)) {
		tun = create_tundialog();
		/* some dialog init */
		gtk_window_set_transient_for(GTK_WINDOW(tun),GTK_WINDOW(prop));
		gtk_window_set_title(GTK_WINDOW(tun),"Edit redirection");
		/* get values from the listselection */
		gtk_tree_model_get (m, &i, COL_TYPE, &type, -1);
		gtk_tree_model_get (m, &i, COL_PORT1, &port1, -1);
		gtk_tree_model_get (m, &i, COL_HOST, &host, -1);
		gtk_tree_model_get (m, &i, COL_PORT2, &port2, -1);
		/* put type into the dialog */
		if (strcmp(type,"remote")==0) {
			typeid=1;
		} else if (strcmp(type,"dynamic")==0) {
			typeid=2;
		}
		if (t=lookup_widget(GTK_WIDGET(tun),"choice_type")) {
			gtk_combo_box_set_active(GTK_COMBO_BOX(t),typeid);
		}
		/* put port1 into the dialog */
		if (t=lookup_widget(GTK_WIDGET(tun),"input_port1")) {
			gtk_entry_set_text(GTK_ENTRY(t),port1);
		}
		/* put host into the dialog */
		if (t=lookup_widget(GTK_WIDGET(tun),"input_host")) {
			gtk_entry_set_text(GTK_ENTRY(t),host);
		}
		/* put port2 into the dialog */
		if (t=lookup_widget(GTK_WIDGET(tun),"input_port2")) {
			gtk_entry_set_text(GTK_ENTRY(t),port2);
		}
		/* run the dialog */
		redir_addedit(button,tun,selid);
		gtk_widget_destroy(tun);
	}
}

void
on_dockletmenu_quit_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data) {
	gstm_terminate();
}
void
on_dockletmenu_toggle_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data) {
	docklet_clicked(1);
}
void
on_dockletmenu_tunnel_activate           (GtkMenuItem     *menuitem,
                                        gpointer         user_data) {
	int id = (int)user_data;
	int selid=-1;
	gboolean active = gSTMtunnels[id]->active;
	GtkTreeSelection *s;
											
	if (s=gstm_interface_get_selected_tunnel()) {
		selid = gstm_interface_selection2id(s,COL_ID);
	}
	if (!active) {
		gstm_interface_paint_row_id(id, !active);
		if (selid==id) gstm_interface_enablebuttons(!active);
		gstm_ssht_starttunnel(id);
	} else {
		gstm_ssht_stoptunnel(id);
		gstm_interface_paint_row_id(id, !active);
		if (selid==id) gstm_interface_enablebuttons(!active);
	}
}

void
on_btn_copy_clicked                  (GtkButton       *button,
                                        gpointer         user_data)
{
	extern GtkWidget *maindialog;
	GtkTreeSelection *s;
	int i,id=-1;
	char *newname=NULL;
	char *fname, *temp;
	struct sshtunnel *tun;
	GtkTreeIter    iter;
	GdkPixbuf *pixbuf_red;
	extern GtkListStore *tunnellist_store;
	
	if (s=gstm_interface_get_selected_tunnel()) {
		id = gstm_interface_selection2id(s,COL_ID);	
		if (gstm_interface_asknewname(&newname) == GTK_RESPONSE_OK) {
			/* create new filename */
			fname = (char *)gstm_name2filename(newname);
			if (fname) {
				/* expand the list */
				gSTMtunnels = realloc(gSTMtunnels, (tunnelCount+1)*sizeof(struct sshtunnel *));
				gSTMtunnels[tunnelCount] = malloc(sizeof(struct sshtunnel));
				/* copy the tunneldata */
				gSTMtunnels[tunnelCount]->name = malloc(strlen(newname)+1);
				strcpy(gSTMtunnels[tunnelCount]->name, newname);
				gSTMtunnels[tunnelCount]->login = malloc(strlen(gSTMtunnels[id]->login)+1);
				strcpy(gSTMtunnels[tunnelCount]->login, gSTMtunnels[id]->login);
				gSTMtunnels[tunnelCount]->host = malloc(strlen(gSTMtunnels[id]->host)+1);
				strcpy(gSTMtunnels[tunnelCount]->host, gSTMtunnels[id]->host);
				gSTMtunnels[tunnelCount]->port = malloc(strlen(gSTMtunnels[id]->port)+1);
				strcpy(gSTMtunnels[tunnelCount]->port, gSTMtunnels[id]->port);
				gSTMtunnels[tunnelCount]->privkey = malloc(strlen(gSTMtunnels[id]->privkey)+1);
				strcpy(gSTMtunnels[tunnelCount]->privkey, gSTMtunnels[id]->privkey);
				gSTMtunnels[tunnelCount]->autostart = gSTMtunnels[id]->autostart;
				gSTMtunnels[tunnelCount]->active = FALSE;
				gSTMtunnels[tunnelCount]->sshpid = 0;
				gSTMtunnels[tunnelCount]->defcount = gSTMtunnels[id]->defcount;
				gSTMtunnels[tunnelCount]->portredirs = NULL;
				gSTMtunnels[tunnelCount]->fn = malloc(strlen(fname)+1);
				strcpy(gSTMtunnels[tunnelCount]->fn, fname);
				for (i=0; i<gSTMtunnels[id]->defcount; i++) {
					gSTMtunnels[tunnelCount]->portredirs = realloc(gSTMtunnels[tunnelCount]->portredirs, (i+1)*sizeof(struct portredir *));
					gSTMtunnels[tunnelCount]->portredirs[i] = malloc(sizeof(struct portredir));
					gSTMtunnels[tunnelCount]->portredirs[i]->type = malloc(strlen(gSTMtunnels[id]->portredirs[i]->type));
					strcpy(gSTMtunnels[tunnelCount]->portredirs[i]->type,gSTMtunnels[id]->portredirs[i]->type);
					gSTMtunnels[tunnelCount]->portredirs[i]->port1 = malloc(strlen(gSTMtunnels[id]->portredirs[i]->port1));
					strcpy(gSTMtunnels[tunnelCount]->portredirs[i]->port1,gSTMtunnels[id]->portredirs[i]->port1);
					gSTMtunnels[tunnelCount]->portredirs[i]->host = malloc(strlen(gSTMtunnels[id]->portredirs[i]->host));
					strcpy(gSTMtunnels[tunnelCount]->portredirs[i]->host,gSTMtunnels[id]->portredirs[i]->host);
					gSTMtunnels[tunnelCount]->portredirs[i]->port2 = malloc(strlen(gSTMtunnels[id]->portredirs[i]->port2));
					strcpy(gSTMtunnels[tunnelCount]->portredirs[i]->port2,gSTMtunnels[id]->portredirs[i]->port2);
				}
				/* save to file */
				gstm_tunnel2file(gSTMtunnels[tunnelCount], gSTMtunnels[tunnelCount]->fn);
				
				//put in interface
				pixbuf_red = create_pixbuf("gstm/red.xpm");
				gtk_list_store_append (tunnellist_store, &iter);
				gtk_list_store_set (tunnellist_store, &iter,
								  COL_ACTIVE, pixbuf_red,
								  COL_NAME, newname,
								  COL_ID, tunnelCount,
								  -1);
				tunnelCount+=1;
				//sort it again
				gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tunnellist_store),
					 COL_NAME,
					 GTK_SORT_ASCENDING);		
			} else {
				gstm_interface_error("error making new filename");
			}
		}
	} else {
		gstm_interface_error("error getting selected tunnel");
	}
	free(newname);
	gtk_window_set_focus(GTK_WINDOW(maindialog),lookup_widget(maindialog,"tunnellist"));
}
