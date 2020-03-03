/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/support.h
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <gtk/gtk.h>

#undef Q_
#ifdef ENABLE_NLS
#  define Q_(String) g_strip_context ((String), gettext (String))
#else
#  define Q_(String) g_strip_context ((String), (String))
#endif

/* This is used to create the pixmaps used in the interface. */
GtkWidget* create_pixmap (GtkWidget *widget, const gchar *filename);

/* This is used to create the pixbufs used in the interface. */
GdkPixbuf* create_pixbuf (const gchar *filename);
