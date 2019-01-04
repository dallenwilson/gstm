#ifndef _FNSSHT_H
#define _FNSSHT_H

#ifdef __cplusplus
extern "C"
{
#endif

struct Shelperargs {
	int tid;
	char **sshargs;
};
	
int gstm_ssht_starttunnel(int id);
int gstm_ssht_stoptunnel(int id);

#ifdef __cplusplus
}
#endif

#endif /* _FNSSHT_H */
