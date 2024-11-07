/*
 * Copyright (C) 2005 Philippe Gerum <rpm@xenomai.org>.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 */
#pragma GCC system_header
#include_next <unistd.h>

#ifndef _EPOS_UNISTD_H
#define _EPOS_UNISTD_H

#include <epos/wrappers.h>

#ifdef __cplusplus
extern "C" {
#endif

// EPOS_DECL(ssize_t, read(int fd, void *buf, size_t nbyte));

// EPOS_DECL(ssize_t, write(int fd, const void *buf, size_t nbyte));

// EPOS_DECL(int, close(int fildes));

EPOS_DECL(unsigned int, sleep(unsigned int seconds));

EPOS_DECL(int, usleep(useconds_t usec));

#ifdef __cplusplus
}
#endif

#endif /* !_EPOS_UNISTD_H */
