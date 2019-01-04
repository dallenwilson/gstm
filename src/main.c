/***************************************************************************
 *            main.c
 *
 *  Thu Jul 28 14:57:32 2005
 *  Copyright  2005  Mark Smulders
 *  msmulders@elsar.nl
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gnome.h>
#include <signal.h>

#include "interface.h"
#include "fniface.h"
#include "support.h"

#include "conffile.h"

#include "common.h"
#include "eggtrayicon.h"

GtkWidget *maindialog;
GtkWidget *prop;
GtkListStore *tunnellist_store=NULL;
char *gstmdir=NULL;
extern EggTrayIcon *docklet;

void signalexit(int sig_num);
int gstm_init(void);
void gstm_init_treeview(GtkTreeView *view);
void gstm_populate_treeview(GtkWidget *dialog, const char *objname, struct sshtunnel **STMtunnels, int tcnt);
int gstm_process_autostart(struct sshtunnel **STMtunnels, int tcnt);
void gstm_quit();

/**
  ***************************************
  MAIN
  ***************************************
**/
int main (int argc, char *argv[]) {
	struct sshtunnel *thetunnel;
	int a_cnt=0;
	
	//initialize gthreads if nescesarry
	if (!g_thread_supported ()) g_thread_init (NULL);
	gdk_threads_init();

    LIBXML_TEST_VERSION
	
#ifdef ENABLE_NLS
  bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
  bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
  textdomain (GETTEXT_PACKAGE);
#endif

  gnome_program_init (PACKAGE, VERSION, LIBGNOMEUI_MODULE,
                      argc, argv,
                      GNOME_PARAM_APP_DATADIR, PACKAGE_DATA_DIR,
                      NULL);

	//init
	gstm_init();
	
	//read tunnelfiles into memory
	tunnelCount = gstm_readfiles(gstmdir, &gSTMtunnels);
	
	//create main dialog objects
	maindialog = create_maindialog ();
	
	//init list
	gstm_init_treeview(GTK_TREE_VIEW(lookup_widget(maindialog,"tunnellist")));
	
	//if there are tunnels, populate the list
	if (tunnelCount>0) {
		gstm_populate_treeview(maindialog, "tunnellist", gSTMtunnels, tunnelCount);
		a_cnt = gstm_process_autostart(gSTMtunnels,tunnelCount);
	}
	
	//create the notification area icon
	docklet_x11_create();
	//show the main dialog
	// * if there's a notification area AND there are one or more 'autostart' tunnels
	//   then maindialog is hidden (ie 'start minimized to tray')
	if (a_cnt==0 || docklet->manager_window==None) {
		gtk_widget_show (maindialog);
	}
	
	//ready for action
	gstm_interface_showinfo("gSTM ready for action.");

	//connect signal handlers before going into the gtk_main loop
	signal(SIGTERM, signalexit);
	//signal(SIGABRT, signalexit);
	signal(SIGINT, signalexit);

	//run forest, run
	gdk_threads_enter();
	gtk_main ();
	gdk_threads_leave();
	
	return 0;
}

/*****************************************************************************************/
/*****************************************************************************************/

int gstm_init(void) {
    struct stat sb;
		
	// get HOME variable and construct gSTM dir
	gstmdir = malloc(strlen(getenv("HOME"))+6+1);
	if (!gstmdir) {
		fprintf(stderr,"** out of memory\n");
		exit(EXIT_FAILURE);
	}
	strcpy(gstmdir, getenv("HOME"));
	strcat(gstmdir,"/.gSTM");

	// check if gSTM dir exists or create it
	if (access(gstmdir,W_OK)) {
		// can't access it, although it might exist try to create it
		mkdir(gstmdir,0755);
		if (access(gstmdir,W_OK)) {
			//still can't access it :(
			fprintf(stderr,"** .gSTM directory in your HOME directory is not accessible\n");
			exit(EXIT_FAILURE);
		}
	} else {
		// check if it is really a directory ;)
		stat(gstmdir,&sb);
		if (!S_ISDIR(sb.st_mode)) {
			fprintf(stderr,"** a file called .gSTM exists in your HOME directory, please delete it.\n");
			exit(EXIT_FAILURE);
		}
	}
	
	gSTMtunnels=NULL;
	tunnelCount=0;
	activeCount=0;
}

void gstm_init_treeview(GtkTreeView *view) {
	GtkTreeSelection *selection;
	GtkCellRenderer *renderer;
	
	if (view) {
		
		// disable multiple select
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(view));
		gtk_tree_selection_set_mode(selection, GTK_SELECTION_SINGLE);
			
		/* --- Column #1 --- */
		renderer = gtk_cell_renderer_pixbuf_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
												   -1,      
												   "",  
												   renderer,
												   "pixbuf", COL_ACTIVE,
												   NULL);
		
		
		/* --- Column #2 --- */
		renderer = gtk_cell_renderer_text_new ();
		gtk_tree_view_insert_column_with_attributes (GTK_TREE_VIEW (view),
												   -1,      
												   "Tunnel",  
												   renderer,
												   "text", COL_NAME,
												   NULL);
		
		tunnellist_store = gtk_list_store_new (N_COLS, GDK_TYPE_PIXBUF, G_TYPE_STRING, G_TYPE_INT);
		
		//put it in
		gtk_tree_view_set_model (GTK_TREE_VIEW (view), GTK_TREE_MODEL(tunnellist_store));
	}
}

int gstm_process_autostart(struct sshtunnel **STMtunnels, int tcnt) {
	int i;
	int s_cnt=0;
	for (i=0; i<tcnt; i++) {
		if (STMtunnels[i]!=NULL && STMtunnels[i]->autostart) {
			gstm_interface_paint_row_id(i,!STMtunnels[i]->active);
			//gstm_interface_enablebuttons(!STMtunnels[i]->active);
			gstm_ssht_starttunnel(i);
			s_cnt++;
		}
	}
	return s_cnt;
}

void gstm_populate_treeview(GtkWidget *dialog, const char *objname, struct sshtunnel **STMtunnels, int tcnt) {
	GtkCellRenderer     *renderer;
	GtkWidget           *view;
	GtkTreeIter    iter;
	GtkTreeSelection* selection;
	GdkPixbuf *pixbuf_yellow, *pixbuf_red, *pixbuf_green;
	GError       *error = NULL;
	int i;
	
	if (view = lookup_widget(dialog, objname)) {
		
		pixbuf_red = create_pixbuf("gstm/red.xpm");
		pixbuf_yellow = create_pixbuf("gstm/yellow.xpm");
		pixbuf_green = create_pixbuf("gstm/green.xpm");
		
		for (i=0; i<tcnt; i++) {
			if (STMtunnels[i]!=NULL) {
				/* Append a row and fill in data */
				gtk_list_store_append (tunnellist_store, &iter);
				if (STMtunnels[i]->active) {
					gtk_list_store_set (tunnellist_store, &iter,
									  COL_ACTIVE, pixbuf_green,
									  COL_NAME, STMtunnels[i]->name,
									  COL_ID, i,
									  -1);
				} else {
					gtk_list_store_set (tunnellist_store, &iter,
									  COL_ACTIVE, pixbuf_red,
									  COL_NAME, STMtunnels[i]->name,
									  COL_ID, i,
									  -1);
				}
			}
		}
		
		//sort it
		gtk_tree_sortable_set_sort_column_id(GTK_TREE_SORTABLE(tunnellist_store),
					 COL_NAME,
					 GTK_SORT_ASCENDING);		
	}
}

void gstm_quit() {
	int i;
	//stop all tunnels
	for (i=0; i<tunnelCount; i++) {
		gstm_ssht_stoptunnel(i);
	}
	//free all dynamically allocated memory
	gstm_freetunnels(&gSTMtunnels, tunnelCount);
	tunnelCount=0;
	//free store
	if(tunnellist_store) g_object_unref (tunnellist_store);
	//remove the docklet
	docklet_x11_destroy();
	// exit
	gtk_main_quit();
}

void signalexit(int sig_num) {
	//printf("!SIGNAL HANDLER!\n");
	gstm_quit();
}
