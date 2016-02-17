/*****************************************************************************
	control_camara.cpp		Rutinas para control de la cámara de video

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Septiembre 2001
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <mil.h>

#include "control_camara.h"

#include "..\include\procesoAux.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

#define QUADLET            MIL_INT32 //Para ModificaBaseExposicion
static QUADLET      Address = 0;     //Para ModificaBaseExposicion

static bool tomaContinua;
extern parametros	paramIni;				// Parámetros iniciales

static QUADLET LeeAddressTimeBase(); //auxiliar


/*************************** toma2_unica *************************************
	Función para la adquisición de una sola imagen.
	Modo normal.
*****************************************************************************/
void  toma2_unica(MIL_ID M_fb)
{
    if (M_fb!=M_NULL)
    {
        fin_toma();

        MdigGrab(M_digitalizador, M_fb);
    }
}


/*************************** toma2_continua **********************************
	Función para la adquisición de imágenes en modo continuado.
	Modo enfoque.
*****************************************************************************/
void toma2_continua(MIL_ID M_fb)
{
    if (M_fb!=M_NULL)
    {
	    if (! tomaContinua)
		    MdigGrabContinuous (M_digitalizador, M_fb);
	    tomaContinua = true;
    }
}


/*************************** fin_toma ****************************************
	Función para detener el sistema de adquisición continua.
	NOTA: usar cuando se desee parar el proceso de toma2_continua.
*****************************************************************************/
void fin_toma()
{
	if (tomaContinua)
		MdigHalt(M_digitalizador);
	tomaContinua = false;
}


/*************************** toma2_acumulada *********************************
	Función para la adquisición de una imagen con menor ruido, mediante la
	acumulacion de un cierto número de tomas 

    buf - buffer donde se devolvera la imagen acumulada
    nAcum - numero de veces a acumular

	La imagen acumulada es devuelta en M_imagen1
*****************************************************************************/
void  toma2_acumulada(MIL_ID buf, int nAcum)
{
/*
    fin_toma(); //	tomaContinua = false;

    if (nAcum == 1) //optimizado
    {
	    MdigGrab (M_digitalizador, buf);
    }
    else
    {
	    MbufClear(buf, nAcum / 2); //	para conseguir que en la división final se produzca un redondeo.
	    int n;
	    for (n = 0; n < nAcum; ++n) 
        {
		        MdigGrab (M_digitalizador, M_imagen1);
		        MimArith (M_imagen1, buf, buf, M_ADD);
	    }
    }
*/
	//	para conseguir que en la división final se produzca un redondeo. 
    if (nAcum != 1)
	    MbufClear(buf, nAcum / 2);
    else
	    MbufClear(buf, 0);

    fin_toma(); //	tomaContinua = false;

	int n;
	for (n = 0; n < nAcum; ++n) {
		MdigGrab (M_digitalizador, M_imagen1);
		MimArith (M_imagen1, buf, buf, M_ADD);
	}
}

/*************************** toma2_promediada *********************************
	Función para la adquisición de una imagen con menor ruido, mediante la
	acumulacion de un cierto número de tomas 
	Modo del digitalizador: enfoque.

    nAcum - numero de veces a acumular

  La imagen acumulada es promediada y devuelta en M_imagen1 
*****************************************************************************/
void  toma2_promediada(int nAcum)
{

    toma2_acumulada(M_imagen_acum, nAcum);

	//	Una vez realizada la acumulación de imágenes en el buffer auxiliar,
	//	procedo a dividirlo por el número de imágenes tomadas.
	//	La imagen corregida es devuelta y almacenada en M_imagen1.
    if (nAcum != 1)
	    MimArith(M_imagen_acum, nAcum, M_imagen1, M_DIV_CONST);
}

/*****************************************************************************
	Modifica la exposicion
*****************************************************************************/
void  ModificaExposicion(int exposicion)
{
	MdigControl(M_digitalizador, M_SHUTTER,exposicion);
}

/*****************************************************************************
	Modifica la exposicion
*****************************************************************************/
void  ModificaExposicionSleep(int exposicion)
{
	ModificaExposicion(exposicion);
	Sleep(TIEMPO_AJUSTE_EXPOSICION); //hay que dar tiempo a que se estabilice la exposicion
}

//Reads a 1394 register
QUADLET ReadRegister(MIL_ID M_sistema, long DevNumber, QUADLET Offset)
{
    QUADLET Value = 0;

    MsysConfigAccess( M_sistema,
                     M_NULL, 
                     M_NULL, 
                     M_DEV0, 
                     M_1394_CONFIGURATION_SPACE, 
                     M_READ+M_BIG_ENDIAN, 
                     Offset, 
                     sizeof(Value), 
                     &Value);
    return Value;
}

//Writes to a 1394 register
void WriteRegister(MIL_ID M_sistema, long DevNumber, QUADLET Offset, QUADLET Value)
{
    MsysConfigAccess( M_sistema,
                     M_NULL, 
                     M_NULL, 
                     M_DEV0, 
                     M_1394_CONFIGURATION_SPACE, 
                     M_WRITE+M_BIG_ENDIAN, 
                     Offset, 
                     sizeof(Value), 
                     &Value);
}

/*****************************************************************************
	Modifica la base de exposicion
    nBaseExposicion - en microsegundos
    Devuelve true si todo bien
*****************************************************************************/
bool  ModificaBaseExposicion(int nBaseExposicion)
{
    if (Address == 0) //Primera vez que se accede al registro avanzado de la base de exposicion
    {
        Address = LeeAddressTimeBase();

        if (Address == 0)
            return false;

        // Leer cabecera (COMPROBACION)
        if (ReadRegister(M_sistema, M_DEV0, Address + 0) == 0)
            return false;
    }

    // Escribir shutter time base
    float fShutterTimeBase = (float)nBaseExposicion/(float)1000000;
    QUADLET qTimeBase = *((QUADLET *)&fShutterTimeBase);
    WriteRegister(M_sistema, M_DEV0, Address + 16, qTimeBase);

    // Leer shutter time base (COMPROBACION)
    QUADLET Value = ReadRegister(M_sistema, M_DEV0, Address + 16);
    float base = *((float *)&Value);
    base = base * 1000000;
    ASSERT((int)base == nBaseExposicion);

    return true;
}


/*****************************************************************************
	Obtiene el valor minimo y maximo (en microsegundos) de la base de exposicion
    Devuelve false si error
*****************************************************************************/
bool  GetLimitesBaseExposicion(long& nMin, long&nMax)
{

    if (Address == 0) //Primera vez que se accede al registro avanzado de la base de exposicion
    {
        Address = LeeAddressTimeBase();

        if (Address == 0)
            return false;

        // Leer cabecera (COMPROBACION)
        if (ReadRegister(M_sistema, M_DEV0, Address + 0) == 0)
            return false;
    }

    // Leer Min
    QUADLET Value = ReadRegister(M_sistema, M_DEV0, Address + 8);
    float min = *((float *)&Value);
    // Leer Max
    Value = ReadRegister(M_sistema, M_DEV0, Address + 12);
    float max = *((float *)&Value);

    nMin = (long)(min * 1000000); 
    nMax = (long)(max * 1000000); 

    return true;
}

// devuelve 0 si no es posible acceder a la caracteristica avanzada de base de exposicion
QUADLET LeeAddressTimeBase()
{
    const int nACR = 0xF2F00000; //direccion del ACR (Access Control Register) (para todos los features)

    // Set feature set ID to unlock advanced features. NO ES NECESARIO, SIEMPRE HABILITADO EN CAMARAS BASLER
    QUADLET
      Enabled1,
      Enabled2;
    WriteRegister(M_sistema, M_DEV0, nACR + 0x00, 0x0030533B);
    WriteRegister(M_sistema, M_DEV0, nACR + 0x04, 0x73C3F000);//last three zeros represent a timeout value, 000 means always available
    Enabled1 = ReadRegister(M_sistema, M_DEV0, nACR + 0x00);
    Enabled2 = ReadRegister(M_sistema, M_DEV0, nACR + 0x04);

    if (Enabled1 == 0xFFFFFFFF && Enabled2 == 0xFFFFFFFF)
        return false;

    // Recuperamos la direccion CSR (Control and Status Register) para el shutter time base feature

        // Escribimos al Smart Features Inquiry Register (que esta 0x10 despues respecto al ACR)
        // El Inquiry Register esta ordenado asi: D1 | D3 | D2 | D4[3] | D4[2] | D4[1] | D4[0] | D4[7] | D4[6] | D4[5] | D4[4]
        // El CSR GUID para el shutter time base es D1:648BE1DA   D2:A416   D3:11D8   D4:9B47 00105A5BAE55
        WriteRegister(M_sistema, M_DEV0, nACR + 0x10, 0x648BE1DA);
        WriteRegister(M_sistema, M_DEV0, nACR + 0x14, 0x11D8A416);
        WriteRegister(M_sistema, M_DEV0, nACR + 0x18, 0x1000479B);
        WriteRegister(M_sistema, M_DEV0, nACR + 0x1C, 0x55AE5B5A);
        // Al haber "preguntado" por nuestro CSR GUID, obtenemos la direccion CSR
        // la direccion estará en el Smart Features Addres Register (desplazado 0x20 del ACR
        return ReadRegister(M_sistema, M_DEV0, nACR + 0x20);
}