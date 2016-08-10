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

#include "navc.h"
#include "aas.h"

/*
aasNode_t aasNodes[MAX_AAS_NODES];
int 	num_aasNodes;

unsigned short aasLinearDist[MAX_AAS_NODES][MAX_AAS_NODES];

void AAS_Main( char source[ MAX_QPATH ] )
{

    Sys_Printf("==== AAS Init==== \n");


    //AAS_Trace = &trap_Trace;
    //AAS_Cvar_VariableStringBuffer = &trap_Cvar_VariableStringBuffer;
    //AAS_FS_FOpenFile = &trap_FS_FOpenFile;
    //AAS_FS_FCloseFile = &trap_FS_FCloseFile;
    //AAS_FS_Read = &trap_FS_Read;
    //AAS_FS_Write = &trap_FS_Write;

    memset(&navEdges, 0, sizeof(navEdge_t)*MAX_NAV_EDGES);
    num_navEdges = 0;

    memset(&navTriangles, 0, sizeof(navTriangle_t)*MAX_NAV_TRIANGLES);
    num_navTriangles = 0;

    memset(&navRegulars, 0, sizeof(navRegular_t)*MAX_NAV_REGULARS);
    num_navRegulars = 0;

    memset(&aasNodes, 0, sizeof(aasNode_t)*MAX_AAS_NODES);
    num_aasNodes = 0;

    memset(&aasLinearDist, 0, sizeof(unsigned short)*MAX_AAS_NODES*MAX_AAS_NODES);

}
*/