#define _GNU_SOURCE

#include "conf.h"

#include <augeas.h>
#include <string.h>
#include <stdlib.h>

#define WRONG_HANDLE -2
#define ALLOC_AT_ONCE 5

augeas **aug = NULL;
int max_handle = 0;

int conf_init(const char *lens)
{
    static int free = ALLOC_AT_ONCE;
    char *path = NULL;
    if (aug == NULL)
    {
        aug = malloc(ALLOC_AT_ONCE * sizeof(augeas *));
    }

    if (free == 0)
    {
        aug = realloc(aug, (max_handle + ALLOC_AT_ONCE) * sizeof(augeas *));
        free = ALLOC_AT_ONCE;
    }
    if (aug == NULL)
        return -1;

    aug[max_handle] = aug_init("", "", AUG_SAVE_BACKUP | AUG_NO_LOAD);
    asprintf(&path, "/augeas/load/*[label() != '%s']", lens);
    aug_rm(aug[max_handle], path);
    aug_load(aug[max_handle]);
    if (aug[max_handle] == NULL)
        return -1;

    return max_handle++;
}

int conf_get(int handle, const char *path, const char **value)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_get(aug[handle], path, value);
}

int conf_set(int handle, const char *path, const char *value)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_set(aug[handle], path, value);
}

int conf_setm(int handle, const char *base, const char *sub, const char *value)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_setm(aug[handle], base, sub, value);
}

int conf_insert(int handle, const char *path, const char *label, int before)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_insert(aug[handle], path, label, before);
}

int conf_mv(int handle, const char *src, const char *dst)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_mv(aug[handle], src, dst);
}

int conf_rm(int handle, const char *path)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_rm(aug[handle], path);
}

int conf_match(int handle, const char *path, char ***matches)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_match(aug[handle], path, matches);
}

int conf_save(int handle)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_save(aug[handle]);
}

int conf_error(int handle)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    return aug_error(aug[handle]);
}

int conf_error_verbose(int handle, const char *message, const char *minor_message, const char *details)
{
    if (handle >= max_handle || aug[handle] == NULL)
        return WRONG_HANDLE;
    message = aug_error_message(aug[handle]);
    minor_message = aug_error_minor_message(aug[handle]);
    details = aug_error_details(aug[handle]);
    return 0;
}

void conf_close(int handle)
{
    if (handle < max_handle && aug[handle] != NULL)
    {
        aug_close(aug[handle]);
        aug[handle] = NULL;
    }
}
