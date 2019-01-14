/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * main.c
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

//GnomeProgram *gapp;
Gstm *app;
char *gstmdir = NULL;
char *gstmpixmaps = NULL;

int main (int argc, char *argv[])
{
	int status;

    LIBXML_TEST_VERSION

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