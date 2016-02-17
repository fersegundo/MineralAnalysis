/*****************************************************************************
 	Define las funciones utilizada para la determinación
	del contraste en el enfoque.

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002
*****************************************************************************/

#include "stdafx.h"


#include <mil.h>

#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#include "autoenfoque.h"
#include "..\ProcesoAux\estructuras.h"
#include "..\ProcesoAux\control_proceso_imagenes.h"
#include "..\ProcesoAux\control_proceso_imagenes.h"
#include "..\ControlDisp\control_digitalizador.h"
#include "..\ProcesoAux\funciones_graficas.h"
#include "..\ProcesoAux\gestion_mensajes.h"

#include "..\ControlDisp\control_camara.h"
#include "..\ControlMicroscopio\control_microscopio.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define  NUM_FILTROS_REVOLVER	8


extern parametros* paramAux; //puntero a los parametros de configuracion local a este fichero

/*  Declaración de VARIABLES GLOBALES empleadas en el manejo del
	microscopio para realizar el barrido de la muestra. */

int		contAjusteFocoLento = 0;
double  AjusteFocoLento = 0.0;


static double BANDAFOCOINI;	    // Semialtura banda de enfoque inicial
static double BANDAFOCOBAJO;	// Semialtura banda de enfoque final
static double BANDAFOCOALTO;    // Semialtura banda de enfoque final
static double BANDAFOCOMAX;		// Semialtura banda de enfoque máxima
static double BANDAFOCOFINAL;	// Semialtura banda de enfoque fino
static double ALTAVELZ;	        // Velocidad rapida (variable que se adaptará a cada objetivo)
static double BAJAVELZ;	        // Velocidad rapida (variable que se adaptará a cada objetivo)

static int  busca_foco(bool bFino, double zMax, double zMin, double foco[], double cota[],
				int *npuntos, double zPlano, int pasada, double velocidad, bool bMoviendose);


/**************************  ini_contraste  *************************
	Función que inicializa el contraste, para determinar el enfoque óptimo
	en la función autoenfoque (ver control_barrido.cpp).

	La zona para realizar el autoenfoque esta definido por el digitalizador,
	siendo este del tamaño del enfoque (ver control_proceso_imagenes.h).
	PROCESO:
		- Definimos el entorno de proceso para medida del contraste.
		- Creamos las LUT lógicas para medida de contraste.
		- Definimos los elementos estructurantes circulares.
		- Definimos un kernel para la convolución de suavizado de imagen.

	NOTA: (bll)
	Esta función se encargará de configurar el digitalizador en modo enfoque.
****************************************************************************/
int ini_contraste(HWND	pImagen)
{
	char  *fn = "ini_contraste";

    if (pImagen != NULL)
    {
        if ( strcmp(paramAux->Cam.formatoDig, paramAux->Cam.formatoDig_e) 
	         || paramAux->Cam.anchoImagen != paramAux->Cam.anchoImagen_e   
             || paramAux->Cam.altoImagen != paramAux->Cam.altoImagen_e         )
        {
	        //	Definimos el entorno de proceso para medida del contraste.
            MdispDeselect(M_display_normal, M_imagen1); //Necesario para que no falle toma2_continua en formato 7_0   
            fin_toma(); //necesario para que no falle configura_digitalizador en formato de 7_0
            if (configura_digitalizador(FORMATO_ENFOQUE))
            {
		        error_fatal("configura_digitalizador", "Formato incorrecto", 0);
                return 1;
	        }
	        if ( M_digitalizador == M_NULL )  
            {
		        error_fatal(fn, "No es posible reservar memoria para el digitalizador", 0);
		        fin_control_proceso_imagenes();
		        return 1;
	        }

            toma2_continua(M_centro_imagen); //necesario porque hemos finalizado la toma justo antes
            MdispSelectWindow(M_display_normal, M_imagen1, pImagen); // volvemos a seleccionar
            //configura_display_normal(); // al haber reseleccionado, para quitar el menú, centrar, etc
        }
    }
	return 0;
}


/**************************  fin_contraste  ********************************
	Una vez realizado el autoenfoque retornamos al modo de digitalizador
	normal, asociando al display normal el buffer correspondiente.
****************************************************************************/
int fin_contraste(HWND	pImagen)
{
        char  *fn = "fin_contraste";

    if (pImagen != NULL)
    {
        if ( strcmp(paramAux->Cam.formatoDig, paramAux->Cam.formatoDig_e) 
	         || paramAux->Cam.anchoImagen != paramAux->Cam.anchoImagen_e   
             || paramAux->Cam.altoImagen != paramAux->Cam.altoImagen_e         )
        {
            //	Configuramos el digitalizador en formato normal.
            MdispDeselect(M_display_normal, M_imagen1); //Necesario para que no falle toma2_continua en formato 7_0   
            fin_toma(); //necesario para que no falle congigura_digitalizador en formato de 7_0
            if (configura_digitalizador(FORMATO_NORMAL))
            {
	            error_fatal("configura_digitalizador", "Formato incorrecto", 0);
                return 1;
            }
            toma2_continua(M_imagen1); //necesario porque hemos finalizado la toma justo antes
            MdispSelectWindow(M_display_normal, M_imagen1, pImagen); // volvemos a seleccionar
            //configura_display_normal(); // al haber reseleccionado, para quitar el menú, centrar, etc
            if ( M_digitalizador == M_NULL )  
            {
	            error_fatal(fn, "No es posible reservar memoria para el digitalizador", 1);
	            return 1;
            }
        }
    }
	MbufClear(M_overlay_normal, TRANSPARENTE);

	return 0;
}


/**************************  autoenfoque  ***********************************
	Función para determinar el punto de enfoque óptimo del campo visual del
	microscopio mediante el estudio de una serie de tomas de imagen dentro
	de un cierto entorno de distancias a la preparación.
****************************************************************************/
int  autoenfoque(double zBase, double zPlano, double *zFoco, double *contraste, double dAumentos,
                 bool bMoviendose, HWND	pImagen)
{
	char  mensaje[512];
	int  i, num, prim, cod;
	unsigned int  peso1, peso2;
	double  zReal[MAXPASOS], foco[MAXPASOS], Cont, maxCont, medCont;
	double  sumaProductos, sumaPesos, zCG, mejorAjuste;

	MbufClear(M_overlay_normal, TRANSPARENTE);

    // ajustamos los rangos de z segun el objetivo actual
    if (dAumentos < 20.0 && dAumentos > 0.0)
    {
        BANDAFOCOINI    =  BANDAFOCOINI_20x   * 20 / dAumentos;	
        BANDAFOCOBAJO   =  BANDAFOCOBAJO_20x  * 20 / dAumentos;
        BANDAFOCOALTO   =  BANDAFOCOALTO_20x  * 20 / dAumentos;
        BANDAFOCOMAX    =  BANDAFOCOMAX_20x   * 20 / dAumentos;	
        BANDAFOCOFINAL  =  BANDAFOCOFINAL_20x * 20 / dAumentos;
        if (EsLeica())
        {
            ALTAVELZ        =  ALTAVELZ_LEICA_20x * 20 / dAumentos;
            BAJAVELZ        =  BAJAVELZ_LEICA_20x * 20 / dAumentos;
        }
        else
        {
            ALTAVELZ        =  ALTAVELZ_ZEISS_20x * 20 / dAumentos;
            BAJAVELZ        =  BAJAVELZ_ZEISS_20x * 20 / dAumentos;
        }
    }
    else
    {
        BANDAFOCOINI    =  BANDAFOCOINI_20x;	
        BANDAFOCOBAJO   =  BANDAFOCOBAJO_20x;
        BANDAFOCOALTO   =  BANDAFOCOALTO_20x;
        BANDAFOCOMAX    =  BANDAFOCOMAX_20x;	
        BANDAFOCOFINAL  =  BANDAFOCOFINAL_20x;
        if (EsLeica())
        {
            ALTAVELZ        =  ALTAVELZ_LEICA_20x;
            BAJAVELZ        =  BAJAVELZ_LEICA_20x;
        }
        else
        {
            ALTAVELZ        =  ALTAVELZ_ZEISS_20x;
            BAJAVELZ        =  BAJAVELZ_ZEISS_20x;
        }
    }

	/*  Se efectúa un primer intento en un entorno cercano al punto de enfoque
		teórico zBase, que idealmente debería ser el último punto enfocado. */

	if ( zBase + 2*BANDAFOCOINI > zPlano + BANDAFOCOMAX )
		zBase = zPlano + BANDAFOCOMAX - 2*BANDAFOCOINI;
	if ( zBase - 2*BANDAFOCOINI < zPlano - BANDAFOCOMAX )
		zBase = zPlano - BANDAFOCOMAX + 2*BANDAFOCOINI;

	sprintf(mensaje, "Iniciamos el autoenfoque en el entorno de Z = %.2lf", zBase);
	sendlog("autoenfoque", mensaje);

// En modo consola hay que cambiar entre modos de enfoque
// En modo dialogo no porque solo hay una ventana
	if (ini_contraste(pImagen))			// Inicio el proceso de contraste.
        return -1;

	cod = busca_foco(false, zBase + 2*BANDAFOCOINI, zBase - 2*BANDAFOCOINI,
			foco, zReal, &num, zPlano, 0, ALTAVELZ, bMoviendose);

#if defined (__BORLANDC__)
	Application->ProcessMessages();
#endif

	/*	Busco el contraste máximo obtenido en el primer recorrido de enfoque. */

	for (maxCont=foco[prim=0], i=1; i < num; i++)
		if (foco[i] > maxCont)  {
			maxCont = foco[i];
			prim = i;
		}

	/*  Calculo el valor promedio del contraste. */

	for (medCont=foco[0], i=1; i < num; i++)
		medCont += foco[i];
	medCont /= num;

	sprintf(mensaje, "Contraste máximo: %.2lf  promedio: %.2lf  cod: %2d", maxCont, medCont, cod);
	sendlog("autoenfoque", mensaje);

	/*	Compruebo que el valor de contraste máximo obtenido es aceptable. */

	if ( maxCont < CONTRASTEMIN )  {
		sprintf(mensaje, "Insuficiente contraste máximo: %.2lf", maxCont);
		sendlog("autoenfoque", mensaje);
		*zFoco = zReal[prim] + DESPLAZAFOCORAPIDO;	// Cota del máximo corregida
		*contraste = maxCont;					// Máximo contraste medido
		fin_contraste(pImagen);						// Finalizamos el proceso de enfoque
		return cod;
	}

	/*	Compruebo que el contraste máximo es suficentemente superior al promedio. */

	if ( maxCont * FACTOR_CONTRASTEMAX <= medCont && cod != BAJADA) {
		sprintf(mensaje, "Contraste medio superior al %d%% del máximo", (int)(FACTOR_CONTRASTEMAX*100));
		sendlog("autoenfoque", mensaje);
		*zFoco = zReal[prim] + DESPLAZAFOCORAPIDO;	// Máximo encontrado en el intervalo
		*contraste = maxCont;						// Máximo contraste medido
#if !defined (__BORLANDC__)
		putch('\a');
		Sleep(200);
		putch('\a');
#endif
		fin_contraste(pImagen);					// Finalizamos el proceso de enfoque
		return cod = NADA;					// Devuelvo el código de nada enfocado
	}

	/*  Comprobamos si se ha hallado un punto de enfoque suficientemente claro.
		En caso contrario, repetimos la búsqueda en un entorno más amplio. */

	if ( cod != MAXIMO ) {
//		if ( cod == BAJADA ) {
//			if ( zBase + 2*BANDAFOCOINI > zPlano + BANDAFOCOMAX )
//				zBase = zPlano + BANDAFOCOMAX - 2*BANDAFOCOINI;
//			sprintf(mensaje, "Se encontró una bajada en el intervalo inicial.\n"
//				"Ahora se recorrerá el intervalo: %f --> %f\n", zBase + 2*BANDAFOCOINI, zBase - BANDAFOCOINI);
//			cod = busca_foco_rapido(zBase + 2*BANDAFOCOINI, zBase - BANDAFOCOINI,
//				foco, zReal, &num, zPlano, 1, ALTAVELZ);
//		}
//		else {
			for (peso1=peso2=0, i=num/2; i >= 0; i--)  {
				peso1 += (int) foco[i];
				peso2 += (int) foco[num-i-1];
			}
			if ( peso1 > 2.40 * peso2 )  {
				if ( zBase + BANDAFOCOBAJO > zPlano + BANDAFOCOMAX )
					zBase = zPlano + BANDAFOCOMAX - BANDAFOCOBAJO;
				sprintf(mensaje, "No se encontró un máximo en el intervalo inicial.\n"
					"Ahora se recorrerá el intervalo: %.2f --> %.2f", zBase+BANDAFOCOBAJO, zReal[num/2+1]);
				sendlog("autoenfoque", mensaje);
				cod = busca_foco(false,zBase + BANDAFOCOBAJO, zReal[num/2+1],
					foco, zReal, &num, zPlano, 1, ALTAVELZ, bMoviendose);
			}
			else if ( peso2 > 2.40 * peso1 ) {
				if ( zBase - BANDAFOCOALTO < zPlano - BANDAFOCOMAX )
					zBase = zPlano - BANDAFOCOMAX + BANDAFOCOALTO;
				sprintf(mensaje, "No se encontró un máximo en el intervalo inicial.\n"
					"Ahora se recorrerá el intervalo: %.2f --> %.2f", zReal[num/2-1], zBase-BANDAFOCOALTO);
				sendlog("autoenfoque", mensaje);
				cod = busca_foco(false,zReal[num/2-1], zBase - BANDAFOCOALTO,
					foco, zReal, &num, zPlano, 1, ALTAVELZ, bMoviendose);
			}
			else  {
				if ( zBase + BANDAFOCOBAJO > zPlano + BANDAFOCOMAX )
					zBase = zPlano + BANDAFOCOMAX - BANDAFOCOBAJO;
				if ( zBase - BANDAFOCOALTO < zPlano - BANDAFOCOMAX )
					zBase = zPlano - BANDAFOCOMAX + BANDAFOCOALTO;
				sprintf(mensaje, "No se encontró un máximo en el intervalo inicial.\n"
					"Ahora se recorrerá el intervalo: %.2f --> %.2f", zBase+BANDAFOCOBAJO, zBase-BANDAFOCOALTO);
				sendlog("autoenfoque", mensaje);
				cod = busca_foco(false, zBase + BANDAFOCOBAJO, zBase - BANDAFOCOALTO,
						foco, zReal, &num, zPlano, 1, ALTAVELZ, bMoviendose);
			}
#if defined (__BORLANDC__)
			Application->ProcessMessages();
#endif
//		}
		cod |= RECORRIDOFIN;

		for (maxCont=foco[prim=0], i=1; i < num; i++)
			if (foco[i] > maxCont)  {
				maxCont = foco[i];
				prim = i;
			}

		sprintf(mensaje, "Contraste máximo en segunda pasada: %.2lf  cod: %2d", maxCont, cod);
		sendlog("autoenfoque", mensaje);
	}

	/*  Ahora ya hemos recogido suficientes datos para estudiar la evolución del
		valor de contraste en el entorno del punto de enfoque. Sólo tenemos que
		procesarlos para determinar la posición del máximo. */

	/*	Si no se ha encontrado un máximo válido, terminamos. */

/*	if ( (cod & MAXIMO) != MAXIMO )  {
		sprintf(mensaje, "No se ha encontrado un máximo válido.");
		sendlog("autoenfoque", mensaje);
		*zFoco = zReal[prim] + DESPLAZAFOCORAPIDO;	// Devolvemos la cota del máximo corregida
		*contraste = maxCont;					// Devolvemos el máximo contraste medido
		fin_contraste();						// Retorno al formato normal.
		return  cod;
	}*/

	/*	Calculamos el centro de gravedad de los valores de contraste en el entorno
		del valor máximo medido como estimación del punto de enfoque óptimo.
		Experimentalmente se ha determinado que el entorno que mejor estima la
		posición de enfoque óptimo es el del máximo y sus cuatro vecinos.
		A este valor hay que aplicarle una corrección debida al decalaje existente
		entre los instantes en que se captura la imagen y en que se mide la posición
		en la función busca_foco_rapido().*/

	sumaProductos = sumaPesos = 0.0;
	for (i=prim-RADIOFOCO; i <= prim+RADIOFOCO; i++)  {
		if ( i < 0 || i >= num )
			continue;
		sumaProductos += zReal[i] * foco[i];
		sumaPesos += foco[i];
	}
	zCG = sumaProductos / sumaPesos + DESPLAZAFOCORAPIDO;

	/*	Recorremos un tercer intervalo en el entorno del punto de enfoque
		óptimo a velocidad lenta, con objeto de encontrar un foco más exacto. */

	busca_foco(true,zCG+BANDAFOCOFINAL, zCG-BANDAFOCOFINAL,
		foco, zReal, &num, zPlano, 2, BAJAVELZ, bMoviendose);
#if defined (__BORLANDC__)
			Application->ProcessMessages();
#endif

	for (maxCont=foco[prim=0], i=1; i < num; i++)
		if (foco[i] > maxCont)  {
			maxCont = foco[i];
			prim = i;
		}

	/*	Calculamos el centro de gravedad de los valores de contraste en el entorno
		del valor máximo medido como estimación del punto de enfoque óptimo.
		Experimentalmente se ha determinado que el entorno que mejor estima la
		posición de enfoque óptimo es el del máximo y sus cuatro vecinos.
		A este valor hay que aplicarle una corrección debida al decalaje existente
		entre los instantes en que se captura la imagen y en que se mide la posición
		en la función busca_foco_rapido().*/

	sumaProductos = sumaPesos = 0.0;
	for (i=prim-RADIOFOCO; i <= prim+RADIOFOCO; i++)  {
		if ( i < 0 || i >= num )	// Compruebo qué valores solicitados
			continue;				// están dentro del intervalo recorrido
		sumaProductos += zReal[i] * foco[i];
		sumaPesos += foco[i];
	}
	zCG = sumaProductos / sumaPesos;

	sprintf(mensaje, "Posición CG máximo:   %.2lf  Contraste máximo: %.2lf",
		zCG, foco[prim]);
	sendlog("autoenfoque", mensaje);

    // Se establece a 0.82 el ajuste del punto de enfoque porque se ha visto que es siempre así
    if (EsLeica())
	    mejorAjuste = MEJOR_AJUSTE_LEICA_20x * 20 / dAumentos;
    else
	    mejorAjuste = MEJOR_AJUSTE_ZEISS_20x * 20 / dAumentos;

/*
    //	Buscamos el valor más aproximado del ajuste del punto de enfoque
	//	debido al desplazamiento. 
    double  zPos, ajuste;
//    mejorAjuste = 1.25;
    mejorAjuste = 2.25;
	maxCont = -1.0;
	for (ajuste=mejorAjuste; ajuste >= 0.25; ajuste-=0.086)  {
		mspGoCheck(Z_, zCG + ajuste + BACKSLASHZ);	// Nos acercamos un poco
		toma2_unica(M_centro_imagen);	// Capturamos una imagen
		mspGoCheck(Z_, zCG + ajuste);	// Aplicamos un ajuste teórico
		toma2_unica(M_centro_imagen);	// Capturamos una imagen
		toma2_unica(M_centro_imagen);	// Capturamos una imagen
		zPos = mspWhere(Z_);				// Evaluamos el ajuste efectivo
		Cont = determina_contraste(M_centro_imagen);	// Nivel de contraste de la imagen.
		if (Cont > maxCont)  {
			maxCont = Cont;
			mejorAjuste = zPos - zCG;
		}
		sprintf(mensaje, "Ajus: %.2lf  Zt: %.2lf  Zr: %.2lf  C: %.2lf",
			ajuste, zCG + ajuste, zPos, Cont);
		sendlog("autoenfoque", mensaje);
	}
	contAjusteFocoLento++;
	AjusteFocoLento += mejorAjuste;

	sprintf(mensaje, "Mejor ajuste local: %.2lf  global: %.2lf",
		mejorAjuste, AjusteFocoLento / contAjusteFocoLento);
	sendlog("autoenfoque", mensaje);
*/
    /*	Nos movemos al punto de máximo enfoque teórico y evaluamos el contraste. */

	zCG += mejorAjuste;
	mspGoCheck(Z_, zCG + BACKSLASHZ);
	toma2_unica(M_centro_imagen);	// Capturamos una imagen.
	mspGoCheck(Z_, zCG);				// Vamos al punto de enfoque óptimo estimado.
	toma2_unica(M_centro_imagen);	// Capturamos una imagen.
	Cont = determina_contraste(M_centro_imagen);	// Nivel de contraste de la imagen.

	sprintf(mensaje, "Posición CG ajustado: %.2lf  Contraste final:  %.2lf", zCG, Cont);

	/*	Devolvemos el punto de enfoque óptimo estimado. */

	*zFoco = zCG;					// Devolvemos la cota del punto de enfoque óptimo
	*contraste = Cont;				// Devolvemos el contraste medido

// En modo consola hay que cambiar entre modos de enfoque
// En modo dialogo no porque solo hay una ventana
	fin_contraste(pImagen);				// Retorno al formato normal.

	return  cod;
}


/**************************  busca_foco_rapido  *****************************
	Función para determinar el nivel de enfoque del campo visual del
	microscopio mediante el estudio de una serie continua de tomas de imagen
	durante un recorrido en Z hasta un punto dado.

	Devuelve una combinación lógica de dos valores:
		1 si se ha encontrado un flanco de subida.
		2 si se ha encontrado un flanco de bajada.
****************************************************************************/
int  busca_foco(bool bFino, double zMax, double zMin, double foco[], double cota[],
   int *npuntos, double zPlano, int pasada, double velocidad, bool bMoviendose)
{
	int  i, estado;
	double  posZ;

	/*	Variables relacionadas con la representación de contraste */

	char  texto[128];
	int  ymax, xmax, ybase, xbase, yini, xini, ymed, xfin, yfin;
	double  paso;

	if ( zMax - zMin < (BACKSLASHZ / 2) ) {
		error_leve("busca_foco_rapido", "cotas inicial y final incorrectas");
		return -1;
	}

	/*	Inicialización de la presentación gráfica de resultados en pantalla. */

	xmax = MbufInquire(M_overlay_normal, M_SIZE_X, M_NULL);
	ymax = MbufInquire(M_overlay_normal, M_SIZE_Y, M_NULL);
	paso = ymax / (1.05 * BANDAFOCOMAX + 1.1 * BANDAFOCOMAX);
	ybase = (int) (ymax - paso * (1.05 * BANDAFOCOMAX));
	switch (pasada) {
		case 0:
			xbase = 0;
			MgraColor(M_DEFAULT, ROJO_OSC);
			MgraLine(M_DEFAULT, M_overlay_normal, 0, ybase, xmax-1, ybase);
			ymed = ybase - (int) ((zPlano - (zMax + zMin)/2) * paso + 0.5);
			MgraColor(M_DEFAULT, AZUL);
			MgraLine(M_DEFAULT, M_overlay_normal, 0, ymed, xmax-1, ymed);
			break;
		case 1:
			xbase = xmax - 256;
			break;
		case 2:
			xbase = xmax/2 - 128;
			break;
	}
	xini = xbase + 50;
	MgraColor(M_DEFAULT, ROJO_OSC);
	MgraLine(M_DEFAULT, M_overlay_normal, xini, 0, xini, ymax - 1);

	/*  Para encontrar el punto de enfoque nos desplazamos con un movimiento
		rápido al extremo superior de la banda de enfoque indicada, y luego
		se desciende a baja velocidad mientras se monitoriza el contraste,
		con lo que se evitan posibles choques con el objetivo.
		Antes nos aseguramos de estar por encima del punto inicial, para
		neutralizar el efecto del backslash. */

	mspGoCheck(Z_, zMax);

    if (bMoviendose)
    {
	    // Comenzamos el desplazamiento a velocidad continua
	    mspGoAtThisSpeed(Z_, -1 * velocidad);// Sentido de desplazamiento descendente.
    }

	estado = NADA;
	i = 0;
    double dPaso = velocidad * TIEMPO_FICTICIO_AUTOENFOQUE_MANUAL;
    posZ = zMax + dPaso; //sumamos dPaso porque lo primero que se va a hacer es restarlo (para movimiento manual)
	do {
        if (!bMoviendose) //Movimiento manual
        {
            posZ -= dPaso; //
	        mspGoCheck(Z_, posZ);
        }
		toma2_unica(M_centro_imagen);	// Capturamos una imagen.
        if (bMoviendose)
		    posZ = mspWhere(Z_);				// Tomamos la cota una vez tomada la imagen.

		foco[i] = determina_contraste(M_centro_imagen);	// Nivel de contraste de la imagen.
		cota[i] = posZ;					// Registramos la cota(Z).

//		sprintf(mensaje, "i: %2d   Posición en Z: %.2lf   Contraste: %.1lf      ",
//			i, cota[i], foco[i]);
//		sendlog("autoenfoque",mensaje);				// Presento la posición en Z, y su contraste.

		/*	Comparo los valores de contraste. */

        double dMultiplicador;
        if (bFino)
            dMultiplicador = 2.0;
        else
            dMultiplicador = 1.0;
		if (i > 2)	// A partir de cuatro imágenes puedo buscar una subida.
			if ( foco[i] > UMBRALCONTRASTE )
				if ( foco[i] - foco[i-1] > 0 )
					if ( foco[i-1] - foco[i-2] > foco[i-1] / (dMultiplicador * FRACCIONFOCOSUBE) )
						if ( foco[i-2] - foco[i-3] > foco[i-2] / (dMultiplicador * FRACCIONFOCOSUBE) )
							if (!bFino && estado == BAJADA)
								estado = SUBIDA;	// Evito encontrar un mínimo
							else
								estado |= SUBIDA;

		if (i > 2)	// A partir de cuatro imágenes puedo buscar una bajada.
			if ( foco[i-3] > UMBRALCONTRASTE )
				if ( foco[i-3] - foco[i-2] > 0 )
					if ( foco[i-2] - foco[i-1] > foco[i-2] / (dMultiplicador * FRACCIONFOCOBAJA) )
						if ( foco[i-1] - foco[i] > foco[i-1] / (dMultiplicador * FRACCIONFOCOBAJA) )
							estado |= BAJADA;

		/*	Presentación gráfica de resultados en pantalla. */

		ymed = ybase - (int) ((zPlano - cota[i]) * paso + 0.5);
		yini = ymed - 1;
		yfin = ymed + 1;
		xfin = (int) (xini + 30 * log10(1.0 + foco[i]));	// Escala logarítmica para el contraste

		MgraColor(M_DEFAULT, AMARILLO_OSC);	// Color para la búsqueda del foco
		if ( estado & SUBIDA )
			MgraColor(M_DEFAULT, CYAN_OSC);	// Color para indicar subida
		if ( estado & BAJADA )
			MgraColor(M_DEFAULT, MAGENTA_OSC);	// Color para indicar bajada
		if ( (estado & MAXIMO) == MAXIMO )
			MgraColor(M_DEFAULT, ROJO);		// Color para indicar subida+bajada
		if ( xfin > xini )
			MgraRectFill(M_DEFAULT, M_overlay_normal, xini+1, yini, (xfin >= xmax)? xmax-1 : xfin, yfin);

		MgraColor(M_DEFAULT, AMARILLO);

        if (bFino)
		    MgraFontScale(M_DEFAULT, 0.8, 0.6);
        else
		    MgraFontScale(M_DEFAULT, 1.0, 0.8);
		sprintf(texto, "%.0lf", foco[i]);
		dibuja_texto(M_overlay_normal, texto, (xfin >= xmax-40)? xmax-40 : xfin+4, ymed, 1);	// Muestro el valor de contraste
		sprintf(texto, "%.2lf", zPlano - cota[i]);
		dibuja_texto(M_overlay_normal, texto, xini-4, ymed, 7);	// Muestro el valor de cota.
		MgraFontScale(M_DEFAULT, 1.0, 1.0);
		MgraColor(M_DEFAULT, TRANSPARENTE);

		/*	Evaluamos las condiciones de terminación del bucle
			en función del estado y del tipo de pasada en curso. */

        if (bFino)
        {
		    if ( estado & BAJADA)  break;
		    if ( ! (estado & SUBIDA) || pasada > 1 )
			    if ( posZ < zMin )  break;
        }
        else // Rapido
        {
		    if ( estado & BAJADA && pasada != 1)  break;
		    if ( ! (estado & SUBIDA) )
			    if ( posZ < zMin )  break;
            /////////////////////// PRUEBA
            //		if (pasada > 1)			// Condición de fin de recorrido fino
            //			if ( posZ < zMin)
            //				break;
            /////////////////////// FIN PRUEBA
        }
	}  while ( ++i < MAXPASOS && posZ >= zPlano - BANDAFOCOMAX );

    if (bMoviendose)
	    mspGoAtThisSpeed(Z_, 0.0);		// Detenemos el movimiento en el eje Z

	*npuntos = i;
	return estado;
}

#if !defined (__BORLANDC__)
/**************************  determina_contraste2 ****************************
	Función para determinar el contraste de una imagen en un buffer.
	Realiza una convolución para determinar los bordes, determinando
	la variable contraste, que es un valor arbitrario.
*****************************************************************************/
double determina_contraste2(MIL_ID M_fb)
{
	int i;
	long  histo[NUMVAL], numpixels;
	double  contraste;

	/*	Realizo una convolución en M_enfoque para determinar los bordes. */

	MimConvolve(M_fb, M_enfoque, M_EDGE_DETECT);	// Devuelve sólo valores positivos

	/*	Determino el histograma de la imagen tomada. */

	MimHistogram(M_enfoque, M_histo_aux);
	MimGetResult(M_histo_aux, M_VALUE, histo);	// Obtengo el resultado del histo.

	/*	Calculamos la suma de los valores al cuadrado para
		evaluar mejor el contraste. */

	//	MimLutMap(M_enfoque2, M_enfoque, M_lut_cuadrado);

	contraste = 0.0;
	for (numpixels=i=0; i < NUMVAL; i++) {
		numpixels += histo[i];
		contraste += (int) (KTE_CUADR * i * i * histo[i]);
	}

	/*	Determino el nivel de contraste por pixel. */

	contraste /= numpixels;

	return contraste;						// Devuelvo el valor de contraste obtenido.
}
#endif


/**************************  determina_contraste ****************************
	Función para determinar el contraste de una imagen en un buffer.
	Realiza una convolución para determinar los bordes, determinando
	la variable contraste, que es un valor arbitrario.
*****************************************************************************/
double determina_contraste(MIL_ID M_fb)
{
	long  ancho, alto, enfoque;
	double  contraste;

	/*	Se aplica la función MdigFocus() de las MIL. */

	MdigFocus(M_NULL, M_fb, M_DEFAULT, M_NULL, M_NULL, M_DEFAULT, M_DEFAULT,
		M_DEFAULT, M_DEFAULT, M_EVALUATE, &enfoque);

	/*	Determinamos el tamaño efectivo de la zona de enfoque para
		poder calcular el contraste por píxel. */

	MbufInquire(M_fb, M_SIZE_X, &ancho);
	MbufInquire(M_fb, M_SIZE_Y, &alto);

	/*	Determino el nivel de contraste por píxel. */

	contraste = (KTE_ENFOQUE * enfoque) / (double) (ancho * alto);

	return contraste;               // Se devuelve el valor de contraste obtenido
}

