/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gAskpass/interface.c
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
/*
#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"

#define GLADE_HOOKUP_OBJECT(component,widget,name) \
  g_object_set_data_full (G_OBJECT (component), name, \
    gtk_widget_ref (widget), (GDestroyNotify) gtk_widget_unref)

#define GLADE_HOOKUP_OBJECT_NO_REF(component,widget,name) \
  g_object_set_data (G_OBJECT (component), name, widget)

GtkWidget*
create_askpassdialog (char *question)
{
  GtkWidget *askpassdialog;
  GtkWidget *dialog_vbox1;
  GtkWidget *fixed1;
  GtkWidget *passentry;
  GtkWidget *image1;
  GtkWidget *label1;
  GtkWidget *dialog_action_area1;
  GtkWidget *cancelbutton1;
  GtkWidget *okbutton1;

  askpassdialog = gtk_dialog_new ();
  gtk_window_set_title (GTK_WINDOW (askpassdialog), _("Enter password"));
  gtk_window_set_position (GTK_WINDOW (askpassdialog), GTK_WIN_POS_CENTER_ALWAYS);
  gtk_window_set_modal (GTK_WINDOW (askpassdialog), TRUE);
  gtk_window_set_type_hint (GTK_WINDOW (askpassdialog), GDK_WINDOW_TYPE_HINT_DIALOG);

  //dialog_vbox1 = GTK_DIALOG (askpassdialog)->vbox;
  //gtk_widget_show (dialog_vbox1);

  fixed1 = gtk_fixed_new ();
  gtk_widget_show (fixed1);
  gtk_box_pack_start (GTK_BOX (dialog_vbox1), fixed1, TRUE, TRUE, 0);

  passentry = gtk_entry_new ();
  gtk_widget_show (passentry);
  gtk_fixed_put (GTK_FIXED (fixed1), passentry, 8, 36);
  gtk_widget_set_size_request (passentry, 288, 24);
  gtk_entry_set_visibility (GTK_ENTRY (passentry), FALSE);

  image1 = gtk_image_new_from_stock ("gtk-dialog-authentication", GTK_ICON_SIZE_DIALOG);
  gtk_widget_show (image1);
  gtk_fixed_put (GTK_FIXED (fixed1), image1, 296, 16);
  gtk_widget_set_size_request (image1, 48, 48);

  if (question==NULL)
  	label1 = gtk_label_new (_("Please enter your password:"));
  else
	label1 = gtk_label_new (question);
  gtk_widget_show (label1);
  gtk_fixed_put (GTK_FIXED (fixed1), label1, 8, 16);
  gtk_widget_set_size_request (label1, 336, 16);
  gtk_label_set_line_wrap (GTK_LABEL (label1), TRUE);
  gtk_label_set_single_line_mode (GTK_LABEL (label1), TRUE);

  dialog_action_area1 = GTK_DIALOG (askpassdialog)->action_area;
  gtk_widget_show (dialog_action_area1);
  gtk_button_box_set_layout (GTK_BUTTON_BOX (dialog_action_area1), GTK_BUTTONBOX_END);

  cancelbutton1 = gtk_button_new_from_stock ("gtk-cancel");
  gtk_widget_show (cancelbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (askpassdialog), cancelbutton1, GTK_RESPONSE_CANCEL);
  //GTK_WIDGET_SET_FLAGS (cancelbutton1, GTK_CAN_DEFAULT);

  okbutton1 = gtk_button_new_from_stock ("gtk-ok");
  gtk_widget_show (okbutton1);
  gtk_dialog_add_action_widget (GTK_DIALOG (askpassdialog), okbutton1, GTK_RESPONSE_OK);
  //GTK_WIDGET_SET_FLAGS (okbutton1, GTK_CAN_DEFAULT);

  g_signal_connect ((gpointer) askpassdialog, "close",
                    G_CALLBACK (gtk_main_quit),
                    NULL);
  g_signal_connect ((gpointer) askpassdialog, "remove",
                    G_CALLBACK (gtk_main_quit),
                    NULL);
  g_signal_connect ((gpointer) passentry, "activate",
                    G_CALLBACK (on_passentry_activate),
                    NULL);
  g_signal_connect ((gpointer) cancelbutton1, "clicked",
                    G_CALLBACK (gtk_main_quit),
                    NULL);
  g_signal_connect ((gpointer) okbutton1, "clicked",
                    G_CALLBACK (on_okbutton1_clicked),
                    NULL);

  // Store pointers to all widgets, for use by lookup_widget().
  GLADE_HOOKUP_OBJECT_NO_REF (askpassdialog, askpassdialog, "askpassdialog");
  GLADE_HOOKUP_OBJECT_NO_REF (askpassdialog, dialog_vbox1, "dialog_vbox1");
  GLADE_HOOKUP_OBJECT (askpassdialog, fixed1, "fixed1");
  GLADE_HOOKUP_OBJECT (askpassdialog, passentry, "passentry");
  GLADE_HOOKUP_OBJECT (askpassdialog, image1, "image1");
  GLADE_HOOKUP_OBJECT (askpassdialog, label1, "label1");
  GLADE_HOOKUP_OBJECT_NO_REF (askpassdialog, dialog_action_area1, "dialog_action_area1");
  GLADE_HOOKUP_OBJECT (askpassdialog, cancelbutton1, "cancelbutton1");
  GLADE_HOOKUP_OBJECT (askpassdialog, okbutton1, "okbutton1");*

  return askpassdialog;
}
*/