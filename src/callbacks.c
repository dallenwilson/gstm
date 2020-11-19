/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/callbacks.c
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

#include <gtk/gtk.h>

#include "main.h"
#include "gstm.h"
#include "interface.h"
#include "callbacks.h"
#include "conffile.h"
#include "fniface.h"
#include "fnssht.h"
#include "systray.h"

void gstm_terminate()
{
	GtkWidget *confirm = gtk_message_dialog_new_with_markup (
		GTK_WINDOW (maindialog),
		0,
		GTK_MESSAGE_QUESTION,
		GTK_BUTTONS_NONE,
		"<b><big>Really quit?</big></b>\n\n"
		"All active tunnels will be stopped.");
		
	gtk_dialog_add_buttons (GTK_DIALOG (confirm),
		"_Cancel", GTK_RESPONSE_NO,
		"_Quit", GTK_RESPONSE_YES,
		NULL);
	
	if (activeCount==0 || gtk_dialog_run (GTK_DIALOG(confirm)) == GTK_RESPONSE_YES)
		gstm_quit();

	gtk_widget_destroy(confirm);

	gtk_window_set_focus (GTK_WINDOW(maindialog), tunlist);
}


/*
 * maindialog callbacks
 */
void on_maindialog_size_allocate (GtkWidget *widget, GtkAllocation *allocation)
{
	gtk_window_get_size (GTK_WINDOW (widget), &maindiag_width, &maindiag_height);
}
gboolean maindialog_delete_event_cb (GtkWidget *widget, GdkEvent *event, gpointer user_data)
{
	gstm_terminate();
	return FALSE;
}
void btn_start_clicked_cb (GtkButton *button, gpointer user_data)
{
	gstm_interface_rowaction();
}
void btn_stop_clicked_cb (GtkButton *button, gpointer user_data)
{
	gstm_interface_rowaction();
}
void btn_add_clicked_cb (GtkButton *button, gpointer user_data)
{
	int id = -1;
	gchar *newname = NULL;
	
	if (gstm_interface_asknewname (&newname) == GTK_RESPONSE_OK)
	{
		id = gstm_tunnel_add (newname);
		if (id > -1)
			gstm_interface_properties(id);
	}

	free (newname);
	gtk_window_set_focus (GTK_WINDOW (maindialog), tunlist);
}
void btn_delete_clicked_cb (GtkButton *button, gpointer user_data)
{
	GtkTreeSelection *s;
	GtkWidget *del = gtk_message_dialog_new (GTK_WINDOW (maindialog),
	                                         0, GTK_MESSAGE_QUESTION,
	                                         GTK_BUTTONS_YES_NO, NULL);
	gint id;

	if ((s=gstm_interface_get_selected_tunnel()))
	{
		id = gstm_interface_selection2id (s, COL_ID);
		
		gchar *messagemarkup = g_strdup_printf (
		"<b><big>Delete tunnel?</big></b>\n\nAre you sure you want to delete "
		"the tunnel '%s'?", gSTMtunnels[id]->name);
		
		gtk_message_dialog_set_markup (GTK_MESSAGE_DIALOG (del), messagemarkup);

		if (gtk_dialog_run (GTK_DIALOG (del)) == GTK_RESPONSE_YES)
			gstm_tunnel_del (gstm_interface_selection2id (s, COL_ID));
		
		g_free (messagemarkup);
	}
	else
	{
		gstm_interface_error("oops, selected tunnel not found!");
	}

	gtk_widget_destroy (del);
	gtk_window_set_focus (GTK_WINDOW (maindialog), tunlist);
	gstm_interface_rowactivity ();
}
void btn_properties_clicked_cb (GtkButton *button, gpointer user_data)
{
	GtkTreeSelection *s;
	int id;

	if ((s=gstm_interface_get_selected_tunnel()))
	{
		id = gstm_interface_selection2id (s, COL_ID);
		gstm_interface_properties (id);
	}
}
void btn_copy_clicked_cb (GtkButton *button, gpointer user_data)
{
	GtkTreeSelection *s;
	int i,id = -1;
	char *newname = NULL;
	char *fname;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf_red;

	if ((s=gstm_interface_get_selected_tunnel()))
	{
		id = gstm_interface_selection2id (s, COL_ID);	

		if (gstm_interface_asknewname (&newname) == GTK_RESPONSE_OK)
		{
			/* create new filename */
			fname = (char *)gstm_name2filename (newname);
			
			if (fname)
			{
				/* expand the list */
				gSTMtunnels = realloc (gSTMtunnels, (tunnelCount + 1) * sizeof(struct sshtunnel *));
				gSTMtunnels[tunnelCount] = malloc (sizeof (struct sshtunnel));
				
				/* copy the tunneldata */
				gSTMtunnels[tunnelCount]->name = malloc (strlen (newname) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->name, newname);
				
				gSTMtunnels[tunnelCount]->login = malloc (strlen ((char *)gSTMtunnels[id]->login) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->login, (char *)gSTMtunnels[id]->login);
				
				gSTMtunnels[tunnelCount]->host = malloc (strlen ((char *)gSTMtunnels[id]->host) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->host, (char *)gSTMtunnels[id]->host);
				
				gSTMtunnels[tunnelCount]->port = malloc (strlen ((char *)gSTMtunnels[id]->port) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->port, (char *)gSTMtunnels[id]->port);
				
				gSTMtunnels[tunnelCount]->privkey = malloc (strlen ((char *)gSTMtunnels[id]->privkey) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->privkey, (char *)gSTMtunnels[id]->privkey);
				
				gSTMtunnels[tunnelCount]->maxrestarts = malloc (strlen ((char *)gSTMtunnels[id]->maxrestarts) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->maxrestarts, (char *)gSTMtunnels[id]->maxrestarts);

				gSTMtunnels[tunnelCount]->autostart = gSTMtunnels[id]->autostart;
				gSTMtunnels[tunnelCount]->notify = gSTMtunnels[id]->notify;
				gSTMtunnels[tunnelCount]->restart = gSTMtunnels[id]->restart;
				gSTMtunnels[tunnelCount]->active = FALSE;
				gSTMtunnels[tunnelCount]->sshpid = 0;
				gSTMtunnels[tunnelCount]->defcount = gSTMtunnels[id]->defcount;
				gSTMtunnels[tunnelCount]->portredirs = NULL;
				
				gSTMtunnels[tunnelCount]->fn = malloc (strlen (fname) + 1);
				strcpy ((char *)gSTMtunnels[tunnelCount]->fn, fname);
				
				for (i = 0; i<gSTMtunnels[id]->defcount; i++)
				{
					gSTMtunnels[tunnelCount]->portredirs = realloc (gSTMtunnels[tunnelCount]->portredirs, (i + 1) * sizeof (struct portredir *));
					gSTMtunnels[tunnelCount]->portredirs[i] = malloc (sizeof (struct portredir));
					
					gSTMtunnels[tunnelCount]->portredirs[i]->type = malloc (strlen ((char *)gSTMtunnels[id]->portredirs[i]->type));
					strcpy ((char *)gSTMtunnels[tunnelCount]->portredirs[i]->type, (char *)gSTMtunnels[id]->portredirs[i]->type);
					
					gSTMtunnels[tunnelCount]->portredirs[i]->port1 = malloc (strlen ((char *)gSTMtunnels[id]->portredirs[i]->port1));
					strcpy ((char *)gSTMtunnels[tunnelCount]->portredirs[i]->port1, (char *)gSTMtunnels[id]->portredirs[i]->port1);
					
					gSTMtunnels[tunnelCount]->portredirs[i]->host = malloc (strlen ((char *)gSTMtunnels[id]->portredirs[i]->host));
					strcpy ((char *)gSTMtunnels[tunnelCount]->portredirs[i]->host, (char *)gSTMtunnels[id]->portredirs[i]->host);
					
					gSTMtunnels[tunnelCount]->portredirs[i]->port2 = malloc (strlen ((char *)gSTMtunnels[id]->portredirs[i]->port2));
					strcpy ((char *)gSTMtunnels[tunnelCount]->portredirs[i]->port2, (char *)gSTMtunnels[id]->portredirs[i]->port2);
				}
							
				/* save to file */
				gstm_tunnel2file (gSTMtunnels[tunnelCount], gSTMtunnels[tunnelCount]->fn);

				//put in interface
				pixbuf_red = create_pixbuf_scaled("red.svg", GTK_ICON_SIZE_MENU);
				gtk_list_store_append (tunnellist_store, &iter);
				gtk_list_store_set (tunnellist_store, &iter, COL_ACTIVE,
				                    pixbuf_red, COL_NAME, newname, COL_ID,
				                    tunnelCount, -1);
				tunnelCount += 1;
				
				//sort it again
				gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (tunnellist_store),
				                                      COL_NAME,
				                                      GTK_SORT_ASCENDING);
			}
			else
			{
				gstm_interface_error ("error making new filename");
			}
		}
	}
	else
	{
		gstm_interface_error ("error getting selected tunnel");
	}
	
	free (newname);
	gtk_window_set_focus (GTK_WINDOW (maindialog), tunlist);
}
void btn_close_clicked_cb (GtkButton *button, gpointer user_data)
{
	gstm_terminate ();
}
void tunnellist_row_activated_cb (GtkTreeView		*treeview,
                                  GtkTreePath		*path,
                                  GtkTreeViewColumn	*column,
                                  gpointer			user_data)
{
	gstm_interface_rowaction();
}
gboolean tunnellist_button_release_event_cb (GtkWidget		*widget,
                                             GdkEventButton	*event,
                                             gpointer		user_data)
{
	gstm_interface_rowactivity();
	return FALSE;
}
gboolean tunnellist_key_release_event_cb (GtkWidget		*widget,
                                          GdkEventKey	*event,
                                          gpointer		user_data)
{
	gstm_interface_rowactivity();
	return FALSE;
}


/*
 * propertiesdialog callbacks
 */
void redir_addedit(GtkButton *button, gint editid)
{
	GtkWidget *choicetype, *inputport1, *inputhost, *inputport2, *redirlist;
	int ok = 0;
	struct portredir p;
	
	while (!ok)
	{
		ok = 1;
		
		if (gtk_dialog_run (GTK_DIALOG (tundialog)) == GTK_RESPONSE_OK)
		{
			choicetype = GTK_WIDGET (gtk_builder_get_object (builder, "choice_type"));
			p.type = (xmlChar *)gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (choicetype));
			
			if (choicetype && p.type && (strlen ((char *)p.type) > 0))
			{
				inputport1 = GTK_WIDGET (gtk_builder_get_object (builder, "input_port1"));
				p.port1 = (xmlChar *)gtk_entry_get_text (GTK_ENTRY (inputport1));
				
				if (!p.port1 || (strlen ((char *)p.port1) == 0))
				{
					gstm_interface_error ("Port entry is empty");
					ok = 0;
				}
				
				if (ok)
				{
					if (strcmp ((char *)p.type, "dynamic") == 0)
					{
						p.host = malloc (4);
						strcpy ((char *)p.host, "n/a\0");
						p.port2 = malloc (4);
						strcpy ((char *)p.port2, "n/a\0");
					}
					else
					{
						inputhost = GTK_WIDGET (gtk_builder_get_object (builder, "input_host"));
						p.host = (xmlChar *)gtk_entry_get_text (GTK_ENTRY (inputhost));
						if (!p.host || (strlen ((char *)p.host) == 0))
						{
							gstm_interface_error ("Host entry is empty");
							ok = 0;
						}
						
						if (ok)
						{
							inputport2 = GTK_WIDGET (gtk_builder_get_object (builder, "input_port2"));
							p.port2 = (xmlChar *)gtk_entry_get_text (GTK_ENTRY (inputport2));
							if (!p.port2 || (strlen ((char *)p.port2) == 0))
							{
								gstm_interface_error ("Port2 entry is empty");
								ok = 0;
							}
						}
					}
					if (ok)
					{
						redirlist = GTK_WIDGET (gtk_builder_get_object (builder, "redirlist"));

						//	Edit existing
						if (editid > -1)
							gstm_interface_redirlist_edit (&p, editid, GTK_TREE_VIEW (redirlist));

						//	Add as new
						else
							gstm_interface_redirlist_add (&p, (int)random(), GTK_TREE_VIEW (redirlist));
					}
				}
			}
			else
			{
				ok = 0;
				gstm_interface_error ("No tunnel type selected");
			}
		}
	}
}
void btn_redir_add_clicked_cb (GtkButton *button, gpointer user_data)
{
	// some dialog init
	gtk_window_set_transient_for (GTK_WINDOW (tundialog), GTK_WINDOW (propertiesdialog));
	
	GtkWidget *choicetype = GTK_WIDGET (gtk_builder_get_object (builder, "choice_type"));
	gtk_combo_box_set_active (GTK_COMBO_BOX (choicetype), 0);
	
	// run the dialog
	redir_addedit (button, -1);
}
void btn_redir_delete_clicked_cb (GtkButton *button, gpointer user_data)
{
	GtkWidget *w = GTK_WIDGET (gtk_builder_get_object (builder, "redirlist"));
	GtkTreeSelection *s = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	
	if (gtk_tree_selection_count_selected_rows (s) > 0)
		gstm_interface_redirlist_del (GTK_TREE_VIEW (w), gstm_interface_selection2id (s, COL_RID));
}
void btn_redir_edit_clicked_cb (GtkButton *button, gpointer user_data)
{
	GtkWidget *w = GTK_WIDGET (gtk_builder_get_object (builder, "redirlist"));
	GtkTreeSelection *s;
	gint selid = -1, typeid = 0;
	GtkTreeModel *m;
	GtkTreeIter i;
	gchar *type, *port1, *host, *port2;
	
	s = gtk_tree_view_get_selection (GTK_TREE_VIEW (w));
	if (gtk_tree_selection_count_selected_rows (s) > 0)
		selid = gstm_interface_selection2id (s, COL_RID);

	if (selid > -1 && gtk_tree_selection_get_selected (s, &m, &i))
	{
		// some dialog init
		gtk_window_set_transient_for (GTK_WINDOW (tundialog), GTK_WINDOW (propertiesdialog));
		gtk_window_set_title (GTK_WINDOW (tundialog), "Edit redirection");
		
		// get values from the listselection
		gtk_tree_model_get (m, &i, COL_TYPE, &type, -1);
		gtk_tree_model_get (m, &i, COL_PORT1, &port1, -1);
		gtk_tree_model_get (m, &i, COL_HOST, &host, -1);
		gtk_tree_model_get (m, &i, COL_PORT2, &port2, -1);
		
		// put type into the dialog
		if (strcmp (type, "remote") == 0)
			typeid = 1;
		else if (strcmp (type, "dynamic") == 0)
			typeid = 2;
		
		GtkWidget *choicetype = GTK_WIDGET (gtk_builder_get_object (builder, "choice_type"));
		gtk_combo_box_set_active (GTK_COMBO_BOX (choicetype), typeid);

		// put port1 into the dialog
		GtkWidget *inputport1 = GTK_WIDGET (gtk_builder_get_object (builder, "input_port1"));
		gtk_entry_set_text (GTK_ENTRY (inputport1), port1);

		// put host into the dialog
		GtkWidget *inputhost = GTK_WIDGET (gtk_builder_get_object (builder, "input_host"));
		gtk_entry_set_text (GTK_ENTRY (inputhost), host);

		// put port2 into the dialog
		GtkWidget *inputport2 = GTK_WIDGET (gtk_builder_get_object (builder, "input_port2"));
		gtk_entry_set_text (GTK_ENTRY (inputport2), port2);

		// run the dialog
		redir_addedit (button, selid);
	}
}


/*
 * tundialog callbacks
 */
void tundg_btn_ok_clicked_cb (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide (tundialog);
}
void tundg_btn_cancel_clicked_cb (GtkButton *button, gpointer user_data)
{
	gtk_widget_hide (tundialog);
}
void
tundg_choice_type_changed_cb (GtkComboBox *combobox, gpointer user_data)
{
	char *cbtext;
	GtkWidget *port1 = GTK_WIDGET (gtk_builder_get_object (builder, "input_port1"));
	GtkWidget *host = GTK_WIDGET (gtk_builder_get_object (builder, "input_host"));
	GtkWidget *port2 = GTK_WIDGET (gtk_builder_get_object (builder, "input_port2"));

	cbtext = gtk_combo_box_text_get_active_text (GTK_COMBO_BOX_TEXT (combobox));
			
	if (port1 && host && port2 && (cbtext != NULL))
	{
		if (strcmp (cbtext, "local") == 0)
			gtk_entry_set_text (GTK_ENTRY (port1), "127.0.0.1:");
		else
			gtk_entry_set_text (GTK_ENTRY (port1), "");

		if (strcmp (cbtext, "dynamic") == 0)
		{
			gtk_entry_set_text (GTK_ENTRY (host), "n/a");
			gtk_entry_set_text (GTK_ENTRY (port2), "n/a");
			gtk_editable_set_editable (GTK_EDITABLE (host), FALSE);
			gtk_editable_set_editable (GTK_EDITABLE (port2), FALSE);
		}
		else
		{
			if (strcmp (gtk_entry_get_text (GTK_ENTRY (host)), "n/a") == 0)
			{
				gtk_entry_set_text (GTK_ENTRY (host), "");
				gtk_entry_set_text (GTK_ENTRY (port2), "");
				gtk_editable_set_editable (GTK_EDITABLE (host), TRUE);
				gtk_editable_set_editable (GTK_EDITABLE (port2), TRUE);
			}
		}
	}
}


/*
 * aboutdialog callbacks
 */
gboolean eventbox1_button_press_event_cb (GtkWidget *widget, GdkEventButton  *event, gpointer user_data)
{
	create_aboutdialog ();
	return FALSE;
}


/*
 * systray callbacks
 */
void on_dockletmenu_tunnel_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	int id = (intptr_t)user_data;
	int selid=-1;
	gboolean active = gSTMtunnels[id]->active;
	GtkTreeSelection *s;

	if ((s=gstm_interface_get_selected_tunnel()))
		selid = gstm_interface_selection2id (s, COL_ID);

	if (!active)
	{
		gstm_interface_paint_row_id (id, !active);

		if (selid == id)
			gstm_interface_enablebuttons (!active);
		
		gstm_ssht_starttunnel (id);
	}
	else
	{
		gstm_ssht_stoptunnel (id);
		gstm_interface_paint_row_id (id, !active);
		
		if (selid == id)
			gstm_interface_enablebuttons (!active);
	}
	gstm_docklet_menu_refresh ();
}
void on_dockletmenu_about_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	create_aboutdialog ();
}
void on_dockletmenu_quit_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gstm_terminate();
}
