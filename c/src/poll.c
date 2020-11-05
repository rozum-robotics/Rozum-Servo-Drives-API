/* Emulation for poll(2)
   Contributed by Paolo Bonzini.

   Copyright 2001-2003, 2006-2018 Free Software Foundation, Inc.

   This file is part of gnulib.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along
   with this program; if not, see <https://www.gnu.org/licenses/>.  */

#include "poll.h"

#include <sys/types.h>
#include <errno.h>
#include <limits.h>

#if defined _WIN32 && ! defined __CYGWIN__
# include <winsock2.h>
# include <windows.h>
# include <io.h>
# include <stdio.h>
# include <conio.h>
#else
# include <sys/socket.h>
# include <sys/time.h>
# include <unistd.h>
#endif

#include <time.h>

#ifndef INFTIM
# define INFTIM (-1)
#endif

/* Convert select(2) returned fd_sets into poll(2) revents values.  */
static int
compute_revents (int fd, int sought, fd_set *rfds, fd_set *wfds, fd_set *efds)
{
  int happened = 0;
  if (FD_ISSET (fd, rfds))
    {
      int r;
      int socket_errno;

      char data[64];
      r = recv (fd, data, sizeof (data), MSG_PEEK);
      socket_errno = (r < 0) ? errno : 0;
      if (r == 0)
        happened |= POLLHUP;

      /* If the event happened on an unconnected server socket,
         that's fine. */
      else if (r > 0 || ( /* (r == -1) && */ socket_errno == ENOTCONN))
        happened |= (POLLIN | POLLRDNORM) & sought;

      /* Distinguish hung-up sockets from other errors.  */
      else if (socket_errno == /*ESHUTDOWN*/ENETDOWN || socket_errno == ECONNRESET
               || socket_errno == ECONNABORTED || socket_errno == ENETRESET)
        happened |= POLLHUP;

      /* some systems can't use recv() on non-socket, including HP NonStop */
      else if (socket_errno == ENOTSOCK)
        happened |= (POLLIN | POLLRDNORM) & sought;

      else
        happened |= POLLERR;
    }

  if (FD_ISSET (fd, wfds))
    happened |= (POLLOUT | POLLWRNORM | POLLWRBAND) & sought;

  if (FD_ISSET (fd, efds))
    happened |= (POLLPRI | POLLRDBAND) & sought;

  return happened;
}

int
poll (struct pollfd *pfd, nfds_t nfd, int timeout)
{
  fd_set rfds, wfds, efds;
  struct timeval tv;
  struct timeval *ptv;
  int maxfd, rc;
  nfds_t i;

  if (nfd > INT_MAX)
    {
      errno = EINVAL;
      return -1;
    }
  /* Don't check directly for NFD greater than OPEN_MAX.  Any practical use
     of a too-large NFD is caught by one of the other checks below, and
     checking directly for getdtablesize is too much of a portability
     and/or performance and/or correctness hassle.  */

  /* EFAULT is not necessary to implement, but let's do it in the
     simplest case. */
  if (!pfd && nfd)
    {
      errno = EFAULT;
      return -1;
    }

  /* convert timeout number into a timeval structure */
  if (timeout == 0)
    {
      ptv = &tv;
      ptv->tv_sec = 0;
      ptv->tv_usec = 0;
    }
  else if (timeout > 0)
    {
      ptv = &tv;
      ptv->tv_sec = timeout / 1000;
      ptv->tv_usec = (timeout % 1000) * 1000;
    }
  else if (timeout == INFTIM)
    /* wait forever */
    ptv = NULL;
  else
    {
      errno = EINVAL;
      return -1;
    }

  /* create fd sets and determine max fd */
  maxfd = -1;
  FD_ZERO (&rfds);
  FD_ZERO (&wfds);
  FD_ZERO (&efds);
  for (i = 0; i < nfd; i++)
    {
      if (pfd[i].fd < 0)
        continue;
      if (maxfd < pfd[i].fd)
        {
          maxfd = pfd[i].fd;
          if (FD_SETSIZE <= maxfd)
            {
              errno = EINVAL;
              return -1;
            }
        }
      if (pfd[i].events & (POLLIN | POLLRDNORM))
        FD_SET (pfd[i].fd, &rfds);
      /* see select(2): "the only exceptional condition detectable
         is out-of-band data received on a socket", hence we push
         POLLWRBAND events onto wfds instead of efds. */
      if (pfd[i].events & (POLLOUT | POLLWRNORM | POLLWRBAND))
        FD_SET (pfd[i].fd, &wfds);
      if (pfd[i].events & (POLLPRI | POLLRDBAND))
        FD_SET (pfd[i].fd, &efds);
    }

  /* examine fd sets */
  rc = select (maxfd + 1, &rfds, &wfds, &efds, ptv);
  if (rc < 0)
    return rc;

  /* establish results */
  rc = 0;
  for (i = 0; i < nfd; i++)
    {
      pfd[i].revents = (pfd[i].fd < 0
                        ? 0
                        : compute_revents (pfd[i].fd, pfd[i].events,
                                           &rfds, &wfds, &efds));
      rc += pfd[i].revents != 0;
    }

  return rc;
}
