/***************************************************************************
 *            fniface.c
 *
 *  Thu Aug  4 09:21:30 2005
 *  Copyright  2005  Mark Smulders
 *  msmulders@elsar.nl
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "fniface.h"
#include "support.h"
#include "common.h"
#include "conffile.h"
#include "interface.h"
#include "callbacks.h"

static GtkListStore *redirstore;

/* enables or disables the maindialog - only used for the about dialog */
void gstm_interface_enable_maindialog(gboolean sensitive) {
	extern GtkWidget *maindialog;
	gtk_widget_set_sensitive(maindialog, sensitive);
	gtk_window_present(GTK_WINDOW(maindialog));
	if (sensitive) {
		gtk_window_set_focus(GTK_WINDOW(maindialog),lookup_widget(maindialog,"tunnellist"));
	}
}

/* shows informational text in our main dialog */
void gstm_interface_showinfo(char *text) {
	extern GtkWidget *maindialog;
	GtkWidget *info;

	info = lookup_widget (maindialog, "statusbar");
	gtk_statusbar_push (GTK_STATUSBAR (info), 0, text);
}

/* returns the ID column value of the selection */
gint gstm_interface_selection2id(GtkTreeSelection *s, int column) {
	GtkTreeModel *m;
	GtkTreeIter i;
	gint retval = -1;
	if (gtk_tree_selection_get_selected(s,&m,&i)) {
		gtk_tree_model_get (m, &i, column, &retval, -1);
	}
	return retval;
}

/* returns the selection only when 1 (or more) rows are selected */
GtkTreeSelection *gstm_interface_get_selected_tunnel() {
	extern GtkWidget *maindialog;
	GtkWidget *w;
	GtkTreeSelection *s;
	
	w = lookup_widget(maindialog, "tunnellist");
	s = gtk_tree_view_get_selection(GTK_TREE_VIEW(w));
	return gtk_tree_selection_count_selected_rows(s)>0?s:NULL;
}

/* enable start/delete/properties/copy buttons */
void gstm_interface_enablebuttons(gboolean active) {
	extern GtkWidget *maindialog;
	GtkWidget *w;
	
	if (w=lookup_widget(maindialog,"btn_delete")) {
		gtk_widget_set_sensitive(w, TRUE);
	}
	if (w=lookup_widget(maindialog,"btn_properties")) {
		gtk_widget_set_sensitive(w, TRUE);
	}
	if (w=lookup_widget(maindialog,"btn_copy")) {
		gtk_widget_set_sensitive(w, TRUE);
	}
	if (w=lookup_widget(maindialog,"btn_stop")) {
		gtk_widget_set_sensitive (w, active);
	}
	if (w=lookup_widget(maindialog,"btn_start")) {
		gtk_widget_set_sensitive (w, !active);
	}
}

/* disable start/delete/properties/copy buttons */
void gstm_interface_disablebuttons() {
	extern GtkWidget *maindialog;
	GtkWidget *w;
	if (w=lookup_widget(maindialog,"btn_start")) {
		gtk_widget_set_sensitive(w, FALSE);
	}
	if (w=lookup_widget(maindialog,"btn_delete")) {
		gtk_widget_set_sensitive(w, FALSE);
	}
	if (w=lookup_widget(maindialog,"btn_properties")) {
		gtk_widget_set_sensitive(w, FALSE);
	}
	if (w=lookup_widget(maindialog,"btn_copy")) {
		gtk_widget_set_sensitive(w, FALSE);
	}
}

/* put the proper 'active' pixmap in the selected row */
void gstm_interface_paint_row(GtkTreeSelection *s, gboolean active) {
	extern GtkListStore *tunnellist_store;
	GtkTreeModel *m;
	GtkTreeIter i;
	GdkPixbuf *pb;
	if (gtk_tree_selection_get_selected(s,&m,&i)) {
		if (active) {
			pb = create_pixbuf("gstm/green.xpm");
		} else {
			pb = create_pixbuf("gstm/red.xpm");
		}
		
		gtk_list_store_set(tunnellist_store, &i, COL_ACTIVE, pb, -1);
	}
}

/* put the proper 'active' pixmap in the row by ID */
void gstm_interface_paint_row_id(int id, gboolean active) {
	extern GtkListStore *tunnellist_store;
	gboolean ret;
	GtkTreeIter i;
	int v_id;
	GdkPixbuf *pb;
	
	ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tunnellist_store),&i);
	while (ret) {
		gtk_tree_model_get (GTK_TREE_MODEL(tunnellist_store), &i, COL_ID, &v_id, -1);
		if (id==v_id) {
			if (active) {
				pb = create_pixbuf("gstm/green.xpm");
			} else {
				pb = create_pixbuf("gstm/red.xpm");
			}
			gtk_list_store_set(tunnellist_store, &i, COL_ACTIVE, pb, -1);
			break;
		}
		ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(tunnellist_store),&i);
	}
}

/* refresh a certain row in case the tunnelname has changed */
void gstm_interface_refresh_row_id(int id, const char *name) {
	extern GtkListStore *tunnellist_store;
	gboolean ret;
	GtkTreeIter i;
	int v_id;
	
	ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tunnellist_store),&i);
	while (ret) {
		gtk_tree_model_get (GTK_TREE_MODEL(tunnellist_store), &i, COL_ID, &v_id, -1);
		if (id==v_id) {
			gtk_list_store_set(tunnellist_store, &i, COL_NAME, name, -1);
			break;
		}
		ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(tunnellist_store),&i);
	}
}

/* activity on a row */
void gstm_interface_rowactivity() {
	GtkTreeSelection *s;
	gint id;

	if (s=gstm_interface_get_selected_tunnel()) {
		id = gstm_interface_selection2id(s,COL_ID);
		gchar *msg;
		msg = g_strdup_printf ("%s@%s", gSTMtunnels[id]->login, gSTMtunnels[id]->host);
		gstm_interface_showinfo(msg);
		free(msg);
		gstm_interface_enablebuttons(gSTMtunnels[id]->active);
	} else {
		gstm_interface_showinfo("");
		gstm_interface_disablebuttons();
	}
}

/* action on a row */
void gstm_interface_rowaction() {
	GtkTreeSelection *s;
	gint id;
	gboolean active;
	if (s=gstm_interface_get_selected_tunnel()) {
		id = gstm_interface_selection2id(s,COL_ID);
		active = gSTMtunnels[id]->active;
		
		if (!active) {
			//try to start the tunnel
			gstm_interface_paint_row_id(id, !active);
			gstm_interface_enablebuttons(!active);
			gstm_ssht_starttunnel(id);
		} else {
			//stop it
			gstm_ssht_stoptunnel(id);
			//redraw
			gstm_interface_paint_row(s, !active);
			gstm_interface_enablebuttons(!active);
		}
	}
}

/* error dialog */
void gstm_interface_error(const char *msg) {
	GtkWidget *lbl;
	extern GtkWidget *maindialog;
	GtkWidget *errordialog = gtk_message_dialog_new (
		GTK_WINDOW (maindialog),
		0,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_OK,
		"");

	//set_urgency hint is too new (gtk 2.8)
	//gtk_window_set_urgency_hint(GTK_WINDOW(errordialog), TRUE);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (errordialog), msg);
	
	gtk_dialog_run(GTK_DIALOG(errordialog));
	gtk_widget_destroy (errordialog);
	
	if (GTK_WIDGET_VISIBLE(maindialog))
		gtk_window_set_focus(GTK_WINDOW(maindialog),lookup_widget(maindialog,"tunnellist"));
}

void gstm_interface_redirlist_init(GtkTreeView *v) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *c;
	
	/* --- Column #1 --- */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (v,
											   -1,      
											   "Type",  
											   renderer,
											   "text", COL_TYPE,
											   NULL);
	/* --- Column #2 --- */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (v,
											   -1,      
											   "Port",  
											   renderer,
											   "text", COL_PORT1,
											   NULL);
	/* --- Column #3 --- */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (v,
											   -1,      
											   "To host",  
											   renderer,
											   "text", COL_HOST,
											   NULL);
	/* --- Column #4 --- */
	renderer = gtk_cell_renderer_text_new ();
	gtk_tree_view_insert_column_with_attributes (v,
											   -1,      
											   "To port",  
											   renderer,
											   "text", COL_PORT2,
											   NULL);
	redirstore = gtk_list_store_new (N_RCOLS, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_INT);
	
	//set widths
	c = gtk_tree_view_get_column(v,COL_TYPE);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(c, 60);
	c = gtk_tree_view_get_column(v,COL_PORT1);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(c, 50);
	c = gtk_tree_view_get_column(v,COL_HOST);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(c, 250);
	c = gtk_tree_view_get_column(v,COL_PORT2);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_FIXED);
	gtk_tree_view_column_set_fixed_width(c, 50);
	
	gtk_tree_view_set_model (v, GTK_TREE_MODEL(redirstore));
}
void gstm_interface_redirlist_add(struct portredir *r, int idx, GtkTreeView *v) {
	GtkTreeIter iter;
	gtk_list_store_append (redirstore, &iter);
	gtk_list_store_set (redirstore, &iter,
					  COL_TYPE, r->type,
					  COL_PORT1, r->port1,
					  COL_HOST, r->host,
					  COL_PORT2, r->port2,
					  COL_RID, idx,
					  -1);
}
void gstm_interface_redirlist_edit(struct portredir *r, int idx, GtkTreeView *v) {
	GtkTreeIter i;
	int v_id;
	gboolean ret;
	
	ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(redirstore),&i);
	while (ret) {
		gtk_tree_model_get (GTK_TREE_MODEL(redirstore), &i, COL_RID, &v_id, -1);
		if (idx==v_id) {
			gtk_list_store_set (redirstore, &i,
							  COL_TYPE, r->type,
							  COL_PORT1, r->port1,
							  COL_HOST, r->host,
							  COL_PORT2, r->port2,
							  COL_RID, idx,
							  -1);
			break;
		}
		ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(redirstore),&i);
	}
}
void gstm_interface_redirlist_del(GtkTreeView *v, int id) {
	gboolean ret;
	GtkTreeIter i;
	int v_id;
	
	ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(redirstore),&i);
	while (ret) {
		gtk_tree_model_get (GTK_TREE_MODEL(redirstore), &i, COL_RID, &v_id, -1);
		if (id==v_id) {
			gtk_list_store_remove(redirstore, &i);
			break;
		}
		ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(redirstore),&i);
	}
}

void gstm_interface_properties(int tid) {
	extern GtkWidget *prop;
	extern GtkWidget *maindialog;
	GtkTreeSelection *selection;
	GtkWidget *wname, *wlogin, *whost, *wport, *wprivkey, *wastart, *wlist, *w;
	int i;
	gboolean ret;
	GtkTreeIter iter;
	char *markup, *fn;
	const gchar *tmp;
	prop = create_propertiesdialog();
	
	if (gSTMtunnels[tid]!=NULL) {
		if (wname=lookup_widget(prop,"entry_name")) {
			gtk_entry_set_text(GTK_ENTRY(wname),gSTMtunnels[tid]->name);
		}
		if (wlogin=lookup_widget(prop,"entry_login")) {
			gtk_entry_set_text(GTK_ENTRY(wlogin),gSTMtunnels[tid]->login);
		}
		if (whost=lookup_widget(prop,"entry_host")) {
			gtk_entry_set_text(GTK_ENTRY(whost),gSTMtunnels[tid]->host);
		}
		if (wport=lookup_widget(prop,"entry_port")) {
			gtk_entry_set_text(GTK_ENTRY(wport),gSTMtunnels[tid]->port);
		}
		if (wprivkey=lookup_widget(prop,"entry_privkey")) {
			gtk_entry_set_text(GTK_ENTRY(wprivkey),gSTMtunnels[tid]->privkey);
		}
		if ((wastart=lookup_widget(prop,"check_auto")) && gSTMtunnels[tid]->autostart) {
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON(wastart),TRUE);
		}
		//fill redir list
		if (wlist=lookup_widget(prop,"redirlist")) {
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(wlist));
			gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
			gstm_interface_redirlist_init(GTK_TREE_VIEW(wlist));
			for (i=0; i<gSTMtunnels[tid]->defcount; i++) {
				gstm_interface_redirlist_add(gSTMtunnels[tid]->portredirs[i], i, GTK_TREE_VIEW(wlist));
			}
		}
		//disable OK if tunnel is active
		if (gSTMtunnels[tid]->active) {
			if (w=lookup_widget(prop,"tunlabel")) {
				markup = g_markup_printf_escaped ("<b>Tunnel configuration | <span foreground=\"#FF0000\">Tunnel is currently running</span></b>");
				gtk_label_set_markup (GTK_LABEL (w), markup);
				g_free (markup);
			}
			if (w=lookup_widget(prop,"Pokbutton")) {
				gtk_widget_set_sensitive(w, FALSE);
			}
		}
		//show the dialog
		if (gtk_dialog_run(GTK_DIALOG(prop))==GTK_RESPONSE_OK) {
			//save the settings if changed
			tmp = gtk_entry_get_text(GTK_ENTRY(wname));
			if (strcmp(tmp,gSTMtunnels[tid]->name)!=0) {
				free(gSTMtunnels[tid]->name);
				gSTMtunnels[tid]->name = malloc(strlen(tmp)+1);
				strcpy(gSTMtunnels[tid]->name,tmp);
				gstm_interface_refresh_row_id(tid,tmp);
			}
			tmp = gtk_entry_get_text(GTK_ENTRY(wlogin));
			if (strcmp(tmp,gSTMtunnels[tid]->login)!=0) {
				free(gSTMtunnels[tid]->login);
				gSTMtunnels[tid]->login = malloc(strlen(tmp)+1);
				strcpy(gSTMtunnels[tid]->login,tmp);
			}
			tmp = gtk_entry_get_text(GTK_ENTRY(whost));
			if (strcmp(tmp,gSTMtunnels[tid]->host)!=0) {
				free(gSTMtunnels[tid]->host);
				gSTMtunnels[tid]->host = malloc(strlen(tmp)+1);
				strcpy(gSTMtunnels[tid]->host,tmp);
			}
			tmp = gtk_entry_get_text(GTK_ENTRY(wport));
			if (strcmp(tmp,gSTMtunnels[tid]->port)!=0) {
				free(gSTMtunnels[tid]->port);
				gSTMtunnels[tid]->port = malloc(strlen(tmp)+1);
				strcpy(gSTMtunnels[tid]->port,tmp);
			}
			tmp = gtk_entry_get_text(GTK_ENTRY(wprivkey));
			if (strcmp(tmp,gSTMtunnels[tid]->privkey)!=0) {
				free(gSTMtunnels[tid]->privkey);
				gSTMtunnels[tid]->privkey = malloc(strlen(tmp)+1);
				strcpy(gSTMtunnels[tid]->privkey,tmp);
			}
			gSTMtunnels[tid]->autostart = gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(wastart));
			//the portredirs need to be erased and readded
			for (i=0; i<gSTMtunnels[tid]->defcount; i++) free(gSTMtunnels[tid]->portredirs[i]);
			free(gSTMtunnels[tid]->portredirs);
			gSTMtunnels[tid]->portredirs = NULL;
			gSTMtunnels[tid]->defcount=0;
			
			ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(redirstore),&iter);
			while (ret) {
				gSTMtunnels[tid]->portredirs = realloc(gSTMtunnels[tid]->portredirs, (gSTMtunnels[tid]->defcount +1)*sizeof(struct portredir *));
				gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount] = malloc(sizeof(struct portredir));
				gtk_tree_model_get (GTK_TREE_MODEL(redirstore), &iter, 
									COL_TYPE, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->type,
									COL_PORT1, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->port1,
									COL_HOST, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->host,
									COL_PORT2, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->port2,
									-1);
				gSTMtunnels[tid]->defcount+=1;
				ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(redirstore),&iter);
			}
			//and finally, save to file
			gstm_tunnel2file(gSTMtunnels[tid], gSTMtunnels[tid]->fn);
		}
		gtk_widget_destroy(prop);
		gtk_window_set_focus(GTK_WINDOW(maindialog),lookup_widget(maindialog,"tunnellist"));
		gstm_interface_rowactivity(); //since login or host may have been changed ;)
	}
}

void gstm_dockletmenu_tunnelitem_new(GtkWidget *menu, const gchar *t_name, int t_id, gboolean t_active) {
  GtkWidget *item_tunnel;
  GtkWidget *img_yesno;
  GdkPixbuf *pb;

  if (t_active) {	
	pb = create_pixbuf("gstm/green.xpm");
  } else {
	pb = create_pixbuf("gstm/red.xpm");
  }
  img_yesno = gtk_image_new_from_pixbuf(pb);
  
  gtk_widget_show (img_yesno);
	
  item_tunnel = gtk_image_menu_item_new_with_mnemonic (t_name);
  gtk_widget_show (item_tunnel);
  gtk_container_add (GTK_CONTAINER (menu), item_tunnel);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_tunnel), img_yesno);
  
  g_signal_connect ((gpointer) item_tunnel, "activate",
                    G_CALLBACK (on_dockletmenu_tunnel_activate),
                    (gpointer)t_id);
}

GtkWidget*
gstm_create_dockletmenu (void)
{
  GtkWidget *dmenu;
  GtkWidget *item_toggle;
  GtkWidget *separator1;
  GtkWidget *image_toggle;
  GtkWidget *separator2;
  GtkWidget *item_quit;
  GtkWidget *image_quit;
  extern GtkWidget *maindialog;
  extern GtkListStore *tunnellist_store;
  GtkTreeIter i;
  int v_id;
  gboolean ret;

  dmenu = gtk_menu_new ();
  image_quit = gtk_image_new_from_stock ("gtk-quit", GTK_ICON_SIZE_MENU);
  image_toggle = gtk_image_new_from_stock ("gtk-dialog-authentication", GTK_ICON_SIZE_MENU);
  gtk_widget_show (image_toggle);
  gtk_widget_show (image_quit);

  if (GTK_WIDGET_VISIBLE(maindialog)) {
	  item_toggle = gtk_image_menu_item_new_with_mnemonic (_("Hide gSTM"));
  } else {
	  item_toggle = gtk_image_menu_item_new_with_mnemonic (_("Show gSTM"));
  }
  gtk_widget_show (item_toggle);
  gtk_container_add (GTK_CONTAINER (dmenu), item_toggle);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_toggle), image_toggle);

  separator1 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator1);
  gtk_container_add (GTK_CONTAINER (dmenu), separator1);
  gtk_widget_set_sensitive (separator1, FALSE);

  /* add tunnels to the menu, based on the maindialog list (it's sorted) */
  ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tunnellist_store),&i);
  while (ret) {
	gtk_tree_model_get (GTK_TREE_MODEL(tunnellist_store), &i, COL_ID, &v_id, -1);
    if (gSTMtunnels[v_id]!=NULL) {
  		gstm_dockletmenu_tunnelitem_new(dmenu, gSTMtunnels[v_id]->name, v_id, gSTMtunnels[v_id]->active);
	}
	ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(tunnellist_store),&i);
  }

  separator2 = gtk_separator_menu_item_new ();
  gtk_widget_show (separator2);
  gtk_container_add (GTK_CONTAINER (dmenu), separator2);
  gtk_widget_set_sensitive (separator2, FALSE);
  
  item_quit = gtk_image_menu_item_new_with_mnemonic (_("Quit"));
  gtk_widget_show (item_quit);
  gtk_container_add (GTK_CONTAINER (dmenu), item_quit);
  gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_quit), image_quit);

  g_signal_connect ((gpointer) item_toggle, "activate",
                    G_CALLBACK (on_dockletmenu_toggle_activate),
                    NULL);
  g_signal_connect ((gpointer) item_quit, "activate",
                    G_CALLBACK (on_dockletmenu_quit_activate),
                    NULL);
  
  return dmenu;
}

int gstm_interface_asknewname(gchar **nname) {
	const gchar *newname;
	GtkWidget *lbl;
	GtkWidget *new = create_newdialog();
	int resp;
	resp = gtk_dialog_run(GTK_DIALOG(new));
	if (resp==GTK_RESPONSE_OK) {
		if (lbl = lookup_widget(new, "newname")) {
			newname = gtk_entry_get_text(GTK_ENTRY(lbl));
			if (strlen(newname)<1) {
				//empty name, try again :P
				gtk_widget_destroy(new);
				return gstm_interface_asknewname(nname);
			} else {
				if (!gstm_tunnel_name_exists(newname)) {
					*nname = malloc(strlen(newname)+1);
					strcpy(*nname, newname);
					gtk_widget_destroy(new);
					return resp;
				} else {
					gstm_interface_error("A tunnel already exists with that name");
					gtk_widget_destroy(new);
					return gstm_interface_asknewname(nname);
				}
			}
		} else {
			gstm_interface_error("oops, newname widget not found!");
		}
	}
	gtk_widget_destroy(new);
	return resp;
}
