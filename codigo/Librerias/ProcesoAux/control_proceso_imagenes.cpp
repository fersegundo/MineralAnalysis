/*****************************************************************************
	Proceso_imágenes.cpp		Módulo principal para la inicialización del
								proceso de imágenes.
								En este módulo se reserva la memoria necesaria
								para el proceso de imágenes.
	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002.
*****************************************************************************/

#include "stdafx.h"
#include "conio.h"
#include <math.h>

#if defined (__BORLANDC__)
    #include <vcl.h>
#else if
    #include <conio.h>                // Para putch

    #ifdef _DEBUG
    #define new DEBUG_NEW
    #undef THIS_FILE
    static char THIS_FILE[] = __FILE__;
    #endif
#endif

#include <mil.h>


#include "estructuras.h"
#include "control_proceso_imagenes.h"
#include "..\include\procesoAux.h"
#include "..\ControlDisp\control_camara.h"
#include "gestion_mensajes.h"


//	Declaración de variables del sistema
MIL_ID	M_aplicacion,			// Aplicación MIL.
		M_sistema,				// Sistema.
		M_display_normal,		// Display normal.
		M_display_resul,		// Display de resultados.
		M_digitalizador;		// Digitalizador.

//	Declaración de bufferes de trabajo
MIL_ID	M_imagen1,				// M_imagen: tamaño normal
		M_imagen2,
		M_imagen3,
		M_imagen4,
		M_imagen5,
		M_imagen6,
		M_enfoque,				// M_enfoque: tamaño enfoque
		M_centro_imagen,		// Buffer hijo de M_imagen1. (tamaño enfoque).
		M_resultados,			// M_resultados: buffer donde se almacenan los resultados.
		M_imagen_acum,
		M_banda[MAX_FILTROS_DTA],
		M_correc_denom[MAX_FILTROS_DTA],
		M_correc_numer[MAX_FILTROS_DTA],
		M_correc_aux;

//	Declaración del buffer asociado al buffer que muestro en el display
MIL_ID	M_overlay_normal;

//	Declaración de LUTs
MIL_ID	M_lut_overlay;

//	Declaración de histogramas
MIL_ID	M_histograma;
MIL_ID	M_histo_aux;

//	Versión de la librería de proceso de imágenes
double	M_version;


void configura_display_normal()
{
	//	Modifico la configuración del display: M_display_normal.
    //      - Fuerzo a que la ventana se ajuste a la imagen
	//		- Deshabilito el menú de tareas.
	//		- Deshabilito la barra del título.
	MdispControl(M_display_normal, M_WINDOW_INITIAL_POSITION_X, 0 );
	MdispControl(M_display_normal, M_WINDOW_INITIAL_POSITION_Y, 0 );
//    if (M_MIL_CURRENT_VERSION >= 8.0)
//	    MdispControl(M_display_normal, M_FULL_SCREEN, M_DISABLE);
#if !defined (__BORLANDC__)
	MdispControl(M_display_normal, M_WINDOW_MENU_BAR, M_DISABLE);
	MdispControl(M_display_normal, M_WINDOW_TITLE_BAR, M_DISABLE);
#else
    if (! HWindow) {
        MdispControl(M_display_normal, M_WINDOW_MENU_BAR, M_DISABLE);
        MdispControl(M_display_normal, M_WINDOW_TITLE_BAR, M_DISABLE);
    }
#endif
    if (M_MIL_CURRENT_VERSION >= 8.0)
    {
//	    MdispControl(M_display_normal, M_FULL_SCREEN, M_ENABLE);
//	    MdispControl(M_display_normal, M_FULL_SCREEN, M_DISABLE);
	    MdispControl(M_display_normal, M_WINDOW_RESIZE, M_DISABLE  );
    }

	//	Modifico la configuración del display: M_display_normal.
	//		- Deshabilito el menú de tareas.
	//		- Deshabilito la barra del título.

	//	Fijo la posición de la ventana en 0,0. 
	MdispControl(M_display_normal, M_WINDOW_INITIAL_POSITION_X, 0 );
	MdispControl(M_display_normal, M_WINDOW_INITIAL_POSITION_Y, 0 );
//	MdispControl(M_display_normal, M_WINDOW_MOVE, M_DISABLE);
}

/*	Índices de la ecuación empleada en la generación de la LUT asociada al display. */

#define  LINEALIDAD		32

/*	Defino los arrays para generar la LUT de falso color. */

static unsigned char color_ovl_prim[NUM_COL_OVR_PRIM][3] = {
/* R     G     B */
   {0,    0,    0},	// NEGRO
 {255,    0,    0},	// ROJO
   {0,  255,    0},	// VERDE
   {0,    0,  255},	// AZUL
 {255,  255,    0},	// AMARILLO
 {255,    0,  255},	// MAGENTA
   {0,  255,  255},	// CYAN
 {255,  255,  255},	// BLANCO
   {0,    0,    0},	// TRANSPARENTE
 {128,  128,  128}  // GRIS
};

static unsigned char color_ovl_sec[NUM_COL_OVR_SEC][3] = {
  {64,   64,   64}, // GRIS_OSC
 {128,    0,    0},	// ROJO_OSC
   {0,  128,    0},	// VERDE_OSC
   {0,    0,  128},	// AZUL_OSC
 {128,  128,    0},	// AMARILLO_OSC
 {128,    0,  128},	// MAGENTA_OSC
   {0,  128,  128},	// CYAN_OSC
 {224,  224,  224},	// BLANCO_OSC
   {0,    0,    0},	// COL_USUARIO
 {192,  192,  192}	// GRIS_CLARO
};

/*************************** ini_control_proceso_imagenes ********************
	Función para la inicialización de la aplicación de proceso de imágenes
	mediante las librerías MIL de MATROX.
*****************************************************************************/
#if !defined (__BORLANDC__)
int  ini_control_proceso_imagenes(parametros *param, int sistema, int gamma)
#else
int  ini_control_proceso_imagenes(parametros *param, int sistema, HWND HWindow,
                                  int gamma)
#endif
{
	char	*fn = "ini_control_proceso_imagenes - error al reservar memoria";

	//	RESERVAMOS MEMORIA PARA LA APLICACIÓN. 

	//	Inicializamos el sistema de adquisición y proceso de imágenes. 
	MappAlloc(M_DEFAULT, &M_aplicacion);	// Selecciono la aplicación MIL.
	if ( M_aplicacion == M_NULL ) {
		error_fatal(fn, "M_aplicacion", 1);
		return -1;
	}

	//	Pregunto al sistema la versión de la librería de control de proceso de imagen
	MappInquire(M_VERSION, &M_version);

	//	Selecciono el sistema hardware.
	if (sistema)
		MsysAlloc(M_SYSTEM_VGA, M_DEV0, M_DEFAULT, &M_sistema);
	else
		MsysAlloc(M_SYSTEM_1394, M_DEV0, M_DEFAULT, &M_sistema);	// Selecciono un sistema hardware

	if ( M_sistema == M_NULL )  {
		error_fatal(fn, "M_sistema", 0);
		return -1;
	}
	//	Reservamos memoria para los diferentes DISPLAYS a través
	//de los cuales se mostrarán las imágenes. 
	//	Display normal para las imágenes adquiridas y procesadas. 
    if (M_MIL_CURRENT_VERSION >= 8.0)
	    MdispAlloc(M_sistema, M_DEFAULT, "M_DEFAULT", M_WINDOWED, &M_display_normal);//M_DISPLAY_SETUP
    else
	    MdispAlloc(M_sistema, M_DEV0, "M_DEFAULT", M_WINDOWED + M_OVR, &M_display_normal);//M_DISPLAY_SETUP
	if ( M_display_normal == M_NULL )  {
		error_fatal(fn, "M_display_normal", 0);
		return -1;
	}



    //	Display para los resultados del análisis. 
    if (M_MIL_CURRENT_VERSION >= 8.0)
	    MdispAlloc(M_sistema, M_DEFAULT, M_DISPLAY_SETUP, M_WINDOWED, &M_display_resul);
    else
	    MdispAlloc(M_sistema, M_DEV1, M_DISPLAY_SETUP, M_WINDOWED + M_OVR, &M_display_resul);
	if ( M_display_resul == M_NULL )  {
		error_fatal(fn, "M_display_resul", 0);
		return -1;
	}
	//	Modifico la configuración del display de resultados: M_display_resul.
	//		- Deshabilito el menú de tareas.
	//		- Título del display: "Display de resultados". 
	MdispControl(M_display_resul, M_WINDOW_MENU_BAR, M_DISABLE);
    if (M_MIL_CURRENT_VERSION >= 8.0)
	    MdispControl(M_display_resul, M_TITLE, M_PTR_TO_DOUBLE("RESULTADOS"));
    else
	    MdispControl(M_display_resul, M_WINDOW_TITLE_NAME, (long)"RESULTADOS");


    
    //	Creamos las LUTs que se van a utilizar en los procesos. 
	MbufAllocColor(M_sistema, 3L, 256L, 1L, 8L+M_UNSIGNED, M_LUT, &M_lut_overlay);
	if ( M_lut_overlay == M_NULL)  {
		error_fatal(fn, "M_lut_overlay", 0);
		return -1;
	}
	crea_LUT_overlay(M_lut_overlay, gamma);	// Genera LUT color y corrección de gamma

	// Reservo memoria para el buffer de resultados. 
	MbufAlloc2d(M_sistema, ANCHO_IMAGEN_R, ALTO_IMAGEN_R, 8+M_UNSIGNED,
		M_IMAGE+M_DISP+M_PROC, &M_resultados);
	if ( M_resultados == M_NULL )  {
		error_fatal(fn, "M_resultados", 0);
		return -1;
	}
	MbufControl(M_resultados, M_ASSOCIATED_LUT, M_lut_overlay);

	//	Reservo memoria para el cálculo de histogramas. 
	MimAllocResult(M_sistema, NUMVAL, M_HIST_LIST, &M_histograma);
	if (M_histograma == M_NULL)  {
		error_fatal(fn, "M_histograma", 0);
		return -1;
	}

	MimAllocResult(M_sistema, NUMVAL, M_HIST_LIST, &M_histo_aux);
	if (M_histo_aux == M_NULL)  {
		error_fatal(fn, "M_histo_aux", 0);
		return -1;
	}

	//	Seleccionamos el display de resultados: M_display_resul,donde se muestran
	//	la serie de histogramas que se obtienen. 

//	MdispLut(M_display_resul, M_lut_overlay);
#if !defined (__BORLANDC__)
	MdispSelect(M_display_resul, M_resultados);
#endif

	// Creamos el vector de coeficientes para el filtrado gaussiano. 
	calcula_coef_filtro_histograma();

    return 0;
}

/*************************** ini_control_proceso_imagenes_lite ********************
	Función para la inicialización de la aplicación de proceso de imágenes
	mediante las librerías MIL de MATROX.
    VERSION PARA TOMA_AUTO CON SOLO LO NECESARIO
*****************************************************************************/
int  ini_control_proceso_imagenes_lite(parametros *param)
{
	char	*fn = "ini_control_proceso_imagenes_lite - error al reservar memoria";

	//	RESERVAMOS MEMORIA PARA LA APLICACIÓN. 

	//	Inicializamos el sistema de adquisición y proceso de imágenes. 
	MappAlloc(M_DEFAULT, &M_aplicacion);	// Selecciono la aplicación MIL.
	if ( M_aplicacion == M_NULL ) {
		error_fatal(fn, "M_aplicacion", 1);
		return -1;
	}

	//	Selecciono el sistema hardware.
	MsysAlloc(M_SYSTEM_1394, M_DEV0, M_DEFAULT, &M_sistema);	// Selecciono un sistema hardware

	if ( M_sistema == M_NULL )  {
		error_fatal(fn, "M_sistema", 0);
		return -1;
	}
	//	Reservamos memoria para los diferentes DISPLAYS a través
	//de los cuales se mostrarán las imágenes. 
	//	Display normal para las imágenes adquiridas y procesadas. 
    if (M_MIL_CURRENT_VERSION >= 8.0)
	    MdispAlloc(M_sistema, M_DEFAULT, "M_DEFAULT", M_WINDOWED, &M_display_normal);//M_DISPLAY_SETUP
    else
	    MdispAlloc(M_sistema, M_DEV0, "M_DEFAULT", M_WINDOWED + M_OVR, &M_display_normal);//M_DISPLAY_SETUP
	if ( M_display_normal == M_NULL )  {
		error_fatal(fn, "M_display_normal", 0);
		return -1;
	}

/*    
	//	Reservo memoria para el cálculo de histogramas. 
	MimAllocResult(M_sistema, NUMVAL, M_HIST_LIST, &M_histograma);
	if (M_histograma == M_NULL)  {
		error_fatal(fn, "M_histograma", 0);
		return -1;
	}

	MimAllocResult(M_sistema, NUMVAL, M_HIST_LIST, &M_histo_aux);
	if (M_histo_aux == M_NULL)  {
		error_fatal(fn, "M_histo_aux", 0);
		return -1;
	}
	// Creamos el vector de coeficientes para el filtrado gaussiano. 
	calcula_coef_filtro_histograma();
*/

    return 0;
}


/*************************** fin_control_proceso_imagenes ********************
	Función para la finalización de la aplicación de proceso de imágenes
	mediante las librerías MIL de MATROX, liberando los recursos reservados.
*****************************************************************************/
int fin_control_proceso_imagenes()
{
	//	Libero la memoria reservada para el cálculo de histogramas
	if (M_histo_aux != M_NULL)  {
		MimFree(M_histo_aux);
		M_histo_aux = M_NULL;
	}
	if (M_histograma != M_NULL)  {
		MimFree(M_histograma);
		M_histograma = M_NULL;
	}

	//	Libero las LUTs reservadas
	if (M_lut_overlay != M_NULL)  {
		MbufFree(M_lut_overlay);
		M_lut_overlay = M_NULL;
	}

	// Libero los bufferes asociados a la aplicación
	if (M_resultados != M_NULL)  {
		MbufControl(M_resultados, M_ASSOCIATED_LUT, M_DEFAULT);
		MbufFree (M_resultados);
		M_resultados = M_NULL;
	}

	if (M_imagen6 != M_NULL)  {
		MbufFree(M_imagen6);
		M_imagen6 = M_NULL;
	}
	if (M_imagen5 != M_NULL)  {
		MbufFree(M_imagen5);
		M_imagen5 = M_NULL;
	}
	if (M_imagen4 != M_NULL)  {
		MbufFree(M_imagen4);
		M_imagen4 = M_NULL;
	}
	if (M_imagen3 != M_NULL)  {
		MbufFree(M_imagen3);
		M_imagen3 = M_NULL;
	}
	if (M_imagen2 != M_NULL)  {
		MbufFree(M_imagen2);
		M_imagen2 = M_NULL;
	}
	if (M_centro_imagen != M_NULL && M_centro_imagen != M_imagen1)  {
		MbufFree(M_centro_imagen);
		M_centro_imagen = M_NULL;
	}
	if (M_imagen1 != M_NULL)  {
		MbufFree (M_imagen1);
		M_imagen1 = M_NULL;
	}

	// Libero los displays asociados al sistema
	if (M_display_resul != M_NULL)  {
		MdispFree (M_display_resul);
		M_display_resul = M_NULL;
	}
	if (M_display_normal != M_NULL)	 {
		MdispFree (M_display_normal);
		M_display_normal = M_NULL;
	}

	// Libero el sistema
	if (M_sistema != M_NULL)  {
		MsysFree (M_sistema);
		M_sistema = M_NULL;
	}

	// Libero la aplicación MIL
	if (M_aplicacion != M_NULL)  {
		MappFree (M_aplicacion);
		M_aplicacion = M_NULL;
	}

	return 0;
}

/***************************  crea_LUT_gamma  *****************************
	Función para generar una lut para corregir la gamma del monitor.
	Ecuación que se ha seguido:
		Y = a*log10(X+LINEALIDAD) + c;
*****************************************************************************/
void crea_LUT_gamma(MIL_ID M_lut)
{
	BYTE  v[256][3];
	double a, c;
	int  i;

	a = 255.0/(log10(255 + LINEALIDAD) - log10(LINEALIDAD));
	c = a * log10(LINEALIDAD);
	for (i=0; i < 256; i++)
		v[i][0] = v[i][1] = v[i][2] = (BYTE) ( a*log10(i + LINEALIDAD) - c );

	MbufPutColor2d(M_lut, M_RGB24+M_PACKED, M_ALL_BAND, 0, 0,
		256, 1, &v);
}


/*************************** crea_LUT_overlay ********************************
	Función que crea una lut de pseudo-colores, para mostrar en el overlay.
	Para ello se utiliza dos lut:
		- color_ovl_prim
		- color_ovl_sec
	Generamos una rampa en la que re-mapeamos la imagen del buffer sobre el
	overlay, dejando los 10 primeros y últimos niveles para definir la gama
	de colores que deseemos.
	NOTA: en esta función no reservamos memoria para la LUT.
*****************************************************************************/
void crea_LUT_overlay(MIL_ID M_lut, int gamma)
{
	/*	Genero una LUT de rampa para re-mapear la imagen. */

	if (gamma)
		crea_LUT_gamma(M_lut);
	else
		MgenLutRamp(M_lut, 0, 0., 255, 255.);

	/*	Modifico los índices del principio y del final para asignar los
		colores de overlay primarios y secundarios. El sistema asigna
		por defecto el índice 8 al color transparente, y hemos definido
		la constante TRANSPARENTE para mantener esa asignación. */

	MbufPutColor2d(M_lut, M_RGB24+M_PACKED, M_ALL_BAND, 0, 0,
		NUM_COL_OVR_PRIM, 1, &color_ovl_prim);
	MbufPutColor2d(M_lut, M_RGB24+M_PACKED, M_ALL_BAND, 256-NUM_COL_OVR_SEC, 0,
		NUM_COL_OVR_SEC, 1, &color_ovl_sec);
}

/*************************** configura_overlay ******************************
	Función para seleccionar un display y asociar un overlay asociado
*****************************************************************************/
#if !defined (__BORLANDC__)
int  configura_overlay(MIL_ID display, MIL_ID buffer, MIL_ID *overlay, int val)
#else
int  configura_overlay(MIL_ID display, MIL_ID buffer, HWND HWindow,
                       MIL_ID *overlay, int val)
#endif
{
#if defined (__BORLANDC__)
  if (HWindow)
    MdispSelectWindow(display, buffer, HWindow);
  else
#endif
    ASSERT(buffer!=M_NULL);
    if (buffer != M_NULL)
	    MdispSelect(display, buffer); //necesario para habilitar overlay
	if (val)  {
        /*
		MdispControl(display, M_WINDOW_OVR_SHOW, M_DISABLE);
		MdispControl(display, M_WINDOW_OVR_WRITE, M_ENABLE);
		MdispInquire(display, M_WINDOW_OVR_BUF_ID, overlay);
		MdispControl(display, M_WINDOW_OVR_SHOW, M_ENABLE);
*/
//		MdispControl(display, M_WINDOW_OVR_SHOW, M_DISABLE);
		MdispControl(display, M_OVERLAY, M_ENABLE); //MIL 8
		MdispInquire(display, M_OVERLAY_ID, overlay); //MIL 8
//		MdispControl(display, M_WINDOW_OVR_SHOW, M_ENABLE);

		//	Asocio la LUT de pseudo-color al display de overlay
//		MdispControl(display, M_WINDOW_OVR_LUT, M_lut_overlay); //MIL 7
		MdispOverlayKey(display, M_KEY_ON_COLOR, M_EQUAL, 0xff, TRANSPARENTE);

		// El texto tiene color de fondo transparente
		MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

		// Relleno el buffer overlay con el color TRANSPARENTE
		//MbufClear(*overlay, TRANSPARENTE); // MIL 7
        MdispControl(display, M_OVERLAY_CLEAR, TRANSPARENTE); //MIL 8
    }
	else
		MdispControl(display, M_WINDOW_OVR_SHOW, M_DISABLE );
    if (buffer != M_NULL)
	    MdispDeselect(display, buffer); 

	return 0;
}

