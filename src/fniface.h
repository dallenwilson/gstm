#ifndef _FNIFACE_H
#define _FNIFACE_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <gnome.h>
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
void gstm_interface_enablebuttons(gboolean active);
void gstm_interface_disablebuttons();
void gstm_interface_paint_row(GtkTreeSelection *s, gboolean active);
void gstm_interface_paint_row_id(int id, gboolean active);
void gstm_interface_refresh_row_id(int id, const char *name);
void gstm_interface_rowactivity();
void gstm_interface_rowaction();
void gstm_interface_error(const char *msg);
void gstm_interface_redirlist_init(GtkTreeView *v);
void gstm_interface_redirlist_add(struct portredir *r, int idx, GtkTreeView *v);
void gstm_interface_redirlist_edit(struct portredir *r, int idx, GtkTreeView *v);
void gstm_interface_redirlist_del(GtkTreeView *v, int id);
void gstm_interface_properties(int tid);
void gstm_dockletmenu_tunnelitem_new(GtkWidget *menu, const gchar *t_name, intptr_t t_id, gboolean t_active);
GtkWidget* gstm_create_dockletmenu (void);
int gstm_interface_asknewname(gchar **nname);
	
#ifdef __cplusplus
}
#endif

#endif /* _FNIFACE_H */
