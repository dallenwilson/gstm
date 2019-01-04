#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "callbacks.h"
#include "interface.h"
#include "support.h"


void on_okbutton1_clicked (GtkButton *button, gpointer user_data){
	extern GtkWidget *askpassdialog;
	GtkEntry *passentry;
	passentry = (GtkEntry *)lookup_widget(askpassdialog,"passentry");
	puts(passentry->text);
	gtk_main_quit();
}

void on_passentry_activate (GtkEntry *entry, gpointer user_data){
	on_okbutton1_clicked(NULL, user_data);
}
