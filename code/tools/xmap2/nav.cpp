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
//
// nav.cpp -- creates the basic information for area awareness system based on recast navigation mesh
//

extern "C"
{

#include "q3map2.h"
#include "../common/surfaceflags.h"

}

#include "Recast.h"
#include "../../shared/aas.h"

class rcContext context(false);

rcHeightfield *heightField;
unsigned char *navData;
int navDataSize;
vec3_t mapmins;
vec3_t mapmaxs;


unsigned char *triareas;
rcContourSet *contours;
rcCompactHeightfield *compHeightField;
rcPolyMesh *polyMesh;
rcPolyMeshDetail *detailedPolyMesh;

// Load indices
const int *indexes;
const bspDrawSurface_t *surfaces;
int numSurfaces;

// Load triangles
int *tris;
int numtris;

// Load vertices
const bspDrawVert_t *vertices;
float *verts;
int numverts;

// Load models
const bspModel_t *models;

// Recast config
rcConfig cfg;


navEdge_t navEdges[MAX_NAV_EDGES];
int num_navEdges;

navTriangle_t navTriangles[MAX_NAV_TRIANGLES];
int num_navTriangles;

navRegular_t navRegulars[MAX_NAV_REGULARS];
int 	num_navRegulars;

static void CreateNavigationData ( void )
{
	vec3_t o;
	vec3_t dir;
	float len;
	
	Sys_Printf("	converting recast data:\n");
	 
	Sys_Printf("		building edges... ");	
	memset(&navEdges, 0, sizeof(navEdge_t)*MAX_NAV_EDGES);
	num_navEdges = 0;
	for ( int i = 0; i < polyMesh->npolys; i++)
	{
		const unsigned short* p = &polyMesh->polys[i*polyMesh->nvp*2];
		for ( int j = 0; j < polyMesh->nvp; j++)
		{
			navEdge_t *edge;
			edge = &navEdges[num_navEdges];
			
			if (p[j] == RC_MESH_NULL_IDX) break;
			
			if( num_navEdges >= MAX_NAV_EDGES){
				Error("MAX_NAV_TRIANGLES(%i) exeeded!\n", MAX_NAV_EDGES);				
			}
			
			if (p[polyMesh->nvp+j] & 0x8000) {
				edge->type = 0;
			}else{				
				edge->type = 1;
				if ((p[polyMesh->nvp+j] & 0xf) != 0xf){
					edge->type = 2;					
				}					
			}
			
			const int nj = (j+1 >= polyMesh->nvp || p[j+1] == RC_MESH_NULL_IDX) ? 0 : j+1;
			unsigned short vi[2] = {p[j], p[nj]};			
			const unsigned short* v;
			
			for( int n = 0; n < 2; n++)
			{
				v = &polyMesh->verts[vi[n]*3];
				VectorSet(o, v[0]*cfg.cs, (v[1])*cfg.ch, v[2]*cfg.cs );
				recast2quake(o, edge->pos[n]);
				edge->pos[n][0] += -mapmins[0];
				edge->pos[n][1] += -mapmins[2];
				edge->pos[n][2] += mapmins[1];	
				edge->pos[n][2] -= (cfg.ch - 1.0f); // it always seams, the z is to height....
			}			
			// throw doubles away
			qboolean skip = qfalse;
			for ( int k = 0; k < num_navEdges; k++){
				if(VectorCompare(edge->pos[0], navEdges[k].pos[0]) && VectorCompare(edge->pos[1], navEdges[k].pos[1]))
				{
					skip = qtrue;
					break;
				}
				if(VectorCompare(edge->pos[1], navEdges[k].pos[0]) && VectorCompare(edge->pos[0], navEdges[k].pos[1]))
				{
					skip = qtrue;
					break;
				}
			}
			if(skip)
				continue;
			num_navEdges++;
		}
	}	 
	Sys_Printf("(%i)\n ", num_navEdges);	 
	 
	Sys_Printf("		building triangles... ");	
	memset(&navTriangles, 0, sizeof(navTriangle_t)*MAX_NAV_TRIANGLES);
	num_navTriangles = 0;

	for ( int i = 0; i < polyMesh->npolys; i++)
	{	
		const unsigned short* p = &polyMesh->polys[i*polyMesh->nvp*2];			
		unsigned short vi[3];		
			const unsigned short* v;
		for ( int j = 2; j < polyMesh->nvp; j++)
		{
			if (p[j] == RC_MESH_NULL_IDX) break;
			vi[0] = p[0];
			vi[1] = p[j-1];
			vi[2] = p[j];	
			
			if( num_navTriangles >= MAX_NAV_TRIANGLES){
				Error("MAX_NAV_TRIANGLES(%i) exeeded!\n", MAX_NAV_TRIANGLES);
			}

			navTriangle_t *triangle;
			triangle = &navTriangles[num_navTriangles];								
			
			triangle->area = polyMesh->areas[i];
			
			for( int n = 0; n < 3; n++)
			{
				v = &polyMesh->verts[vi[n]*3];
				VectorSet(o, v[0]*cfg.cs, (v[1])*cfg.ch, v[2]*cfg.cs );
				recast2quake(o, triangle->pos[n]);
				triangle->pos[n][0] += -mapmins[0];
				triangle->pos[n][1] += -mapmins[2];
				triangle->pos[n][2] += mapmins[1];	
				triangle->pos[n][2] += 1.0f - cfg.ch ;
			}			
			num_navTriangles++;			
		}
	} 
	
	Sys_Printf("(%i)\n ", num_navTriangles); 	
	
	Sys_Printf("		building grid... ");	
	memset(&navRegulars, 0, sizeof(navRegular_t)*MAX_NAV_REGULARS);
	num_navRegulars = 0;
	
	const float cs = compHeightField->cs;
	const float ch = compHeightField->ch;
	const short stepSize = cfg.cs;
	
	for (int y = 0; y < compHeightField->height; y+=stepSize)
	{
		for (int x = 0; x < compHeightField->width; x+=stepSize)
		{
			const float fx = compHeightField->bmin[0] + x*cs;
			const float fz = compHeightField->bmin[2] + y*cs;
			const rcCompactCell& c = compHeightField->cells[x+y*compHeightField->width];

			for (unsigned i = c.index, ni = c.index+c.count; i < ni; ++i)
			{
				const rcCompactSpan& s = compHeightField->spans[i];
			
				navRegular_t *regular;
				regular = &navRegulars[num_navRegulars];	
			
				if (compHeightField->areas[i] == RC_WALKABLE_AREA)
					regular->type = INNER;
				else
					continue;
				
				const float fy = compHeightField->bmin[1] + (s.y+1)*ch;
								
				VectorSet(o, fx, fy, fz );
				recast2quake(o, regular->pos);
				regular->pos[2] += 16.0f - cfg.ch ;
			
				num_navRegulars++;
			}
		}
	}

	Sys_Printf("(%i)\n ", num_navRegulars); 
	
	
}

static void WriteNavigationData ( void ){
    FILE                    *file;
    int                     version = NAVMESH_MAGIC;
    int             size;
	
    StripExtension(source);
	DefaultExtension(source, ".nav");
	Sys_Printf("Writing %s\n", source);
	
    file = fopen(source, "w");
    if(!file)
    {
        Error("Couldn't write navigation mesh : %s\n", source);

    }

    fwrite(&version, sizeof(int), 1, file);
	
    fwrite(&num_navEdges, sizeof(int), 1, file);
	fwrite(&navEdges,sizeof(navEdge_t)*num_navEdges, 1, file);
	
    fwrite(&num_navTriangles, sizeof(int), 1, file);
	fwrite(&navTriangles,sizeof(navTriangle_t)*num_navTriangles, 1, file);
	
    fwrite(&num_navRegulars, sizeof(int), 1, file);
	fwrite(&navRegulars,sizeof(navRegular_t)*num_navRegulars, 1, file);

    /* emit bsp size */
    size = ftell(file);
    Sys_Printf("Wrote %.1f MB (%d bytes)\n", (float)size / (1024 * 1024), size);

    fclose(file);
	
}

static void UpdatePolyFlags(void){

    Sys_Printf("    updating flags...\n");
	// Update poly flags from areas.
    for (int i = 0; i < polyMesh->npolys; ++i)
    {
        if (polyMesh->areas[i] == RC_WALKABLE_AREA){
            polyMesh->areas[i] = RECAST_POLYAREA_GROUND;
			polyMesh->flags[i] = RECAST_POLYFLAGS_WALK;
        }
        
    }

}

static void CreateDetailMesh ( void )
{
    Sys_Printf("    creating detail mesh...\n");

    detailedPolyMesh = rcAllocPolyMeshDetail();
    if ( !rcBuildPolyMeshDetail (&context, *polyMesh, *compHeightField, cfg.detailSampleDist, cfg.detailSampleMaxError, *detailedPolyMesh) )
    {
        Error ("Failed to create detail mesh for navigation mesh.\n");
    }
}

static void BuildMeshFromContours ( void )
{
    Sys_Printf("    building mesh from contours...\n");

    polyMesh = rcAllocPolyMesh();
    if ( !rcBuildPolyMesh (&context, *contours, cfg.maxVertsPerPoly, *polyMesh) )
    {
        Error ("Failed to triangulate contours.\n");
    }
}

static void CreateContours( void )
{
    Sys_Printf("    creating contours...\n");

    contours = rcAllocContourSet();
    if ( !rcBuildContours (&context, *compHeightField, cfg.maxSimplificationError, cfg.maxEdgeLen, *contours) )
    {
        Error ("Failed to create contour set for navigation mesh.\n");
    }
}

static void CreateRegions ( void )
{
    Sys_Printf("    creating regions...\n");

    compHeightField = rcAllocCompactHeightfield();
    if ( !rcBuildCompactHeightfield (&context, cfg.walkableHeight, cfg.walkableClimb, *heightField, *compHeightField) )
    {
        Error ("Failed to create compact heightfield for navigation mesh.\n");
    }

    if ( !rcErodeWalkableArea (&context, cfg.walkableRadius, *compHeightField) )
    {
        Error ("Unable to erode walkable surfaces.\n");
    }

    if ( !rcBuildDistanceField (&context, *compHeightField) )
    {
        Error ("Failed to build distance field for navigation mesh.\n");
    }

    if ( !rcBuildRegions (&context, *compHeightField, 0, cfg.minRegionArea, cfg.mergeRegionArea) )
    {
        Error ("Failed to build regions for navigation mesh.\n");
    }
}

static void FilterSurfaces ()
{
    Sys_Printf("    filtering surfaces...\n");

    rcFilterLowHangingWalkableObstacles (&context, cfg.walkableClimb, *heightField);
    rcFilterLedgeSpans (&context, cfg.walkableHeight, cfg.walkableClimb, *heightField);
    rcFilterWalkableLowHeightSpans (&context, cfg.walkableHeight, *heightField);
}

static void CreateHeightfield ( void )
{
    Sys_Printf("    creating heightfield...\n");

    heightField = rcAllocHeightfield();
    if ( !rcCreateHeightfield (&context, *heightField, cfg.width, cfg.height, cfg.bmin, cfg.bmax, cfg.cs, cfg.ch) )
    {
        Error ("Failed to create heightfield for navigation mesh.\n");
    }

    triareas = new unsigned char[numtris];
    memset (triareas, 0, numtris);
    rcMarkWalkableTriangles (&context, cfg.walkableSlopeAngle, verts, numverts, tris, numtris, triareas);
    rcRasterizeTriangles (&context, verts, numverts, tris, triareas, numtris, *heightField, cfg.walkableClimb);
    delete[] triareas;
    triareas = NULL;

}

static void ConfigureRecast ( void )
{
    Sys_Printf("    configuring recast options...\n");

    memset (&cfg, 0, sizeof (cfg));
    VectorCopy (mapmaxs, cfg.bmax);
    VectorCopy (mapmins, cfg.bmin);

    /* Some values are taken from Omnibot ET Implementation */

	cfg.cs = 8.f;
	cfg.ch = 16.f;
	cfg.walkableSlopeAngle = 45;
	cfg.maxEdgeLen = 64;
	cfg.maxSimplificationError = 1.f;
	cfg.maxVertsPerPoly = 6;
	cfg.borderSize = 0;
	cfg.detailSampleDist = 6.f;
	cfg.detailSampleMaxError = 8.f;

    cfg.walkableSlopeAngle = 45.0f; // MIN_WALK_NORMAL
    cfg.walkableHeight = 64 / cfg.ch;
    cfg.walkableClimb = STEPSIZE / cfg.ch;
    cfg.walkableRadius = 32 / cfg.cs;

    rcCalcGridSize (cfg.bmin, cfg.bmax, cfg.cs, &cfg.width, &cfg.height);

}

static int CountIndices ( const bspDrawSurface_t *surfaces, int numSurfaces )
{
    int count = 0;
    for ( int i = 0; i < numSurfaces; i++, surfaces++ )
    {
        if( bspShaders[surfaces->shaderNum].surfaceFlags & ( SURF_NODRAW | SURF_SKIP | SURF_SKY | SURF_SLICK | SURF_HINT | SURF_NONSOLID))
        {
            continue;
        }

        if ( surfaces->surfaceType != MST_PLANAR && surfaces->surfaceType != MST_TRIANGLE_SOUP )
        {
            continue;
        }

        count += surfaces->numIndexes;
    }

    return count;
}

static void LoadTriangles ( const int *indexes, int* tris, const bspDrawSurface_t *surfaces, int numSurfaces )
{
    int t = 0;
    int v = 0;
    for ( int i = 0; i < numSurfaces; i++, surfaces++ )
    {
        if( bspShaders[surfaces->shaderNum].surfaceFlags & ( SURF_NODRAW | SURF_SKIP | SURF_SKY | SURF_SLICK | SURF_HINT | SURF_NONSOLID))
        {
            continue;
        }

        if ( surfaces->surfaceType != MST_PLANAR && surfaces->surfaceType != MST_TRIANGLE_SOUP )
        {
            continue;
        }

        for ( int j = surfaces->firstIndex, k = 0; k < surfaces->numIndexes; j++, k++ )
        {
            tris[t++] = v + indexes[j];
        }

        v += surfaces->numVerts;
					
    }
}

static void LoadVertices ( const bspDrawVert_t *vertices, float* verts, const bspDrawSurface_t *surfaces, const int numSurfaces )
{
    int v = 0;
    for ( int i = 0; i < numSurfaces; i++, surfaces++ )
    {

        if( bspShaders[surfaces->shaderNum].surfaceFlags & ( SURF_NODRAW | SURF_SKIP | SURF_SKY | SURF_SLICK | SURF_HINT | SURF_NONSOLID))
        {
            continue;
        }

        if ( surfaces->surfaceType != MST_PLANAR && surfaces->surfaceType != MST_TRIANGLE_SOUP )
        {
            continue;
        }

        for ( int j = surfaces->firstVert, k = 0; k < surfaces->numVerts; j++, k++ )
        {
            verts[v++] = -vertices[j].xyz[0];
            verts[v++] = vertices[j].xyz[2];
            verts[v++] = -vertices[j].xyz[1];
        }
    }
}


static void LoadGeometry()
{

	Sys_Printf("    loading geometry...\n");

    // Load indices
    indexes = bspDrawIndexes;
    surfaces = bspDrawSurfaces;
    numSurfaces = numBSPDrawSurfaces;

    Sys_Printf("        using %i surfaces...\n", numBSPDrawSurfaces);
    Sys_Printf("        using %i indexes...\n", numBSPDrawIndexes);

    // load triangles
    numtris = CountIndices (surfaces, numSurfaces);
    tris = new int[numtris];
    numtris /= 3;

    LoadTriangles (indexes, tris, surfaces, numSurfaces);
    Sys_Printf("        using %i triangles...\n", numtris);

    // Load vertices
    vertices = bspDrawVerts;
    numverts = numBSPDrawVerts;

    verts = new float[3 * numverts];
    LoadVertices (vertices, verts, surfaces, numSurfaces);
    Sys_Printf("        using %i vertices...\n", numverts);

    // Get bounds
    models = bspModels;
    mapmins[0] = -models[0].maxs[0];
    mapmins[1] = models[0].mins[2];
    mapmins[2] = -models[0].maxs[1];

    mapmaxs[0] = -models[0].mins[0];
    mapmaxs[1] = models[0].maxs[2];
    mapmaxs[2] = -models[0].mins[1];

    Sys_Printf("        set world bounds to\n", numverts);
    Sys_Printf("                  min: %f %f %f\n", mapmins[0], mapmins[1], mapmins[2]);
    Sys_Printf("                  max: %f %f %f\n", mapmaxs[0], mapmaxs[1], mapmaxs[2]);

}

static void LoadRecast()
{
    Sys_Printf("    setting up recast...\n");

    VectorClear(mapmins);
    VectorClear(mapmaxs);

    verts = NULL;
    numverts = 0;
    tris = NULL;
    numtris = 0;
    triareas = NULL;
    contours = NULL;
    compHeightField = NULL;
    polyMesh = NULL;
    detailedPolyMesh = NULL;

}


/*
===========
NavMain
===========
*/
int NavMain(int argc, char **argv)
{
	int             i;

	/* note it */
	Sys_Printf("--- Nav ---\n");

	/* process arguments */
	for(i = 1; i < (argc - 1); i++)
	{
		/*if(!strcmp(argv[i], "-someNavCommand"))
		{
			Sys_Printf("someNavCommand given...\n");
		}
		else
		{*/
			Sys_Printf("WARNING: Unknown option \"%s\"\n", argv[i]);
		//}
	}

	//if(i != argc - 1)
	//	Error("usage: nav [-threads #] [-level 0-4] [-fast] [-v] bspfile");

	/* load the bsp */
	sprintf(source, "%s%s", inbase, ExpandArg(argv[i]));
	StripExtension(source);
	strcat(source, ".bsp");
	Sys_Printf("Loading %s\n", source);
	LoadBSPFile(source);

	/* set up recast */
    LoadRecast();

    /* get the data into recast */
    LoadGeometry();

    /* configure recast */
    ConfigureRecast();

    /* create recast height field */
    CreateHeightfield();

    /* filter walkable surfaces */
    FilterSurfaces ();

    /* partition walkable surface to simple regions */
    CreateRegions ();

    /* create contours */
    CreateContours();

    /* build polygons mesh from contours */
    BuildMeshFromContours();

    /* Create detail mesh */
    //CreateDetailMesh(); // we dont need the detail mesh.

    /* Update poly flags */
    UpdatePolyFlags();

	/* create navigation data from map info and recast nav mesh */
	CreateNavigationData ();
	
	/* write converted navigation data to disk */
    WriteNavigationData();
	
    /* clean up */
    Sys_Printf("    cleaning up recast...\n");

    rcFreeHeightField (heightField);
    rcFreeCompactHeightfield (compHeightField);
    rcFreeContourSet (contours);
    rcFreePolyMesh (polyMesh);
    rcFreePolyMeshDetail (detailedPolyMesh);

    delete[] verts;
    delete[] tris;

	return 0;
}
