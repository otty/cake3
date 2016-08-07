
/*
==============================================================

RAW NAVMESH DATA BASED ON RECASTS NAVMESH GENERATION

==============================================================
*/
#ifndef __AAS
#define __AAS
#define	NAVMESH_MAGIC	0x1337

/* should be the same as in rest of engine */
#ifndef STEPSIZE
#define	STEPSIZE		18.f
#endif
#ifndef DEFAULT_VIEWHEIGHT
#define	DEFAULT_VIEWHEIGHT		32
#endif
#ifndef MINWIDTH
#define	MINWIDTH		36
#endif
#ifndef INVALID
#define INVALID -1
#endif

/* recast to quake transitions */
#define recast2quake(a,b)((b)[0]=-(a)[0], (b)[1]=-(a)[2], (b)[2]=(a)[1])
//#define quake2recast(a,b)((b)[0]=-(a)[0], (b)[1]=(a)[2], (b)[2]=-(a)[1])

enum RecastPolyAreas
{
	RECAST_POLYAREA_GROUND
};
enum RecastPolyFlags
{
	RECAST_POLYFLAGS_WALK		= 0x01
};

#define MAX_NAV_EDGES 8192
typedef struct
{
	vec3_t pos[2];
	short unsigned type;	
} navEdge_t;
extern navEdge_t navEdges[MAX_NAV_EDGES];
extern int 	num_navEdges;

#define MAX_NAV_TRIANGLES 8192
typedef struct
{
	vec3_t pos[3];
	short unsigned area;	
} navTriangle_t;
extern navTriangle_t navTriangles[MAX_NAV_TRIANGLES];
extern int 	num_navTriangles;

#define MAX_NAV_REGULARS 8192
typedef struct
{
	vec3_t pos;
	short unsigned type;
	short unsigned area;
} navRegular_t;
extern navRegular_t navRegulars[MAX_NAV_REGULARS];
extern int 	num_navRegulars;


	
	
void AAS_NavMeshInit( void );

/*
==============================================================

AREA AWARENESS SYSTEM DATA

==============================================================
*/
#define MAX_AAS_NODES 8192
#define MAX_AAS_LINKS_PER_NODE 32

#define OUTER 0
#define INNER 1

typedef struct  {
	//qboolean valid; // if link is suitable for navigation, set to true
	//short type;
	float dist; // linear distance, for A* heuristics
	short next; // next node ;)
} aasLink_t;

typedef struct  {
	vec3_t origin; // position in world.
	short num;
	short type; // outer or inner?
	
	short edge; // for outer edge connections
	
	aasLink_t links[MAX_AAS_LINKS_PER_NODE]; // links to closest neighbours
	int num_links;
	
	// for pathfinding
	short parent;	
	short F,G,H;
	
	// various path & behaiviour things
	short openess;
	
} aasNode_t;
extern aasNode_t aasNodes[MAX_AAS_NODES];
extern int 	num_aasNodes;

extern unsigned short aasLinearDist[MAX_AAS_NODES][MAX_AAS_NODES];


void AAS_Init( qboolean build  );
void AAS_NodeInit( void );
void AAS_LinkNodes( void );
void AAS_NodeLinearDist( void );
void AAS_AreaLinks( void );
void AAS_NodeOpenness( void );

/*
==============================================================

PATHFINDING TOOLS

==============================================================
*/

#define MAX_AAS_PATH 8192

#define UNKNOWN 0
#define OPEN 1
#define CLOSED 2


typedef struct 
{
	int nodes[MAX_AAS_PATH];
	int num_nodes;

} aasPath_t;

qboolean AAS_Visible( vec3_t a, vec3_t b );
aasNode_t *AAS_FindNearestNode( vec3_t origin );
qboolean AAS_Path( aasPath_t *path, aasNode_t *start, aasNode_t *end );

extern int nodeList[MAX_AAS_NODES];
	
// cgame debugging
extern aasNode_t *startNode;
extern aasNode_t *endNode;
extern aasPath_t testPath;



#endif