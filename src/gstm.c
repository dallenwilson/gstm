/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/gstm.c
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

#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <glib/gi18n.h>

#include "common.h"
#include "conffile.h"
#include "main.h"
#include "gstm.h"
#include "systray.h"
#include "callbacks.h"
#include "fniface.h"
#include "fnssht.h"

#define TOP_WINDOW "maindialog"
G_DEFINE_TYPE (Gstm, gstm, GTK_TYPE_APPLICATION);

int maindiag_width;
int maindiag_height;

/* ANJUTA: Macro GSTM_APPLICATION gets Gstm - DO NOT REMOVE */
struct _GstmPrivate
{
	/* ANJUTA: Widgets declaration for gstm.ui - DO NOT REMOVE */
};

GtkBuilder *builder;

GtkWidget *maindialog = NULL;
GtkWidget *aboutdialog = NULL;
GtkWidget *newdialog = NULL;
GtkWidget *tundialog = NULL;
GtkWidget *propertiesdialog = NULL;

GtkWidget *statusbar = NULL;
GtkWidget *tunlist = NULL;
GtkImage *banner = NULL;

GtkListStore *tunnellist_store = NULL;

/* Create a new window loading a file */
static void
gstm_new_window (GApplication *app)
{
	GError* error = NULL;

	/*	Load UI from file	*/
	builder = gtk_builder_new ();
	
	if (!gtk_builder_add_from_file (builder, gstmui, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/*	Auto-connect signal handlers */
	gtk_builder_connect_signals (builder, app);

	/*	Get handles for various windows	*/
	maindialog = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
	aboutdialog = GTK_WIDGET (gtk_builder_get_object (builder, "aboutdialog"));
	newdialog = GTK_WIDGET (gtk_builder_get_object (builder, "newdialog"));
	tundialog = GTK_WIDGET (gtk_builder_get_object (builder, "tundialog"));
	propertiesdialog = GTK_WIDGET (gtk_builder_get_object (builder, "propertiesdialog"));
	
	statusbar = GTK_WIDGET (gtk_builder_get_object (builder, "statusbar"));
	tunlist = GTK_WIDGET (gtk_builder_get_object (builder, "tunnellist"));
	banner = GTK_IMAGE (gtk_builder_get_object (builder, "logo"));

	gtk_window_set_transient_for (GTK_WINDOW (aboutdialog), GTK_WINDOW (maindialog));
	gtk_window_set_transient_for (GTK_WINDOW (newdialog), GTK_WINDOW (maindialog));
	gtk_window_set_transient_for (GTK_WINDOW (tundialog), GTK_WINDOW (maindialog));
	gtk_window_set_transient_for (GTK_WINDOW (propertiesdialog), GTK_WINDOW (maindialog));

	gtk_window_set_application (GTK_WINDOW (maindialog), GTK_APPLICATION (app));
}

/* GApplication implementation */
static void
gstm_activate (GApplication *application)
{
	int a_cnt = 0;

	gstm_init ((Gstm *)app);
	
	gstm_new_window (application);

	//read tunnelfiles into memory
	tunnelCount = gstm_readfiles (gstmdir, &gSTMtunnels);

	//init list
	gstm_init_treeview (GTK_TREE_VIEW (tunlist));

	//if there are tunnels, populate the list
	if (tunnelCount > 0)
	{
		gstm_populate_treeview (maindialog, "tunnellist", gSTMtunnels, tunnelCount);
		a_cnt = gstm_process_autostart (gSTMtunnels, tunnelCount);
	}

	//	set window icon
	GdkPixbuf *pbicon = create_pixbuf ("gSTM.png");
	gtk_window_set_icon (GTK_WINDOW (maindialog), pbicon);

	//	set banner
	GdkPixbuf *pbbanner = create_pixbuf ("STMbanner.png");
	gtk_image_set_from_pixbuf (banner, pbbanner);
	
	//	create the notification area icon
	gstm_docklet_create();

	/*	if there's a notification area AND there are one or more 'autostart'
	 *	tunnels then maindialog is hidden (ie 'start minimized to tray') */
	if ((a_cnt == 0) || !(IS_APP_INDICATOR(ci) && APP_INDICATOR_STATUS_ACTIVE == app_indicator_get_status(ci)))
		gtk_widget_show_all (GTK_WIDGET (maindialog));

	//	load saved window size, if any
	gstm_load_window_size ();

	//	set the window size
	gtk_window_resize (GTK_WINDOW (maindialog), maindiag_width, maindiag_height);

	//ready for action
	gstm_interface_showinfo("gSTM ready for action.");
}

static void gstm_init (Gstm *object)
{
	gSTMtunnels = NULL;
	tunnelCount = 0;
	activeCount = 0;
}

static void
gstm_finalize (GObject *object)
{
	G_OBJECT_CLASS (gstm_parent_class)->finalize (object);
}

static void
gstm_class_init (GstmClass *klass)
{
	G_APPLICATION_CLASS (klass)->activate = gstm_activate;
	G_OBJECT_CLASS (klass)->finalize = gstm_finalize;
}

Gstm *
gstm_new (void)
{
	return g_object_new (gstm_get_type (),
	                     "application-id", "org.gtk.gstm",
	                     "flags", G_APPLICATION_HANDLES_OPEN,
	                     NULL);
}

void gstm_init_treeview (GtkTreeView *view)
{
	GtkTreeSelection *selection = NULL;
	GtkCellRenderer *renderer = NULL;
	
	if (view)
	{
		// disable multiple select
		selection = gtk_tree_view_get_selection (GTK_TREE_VIEW (view));
		gtk_tree_selection_set_mode (selection, GTK_SELECTION_SINGLE);
			
		/* --- Column #1 --- */
		renderer = gtk_cell_renderer_pixbuf_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), -1,
		                                             "", renderer, "pixbuf",
		                                             COL_ACTIVE, NULL);

		/* --- Column #2 --- */
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view), -1,
		                                             "Tunnel",  renderer,
		                                             "text", COL_NAME, NULL);

		tunnellist_store = gtk_list_store_new (N_COLS, GDK_TYPE_PIXBUF,
		                                       G_TYPE_STRING, G_TYPE_INT);
		
		//put it in
		gtk_tree_view_set_model (GTK_TREE_VIEW (view),
		                         GTK_TREE_MODEL (tunnellist_store));
	}
}

int gstm_process_autostart (struct sshtunnel **STMtunnels, int tcnt)
{
	int i;
	int s_cnt = 0;
	
	for (i = 0; i < tcnt; i++)
	{
		if (STMtunnels[i] != NULL && STMtunnels[i]->autostart)
		{
			gstm_interface_paint_row_id (i, !STMtunnels[i]->active);
			//gstm_interface_enablebuttons(!STMtunnels[i]->active);
			gstm_ssht_starttunnel (i);
			s_cnt++;
		}
	}
	
	return s_cnt;
}

void gstm_populate_treeview (GtkWidget *dialog, const char *objname,
                             struct sshtunnel **STMtunnels, int tcnt)
{
	GtkTreeIter iter;
	GdkPixbuf *pixbuf_red, *pixbuf_green;
	int i;

	pixbuf_red = create_pixbuf_scaled ("red.svg", GTK_ICON_SIZE_MENU);
	pixbuf_green = create_pixbuf_scaled ("green.svg", GTK_ICON_SIZE_MENU);

	for (i = 0; i < tcnt; i++)
	{
		if (STMtunnels[i] != NULL)
		{
			/* Append a row and fill in data */
			gtk_list_store_append (tunnellist_store, &iter);
			if (STMtunnels[i]->active)
				gtk_list_store_set (tunnellist_store, &iter, COL_ACTIVE,
				                    pixbuf_green, COL_NAME,
				                    STMtunnels[i]->name, COL_ID, i, -1);
			
			else
				gtk_list_store_set (tunnellist_store, &iter, COL_ACTIVE,
				                    pixbuf_red, COL_NAME,
				                    STMtunnels[i]->name, COL_ID, i, -1);
			
		}
	}

	//sort it
	gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE(tunnellist_store),
	                                      COL_NAME, GTK_SORT_ASCENDING);
}

void gstm_quit()
{
	gstm_store_window_size ();
	
	int i;
	
	//stop all tunnels
	for (i=0; i<tunnelCount; i++) {
		gstm_ssht_stoptunnel(i);
	}

	//free all dynamically allocated memory
	gstm_freetunnels(&gSTMtunnels, tunnelCount);
	tunnelCount=0;

	//free store
	if (tunnellist_store)
		g_object_unref (tunnellist_store);

	// exit
	g_application_quit ( G_APPLICATION (app));
}
