//===- llvm/System/Unix/Unix.h - Common Unix Include File -------*- C++ -*-===//
// 
//                     The LLVM Compiler Infrastructure
//
// This file was developed by Reid Spencer and is distributed under the 
// University of Illinois Open Source License. See LICENSE.TXT for details.
// 
//===----------------------------------------------------------------------===//
//
// This file defines things specific to Unix implementations.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_SYSTEM_UNIX_UNIX_H
#define LLVM_SYSTEM_UNIX_UNIX_H

//===----------------------------------------------------------------------===//
//=== WARNING: Implementation here must contain only generic UNIX code that
//===          is guaranteed to work on all UNIX variants.
//===----------------------------------------------------------------------===//

#include "llvm/Config/config.h"     // Get autoconf configuration settings
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <cerrno>
#include <string>
#include <algorithm>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif 

#ifdef HAVE_ASSERT_H
#include <assert.h>
#endif

#ifdef TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# ifdef HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif

#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

inline void ThrowErrno(const std::string& prefix) {
    char buffer[MAXPATHLEN];
#ifdef HAVE_STRERROR_R
    // strerror_r is thread-safe.
    strerror_r(errno,buffer,MAXPATHLEN-1);
#elif HAVE_STRERROR
    // Copy the thread un-safe result of strerror into
    // the buffer as fast as possible to minimize impact
    // of collision of strerror in multiple threads.
    strncpy(buffer,strerror(errno),MAXPATHLEN-1);
    buffer[MAXPATHLEN-1] = 0;
#else
    // Strange that this system doesn't even have strerror
    // but, oh well, just use a generic message
    sprintf(buffer, "Error #%d", errno);
#endif
    throw prefix + ": " + buffer;
}

#endif
