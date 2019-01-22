/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * gAskpass/gaskpass.h
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

#ifndef _GASKPASS_
#define _GASKPASS_

#include <gtk/gtk.h>

G_BEGIN_DECLS

#define GASKPASS_TYPE_APPLICATION             (gaskpass_get_type ())
#define GASKPASS_APPLICATION(obj)             (G_TYPE_CHECK_INSTANCE_CAST ((obj), GASKPASS_TYPE_APPLICATION, gAskpass))
#define GASKPASS_APPLICATION_CLASS(klass)     (G_TYPE_CHECK_CLASS_CAST ((klass), GASKPASS_TYPE_APPLICATION, gAskopassClass))
#define GASKPASS_IS_APPLICATION(obj)          (G_TYPE_CHECK_INSTANCE_TYPE ((obj), GASKPASS_TYPE_APPLICATION))
#define GASKPASS_IS_APPLICATION_CLASS(klass)  (G_TYPE_CHECK_CLASS_TYPE ((klass), GASKPASS_TYPE_APPLICATION))
#define GASKPASS_APPLICATION_GET_CLASS(obj)   (G_TYPE_INSTANCE_GET_CLASS ((obj), GASKPASS_TYPE_APPLICATION, gAskpassClass))

typedef struct _gAskpassClass gAskpassClass;
typedef struct _gAskpass gAskpass;

struct _gAskpass
{
  GtkApplication parent;
};

struct _gAskpassClass
{
	GtkApplicationClass parent_class;
};

GtkWidget *dialog;

G_END_DECLS

#endif /* _APPLICATION_H_ */

gAskpass *gaskpass_new (void);