/*
===========================================================================
Copyright (C) 2011-2016 Adrian Fuhrmann <aliasng@gmail.com>

This file is part of XreaL source code.

XreaL source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

XreaL source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with XreaL source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/

#ifndef __NAV
#define __NAV
#define    NAVMESH_MAGIC    0x1337

/* should be the same as in rest of engine */
#ifndef STEPSIZE
#define    STEPSIZE        18.f
#endif
#ifndef DEFAULT_VIEWHEIGHT
#define    DEFAULT_VIEWHEIGHT        32
#endif
#ifndef MINWIDTH
#define    MINWIDTH        36
#endif
#ifndef INVALID
#define INVALID -1
#endif

#define OUTER 0
#define INNER 1
/* recast to quake transitions */
#define recast2quake(a, b)((b)[0]=-(a)[0], (b)[1]=-(a)[2], (b)[2]=(a)[1])
//#define quake2recast(a,b)((b)[0]=-(a)[0], (b)[1]=(a)[2], (b)[2]=-(a)[1])

enum RecastPolyAreas {
    RECAST_POLYAREA_GROUND
};
enum RecastPolyFlags {
    RECAST_POLYFLAGS_WALK = 0x01
};

#define MAX_NAV_EDGES 8192
typedef struct {
    vec3_t pos[2];
    short unsigned type;
} navEdge_t;
extern navEdge_t navEdges[MAX_NAV_EDGES];
extern int num_navEdges;

#define MAX_NAV_TRIANGLES 8192
typedef struct {
    vec3_t pos[3];
    short unsigned area;
} navTriangle_t;
extern navTriangle_t navTriangles[MAX_NAV_TRIANGLES];
extern int num_navTriangles;

#define MAX_NAV_REGULARS 8192
typedef struct {
    vec3_t pos;
    short unsigned type;
    short unsigned area;
} navRegular_t;
extern navRegular_t navRegulars[MAX_NAV_REGULARS];
extern int num_navRegulars;

#endif