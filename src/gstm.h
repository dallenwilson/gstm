/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gstm.h
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

#ifndef _GSTM_
#define _GSTM_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GSTM_TYPE_APPLICATION             (gstm_get_type ())
#define GSTM_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GSTM_TYPE_APPLICATION, Gstm))
#define GSTM_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GSTM_TYPE_APPLICATION, GstmClass))
#define GSTM_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GSTM_TYPE_APPLICATION))
#define GSTM_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GSTM_TYPE_APPLICATION))
#define GSTM_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GSTM_TYPE_APPLICATION, GstmClass))

typedef struct _GstmClass GstmClass;
typedef struct _Gstm Gstm;
typedef struct _GstmPrivate GstmPrivate;

struct _GstmClass
{
	GtkApplicationClass parent_class;
};

struct _Gstm
{
	GtkApplication parent_instance;

	GstmPrivate *priv;

};

GtkBuilder *builder;
GtkWidget *maindialog;
GtkWidget *aboutdialog;
GtkWidget *newdialog;
GtkWidget *tundialog;
GtkWidget *propertiesdialog;

GtkWidget *statusbar;
GtkWidget *tunlist;

GtkListStore *tunnellist_store;

GType gstm_get_type (void) G_GNUC_CONST;
Gstm *gstm_new (void);

/* Callbacks */

G_END_DECLS

#endif /* _APPLICATION_H_ */

void gstm_init_treeview (GtkTreeView *view);
int gstm_process_autostart (struct sshtunnel **STMtunnels, int tcnt);
void gstm_populate_treeview (GtkWidget *dialog, const char *objname,
                             struct sshtunnel **STMtunnels, int tcnt);
void gstm_quit ();