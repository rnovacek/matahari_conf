
#ifndef CONF_H
#define CONF_H

int conf_init(const char *lens);
int conf_get(int handle, const char *path, const char **value);
int conf_set(int handle, const char *path, const char *value);
int conf_setm(int handle, const char *base, const char *sub, const char *value);
int conf_insert(int handle, const char *path, const char *label, int before);
int conf_mv(int handle, const char *src, const char *dst);
int conf_rm(int handle, const char *path);
int conf_match(int handle, const char *path, char ***matches);
int conf_save(int handle);
int conf_error(int handle);
int conf_error_verbose(int handle, const char *message, const char *minor_message, const char *details);
void conf_close(int handle);

#endif
