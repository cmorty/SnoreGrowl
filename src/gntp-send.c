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

#ifdef _WIN32
#include <winsock2.h>
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "md5.h"
#include "tcp.h"
#include "growl.h"

int wait_for_callback = 0;

static char *
string_to_utf8_alloc(const char *str)
{
#ifdef _WIN32
    unsigned int codepage;
    size_t in_len = strlen(str);
    wchar_t *wcsdata;
    char *mbsdata;
    size_t mbssize, wcssize;

    codepage = GetACP();
    wcssize = MultiByteToWideChar(codepage, 0, str, in_len,  NULL, 0);
    wcsdata = (wchar_t *) malloc((wcssize + 1) * sizeof(wchar_t));
    wcssize = MultiByteToWideChar(codepage, 0, str, in_len, wcsdata, wcssize + 1);
    wcsdata[wcssize] = 0;

    mbssize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wcsdata, -1, NULL, 0, NULL, NULL);
    mbsdata = (char *) malloc((mbssize + 1));
    mbssize = WideCharToMultiByte(CP_UTF8, 0, (LPCWSTR) wcsdata, -1, mbsdata, mbssize, NULL, NULL);
    mbsdata[mbssize] = 0;
    free(wcsdata);
    return mbsdata;
#else
    return strdup(str);
#endif
}

static int  opterr = 1;
static int  optind = 1;
static int  optopt;
static char *optarg;

static int
getopts(int argc, char **argv, char *opts)
{
    static int sp = 1;
    register int c;
    register char *cp;

    if (sp == 1) {
        if (optind >= argc ||
                argv[optind][0] != '-' || argv[optind][1] == '\0') {
            return (EOF);
        } else if (strcmp(argv[optind], "--") == 0) {
            optind++;
            return (EOF);
        }
    }
    optopt = c = argv[optind][sp];
    if (c == ':' || (cp = strchr(opts, c)) == NULL) {
        if (argv[optind][++sp] == '\0') {
            optind++;
            sp = 1;
        }
        return ('?');
    }
    if (*++cp == ':') {
        if (argv[optind][sp + 1] != '\0') {
            optarg = &argv[optind++][sp + 1];
        } else if (++optind >= argc) {
            sp = 1;
            return ('?');
        } else {
            optarg = argv[optind++];
        }
        sp = 1;
    } else {
        if (argv[optind][++sp] == '\0') {
            sp = 1;
            optind++;
        }
        optarg = NULL;
    }
    return (c);
}

void callback(const growl_callback_data *data)
{
    printf("Recieved a calbback for ID: %i, Reason: %s, Data: %s\n", data->id, data->reason, data->data);
    wait_for_callback = 0;
}

int
main(int argc, char *argv[])
{
    int c;
    int rc;
    char *server = NULL;
    char *password = NULL;
    char *appname = "gntp-send";
    char *notify = "gntp-send notify";
    char *title = NULL;
    char *message = NULL;
    char *icon = NULL;
    char *url = NULL;
    int tcpsend = 1;
    int read_stdin = 0;

    opterr = 0;
    while ((c = getopts(argc, argv, "a:n:s:p:ui")) != -1) {
        switch (optopt) {
        case 'a': appname = optarg; break;
        case 'n': notify = optarg; break;
        case 's': server = optarg; break;
        case 'p': password = optarg; break;
        case 'u': tcpsend = 0; break;
        case 'i': read_stdin = 1; break;
        case 'w' : wait_for_callback = 1;
        case '?': break;
        default: argc = 0; break;
        }
        optarg = NULL;
    }

    if (!read_stdin && ((argc - optind) < 2 || (argc - optind) > 4)) {
        fprintf(stderr, "%s: [-u] [-i] [-w] [-a APPNAME] [-n NOTIFY] [-s SERVER:PORT] [-p PASSWORD] title message [icon] [url]\n", argv[0]);
        exit(1);
    }

    if (!read_stdin) {
        title = string_to_utf8_alloc(argv[optind]);
        message = string_to_utf8_alloc(argv[optind + 1]);
        if ((argc - optind) >= 3) {
            icon = string_to_utf8_alloc(argv[optind + 2]);
        }
        if ((argc - optind) == 4) {
            url = string_to_utf8_alloc(argv[optind + 3]);
        }
    } else {
        char buf[BUFSIZ], *ptr;
        if (fgets(buf, sizeof(buf) - 1, stdin) == NULL) {
            exit(1);
        }
        if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
            *ptr = 0;
        }
        title = strdup(buf);
        while (fgets(buf, sizeof(buf) - 1, stdin)) {
            if ((ptr = strpbrk(buf, "\r\n")) != NULL) {
                *ptr = 0;
            }
            if (!message) {
                message = malloc(strlen(buf) + 2);
                *message = 0;
            } else {
                strcat(message, "\n");
                message = realloc(message, strlen(message) + strlen(buf) + 2);
            }
            strcat(message, buf);
        }
        if ((argc - optind) >= 1) {
            icon = string_to_utf8_alloc(argv[optind]);
        }
        if ((argc - optind) == 2) {
            url = string_to_utf8_alloc(argv[optind + 1]);
        }
    }

    if (!server) {
        server = "127.0.0.1";
    }

    growl_init(wait_for_callback ? callback : NULL);

    growl_notification_data notification;
    memset(&notification, 0, sizeof(growl_notification_data));
    notification.app_name = appname;
    notification.notify = notify;
    notification.title = title;
    notification.message = message;
    notification.icon = icon;
    notification.url = url;

    if (wait_for_callback) {
        notification.callback_context = "This ia  a test callback context";
    }

    if (tcpsend) {
        rc = growl_tcp_register(server, appname, (const char **const)&notify, 1, password, icon);
        if (rc == 0) {
            growl_tcp_notify(server, password, &notification);
        }
    } else {
        rc = growl_udp_register(server, appname, (const char **const)&notify, 1, password);
        if (rc == 0) {
            rc = growl_udp_notify(server, password, &notification);
        }
    }

    while (wait_for_callback);
    growl_shutdown();

    if (title) {
        free(title);
    }
    if (message) {
        free(message);
    }
    if (icon) {
        free(icon);
    }
    if (url) {
        free(url);
    }

    return rc;
}

/* vim:set et sw=2 ts=2 ai: */
