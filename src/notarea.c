/***************************************************************************
 *            notarea.c
 *
 *  Thu Aug  4 14:57:32 2005
 *  Copyright  2005  Mark Smulders
 *  msmulders@elsar.nl
 ****************************************************************************/
 /*
 * System tray icon for gSTM
 *
 * Inspired by the Gaim plugin:
 * Robert McQueen <robot101@debian.org>
 * Herman Bloggs <hermanator12002@yahoo.com>
 * Inspired by a similar plugin by:
 *  John (J5) Palmieri <johnp@martianrock.com>
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA.
 */
 
#include <gtk/gtk.h>
#include "eggtrayicon.h"
EggTrayIcon *docklet = NULL;
static GtkWidget *image = NULL;
extern void docklet_clicked(int);

static void docklet_x11_destroyed_cb(GtkWidget *widget, void *data);

static void
docklet_x11_clicked_cb(GtkWidget *button, GdkEventButton *event, void *data)
{
	if (event->type != GDK_BUTTON_PRESS)
		return;

	docklet_clicked(event->button);
}

void
docklet_x11_destroy()
{
	g_signal_handlers_disconnect_by_func(G_OBJECT(docklet),
		G_CALLBACK(docklet_x11_destroyed_cb), NULL);
	gtk_widget_destroy(GTK_WIDGET(docklet));

	g_object_unref(G_OBJECT(docklet));
	docklet = NULL;
}

void
docklet_x11_create()
{
	GtkWidget *box;
	GdkPixbuf *pb, *pbsmall;

	if (docklet) {
		/* if this is being called when a tray icon exists, it's because
		   something messed up. try destroying it before we proceed,
		   although docklet_refcount may be all hosed. hopefully won't happen. */
		docklet_x11_destroy();
	}

	docklet = egg_tray_icon_new("gSTM");
	box = GTK_WIDGET(gtk_event_box_new());
	image = GTK_WIDGET(gtk_image_new());

	g_signal_connect(G_OBJECT(docklet), "destroy", G_CALLBACK(docklet_x11_destroyed_cb), NULL);
	g_signal_connect(G_OBJECT(box), "button-press-event", G_CALLBACK(docklet_x11_clicked_cb), NULL);

	gtk_container_add(GTK_CONTAINER(box), image);
	gtk_container_add(GTK_CONTAINER(docklet), box);

	if(!gtk_check_version(2,4,0))
		g_object_set(G_OBJECT(box), "visible-window", FALSE, NULL);

	gtk_widget_show_all(GTK_WIDGET(docklet));

	/* ref the docklet before we bandy it about the place */
	g_object_ref(G_OBJECT(docklet));

	/* set gSTM icon */
	pb = GDK_PIXBUF(create_pixbuf("gstm/gSTM.xpm"));
	/* resize the icon to a more appropriate size */
	pbsmall = gdk_pixbuf_scale_simple(pb,24,24,GDK_INTERP_BILINEAR); //FIXME? static size
	
	gtk_image_set_from_pixbuf(GTK_IMAGE(image),pbsmall);
}

static gboolean
docklet_x11_create_cb()
{
	docklet_x11_create();
	
	return FALSE; /* for when we're called by the glib idle handler */
}

static void
docklet_x11_destroyed_cb(GtkWidget *widget, void *data)
{

	g_object_unref(G_OBJECT(docklet));
	docklet = NULL;

	g_idle_add(docklet_x11_create_cb, NULL);
}

void
docklet_x11_position_menu(GtkMenu *menu, int *x, int *y, gboolean *push_in,
						  gpointer user_data)
{
	GtkWidget *widget = GTK_WIDGET(docklet);
	GtkRequisition req;
	gint menu_xpos, menu_ypos;

	gtk_widget_size_request(GTK_WIDGET(menu), &req);
	gdk_window_get_origin(widget->window, &menu_xpos, &menu_ypos);

	menu_xpos += widget->allocation.x;
	menu_ypos += widget->allocation.y;

	if (menu_ypos > gdk_screen_get_height(gtk_widget_get_screen(widget)) / 2)
		menu_ypos -= req.height;
	else
		menu_ypos += widget->allocation.height;

	*x = menu_xpos;
	*y = menu_ypos;

	*push_in = TRUE;
}
