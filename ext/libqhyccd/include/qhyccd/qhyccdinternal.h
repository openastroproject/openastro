#ifndef __QHYCCDINTERNAL_H__
#define __QHYCCDINTERNAL_H__

#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


#if (defined(__linux__ )&&!defined (__ANDROID__)) ||(defined (__APPLE__)&&defined( __MACH__))
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdbool.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif
	
#if defined (_WIN32)
#include <windows.h>
#include <process.h>
#include <time.h>
#endif


#endif

