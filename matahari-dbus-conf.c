#define _POSIX_C_SOURCE 199309L
#include <stdio.h>
#include <string.h>
#include <matahari/mh_dbus_common.h>

#include "conf.h"
#include <dbus/dbus-glib-lowlevel.h>
#include <stdlib.h>

#define CONF_BUS_NAME "org.matahariproject.Config"
#define CONF_OBJECT_PATH "/org/matahariproject/Config"
#define CONF_INTERFACE_NAME "org.matahariproject.Config"

enum { MATAHARI_CONFIG_FAILED, MATAHARI_NOT_OWNER };

Property properties[];
// List of owners of handles
GArray *owners;

/** Each method should check if caller of the method is the same as owner of
 * augeas instance (who called the init method).
 */
gboolean
check_ownership(int handle, DBusGMethodInvocation *context)
{
    // Check ownership of the handle
    gchar *owner = g_array_index(owners, char *, handle);
    if (g_strcmp0(owner, dbus_g_method_get_sender(context)) != 0)
    {
        GError *error = g_error_new(MATAHARI_ERROR, MATAHARI_NOT_OWNER, "Caller is not owner of a handle");
        fprintf(stderr, "Caller (%s) is not owner (%s) of a handle\n", dbus_g_method_get_sender(context), owner);
        dbus_g_method_return_error(context, error);
        return FALSE;
    }
    return TRUE;
}

void
return_error(int handle, DBusGMethodInvocation *context)
{
    char *message = NULL, *message_minor = NULL, *details = NULL;
    conf_error_verbose(handle, message, message_minor, details);
    if (message == NULL)
        message = "No Augeas context with this handle";
    GError *error = g_error_new(MATAHARI_ERROR, MATAHARI_CONFIG_FAILED, message);
    dbus_g_method_return_error(context, error);
}

int
Config_init(Matahari* matahari, const char *lens, DBusGMethodInvocation *context)
{
    int handle = conf_init(lens);
    if (handle < 0)
    {
        GError *error = g_error_new(MATAHARI_ERROR, MATAHARI_CONFIG_FAILED, "Unable to initialize Augeas.");
        dbus_g_method_return_error(context, error);
    }
    printf("Initializing Augeas for sender: %s\n", dbus_g_method_get_sender(context));
    char *o = g_strdup(dbus_g_method_get_sender(context));
    g_array_append_val(owners, o);
    dbus_g_method_return(context, handle);
    return TRUE;
}

int
Config_get(Matahari* matahari, int handle, const char *path, DBusGMethodInvocation *context)
{
    const char *value = NULL;
    if (!check_ownership(handle, context))
        return FALSE;
    int result = conf_get(handle, path, &value);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    printf("Get from: %s\n", dbus_g_method_get_sender(context));
    dbus_g_method_return(context, value, result);
    return TRUE;
}

int
Config_set(Matahari* matahari, int handle, const char *path, const char *value, DBusGMethodInvocation *context)
{
    if (!check_ownership(handle, context))
        return FALSE;
    int result = conf_set(handle, path, value);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_setm(Matahari *matahari, int handle, const char *base, const char *sub, const char *value, DBusGMethodInvocation *context)
{
    if (!check_ownership(handle, context))
        return FALSE;
    int result = conf_setm(handle, base, sub, value);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_insert(Matahari *matahari, int handle, const char *path, const char *label, int before, DBusGMethodInvocation *context)
{
    if (!check_ownership(handle, context))
        return FALSE;
    int result = conf_insert(handle, path, label, before);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_mv(Matahari *matahari, int handle, const char *src, const char *dst, DBusGMethodInvocation *context)
{
    if (!check_ownership(handle, context))
        return FALSE;
    int result = conf_mv(handle, src, dst);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_rm(Matahari *matahari, int handle, const char *path, DBusGMethodInvocation *context)
{
    if (!check_ownership(handle, context))
        return FALSE;
    int result = conf_rm(handle, path);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_match(Matahari *matahari, int handle, const char *path, DBusGMethodInvocation *context)
{
    if (!check_ownership(handle, context))
        return FALSE;
    char **matches = NULL;
    int result = conf_match(handle, path, &matches);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    if (result > 0)
    {
        // Add a sentinel
        matches = realloc(matches, (result + 1) * sizeof(char **));
        matches[result] = NULL;
    }
    dbus_g_method_return(context, matches, result);
    g_strfreev(matches);
    return TRUE;
}

int
Config_save(Matahari *matahari, int handle, DBusGMethodInvocation *context)
{
    GError *error = NULL;
    if (!check_ownership(handle, context))
        return FALSE;

    if (!check_authorization(CONF_BUS_NAME ".save", &error, context))
    {
        dbus_g_method_return_error(context, error);
        return FALSE;
    }

    int result = conf_save(handle);
    if (result < 0)
    {
        return_error(handle, context);
        return FALSE;
    }
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_error(Matahari *matahari, int handle, DBusGMethodInvocation *context)
{
    int result = conf_error(handle);
    dbus_g_method_return(context, result);
    return TRUE;
}

int
Config_error_verbose(Matahari *matahari, int handle, DBusGMethodInvocation *context)
{
    char *message = NULL, *message_minor = NULL, *details = NULL;
    conf_error_verbose(handle, message, message_minor, details);
    dbus_g_method_return(context, message, message_minor, details);
    return TRUE;
}

int
Config_close(Matahari *matahari, int handle, DBusGMethodInvocation *context)
{
    conf_close(handle);
    dbus_g_method_return(context);
    return TRUE;
}


#include "conf-dbus-glue.h"

void 
matahari_set_property(GObject *object, guint property_id, const GValue *value, 
                      GParamSpec *pspec)
{

}

void 
matahari_get_property(GObject *object, guint property_id, GValue *value, 
                      GParamSpec *pspec)
{

}

GType
matahari_dict_type(int prop)
{
    fprintf(stderr, "Type of property %d is map of unknown types\n", prop);
    return G_TYPE_VALUE;
}

int
main(int argc, char *argv[])
{
    g_type_init();
    owners = g_array_new(FALSE, FALSE, sizeof(char *));
    return run_dbus_server(CONF_BUS_NAME, CONF_OBJECT_PATH);
}

