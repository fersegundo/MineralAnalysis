/*****************************************************************************
	control_digitalizador.cpp		Módulo para la inicialización del
											digitalizador
	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002.
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <mil.h>

#if !defined (__BORLANDC__)
#include <conio.h>                // Para putch
#endif

#include "..\include\procesoAux.h"
#include "control_camara.h"

#include "control_digitalizador.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

extern limitesCam	limCam; // variable definida en la aplicacion llamante. Aqui se rellena 

parametros* paramAux; //puntero a los parametros de configuracion local a este fichero

//#define BASLER_101_F

/*************************** ini_control_digitalizador  *********************
	Función para reservar memoria para los diferentes formatos de
	digitalizadores que se utilizarán en la toma de imágenes.
	Previamente, se comprueba que los parámetros iniciales son válidos para
	el formato y tipo de digitalizador reservado.
	Seguidamente se reserva el display y los buffers que se utilizan en el
	proceso de adquisición de imágenes.
*****************************************************************************/
int ini_control_digitalizador(parametros *param)
{
	char	*fn = "ini_control_digitalizador - ";
	int		i;

	paramAux = param; //para usar en otras funciones

	//	SI DEFINO UN TAMAÑO DE DIGITALIZADOR DE ENFOQUE...	(DIGITALIZADOR)
	//	Reservo memoria para un nuevo display en el que se mostrarán los campos
	//	recorridos y los que se han podido enfocar. 
	if (param->Cam.formatoDig_e[0] != '\0' &&
		strcmp(param->Cam.formatoDig, param->Cam.formatoDig_e) )  
    {

		//	Configuro el digitalizador en formato enfoque
		if (M_digitalizador != M_NULL)
			libera_digitalizador();
		if ( MdigAlloc (M_sistema, M_DEV0, param->Cam.formatoDig_e, M_DEFAULT, &M_digitalizador)
			== M_NULL )  
        {
			error_fatal("ini_control_digitalizador", "Formato incorrecto", 0);
#if !defined (__BORLANDC__)
			putch('\a');
#endif
			return 1;
		}
		MdigInquire(M_digitalizador, M_SIZE_X, &limCam.anchoDig_e);
		if ( limCam.anchoDig_e > MAX_ANCHO_IMAGEN ||
				limCam.anchoDig_e < param->Cam.anchoImagen_e )  
        {
			error_fatal(fn, "Error en los parámetros iniciales", 0);
#if !defined (__BORLANDC__)
			putch('\a');
#endif
			return 1;
		}

		MdigInquire(M_digitalizador, M_SIZE_Y, &limCam.altoDig_e);
		if ( limCam.altoDig_e > MAX_ALTO_IMAGEN || 
                limCam.altoDig_e < param->Cam.altoImagen_e )  
        {
			error_fatal(fn, "Error en los parámetros iniciales", 0);
#if !defined (__BORLANDC__)
			putch('\a');
#endif
			return 1;
		}
	}


	//	Configuro el digitalizador en formato normal. 
	if (M_digitalizador != M_NULL)
		libera_digitalizador();

	if ( MdigAlloc (M_sistema, M_DEV0, param->Cam.formatoDig, M_DEFAULT, &M_digitalizador)
		== M_NULL )  {
		error_fatal("ini_control_proceso_imagenes", "Formato incorrecto", 0);
		return 1;
	}
	MdigInquire(M_digitalizador, M_SIZE_X, &limCam.anchoDig);
	if ( limCam.anchoDig > MAX_ANCHO_IMAGEN ||
			limCam.anchoDig < param->Cam.anchoImagen )  {
		error_fatal(fn, "Error en los parámetros iniciales", 0);
#if !defined (__BORLANDC__)
		putch('\a');
#endif
		return 1;
	}

	MdigInquire(M_digitalizador, M_SIZE_Y, &limCam.altoDig);
	if ( limCam.altoDig > MAX_ALTO_IMAGEN || 
            limCam.altoDig < param->Cam.altoImagen )  {
		error_fatal(fn, "Error en los parámetros iniciales", 0);
#if !defined (__BORLANDC__)
		putch('\a');
#endif
		return 1;
	}

	//	Si el formato del digitalizador de enfoque es el mismo que el de
	//	captura normal, copiamos las dimensiones porque no se leyeron. 
	if ( strcmp(param->Cam.formatoDig, param->Cam.formatoDig_e) == 0 )  {
		limCam.anchoDig_e = limCam.anchoDig;
		limCam.altoDig_e = limCam.altoDig;
	}

	//	Compruebo los valores iniciales de brillo, ganancia y exposición. 
	if ( info_digitalizador(M_digitalizador) )  {
		error_fatal(fn, "Error en los parámetros iniciales", 0);
		return 1;
	}

    // Se centra la camara en el caso de que se haya configurado un tamaño de toma diferente al maximo de la camara
	if ( configura_digitalizador(FORMATO_NORMAL) )  
        return 1;

	//	Buffer auxiliar de PROFUNDIDAD_ACUMULACION bits para acumular valores.
	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC+M_GRAB, &M_imagen_acum);
	if ( M_imagen_acum == M_NULL )  {
		error_fatal(fn, "M_imagen_acum", 0);
		return 1;
	}

    // Obtenemos la profundidad de la camara
	MdigInquire(M_digitalizador, M_SIZE_BIT , &param->Cam.profundidad);

    if ( (BITS_CAMARA > 8 && param->Cam.profundidad <= 8) ||
         (BITS_CAMARA <= 8 && param->Cam.profundidad > 8) )
    {
		error_fatal(fn, "BITS_CAMARA (DEFINE en codigo) y profundidad camara no coherentes", 0);
		return 1;
	}

    //Si la profundidad no es de 8bit, hay que configurar el display.
    //El display siempre es de 8 bit
    if (param->Cam.profundidad == 16)
    {
        MdispControl(M_display_normal,M_VIEW_MODE,M_BIT_SHIFT);
        MdispControl(M_display_normal,M_VIEW_BIT_SHIFT,4); //despreciamos los 4 bits menos significativos
		MdispLut(M_display_normal, M_PSEUDO); //esto es necesario para que la intensidad no sea anormalmente elevada
    }

	//	Reservamos los buffers donde se almacenarán y procesarán las imágenes. 
	//	M_imagen1 será el buffer que normalmente estará asociado al display_normal.

	// En modo VGA no permite grabar datos en el buffer.
    long nSD_size;
    MsysInquire(M_sistema,M_SYSTEM_DESCRIPTOR_SIZE,&nSD_size);
    char* systemDescriptor = new char[nSD_size];
    MsysInquire(M_sistema,M_SYSTEM_DESCRIPTOR,systemDescriptor);
	if (systemDescriptor == M_SYSTEM_VGA)
		MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
			M_IMAGE+M_DISP+M_PROC, &M_imagen1 );
	else
		MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
			M_IMAGE+M_DISP+M_GRAB+M_PROC, &M_imagen1 );
    delete [ ] systemDescriptor;
	if ( M_imagen1 == M_NULL)  {
		error_fatal(fn, "M_imagen1", 0);
		return -1;
	}
	if ( M_imagen1 != M_NULL )  
    {
	//	Seleccionamos el display para imagen normal, asociando el overlay. 
#if !defined (__BORLANDC__)
	    configura_overlay(M_display_normal, M_imagen1, &M_overlay_normal, 1);
#else
	    configura_overlay(M_display_normal, M_imagen1, HWindow, &M_overlay_normal, 1);
        borra_buffer(M_imagen1, NEGRO, 0, 0, NULL, NULL);
#endif
/*
*/
	    if (param->Cam.anchoImagen != param->Cam.anchoImagen_e    || 
            param->Cam.altoImagen != param->Cam.altoImagen_e)  
        {
            // Reservo un child buffer de M_imagen1 con el tamaño de enfoque.
            MbufChild2d(M_imagen1, (param->Cam.anchoImagen - param->Cam.anchoImagen_e) /2,
                (param->Cam.altoImagen- param->Cam.altoImagen_e) /2,
                param->Cam.anchoImagen_e, param->Cam.altoImagen_e, &M_centro_imagen);
            if (M_centro_imagen == M_NULL)  
            {
                error_fatal(fn, "M_centro_imagen", 0);
                return -1;
		    }
        }
        else
            M_centro_imagen = M_imagen1;
	}

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_imagen2 );
	if ( M_imagen2 == M_NULL )  {
		error_fatal(fn, "M_imagen2", 0);
		return -1;
	}

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_imagen3);
	if ( M_imagen3 == M_NULL )  {
		error_fatal(fn, "M_imagen3", 0);
		return -1;
	}

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_imagen4);
	if ( M_imagen4 == M_NULL )  {
		error_fatal(fn, "M_imagen4", 0);
		return -1;
	}

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_imagen5);
	if ( M_imagen5 == M_NULL )  {
		error_fatal(fn, "M_imagen5", 0);
		return -1;
	}

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_imagen6);
	if ( M_imagen6 == M_NULL )  {
		error_fatal(fn, "M_imagen6", 0);
		return -1;
	}

	//	Reservo memoria para los buffers utilizados en la adquisición
	//	de las bandas y en la correccion de la iluminación.
	for (i=0; i < param->nBandas; i++) {
		MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, param->Cam.profundidad+M_UNSIGNED,
			M_IMAGE+M_DISP+M_PROC, &M_banda[i]);
		MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, 32+M_FLOAT,
			M_IMAGE+M_PROC, &M_correc_denom[i]);
		MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, 32+M_FLOAT,
			M_IMAGE+M_PROC, &M_correc_numer[i]);

		if ( M_banda[i] == M_NULL ||
				M_correc_numer[i] == M_NULL || M_correc_denom[i] == M_NULL )  {
			error_fatal(fn, "M_banda", 0);
			return 1;
		}
	}
    //El resto de filtros no usados a NULL
	for (; i < MAX_FILTROS_DTA; i++)
		M_banda[i] = M_correc_numer[i] = M_correc_denom[i] = M_NULL;

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, 32+M_FLOAT,
		M_IMAGE+M_PROC, &M_correc_aux);
	if ( M_correc_aux == M_NULL )  {
		error_fatal(fn, "M_correc_aux", 0);
		return 1;
	}

	//	SI DEFINO UN TAMAÑO DE DIGITALIZADOR DE ENFOQUE...	(BUFFERS)
    //	Reservo memoria para los buffers empleados durante el enfoque mediante la funcion determina_contraste2
	if ( param->Cam.formatoDig_e[0] != '\0' )  {
		MbufAlloc2d(M_sistema, param->Cam.anchoImagen_e, param->Cam.altoImagen_e, param->Cam.profundidad+M_UNSIGNED,
			M_IMAGE+M_DISP+M_GRAB+M_PROC, &M_enfoque);
		if ( M_enfoque == M_NULL )  {
			error_fatal(fn, "M_enfoque", 0);
			return 1;
		}
	}

	return 0;
}


/*************************** fin_control_digitalizador  *********************
	Libero la memoria reservada para el control del digitalizador (adquisición
	de imágenes).
*****************************************************************************/
int fin_control_digitalizador()
{
	int i;

	// Libero los buffers asociados al digitalizador
	if (M_enfoque != M_NULL)				MbufFree(M_enfoque); // relacionado con determina_contraste2

	if (M_correc_aux != M_NULL)				MbufFree(M_correc_aux);
	for (i=0; i < MAX_FILTROS_DTA; i++)  {
		if (M_correc_numer[i] != M_NULL)	MbufFree(M_correc_numer[i]);
		if (M_correc_denom[i] != M_NULL)	MbufFree(M_correc_denom[i]);
		if (M_banda[i] != M_NULL)			MbufFree(M_banda[i]);
	}
	if (M_imagen_acum != M_NULL)			MbufFree(M_imagen_acum);

	// Libero el digitalizador seleccionado
	if (M_digitalizador != M_NULL)
		libera_digitalizador();

	return 0;
}


/*************************** configura_digitalizador *************************
	Función para configurar el digitalizador para el formato de imagen que
	se desea emplear en la adquisición.
*****************************************************************************/
int configura_digitalizador(int formato)
{
    static bool bConfigurado = false;

	//	Compruebo si los digitalizadores son iguales. Si no son iguales
	//	libero el digitalizador antiguo, para reservarlo según las nuevas
	//	especificaciones. 
	if ( strcmp(paramAux->Cam.formatoDig, paramAux->Cam.formatoDig_e) || !bConfigurado)  
    {

	//	Si el digitalizador esta reservado, libero la memoria reservada, para
	//		poder configurarlo. 
		if ( M_digitalizador != M_NULL )	libera_digitalizador();

		switch (formato) {
			case FORMATO_NORMAL:
				MdigAlloc (M_sistema, M_DEV0, paramAux->Cam.formatoDig, M_DEFAULT, &M_digitalizador);
				break;
			case FORMATO_ENFOQUE:
				MdigAlloc (M_sistema, M_DEV0, paramAux->Cam.formatoDig_e, M_DEFAULT, &M_digitalizador);
				break;
			default:
				error_fatal("configura_digitalizador", "Formato de digitalización inaceptable", 0);
				return 1;
		}

		// Fijamos los parámetros de adquisición, según los datos leidos del fichero ini. 
		MdigControl(M_digitalizador, M_BRIGHTNESS, paramAux->Cam.brillo);
		MdigControl(M_digitalizador, M_GAIN, paramAux->Cam.ganancia);
        ModificaExposicion(paramAux->Rueda.exposicion[paramAux->Rueda.posFiltro]);
        ModificaBaseExposicion(paramAux->Rueda.base_exp[paramAux->Rueda.posFiltro]);
	}

	if (paramAux->Cam.anchoImagen != paramAux->Cam.anchoImagen_e    || 
        paramAux->Cam.altoImagen != paramAux->Cam.altoImagen_e      || 
        !bConfigurado)
    {

	    //	Selecciono la configuración del digitalizador. 
	    MdigControl(M_digitalizador, M_SOURCE_OFFSET_X, 0); // Necesario para evitar salirnos del cuadro
	    MdigControl(M_digitalizador, M_SOURCE_OFFSET_Y, 0); // al modificar el tamaño de la captura
	    switch (formato) {
		    case FORMATO_NORMAL:
			    MdigControl(M_digitalizador, M_SOURCE_SIZE_X, paramAux->Cam.anchoImagen);
			    MdigControl(M_digitalizador, M_SOURCE_OFFSET_X, (limCam.anchoDig - paramAux->Cam.anchoImagen) / 2);

                MdigControl(M_digitalizador, M_SOURCE_SIZE_Y, paramAux->Cam.altoImagen);
			    MdigControl(M_digitalizador, M_SOURCE_OFFSET_Y, (limCam.altoDig - paramAux->Cam.altoImagen) / 2);

    //			ModificaExposicion(paramAux->Cam.exposicion[paramAux->Mtb.posFiltro]);
			    break;
		    case FORMATO_ENFOQUE:
			    MdigControl(M_digitalizador, M_SOURCE_SIZE_X, paramAux->Cam.anchoImagen_e);
			    MdigControl(M_digitalizador, M_SOURCE_OFFSET_X, (limCam.anchoDig_e - paramAux->Cam.anchoImagen_e) / 2);

			    MdigControl(M_digitalizador, M_SOURCE_SIZE_Y, paramAux->Cam.altoImagen_e);
			    MdigControl(M_digitalizador, M_SOURCE_OFFSET_Y, (limCam.altoDig_e - paramAux->Cam.altoImagen_e) / 2);

    //			ModificaExposicion(paramAux->Cam.exposicion[paramAux->Mtb.posFiltro]);
			    break;
	    }
    }

    // Nos aseguramos de que lo anterior sólo se ejecuta una vez si el formato y las dimensiones 
    // del digitalizador son iguales para adquisición normal y autoenfoque
    bConfigurado = true;

	return 0;
}


/*************************** info_digitalizador *****************************
	Función para solicitar información al digitalizador sobre los valores
	máximos y mínimos de los parámetros modificables del digitalizador, y
	comprobar que los valores que se desean fijar para la adquisición de
	imágenes está dentro de los valores permitidos.
*****************************************************************************/
int  info_digitalizador(MIL_ID digitalizador)
{
	char t[LONGITUD_TEXTO];

	MdigInquire(digitalizador, M_BRIGHTNESS + M_MAX_VALUE, &limCam.MaxBrillo);
	MdigInquire(digitalizador, M_BRIGHTNESS + M_MIN_VALUE, &limCam.MinBrillo);
	if (paramAux->Cam.brillo < limCam.MinBrillo || paramAux->Cam.brillo > limCam.MaxBrillo)  {
		sprintf(t, "Valor del parámetro de brillo inaceptable: [%5d, %5d] = %d",
			limCam.MinBrillo, limCam.MaxBrillo, paramAux->Cam.brillo);
		error_fatal("info_digitalizador", t, 0);
		Sleep(1000);
		return 1;
	}

	MdigInquire(digitalizador, M_GAIN+M_MAX_VALUE, &limCam.MaxGanancia);
	MdigInquire(digitalizador, M_GAIN+M_MIN_VALUE, &limCam.MinGanancia);
	if (paramAux->Cam.ganancia < limCam.MinGanancia || paramAux->Cam.ganancia > limCam.MaxGanancia)  {
		sprintf(t, "Valor del parámetro de ganancia inaceptable: [%5d, %5d] = %d",
			limCam.MinGanancia, limCam.MaxGanancia, paramAux->Cam.ganancia);
		error_fatal("info_digitalizador", t, 0);
		Sleep(1000);
		return 1;
	}

	MdigInquire(digitalizador, M_SHUTTER+M_MAX_VALUE, &limCam.MaxExposicion);
	MdigInquire(digitalizador, M_SHUTTER+M_MIN_VALUE, &limCam.MinExposicion);
    GetLimitesBaseExposicion(limCam.MinBaseExposicion,limCam.MaxBaseExposicion);
	//Debemos comprobar si es correcta la exposicion en todos los filtros
	for (int i=0; i<MAX_FILTROS_DTA;i++)
    {
		if (paramAux->Rueda.exposicion[i] < limCam.MinExposicion || paramAux->Rueda.exposicion[i] > limCam.MaxExposicion)  {
			sprintf(t, "Valor del parámetro de exposicion para la banda %d inaceptable: [%5d, %5d] = %d",
				i,limCam.MinExposicion, limCam.MaxExposicion, paramAux->Rueda.exposicion[i]);
			error_fatal("info_digitalizador", t, 0);
			Sleep(1000);
			return 1;
		}
		if (paramAux->Rueda.base_exp[i] < limCam.MinBaseExposicion || paramAux->Rueda.base_exp[i] > limCam.MaxBaseExposicion)  {
			sprintf(t, "Valor del parámetro de base de exposicion para la banda %d inaceptable: [%5d, %5d] = %d",
				i,limCam.MinExposicion, limCam.MaxExposicion, paramAux->Rueda.base_exp[i]);
			error_fatal("info_digitalizador", t, 0);
			Sleep(1000);
			return 1;
		}
    }


	return 0;
}


/*************************** libera_digitalizador ****************************
	Función para liberar el digitalizador previamente configurado.
*****************************************************************************/
int libera_digitalizador()
{
	if ( M_digitalizador == M_NULL )
		return 1;

	MdigFree(M_digitalizador);		// Libero el digitalizador.
	M_digitalizador = M_NULL;		// Pongo a M_NULL el valor del digitalizador

	return 0;
}

