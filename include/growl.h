#ifndef _GROWL_H_
#define _GROWL_H_

#include <string.h>

#include "growl_exports.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct notification_data
{
    const char *app_name;
    int id;
    const char *notify;
    const char *title;
    const char *message;
    const char *icon;
    const char *icon_data;
    size_t icon_data_size;
    const char *url;
    const char *callback_context;


}growl_notification_data;

typedef struct callback_data
{
    int id;
    char *reason;
    char *data;

}growl_callback_data;

typedef void (*GROWL_CALLBACK)(const growl_callback_data* data);

GROWL_EXPORT int growl_tcp_notify( const char *const server, const char *const password, const growl_notification_data *data );
GROWL_EXPORT int growl_tcp_register( const char *const server , const char *const appname , const char **const notifications , const int notifications_count , const char *const password, const char *const icon );


GROWL_EXPORT int growl_udp_notify( const char *const server, const char *const password, const growl_notification_data *data);
GROWL_EXPORT int growl_udp_register( const char *const server , const char *const appname , const char **const notifications , const int notifications_count , const char *const password  );


GROWL_EXPORT int growl_init(GROWL_CALLBACK callback);

GROWL_EXPORT int growl_shutdown();

GROWL_EXPORT int growl_tcp_is_running(const char *const server);


#ifdef __cplusplus
}
#endif


#endif /* _GROWL_H_ */

/* vim:set et sw=2 ts=2 ai: */
