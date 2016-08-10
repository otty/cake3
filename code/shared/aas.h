
/*
==============================================================

RAW NAVMESH DATA BASED ON RECASTS NAVMESH GENERATION


 TODO: clean this up!
==============================================================
*/
#ifndef __AAS
#define __AAS

void AAS_NavMeshInit( void );

/*
==============================================================

AREA AWARENESS SYSTEM DATA

==============================================================
*/
#define MAX_AAS_NODES 8192
#define MAX_AAS_LINKS_PER_NODE 32


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

/*
void AAS_Init( qboolean build  );
void AAS_NodeInit( void );
void AAS_LinkNodes( void );
void AAS_NodeLinearDist( void );
void AAS_AreaLinks( void );
void AAS_NodeOpenness( void );
*/

/*
==============================================================

PATHFINDING TOOLS

==============================================================
*/
/*
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
*/
#endif