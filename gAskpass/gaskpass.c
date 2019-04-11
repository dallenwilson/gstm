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
GtkWidget *entry = NULL;

G_DEFINE_TYPE(gAskpass, gaskpass, GTK_TYPE_APPLICATION);

static void
gaskpass_init (gAskpass *obj)
{
}

static void
gaskpass_activate (GApplication *app)
{
	GtkWidget *content, *label;

	GdkPixbuf *icon = create_pixbuf ("gSTM.png");
	
	GtkDialogFlags flags = GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT;
	dialog = gtk_dialog_new_with_buttons ("Enter password", NULL, flags,
	                                      "_Cancel", GTK_RESPONSE_REJECT,
	                                      "_OK", GTK_RESPONSE_ACCEPT,
	                                      NULL);

	gtk_window_set_resizable (GTK_WINDOW (dialog), FALSE);
	gtk_window_set_icon (GTK_WINDOW (dialog), icon);

	content = gtk_dialog_get_content_area (GTK_DIALOG (dialog));

	label = gtk_label_new ("Please enter your SSH password:");
	gtk_container_add (GTK_CONTAINER (content), label);
	
	entry = gtk_entry_new ();
	gtk_entry_set_visibility (GTK_ENTRY (entry), FALSE);
	gtk_entry_set_input_purpose (GTK_ENTRY (entry), GTK_INPUT_PURPOSE_PASSWORD);
	gtk_entry_set_icon_from_icon_name (GTK_ENTRY (entry),
	                                   GTK_ENTRY_ICON_SECONDARY,
	                                   "dialog-password");

	if (curpass)
		gtk_label_set_text (GTK_LABEL (label), curpass);

	gtk_container_add (GTK_CONTAINER (content), entry);
	gtk_widget_show_all (dialog);

	gtk_window_set_application (GTK_WINDOW (dialog), GTK_APPLICATION (app));

	gaskpass_init ((gAskpass *)app);

	int result = gtk_dialog_run (GTK_DIALOG (dialog));

	switch (result)
	{
	case GTK_RESPONSE_ACCEPT:
		puts (gtk_entry_get_text (GTK_ENTRY (entry)));
		break;
	default:
		break;
	}
	
	gtk_widget_destroy (dialog);

	g_application_quit (app);
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
