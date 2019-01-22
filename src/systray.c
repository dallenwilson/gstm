/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/systray.c
 *
 * Copyright (c) 2005-2019	Mark Smulders <msmulders@elsar.nl>
 * Copyright (C) 2019		Dallen Wilson <dwjwilson@lavabit.com>
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
#include <glib/gi18n.h>

#include "systray.h"
#include "conffile.h"
#include "gstm.h"
#include "callbacks.h"
#include "main.h"
#include "support.h"

//AppIndicator *ci = NULL;
GtkStatusIcon *ci = NULL;

void gstm_docklet_create ()
{
	/* TODO: Transition away from depreceated GtkStatusIcon.
	 * AppIndicator is one option if left/right mouseclick events can be done.
	 * Disabling warnings for this section in the meantime.	*/
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	//	Create systray icon

	GdkPixbuf *pb = create_pixbuf ("gSTM.png");
	ci = gtk_status_icon_new_from_pixbuf (pb);

	gtk_status_icon_set_title (ci, "gSTM");
	gtk_status_icon_set_name (ci, "gSTM");

	//	Connect handlers
    g_signal_connect ((gpointer) ci, "activate", G_CALLBACK (gstm_docklet_activated_cb), NULL);
    g_signal_connect ((gpointer) ci, "popup-menu", G_CALLBACK (gstm_docklet_popupmenu_cb), NULL);

	//	AppIndicator alternative
	//ci = app_indicator_new ("gSTM", DEFAULT_ICON, APP_INDICATOR_CATEGORY_APPLICATION_STATUS);
	//g_assert (IS_APP_INDICATOR (ci));
	//g_assert (G_IS_OBJECT (ci));
	//app_indicator_set_status (ci, APP_INDICATOR_STATUS_ACTIVE);

	#pragma GCC diagnostic pop
}

void gstm_toggle_mainwindow ()
{
	if (gtk_widget_is_visible (maindialog))
		gtk_widget_hide (maindialog);
	else
		gtk_widget_show (maindialog);
}

void gstm_docklet_activated_cb (GtkWidget *widget, gpointer user_data)
{
	gstm_toggle_mainwindow();
}

void gstm_docklet_popupmenu_cb (GtkWidget *widget, gpointer user_data)
{
	GtkMenu *newmenu = gstm_docklet_menu_regen ();
	gtk_menu_popup_at_pointer (newmenu, NULL);
}

void gstm_dockletmenu_tunnelitem_new (GtkMenu *menu, const gchar *t_name,
                                      intptr_t t_id, gboolean t_active)
{
	/* TODO: Transition away from depreceated gtk_image_menu* functions.
	 * Disabling warnings for this section in the meantime.	*/
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
	GtkWidget *item_tunnel;
	GtkWidget *img_yesno;
	GdkPixbuf *pb;

	if (t_active)
		pb = create_pixbuf("green.xpm");
	else
		pb = create_pixbuf("red.xpm");
	
	img_yesno = gtk_image_new_from_pixbuf(pb);

	gtk_widget_show (img_yesno);

	item_tunnel = gtk_image_menu_item_new_with_mnemonic (t_name);
	gtk_widget_show (item_tunnel);
	gtk_container_add (GTK_CONTAINER (menu), item_tunnel);
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_tunnel), img_yesno);

	g_signal_connect ((gpointer) item_tunnel, "activate",
	                  G_CALLBACK (on_dockletmenu_tunnel_activate),
	                  (gpointer)t_id);

	#pragma GCC diagnostic pop
}

GtkMenu* gstm_docklet_menu_regen ()
{
	GtkMenu *menu = GTK_MENU (gtk_menu_new ());
	
	/* TODO: Transition away from depreceated gtk_image_menu* functions.
	 * See comment block at bottom for one possible path in Gtk3.
	 * Disabling warnings for this section in the meantime.	*/
	#pragma GCC diagnostic push
	#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

	// Show/Hide
	GtkWidget *image_toggle = gtk_image_new_from_icon_name ("dialog-password", GTK_ICON_SIZE_MENU);
	GtkWidget *item_toggle = NULL;
	if (gtk_widget_is_visible (maindialog))
		item_toggle = gtk_image_menu_item_new_with_mnemonic ("Hide gSTM");
	else
		item_toggle = gtk_image_menu_item_new_with_mnemonic ("Show gSTM");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_toggle), image_toggle);
	gtk_widget_show_all (item_toggle);
	gtk_container_add (GTK_CONTAINER (menu), item_toggle);
	g_signal_connect ((gpointer) item_toggle, "activate", G_CALLBACK (gstm_docklet_activated_cb), NULL);

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
	GtkWidget *image_about = gtk_image_new_from_icon_name ("help-about", GTK_ICON_SIZE_MENU);
	GtkWidget *item_about = gtk_image_menu_item_new_with_mnemonic ("About");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_about), image_about);
	gtk_widget_show_all (item_about);
	gtk_container_add (GTK_CONTAINER (menu), item_about);
	g_signal_connect ((gpointer) item_about, "activate", G_CALLBACK (on_dockletmenu_about_activate), NULL);

	//	Quit
	GtkWidget *image_quit = gtk_image_new_from_icon_name ("application-exit", GTK_ICON_SIZE_MENU);
	GtkWidget *item_quit = gtk_image_menu_item_new_with_mnemonic ("Quit");
	gtk_image_menu_item_set_image (GTK_IMAGE_MENU_ITEM (item_quit), image_quit);
	gtk_widget_show_all (item_quit);
	gtk_container_add (GTK_CONTAINER (menu), item_quit);
	g_signal_connect ((gpointer) item_quit, "activate", G_CALLBACK (on_dockletmenu_quit_activate), NULL);

	#pragma GCC diagnostic pop

	return menu;

	///app_indicator_set_menu (ci, GTK_MENU (menu));

	/*
	//	Work-around for gtk_image_menu* being depreceated in gtk3
	//	Saving here for gtk4 use one day
	GtkWidget *box = gtk_box_new (GTK_ORIENTATION_HORIZONTAL, 6);
	GtkWidget *icon = gtk_image_new_from_icon_name ("folder-music-symbolic", GTK_ICON_SIZE_MENU);
	GtkWidget *label = gtk_label_new ("Music");
	GtkWidget *menu_item = gtk_menu_item_new ();

	gtk_container_add (GTK_CONTAINER (box), icon);
	gtk_container_add (GTK_CONTAINER (box), label);
	gtk_container_add (GTK_CONTAINER (menu_item), box);

	gtk_widget_show_all (menu_item);
	gtk_container_add (GTK_CONTAINER (menu), menu_item);
	*/
}