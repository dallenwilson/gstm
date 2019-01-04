/***************************************************************************
 *            fnssht.c
 *
 *  Thu Aug  4 14:57:32 2005
 *  Copyright  2005  Mark Smulders
 *  msmulders@elsar.nl
 ****************************************************************************/
/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>

#include "fnssht.h"
#include "fniface.h"
#include "conffile.h"

/*
* setenv("SSH_ASKPASS", "gaskpass", 1)
* ssh -nNf -l LOGIN -p PORT -L...... -R...... HOST
*/

// the helperthread: it forks off an ssh child and wait()s for it's return
gpointer *gstm_ssht_helperthread(gpointer *args) {
	struct Shelperargs *harg = (struct Shelperargs *)args;
	GtkTreeSelection *s;
	char **a = harg->sshargs;
	char *buf=NULL, c, *buf2=NULL, *bufptr=NULL;
	int i,ret,rv=0,fd[2];
	FILE *p;
	extern gboolean noerrors;

	//open a pipe to receive stderr - ssh barfs it's errors on it ;)
	if (pipe(fd) == -1) {
		gdk_threads_enter();
		gstm_interface_error("pipe() error !");
		gdk_threads_leave();
	}
	
	switch(ret=fork()) {
		case -1: //error
			gdk_threads_enter();
			gstm_interface_error("fork() error !");
			gdk_threads_leave();
			break;
		case 0: //child
			//set the ASKPASS env var
			setenv("SSH_ASKPASS", "gaskpass", 1);
			//dup stderr so our parent can read it
			dup2(fd[1], fileno(stderr));
			close(fd[0]);
			close(fd[1]);
			//fire up the tunnel
			_exit(execvp(a[0],a));
			break;
		default: //parent
			//The helperthread waits for the ssh child to return. If the tunnel
			// is succesfully started, the child won't return until it exits for
			// whatever reason.
			close(fd[1]);
			//save the childs pid
			gSTMtunnels[harg->tid]->sshpid = ret; //should this be mutexed?
			//try to read stderr
			if (p = fdopen(fd[0],"r")) {
				i=0;
				while ((c=getc(p)) != EOF) {
					buf = realloc(buf,i+1);
					buf[i] = c;
					i++;
				}
				fclose(p);
				if (i>0) {
					buf = realloc(buf,i+1);
					buf[i]='\0';
				}
			}
			// and wait ...
			wait(&rv);
			break;
	}
	//take care of errorhandling
	if (rv!=0) {
		//printf("%d\n",rv);
		if (buf!=NULL && rv!=15) {  
			i = strlen(gSTMtunnels[harg->tid]->name)+19+strlen(buf)+1;
			buf2 = malloc(i);
			snprintf(buf2,i,"Tunnel '%s' stopped.\n%s",gSTMtunnels[harg->tid]->name,buf);
			bufptr = buf2;
		} else if (rv==9) { //kill -9 doesnt produce stderr output
			i = strlen(gSTMtunnels[harg->tid]->name)+19+19+1;
			buf = malloc(i);
			snprintf(buf,i,"Tunnel '%s' stopped.\nssh process killed!",gSTMtunnels[harg->tid]->name);
			bufptr = buf;
		} else if (rv==15) { //custom message on TERM signal
			i = strlen(gSTMtunnels[harg->tid]->name)+20+1;
			buf2 = malloc(i);
			snprintf(buf2,i,"Tunnel '%s' terminated",gSTMtunnels[harg->tid]->name);
			bufptr = buf2;
		} else {
			i = strlen(gSTMtunnels[harg->tid]->name)+19+20+5+1;
			buf = malloc(i);
			snprintf(buf,i,"Tunnel '%s' stopped.\nUnknown error code: %d",gSTMtunnels[harg->tid]->name,rv);
			bufptr = buf;
		}
		
		// if 'noerrors' flag is true, the main program is probably in a while(){sleep()) loop
		// waiting for the tunnel to stop. Don't do any interface stuff coz it will lock the program.
		if (!noerrors) {
			gdk_threads_enter();
			//repaint row
			gstm_interface_paint_row_id(harg->tid,FALSE);
			// Update start/stop buttons sensitivity
			if (s = gstm_interface_get_selected_tunnel()) {
				if (gstm_interface_selection2id(s,COL_ID)==harg->tid) {
					gstm_interface_enablebuttons(FALSE); 
				}
			}
			if (bufptr!=NULL) gstm_interface_error(bufptr);
			gdk_threads_leave();
		}
	}
	
	//we're finished
	if (buf!=NULL) free(buf);
	if (buf2!=NULL) free(buf2);

	//clean up the arg list
	for (i=0; a[i]!=NULL; i++) {
		free(a[i]);
	}
	free(a);
	
	//bye
	if (gSTMtunnels[harg->tid]->active) gSTMtunnels[harg->tid]->active=FALSE;
	activeCount--;
	free(harg);
}

static int argcnt;
char **gstm_ssht_addssharg(char **args, const char *str) {
	char **ret=NULL;
	if (args==NULL) argcnt=0;
		
	//enlarge the list
	ret=realloc(args, (argcnt+1) * sizeof(char *) );
	if (str!=NULL) {
		//put the string in
		ret[argcnt]=malloc(strlen(str)+1);
		strcpy(ret[argcnt], str);
	} else {
		//NULL str, assume we have to end the list with a NULL-pointer
		ret[argcnt]=NULL;
	}
	argcnt++;
	
	//all done
	return ret;
}

// starttunnel() creates the proper ssh command and fires the helper thread
int gstm_ssht_starttunnel(int id) {
	struct Shelperargs *hargs;
	char type, *tmp;
	int i,k;
	GThread *ret;
	
	if (!gSTMtunnels[id]->active) {
		hargs = malloc(sizeof(struct Shelperargs));
		hargs->tid = id;
		hargs->sshargs=NULL;
		
		//ok, now create the argument list to ssh
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "ssh");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, gSTMtunnels[id]->host);
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "-p");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, gSTMtunnels[id]->port);
		if (strlen(gSTMtunnels[id]->privkey)>1) {
			hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "-i");
			hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, gSTMtunnels[id]->privkey);
		}
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "-l");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, gSTMtunnels[id]->login);
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "-nN");
		// port redirect args
		for (i=0; i<gSTMtunnels[id]->defcount; i++) {
			tmp = malloc(4 + strlen(gSTMtunnels[id]->portredirs[i]->port1) +strlen(gSTMtunnels[id]->portredirs[i]->host) +strlen(gSTMtunnels[id]->portredirs[i]->port2) +1);
			if (strcmp(gSTMtunnels[id]->portredirs[i]->type,"local")==0) {
				type = 'L';
				sprintf(tmp,"-%c%s:%s:%s",type,gSTMtunnels[id]->portredirs[i]->port1,gSTMtunnels[id]->portredirs[i]->host,gSTMtunnels[id]->portredirs[i]->port2);
			} else if (strcmp(gSTMtunnels[id]->portredirs[i]->type,"remote")==0) {
				type = 'R';
				sprintf(tmp,"-%c%s:%s:%s",type,gSTMtunnels[id]->portredirs[i]->port1,gSTMtunnels[id]->portredirs[i]->host,gSTMtunnels[id]->portredirs[i]->port2);
			} else {
				type = 'D';
				sprintf(tmp,"-%c%s",type,gSTMtunnels[id]->portredirs[i]->port1);
			}
			hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, tmp);
			free(tmp);
		}
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "-o");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "ConnectTimeout=5");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "-o");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, "NumberOfPasswordPrompts=1");
		hargs->sshargs = gstm_ssht_addssharg(hargs->sshargs, NULL); //end list
		
		// good, now start the helper thread
		ret = g_thread_create((GThreadFunc)gstm_ssht_helperthread,hargs,FALSE,NULL);
		if (ret!=NULL) {
			gSTMtunnels[id]->active = TRUE;
			activeCount++;
		} else {
			gSTMtunnels[id]->active=FALSE;
			gstm_interface_error("g_thread_create error!\n");
		}
	} else {
		//hmm, we tried to activate an active tunnel ?
		//perhaps an error dialog, but for now just ignore this cause it shouldn't happen
	}
}

int gstm_ssht_stoptunnel(int id) {
	//to stop a tunnel, we just kill the ssh process. The helperthread will
	//take care of the rest (interface update, cleanup, etc)
	if (gSTMtunnels[id] != NULL && gSTMtunnels[id]->active && gSTMtunnels[id]->sshpid!=0) {
		kill(gSTMtunnels[id]->sshpid, SIGTERM);
		gSTMtunnels[id]->sshpid = 0;
	}
}
