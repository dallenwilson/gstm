/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*-  */
/*
 * src/conffile.c
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

#include <sys/types.h>
#include <dirent.h>
#include <glib.h>

#include "main.h"
#include "conffile.h"
#include "fniface.h"
#include "fnssht.h"
#include "support.h"

struct sshtunnel **gSTMtunnels;
struct portredir *portredirPtr;
struct sshtunnel stunnel;
struct sshtunnel *sshtunnelPtr;

int tunnelCount = 0;
int activeCount = 0;
gboolean noerrors = FALSE;

void gstm_free1tunnel(struct sshtunnel *tun) {
	if (tun != NULL) {
		free(tun->name);
		free(tun->host);
		free(tun->port);
		free(tun->login);
		free(tun->privkey);
		free(tun->maxrestarts);
		free(tun->fn);

		if ((tun->defcount > 0) && (tun->portredirs != NULL)) {
			for (int j = 0; j < tun->defcount; j++) {
				free(tun->portredirs[j]->type);
				free(tun->portredirs[j]->port1);
				free(tun->portredirs[j]->host);
				free(tun->portredirs[j]->port2);
				free(tun->portredirs[j]);
			}

			free(tun->portredirs);
		}

		free(tun);
	}
}

char *gstm_name2filename (char *n)
{
	char *fname, *retval=NULL;
	int fd;
	
	//first, we use mkstemp to get our 'unique' filename
	fname = malloc (strlen (gstmdir) + 1 + strlen (n) + 7 + 1);
	sprintf (fname, "%s/%s.XXXXXX", gstmdir, n);

	if ((fd = mkstemp (fname)) != -1)
	{
		close (fd);
		unlink (fname);
		retval = malloc (strlen (fname) + 5 + 1);
		sprintf (retval, "%s.gstm", fname);
		free (fname);
	}

	return retval;
}

int gstm_tunnel_add(const char *tname)
{
	char *xname, *fname;
	int i,ret = -1;
	struct sshtunnel *tun;
	GtkTreeIter iter;
	GdkPixbuf *pixbuf_red;

	if (gstmdir != NULL && access (gstmdir, W_OK) == 0)
	{
		xname = (char *)tname;
		
		//first we'll take care of some replacements
		for (i = 0; i < strlen (xname); i++)
		{
			switch (xname[i])
			{
				//case ' ':
				case '/':
				case '\\':
				case '?':
				case '*':
				case ':':
				case '<':
				case '>':
				case '|':
				case '"':
				case '&':
				case '!':
				case '`':
				case '\'':
					xname[i]='_';
					break;
			}
		}
		
		//now make it into a filename
		if ((fname=gstm_name2filename(xname))) {

			tun = malloc (sizeof (struct sshtunnel));
			tun->name = malloc (strlen (xname) + 1);
			strcpy ((char *)tun->name, xname);
			tun->host = malloc(1); tun->host[0] = '\0';
			tun->port = malloc(3); strcpy ((char *)tun->port, "22");
			tun->login= malloc(1); tun->login[0] = '\0';
			tun->privkey = malloc(1); tun->privkey[0] = '\0';
			tun->portredirs = NULL;
			tun->defcount = 0;
			tun->autostart = FALSE;
			tun->restart = FALSE;
			tun->notify = FALSE;
			tun->maxrestarts = malloc(3); strcpy ((char *)tun->maxrestarts, "9");
			tun->preset = FALSE;
			tun->active = FALSE;
			tun->sshpid = 0;
			tun->fn = malloc (strlen (fname) + 1); strcpy (tun->fn, fname);
			
			if (gstm_tunnel2file (tun, fname))
			{
				//put in list
				gSTMtunnels = realloc (gSTMtunnels, (tunnelCount+1) * sizeof (struct sshtunnel *));
				gSTMtunnels[tunnelCount] = tun;
				
				//put in interface
				pixbuf_red = create_pixbuf_scaled ("red.svg", GTK_ICON_SIZE_MENU);
				gtk_list_store_append (tunnellist_store, &iter);
				gtk_list_store_set (tunnellist_store, &iter, COL_ACTIVE,
				                    pixbuf_red, COL_NAME, tun->name, COL_ID,
				                    tunnelCount, -1);
				g_object_unref (pixbuf_red);
				ret = tunnelCount;
				tunnelCount += 1;
				
				//sort it again
				gtk_tree_sortable_set_sort_column_id (GTK_TREE_SORTABLE (tunnellist_store),
				                                      COL_NAME,
				                                      GTK_SORT_ASCENDING);		
			}
			else
			{
				gstm_free1tunnel (tun);
				gstm_interface_error ("fileconversion failed");
			}
			
			free(fname);
		}
		else
		{
			gstm_interface_error ("filename conversion failed");
		}			
	}
	else
	{
		gstm_interface_error (".gSTM directory not writeable!");
	}
	
	return ret;
}

void gstm_tunnel_del(int id) {
	GtkTreeIter i;
	int v_id;
	gboolean ret;
	if (gSTMtunnels[id]->active) {
		//the 'noerrors' flag make sure the usleep() below doesn't lock the helperthread
		noerrors = TRUE;
		gstm_ssht_stoptunnel(id);
	}
	//I think we should wait here till the thread has stopped
	//however, is this while() thread safe?
	while (gSTMtunnels[id]->active) usleep(100000);
	noerrors = FALSE;
	
	//delete the tunnel from disk and list
	unlink(gSTMtunnels[id]->fn);
	gstm_free1tunnel(gSTMtunnels[id]);
	gSTMtunnels[id]=NULL;
	tunnelCount-=1;
	
	ret = gtk_tree_model_get_iter_first(GTK_TREE_MODEL(tunnellist_store),&i);
	while (ret) {
		gtk_tree_model_get (GTK_TREE_MODEL(tunnellist_store), &i, COL_ID, &v_id, -1);
		if (id==v_id) {
			gtk_list_store_remove(tunnellist_store, &i);
			break;
		}
		ret=gtk_tree_model_iter_next(GTK_TREE_MODEL(tunnellist_store),&i);
	}
}

gboolean gstm_tunnel2file(struct sshtunnel *st, const char *fn) {
	int rc,i;
	xmlTextWriterPtr writer;
	//TDOD Del xmlChar *tmp;
	gboolean ret=FALSE;

	if (st!=NULL && fn!=NULL) {
		/* Create a new XmlWriter with no compression. */
		writer = xmlNewTextWriterFilename(fn, 0);
		if (writer == NULL) {
			gstm_interface_error("Error creating the xml writer");
			return ret;
		}
		
		xmlTextWriterSetIndent(writer,1);
		xmlTextWriterSetIndentString(writer, (xmlChar *)"\t");
		
	    /* root element */
		rc = xmlTextWriterStartElement(writer, BAD_CAST "sshtunnel");
		if (rc < 0) {
			gstm_interface_error("Error at root xmlTextWriterStartElement");
			return ret;
		}
		
	    /* name element */
		rc = xmlTextWriterWriteElement(writer, BAD_CAST "name", BAD_CAST st->name);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: name");
			return ret;
		}
	    /* host element */
		rc = xmlTextWriterWriteElement(writer, BAD_CAST "host", BAD_CAST st->host);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: host");
			return ret;
		}
	    /* port element */
		rc = xmlTextWriterWriteElement(writer, BAD_CAST "port", BAD_CAST st->port);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: port");
			return ret;
		}
	    /* login element */
		rc = xmlTextWriterWriteElement(writer, BAD_CAST "login", BAD_CAST st->login);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: login");
			return ret;
		}
	    /* privkey element */
		rc = xmlTextWriterWriteElement(writer, BAD_CAST "privkey", BAD_CAST st->privkey);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: privkey");
			return ret;
		}
	    /* autostart element */
		if (st->autostart) {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "autostart", BAD_CAST "1");
		} else {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "autostart", BAD_CAST "0");
		}
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: autostart");
			return ret;
		}
		/* restart element */
		if (st->restart) {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "restart", BAD_CAST "1");
		} else {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "restart", BAD_CAST "0");
		}
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: restart");
			return ret;
		}		
		/* notify element */
		if (st->notify) {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "notify", BAD_CAST "1");
		} else {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "notify", BAD_CAST "0");
		}
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: notify");
			return ret;
		}		
		/* maxrestarts element */
		rc = xmlTextWriterWriteElement(writer, BAD_CAST "maxrestarts", BAD_CAST st->maxrestarts);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: maxrestarts");
			return ret;
		}
		/* preset element */
		if (st->preset) {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "preset", BAD_CAST "1");
		} else {
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "preset", BAD_CAST "0");
		}
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterWriteElement: preset");
			return ret;
		}

		for(i=0; i<st->defcount;i++) {
			/* port redirect */
			rc = xmlTextWriterStartElement(writer, BAD_CAST "tunnel");
			if (rc < 0) {
				gstm_interface_error("Error at xmlTextWriterStartElement: portdef start");
				return ret;
			}
			/* type element */
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "type", BAD_CAST st->portredirs[i]->type);
			if (rc < 0) {
				gstm_interface_error("Error at xmlTextWriterWriteElement: type");
				return ret;
			}
			/* port1 element */
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "port1", BAD_CAST st->portredirs[i]->port1);
			if (rc < 0) {
				gstm_interface_error("Error at xmlTextWriterWriteElement: port1");
				return ret;
			}
			/* host element */
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "host", BAD_CAST st->portredirs[i]->host);
			if (rc < 0) {
				gstm_interface_error("Error at xmlTextWriterWriteElement: host");
				return ret;
			}
			/* port2 element */
			rc = xmlTextWriterWriteElement(writer, BAD_CAST "port2", BAD_CAST st->portredirs[i]->port2);
			if (rc < 0) {
				gstm_interface_error("Error at xmlTextWriterWriteElement: port2");
				return ret;
			}
			rc = xmlTextWriterEndElement(writer);
			if (rc < 0) {
				gstm_interface_error("Error at xmlTextWriterEndElement: portdef end");
				return ret;
			}
		}
		
		/* end writer */
		rc = xmlTextWriterEndDocument(writer);
		if (rc < 0) {
			gstm_interface_error("Error at xmlTextWriterEndDocument");
			return ret;
		}
		xmlFreeTextWriter(writer);		
	}
	ret = TRUE;
	return ret;
}

gboolean gstm_tunnel_name_exists(const char *tname) {
	int i;
	gboolean ret=FALSE;
	for (i=0; i<tunnelCount; i++) {
		if ((gSTMtunnels[i] != NULL) && strcasecmp ((char *)gSTMtunnels[i]->name, tname) == 0) {
			ret = TRUE;
			break;
		}
	}
	return ret;
}

int gstm_readfiles(char *dir, struct sshtunnel ***tptr) {
	struct dirent **entrylist=NULL;
	struct sshtunnel **mptr, *fptr=NULL;

	char *sptr=NULL;
	int len=0, scnt=0, listlen=0, l=0;

	listlen = scandir(dir, &entrylist, 0, alphasort);

	if (listlen<0) {
		fprintf(stderr,"** unable to open gSTM directory (%s)\n",dir);
		free(dir);
		exit(EXIT_FAILURE);

	} else {
		while (l<listlen) {		
			len = strlen(entrylist[l]->d_name);

			if (len<6) {
				free(entrylist[l]);
				l++;
				continue;
			}

			sptr = entrylist[l]->d_name + (len-5);

			if (strcmp(sptr,".gstm")==0) {
				*tptr = realloc(*tptr, (scnt+1)*sizeof(struct sshtunnel *));

				if (*tptr==NULL) {
					fprintf(stderr,"** out of memory");
					free(dir);
					exit(EXIT_FAILURE);
				}

				mptr = *tptr;
				mptr[scnt] = malloc(sizeof(struct sshtunnel));

				if (mptr[scnt]==NULL) {
					fprintf(stderr,"** out of memory");
					free(tptr);
					free(dir);
					exit(EXIT_FAILURE);
				}

				fptr = mptr[scnt];
				sptr = malloc(strlen(dir)+1+strlen(entrylist[l]->d_name)+1);
				sprintf(sptr, "%s/%s", dir, entrylist[l]->d_name);

				if (gstm_file2tunnel(sptr,fptr))
					scnt+=1;

				free (sptr);
			}

			free(entrylist[l]);
			l++;
		}

		free(entrylist);
	}

	return scnt;
}

void gstm_freetunnels(struct sshtunnel ***tptr, int cnt) {
	struct sshtunnel **tunnels;

	if (tptr != NULL) {
		tunnels = *tptr;

		if (tunnels != NULL) {
			for (int i = 0; i < cnt; i++) {
				if (tunnels[i] != NULL)
					gstm_free1tunnel(tunnels[i]);
			}

			free(tunnels);
		}
	}
}

int gstm_file2tunnel(char *file, struct sshtunnel *tunnel) {
	int retval=0, halt=0;
	xmlDocPtr doc;
	xmlNodePtr cur;
	xmlChar *tmp, *tmpauto, *tmprest, *tmpnotify;
	tmpauto=NULL;
	tmprest=NULL;
	tmpnotify=NULL;
	
	// build an XML tree from the file;
	doc = xmlParseFile(file);
	if (doc == NULL)
		return(retval);
		
	// check the document is of the right kind
	cur = xmlDocGetRootElement(doc);
	if (cur == NULL) {
		fprintf(stderr,"** WARNING: empty document: '%s'\n",file);
		xmlFreeDoc(doc);
		return(retval);
	}

	// check toplevel name
	if (xmlStrcmp (cur->name, (xmlChar *)"sshtunnel")) {
		fprintf(stderr,"** WARNING: not an sshtunnel document: '%s'\n",file);
		xmlFreeDoc(doc);
		return(retval);
	}

	cur = cur->xmlChildrenNode;
	tunnel->defcount = 0;
	tunnel->name = malloc(1); tunnel->name[0]='\0';
	tunnel->host = malloc(1); tunnel->host[0]='\0';
	tunnel->port = malloc(1); tunnel->port[0]='\0';
	tunnel->login = malloc(1); tunnel->login[0]='\0';
	tunnel->privkey = malloc(1); tunnel->privkey[0]='\0';
	tunnel->maxrestarts = malloc(1); tunnel->maxrestarts[0]='\0';

	tunnel->portredirs = NULL;
	tunnel->active = FALSE;
	tunnel->autostart = FALSE;
	tunnel->restart = FALSE;
	tunnel->preset = FALSE;
	tunnel->notify = TRUE;
	tunnel->sshpid=0;
	tunnel->fn = malloc(strlen(file)+1); strcpy(tunnel->fn,file);
	while (cur && !halt) {
		if (!xmlIsBlankNode(cur)) {
			if (!xmlStrcmp (cur->name, (xmlChar *)"tunnel")) {
				if (gstm_addtunneldef2tunnel(doc, cur->xmlChildrenNode, tunnel, tunnel->defcount)) {
					tunnel->defcount+=1;
				}
			} else {
				tmp = xmlNodeListGetString(doc, cur->xmlChildrenNode, 1);
				if ((strcmp ((char *)cur->name, "name") == 0) && tmp)
				{
					tunnel->name = realloc (tunnel->name, strlen ((char *)tmp) + 1);
					strcpy ((char *)tunnel->name, (char *)tmp);
				}
				else if (strcmp ((char *)cur->name, "host") == 0 && tmp)
				{
					tunnel->host = realloc (tunnel->host, strlen ((char *)tmp) + 1);
					strcpy ((char *)tunnel->host, (char *)tmp);
				}
				else if (strcmp ((char *)cur->name, "port") == 0 && tmp)
				{
					tunnel->port = realloc (tunnel->port, strlen ((char *)tmp) + 1);
					strcpy ((char *)tunnel->port, (char *)tmp);
				}
				else if (strcmp ((char *)cur->name, "login") == 0 && tmp)
				{
					tunnel->login = realloc (tunnel->login, strlen ((char *)tmp) + 1);
					strcpy ((char *)tunnel->login, (char *)tmp);
				}
				else if (strcmp ((char *)cur->name, "privkey") == 0 && tmp)
				{
					tunnel->privkey = realloc (tunnel->privkey, strlen ((char *)tmp) + 1);
					strcpy ((char *)tunnel->privkey, (char *)tmp);
				}
				else if (strcmp ((char *)cur->name, "autostart") == 0 && tmp)
				{
					tmpauto = malloc (strlen ((char *)tmp) + 1);
					strcpy ((char *)tmpauto, (char *)tmp);
					if (strcmp ((char *)tmpauto, "1") == 0) {
						tunnel->autostart = TRUE;
					}
					free(tmpauto);
					tmpauto = NULL;
				}
				else if (strcmp ((char *)cur->name, "restart") == 0 && tmp)
				{
					tmprest = malloc (strlen ((char *)tmp) + 1);
					strcpy ((char *)tmprest, (char *)tmp);
					if (strcmp ((char *)tmprest, "1") == 0) {
					tunnel->restart = TRUE;
					}
					free(tmprest);
					tmprest = NULL;
				}
				else if (strcmp ((char *)cur->name, "preset") == 0 && tmp)
				{
					tmprest = malloc (strlen ((char *)tmp) + 1);
					strcpy ((char *)tmprest, (char *)tmp);
					if (strcmp ((char *)tmprest, "1") == 0) {
					tunnel->preset = TRUE;
					}
					free(tmprest);
					tmprest = NULL;
				}
				else if (strcmp ((char *)cur->name, "notify") == 0 && tmp)
				{
					tmpnotify = malloc (strlen ((char *)tmp) + 1);
					strcpy ((char *)tmpnotify, (char *)tmp);
					if (strcmp ((char *)tmpnotify, "1") != 0) {
					tunnel->notify = FALSE;
					}
					free(tmpnotify);
					tmpnotify = NULL;
				}
				else if (strcmp ((char *)cur->name, "maxrestarts") == 0 && tmp)
				{
					tunnel->maxrestarts = realloc (tunnel->maxrestarts, strlen ((char *)tmp) + 1);
					strcpy ((char *)tunnel->maxrestarts, (char *)tmp);
				} 
				else 
				{
					// ??
					//halt=1;
					//retval=0;
				}
				xmlFree(tmp);
			}
		}
		cur = cur->next;
	}
	if (strlen ((char *)tunnel->port) == 0) {
		tunnel->port = realloc(tunnel->port,3);
		strcpy ((char *)tunnel->port, "22");
	}
	
	if (strlen ((char *)tunnel->maxrestarts) == 0) {
		tunnel->maxrestarts = realloc(tunnel->maxrestarts,2);
		strcpy ((char *)tunnel->maxrestarts, "9");
	}

	xmlFree (tmpauto);
	xmlFree (tmprest);
	xmlFree (tmpnotify);
	xmlFreeDoc(doc);

	retval=1;
	return(retval);
}

int gstm_addtunneldef2tunnel(xmlDocPtr doc, xmlNodePtr def, struct sshtunnel *tunnel, int idx) {
	int halt = 0;
	int retval = 0;
	struct portredir *tdef;

	tunnel->portredirs = realloc(tunnel->portredirs, (idx+1)*sizeof(struct tunneldef *));

	if (tunnel->portredirs == NULL) {
		fprintf(stderr,"** out of memory");
		exit(EXIT_FAILURE);
	}

	tunnel->portredirs [idx] = malloc(sizeof(struct portredir));

	if (tunnel->portredirs [idx] == NULL) {
		fprintf(stderr,"** out of memory");
		exit(EXIT_FAILURE);
	}

	tdef = tunnel->portredirs [idx];
	tdef->type	= malloc(1);	tdef->type[0]	= '\0';
	tdef->port1	= malloc(1);	tdef->port1[0]	= '\0';
	tdef->host	= malloc(1);	tdef->host[0]	= '\0';
	tdef->port2	= malloc(1);	tdef->port2[0]	= '\0';

	while (def && !halt) {
		if (!xmlIsBlankNode (def)) {
			xmlChar *tmp = xmlNodeListGetString(doc, def->xmlChildrenNode, 1);

			if ((strcmp ((char *)def->name, "type") == 0) && tmp) {
				tdef->type = realloc(tdef->type, strlen ((char *)tmp) + 1);
				strcpy ((char *)tdef->type, (char *)tmp);

			} else if ((strcmp ((char *)def->name, "port1") == 0) && tmp) {
				tdef->port1 = realloc (tdef->port1, strlen ((char *)tmp) + 1);
				xmlChar *tmp2 = xmlNodeListGetString(doc, def->xmlChildrenNode, 1);
				strcpy ((char *)tdef->port1, (char *)tmp2);
				xmlFree (tmp2);

			} else if ((strcmp ((char *)def->name, "host") == 0) && tmp) {
				tdef->host = realloc (tdef->host, strlen ((char *)tmp) + 1);
				strcpy ((char *)tdef->host, (char *)tmp);

			} else if ((strcmp ((char *)def->name, "port2") == 0) && tmp) {
				tdef->port2 = realloc (tdef->port2, strlen ((char *)tmp) + 1);
				strcpy ((char *)tdef->port2, (char *)tmp);

			}
			xmlFree(tmp);

		}
		def = def->next;

	}

	retval = 1;

	return retval;
}

// Scan ~/.ssh/config, if it exists, and build a list of preset hosts
void parseSSHconfig (GtkWidget *widget, gchar *host) {
	// Clear Preset list
	gtk_combo_box_text_remove_all (GTK_COMBO_BOX_TEXT (widget));

	FILE* file = fopen(sshconfig, "r");

	int counter = 0;
	int idx = 0;
	if (file != NULL) {
		char line[256];
		while (fgets(line, sizeof(line), file)) {
			char checkHost[6];
			strncpy (checkHost, line, 5);
			checkHost [5] = 0;

			if (strcmp (checkHost, "Host ") == 0) {
				char hostName [255];
				strncpy (hostName, line + 5, strlen(line));
				hostName [strlen (hostName) - 1] = 0;

				if (strcmp (hostName, "*") != 0) {
					counter++;
					gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (widget), hostName);

					if (host != NULL)
						if (strcmp (host, hostName) == 0)
							idx = counter;
				}
			}
		}

		fclose(file);
	}

	if (counter)
		gtk_combo_box_text_prepend_text (GTK_COMBO_BOX_TEXT (widget), (gchar *)"No preset selected");
	else
		gtk_combo_box_text_prepend_text (GTK_COMBO_BOX_TEXT (widget), (gchar *)"No presets available");

	gtk_combo_box_set_active (GTK_COMBO_BOX (widget), idx);
}