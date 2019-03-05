/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/main.c
 * 
 * Copyright (c) 2005-2019	Mark Smulders <msmulders@elsar.nl>
 * Copyright (C) 2019		Dallen Wilson <dwjwilson@lavabit.com>
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

	return status;
}

void signalexit(int sig_num)
{
        gstm_quit();
}

//	Find .gSTM from user's home directory if available
void init_config ()
{
	struct stat sb;

	// get HOME variable and construct gSTM dir
	gstmdir = malloc (strlen (getenv ("HOME")) + 6 + 1);
	
	if (!gstmdir)
	{
		fprintf (stderr, "** out of memory\n");
		exit (EXIT_FAILURE);
	}
	
	strcpy (gstmdir, getenv ("HOME"));
	strcat (gstmdir, "/.gSTM");

	// check if gSTM dir exists or create it
	if (access (gstmdir, W_OK))
	{
		
		// can't access it, although it might exist try to create it
		mkdir (gstmdir, 0755);

		if (access (gstmdir, W_OK))
		{
			//still can't access it :(
			fprintf(stderr, "** .gSTM directory in your HOME directory is not accessible\n");
			exit (EXIT_FAILURE);
		}
		else
		{
			// check if it is really a directory ;)
			stat (gstmdir, &sb);
			if (!S_ISDIR (sb.st_mode))
			{
				fprintf (stderr, "** a file called .gSTM exists in your HOME directory, please delete it.\n");
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
		gstmpixmaps = malloc (strlen (tempdir));
		strcpy (gstmpixmaps, tempdir);

		gstmui = malloc (strlen (PACKAGE_SRC_DIR) + strlen ("/gstm.ui") + 1);
		strcpy (gstmui, PACKAGE_SRC_DIR);
		strcat (gstmui, "/gstm.ui");
	}

	//	If not, check if system data exists
	if (!gstmpixmaps)
	{
		dir = opendir(PACKAGE_DATA_DIR);

		if (dir)
		{
			closedir (dir);
			gstmpixmaps = malloc (strlen (PACKAGE_DATA_DIR) + strlen ("/pixmaps/gstm/") + 1);
			strcpy (gstmpixmaps, PACKAGE_DATA_DIR);
			strcat (gstmpixmaps, "/pixmaps/gstm/");

			gstmui = malloc (strlen (PACKAGE_DATA_DIR) + strlen ("/gstm/ui/gstm.ui") + 1);
			strcpy (gstmui, PACKAGE_DATA_DIR);
			strcat (gstmui, "/gstm/ui/gstm.ui");
		}
	}

	if (!gstmpixmaps)
	{
		exit (EXIT_FAILURE);
	}
}
