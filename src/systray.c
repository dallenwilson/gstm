/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/systray.c
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

#include "config.h"
#include <gtk/gtk.h>
#include <glib/gi18n.h>

#ifdef AYATANA
    #include <libayatana-appindicator/app-indicator.h>
#else
    #include <libappindicator/app-indicator.h>
#endif

#include "systray.h"
#include "conffile.h"
#include "gstm.h"
#include "callbacks.h"
#include "main.h"
#include "support.h"

//GtkStatusIcon *ci = NULL;
AppIndicator *ci = NULL;
bool docklet_connected = FALSE;

static void gstm_docklet_connection_changed_cb(AppIndicator* indicator, gboolean connected,
                                              gpointer user_data)
{
	docklet_connected = connected;
}

void gstm_docklet_create ()
{
	//	Create systray icon
	//	AppIndicator alternative
	ci = app_indicator_new ("gSTM", "gSTM", APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	g_assert (IS_APP_INDICATOR (ci));
	g_assert (G_IS_OBJECT (ci));
	g_signal_connect(G_OBJECT(ci), APP_INDICATOR_SIGNAL_CONNECTION_CHANGED, G_CALLBACK(gstm_docklet_connection_changed_cb), 0);
	gstm_docklet_menu_refresh();
	app_indicator_set_status (ci, APP_INDICATOR_STATUS_ACTIVE);
}

bool gstm_docklet_active ()
{
	return docklet_connected;
}

void gstm_toggle_mainwindow ()
{
	if (gtk_widget_is_visible (maindialog))
		gtk_widget_hide (maindialog);
	else
		gtk_widget_show (maindialog);

	gstm_docklet_menu_refresh ();
}

void gstm_docklet_activated_cb (GtkWidget *widget, gpointer user_data)
{
	gstm_toggle_mainwindow();
}

void gstm_docklet_menu_refresh ()
{
	GtkMenu *newmenu = gstm_docklet_menu_regen ();
	app_indicator_set_menu (ci, GTK_MENU (newmenu));
}

void gstm_dockletmenu_tunnelitem_new (GtkMenu *menu, const gchar *t_name,
                                      intptr_t t_id, gboolean t_active)
{
	GdkPixbuf *pb;

	if (t_active)
		pb = create_pixbuf_scaled("green.svg", GTK_ICON_SIZE_MENU);
	else
		pb = create_pixbuf_scaled("red.svg", GTK_ICON_SIZE_MENU);

	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	GtkWidget *icon = gtk_image_new_from_pixbuf(pb);
	GtkWidget *label;

	gchar *tmpchar = NULL;
	if (t_active)
		tmpchar = g_strconcat ("Disconnect: ", t_name, NULL);
	else
		tmpchar = g_strconcat ("Connect: ", t_name, NULL);

	label = gtk_label_new (tmpchar);
	g_free (tmpchar);

	GtkWidget *menu_item = gtk_menu_item_new ();

	gtk_container_add (GTK_CONTAINER (box), icon);
	gtk_container_add (GTK_CONTAINER (box), label);
	gtk_container_add (GTK_CONTAINER (menu_item), box);

	gtk_widget_show_all (menu_item);
	gtk_container_add (GTK_CONTAINER (menu), menu_item);
	
	g_signal_connect ((gpointer) menu_item, "activate",
	                  G_CALLBACK (on_dockletmenu_tunnel_activate),
	                  (gpointer)t_id);

	g_object_unref(pb);
}

GtkMenu* gstm_docklet_menu_regen ()
{
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	GtkWidget *box = NULL;
	GtkWidget *icon = NULL;
	GtkWidget *label = NULL;
	GtkWidget *menu_item = NULL;
	
	// Show/Hide
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	icon = gtk_image_new_from_icon_name ("dialog-password", GTK_ICON_SIZE_MENU);
	if (gtk_widget_is_visible (maindialog))
		label = gtk_label_new ("Hide gSTM");
	else
		label = gtk_label_new ("Show gSTM");
	menu_item = gtk_menu_item_new ();

	gtk_container_add (GTK_CONTAINER (box), icon);
	gtk_container_add (GTK_CONTAINER (box), label);
	gtk_container_add (GTK_CONTAINER (menu_item), box);
	gtk_widget_show_all (menu_item);
	gtk_container_add (GTK_CONTAINER (menu), menu_item);

	g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (gstm_docklet_activated_cb), NULL);

	//	Separator 1
	GtkWidget *separator1 = gtk_separator_menu_item_new ();
	gtk_widget_show (separator1);
	gtk_container_add (GTK_CONTAINER (menu), separator1);
	gtk_widget_set_sensitive (separator1, FALSE);

	//	Tunnel listings
	GtkTreeIter i;
	int v_id;
	gboolean ret = gtk_tree_model_get_iter_first (GTK_TREE_MODEL (tunnellist_store), &i);

	while (ret)
	{
		gtk_tree_model_get (GTK_TREE_MODEL (tunnellist_store), &i, COL_ID, &v_id, -1);
		
		if (gSTMtunnels[v_id] != NULL)
			gstm_dockletmenu_tunnelitem_new (menu, (gchar *)gSTMtunnels[v_id]->name, v_id, gSTMtunnels[v_id]->active);
		
		ret=gtk_tree_model_iter_next (GTK_TREE_MODEL (tunnellist_store), &i);
	}

	//	Separator 2
	GtkWidget *separator2 = gtk_separator_menu_item_new ();
	gtk_widget_show (separator2);
	gtk_container_add (GTK_CONTAINER (menu), separator2);
	gtk_widget_set_sensitive (separator2, FALSE);

	//	About
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	icon = gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
	label = gtk_label_new ("About");
	menu_item = gtk_menu_item_new ();

	gtk_container_add (GTK_CONTAINER (box), icon);
	gtk_container_add (GTK_CONTAINER (box), label);
	gtk_container_add (GTK_CONTAINER (menu_item), box);

	gtk_widget_show_all (menu_item);
	gtk_container_add (GTK_CONTAINER (menu), menu_item);
	
	g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (on_dockletmenu_about_activate), NULL);

	//	Quit
	box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	icon = gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
	label = gtk_label_new ("Quit");
	menu_item = gtk_menu_item_new ();

	gtk_container_add (GTK_CONTAINER (box), icon);
	gtk_container_add (GTK_CONTAINER (box), label);
	gtk_container_add (GTK_CONTAINER (menu_item), box);

	gtk_widget_show_all (menu_item);
	gtk_container_add (GTK_CONTAINER (menu), menu_item);
	g_signal_connect ((gpointer) menu_item, "activate", G_CALLBACK (on_dockletmenu_quit_activate), NULL);

	return menu;
}
