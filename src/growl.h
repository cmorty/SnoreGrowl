// [The "BSD licence"]
// Copyright (c) 2014 Patrick von Reth <vonreth@kde.org>
// Copyright (c) 2009-2011 Yasuhiro Matsumoto
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//  1. Redistributions of source code must retain the above copyright
//     notice, this list of conditions and the following disclaimer.
//  2. Redistributions in binary form must reproduce the above copyright
//     notice, this list of conditions and the following disclaimer in the
//     documentation and/or other materials provided with the distribution.
//  3. The name of the author may not be used to endorse or promote products
//     derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
// OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
// IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
// NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
// THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

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
