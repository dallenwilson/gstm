/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/conffile.h
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

#ifndef _CONFFILE_H
#define _CONFFILE_H

//#ifdef __cplusplus
//extern "C"
//{
//#endif

	#include <gtk/gtk.h>
	#include <libxml/xmlmemory.h>
	#include <libxml/parser.h>
	#include <libxml/encoding.h>
	#include <libxml/xmlwriter.h>
	#include "support.h"
	#include "common.h"
	
	//portredirection object
	struct portredir {
		xmlChar *type; //local or remote
		xmlChar *port1;
		xmlChar *host;
		xmlChar *port2;
	} *portredirPtr;
	
	//tunnelobject
	struct sshtunnel {
		xmlChar *name;
		xmlChar *host;
		xmlChar *port;
		xmlChar *login;
		xmlChar *privkey;
		gboolean autostart;
		struct portredir **portredirs;
		int defcount;
		gboolean active;
		int sshpid;
		char *fn;
	} stunnel, *sshtunnelPtr;
	
	struct sshtunnel **gSTMtunnels;
	int tunnelCount;
	int activeCount;
	gboolean noerrors;

	int gstm_readfiles(char *dir, struct sshtunnel ***tptr);
	int gstm_file2tunnel(char *file, struct sshtunnel *tunnel);
	gboolean gstm_tunnel2file(struct sshtunnel *st, const char *fn);
	char *gstm_name2filename (char *n);
	int gstm_addtunneldef2tunnel(xmlDocPtr doc, xmlNodePtr def, struct sshtunnel *tunnel, int idx);
	void gstm_freetunnels(struct sshtunnel ***tptr, int cnt);
	gboolean gstm_tunnel_name_exists(const char *name);
	int gstm_tunnel_add(const char *name);
	void gstm_tunnel_del(int id);
	
//#ifdef __cplusplus
//}
//#endif

#endif /* _CONFFILE_H */
