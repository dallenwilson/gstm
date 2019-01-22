/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gAskpass/support.c
 * 
 * Copyright (c) 2005-2019	Mark Smulders <msmulders@elsar.nl>
 * Copyright (C) 2019		Dallen Wilson <dwjwilson@lavabit.com>
 *
 * gSTM is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * gSTM is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <glib/gi18n.h>

#include "support.h"
#include "main.h"

/*	This is an internally used function to create pixmaps.	*/
GtkWidget* create_pixmap (GtkWidget *widget, const gchar *filename)
{
	GtkWidget *pixmap;
	char *iconfile = malloc (strlen (gstmpixmaps) + strlen (filename) + 1);
	strcpy (iconfile, gstmpixmaps);
	strcat (iconfile, filename);

	if (!iconfile || !iconfile[0])
		return gtk_image_new ();

	if (access (iconfile, R_OK))
	{
		g_warning (_("create_pixmap: Couldn't find pixmap file: %s"), iconfile);
		return gtk_image_new ();
	}

	pixmap = gtk_image_new_from_file (iconfile);
	return pixmap;
}

/*	This is an internally used function to create pixmaps.	*/
GdkPixbuf* create_pixbuf (const gchar *filename)
{
	GdkPixbuf *pixbuf;
	GError *error = NULL;

	char *iconfile = malloc (strlen (gstmpixmaps) + strlen (filename) + 1);
	strcpy (iconfile, gstmpixmaps);
	strcat (iconfile, filename);

	if (!iconfile || !iconfile[0])
		return NULL;

	if (access (iconfile, R_OK))
	{
		g_warning (_("create_pixbuf: Couldn't find pixmap file: %s"), iconfile);
		return NULL;
	}

	pixbuf = gdk_pixbuf_new_from_file (iconfile, &error);
	if (!pixbuf)
	{
		fprintf (stderr, "Failed to load pixbuf file: %s: %s\n",
		iconfile, error->message);
		g_error_free (error);
	}

	return pixbuf;
}