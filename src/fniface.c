/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/fniface.c
 *
 * Copyright (c) 2005-2020	Mark Smulders <msmulders@elsar.nl>
 * Copyright (C) 2019-2020	Dallen Wilson <dwjwilson@lavabit.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "fniface.h"
#include "fnssht.h"
#include "common.h"
#include "conffile.h"
#include "interface.h"
#include "callbacks.h"
#include "gstm.h"
#include "support.h"
#include "systray.h"


static GtkListStore *redirstore;

/*	shows informational text in our main dialog	*/
void gstm_interface_showinfo(char *text)
{
	GtkTextBuffer *statusbuf = gtk_text_buffer_new(NULL);;
	gtk_text_buffer_set_text (statusbuf, (gchar *)text, -1);
	gtk_text_view_set_buffer (GTK_TEXT_VIEW (statusbar), statusbuf);
	g_object_unref (statusbuf);
}

/* returns the ID column value of the selection */
gint gstm_interface_selection2id(GtkTreeSelection *s, int column)
{
	GtkTreeModel *m;
	GtkTreeIter i;
	gint retval = -1;
	
	if (gtk_tree_selection_get_selected(s,&m,&i))
		gtk_tree_model_get (m, &i, column, &retval, -1);

	return retval;
}

/* returns the selection only when 1 (or more) rows are selected */
GtkTreeSelection *gstm_interface_get_selected_tunnel()
{
	GtkTreeSelection *s;

	s = gtk_tree_view_get_selection (GTK_TREE_VIEW (tunlist));
	
	return gtk_tree_selection_count_selected_rows(s)>0?s:NULL;
}

/* enable start/delete/properties/copy buttons */
void gstm_interface_enablebuttons (gboolean active)
{
	GtkWidget *btn_delete = GTK_WIDGET (gtk_builder_get_object (builder, "btn_delete"));
	GtkWidget *btn_properties = GTK_WIDGET (gtk_builder_get_object (builder, "btn_properties"));
	GtkWidget *btn_copy = GTK_WIDGET (gtk_builder_get_object (builder, "btn_copy"));
	GtkWidget *btn_stop = GTK_WIDGET (gtk_builder_get_object (builder, "btn_stop"));
	GtkWidget *btn_start = GTK_WIDGET (gtk_builder_get_object (builder, "btn_start"));
	
	gtk_widget_set_sensitive (btn_delete, TRUE);
	gtk_widget_set_sensitive (btn_properties, TRUE);
	gtk_widget_set_sensitive (btn_copy, TRUE);
	gtk_widget_set_sensitive (btn_stop, active);
	gtk_widget_set_sensitive (btn_start, !active);
}

/* disable start/delete/properties/copy buttons */
void gstm_interface_disablebuttons()
{
	GtkWidget *btn_delete = GTK_WIDGET (gtk_builder_get_object (builder, "btn_delete"));
	GtkWidget *btn_properties = GTK_WIDGET (gtk_builder_get_object (builder, "btn_properties"));
	GtkWidget *btn_copy = GTK_WIDGET (gtk_builder_get_object (builder, "btn_copy"));
	GtkWidget *btn_start = GTK_WIDGET (gtk_builder_get_object (builder, "btn_start"));

	gtk_widget_set_sensitive (btn_start, FALSE);
	gtk_widget_set_sensitive (btn_delete, FALSE);
	gtk_widget_set_sensitive (btn_properties, FALSE);
	gtk_widget_set_sensitive (btn_copy, FALSE);
}

/* put the proper 'active' pixmap in the selected row */
void gstm_interface_paint_row(GtkTreeSelection *s, gboolean active)
{
	GtkTreeModel *m;
	GtkTreeIter i;
	GdkPixbuf *pb;
	
	if (gtk_tree_selection_get_selected(s,&m,&i))
	{
		if (active)
			pb = create_pixbuf_scaled("green.svg", GTK_ICON_SIZE_MENU);
		else
			pb = create_pixbuf_scaled("red.svg", GTK_ICON_SIZE_MENU);
		
		gtk_list_store_set(tunnellist_store, &i, COL_ACTIVE, pb, -1);

		g_free (pb);
	}
}

/* put the proper 'active' pixmap in the row by ID */
void gstm_interface_paint_row_id (int id, gboolean active)
{
	gboolean ret;
	GtkTreeIter i;
	int v_id;
	GdkPixbuf *pb;
	
	ret = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (tunnellist_store), &i);
	while (ret)
	{
		gtk_tree_model_get (GTK_TREE_MODEL (tunnellist_store), &i, COL_ID, &v_id, -1);
		
		if (id == v_id)
		{
			if (active)
				pb = create_pixbuf_scaled ("green.svg", GTK_ICON_SIZE_MENU);
			else
				pb = create_pixbuf_scaled ("red.svg", GTK_ICON_SIZE_MENU);
			
			gtk_list_store_set (tunnellist_store, &i, COL_ACTIVE, pb, -1);
			g_free (pb);
			break;
		}
		
		ret=gtk_tree_model_iter_next (GTK_TREE_MODEL (tunnellist_store), &i);
	}
}

/* refresh a certain row in case the tunnelname has changed */
void gstm_interface_refresh_row_id(int id, const char *name) {
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

	if ((s=gstm_interface_get_selected_tunnel()))
	{
		id = gstm_interface_selection2id (s,COL_ID);

		gchar *msg;
		msg = gstm_ssht_command2string (id);
		gstm_interface_showinfo (msg);
		free (msg);

		gstm_interface_enablebuttons (gSTMtunnels[id]->active);
	}
	else
	{
		gstm_interface_showinfo ("");
		gstm_interface_disablebuttons ();
	}
}

/* action on a row */
void gstm_interface_rowaction()
{
	GtkTreeSelection *s;
	gint id;
	gboolean active;
	if ((s=gstm_interface_get_selected_tunnel())) {
		id = gstm_interface_selection2id (s, COL_ID);
		active = gSTMtunnels[id]->active;
		
		if (!active) {
			//try to start the tunnel
			gstm_interface_paint_row_id (id, !active);
			gstm_interface_enablebuttons (!active);
			gstm_ssht_starttunnel (id);
		} else {
			//stop it
			gstm_ssht_stoptunnel (id);
			//redraw
			gstm_interface_paint_row (s, !active);
			gstm_interface_enablebuttons (!active);
		}

		gstm_docklet_menu_refresh ();
	}
}

/* error dialog */
gboolean gstm_interface_error(const char *msg) {
	GtkWidget *errordialog = gtk_message_dialog_new (
		GTK_WINDOW (maindialog),
		0,
		GTK_MESSAGE_ERROR,
		GTK_BUTTONS_OK,
		NULL);

	gtk_window_set_urgency_hint (GTK_WINDOW (errordialog), TRUE);

	gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (errordialog), msg);
	
	gtk_dialog_run(GTK_DIALOG(errordialog));
	gtk_widget_destroy (errordialog);
	
	if (gtk_widget_is_visible (maindialog))
	{
		GtkWidget *w = GTK_WIDGET (gtk_builder_get_object (builder, "tunnellist"));
		gtk_window_set_focus(GTK_WINDOW(maindialog), w);
	}

	return FALSE;
}

void gstm_interface_redirlist_init(GtkTreeView *v) {
	GtkCellRenderer *renderer;
	GtkTreeViewColumn *c;

	// Clear existing to avoid double/triple/more entries
	GList *existing = gtk_tree_view_get_columns (v);
	for (GList *cur = existing; cur != NULL; cur = cur->next)
		gtk_tree_view_remove_column (v, cur->data);
	g_list_free (existing);

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

	c = gtk_tree_view_get_column(v,COL_TYPE);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	c = gtk_tree_view_get_column(v,COL_PORT1);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	c = gtk_tree_view_get_column(v,COL_HOST);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	c = gtk_tree_view_get_column(v,COL_PORT2);
	gtk_tree_view_column_set_sizing(c, GTK_TREE_VIEW_COLUMN_AUTOSIZE);
	
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
	GtkTreeSelection *selection;
	GtkWidget *wname, *wlogin, *whost, *wport, *wprivkey, *wastart, *warestart, *wanotify, *wmaxrestarts, *wlist;
	GtkWidget *wtunlabel, *wpokbutton;
	GtkWidget *combo_preset;
	int i;
	gboolean ret;
	GtkTreeIter iter;
	char *markup;
	const gchar *tmp;

	if (gSTMtunnels[tid] != NULL)
	{
		wname = GTK_WIDGET (gtk_builder_get_object (builder, "entry_name"));
		gtk_entry_set_text (GTK_ENTRY (wname), (char *)gSTMtunnels[tid]->name);

		wlogin = GTK_WIDGET (gtk_builder_get_object (builder, "entry_login"));
		gtk_entry_set_text (GTK_ENTRY (wlogin), (char *)gSTMtunnels[tid]->login);

		whost = GTK_WIDGET (gtk_builder_get_object (builder, "entry_host"));
		gtk_entry_set_text (GTK_ENTRY (whost), (char *)gSTMtunnels[tid]->host);

		wport = GTK_WIDGET (gtk_builder_get_object (builder, "entry_port"));
		gtk_entry_set_text (GTK_ENTRY (wport), (char *)gSTMtunnels[tid]->port);

		wprivkey = GTK_WIDGET (gtk_builder_get_object (builder, "entry_privkey"));
		gtk_entry_set_text (GTK_ENTRY (wprivkey), (char *)gSTMtunnels[tid]->privkey);

		wastart = GTK_WIDGET (gtk_builder_get_object (builder, "check_auto"));
		if (gSTMtunnels[tid]->autostart)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wastart), TRUE);

		warestart = GTK_WIDGET (gtk_builder_get_object (builder, "check_restart"));
		if (gSTMtunnels[tid]->restart)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (warestart), TRUE);

		wanotify = GTK_WIDGET (gtk_builder_get_object (builder, "check_notify"));
		if (gSTMtunnels[tid]->notify)
			gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (wanotify), TRUE);

		wmaxrestarts = GTK_WIDGET (gtk_builder_get_object (builder, "entry_maxrestarts"));
		gtk_entry_set_text (GTK_ENTRY (wmaxrestarts), (char *)gSTMtunnels[tid]->maxrestarts);

		combo_preset = GTK_WIDGET (gtk_builder_get_object (builder, "combo_preset"));
		gchar *tempHost = NULL;
		if (gSTMtunnels[tid]->preset)
			tempHost = (gchar *)gSTMtunnels[tid]->host;

		parseSSHconfig (combo_preset, tempHost);

		//fill redir list
		wlist = GTK_WIDGET (gtk_builder_get_object (builder, "redirlist"));
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (wlist));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
		gstm_interface_redirlist_init (GTK_TREE_VIEW (wlist));
		
		for (i = 0; i < gSTMtunnels[tid]->defcount; i++) 
			gstm_interface_redirlist_add (gSTMtunnels[tid]->portredirs[i], i, GTK_TREE_VIEW (wlist));
		
		//disable OK if tunnel is active
		wtunlabel = GTK_WIDGET (gtk_builder_get_object (builder, "wtunlabel"));
		wpokbutton = GTK_WIDGET (gtk_builder_get_object (builder, "propdg_btn_ok"));
		if (gSTMtunnels[tid]->active)
		{
			markup = g_markup_printf_escaped ("<b>Cannot edit properties | <span foreground=\"#FF0000\">Tunnel is currently running</span></b>");
			gtk_label_set_markup (GTK_LABEL (wtunlabel), markup);
			g_free (markup);

			gtk_widget_set_sensitive (wpokbutton, FALSE);
		}
		else
		{
			markup = g_markup_printf_escaped (" ");
			gtk_label_set_markup (GTK_LABEL (wtunlabel), markup);
			g_free (markup);

			gtk_widget_set_sensitive (wpokbutton, TRUE);
		}

		//show the dialog
		if (gtk_dialog_run (GTK_DIALOG (propertiesdialog)) == GTK_RESPONSE_OK)
		{
			//save the settings if changed
			tmp = gtk_entry_get_text (GTK_ENTRY (wname));
			if (strcmp (tmp, (char *)gSTMtunnels[tid]->name) != 0)
			{
				free (gSTMtunnels[tid]->name);
				gSTMtunnels[tid]->name = malloc (strlen (tmp) + 1);
				strcpy ((char *)gSTMtunnels[tid]->name, tmp);
				gstm_interface_refresh_row_id (tid, tmp);
			}

			tmp = gtk_entry_get_text (GTK_ENTRY (wlogin));
			if (strcmp (tmp, (char *)gSTMtunnels[tid]->login) != 0)
			{
				free (gSTMtunnels[tid]->login);
				gSTMtunnels[tid]->login = malloc ( strlen (tmp) + 1);
				strcpy ((char *)gSTMtunnels[tid]->login, tmp);
			}
			
			tmp = gtk_entry_get_text (GTK_ENTRY (whost));
			if (strcmp (tmp, (char *)gSTMtunnels[tid]->host) != 0)
			{
				free (gSTMtunnels[tid]->host);
				gSTMtunnels[tid]->host = malloc (strlen (tmp) + 1);
				strcpy ((char *)gSTMtunnels[tid]->host, tmp);
			}
			
			tmp = gtk_entry_get_text (GTK_ENTRY (wport));
			if (strcmp (tmp, (char *)gSTMtunnels[tid]->port) != 0)
			{
				free (gSTMtunnels[tid]->port);
				gSTMtunnels[tid]->port = malloc (strlen (tmp) + 1);
				strcpy ((char *)gSTMtunnels[tid]->port, tmp);
			}

			tmp = gtk_entry_get_text (GTK_ENTRY (wprivkey));
			if (strcmp (tmp, (char *)gSTMtunnels[tid]->privkey) != 0)
			{
				free (gSTMtunnels[tid]->privkey);
				gSTMtunnels[tid]->privkey = malloc (strlen (tmp) + 1);
				strcpy ((char *)gSTMtunnels[tid]->privkey, tmp);
			}
			
			gSTMtunnels[tid]->autostart = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wastart));
			
			gSTMtunnels[tid]->restart = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (warestart));

			gSTMtunnels[tid]->notify = gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (wanotify));

			if (gtk_combo_box_get_active (GTK_COMBO_BOX (combo_preset)) <= 0)
				gSTMtunnels[tid]->preset = FALSE;
			else
				gSTMtunnels[tid]->preset = TRUE;

			tmp = gtk_entry_get_text (GTK_ENTRY (wmaxrestarts));
			if (strcmp (tmp, (char *)gSTMtunnels[tid]->maxrestarts) != 0)
			{
				free (gSTMtunnels[tid]->maxrestarts);
				gSTMtunnels[tid]->maxrestarts = malloc (strlen (tmp) + 1);
				strcpy ((char *)gSTMtunnels[tid]->maxrestarts, tmp);
			}

			//the portredirs need to be erased and readded
			for (i = 0; i < gSTMtunnels[tid]->defcount; i++)
				free (gSTMtunnels[tid]->portredirs[i]);

			free (gSTMtunnels[tid]->portredirs);
			gSTMtunnels[tid]->portredirs = NULL;
			gSTMtunnels[tid]->defcount = 0;
			
			ret = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (redirstore), &iter);
			while (ret)
			{
				gSTMtunnels[tid]->portredirs = realloc (gSTMtunnels[tid]->portredirs, (gSTMtunnels[tid]->defcount + 1) * sizeof (struct portredir *));
				gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount] = malloc (sizeof (struct portredir));
				gtk_tree_model_get (GTK_TREE_MODEL(redirstore), &iter, 
									COL_TYPE, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->type,
									COL_PORT1, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->port1,
									COL_HOST, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->host,
									COL_PORT2, &gSTMtunnels[tid]->portredirs[gSTMtunnels[tid]->defcount]->port2,
									-1);
				gSTMtunnels[tid]->defcount += 1;
				ret = gtk_tree_model_iter_next (GTK_TREE_MODEL (redirstore), &iter);
			}
			
			//and finally, save to file
			gstm_tunnel2file (gSTMtunnels[tid], gSTMtunnels[tid]->fn);
		}
		
		gtk_widget_hide (propertiesdialog);

		gtk_window_set_focus (GTK_WINDOW (maindialog), tunlist);
		gstm_interface_rowactivity();

	}
}

int gstm_interface_asknewname (gchar **nname) {
	const gchar *newname;
	GtkWidget *lbl;
	int resp;

	resp = gtk_dialog_run (GTK_DIALOG (newdialog));
	lbl = GTK_WIDGET (gtk_builder_get_object (builder, "newname"));
	
	if (resp == GTK_RESPONSE_OK)
	{
		newname = gtk_entry_get_text (GTK_ENTRY (lbl));
		
		if (strlen (newname) < 1)
		{
			//empty name, try again :P
			gtk_widget_hide (newdialog);
			return gstm_interface_asknewname (nname);
		}
		else
		{
			if (!gstm_tunnel_name_exists (newname))
			{
				*nname = malloc (strlen (newname) + 1);
				strcpy (*nname, newname);

				gtk_entry_set_text (GTK_ENTRY (lbl), "");
				gtk_widget_hide (newdialog);
				return resp;
			}
			else
			{
				gstm_interface_error ("A tunnel already exists with that name");
				gtk_widget_hide (newdialog);
				return gstm_interface_asknewname (nname);
			}
		}
	}

	gtk_entry_set_text (GTK_ENTRY (lbl), "");
	gtk_widget_hide (newdialog);
	
	return resp;
}
