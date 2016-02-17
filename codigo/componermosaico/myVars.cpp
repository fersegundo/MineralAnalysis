///////////////////////////////////////////////////////////////////////
// Actualizado 6/2/2008 Fernando Segundo para que admita secciones "[SECCION]"

#include "stdafx.h"
///////////////////////////////////////////////////////////////////////
#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <stdio.h>
#include <stdlib.h>
///////////////////////////////////////////////////////////////////////
#include "myVars.h"
#include "myUtilIO.h"
///////////////////////////////////////////////////////////////////////
#define MAXSECTIONS 2000
#define MAXVARS 20000
#define MAXNAME 100

char varV[MAXVARS][MAXRES];
char valV[MAXVARS][MAXRES];
char varSections[MAXSECTIONS][MAXRES];       //name of the the sections
int  indexSections[MAXSECTIONS];              // start index (in varV and valV) of the sections
int  nvars = 0;
int  nsections = 0;
///////////////////////////////////////////////////////////////////////
int LoadVars(const char * filename) {
  FILE * f;
  char * v;
  int p, eof;
  char * line;
  nvars = 0;
  nsections = 0;
  f = fopen(filename, "rt");
  if ( f == NULL ) return 0;
  while ( 1 ) {
	if ( nvars > MAXVARS ) break;
    line = ReadLine(f,eof);
	if ( eof ) break;
	if ( line[0]=='#' ) continue;
	if ( line[0]== 0  ) continue;
	if ( line[0]== '[') 
    {
        // new section found
        if (nsections > MAXSECTIONS) break;
        //get name of section
        p = 0;
	    v = GetField(line+1 /*skipping '['*/,']',p);
	    v = LTrim(v);
	    v = RTrim(v);
        strcpy(varSections[nsections],v);
        //store index
        indexSections[nsections] = nvars; //store the index of the first variable of this secction
        nsections++;
        continue; //fin de procesamiento de esta linea
    }
	p = 0;
	v = GetField(line,'=',p);
	v = LTrim(v);
	v = RTrim(v);
	strcpy(varV[nvars],v);
	v = line+p;
	v = LTrim(v);
	v = RTrim(v);
	strcpy(valV[nvars],v);
	nvars++;
  }
  fclose(f);
  return 1;
}

// Gets the number of read sections
int GetNumSections()
{
    return nsections;
}

// Find given section by index
//Returns index of first and last variables of the section
// Returns NULL if not found, else name of section
//INTERFACE
char* FindSection(int nSection, int& nIndexFirstVar, int& nIndexLastVar)
{
  if (nSection >= nsections) return NULL; // section not found

  nIndexFirstVar = indexSections[nSection];

  if (nSection+1 == nsections)
      nIndexLastVar = nvars; //there is no next section, so last var of this section will be last var
  else
      nIndexLastVar = indexSections[nSection+1]; //last var of this section is beginning of next section

    return varSections[nSection];
}

// Find given section by name
//Returns index of first and last variables of the section
// Returns false if not found
//AUXILIARY
bool FindSection(char* section, int& nIndexFirstVar, int& nIndexLastVar)
{
  int j;
  for (j=0; j<nsections; j++) {
	if ( strcmp(section, varSections[j]) == 0 ) break; // section found: j
  }

  return (FindSection(j,nIndexFirstVar,nIndexLastVar) != NULL);
}

///////////////////////////////////////////////////////////////////////
//Find first string of name "var" in the whole file
char * GetVarStr(char * var) {
  char * vac = "";
  int i;
  for (i=0; i<nvars; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return (char*)valV[i];
  }
  return vac;
}
//Find first string of name "var" within the given section
char * GetVarStrSection(char * var, char* section) {
  char * vac = "";

  //find section
  int nIndexFirstVar = 0;
  int nIndexLastVar = 0;
  if (FindSection(section,nIndexFirstVar, nIndexLastVar) == false)
      return vac;

  //lookup var in this section
  int i;
  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return (char*)valV[i];
  }

  return vac; //var not found
}
//Find first string of name "var" within the given section
char * GetVarStrSection(char* var, int nSection) {
  char * vac = "";

  //find section
  int nIndexFirstVar = 0;
  int nIndexLastVar = 0;
  if (FindSection(nSection,nIndexFirstVar, nIndexLastVar) == NULL)
      return vac;

  //lookup var in this section
  int i;
  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return (char*)valV[i];
  }

  return vac; //var not found
}


///////////////////////////////////////////////////////////////////////
// If not found returns -1
int GetVarInt(char * var) {
  int i;
  for (i=0; i<nvars; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return atoi(valV[i]);
  }
  return -1;
}

///////////////////////////////////////////////////////////////////////
// If not found returns -1
int GetVarIntSection(char * var, char* section) {
  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  if (FindSection(section,nIndexFirstVar, nIndexLastVar) == false)
      return -1;

  int i;
  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return atoi(valV[i]);
  }
  return -1;
}
///////////////////////////////////////////////////////////////////////
// If not found returns -1
int GetVarIntSection(char * var, int nSection) {
  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  if (FindSection(nSection,nIndexFirstVar, nIndexLastVar) == NULL)
      return -1;

  int i;
  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return atoi(valV[i]);
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////
// If not found returns -1
double GetVarDouble(char * var) {
  int i;
  for (i=0; i<nvars; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return atof(valV[i]);
  }
  return -1;
}
///////////////////////////////////////////////////////////////////////
// If not found returns -1
double GetVarDoubleSection(char * var, char* section) {
  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  if (FindSection(section,nIndexFirstVar, nIndexLastVar) == false)
      return -1;
  int i;
  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return atof(valV[i]);
  }
  return -1;
}
///////////////////////////////////////////////////////////////////////
// If not found returns -1
double GetVarDoubleSection(char * var, int nSection) {
  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  if (FindSection(nSection,nIndexFirstVar, nIndexLastVar) == false)
      return -1;
  int i;
  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) return atof(valV[i]);
  }
  return -1;
}


///////////////////////////////////////////////////////////////////////
void GetVarListOfInts(int * loi, int n, char * var) {
  int i, x, p;
  char * lin;
  char * field;
  for ( x=0; x<n; x++ ) loi[x] = 0; //inicializacion

  for (i=0; i<nvars; i++) {
	if ( strcmp(var, varV[i]) == 0 ) {
      lin = valV[i];
	  for (x=0; x<n; x++) {
        lin = LTrim(lin);
	    p = 0;
	    field = GetField(lin," \t",p);
		lin += p;
		loi[x] = atoi(field);
	  }
	  return;
	}
  }
}
///////////////////////////////////////////////////////////////////////
void GetVarListOfIntsSection(int * loi, int n, char * var, char* section) {
  int i, x, p;
  char * lin;
  char * field;
  for ( x=0; x<n; x++ ) loi[x] = 0; //initialization

  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  FindSection(section,nIndexFirstVar, nIndexLastVar);

  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) {
      lin = valV[i];
	  for (x=0; x<n; x++) {
        lin = LTrim(lin);
	    p = 0;
	    field = GetField(lin," \t",p);
		lin += p;
		loi[x] = atoi(field);
	  }
	  return;
	}
  }
}
///////////////////////////////////////////////////////////////////////
void GetVarListOfIntsSection(int * loi, int n, char * var, int nSection) {
  int i, x, p;
  char * lin;
  char * field;
  for ( x=0; x<n; x++ ) loi[x] = 0; //initialization

  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  FindSection(nSection,nIndexFirstVar, nIndexLastVar);

  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) {
      lin = valV[i];
	  for (x=0; x<n; x++) {
        lin = LTrim(lin);
	    p = 0;
	    field = GetField(lin," \t",p);
		lin += p;
		loi[x] = atoi(field);
	  }
	  return;
	}
  }
}


///////////////////////////////////////////////////////////////////////
void GetVarListOfDoubles(double * lod, int n, char * var) {
  int i, x, p;
  char * lin;
  char * field;
  for ( x=0; x<n; x++ ) lod[x] = 0; //initilization

  for (i=0; i<nvars; i++) {
	if ( strcmp(var, varV[i]) == 0 ) {
      lin = valV[i];
	  for (x=0; x<n; x++) {
        lin = LTrim(lin);
	    p = 0;
	    field = GetField(lin," \t",p);
		lin += p;
		lod[x] = atof(field);
	  }
	  return;
	}
  }
}
///////////////////////////////////////////////////////////////////////
void GetVarListOfDoublesSection(double * lod, int n, char * var, char* section) {
  int i, x, p;
  char * lin;
  char * field;
  for ( x=0; x<n; x++ ) lod[x] = 0; //initilization

  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  FindSection(section,nIndexFirstVar, nIndexLastVar);

  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) {
      lin = valV[i];
	  for (x=0; x<n; x++) {
        lin = LTrim(lin);
	    p = 0;
	    field = GetField(lin," \t",p);
		lin += p;
		lod[x] = atof(field);
	  }
	  return;
	}
  }
}
///////////////////////////////////////////////////////////////////////
void GetVarListOfDoublesSection(double * lod, int n, char * var, int nSection) {
  int i, x, p;
  char * lin;
  char * field;
  for ( x=0; x<n; x++ ) lod[x] = 0; //initilization

  //find section
  int nIndexFirstVar;
  int nIndexLastVar;
  FindSection(nSection,nIndexFirstVar, nIndexLastVar);

  for (i=nIndexFirstVar; i<nIndexLastVar; i++) {
	if ( strcmp(var, varV[i]) == 0 ) {
      lin = valV[i];
	  for (x=0; x<n; x++) {
        lin = LTrim(lin);
	    p = 0;
	    field = GetField(lin," \t",p);
		lin += p;
		lod[x] = atof(field);
	  }
	  return;
	}
  }
}
///////////////////////////////////////////////////////////////////////

