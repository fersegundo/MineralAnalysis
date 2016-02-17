/*****************************************************************************
   
    Interfaz para control de los dispositivos del microscopio ZEISS

*****************************************************************************/

#ifndef CONTROL_MICROSCOPIO_ZEISS_H
#define CONTROL_MICROSCOPIO_ZEISS_H

// include declarations of dataclasses for different devices
#include "..\ProcesoAux\estructuras.h"
#include ".\mtb\mtb_data.h"		// definitions of ToolBox classes

// Declaración de funciones incluidas en control_microscopio.cpp 
int		ini_control_microscopio_zeiss(int posFiltro,double voltLamp, limitesMtb *limMtb, int filtroUltimaBanda);
int		fin_control_microscopio_zeiss();

int	    mspGo_zeiss(int xyz, double pos);
int	    mspGoXY_zeiss(double xPos, double yPos);
int	    mspGoZ_zeiss(double pos);
double	mspWhere_zeiss(int xyz);
int	    mspWhereXY_zeiss(double *xPos, double *yPos);
int	    mspWhereZ_zeiss(double *pos);

void	mspGoAtThisSpeed_zeiss(int xyz, double velo);


int		mspSetLamp_zeiss(double val);
double	mspGetLamp_zeiss();

int		mspSetRevolver_zeiss(int val);
int		mspSetRevolver_zeiss(int val, bool bEspera);
int     mspGetObjetivo_zeiss();
CString mspGetNombreObjetivo_zeiss(int nObjetivo);
#if !defined (__BORLANDC__)
int		mspGetRevolver_zeiss(CString & posFiltro);
#else
int		mspGetRevolver_zeiss(AnsiString & posFiltro);
#endif
int		mspEsperaRevolverFin_zeiss();


// AUXILIARES
int		coinciden(int xyz, double v1, double v2);
int		coincidenP(point p1, point p2);
char	nombre_eje(int xyz);
int		mspEsperaRevolverInicio();
char*   mspRevolverInfo(limitesMtb *limMtb);
char*   mspLampInfo( limitesMtb *limMtb);
#if !defined (__BORLANDC__)
int		mspFijaPos(int xyz, double pos);
#endif
void	mspGetStageSpeed(double *vx, double *vy);
void	mspSetStageSpeed(double vx, double vy);
void	mspGetFocSpeed(double *vz);
void	mspStop(int xyz = 0);

#endif 

