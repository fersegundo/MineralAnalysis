/*****************************************************************************
	control_microscopio.cpp		
    
    Rutinas para control de los dispositivos de los microscopios ZEISS AXIOSKOP2 o LEICA DM6000

	Autores:	Fernando Segundo
	Copyright:	© AITEMIN 2008
*****************************************************************************/

#include "stdafx.h"


#include "..\ProcesoAux\gestion_mensajes.h"


#include "control_microscopio.h"
#include "control_microscopio_zeiss.h"
#include "control_microscopio_leica.h"
#include <math.h> // fabs

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#define MAXPOSERRORX       1.01	// Máximo error admisible en X
#define MAXPOSERRORY       2.01	// Máximo error admisible en Y
#define MAXPOSERRORZ       0.2	// Máximo error admisible en Z
#define RETRASO			  60	// Intervalo de espera típico para el control (ms)
#define MAXINTENTOS		  10	// Número máximo de intervalos de espera para control

HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)

bool m_bZeiss = false;
bool m_bLeica = false;
CLeica leica;

/*************************** ini_control_microscopio *************************
	Función para inicializar las funciones de control de los dispositivos
	del microscopio.
	posFiltro - Filtro predeterminado de la Rueda de filtros
	voltLamp - voltaje configurado de trabajo de la lampara del microscopio
	filtroUltimaBanda - filtro correspondiente a la ultima banda a tomar
*****************************************************************************/
int ini_control_microscopio(int posFiltro,double voltLamp, limitesMtb *limMtb, int filtroUltimaBanda)
{
	//ZEISS
	if (ini_control_microscopio_zeiss(posFiltro, voltLamp, limMtb, filtroUltimaBanda) != -1)
	{
		m_bZeiss = true;
		return 0;
	}
	else if (leica.ini_control_microscopio(posFiltro, voltLamp, limMtb) != -1)
	{
		m_bLeica = true;
		return 0;
	}
	else
	{
		m_bZeiss = false;
		m_bLeica = false;
		error_fatal("ini_control_microscopio", "No se ha encontrado ningun microscopio",0);
	}
	
	return -1;
}


/*************************** fin_control_microscopio *************************
	Función para desactivar las funciones de control de los dispositivos
	del microscopio.
	Además baja el nivel de intensidad de la lámpara a un nivel de 'reposo'.
*****************************************************************************/
int fin_control_microscopio()
{
	if (m_bZeiss == true)
	{
		return fin_control_microscopio_zeiss();
	}
	else if (m_bLeica == true)
	{
		return leica.fin_control_microscopio();
	}
	
	return -1;
}


// ***************************************************************************
//	true si estamos en un sistema Leica
bool EsLeica()
{
	return (m_bLeica);
}



/*  FUNCIONES DE MANEJO DEL MICROSCOPIO */

/**************************  mspGo  *****************************************
	Función para ordenar un movimiento absoluto en uno de los ejes.
****************************************************************************/
int  mspGo(int xyz, double pos)
{
	if (m_bZeiss == true)
	{
		return mspGo_zeiss(xyz, pos);
	}
	else if (m_bLeica == true)
	{
		leica.mspGo(xyz, pos);
	}
    return 0;
}

/**************************  mspGoXY  ***************************************
	Función para ordenar un movimiento absoluto de la platina.
****************************************************************************/
int  mspGoXY(double xPos, double yPos)
{
	if (m_bZeiss == true)
	{
		return mspGoXY_zeiss(xPos, yPos);
	}
	else if (m_bLeica == true)
	{
		leica.mspGo(X_, xPos);
		leica.mspGo(Y_, yPos);
	}
    return 0;
}

/**************************  mspGoZ  ****************************************
	Función para ordenar un movimiento absoluto del enfoque.
****************************************************************************/
int  mspGoZ(double pos)
{
	if (m_bZeiss == true)
	{
		return mspGoZ_zeiss(pos);
	}
	else if (m_bLeica == true)
	{
		leica.mspGo(Z_, pos);
	}
    return 0;
}

/**************************  mspGoP  ****************************************
   Función para efectuar un movimiento absoluto en los tres ejes,
   comprobando que la posición alcanzada corresponde realmente a la deseada.
****************************************************************************/
void  mspGoP(point pGo)
{
	//	En primer lugar comprobamos la posición actual de los ejes, por
	//	si no fuera necesario efectuar alguno de los movimientos.
	if ( ! coinciden(Y_, pGo.coord[Y_], mspWhere(Y_)) )
		mspGo(Y_, pGo.coord[Y_]);
	if ( ! coinciden(Z_, pGo.coord[Z_], mspWhere(Z_)) )
		mspGo(Z_, pGo.coord[Z_]);
	if ( ! coinciden(X_, pGo.coord[X_], mspWhere(X_)) )
		mspGo(X_, pGo.coord[X_]);

	Sleep(RETRASO);

	//	Después, verificamos que se ha llegado al punto solicitado. 
	mspCheck(X_, pGo.coord[X_]);
	mspCheck(Z_, pGo.coord[Z_]);
	mspCheck(Y_, pGo.coord[Y_]);
}

/**************************  mspWhere  **************************************
	Función para obtener la posición actual en uno de los ejes.
****************************************************************************/
double mspWhere(int xyz)
{
	if (m_bZeiss == true)
	{
		return mspWhere_zeiss(xyz);
	}
	else if (m_bLeica == true)
	{
		return leica.mspWhere(xyz);
	}
	
	return -1;
}

/**************************  mspGoAtThisSpeed  ******************************
	Función para fijar una velocidad absoluta en uno de los ejes.
****************************************************************************/
void  mspGoAtThisSpeed(int xyz, double velo)	// Función para fijar una velocidad
{												// absoluta en uno de los ejes.
	if (m_bZeiss == true)
	{
		mspGoAtThisSpeed_zeiss(xyz, velo);
	}
	else if (m_bLeica == true)
	{
		leica.mspGoAtThisSpeed(xyz, velo);
	}
}

/**************************  coinciden  *************************************
	Función para comprobar si dos valores X, Y ó Z coinciden aproximadamente.
	AUXILIAR a mspCheck
****************************************************************************/
int  coinciden(int xyz, double v1, double v2)
{
	return  ( fabs(v1 - v2) <= ((xyz == Z_)? MAXPOSERRORZ :
		((xyz == X_)? MAXPOSERRORX : MAXPOSERRORY)) );
}

/**************************  mspCheck  **************************************
   Función para verificar un movimiento absoluto en uno de los ejes,
   comprobando que la posición alcanzada corresponde realmente a la deseada.
   AUXILIAR a mspGoCheck
****************************************************************************/
int  mspCheck(int xyz, double pos)
{
	char  mensaje[1024];
    HWND console = NULL;
	int	 i = 1;
	double  posAct;

	// Esperamos a que se alcance la posición elegida antes de un tiempo límite. 
	if (m_bZeiss == true)
		posAct = mspWhere_zeiss(xyz);
	else if (m_bLeica == true)
		posAct = leica.mspWhere(xyz);

	while( ! coinciden(xyz, pos, posAct) )
	{
        Sleep(RETRASO);
		if (++i > MAXINTENTOS) 
        {
            sprintf(mensaje, "Error al mover en el eje %c a: %.2f. Posicion actual: %.2f", 'X' + xyz, pos, posAct);
            sendlog("mspCheck",mensaje);
            return -1;
        }
		if (m_bZeiss == true)
			posAct = mspWhere_zeiss(xyz);
		else if (m_bLeica == true)
			posAct = leica.mspWhere(xyz);
	}

    sprintf(mensaje, "Se queria mover el eje %c a: %.2f y se ha movido a %.2f", 'X' + xyz, pos, posAct);
    sendlog("mspCheck",mensaje);

	return 0;
}

/**************************  mspCheckXY  **************************************
   Función para verificar un movimiento absoluto en los ejes,
   comprobando que la posición alcanzada corresponde realmente a la deseada.
   AUXILIAR a mspGoCheckXY
****************************************************************************/
int  mspCheckXY(double xPos, double yPos)
{
	char  mensaje[1024];
    HWND console = NULL;
	int	 i = 1;
    int  res;
	double  xPosAct, yPosAct;

	// Esperamos a que se alcance la posición elegida antes de un tiempo límite. 
	if (m_bZeiss == true)
		res = mspWhereXY_zeiss(&xPosAct, &yPosAct);
	else if (m_bLeica == true)
    {
		xPosAct = leica.mspWhere(X_);
		yPosAct = leica.mspWhere(Y_);
        res = 0;
    }

	while( !coinciden(X_, xPos, xPosAct) && !coinciden(Y_, yPos, yPosAct) )
	{
        Sleep(RETRASO);
		if (++i > MAXINTENTOS) 
        {
            sprintf(mensaje, "Error al mover a (%.2f, %.2f). Posicion actual: (%.2f, %.2f)", xPos, yPos, xPosAct, yPosAct);
            sendlog("mspCheckXY",mensaje);
            return res;
        }
	    if (m_bZeiss == true)
		    mspWhereXY_zeiss(&xPosAct, &yPosAct);
	    else if (m_bLeica == true)
        {
		    xPosAct = leica.mspWhere(X_);
		    yPosAct = leica.mspWhere(Y_);
        }
	}

    sprintf(mensaje, "Se queria mover a (%.2f, %.2f) y se ha movido a (%.2f, %.2f)", xPos, yPos, xPosAct, yPosAct);
    sendlog("mspCheckXY",mensaje);

	return res;
}

/**************************  mspCheckZ  *************************************
   Función para verificar un movimiento absoluto del enfoque,
   comprobando que la posición alcanzada corresponde realmente a la deseada.
   AUXILIAR a mspGoCheck
****************************************************************************/
int  mspCheckZ(double pos)
{
	char  mensaje[1024];
    HWND console = NULL;
	int	 res, i = 1;
	double  posAct;

	// Esperamos a que se alcance la posición elegida antes de un tiempo límite. 
	if (m_bZeiss == true)
		res = mspWhereZ_zeiss(&posAct);
	else if (m_bLeica == true)
		posAct = leica.mspWhere(Z_);

	while( ! coinciden(Z_, pos, posAct) )
	{
        Sleep(RETRASO);
		if (++i > MAXINTENTOS) 
        {
            sprintf(mensaje, "Error al mover en el eje Z a: %.2f", pos);
            sendlog("mspCheckZ",mensaje);
	        if (m_bZeiss == true)
		        mspStop(Z_);
	        else if (m_bLeica == true)
                // HACER: implementar un mspStop para leica
		        leica.mspGoAtThisSpeed(Z_, 0.0);
            Sleep(RETRASO*5);
            sprintf(mensaje, "Posicion actual del eje Z: %.2f", posAct);
            sendlog("mspCheckZ",mensaje);
            return -1;
        }
		if (m_bZeiss == true)
			res = mspWhereZ_zeiss(&posAct);
		else if (m_bLeica == true)
			posAct = leica.mspWhere(Z_);
	}

    sprintf(mensaje, "Se queria mover el eje Z a: %.2f y se ha movido a %.2f", pos, posAct);
    sendlog("mspCheckZ",mensaje);

	return res;
}

/**************************  mspGoCheck  ************************************
   Función para efectuar un movimiento absoluto en uno de los ejes,
   comprobando que la posición alcanzada corresponde realmente a la deseada.
   Devuelve 1 si la posicion es correcta. -1 en otro caso
****************************************************************************/
int mspGoCheck(int xyz, double pos)
{

    mspGo(xyz, pos);

	switch (xyz) {
		case X_:  
            Sleep(RETRASO * 2);  
            break;
		case Y_:  
            Sleep(RETRASO * 6);  
            break;
		case Z_:  
            Sleep(RETRASO * 10);
	}

    return mspCheck(xyz, pos);
}

/**************************  mspGoCheckXY  ************************************
   Función para efectuar un movimiento absoluto en los ejes X e Y
   comprobando que la posición alcanzada corresponde realmente a la deseada.
   Devuelve 1 si la posicion es correcta. -1 en otro caso
****************************************************************************/
int mspGoCheckXY(double xPos, double yPos)
{

    mspGoXY(xPos, yPos);
    Sleep(RETRASO * 6);  
    return mspCheckXY(xPos, yPos);
}

/**************************  mspWhereP  *************************************
   Función para solicitar la posición actual en los tres ejes.
****************************************************************************/
point  mspWhereP()
{
	point pRes;
	pRes.peso		= -1;

	pRes.coord[X_] = mspWhere(X_);
	pRes.coord[Y_] = mspWhere(Y_);
	pRes.coord[Z_] = mspWhere(Z_);

	return pRes;
}




/***************************  mspSetLamp  ***********************************
	Función para fijar un nivel de intensidad de la luz.
*****************************************************************************/
int  mspSetLamp(double val)
{
	if (m_bZeiss == true)
	{
		return mspSetLamp_zeiss( val);
	}
	else if (m_bLeica == true)
	{
		leica.mspSetLamp(val);
	}
	
	return -1;
}

/***************************  mspGetLamp  ***********************************
	Función para leer el nivel de intensidad de la luz.
*****************************************************************************/
double  mspGetLamp()
{
	if (m_bZeiss == true)
	{
		return mspGetLamp_zeiss();
	}
	else if (m_bLeica == true)
	{
		return leica.mspGetLamp();
	}
	
	return -1;
}

//*****************************************************************************
//     CUBOS
//*****************************************************************************

// Función para leer el cubo actualmente en uso (1..MAX_CUBOS)
int  mspGetCubo()
{
	if (m_bZeiss == true)
	{
		return -1; //mspGetObjetivo_zeiss(); //HACER
	}
	else if (m_bLeica == true)
	{
		return leica.mspGetTurret();
	}
	
	return -1;
}

// nCubo - (1..MAX_CUBOS)
// Si nIndice es -1, se entiende como una pregunta "¿Es posible cambiar objetivo?"
bool  mspSetCubo(int nCubo)
{
    if (m_bLeica == true)
	{
        if (nCubo == -1)
            return true;
        else
		    return leica.mspSetTurret(nCubo);
	}
	
	return false;
}

//Obtiene el nombre del objetivo dado por el indice
// nCubo - (1..MAX_OBJETIVOS)
CString mspGetNombreCubo(int nCubo)
{
	if (m_bZeiss == true)
	{
		return ""; //mspGetNombreObjetivo_zeiss(nCubo); //HACER
	}
	else if (m_bLeica == true)
	{
		return leica.mspGetNombreTurret(nCubo);
	}
	
	return "";
}


//*****************************************************************************
//     OBJETIVO
//*****************************************************************************

// Función para leer el objetivo actualmente en uso (1..MAX_OBJETIVOS)
int  mspGetObjetivo()
{
	if (m_bZeiss == true)
	{
		return mspGetObjetivo_zeiss();
	}
	else if (m_bLeica == true)
	{
		return leica.mspGetObjetivo();
	}
	
	return -1;
}

// nObjetivo - (1..MAX_OBJETIVOS)
// Si nIndice es -1, se entiende como una pregunta "¿Es posible cambiar objetivo?"
bool  mspSetObjetivo(int nObjetivo)
{
    if (m_bLeica == true)
	{
        if (nObjetivo == -1)
            return true;
        else
		    return leica.mspSetObjetivo(nObjetivo);
	}
	
	return false;
}

//Obtiene el nombre del objetivo dado por el indice
// nObjetivo - (1..MAX_OBJETIVOS)
CString mspGetNombreObjetivo(int nObjetivo)
{
	if (m_bZeiss == true)
	{
		return mspGetNombreObjetivo_zeiss(nObjetivo);
	}
	else if (m_bLeica == true)
	{
		return leica.mspGetNombreObjetivo(nObjetivo);
	}
	
	return "";
}



/***************************  mspSetRevolver  *******************************
	Función para fijar la posición del revolver.
	No gestiona espera.
*****************************************************************************/
int  mspSetRevolver( int val)
{
	if (m_bZeiss == true)
	{
		return mspSetRevolver_zeiss( val);
	}
	else if (m_bLeica == true)
	{
		return leica.mspSetRevolver(val);
	}

	//HACER TING: modificar la exposicion segun el porcentaje de este filtro
	//ModificaExposicion(...);

	return -1;
}

/***************************  mspSetRevolver  *******************************
	Función para fijar la posición del revolver
	bEspera - si 'true', se espera a que termine de cambiar el revolver
	          si 'false', se espera unicamente a que el revolver comience a cambiar, despues
			  se devuelve inmediatamente la ejecucion.
*****************************************************************************/
int  mspSetRevolver( int val,bool bEspera)
{
	if (m_bZeiss == true)
	{
		return mspSetRevolver_zeiss( val, bEspera);
	}
	else if (m_bLeica == true)
	{
        //En Leica no se puede gestionar espera
		return leica.mspSetRevolver(val);
	}
	
	return -1;
}

/***************************  mspGetRevolver  *******************************
	Función para leer la posición de actual del revolver.
*****************************************************************************/
#if !defined (__BORLANDC__)
int mspGetRevolver( CString & nomFiltro)
#else
int mspGetRevolver( AnsiString & nomFiltro)
#endif
{
	if (m_bZeiss == true)
	{
#if !defined (__BORLANDC__)
		mspGetRevolver_zeiss( nomFiltro);
#else
		mspGetRevolver_zeiss( nomFiltro);
#endif
	}
	else if (m_bLeica == true)
	{
        //En Leica no se puede mover asincronamente
		return leica.mspGetRevolver();
	}
	
	return -1;
}

/***************************  mspEsperaRevolverFin  *******************************
	Espera activa hasta que el revolver deja de moverse o error
	devuelve 0 cuando el revolver esta parado y activo
	devuelve -1 en caso de error
*****************************************************************************/
int mspEsperaRevolverFin()
{
	if (m_bZeiss == true)
	{
		return mspEsperaRevolverFin_zeiss();
	}
	else if (m_bLeica == true)
    {
        //En Leica no se puede mover asincronamente
        return 0;
    }
	return -1;
}
