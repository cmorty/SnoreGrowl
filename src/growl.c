// [The "BSD licence"]
// Copyright (c) 2014-2015 Hannah von Reth <vonreth@kde.org>
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
#include <windows.h>
#else
#include <arpa/inet.h>
#include <stdint.h>
#include <pthread.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "md5.h"
#include "tcp.h"
#include "growl.h"

#define ERRSTR "GNTP/1.0 -ERROR NONE"

static const char hex_table[] = "0123456789ABCDEF";
static char *
string_to_hex_alloc(const char *str, int len)
{
    int n, l;
    char *tmp = (char *)calloc(1, len * 2 + 1);
    if (tmp) {
        for (l = 0, n = 0; l < len; l++) {
            tmp[n++] = hex_table[(str[l] & 0xF0) >> 4];
            tmp[n++] = hex_table[str[l] & 0x0F];
        }
    }
    return tmp;
}

static volatile int growl_init_ = 0;
static GROWL_CALLBACK gowl_callback = NULL;

GROWL_EXPORT
int growl_init(GROWL_CALLBACK callback)
{
    if (growl_init_ == 0) {
        gowl_callback = callback;
#ifdef _WIN32
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
            return -1;
        }
#endif

        srand(time(NULL));
        growl_init_ = 1;
    }
    return 1;
}

GROWL_EXPORT
int growl_shutdown()
{
    if (growl_init_ == 1) {
#ifdef _WIN32
        if (WSACleanup() != 0) {
            return -1;
        }
#endif
        growl_init_ = 0;
        return 1;
    }
    return -1;
}

GROWL_EXPORT
int growl_tcp_is_running(const char *const server)
{
    int sock = growl_tcp_open(server);
    if (sock != -1) {
        growl_tcp_close(sock);
        return 1;
    }
    return -1;
}

static char *
gen_salt_alloc(int count)
{
    char *salt = (char *)malloc(count + 1);
    if (salt) {
        int n;
        for (n = 0; n < count; n++) {
            salt[n] = (((int)rand()) % 255) + 1;
        }
        salt[n] = 0;
    }
    return salt;
}

static char *
gen_password_hash_alloc(const char *password, const char *salt)
{
    md5_context md5ctx;
    char md5tmp[20] = {0};
    char *md5digest;

    md5_starts(&md5ctx);
    md5_update(&md5ctx, (uint8_t *)password, strlen(password));
    md5_update(&md5ctx, (uint8_t *)salt, strlen(salt));
    md5_finish(&md5ctx, (uint8_t *)md5tmp);

    md5_starts(&md5ctx);
    md5_update(&md5ctx, (uint8_t *)md5tmp, 16);
    md5_finish(&md5ctx, (uint8_t *)md5tmp);
    md5digest = string_to_hex_alloc(md5tmp, 16);

    return md5digest;
}

static char *
growl_generate_authheader_alloc(const char *const password)
{
    char *auth_header = NULL;

    if (password && *password) {
        char *salt = gen_salt_alloc(8);
        if (salt) {
            char *keyhash = gen_password_hash_alloc(password, salt);
            if (keyhash) {
                char *salthash = string_to_hex_alloc(salt, 8);
                if (salthash) {
                    auth_header = (char *)malloc(strlen(keyhash) + strlen(salthash) + 7);
                    if (auth_header) {
                        sprintf(auth_header, " MD5:%s.%s", keyhash, salthash);
                    }
                    free(salthash);
                }
                free(keyhash);
            }
            free(salt);
        }
    }

    return auth_header;
}

#ifdef _WIN32
DWORD WINAPI growl_callback_thread(void *socket)
#else
void *growl_callback_thread(void *socket)
#endif
{
    int sock = *(int *)socket;
    if (gowl_callback) {
        growl_callback_data data;
        memset(&data, 0, sizeof(growl_callback_data));
        while (1) {
            char *line = growl_tcp_read(sock);
            if (line) {
                if (strncmp(line, "Notification-ID: ", 17) == 0) {
                    data.id = atoi(line + 17);
                } else if (strncmp(line, "Notification-Callback-Result: ", 30) == 0) {
                    size_t size = strlen(line + 30) + 1;
                    data.reason = malloc(size);
                    strncpy(data.reason, line + 30, size);
                } else if (strncmp(line, "Notification-Callback-Context: ", 31) == 0) {
                    size_t size = strlen(line + 31) + 1;
                    data.data = malloc(size);
                    strncpy(data.data, line + 31, size);
                } else if (strlen(line) == 0) {
                    free(line);
                    break;
                }
                free(line);
            }
        }

        gowl_callback(&data);
        free(data.reason);
        free(data.data);
    }

    growl_tcp_close(sock);
    free(socket);
    return 0;
}

GROWL_EXPORT
int
growl_tcp_register(
    const char *const server,
    const char *const appname,
    const char **const notifications,
    const int notifications_count,
    const char *const password,
    const char *const icon)
{
    int sock = -1;
    int i = 0;
    char *auth_header;
    char *icon_id = NULL;
    FILE *icon_file = NULL;
    long icon_size = 0;
    uint8_t buffer[1024];

    growl_init(NULL);
    auth_header = growl_generate_authheader_alloc(password);
    sock = growl_tcp_open(server);
    if (sock == -1) {
        goto leave;
    }

    if (icon) {
        size_t bytes_read;
        md5_context md5ctx;
        uint8_t md5tmp[20];
        icon_file = fopen(icon, "rb");
        if (icon_file) {
            fseek(icon_file, 0, SEEK_END);
            icon_size = ftell(icon_file);
            fseek(icon_file, 0, SEEK_SET);
            memset(md5tmp, 0, sizeof(md5tmp));
            md5_starts(&md5ctx);
            while (!feof(icon_file)) {
                bytes_read = fread(buffer, 1, 1024, icon_file);
                if (bytes_read) {
                    md5_update(&md5ctx, buffer, bytes_read);
                }
            }
            fseek(icon_file, 0, SEEK_SET);
            md5_finish(&md5ctx, md5tmp);
            icon_id = string_to_hex_alloc((const char *) md5tmp, 16);
        }
    }

    growl_tcp_write(sock, "GNTP/1.0 REGISTER NONE %s", auth_header ? auth_header : "");
    growl_tcp_write(sock, "Application-Name: %s", appname);
    if (icon_id) {
        growl_tcp_write(sock, "Application-Icon: x-growl-resource://%s", icon_id);
    } else if (icon) {
        growl_tcp_write(sock, "Application-Icon: %s", icon);
    }
    growl_tcp_write(sock, "Notifications-Count: %d", notifications_count);
    growl_tcp_write(sock, "%s", "");

    for (i = 0; i < notifications_count; i++) {
        growl_tcp_write(sock, "Notification-Name: %s", notifications[i]);
        growl_tcp_write(sock, "Notification-Display-Name: %s", notifications[i]);
        growl_tcp_write(sock, "Notification-Enabled: True");
        if (icon_id) {
            growl_tcp_write(sock, "Notification-Icon: x-growl-resource://%s", icon_id);
        } else if (icon) {
            growl_tcp_write(sock, "Notification-Icon: %s", icon);
        }
        growl_tcp_write(sock, "%s", "");
    }

    if (icon_id) {
        growl_tcp_write(sock, "Identifier: %s", icon_id);
        growl_tcp_write(sock, "Length: %ld", icon_size);
        growl_tcp_write(sock, "%s", "");

        while (!feof(icon_file)) {
            size_t bytes_read = fread(buffer, 1, 1024, icon_file);
            if (bytes_read) {
                growl_tcp_write_raw(sock, (const unsigned char *)buffer, bytes_read);
            }
        }
        growl_tcp_write(sock, "%s", "");
    }
    growl_tcp_write(sock, "%s", "");

    while (1) {
        char *line = growl_tcp_read(sock);
        if (!line) {
            growl_tcp_close(sock);
            sock = -1;
            goto leave;
        } else {
            int len = strlen(line);
            if (!strncmp(line, ERRSTR, strlen(ERRSTR))) {
                fprintf(stderr, "Failed to register notification:\n%s\n", line);
                free(line);
                //Print full error-message
                while((line = growl_tcp_read(sock)) != 0){
                    fprintf(stderr, "%s\n", line);
                    char firstchar = line[0];
                    free(line);
                    if(firstchar == '\0') break;
                }
                goto leave;
            }
            free(line);
            if (len == 0) {
                break;
            }
        }
    }
    growl_tcp_close(sock);
    sock = 0;

leave:
    if (icon_file) {
        fclose(icon_file);
    }
    if (icon_id) {
        free(icon_id);
    }
    free(auth_header);

    return (sock == 0) ? 0 : -1;
}

void
growl_notification_with_icon(int sock, const char *icon)
{
    uint8_t buffer[1024];
    char *icon_id = NULL;
    FILE *icon_file = NULL;
    long icon_size = 0;
    size_t bytes_read;
    md5_context md5ctx;
    uint8_t md5tmp[20];
    icon_file = fopen(icon, "rb");
    if (icon_file) {
        fseek(icon_file, 0, SEEK_END);
        icon_size = ftell(icon_file);
        fseek(icon_file, 0, SEEK_SET);
        memset(md5tmp, 0, sizeof(md5tmp));
        md5_starts(&md5ctx);
        while (!feof(icon_file)) {
            bytes_read = fread(buffer, 1, 1024, icon_file);
            if (bytes_read) {
                md5_update(&md5ctx, buffer, bytes_read);
            }
        }
        fseek(icon_file, 0, SEEK_SET);
        md5_finish(&md5ctx, md5tmp);
        icon_id = string_to_hex_alloc((const char *) md5tmp, 16);
    }
    if (icon_id) {
        growl_tcp_write(sock, "Notification-Icon: x-growl-resource://%s", icon_id);
    } else if (icon) {
        growl_tcp_write(sock, "Notification-Icon: %s", icon);
    }

    if (icon_id) {
        growl_tcp_write_nl(sock);
        growl_tcp_write(sock, "Identifier: %s", icon_id);
        growl_tcp_write(sock, "Length: %ld", icon_size);
        growl_tcp_write_nl(sock);
        while (!feof(icon_file)) {
            size_t bytes_read = fread(buffer, 1, 1024, icon_file);
            if (bytes_read) {
                growl_tcp_write_raw(sock, (const unsigned char *)buffer, bytes_read);
            }
        }
        growl_tcp_write_nl(sock);
    }

    if (icon_file) {
        fclose(icon_file);
    }
    if (icon_id) {
        free(icon_id);
    }
}

void
growl_notification_with_data_icon(int sock, const char *icon_data, size_t icon_data_size)
{

    char *icon_id = NULL;
    if (icon_data) {
        md5_context md5ctx;
        uint8_t md5tmp[20];
        memset(md5tmp, 0, sizeof(md5tmp));
        md5_starts(&md5ctx);
        md5_update(&md5ctx, (uint8_t *)icon_data, icon_data_size);
        md5_finish(&md5ctx, md5tmp);
        icon_id = string_to_hex_alloc((const char *) md5tmp, 16);
    }
    if (icon_id) {
        size_t rest = icon_data_size;
        unsigned char *ptr = (unsigned char *) icon_data;
        growl_tcp_write(sock, "Notification-Icon: x-growl-resource://%s", icon_id);
        growl_tcp_write(sock, "%s", "");
        growl_tcp_write(sock, "Identifier: %s", icon_id);
        growl_tcp_write(sock, "Length: %ld", (unsigned long)icon_data_size);
        growl_tcp_write(sock, "%s", "");
        while (rest > 0) {
            long send_size = rest > 1024 ? 1024 : rest;
            growl_tcp_write_raw(sock, (const unsigned char *)ptr, send_size);
            ptr += send_size;
            rest -= send_size;
        }
        growl_tcp_write_nl(sock);
    }
    if (icon_id) {
        free(icon_id);
    }
}

GROWL_EXPORT
int growl_tcp_notify(const char *const server,
                     const char *const password,
                     const growl_notification_data *data)
{
    int sock = -1;

    char *auth_header = growl_generate_authheader_alloc(password);

    growl_init(NULL);

    sock = growl_tcp_open(server);
    if (sock == -1) {
        goto leave;
    }
    growl_tcp_write(sock, "GNTP/1.0 NOTIFY NONE %s", auth_header ? auth_header : "");
    growl_tcp_write(sock, "Application-Name: %s", data->app_name);
    growl_tcp_write(sock, "Notification-Name: %s", data->notify);

    growl_tcp_write(sock, "Notification-ID: %i", data->id);
    growl_tcp_write(sock, "Notification-Title: %s", data->title);
    growl_tcp_write(sock, "Notification-Text: %s", data->message);

    if (data->url) {
        growl_tcp_write(sock, "Notification-Callback-Target: %s", data->url);
    } else if (data->callback_context) {
        growl_tcp_write(sock, "Notification-Callback-Context: %s", data->callback_context);
        growl_tcp_write(sock, "Notification-Callback-Context-Type: string");
    }

    if (data->icon_data) {
        growl_notification_with_data_icon(sock, data->icon_data, data->icon_data_size);
    } else if (data->icon) {
        growl_notification_with_icon(sock, data->icon);
    }

    growl_tcp_write_nl(sock);

    while (1) {
        char *line = growl_tcp_read(sock);
        if (!line) {
            growl_tcp_close(sock);
            sock = -1;
            goto leave;
        } else {
            int len = strlen(line);
            if (!strncmp(line, ERRSTR, strlen(ERRSTR))) {
                fprintf(stderr, "Failed to post notification:\n%s\n", line);
                free(line);
                //Print full error-message
                while((line = growl_tcp_read(sock)) != 0){
                    fprintf(stderr, "%s\n", line);
                    char firstchar = line[0];
                    free(line);
                    if(firstchar == '\0') break;
                }
                goto leave;
            }
            free(line);
            if (len == 0) {
                break;
            }
        }
    }

    if (data->callback_context) {
        int *socket = malloc(sizeof(int));
        *socket = sock;
#ifdef _WIN32
        CreateThread(NULL, 0, growl_callback_thread, socket, 0, NULL);
#else
        pthread_t thread;
        pthread_create(&thread, NULL, growl_callback_thread, socket);

#endif
    } else {
        growl_tcp_close(sock);
    }

    sock = 0;

leave:
    free(auth_header);
    return (sock == 0) ? 0 : -1;
}

void
growl_append_md5(
    unsigned char *const data,
    const int data_length,
    const char *const password)
{

    md5_context md5ctx;
    char md5tmp[20] = {0};

    md5_starts(&md5ctx);
    md5_update(&md5ctx, (uint8_t *)data, data_length);
    if (password && *password) {
        md5_update(&md5ctx, (uint8_t *)password, strlen(password));
    }
    md5_finish(&md5ctx, (uint8_t *)md5tmp);

    memcpy(data + data_length, md5tmp, 16);
}

GROWL_EXPORT
int growl_udp_register(
    const char *const server,
    const char *const appname,
    const char **const notifications,
    const int notifications_count,
    const char *const password)
{

    int register_header_length = 22 + strlen(appname);
    unsigned char *data;
    int pointer = 0;
    int rc = 0;
    int i = 0;

    uint8_t GROWL_PROTOCOL_VERSION  = 1;
    uint8_t GROWL_TYPE_REGISTRATION = 0;

    uint16_t appname_length = ntohs(strlen(appname));
    uint8_t _notifications_count = notifications_count;
    uint8_t default_notifications_count = notifications_count;
    uint8_t j;

    growl_init(NULL);

    for (i = 0; i < notifications_count; i++) {
        register_header_length += 3 + strlen(notifications[i]);
    }
    data = (unsigned char *)calloc(1, register_header_length);
    if (!data) {
        return -1;
    }

    pointer = 0;
    memcpy(data + pointer, &GROWL_PROTOCOL_VERSION, 1);
    pointer++;
    memcpy(data + pointer, &GROWL_TYPE_REGISTRATION, 1);
    pointer++;
    memcpy(data + pointer, &appname_length, 2);
    pointer += 2;
    memcpy(data + pointer, &_notifications_count, 1);
    pointer++;
    memcpy(data + pointer, &default_notifications_count, 1);
    pointer++;
    sprintf((char *)data + pointer, "%s", appname);
    pointer += strlen(appname);

    for (i = 0; i < notifications_count; i++) {
        uint16_t notify_length = ntohs(strlen(notifications[i]));
        memcpy(data + pointer, &notify_length, 2);
        pointer += 2;
        sprintf((char *)data + pointer, "%s", notifications[i]);
        pointer += strlen(notifications[i]);
    }

    for (j = 0; j < notifications_count; j++) {
        memcpy(data + pointer, &j, 1);
        pointer++;
    }

    growl_append_md5(data, pointer, password);
    pointer += 16;

    rc = growl_tcp_datagram(server, data, pointer);
    free(data);
    return rc;
}

GROWL_EXPORT
int growl_udp_notify(
    const char *const server,
    const char *const password,
    const growl_notification_data *notification)
{

    int notify_header_length = 28 + strlen(notification->app_name) + strlen(notification->notify) + strlen(notification->message) + strlen(notification->title);
    unsigned char *data = (unsigned char *)calloc(1, notify_header_length);
    int pointer = 0;
    int rc = 0;

    uint8_t GROWL_PROTOCOL_VERSION  = 1;
    uint8_t GROWL_TYPE_NOTIFICATION = 1;

    uint16_t flags = ntohs(0);
    uint16_t appname_length = ntohs(strlen(notification->app_name));
    uint16_t notify_length = ntohs(strlen(notification->notify));
    uint16_t title_length = ntohs(strlen(notification->title));
    uint16_t message_length = ntohs(strlen(notification->message));

    if (!data) {
        return -1;
    }

    growl_init(NULL);

    pointer = 0;
    memcpy(data + pointer, &GROWL_PROTOCOL_VERSION, 1);
    pointer++;
    memcpy(data + pointer, &GROWL_TYPE_NOTIFICATION, 1);
    pointer++;
    memcpy(data + pointer, &flags, 2);
    pointer += 2;
    memcpy(data + pointer, &notify_length, 2);
    pointer += 2;
    memcpy(data + pointer, &title_length, 2);
    pointer += 2;
    memcpy(data + pointer, &message_length, 2);
    pointer += 2;
    memcpy(data + pointer, &appname_length, 2);
    pointer += 2;
    strcpy((char *)data + pointer, notification->notify);
    pointer += strlen(notification->notify);
    strcpy((char *)data + pointer, notification->title);
    pointer += strlen(notification->title);
    strcpy((char *)data + pointer, notification->message);
    pointer += strlen(notification->message);
    strcpy((char *)data + pointer, notification->app_name);
    pointer += strlen(notification->app_name);

    growl_append_md5(data, pointer, password);
    pointer += 16;

    rc = growl_tcp_datagram(server, data, pointer);
    free(data);
    return rc;
}

/* vim:set et sw=2 ts=2 ai: */
