#ifndef _CONFFILE_H
#define _CONFFILE_H

#ifdef __cplusplus
extern "C"
{
#endif
	
	#include <libxml/xmlmemory.h>
	#include <libxml/parser.h>
	#include <libxml/encoding.h>
	#include <libxml/xmlwriter.h>
	#include <gnome.h>		
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

	int gstm_readfiles(char *dir, struct sshtunnel ***tptr);
	int gstm_file2tunnel(char *file, struct sshtunnel *tunnel);
	int gstm_addtunneldef2tunnel(xmlDocPtr doc, xmlNodePtr def, struct sshtunnel *tunnel, int idx);
	void gstm_freetunnels(struct sshtunnel ***tptr, int cnt);
	gboolean gstm_tunnel_name_exists(const char *name);
	int gstm_tunnel_add(const char *name);
	
#ifdef __cplusplus
}
#endif

#endif /* _CONFFILE_H */
