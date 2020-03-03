/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gAskpass/gaskpass.c
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

#include <gtk/gtk.h>

#include "main.h"
#include "gaskpass.h"

char *curpass = NULL;

GtkWidget *dialog = NULL;
GtkWidget *label = NULL;
GtkWidget *entry = NULL;

G_DEFINE_TYPE(gAskpass, gaskpass, GTK_TYPE_APPLICATION);
#define TOP_WINDOW "gaskpass_main"
GtkBuilder *builder;

static void
gaskpass_init (gAskpass *obj)
{
}

/* Create a new window loading a file */
static void
gaskpass_new_window (GApplication *app)
{
	GError* error = NULL;

	/*	Load UI from file	*/
	builder = gtk_builder_new ();
	
	if (!gtk_builder_add_from_file (builder, gstmui, &error))
	{
		g_critical ("Couldn't load builder file: %s", error->message);
		g_error_free (error);
	}

	/*	Auto-connect signal handlers =	*/
	gtk_builder_connect_signals (builder, app);

	/*	Get handles for various windows	*/
	dialog = GTK_WIDGET (gtk_builder_get_object (builder, TOP_WINDOW));
	label = GTK_WIDGET (gtk_builder_get_object (builder, "lbl_ssh_message"));
	entry = GTK_WIDGET (gtk_builder_get_object (builder, "txt_password"));
	
	gtk_window_set_application (GTK_WINDOW (dialog), GTK_APPLICATION (app));
}

static void
gaskpass_activate (GApplication *app)
{
	gaskpass_init ((gAskpass *)app);
	
	gaskpass_new_window (app);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	GdkPixbuf *pbicon = create_pixbuf ("gSTM.png");
	gtk_window_set_icon (GTK_WINDOW (dialog), pbicon);

	if (curpass)
		gtk_label_set_text (GTK_LABEL (label), curpass);
	
	gtk_widget_show_all (GTK_WIDGET (dialog));
}

void gaskpass_cb_cancel (GtkButton *button, gpointer user_data)
{
	gtk_widget_destroy (dialog);
	g_application_quit (G_APPLICATION (app));
}

void gaskpass_cb_ok (GtkButton *button, gpointer user_data)
{
	puts (gtk_entry_get_text ( GTK_ENTRY(entry)));
	gtk_widget_destroy (dialog);
	g_application_quit (G_APPLICATION (app));
}

static int
commandLine (GApplication            *app,
             GApplicationCommandLine *cmdline)
{
	gchar **argv;
	gint argc;
	
	argv = g_application_command_line_get_arguments (cmdline, &argc);

	if ( argc > 1 )
	{
		curpass = malloc (strlen(argv[1]) + 1);
		strcpy (curpass, argv[1]);
	}

	g_strfreev (argv);

	g_application_activate (app);
	
	return 0;
}

static void
gaskpass_class_init (gAskpassClass *class)
{
	G_APPLICATION_CLASS (class)->activate = gaskpass_activate;
}

gAskpass * gaskpass_new (void)
{
	gAskpass *app = g_object_new (gaskpass_get_type(),
	                              "application-id", "org.gtk.gaskpass",
	                              "flags", G_APPLICATION_HANDLES_COMMAND_LINE,
	                              NULL);
	g_signal_connect (app, "command-line", G_CALLBACK (commandLine), NULL);

	return app;
}
