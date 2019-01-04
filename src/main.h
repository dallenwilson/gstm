/*
 * main.h
 *
 * Copyright (C) 2019 - Dallen Wilson <dwjwilson@lavabit.com>
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

#ifndef _MAIN_H
#define _MAIN_H

#include "conffile.h"

void signalexit(int sig_num);
int gstm_init(void);
void gstm_init_treeview(GtkTreeView *view);
void gstm_populate_treeview(GtkWidget *dialog, const char *objname, struct sshtunnel **STMtunnels, int tcnt);
int gstm_process_autostart(struct sshtunnel **STMtunnels, int tcnt);
void gstm_quit();

#endif