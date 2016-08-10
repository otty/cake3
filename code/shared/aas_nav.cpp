/*
===========================================================================
Copyright (C) 2011 Adrian Fuhrmann <aliasng@gmail.com>

This file is part of Legacy source code.

Legacy source code is free software; you can redistribute it
and/or modify it under the terms of the GNU General Public License as
published by the Free Software Foundation; either version 2 of the License,
or (at your option) any later version.

Legacy source code is distributed in the hope that it will be
useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Legacy source code; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
===========================================================================
*/
// aas_nav.cpp -- legacy navmesh tools
//

#include "q_shared.h"
#include "aas_public.h"
#include "aas.h"
#include "aas.h"

navEdge_t navEdges[MAX_NAV_EDGES];
int num_navEdges;

navTriangle_t navTriangles[MAX_NAV_TRIANGLES];
int num_navTriangles;

navRegular_t navRegulars[MAX_NAV_REGULARS];
int 	num_navRegulars;

/*
================
AAS_NavMeshLoadFile

read from disk file
================
*/

static void AAS_NavMeshLoadFile(void)
{
	fileHandle_t    file;
	char filename[MAX_QPATH];
	char            mapname[MAX_QPATH];
	int             i,j;
	int 			version;	

	AAS_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
	Com_sprintf(filename, sizeof(filename), "maps/%s.navMesh", mapname);
    Com_Printf("    loading navigation mesh file '%s'...\n", filename);

	AAS_FS_FOpenFile(filename, &file, FS_READ);
	if(!file)
	{
		Com_Printf("    no navigation mesh file '%s' found!\n", filename);
		return;
	}

	// read version
	AAS_FS_Read(&version, sizeof(int), file);	

	if(version == NAVMESH_MAGIC)
	{
		Com_Printf("    loading navmesh version %i\n", version);
		
		AAS_FS_Read(&num_navEdges, sizeof(int), file);		
		AAS_FS_Read(&navEdges, sizeof(navEdge_t)*num_navEdges, file);
		Com_Printf("    loaded %i edges\n", num_navEdges);
		
		AAS_FS_Read(&num_navTriangles, sizeof(int), file);		
		AAS_FS_Read(&navTriangles, sizeof(navTriangle_t)*num_navTriangles, file);	
		Com_Printf("    loaded %i triangles\n", num_navTriangles);
		
		AAS_FS_Read(&num_navRegulars, sizeof(int), file);		
		AAS_FS_Read(&navRegulars, sizeof(navRegular_t)*num_navRegulars, file);	
		Com_Printf("    loaded %i grid points\n", num_navRegulars);		

	}
	else
	{
		Com_Printf("    '%s' has wrong version %i!\n", filename, version);
	}

}

/*
==================
AAS_NavMeshInit
==================
*/

void AAS_NavMeshInit( void )
{
	
	AAS_NavMeshLoadFile();
}