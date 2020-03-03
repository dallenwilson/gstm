/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/fniface.h
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

#ifndef _FNIFACE_H
#define _FNIFACE_H

#include <gtk/gtk.h>

#include "conffile.h"

enum {
	COL_TYPE,
 	COL_PORT1,
 	COL_HOST,
	COL_PORT2,
	COL_RID,
	N_RCOLS
};

void gstm_interface_enable_maindialog(gboolean sensitive);
void gstm_interface_showinfo(char *text);
gint gstm_interface_selection2id(GtkTreeSelection *s, int column);
GtkTreeSelection *gstm_interface_get_selected_tunnel();
gboolean gstm_interface_is_startbutton();
int gstm_interface_asknewname(gchar **nname);
void gstm_interface_enablebuttons(gboolean active);
void gstm_interface_redirlist_del(GtkTreeView *v, int id);
void gstm_interface_redirlist_edit(struct portredir *r, int idx, GtkTreeView *v);
void gstm_interface_redirlist_add(struct portredir *r, int idx, GtkTreeView *v);
void gstm_interface_disablebuttons();
void gstm_interface_rowactivity();
void gstm_interface_rowaction();
void gstm_interface_paint_row(GtkTreeSelection *s, gboolean active);
void gstm_interface_paint_row_id(int id, gboolean active);
gboolean gstm_interface_error(const char *msg);
void gstm_interface_properties(int id);
void gstm_interface_selectrow_id(int id);
void gstm_dockletmenu_tunnelitem_new(GtkWidget *menu, const gchar *t_name, intptr_t t_id, gboolean t_active);

#endif /* _FNIFACE_H */
