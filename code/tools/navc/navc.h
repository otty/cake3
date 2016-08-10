/*
===========================================================================
Copyright (C) 1999-2005 Id Software, Inc.

This file is part of Quake III Arena source code.

Quake III Arena source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Quake III Arena source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Foobar; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/


#if defined(WIN32) || defined(_WIN32)
#include <direct.h>
    #include <windows.h>
    #include <sys/types.h>
    #include <sys/stat.h>
    #include <io.h>
#else
#include <unistd.h>
#include <glob.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

#include <string.h>
#include <malloc.h>

#include "log.h"

#include "../../shared/q_shared.h"
#include "../../shared/nav.h"

#define NAVC_VERSION		"1.0"
