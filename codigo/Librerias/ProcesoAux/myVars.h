#ifndef myVars_h
#define myVars_h
///////////////////////////////////////////////////////////////////////
int LoadVars(const char * filename);
int GetNumSections();
char* FindSection(int nSection, int& nIndexFirstVar, int& nIndexLastVar);
int GetVarInt(char * var);
char * GetVarStr(char * var);
double GetVarDouble(char * var);
void GetVarListOfInts(int * loi, int n, char * var);
void GetVarListOfDoubles(double * lod, int n, char * var);

int GetVarIntSection(char * var, char* section);
char * GetVarStrSection(char * var, char* section);
double GetVarDoubleSection(char * var, char* section);
void GetVarListOfIntsSection(int * loi, int n, char * var, char* section);
void GetVarListOfDoublesSection(double * lod, int n, char * var, char* section);

int GetVarIntSection(char * var, int nSection);
char * GetVarStrSection(char * var, int nSection);
double GetVarDoubleSection(char * var, int nSection);
void GetVarListOfIntsSection(int * loi, int n, char * var, int nSection);
void GetVarListOfDoublesSection(double * lod, int n, char * var, int nSection);
///////////////////////////////////////////////////////////////////////
#define LOADINT(v) (v = GetVarInt(#v))
#define LOADDBL(v) (v = GetVarDouble(#v))
#define LOADSTR(v) strcpy(v,GetVarStr(#v))
#define LOADSTR2(v,name) strcpy(v,GetVarStr(name))
#define LOADVINT(v,n) GetVarListOfInts(v, n, #v)
#define LOADVDBL(v,n) GetVarListOfDoubles(v, n, #v)

#define LOADINTSECTION(v,s) (v = GetVarIntSection(#v,s))
#define LOADDBLSECTION(v,s) (v = GetVarDoubleSection(#v,s))
#define LOADSTRSECTION(v,s) strcpy(v,GetVarStrSection(#v,s))
#define LOADSTR2SECTION(v,name,s) strcpy(v,GetVarStrSection(name,s))
#define LOADVINTSECTION(v,n,s) GetVarListOfIntsSection(v, n, #v,s)
#define LOADVDBLSECTION(v,n,s) GetVarListOfDoublesSection(v, n, #v,s)
// LOADVDBL2SECTION - version que carga una lista de doubles especificando explicitamente el nombre de la lista (en vez de usar el nombre de la variable )
#define LOADVDBL2SECTION(v,name,n,s) GetVarListOfDoublesSection(v, n, name,s)

///////////////////////////////////////////////////////////////////////
#endif

