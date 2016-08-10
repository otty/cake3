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

navEdge_t navEdges[MAX_NAV_EDGES];
int num_navEdges;

navTriangle_t navTriangles[MAX_NAV_TRIANGLES];
int num_navTriangles;

navRegular_t navRegulars[MAX_NAV_REGULARS];
int num_navRegulars;

/*
================
Nav_MeshLoadFile

read from disk file
================
*/

static int Nav_MeshLoadFile(char filename[MAX_QPATH]) {
    FILE *file;
    int i, j;
    int version;
    int size;

    Sys_Printf("    loading navigation mesh file '%s'...\n", filename);

    if ((file = fopen(filename, "rb")) == 0) {
        Sys_Printf("    could not read navigation mesh file '%s'!\n", filename);
        return 0;
    }
    fseek(file, 0, SEEK_END);
    Sys_Printf("    %i bytes loaded...\n", ftell(file));
    fseek(file, 0, 0);

    Sys_Printf("    analysing mesh...\n", ftell(file));
    // read version
    fread(&version, sizeof(int), 1, file);

    if (version == NAVMESH_MAGIC) {
        Sys_Printf("        loading navmesh version %i\n", version);

        fread(&num_navEdges, sizeof(int), 1, file);
        fread(&navEdges, sizeof(navEdge_t) * num_navEdges, 1, file);
        Sys_Printf("        loaded %i edges\n", num_navEdges);

        fread(&num_navTriangles, sizeof(int), 1, file);
        fread(&navTriangles, sizeof(navTriangle_t) * num_navTriangles, 1, file);
        Sys_Printf("        loaded %i triangles\n", num_navTriangles);

        fread(&num_navRegulars, sizeof(int), 1, file);
        fread(&navRegulars, sizeof(navRegular_t) * num_navRegulars, 1, file);
        Sys_Printf("        loaded %i grid points\n", num_navRegulars);

    } else {
        Sys_Printf("        '%s' has wrong version %i!\n", filename, version);
        return 0;
    }

    // everything ok
    return 1;

}


/*
================
main func; yippie!
================
*/

int main(int argc, char **argv) {
    int i, comp = 0;
    double start_time;
    char source[MAX_QPATH];

    Sys_Printf("navc          - v" NAVC_VERSION " @ 2016 Adrian 'otty' Fuhrmann.\n");
    Sys_Printf("----------------------------------------------------------\n");

    /* handle log file */
    //Log_Open("navc.log");		//open a log file
    //Log_Print("NAVC version "NAVC_VERSION", %s %s\n", __DATE__, __TIME__);

    start_time = I_FloatTime();

    /* read general options first */
    for (i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "-example1")) {
            //Log_Print("1\n");
        } else if (!strcmp(argv[i], "-example2")) {
            //Log_Print("2\n");
        } else {
            //Log_Print("unknown parameter %s\n", argv[i]);
            break;
        } //end else
    } //end for


    if (i != (argc - 1))
        Error("usage: navc [options] navfile");

    /* copy source name */
    strcpy(source, ExpandArg(argv[i]));

    /* init main aas routine */
    if (Nav_MeshLoadFile(source) != 1)
        Error("usage: navc [options] navfile");

    //Log_Print("NAVC run time is %5.0f seconds\n", I_FloatTime() - start_time);
    //Log_Close();						//close the log file
    return 0;
} //end of the function main

