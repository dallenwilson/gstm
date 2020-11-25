/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/main.h
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

#include "conffile.h"
#include "gstm.h"

extern Gstm *app;
extern char *gstmdir;
extern char *sshconfig;
extern char *gstmpixmaps;
extern char *gstmui;

void signalexit					(int sig_num);
void init_paths					();
void init_config				();
void gstm_store_window_size		();
void gstm_load_window_size		();
