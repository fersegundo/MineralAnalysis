/*****************************************************************************
	control_microscopio.h		
    
    Interfaz generico para control de los dispositivos del microscopio

*****************************************************************************/

#ifndef CONTROL_MICROSCOPIO_H
#define CONTROL_MICROSCOPIO_H

#define BACKSLASHZ		  6.0 //1.2		// Juego típico de los engranajes del eje Z

// include declarations of dataclasses for different devices
#include "..\ProcesoAux\estructuras.h"

// Declaración de funciones incluidas en control_microscopio.cpp 
int ini_control_microscopio(int posFiltro,double voltLamp, limitesMtb *limMtb, int filtroUltimaBanda);
int  fin_control_microscopio();
bool EsLeica();

int	    mspGo(int xyz, double pos);
int	    mspGoXY(double xPos, double yPos);
int	    mspGoZ(double pos);
int		mspGoCheck(int xyz, double pos);
int     mspGoCheckXY(double xPos, double yPos);
void	mspGoP(point pGo);
int		mspCheck(int xyz, double pos);
int     mspCheckXY(double xPos, double yPos);
int     mspCheckZ(double pos);
double	mspWhere(int xyz);
int	    mspWhereXY(double *xPos, double *yPos);
int	    mspWhereZ(double *pos);
point	mspWhereP(void);
void	mspGoAtThisSpeed(int xyz, double velo);
void	mspGetStageSpeed(double *vx, double *vy);
void	mspSetStageSpeed(double vx, double vy);
void	mspStop(int xyz);
void	mspGetFocSpeed(double *vz);
#if !defined (__BORLANDC__)
int		mspFijaPos(int xyz, double pos);
#endif

//AUXILIARES
int		coinciden(int xyz, double v1, double v2);
int		coincidenP(point p1, point p2);
char	nombre_eje(int xyz);


char*   mspLampInfo(limitesMtb *limMtb);
int		mspSetLamp(double val);
double	mspGetLamp();

int  mspGetCubo();
bool  mspSetCubo(int nCubo);
CString mspGetNombreCubo(int nCubo);

int  mspGetObjetivo();
bool  mspSetObjetivo(int nObjetivo);
CString mspGetNombreObjetivo(int nObjetivo);

char*   mspRevolverInfo( limitesMtb *limMtb);
int		mspSetRevolver( int val);
int		mspSetRevolver( int val, bool bEspera);
#if !defined (__BORLANDC__)
int		mspGetRevolver( CString & posFiltro);
#else
int		mspGetRevolver( AnsiString & posFiltro);
#endif
int		mspEsperaRevolverInicio();
int		mspEsperaRevolverFin();


#endif // CONTROL_MICROSCOPIO_H

