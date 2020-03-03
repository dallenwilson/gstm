/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/callbacks.h
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

void on_maindialog_size_allocate	(GtkWidget *widget, GtkAllocation *allocation);
void btn_start_clicked_cb			(GtkButton *button, gpointer user_data);
void btn_stop_clicked_cb			(GtkButton *button, gpointer user_data);
void btn_add_clicked_cb				(GtkButton *button, gpointer user_data);
void btn_delete_clicked_cb			(GtkButton *button, gpointer user_data);
void btn_properties_clicked_cb		(GtkButton *button, gpointer user_data);
void btn_copy_clicked_cb			(GtkButton *button, gpointer user_data);
void btn_close_clicked_cb			(GtkButton *button, gpointer user_data);

void newgd_btn_ok_clicked_cb		(GtkButton *button, gpointer user_data);
void newdg_btn_cancel_clicked_cb	(GtkButton *button, gpointer user_data);

void tundg_btn_ok_clicked_cb		(GtkButton *button, gpointer user_data);
void tundg_btn_cancel_clicked_cb	(GtkButton *button, gpointer user_data);

void btn_redir_add_clicked_cb		(GtkButton *button, gpointer user_data);
void btn_redir_delete_clicked_cb	(GtkButton *button, gpointer user_data);
void btn_redir_edit_clicked_cb		(GtkButton *button, gpointer user_data);

//	Systray
void on_dockletmenu_toggle_activate (GtkMenuItem *menuitem, gpointer user_data);
void on_dockletmenu_tunnel_activate	(GtkMenuItem *menuitem, gpointer user_data);
void on_dockletmenu_about_activate	(GtkMenuItem *menuitem, gpointer user_data);
void on_dockletmenu_quit_activate	(GtkMenuItem *menuitem, gpointer user_data);
