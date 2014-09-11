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

#ifndef _TCP_H_
#define _TCP_H_

#ifdef _MSC_VER
#pragma warning( disable : 4996 4244 )
#define __attribute__(x)
#endif
void growl_tcp_write_raw(int sock, const unsigned char *data, const int data_length);
void growl_tcp_write_nl(int sock);
void growl_tcp_write(int sock , const char *const format , ...) __attribute__((format(printf, 2, 3)));
char *growl_tcp_read(int sock);
int growl_tcp_open(const char *server);
void growl_tcp_close(int sock);
int growl_tcp_datagram(const char *server , const unsigned char *data , const int data_length);

#endif /* _TCP_H_ */

/* vim:set et sw=2 ts=2 ai: */
