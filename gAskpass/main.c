/*
 * gAskpass main.c 
 * (c) Mark Smulders 2005
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>

#include "interface.h"
#include "support.h"

GtkWidget *askpassdialog;

int main (int argc, char **argv) {

#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);
  if (argc>1)
  	askpassdialog = create_askpassdialog (argv[1]);
  else
  	askpassdialog = create_askpassdialog (NULL);
	  
  gtk_widget_show (askpassdialog);

  gtk_main ();
  return 0;
}
