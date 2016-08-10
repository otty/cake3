

extern void (*AAS_Trace)(trace_t * result, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int skipNumber, int mask);
extern void (*AAS_Cvar_VariableStringBuffer)(const char *var_name, char *buffer, int bufsize);
extern int (*AAS_FS_FOpenFile)(const char *qpath, fileHandle_t * f, fsMode_t mode);
extern void (*AAS_FS_Read)(void *buffer, int len, fileHandle_t f);
extern void (*AAS_FS_FCloseFile)(fileHandle_t f);
extern void (*AAS_FS_Write)(const void *buffer, int len, fileHandle_t f);
