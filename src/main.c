/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/main.c
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

#include <config.h>
#include <signal.h>
#include <sys/stat.h>
#include <glib/gi18n.h>

#include "main.h"
#include "gstm.h"
#include "systray.h"
#include "conffile.h"
#include "fniface.h"
#include "fnssht.h"

Gstm *app;

char *gstmdir = NULL;
char *sshdir = NULL;
char *sshconfig = NULL;
char *gstmpixmaps = NULL;
char *gstmui = NULL;

int main (int argc, char *argv[])
{
	int status;

    LIBXML_TEST_VERSION

	init_config();
	init_paths();

#ifdef ENABLE_NLS
	bindtextdomain (GETTEXT_PACKAGE, PACKAGE_LOCALE_DIR);
	bind_textdomain_codeset (GETTEXT_PACKAGE, "UTF-8");
	textdomain (GETTEXT_PACKAGE);
#endif

	//	Create the main window and associated stuffs
	app = gstm_new ();
	
	//connect signal handlers before going into the gtk_main loop
	signal(SIGTERM, signalexit);
	signal(SIGINT, signalexit);

	status = g_application_run (G_APPLICATION (app), argc, argv);

	if (builder)
		g_object_unref (builder);
	
	g_object_unref (app);

	xmlCleanupParser ();

	free (gstmdir);
	free (gstmpixmaps);
	free (sshdir);
	free (sshconfig);
	free (gstmui);

	return status;
}

void signalexit(int sig_num)
{
        gstm_quit();
}

//	Locate gSTM and ssh config directories
void init_config ()
{
	// Check if old config location (~/.gSTM/) exists, if so migrate to new
	// location (~/.config/gSTM/).
	if (!check_newConfig())
	{
		fprintf(stderr, "Did not find new config dir (~/.config/gSTM/)\n");
		if (init_oldConfig())
		{
			fprintf(stderr, "Found legacy config dir (~/.gSTM/)\n");
			// Read in data from old config location
			int tCount = gstm_readfiles (gstmdir, &gSTMtunnels);

			// Reset gstmdir to new config location
			init_newConfig();

			// Save data to new location
			if (tCount > 0)
			{
				fprintf (stderr,"Found %i tunnels, migrating to new location (~/.config/gSTM/)\n",tCount);
				for (int tid = 0; tid < tCount; tid++)
				{
					fprintf (stderr, "- Migrating %s...\n", gSTMtunnels[tid]->name);
					free (gSTMtunnels[tid]->fn);
					gSTMtunnels[tid]->fn = malloc (strlen (gstmdir) + 1 + strlen ((char *)gSTMtunnels[tid]->name) + 7 + 1);
					gSTMtunnels[tid]->fn = gstm_name2filename ((char *)gSTMtunnels[tid]->name);
					gstm_tunnel2file (gSTMtunnels[tid], gSTMtunnels[tid]->fn);
				}
			}

			// Free all tunnel-related things, will be re-populated later
			gstm_freetunnels(&gSTMtunnels, tCount);
			tCount=0;
			if (tunnellist_store)
				g_object_unref (tunnellist_store);

			fprintf(stderr, "Migration complete; Please manually delete ~/.gSTM/ once you verify everything is working.\n");
		}
	}

	init_newConfig();

	// get HOME variable and construct sshconfig file path
	sshdir = malloc (strlen (getenv ("HOME")) + 6 + 1);
	strcpy (sshdir, getenv ("HOME"));
	strcat (sshdir, "/.ssh/");

	sshconfig = malloc (strlen (getenv ("HOME")) + 12 + 1);
	strcpy (sshconfig, sshdir);
	strcat (sshconfig, "config");
}

bool init_oldConfig()
{
	struct stat sb;

	// get HOME variable and construct gSTM dir
	gstmdir = malloc (strlen (getenv ("HOME")) + 6 + 1);
	strcpy (gstmdir, getenv ("HOME"));
	strcat (gstmdir, "/.gSTM");

	// check if gSTM dir exists or create it
	if (access (gstmdir, W_OK))
		return false;
	else
	{
		// check if it is really a directory ;)
		stat (gstmdir, &sb);
		if (!S_ISDIR (sb.st_mode))
			return false;
	}

	return true;
}

bool check_newConfig()
{
	char *tempDir = malloc (strlen (getenv ("HOME")) + 13 + 1);
	strcpy (tempDir, getenv ("HOME"));
    strcat (tempDir, "/.config/gSTM");

	if (access (tempDir, W_OK))
	{
		free (tempDir);
		return false;
	}

	free (tempDir);
	return true;
}

void init_newConfig()
{
    struct stat sb;

	if (gstmdir != NULL)
		free (gstmdir);

    gstmdir = malloc (strlen (getenv ("HOME")) + 13 + 1);
    strcpy (gstmdir, getenv ("HOME"));
    strcat (gstmdir, "/.config");

	if (access (gstmdir, W_OK))
		mkdir (gstmdir, 0755);

	strcat (gstmdir, "/gSTM");
	if (access (gstmdir, W_OK))
	{
        mkdir (gstmdir, 0755);

        if (access (gstmdir, W_OK))
        {
            fprintf(stderr, "** ~/.config/gSTM directory is not accessible\n");
            exit (EXIT_FAILURE);
        }
        else
        {
            // check if it is really a directory ;)
            stat (gstmdir, &sb);
            if (!S_ISDIR (sb.st_mode))
            {
                fprintf (stderr, "** a file (~/.config/gSTM) exists in your HOME directory, please delete it.\n");
                exit (EXIT_FAILURE);
            }
        }
    }
}


//	Find location of pixmaps and glade ui file
void init_paths ()
{
	//	Check if local data exists
	char *tempdir = NULL;
	tempdir  = malloc (strlen (PACKAGE_SRC_DIR) + strlen ("/../pixmaps/") + 1);
	strcpy (tempdir, PACKAGE_SRC_DIR);
	strcat (tempdir, "/../pixmaps/");

	DIR* dir = opendir (tempdir);
	
	if (dir)
	{
		closedir (dir);
		gstmpixmaps = malloc (strlen (tempdir) + 1);
		strcpy (gstmpixmaps, tempdir);

		gstmui = malloc (strlen (PACKAGE_SRC_DIR) + strlen ("/gstm.ui") + 1);
		strcpy (gstmui, PACKAGE_SRC_DIR);
		strcat (gstmui, "/gstm.ui");
	}

	free (tempdir);

	//	If not, check if system data exists
	if (!gstmpixmaps)
	{
		dir = opendir(PACKAGE_DATA_DIR);

		if (dir)
		{
			closedir (dir);
			gstmpixmaps = malloc (strlen (PACKAGE_DATA_DIR) + strlen ("/pixmaps/") + 1);
			strcpy (gstmpixmaps, PACKAGE_DATA_DIR);
			strcat (gstmpixmaps, "/pixmaps/");

			gstmui = malloc (strlen (PACKAGE_DATA_DIR) + strlen ("/ui/gstm.ui") + 1);
			strcpy (gstmui, PACKAGE_DATA_DIR);
			strcat (gstmui, "/ui/gstm.ui");
		}
	}

	if (!gstmpixmaps)
	{
		exit (EXIT_FAILURE);
	}
}

//	Store maindialog window width and height
void gstm_store_window_size ()
{
	GKeyFile *keyfile = g_key_file_new ();

	g_key_file_set_integer (keyfile, "WindowState", "Width", maindiag_width);
	g_key_file_set_integer (keyfile, "WindowState", "Height", maindiag_height);
	
	const char *appid = g_application_get_application_id (g_application_get_default ());
	char *path = g_build_filename (g_get_user_cache_dir (), appid, NULL);

	if (g_mkdir_with_parents (path, 0700) < 0)
	{
		goto out;
	}

	char *file = g_build_filename (path, "state.ini", NULL);

	g_key_file_save_to_file (keyfile, file, NULL);

	g_free (file);

	out:
		g_key_file_unref (keyfile);
		g_free (path);
}

//	Load maindialog window width and height
void gstm_load_window_size ()
{
	const char *appid = g_application_get_application_id (g_application_get_default ());
	char *file = g_build_filename (g_get_user_cache_dir (), appid, "state.ini", NULL);
	GKeyFile *keyfile = g_key_file_new ();

	if ( !g_key_file_load_from_file (keyfile, file, G_KEY_FILE_NONE, NULL) )
	{
		goto out;
	}

	GError *error;
	
	error = NULL;
	maindiag_width = g_key_file_get_integer (keyfile, "WindowState", "Width", &error);
	if (error != NULL)
	{
		g_clear_error (&error);
		maindiag_width = -1;
	}

	error = NULL;
	maindiag_height = g_key_file_get_integer (keyfile, "WindowState", "Height", &error);
	if (error != NULL)
	{
		g_clear_error (&error);
		maindiag_height = -1;
	}

	out:
		g_key_file_unref (keyfile);
		g_free (file);
}
