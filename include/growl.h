#ifndef _GROWL_H_
#define _GROWL_H_

#ifdef _WIN32
  #ifndef GROWL_STATIC
    #ifdef GROWL_DLL
      #define GROWL_EXPORT __declspec(dllexport)
    #else
      #define GROWL_EXPORT __declspec(dllimport)
    #endif
  #else
    #define GROWL_EXPORT
  #endif
#else
  #define GROWL_EXPORT
#endif /*_WIN32 */

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
    int icon_data_size;
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


GROWL_EXPORT int growl_init(void);

GROWL_EXPORT void growl_shutdown(void);

GROWL_EXPORT void growl_set_callback(GROWL_CALLBACK callback);


#ifdef __cplusplus
}
#endif


#endif /* _GROWL_H_ */

/* vim:set et sw=2 ts=2 ai: */
