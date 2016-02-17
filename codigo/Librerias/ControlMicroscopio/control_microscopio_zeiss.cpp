/*****************************************************************************
	control_microscopio_zeiss.cpp		
    
    Rutinas para control de los dispositivos del microscopio ZEISS

	Autores:	JCC, BLM, FSG
	Copyright:	© AITEMIN 2007
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#if !defined (__BORLANDC__)
#include <stdio.h>
#endif

#include "..\ProcesoAux\gestion_mensajes.h"
#include "control_microscopio_zeiss.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// Defines para control de la platina
#define ESPERA            50    // Espera para reintentos de lectura de posición
#define MAX_INTENTOS       3    // Número máximo de reintentos de lectura de posición

// Acceso a los elementos del microscopio
#define LAMP_HALOGENA	   0	// Lámpara halógena.
#define OBJETIVOS          0    // nosepiece (objective turret)
#define FILTRO_EXCITACION 11	// Rueda de filtros.


CMTB_Data m_dat;		// class for MicroToolbox data

/*************************** ini_control_microscopio_zeiss *************************
	Función para inicializar las funciones de control de los dispositivos
	del microscopio.
	posFiltro - Filtro predeterminado de la Rueda de filtros
	voltLamp - voltaje configurado de trabajo de la lampara del microscopio
	filtroUltimaBanda - filtro correspondiente a la ultima banda a tomar
	Devuelve -1 si no se encuentra el microscopio
*****************************************************************************/
int ini_control_microscopio_zeiss(int posFiltro,double voltLamp, limitesMtb *limMtb, int filtroUltimaBanda)
{
#if _DEBUG
    char mensaje[512];
#endif
	char *fn = "ini_control_microscopio";


	m_dat.Connect_MTB();
	if ( m_dat.m_connected == 0 )
	{
//		error_fatal(fn, "Error: MTB no disponible\n", 0);
		return -1;
	}

#if _DEBUG
	int i;
    char linea[256];
#if !defined (__BORLANDC__)
	char mensaje2[2048];
#endif
	    // Comprobación de elementos disponibles.

	    strcpy(mensaje, "Lista de dispositivos:");
	    if ( m_dat.m_stand.m_Stand_available )
	    {
    #if !defined (__BORLANDC__)
		    sprintf(linea, "\nMicroscopio: %s", m_dat.m_stand.m_MicroscopeName);
    #else
		    sprintf(linea, "\nMicroscopio: %s", m_dat.m_stand.m_MicroscopeName.c_str());
    #endif
		    strcat(mensaje, linea);
	    }
	    if ( m_dat.m_focus.m_FocusType )
	    {
    #if !defined (__BORLANDC__)
		    sprintf(linea, "\nEnfoque: %s", m_dat.m_focus.m_FocusName);
    #else
		    sprintf(linea, "\nEnfoque: %s", m_dat.m_focus.m_FocusName.c_str());
    #endif
		    strcat(mensaje, linea);
	    }
	    if ( m_dat.m_stage.m_StageType )
	    {
    #if !defined (__BORLANDC__)
		    sprintf(linea, "\nPlatina: %s", m_dat.m_stage.m_StageName);
    #else
		    sprintf(linea, "\nPlatina: %s", m_dat.m_stage.m_StageName.c_str());
    #endif
		    strcat(mensaje, linea);
	    }
	    if ( m_dat.m_lightpath.m_DeviceType )
	    {
    #if !defined (__BORLANDC__)
		    sprintf(linea, "\nLightpath: %s", m_dat.m_lightpath.m_DeviceName);
    #else
		    sprintf(linea, "\nLightpath: %s", m_dat.m_lightpath.m_DeviceName.c_str());
    #endif
		    strcat(mensaje, linea);
	    }
	    // El número del revolver de la rueda de exitancia es el 11.
	    for (i=0; i < MTB_CHG; i++)
		    if ( m_dat.m_revolvers.m_RevolverType[i] )
		    {
    #if !defined (__BORLANDC__)
			    sprintf(linea, "\nRevólver %d: %s", i, m_dat.m_revolvers.m_RevolverName[i]);
	            // Leo el nombre [posición numérica] de la posición actual.
			    sprintf(linea, "\nRevólver %d: %s", i, m_dat.m_revolvers.m_RevolverName[i]);
                CString csNombre;
	            m_dat.m_revolvers.GetRevolverPosName(i, mspGetObjetivo_zeiss(), csNombre);
			    sprintf(linea, "\nNombre posicion revólver %d: %s", i, (LPCSTR)csNombre);
    #else
			    sprintf(linea, "\nRevólver %d: %s", i, m_dat.m_revolvers.m_RevolverName[i].c_str());
    #endif

			    strcat(mensaje, linea);
		    }
	    for (i=0; i < _PL_LAST_SERVO; i++)
		    if ( m_dat.m_servos.m_ServoType[i] )
		    {
    #if !defined (__BORLANDC__)
			    sprintf(linea, "\nServo %d: %s", i, m_dat.m_servos.m_ServoName[i]);
    #else
			    sprintf(linea, "\nServo %d: %s", i, m_dat.m_servos.m_ServoName[i].c_str());
    #endif
			    strcat(mensaje, linea);
		    }
	    // El número de la lámpara utilizada es cero.
	    for (i=0; i < _PLA_LAST_LAMP; i++)
		    if ( m_dat.m_lamps.m_LampType[i] )
		    {
    #if !defined (__BORLANDC__)
			    sprintf(linea, "\nLámpara %d: %s", i, m_dat.m_lamps.m_LampName[i]);
    #else
			    sprintf(linea, "\nLámpara %d: %s", i, m_dat.m_lamps.m_LampName[i].c_str());
    #endif
			    strcat(mensaje, linea);
		    }

    #if !defined (__BORLANDC__)
	    CharToOem(mensaje, mensaje2);
	    printf("\n%s\n", mensaje2);
    #else
	    MuestraMensaje(mensaje);
    #endif
#endif //_DEBUG

	//	Una vez comprobados los dispositivos de los que consta el microscopio,
	//	fijamos los parámetros relativos al mismo para la toma de imágenes. 

	if (mspLampInfo(limMtb) == NULL)			// Características de la lámpara.
        return -1;
	if (mspRevolverInfo(limMtb) == NULL)	// Características de la rueda de filtros.
        return -1;

	if (voltLamp > limMtb->MaxLamp || voltLamp < limMtb->MinLamp)  {
		error_fatal(fn,"Valor inadmisible de MTB_voltLamp", 0);
		return -1;
	}

	mspSetLamp_zeiss(voltLamp);		// Posición de la lámpara.

	if (posFiltro > limMtb->MaxPosRevolver || posFiltro < 1)  {
		error_fatal(fn, "Valor inadmisible de MTB_posFiltro", 0);
		return -1;
	}

	/*	Nos desplazamos a la ultima banda antes de posicionarnos en el filtro predefinido
		Esto se hace para asegurarnos que la rueda cambia en la misma direccion que cuando
	    cambie al hacer el barriro. Puede ser que si cambiase en diferente direccion, la posicion
	    del filtro no fuese exactamente la misma y por tanto alguna mota de polvo nos afectase */
    // Como los filtros del microscopio no se van a usar (polarizacion), podemos ahorrarnos este paso
	//mspSetRevolver_zeiss(filtroUltimaBanda,true); // Posición del filtro de la ultima banda
	//mspSetRevolver_zeiss(posFiltro,true);// Posición de la rueda de filtros.

/////////////////////////////////////////////////
//	printf("Archivo de configuracion:\n   %s\n", m_dat.m_IniPath);
//	printf("Directorio de DLL's:\n   %s\n", m_dat.m_DLLPath);
//	printf("Tipo de platina: %d\n", m_dat.m_stage.m_StageType);
//	printf("Nombre de platina: %s\n", m_dat.m_stage.m_StageName);

	return 0;
}


/*************************** fin_control_microscopio *************************
	Función para desactivar las funciones de control de los dispositivos
	del microscopio.
	Además baja el nivel de intensidad de la lámpara a un nivel de 'reposo'.
*****************************************************************************/
int fin_control_microscopio_zeiss()
{
	//	Desactivo el microscopio.
	m_dat.Disconnect_MTB();

	return 0;
}


/*  FUNCIONES DE MANEJO DEL MICROSCOPIO */

/**************************  mspGo  *****************************************
	Función para ordenar un movimiento absoluto en uno de los ejes.
****************************************************************************/
int  mspGo_zeiss(int xyz, double pos)
{
	static char mensaje[1024];
	int	 res, num = 0;
	double	xPos, yPos;

	// Movimiento en el eje X o Y. 

	if ( xyz != Z_ )
	{
        // Leemos la posición actual de la platina
        while (res = m_dat.m_stage.ReadStagePos(&xPos, &yPos)) 
        {
		    //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
            // Esperamos y lo intentamos de nuevo 
            Sleep(ESPERA);
            if (++num > MAX_INTENTOS) 
	        {
			    sprintf(mensaje, "Error %d al enviar el comando ReadStagePos. Posición devuelta: (%.2lf,%.2lf)", res, xPos, yPos);
			    error_leve("mspGo", mensaje);
                break; 
            }
		}

        // Reajustamos la posición a la deseada
        if ( xyz == X_ ) 
            xPos = pos;
		else 
            yPos = pos;

        // Ordenamos el desplazamiento de la platina a la nueva posición
        while (res = m_dat.m_stage.MoveStage(xPos, yPos))
        {
		    //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
            // Esperamos y lo intentamos de nuevo 
            Sleep(ESPERA);
            if (++num > MAX_INTENTOS) 
    		{
	    		sprintf(mensaje, "Error %d al enviar el comando MoveStage(%.2lf,%.2lf)", res, xPos, yPos);
		    	error_leve("mspGo", mensaje);
                break; 
		    }
        }
	}

    // Movimiento en el eje Z. 

    else
	{
        // Ordenamos el desplazamiento del enfoque a la posición indicada
        while (res = m_dat.m_focus.MoveFocus(pos))
        {
		    //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
            // Esperamos y lo intentamos de nuevo 
            Sleep(ESPERA);
            if (++num > MAX_INTENTOS) 
    		{
	    		sprintf(mensaje, "Error %d al enviar el comando MoveFocus(%.2lf)", res, pos);
		    	error_leve("mspGo", mensaje);
                break;
		    }
        }
	}
    return res;
}


/**************************  mspGoXY  ***************************************
	Función para ordenar un movimiento absoluto de la platina.
****************************************************************************/
int  mspGoXY_zeiss(double xPos, double yPos)
{
	static char mensaje[1024];
	int	 res, num = 0;

    // Ordenamos el desplazamiento de la platina a la nueva posición
    while (res = m_dat.m_stage.MoveStage(xPos, yPos))
    {
        //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
        // Esperamos y lo intentamos de nuevo 
        Sleep(ESPERA);
        if (++num > MAX_INTENTOS) 
        {
	    	sprintf(mensaje, "Error %d al enviar el comando MoveStage(%.2lf,%.2lf)", res, xPos, yPos);
		   	error_leve("mspGoXY", mensaje);
            break; 
        }
	}
    return res;
}


/**************************  mspGoZ  ****************************************
	Función para ordenar un movimiento absoluto del enfoque.
****************************************************************************/
int  mspGoZ_zeiss(double pos)
{
	static char mensaje[1024];
	int	 res, num = 0;

    // Ordenamos el desplazamiento del enfoque a la posición indicada
    while (res = m_dat.m_focus.MoveFocus(pos))
    {
        //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
        // Esperamos y lo intentamos de nuevo 
        Sleep(ESPERA);
        if (++num > MAX_INTENTOS) 
    	{
	   		sprintf(mensaje, "Error %d al enviar el comando MoveFocus(%.2lf)", res, pos);
	    	error_leve("mspGoZ", mensaje);
            break;
        }
	}
    return res;
}


/**************************  mspWhere  **************************************
	Función para obtener la posición actual en uno de los ejes.
****************************************************************************/
double mspWhere_zeiss(int xyz)
{
	static char mensaje[1024];
	int res, num = 0;
	double xPos, yPos, zPos;

	if ( xyz != Z_ )
	{
        // Leemos la posición actual de la platina
        while (res = m_dat.m_stage.ReadStagePos(&xPos, &yPos)) 
        {
		    //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
            // Esperamos y lo intentamos de nuevo 
            Sleep(ESPERA);
            if (++num > MAX_INTENTOS) 
	        {
			    sprintf(mensaje, "Error %d al enviar el comando ReadStagePos. Posición devuelta: (%.2lf,%.2lf)", res, xPos, yPos);
			    error_leve("mspWhere", mensaje);
                break; 
            }
		}
        // Devolvemos el valor pedido
		if ( xyz == X_ ) 
            return xPos;
		else            
            return yPos;
    } else {
        // Leemos la posición actual del enfoque
	    while (res = m_dat.m_focus.ReadFocusPos(&zPos)) 
        {
	        //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
            // Esperamos y lo intentamos de nuevo 
            Sleep(ESPERA);
            if (++num > MAX_INTENTOS) 
	        {
    		    sprintf(mensaje, "Error %d al enviar el comando ReadFocusPos. Posición devuelta: %.2lf", res, zPos);
    		    error_leve("mspWhere", mensaje);
                break;
            }
        }
    	return zPos;
	}
}


/**************************  mspWhereXY  ************************************
	Función para obtener la posición actual de la platina.
****************************************************************************/
int mspWhereXY_zeiss(double *xPos, double *yPos)
{
	static char mensaje[1024];
	int res, num = 0;

    // Leemos la posición actual de la platina
    while (res = m_dat.m_stage.ReadStagePos(xPos, yPos)) 
    {
	    //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
        // Esperamos y lo intentamos de nuevo 
        Sleep(ESPERA);
        if (++num > MAX_INTENTOS) 
        {
		    sprintf(mensaje, "Error %d al enviar el comando ReadStagePos. Posición devuelta: (%.2lf,%.2lf)", res, *xPos, *yPos);
		    error_leve("mspWhereXY", mensaje);
            break;
        }
	}
    return res; 
}


/**************************  mspWhereZ  *************************************
	Función para obtener la posición actual del enfoque.
****************************************************************************/
int mspWhereZ_zeiss(double *pos)
{
	static char mensaje[1024];
	int res, num = 0;

    // Leemos la posición actual del enfoque
    while (res = m_dat.m_focus.ReadFocusPos(pos)) 
    {
        //res 16 es un error que da continuamente, pero si lo obviamos todo parece funcionar perfectamente
        // Esperamos y lo intentamos de nuevo 
        Sleep(ESPERA);
        if (++num > MAX_INTENTOS) 
        {
   		    sprintf(mensaje, "Error %d al enviar el comando ReadFocusPos. Posición devuelta: %.2lf", res, *pos);
   		    error_leve("mspWhereZ", mensaje);
            break;
        }
	}
    return res;
}


/**************************  mspGoAtThisSpeed  ******************************
	Función para fijar una velocidad absoluta en uno de los ejes.
****************************************************************************/
void  mspGoAtThisSpeed_zeiss(int xyz, double velo)	// Función para fijar una velocidad
{												// absoluta en uno de los ejes.
	static char mensaje[64];
	int	 res;

	if ( xyz != Z_ )
	{
		if ( xyz == X_ )
			res = m_dat.m_stage.GoStage(velo, 0.0);
		else
			res = m_dat.m_stage.GoStage(0.0, velo);
		if ( res )
		{
			sprintf(mensaje, "Error %d al enviar el comando GoStage", res);
			error_leve("mspGoAtThisSpeed", mensaje);
		}
	}
    /* Z */
	else
	{
		res = m_dat.m_focus.GoFocus(velo);
		if ( res )
		{
			sprintf(mensaje, "Error %d al enviar el comando GoFocus", res);
			error_leve("mspGoAtThisSpeed", mensaje);
		}
	}

}
/**************************  nombre_eje  ************************************
   Función para indicar el nombre del eje considerado.
****************************************************************************/
char  nombre_eje(int xyz)
{
   return ('X' + xyz);
}

/**************************  mspGetObjetivo  *****************************
****************************************************************************/
int mspGetObjetivo_zeiss()
{
	unsigned int  val;

	if ( ! m_dat.m_revolvers.m_RevolverType[OBJETIVOS] ) {
		error_leve("mspGetObjetivo", "Objetivos inaccesibles");
		return -1;
	}

	// Leo la posición actual: posición numérica.
	m_dat.m_revolvers.ReadRevolverPos(OBJETIVOS, &val);

	return (int)val;
}

//Obtiene el nombre del objetivo dado por el indice
CString mspGetNombreObjetivo_zeiss(int nObjetivo)
{
	if ( ! m_dat.m_revolvers.m_RevolverType[OBJETIVOS] ) {
		error_leve("mspGetObjetivo", "Objetivos inaccesibles");
		return -1;
	}

	// Leo la posición actual: posición numérica.
    CString csName;
	m_dat.m_revolvers.GetRevolverPosName(OBJETIVOS, nObjetivo, csName);

	return csName;

}

/**************************  mspGetStageSpeed  *****************************
NO USADO
****************************************************************************/
void mspGetStageSpeed(double *vx, double *vy)
{
	m_dat.m_stage.GetStageSpeed(vx, vy);
}

/**************************  mspSetStageSpeed  *****************************
NO USADO
****************************************************************************/
void mspSetStageSpeed(double vx, double vy)
{
	m_dat.m_stage.GoStage(vx, vy);
}


/**************************  mspGetFocSpeed  *******************************
NO USADO
****************************************************************************/
void mspGetFocSpeed(double *vz)
{
	m_dat.m_focus.GetFocSpeed(vz);
}


/**************************  mspStop  ***************************************
	Función para ordenar la parada en XY o Z
    xyz - opcional. por defecto será 0 (es decir X_, es decir parada en XY)
****************************************************************************/
void  mspStop(int xyz)                 /* Función para ordenar la parada */
{                                      /* de uno de los ejes. */
	char  mensaje[128];
	int	 res;

	if ( xyz != Z_ )
	{
		res = m_dat.m_stage.StopStage();
		if ( res != 0 )
		{
			sprintf(mensaje, "Error %d al enviar el comando StopStage", res);
			error_leve("mspStop", mensaje);
		}
	}
	else
	{
		res = m_dat.m_focus.StopFocus();
		if ( res != 0 )
		{
			sprintf(mensaje, "Error %d al enviar el comando StopFocus", res);
			error_leve("mspStop", mensaje);
		}
	}
}


#if !defined (__BORLANDC__)
/***************************  mspFijaPos  ***********************************
	Función para fijar una posición deseada en el eje de las X e Y
NO USADO
*****************************************************************************/
int  mspFijaPos(int xyz, double pos)
{
	int res;
	char mensaje[128];
	double posX, posY;

	res = m_dat.m_stage.ReadStagePos(&posX, &posY);
	if (res != 0)  {
		sprintf(mensaje, "Error %d al enviar el comando ReadStagePos", res);
		error_leve("mspFijaPos", mensaje);
		return 1;
	}

	switch (xyz) {
		case X_:
			res =  m_dat.m_stage.SetStagePos(pos, posY);
			if ( res != 0)  {
				sprintf(mensaje, "Error %d al enviar el comando SetStage", res);
				error_leve("mspFijaPos", mensaje);
				return 1;
			}
			break;
		case Y_:
			res =  m_dat.m_stage.SetStagePos(posX, pos);
			if ( res != 0)  {
				sprintf(mensaje, "Error %d al enviar el comando SetStage", res);
				error_leve("mspFijaPos", mensaje);
				return 1;
			}
			break;
		case Z_:
			res = m_dat.m_focus.SetFoc(pos);
			if ( res != 0)  {
				sprintf(mensaje, "Error %d al enviar el comando SetFoc", res);
				error_leve("mspFijaPos", mensaje);
				return 1;
			}
			break;
	}

	return 0;
}
#endif






/***************************  mspSetLamp  ***********************************
	Función para fijar un nivel de intensidad de la luz.
*****************************************************************************/
int  mspSetLamp_zeiss(double val)
{
	if ( ! m_dat.m_lamps.m_LampType[LAMP_HALOGENA] ) {
		error_leve("mspSetLamp", "Lámpara inexistente");
		return -1;
	}

	m_dat.m_lamps.SetLampPos(LAMP_HALOGENA, val);
	return 0;
}


/***************************  mspGetLamp  ***********************************
	Función para leer el nivel de intensidad de la luz.
*****************************************************************************/
double  mspGetLamp_zeiss()
{
	double  val;

	if ( ! m_dat.m_lamps.m_LampType[LAMP_HALOGENA] ) {
		error_leve("mspGetLamp", "Lámpara inexistente");
		return -1.0;
	}

	m_dat.m_lamps.ReadLampPos(LAMP_HALOGENA, &val);
	return  val;
}

/***************************  mspLampInfo  **********************************
	Función para leer el tipo de lámpara, así como las características de
	intensidad máximas y mínimas de la misma.
*****************************************************************************/
char  * mspLampInfo(limitesMtb *limMtb)
{
	if ( ! m_dat.m_lamps.m_LampType[LAMP_HALOGENA] ) {
		error_leve("mspLampInfo", "Lámpara inexistente");
		return NULL;
	}

	limMtb->MaxLamp = m_dat.m_lamps.m_LampMax[LAMP_HALOGENA];
	limMtb->MinLamp = m_dat.m_lamps.m_LampMin[LAMP_HALOGENA];

#if !defined (__BORLANDC__)
	return  (char *) LPCTSTR(m_dat.m_lamps.m_LampName[LAMP_HALOGENA]);
#else
	return  (char *) LPCTSTR(m_dat.m_lamps.m_LampName[LAMP_HALOGENA].c_str());
#endif
}






/***************************  mspSetRevolver  *******************************
	Función para fijar la posición del revolver.
	Version OBSOLETA mantenida por compatibilidad. No gestiona espera.
*****************************************************************************/
int  mspSetRevolver_zeiss(int val)
{
	UINT val_act;

	if ( ! m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION] ) {
		error_leve("mspSetRevolver", "Revolver inexistente");
		return -1;
	}

	// Antes de nada compruebo que la posicion deseada no sea la actual
	m_dat.m_revolvers.ReadRevolverPos(FILTRO_EXCITACION, &val_act);
	if (int(val_act) == val)
		return 0;

	m_dat.m_revolvers.SetRevolverPos(FILTRO_EXCITACION, val);	// Desplazamiento a la posición deseada

	return 0;
}

/***************************  mspSetRevolver  *******************************
	Función para fijar la posición del revolver
	bEspera - si 'true', se espera a que termine de cambiar el revolver
	          si 'false', se espera unicamente a que el revolver comience a cambiar, despues
			  se devuelve inmediatamente la ejecucion.
*****************************************************************************/
int  mspSetRevolver_zeiss( int val,bool bEspera)
{
	UINT val_act;
	char *fn = "mspSetRevolver";

	if ( ! m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION] ) {
		error_leve("mspSetRevolver", "Revolver inexistente");
		return -1;
	}

	// Antes de nada compruebo que la posicion deseada no sea la actual
	m_dat.m_revolvers.ReadRevolverPos(FILTRO_EXCITACION, &val_act);
#ifdef _DEBUG
	char mensaje[LONGITUD_TEXTO];
	sprintf(mensaje, "%d -> %d  Debe esperar:%d", val_act,val,int(bEspera));
	sendlog(fn, mensaje);
#endif
	if (int(val_act) == val)
	{
		//Puede ser que ya se hubiese mandado a esta posicon (y por tanto es la misma que la de ahora), 
		//pero aun se este moviendo, en cuyo caso y si bEspera == true, hay que esperar a que termine
		if (bEspera)
			//Nos aseguramos que el revolver este parado
			if (mspEsperaRevolverFin_zeiss() != 0)
			{
				error_leve(fn, "No se puede consultar el estado");
				return -1;
			}

		return 0;
	}

	m_dat.m_revolvers.SetRevolverPos(FILTRO_EXCITACION, val);	// Desplazamiento a la posición deseada

	//Esperamos a que el revolver empiece a moverse
	if (mspEsperaRevolverInicio() != 0)
	{
		error_leve(fn, "No se puede consultar el estado");
		return -1;
	}

	if (bEspera == true)
	{
		//Esperamos a que el revolver termine de moverse
		if (mspEsperaRevolverFin_zeiss() != 0)
		{
			error_leve(fn, "No se puede consultar el estado");
			return -1;
		}
	}

	return 0;
}

/***************************  mspGetRevolver  *******************************
	Función para leer la posición de actual del revolver.
*****************************************************************************/
#if !defined (__BORLANDC__)
int mspGetRevolver_zeiss(CString & nomFiltro)
#else
int mspGetRevolver_zeiss(AnsiString & nomFiltro)
#endif
{
	unsigned int  val;

	if ( ! m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION] ) {
		error_leve("mspGetRevolver", "Revolver inexistente");
		return -1;
	}

	// Leo la posición actual: posición numérica.
	m_dat.m_revolvers.ReadRevolverPos(FILTRO_EXCITACION, &val);

	// Leo el nombre [posición numérica] de la posición actual.
	m_dat.m_revolvers.GetRevolverPosName(FILTRO_EXCITACION, val, nomFiltro);

	return val;
}

/***************************  mspEsperaRevolverFin  *******************************
	Espera activa hasta que el revolver deja de moverse o error
	devuelve 0 cuando el revolver esta parado y activo
	devuelve -1 en caso de error
*****************************************************************************/
int mspEsperaRevolverFin_zeiss()
{
	char *fn = "mspEsperaRevolverFin";
#ifdef _DEBUG
	int nCount = 0;
#endif

	if ( ! m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION] ) {
		error_leve(fn, "Revolver inexistente");
		return -1;
	}

	// Espera activa
	while (m_dat.m_revolvers.GetRevStat(FILTRO_EXCITACION) == _E_HARDW_BUSY)
	{
		//Espera
		Sleep(10);
#ifdef _DEBUG
		nCount++;
#endif
	}

#ifdef _DEBUG
	char mensaje[LONGITUD_TEXTO];
	sprintf(mensaje, "%d milisegundos", nCount*10);
	sendlog(fn, mensaje);
#endif

	if (m_dat.m_revolvers.GetRevStat(FILTRO_EXCITACION) == _E_NO_ERROR)
		return 0;
	else
		return -1;
}

/***************************  mspEsperaRevolverInicio  *******************************
	Espera activa hasta que el revolver empieza a moverse o error
	devuelve 0 cuando el revolver este en movimiento
	devuelve -1 en caso de error
	NOTA: Para utilizarse al mover el revolver sin espera, de tal manera que nos aseguremos que 
	podemos comprobar cuando ha terminado el movimiento
	AUXILIAR
*****************************************************************************/
int mspEsperaRevolverInicio()
{
	char *fn = "mspEsperaRevolverInicio";
#ifdef _DEBUG
	int nCount = 0;
#endif
	if ( ! m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION] ) {
		error_leve(fn, "Revolver inexistente");
		return -1;
	}

	// Espera activa
	while (m_dat.m_revolvers.GetRevStat(FILTRO_EXCITACION) == _E_NO_ERROR)
	{
		//Espera
		Sleep(10);
#ifdef _DEBUG
		nCount++;
#endif
	}

#ifdef _DEBUG
	char mensaje[LONGITUD_TEXTO];
	sprintf(mensaje, "%d milisegundos", nCount*10);
	sendlog(fn, mensaje);
#endif
	if (m_dat.m_revolvers.GetRevStat(FILTRO_EXCITACION) == _E_HARDW_BUSY)
		return 0;
	else
		return -1;
}

/***************************  mspRevolverInfo  ******************************
	Función para obtener información relativa a la rueda de filtros:
	  - Tipo de rueda (motorizado[2], manual[1], no disponible[0]
	  - Nombre de la rueda de filtros.
	  - Número máximo de posiciones de la rueda
		(almacenada en variable global).
	Devuelve el nombre de la rueda de filtros.
*****************************************************************************/
char  *mspRevolverInfo(limitesMtb *limMtb)
{
	int i;

	if ( ! m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION] ) {
		error_leve("mspRevolverInfo", "Revolver inexistente");
		return NULL;
	}

	/*	Solicito información relativa a la rueda solicitada. */

//	int type = m_dat.m_revolvers.m_RevolverType[FILTRO_EXCITACION];
//	CString name = m_dat.m_revolvers.m_RevolverName[FILTRO_EXCITACION];

	limMtb->MaxPosRevolver = m_dat.m_revolvers.m_RevolverNumberOfPositions[FILTRO_EXCITACION];

	for (i=1; i <= limMtb->MaxPosRevolver; i++)  {
		m_dat.m_revolvers.GetRevolverPosName(FILTRO_EXCITACION, i, limMtb->nomFiltro[i-1]);
	}


#if !defined (__BORLANDC__)
	return  (char *) LPCTSTR(m_dat.m_revolvers.m_RevolverName[FILTRO_EXCITACION]);
#else
	return  (char *) LPCTSTR(m_dat.m_revolvers.m_RevolverName[FILTRO_EXCITACION].c_str());
#endif
}
