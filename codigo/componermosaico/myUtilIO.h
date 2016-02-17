#ifndef myUtilIO_h
#define myUtilIO_h

#define MAXRES 1600 //maximo numero de caracteres en una linea

///////////////////////////////////////////////////////////////////////
#if !defined (__BORLANDC__)
int  getKey(void);
void strcatchar(char * s, char c);
char * d2a(double num);
char * d2a(double num, int dec);
char * i2a(int num);
int  Letter(char c);
char Upper(char c);
void Upper(char * s);
#endif
int  Pos(char c, char * s);
#if !defined (__BORLANDC__)
int  Pos(char * s, char * ss);
int  Count(char c, char * s);
void MovConsole(int s = 0);
int  FileExist(char * fn);
char * ReadWhile(FILE * f, char * s, int & eof);
char * ReadUntil(FILE * f, char * s, int & eof);
unsigned long BuscaString(FILE* pFile, const char* lpszStringA_Buscar,unsigned long& ulNumLineas);
#endif
char * ReadLine(FILE * f, int & eof);
char * GetField(char * lin, char sep, int & p);
char * GetField(char * lin, char * sep, int & p);
char * LTrim(char * lin);
char * RTrim(char * lin);
#if !defined (__BORLANDC__)
char * Conv(char * text);
void ErrorExit(char * p, void * m = NULL, ...);
void f2printf(FILE * f, char * p, void * m = NULL, ...);
///////////////////////////////////////////////////////////////////////
#endif
#endif

