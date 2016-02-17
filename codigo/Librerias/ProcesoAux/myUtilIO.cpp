///////////////////////////////////////////////////////////////////////
//	Autor D.A. modificado por B.Ll.M.
//  Modificado por F.S. en Mayo del 2009
///////////////////////////////////////////////////////////////////////
#include "stdafx.h"
#include "math.h" //para ceil
///////////////////////////////////////////////////////////////////////
#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <stdio.h>
#if !defined (__BORLANDC__)
#include <stdlib.h>
#include <conio.h>
#endif
///////////////////////////////////////////////////////////////////////
#include "myUtilIO.h"
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
#if !defined (__BORLANDC__)
int getKey()
{
	int  r, c;

	r = c = 0;
	if (kbhit()) {
		do {
			r = getch();
			c++;
		} while (r == 0 || r == 0xE0);
		if ( c > 1 )
			r += 256;
	}

	return r;
}
///////////////////////////////////////////////////////////////////////
void strcatchar(char * s, char c) {
  int l = strlen(s);
  s[l]=c;
  s[l+1]=0;
}
///////////////////////////////////////////////////////////////////////
char * d2a(double num) {
  static char res[MAXRES];
  sprintf(res,"%lf",num);
  int ll, p;
  if ( Pos('.',res) != -1 ) {
   ll = strlen(res);
   for ( p=ll-1; p>=0; p-- ) {
	 if ( res[p] == '.' ) { res[p]=(char)0; break; }
     if ( res[p] != '0' ) break;
     res[p]=(char)0;
   }
  }
  return res;
}
///////////////////////////////////////////////////////////////////////
char * d2a(double num, int dec) {
  static char res[MAXRES];
  sprintf(res,"%.*lf",dec,num);
  return res;
}
///////////////////////////////////////////////////////////////////////
char * i2a(int num) {
  static char res[MAXRES];
  sprintf(res,"%d",num);
  return res;
}
///////////////////////////////////////////////////////////////////////
int Letter(char c) {
  if ( 'a' <= c && c <= 'z' ) return 1;
  if ( 'A' <= c && c <= 'Z' ) return 1;
  return 0;
}
///////////////////////////////////////////////////////////////////////
char Upper(char c) {
  char r = c;
  if ( 'a' <= c && c <= 'z' ) r = (char) (c - 'a' + 'A');
  return r;
}
///////////////////////////////////////////////////////////////////////
void Upper(char * s) {
  int l = strlen(s);
  int i;
  for (i=0; i<l; i++) s[i] = Upper(s[i]);
}
#endif
///////////////////////////////////////////////////////////////////////
int Pos(char c, char * s) {
  int l = strlen(s);
  int i;
  for (i=0; i<l; i++) if ( s[i]==c ) return i;
  return -1;
}
///////////////////////////////////////////////////////////////////////
#if !defined (__BORLANDC__)
int Pos(char * s, char * ss) {
  int l = strlen(s);
  int ll = strlen(ss);
  int i;
  int max = ll-l+1;
  for (i=0; i<max; i++)	if ( strncmp(s,ss+i,l)==0 ) return i;
  return -1;
}
///////////////////////////////////////////////////////////////////////
// Devuelve el numero de veces que se encuentra el caracter 'c' en el string 's'
int Count(char c, char * s) {
  int l = strlen(s);
  int i;
  int vc = 0;
  for (i=0; i<l; i++) if ( s[i]==c )  vc++;
  return vc;
}
///////////////////////////////////////////////////////////////////////
void MovConsole(int s) {
  static int j = 0;
  static int i = 0;
  if ( s > j ) { j++; return; }
  j = 0;
  switch (i++) {
    case 0:  putchar('\r');  putchar('|');  break;
    case 1:  putchar('\r');  putchar('/');  break;
    case 2:  putchar('\r');  putchar('-');  break;
    case 3:  putchar('\r');  putchar('\\');  i = 0;
  }
}
///////////////////////////////////////////////////////////////////////
int FileExist(char * fn) {
  FILE * f = fopen(fn,"rt");
  if ( f == NULL ) return 0;
  fclose(f);
  return 1;
}
///////////////////////////////////////////////////////////////////////
char * ReadWhile(FILE * f, char * s, int & eof) {
  static char res[MAXRES];
  res[0] = 0;
  eof = 0;
  char c;
  int i = 0;
  while (1) {
    if ( feof(f) ) { eof = 1; break; }
    if ( i == MAXRES-1 ) break;
    c = (char) getc(f);
	if ( Pos(c,s) == -1 ) { ungetc(c,f); break; }
	res[i++] = c;
  }
  res[i]=0;
  return res;
}
///////////////////////////////////////////////////////////////////////
//lee y devuelve hasta que el caracter leido sea uno de los contenidos en s (UNO CUALQUIERA)
// eof - valdra 1 si se ha encontrado el fin de fichero
char * ReadUntil(FILE * f, char * s, int & eof) {
  static char res[MAXRES];
  res[0] = 0;
  eof = 0;
  char c;
  int i = 0;
  while (1) {
    if ( feof(f) ) { eof = 1; break; }
	if ( i == MAXRES-1 ) break;
    c = (char) getc(f);
	if ( Pos(c,s) != -1 ) { ungetc(c,f); break; }
	res[i++] = c;
  }
  res[i]=0;
  return res;
}
#endif
///////////////////////////////////////////////////////////////////////
char * ReadLine(FILE * f, int & eof) {
  static char res[MAXRES];
  res[0] = 0;
  eof = 0;
  if ( fgets(res,MAXRES,f) == NULL ) { eof = 1; return (char*)res; }
  int l = strlen(res);
  if ( l>MAXRES ) l = MAXRES;
  if ( l>0 ) if ( res[l-1]=='\n' ) res[l-1] = 0;
  return (char*)res;
}
///////////////////////////////////////////////////////////////////////
// busca secuencialmente por la cadena a buscar, quedando posicionado
// inmediatamente despues
//Devuelve la posicion del fichero o -1 si no se encuentra
//Tambien devuelve el nuemro de lineas (retornos de carro) encontrados para descontarlos 
//si se usa la posicion devuelta para otra cosa que no sea fseek
unsigned long BuscaString(FILE* pFile, const char* lpszStringA_Buscar,unsigned long& ulNumLineas)
{
    //make sure we were passed a valid, if it isn't return -1
    if ((!pFile)||(!lpszStringA_Buscar))
        return -1;

    //if the file is empty return -1
    unsigned long ulOrigPos=ftell(pFile);
    fseek(pFile,0,SEEK_END);
    unsigned long ulFileSize=ftell(pFile);
    if (!ulFileSize)
        return -1;

    //get the length of the string we're looking for, this is
    //the size the buffer will need to be
    unsigned long ulBufferSize=strlen(lpszStringA_Buscar);

    if (ulBufferSize>ulFileSize)
        return -1;

    //allocate the memory for the buffer
    char* lpBuffer=(char*)malloc(ulBufferSize);
    if (!lpBuffer)
        return -1;


    //this is where the actual searching will happen, what happens
    //here is we set the file pointer to the current position
    //is incremented by one each pass, then we read the size of
    //the buffer into the buffer and compare it with the string
    //we're searching for, if the string is found we return the
    //position at which it is found
    unsigned long ulCurrentPosition=ulOrigPos;
    ulNumLineas = 0;
    while (ulCurrentPosition<ulFileSize-ulBufferSize)
    {
        //set the pointer to the current position
        fseek(pFile,ulCurrentPosition,SEEK_SET);

        //read ulBufferSize bytes from the file
        fread(lpBuffer,1,ulBufferSize,pFile);

        //if the data read matches the string we're looking for
        if (!memcmp(lpBuffer,lpszStringA_Buscar,ulBufferSize))
        {
            //free the buffer
            free(lpBuffer);

            //nos posicionamos DESPUES del string
            ulCurrentPosition += ulBufferSize;
            fseek(pFile,ulCurrentPosition,SEEK_SET);

            // al haber contado '\n' doble, deberia ser par
            ulNumLineas = ulNumLineas/2;

            //return the position the string was found at
            return ulCurrentPosition;
        }
        //esto va a ocurrir dos veces por cada linea porque fread sustituye "0x0a 0x0d" por "0x0a" 
        //y tambien "0x0d loquesea" por "0x0a loquesea", asi que al leer byte a byte fread nos 
        //devolvera dos veces 0x0a
        if (ulNumLineas!=-1 && lpBuffer[0] == '\n') 
            ulNumLineas++;
        ulCurrentPosition++;
    }

    //if we made it this far the string was not found in the file
    free(lpBuffer);

    // al haber contado '\n' doble, al encontrar fin de fichero contará solo un 0x0a, asi que hay que redondear
    ulNumLineas = (unsigned long) ceil((double)ulNumLineas/2.0);

    return -1;
} 
///////////////////////////////////////////////////////////////////////
char * GetField(char * lin, char sep, int & p) {
  static char res[MAXRES];
  int lr = 0;
  int ll = strlen(lin);
  for ( ; p<ll; p++ ) {
    if ( lin[p] == sep ) { p++; break; }
    res[lr++] = lin[p];
  }
  res[lr]=0;
  return (char*)res;
}
///////////////////////////////////////////////////////////////////////
char * GetField(char * lin, char * sep, int & p) {
  static char res[MAXRES];
  int lr = 0;
  int ll = strlen(lin);
  for ( ; p<ll; p++ ) {
    if ( Pos(lin[p],sep) != -1 ) { p++; break; }
    res[lr++] = lin[p];
  }
  res[lr]=0;
  return (char*)res;
}
///////////////////////////////////////////////////////////////////////
char * LTrim(char * lin) {
  static char res[MAXRES];
  int ll, p, x;
  ll = strlen(lin);
  bool ini = true;
  x = 0;
  for ( p=0; p<ll; p++ ) {
	if ( ini ) if (lin[p]==' ' || lin[p]=='\t') continue;
	ini = false;
	res[x++] = lin[p];
  }
  res[x] = 0;
  return res;
}
///////////////////////////////////////////////////////////////////////
char * RTrim(char * lin) {
  static char res[MAXRES];
  int p, ll;
  strcpy(res,lin);
  ll = strlen(res);
  for ( p=ll-1; p>=0; p-- ) {
    if ( res[p] == ' ' || res[p] == '\t' ) res[p]=(char)0; else break;
  }
  return res;
}
///////////////////////////////////////////////////////////////////////
#if !defined (__BORLANDC__)
char * Conv(char * text) {
  static char aux[1000];
  CharToOem(text, aux);
  return aux;
}
///////////////////////////////////////////////////////////////////////
void ErrorExit(char * p, void * m, ...) {
  printf(p,m);
  printf("\nPress any key to exit\n");
  getchar();
  exit(1);
}
///////////////////////////////////////////////////////////////////////
void f2printf(FILE * f, char * p, void * m, ...) {
  fprintf(f,p,m);
  printf(p,m);
}
///////////////////////////////////////////////////////////////////////
#endif

