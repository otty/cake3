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
// aas.cpp -- legacy area awareness system
//

#include "q_shared.h"
#include "aas_public.h"
#include "aas.h"
#include "nav.h"

void (*AAS_Trace)(trace_t * result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask);
void (*AAS_Cvar_VariableStringBuffer)(const char *var_name, char *buffer, int bufsize);
int (*AAS_FS_FOpenFile)(const char *qpath, fileHandle_t * f, fsMode_t mode);
void (*AAS_FS_Read)(void *buffer, int len, fileHandle_t f);
void (*AAS_FS_FCloseFile)(fileHandle_t f);
void (*AAS_FS_Write)(const void *buffer, int len, fileHandle_t f);

aasNode_t aasNodes[MAX_AAS_NODES];
int 	num_aasNodes;

unsigned short aasLinearDist[MAX_AAS_NODES][MAX_AAS_NODES];

/*
==================
AAS_LinkNodes
==================
*/

void AAS_LinkNodes( void )
{
	trace_t         trace;
	vec3_t dir;
		
	Com_Printf("	creating links...\n");
	
	/* create links between nodes */
	for ( int i = 0; i < num_aasNodes; i++)
	{	
		aasNode_t *node;
		node = &aasNodes[i];	
		
		memset(&node->links, 0, sizeof(aasLink_t)*MAX_AAS_LINKS_PER_NODE);
		node->num_links = 0;
		
		for ( int n = 0; n < num_aasNodes; n++)
		{	
			aasNode_t *next;
			next = &aasNodes[n];
			
			if(i == n)
				continue;
				
			if(node->num_links >= MAX_AAS_LINKS_PER_NODE){
				Com_Printf("Warning: node %i exeeds link limit(%i).\n",i, MAX_AAS_LINKS_PER_NODE);
				break;
			}
			aasLink_t *link;
			link = &node->links[node->num_links];			
			
			// check for solid - damn, that is expansive!
			AAS_Trace(&trace, node->origin, NULL, NULL, next->origin, -1, CONTENTS_SOLID | CONTENTS_MONSTERCLIP);
			if(trace.startsolid)				
				continue;
			if(trace.allsolid)
				continue;
			if(trace.fraction < 1.0f)
				continue;
			
			//connect outer edges
			if(node->type == OUTER && next->type == OUTER){
				// check if n is on an edge, which shares one of i's edge points				
				navEdge_t *e1, *e2;
				e1 = &navEdges[node->edge];
				e2 = &navEdges[next->edge];
				unsigned short num = 0;
				
				for(int k = 0; k < 2; k++){
					for(int m = 0; m < 2; m++){				
						if(VectorCompare(e1->pos[k], e2->pos[m] ))
							num++;
					}
				}				
				if( num == 1) // link found!	
				{					
					// already connected?
					qboolean skip = qfalse;
					for(int k = 0; k < node->num_links; k++){
						if(node->links[k].next == n){
							skip = qtrue;
							break;
						}
					}
					if(skip)
						continue;
						
					VectorSubtract(node->origin, next->origin, dir);
					link->dist = VectorLength(dir);
					link->next = n;
					node->num_links++;				
				}
			}else{
			
				VectorSubtract(node->origin, next->origin, dir);
				float len = VectorLength(dir);
				
				if(len > 96.0f) // cfg.cs * cfg.ch is pretty dense, 64.0f causes leaks, 96.0f is good.
					continue;
					
				// already connected?
				qboolean skip = qfalse;
				for(int k = 0; k < node->num_links; k++){
					if(node->links[k].next == n){
						skip = qtrue;
						break;
					}
				}
				if(skip)
					continue;
						
				link->dist = len;
				link->next = n;
				node->num_links++;				
			}
		}
	}
}

/*
==================
AAS_NodeInit
==================
*/

void AAS_NodeInit( void )
{
	trace_t         trace;
	vec3_t start, end, dir;
	float len;
	
	Com_Printf("	creating nodes... ");
	
	/* create nodes from outer edges */		
	for ( int i = 0; i < num_navEdges; i++)
	{
		aasNode_t *node;
		node = &aasNodes[num_aasNodes];				
			
		if( num_aasNodes >= MAX_AAS_NODES){
			Com_Printf("MAX_AAS_NODES(%i) exeeded!\n", MAX_AAS_NODES);
			return;
		}
		navEdge_t *edge;
		edge = &navEdges[i];	
	
		if(edge->type != 0)
			continue;
			
		node->edge = i;
		node->type = OUTER;
		
		VectorCopy(edge->pos[0], start);			
		start[2] += DEFAULT_VIEWHEIGHT; // increase a little bit
		VectorCopy(edge->pos[1], end);
		end[2] += DEFAULT_VIEWHEIGHT; // increase a little bit
		
		// check for solid
		AAS_Trace(&trace, start, NULL, NULL, end, -1, CONTENTS_SOLID | CONTENTS_MONSTERCLIP);
		if(trace.startsolid)				
			continue;
		if(trace.allsolid)
			continue;			
		
		// calculate center point
		VectorSubtract(end, start, dir);
		len = VectorLength(dir);
		VectorNormalize(dir);
		VectorMA(start, len*0.5f, dir, node->origin);
		node->num = num_aasNodes;		
		num_aasNodes++;		
	}
		
	/* create nodes from grid */	
	for ( int i = 0; i < num_navRegulars; i++)
	{		
		aasNode_t *node;
		node = &aasNodes[num_aasNodes];				
			
		if( num_aasNodes >= MAX_AAS_NODES){
			Com_Printf("MAX_AAS_NODES(%i) exeeded!\n", MAX_AAS_NODES);
			return;
		}
		
		navRegular_t *regular;
		regular = &navRegulars[i];	
		
		node->type = INNER;

		vec3_t test;
		VectorCopy(regular->pos, node->origin);
		VectorCopy(regular->pos, test);
		test[2] -= 999;
		
		// check if stuck in solid
		AAS_Trace(&trace, node->origin,  NULL, NULL, test, -1, CONTENTS_SOLID | CONTENTS_MONSTERCLIP);
		if(trace.startsolid)
			continue;
		if(trace.allsolid)
			continue;
			
		node->num = num_aasNodes;		
		num_aasNodes++;			
	}	
	 
	Com_Printf("(%i)\n ", num_aasNodes); 
		
}

/*
==================
AAS_NodeLinearDist
==================
*/

void AAS_NodeLinearDist( void )
{
	for ( int i = 0; i < num_aasNodes; i++){
		for ( int n = 0; n < num_aasNodes; n++){
			vec3_t dir;
			int dist;
			if(i == n){
				aasLinearDist[i][n] = 0;
				continue;
			}
			VectorSubtract(aasNodes[n].origin, aasNodes[i].origin, dir);
			dist = (int) VectorLength(dir);
			aasLinearDist[i][n] = dist;			
		}		
	}
}

/*
==================
AAS_NodeOpenness
==================
*/

void AAS_NodeOpenness( void )
{
	trace_t         trace;
	
	/* Calculate how "open" a node is. AI can use that value to alter heuristics */
	
	for ( int i = 0; i < num_aasNodes; i++)
	{
		aasNode_t *node;
		node = &aasNodes[i];		
		
		int numVis;
		numVis = 0;
		for ( int n = 0; n < num_aasNodes; n++)
		{
			aasNode_t *other;	
			other = &aasNodes[n];			
			if(i==n)
				continue;
				
			AAS_Trace(&trace, node->origin, NULL, NULL, other->origin, -1, CONTENTS_SOLID | CONTENTS_MONSTERCLIP);
				
			if(trace.entityNum > ENTITYNUM_WORLD)		
				numVis++;
								
		}
		
		node->openess = (int)( 100.0f / num_aasNodes * numVis );
		//Com_Printf("Node %i has %i visible others (openness= %i percent)  \n", i, numVis, node->openess);
		
	}
}


/*
==================
AAS_AreaLinks
==================
*/

void AAS_AreaLinks( void )
{
	/* Build additional links for
	 * 	- jumping
	 * 	- climbing
	 *  - falling
	 * 	- teleporters
	*/
	
	// TODO...
}

/*
==================
AAS_LoadData

Load precompiled AAS data
==================
*/

qboolean AAS_LoadData( void )
{
	fileHandle_t    file;
	char            filename[MAX_QPATH];
	char            mapname[MAX_QPATH];
	int 			version;
	
	AAS_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
	Com_sprintf(filename, sizeof(filename), "maps/%s.aas", mapname);

	AAS_FS_FOpenFile(filename, &file, FS_READ);
	
	if(!file)
		return qfalse;
		
	// read version
	AAS_FS_Read(&version, sizeof(int), file);	

	if(version == NAVMESH_MAGIC)
	{
		Com_Printf("    loading AAS from file: '%s'...\n", filename);		
		
		AAS_FS_Read(&num_aasNodes, sizeof(int), file);
		AAS_FS_Read(&aasNodes, sizeof(aasNode_t)*num_aasNodes, file);
		
		AAS_FS_Read(&num_aasNodes, sizeof(int), file);
		AAS_FS_Read(&aasNodes, sizeof(aasNode_t)*num_aasNodes, file);
		
		for(int i = 0; i < num_aasNodes; i++)
			for(int j = 0; j < num_aasNodes; j++)
				AAS_FS_Read(&aasLinearDist[i][j], sizeof(unsigned short), file);
				
		AAS_FS_Read(&aasLinearDist, sizeof(unsigned short)*num_aasNodes*num_aasNodes, file);
		
		Com_Printf("    loaded %i nodes\n", num_aasNodes);


		AAS_FS_FCloseFile(file);
		return qtrue;
	}
	else
	{
		Com_Printf("    '%s' has wrong version %i!\n", filename, version);
	}
	
	AAS_FS_FCloseFile(file);

	return qfalse;
}

/*
==================
AAS_SaveData

Save AAS data for later use
==================
*/

void AAS_SaveData( void )
{
	fileHandle_t    file;
	char            filename[MAX_QPATH];
	char            mapname[MAX_QPATH];
	int 			version;
	
	AAS_Cvar_VariableStringBuffer("mapname", mapname, sizeof(mapname));
	Com_sprintf(filename, sizeof(filename), "maps/%s.aas", mapname);
	
	AAS_FS_FOpenFile(filename, &file, FS_WRITE);

	Com_Printf("    saving AAS to file: '%s'...\n", filename);
		
	version = NAVMESH_MAGIC;
	
	AAS_FS_Write(&version, sizeof(int), file);
	
    AAS_FS_Write(&num_aasNodes, sizeof(int), file);
	AAS_FS_Write(&aasNodes, sizeof(aasNode_t)*num_aasNodes, file);
	
    AAS_FS_Write(&num_aasNodes, sizeof(int), file);
	AAS_FS_Write(&aasNodes, sizeof(aasNode_t)*num_aasNodes, file);
	
	for(int i = 0; i < num_aasNodes; i++)
		for(int j = 0; j < num_aasNodes; j++)
			AAS_FS_Write(&aasLinearDist[i][j], sizeof(unsigned short), file);

	AAS_FS_FCloseFile(file);
}

/*
==================
AAS_Init

Load the AAS
==================
*/

void AAS_Init( char source[ MAX_QPATH ] )
{	
	
	memset(&navEdges, 0, sizeof(navEdge_t)*MAX_NAV_EDGES);
	num_navEdges = 0;

	memset(&navTriangles, 0, sizeof(navTriangle_t)*MAX_NAV_TRIANGLES);
	num_navTriangles = 0;

	memset(&navRegulars, 0, sizeof(navRegular_t)*MAX_NAV_REGULARS);
	num_navRegulars = 0;

	memset(&aasNodes, 0, sizeof(aasNode_t)*MAX_AAS_NODES);
	num_aasNodes = 0;

	memset(&aasLinearDist, 0, sizeof(unsigned short)*MAX_AAS_NODES*MAX_AAS_NODES);

	/* load the raw navmesh data from file */
	//AAS_NavMeshInit();

	/* check for precompiled data */
	//if(AAS_LoadData())
	//		return;

	/*if(build){
		Com_Printf("    building AAS...\n");

		/* convert the raw navmesh data into useable nodes */
		//AAS_NodeInit();

		/* create and evaluate links between nodes */
		//AAS_LinkNodes();

		/* build area links */
		//AAS_AreaLinks();

		/* build linear distance table */
		//AAS_NodeLinearDist();

		/* calculate openness */
		//AAS_NodeOpenness();

		/* create cover plates */
		//AAS_NodeCover();

		/* save precomputed data */
		//AAS_SaveData();
	//}

}


