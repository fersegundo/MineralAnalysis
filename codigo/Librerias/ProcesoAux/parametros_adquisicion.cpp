/*****************************************************************************
	parametros_adquisicion.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Febrero 2002
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <stdio.h>

#include "estructuras.h"

#include "parametros_adquisicion.h"

#include "control_proceso_imagenes.h"

#include "gestion_mensajes.h"
#include "gestion_ficheros.h"

#include "myVars.h"

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX			  100	// Valor máximo del parámetro seleccionado
#define MIN			 -100	// Valor mínimo del parámetro seleccionado
#define SALTO		   10	// Salto definido para cambiar de una forma escalonada de valor
#endif


/***************************  lee_paramPat  ********************************
	Función para leer los parámetros de toma de imágenes relativa al microscopio.
	Se devuelve 0 si todo ha ido bien, -2 en otro caso
*****************************************************************************/
int lee_paramMicroscopio(char *nom_fich, parametros* param)
{
	double  voltaje_lampara;
	char	*fn = "lee_paramMicroscopio";

	//	Carga del fichero
	if ( LoadVars(nom_fich) == NULL) {
		error_fatal(nom_fich, "El fichero indicado no se encuentra", 0);
		return -1;
	}

	LOADDBL(voltaje_lampara);		// Tensión nominal de la lámpara
	param->Mtb.voltLamp = voltaje_lampara;


	//Cargamos parametros Polarizacion (Ting)
	int polarizador_predefinido = 0;
	int num_filtros_polarizadores = 0;
	int* filtros_polarizadores = NULL;
	int* porcentaje_exposicion = NULL;
	
	LOADINT(polarizador_predefinido);
	if (polarizador_predefinido<1 || polarizador_predefinido>MAX_FILTROS_MTB)
	{
		error_leve(fn,"Se ha encontrado un filtro con valor fuera del rango 1..MAX_FILTROS:MTB en el fichero de configruacion");
		polarizador_predefinido = 1;
	}
	param->Mtb.rueda_filtros.posFiltro = polarizador_predefinido;
	
	LOADINT(num_filtros_polarizadores);
	if (num_filtros_polarizadores > MAX_FILTROS_MTB || num_filtros_polarizadores < 0) {
		error_fatal(fn, "Valor de 'num_filtros_polarizadores' inaceptable", 1);
		return -1;
	}
	param->Mtb.rueda_filtros.numFiltros = num_filtros_polarizadores;

	//	Inicializamos los parámetros con valores por defecto. 
	for (int i = 0; i<MAX_FILTROS_MTB;i++)
	{
		param->Mtb.rueda_filtros.filtros[i] = 0;
		param->Mtb.rueda_filtros.porcentaje_exposicion[i] = 100;
	}
	
	filtros_polarizadores = param->Mtb.rueda_filtros.filtros;
	LOADVINT(filtros_polarizadores, MAX_FILTROS_MTB);
	for (i=0; i<num_filtros_polarizadores;i++)
	{
		if (filtros_polarizadores[i]<1 || filtros_polarizadores[i]>MAX_FILTROS_MTB)
		{
			error_leve(fn,"Se ha encontrado un filtro con valor fuera del rango 1..MAX_FILTROS_MTB en el fichero de configruacion");
			filtros_polarizadores[i] = 1;
		}
	}
	
	porcentaje_exposicion = param->Mtb.rueda_filtros.porcentaje_exposicion;
	LOADVINT(porcentaje_exposicion, MAX_FILTROS_MTB);
	
    // OBJETIVOS
    //Inicializacion
    for (i=0;i<MAX_OBJETIVOS;i++)
        param->objetivos[i].dAumento = 0; //0 indica que este objetivo no esta en uso
    
    for (i=0;i<GetNumSections();i++)
    {
        int nDummy1, nDummy2;
        char* strNumObjetivo;
        strNumObjetivo = FindSection(i,nDummy1, nDummy2);
        if (strNumObjetivo != NULL)
        {
            int nIndex = atoi(strNumObjetivo)-1;

            char* directorio_patrones = param->objetivos[nIndex].csDirectorio.GetBuffer(_MAX_PATH + 1);
            LOADSTRSECTION(directorio_patrones,i);
            param->objetivos[nIndex].csDirectorio.ReleaseBuffer(-1);
             // Nos aseguramos de que el ultimo caracter sea '\'
            CString csUltimoCaracter = param->objetivos[nIndex].csDirectorio.Right(1);
            if (csUltimoCaracter != "\\" && csUltimoCaracter != "/")
                param->objetivos[nIndex].csDirectorio += _T("\\");

            double aumentos;
            LOADDBLSECTION(aumentos,i);
            param->objetivos[nIndex].dAumento = aumentos;

            double distancia_maxima_plano ;
            LOADDBLSECTION(distancia_maxima_plano ,i);
            param->objetivos[nIndex].dMaxDistPlano = distancia_maxima_plano ;
        }
    }

	return 0;
}

/***********************************************************************
	Función para leer los parámetros de toma de imágenes relativa al revovler DTA
	Se devuelve 0 si todo ha ido bien, -2 en otro caso
*****************************************************************************/
int lee_paramRevolver(char *nom_fich, parametros* param)
{
	//	Inicializamos los parámetros con valores por defecto. 
	for (int i = 0; i<MAX_FILTROS_DTA;i++)
	{
		param->Rueda.reflectancia_nominal_pat_os[i]	= 0;
		param->Rueda.reflectancia_nominal_pat_cl[i]	= 0;
		param->Rueda.aplicaTing[i]					= 0;
		param->Rueda.exposicion[i]				    = 0;
		param->Rueda.diferencia_z[i]				= 0;
		param->Rueda.espectro[i]				    = 0;
		param->BarridoPatrones.rango_util[i]		= 0;
		strcpy(param->Rueda.comentario[i],"");
	}


	//	Carga del fichero
	if ( LoadVars(nom_fich) == NULL) {
		error_fatal(nom_fich, "El fichero indicado no se encuentra", 0);
		return -1;
	}

	int  filtro_predefinido;
	LOADINT(filtro_predefinido);	// Posición normal de la rueda de filtros
	param->Rueda.posFiltro = filtro_predefinido - 1; // En el fichero, los filtros van de 1 a 16, en memoria trabajamos de 0 a 15
	int  filtro_enfoque;
	LOADINT(filtro_enfoque);	// Posición normal de la rueda de filtros
	param->Rueda.posEnfoque = filtro_enfoque - 1; // En el fichero, los filtros van de 1 a 16, en memoria trabajamos de 0 a 15

	// Cargar parametros filtros
	char nombre_variable[9]; 
	char info_filtros[MAX_FILTROS_DTA][TAM_INFO_FILTROS];
	for (i=0; i<MAX_FILTROS_DTA;i++)
	{
		sprintf(nombre_variable,"filtro%d",i+1);
		LOADSTR2(info_filtros[i] ,nombre_variable);
		if (sscanf(info_filtros[i],"%lf %lf %d %d %lf %d %d %d %[^\n]",
			&param->Rueda.reflectancia_nominal_pat_cl[i],
			&param->Rueda.reflectancia_nominal_pat_os[i],
			&param->Rueda.exposicion[i],
			&param->Rueda.base_exp[i],
			&param->Rueda.diferencia_z[i],
			&param->Rueda.aplicaTing[i],
			&param->Rueda.espectro[i],
			&param->BarridoPatrones.rango_util[i],
			&param->Rueda.comentario[i]) == EOF)
		{
			//No hay mas filtros en el fichero de configuracion
			break;
		}
		if (param->Rueda.aplicaTing[i] != 1 && param->Rueda.aplicaTing[i] != 0)
			//ERROR
			param->Rueda.aplicaTing[i] = 0; //si el valor no es valido, suponemos que no se aplica Ting
	}

    //segun esta programado los cambios de z al cambiar de filtro, la referencia tiene que estar en el filtro 0
	for (i=1; i<MAX_FILTROS_DTA;i++)
	    param->Rueda.diferencia_z[i] = param->Rueda.diferencia_z[i] - param->Rueda.diferencia_z[0]; 
	param->Rueda.diferencia_z[0] = 0; 

	return 0;
}

/***************************  lee_paramPat  ********************************
	Función para leer los parámetros de toma de imágenes relativa a la cámara.
	Se devuelve 0 si todo ha ido bien, -2 en otro caso
*****************************************************************************/
int lee_paramCamara(char *nom_fich, parametros *paramAdq)
{
	int	 brillo_camara, ganancia_camara;
	char *formato_digitalizador, *formato_digitalizador_e;
	int	 ancho_imagen, alto_imagen, ancho_imagen_e, alto_imagen_e;

	char	*fn = "lee_ParamCamara";

	//	Inicializamos los parámetros con valores por defecto. 
	formato_digitalizador = paramAdq->Cam.formatoDig;
	strcpy(formato_digitalizador, FORMATO_DIG_NORMAL);
	ancho_imagen = ANCHO_IMAGEN_N;
	paramAdq->Cam.anchoImagen = ancho_imagen;
	alto_imagen = ALTO_IMAGEN_N;
	paramAdq->Cam.altoImagen = alto_imagen;

	formato_digitalizador_e = paramAdq->Cam.formatoDig_e;
	strcpy(formato_digitalizador_e, FORMATO_DIG_ENFOQUE);
	ancho_imagen_e = ANCHO_IMAGEN_E;
	paramAdq->Cam.anchoImagen_e = ancho_imagen_e;
	alto_imagen_e = ALTO_IMAGEN_E;
	paramAdq->Cam.altoImagen_e = alto_imagen_e;

	/*	Indico el nombre del archivo donde se encuentran los parámetros
		de toma de imágenes, así como del microscopio y la cámara. */

	if ( LoadVars(nom_fich) == NULL) {
		error_fatal(nom_fich, "El fichero indicado no se encuentra", 0);
		return -2;
	}

	LOADSTR(formato_digitalizador);

	if ( LOADINT(ancho_imagen) == 0)
		ancho_imagen = ANCHO_IMAGEN_N;
	if (ancho_imagen > MAX_ANCHO_IMAGEN || ancho_imagen <= 0) {
		error_fatal(fn, "Valor de 'ancho_imagen' inaceptable", 1);
		return -2;
	}
	paramAdq->Cam.anchoImagen = ancho_imagen;

	if ( LOADINT(alto_imagen) == 0)
		alto_imagen = ALTO_IMAGEN_N;
	if (alto_imagen > MAX_ANCHO_IMAGEN || alto_imagen <= 0) {
		error_fatal(fn, "Valor de 'alto_imagen' inaceptable", 1);
		return -2;
	}
	paramAdq->Cam.altoImagen = alto_imagen;

	LOADSTR(formato_digitalizador_e);

	if ( LOADINT(ancho_imagen_e) == 0)
		ancho_imagen_e = ANCHO_IMAGEN_E;
	if (ancho_imagen_e > ancho_imagen || ancho_imagen_e <= 0) {
		error_fatal(fn, "Valor de 'ancho_imagen_e' inaceptable", 1);
		return -2;
	}
	paramAdq->Cam.anchoImagen_e = ancho_imagen_e;

	if ( LOADINT(alto_imagen_e) == 0 )
		alto_imagen_e = ALTO_IMAGEN_E;
	if (alto_imagen_e > alto_imagen || alto_imagen_e <= 0) {
		error_fatal(fn, "Valor de 'alto_imagen_e' inaceptable", 1);
		return -2;
	}
	paramAdq->Cam.altoImagen_e = alto_imagen_e;


	LOADINT(brillo_camara);
	paramAdq->Cam.brillo = brillo_camara;
	LOADINT(ganancia_camara);
	paramAdq->Cam.ganancia = ganancia_camara;

	return 0;
}


/**************************************************************************
	Función para crear el fichero de parámetros iniciales relativos al revovler DTA 
	Devuelve 0 si todo ha ido bien, -1 en otro caso
*****************************************************************************/
int crea_paramRevolver(char *raizIni, parametros* param)
{
	char nom_fich[512];
	FILE  *archivo;

	sprintf(nom_fich, "%s%s%s", raizIni, FICH_PARAM_REVOLVER, EXT_INI);

	if ((archivo = fopen(nom_fich, "wt")) == NULL)
		return -1;

	fprintf(archivo, "# PARÁMETROS INICIALES RELATIVOS AL REVOLVER\n\n");


	fprintf(archivo, "# Parametros relativos a la rueda de filtros\n");
	fprintf(archivo, "filtro_predefinido =%2d	\t# Posición de la rueda de filtros en modo observación\n\n", param->Rueda.posFiltro+1);
	fprintf(archivo, "filtro_enfoque =%2d	\t# Filtro con el que se enfocará\n\n", param->Rueda.posEnfoque+1);

	fprintf(archivo, "# Reflectancia nominal de los patrones\n");
	fprintf(archivo, "#		400		450		500		550		600		650		700		750		800		850		900		950		1000    350    355    370\n");

	fprintf(archivo, "#Alta:	89,525	89,788	89,740	89,626	89,350	88,623	87,829	86,720	84,641	84,727	88,450	91,432	93,020	89,020	89.26	88.90\n");
	fprintf(archivo, "#Baja: 	4,512	4,391	4,333	4,290	4,245	4,201	4,168	4,160	4,160	4,150	4,083	4,083	4,081	4,703	4.70	4.65\n\n");


	fprintf(archivo, "# FILTROS\n");
	fprintf(archivo, "# ---------------------------------------------------------------------------\n");
	fprintf(archivo, "#         Ref_cl Ref_os Exp   BaseExp   Z  Ting Lambda RangoUtil COMENTARIO\n");
	fprintf(archivo, "# ---------------------------------------------------------------------------\n");
	char nombre_variable[9]; 
	for (int i=0; i < MAX_FILTROS_DTA; i++)
	{
		sprintf(nombre_variable,"filtro%d",i+1);

		fprintf(archivo, "%s = %5.2lf  %5.2lf  %4d  %4d  %5.2lf  %d   %4d    %d       %s\n", nombre_variable,
			param->Rueda.reflectancia_nominal_pat_cl[i],
			param->Rueda.reflectancia_nominal_pat_os[i],
			param->Rueda.exposicion[i],
			param->Rueda.base_exp[i],
			param->Rueda.diferencia_z[i] - param->Rueda.diferencia_z[param->Rueda.posEnfoque], //queremos que en el fichero el 0 este en la banda de referencia
			param->Rueda.aplicaTing[i],
			param->Rueda.espectro[i],
            param->BarridoPatrones.rango_util[i],
			param->Rueda.comentario[i]);
	}
	fprintf(archivo, "\n\n");

    fprintf(archivo, "# NOTA: BaseExp: En microsegundos. La exposicion real de la camara es la multiplicacion \n");
    fprintf(archivo, "# de BaseExp*Exp (OJO: aunque los valores maximos de Exp y BaseExp son 4095 y 22222 respectivamente, \n");
    fprintf(archivo, "# el valor maximo efectivo de exposicion son solo 10 segundos \n");
    fprintf(archivo, "# \n");
    fprintf(archivo, "# NOTA: RangoUtil: Nivel de gris a restar a la moda para tener en cuenta el valor a \n");
	fprintf(archivo, "# la hora de calcular los patrones. Cuanto mayor, mas tolerancia. \n");

	fclose(archivo);
	return 0;
}



/***************************  crea_paramAdq  ********************************
	Función para crear el fichero parametros_aplicacion.ini
	Devuelve 0 si todo ha ido bien, -1 en otro caso
    NO SE NECESITA
int  crea_paramAplicacion(char *raizIni, parametros *param)
{
	int  i;
	char nom_fich[512];
	FILE  *archivo;

	sprintf(nom_fich, "%s%s%s", raizIni, FICH_PARAM_APLICACION, EXT_INI);

	if ((archivo = fopen(nom_fich, "wt")) == NULL)
		return -1;

	fprintf(archivo, "# PARÁMETROS DE ADQUISICION GENERALES\n\n");

	fprintf(archivo, "# Raíz patrones\n");
	fprintf(archivo, "raiz_patrones = %s\n\n", param->raiz_patrones);

    fprintf(archivo, "# Condiciones de realización del ensayo.\n");
	fprintf(archivo, "num_bandas= %ld	\t# Número de bandas a procesar\n", param->nBandas );
	fprintf(archivo, "filtros_bandas =  ");
	for (i=0; i < param->nBandas; i++)
		fprintf(archivo, "%2d ", param->filtro[i]);
	fprintf(archivo, "\t# Vector con la posición de los filtros\n");

	fprintf(archivo, "# Parámetros relativos a la representación de histogramas.\n");
	fprintf(archivo, "escala = %.6lf\n\n", param->escala);

	fclose(archivo);

	return 0;
}
*****************************************************************************/

/***************************  crea_paramPat  ********************************
	Función para crear el fichero de parámetros iniciales relativos al microscopio 
	Devuelve 0 si todo ha ido bien, -1 en otro caso
*****************************************************************************/
int crea_paramMicroscopio(char *raizIni, parametros* param)
{
	char nom_fich[512];
	FILE  *archivo;

	sprintf(nom_fich, "%s%s%s", raizIni, FICH_PARAM_MICROSCOPIO, EXT_INI);

	if ((archivo = fopen(nom_fich, "wt")) == NULL)
		return -1;

	fprintf(archivo, "# PARÁMETROS INICIALES RELATIVOS AL MICROSCOPIO\n\n");

	fprintf(archivo, "voltaje_lampara =%6.3lf	\t# Tensión en la lámpara.\n\n", param->Mtb.voltLamp);

	fprintf(archivo, "# POLARIZACION (TING)\n");
	fprintf(archivo, "polarizador_predefinido =  %d         # se usará en enfoque y para aquellas filtros que no requieran Ting\n", param->Mtb.rueda_filtros.posFiltro);
	fprintf(archivo, "num_filtros_polarizadores =  %d     # si es 0, indica que no se realizará polarizacion alguna\n", param->Mtb.rueda_filtros.numFiltros);
	fprintf(archivo, "filtros_polarizadores =  ");
	for (int i=0; i < MAX_FILTROS_MTB; i++)
		fprintf(archivo, "%4ld ", param->Mtb.rueda_filtros.filtros[i]);
	fprintf(archivo, "\n");
	fprintf(archivo, "porcentaje_exposicion =  ");
	for (i=0; i < MAX_FILTROS_MTB; i++)
		fprintf(archivo, "%4ld ", param->Mtb.rueda_filtros.porcentaje_exposicion[i]);
	fprintf(archivo, "       # Porcentaje de reduccion de la luminosidad\n");

	fprintf(archivo, "\n#OBJETIVOS\n");
    for (i=0;i<MAX_OBJETIVOS;i++)
    {
        if (param->objetivos[i].dAumento != 0)
        {
	        fprintf(archivo, "[%d]\n",                      i+1);
	        fprintf(archivo, "directorio_patrones = %s\n",      param->objetivos[i].csDirectorio);
	        fprintf(archivo, "aumentos = %lf\n",                param->objetivos[i].dAumento);
	        fprintf(archivo, "distancia_maxima_plano = %lf\n\n",param->objetivos[i].dMaxDistPlano);
        }
    }
    
	fclose(archivo);
	return 0;
}

/***************************  crea_paramPat  ********************************
	Función para crear el fichero de parámetros iniciales relativos a la cámara 
	(brillo, ganancia, exposición)
	Devuelve 0 si todo ha ido bien, -1 en otro caso
*****************************************************************************/
int crea_paramCamara(char *raizIni, parametros* param)
{
	char nom_fich[512];
	FILE  *archivo;

	sprintf(nom_fich, "%s%s%s", raizIni, FICH_PARAM_CAMARA, EXT_INI);

	if ((archivo = fopen(nom_fich, "wt")) == NULL)
		return -1;

	fprintf(archivo, "# PARÁMETROS INICIALES RELATIVOS A LA CÁMARA\n\n");

	fprintf(archivo, "# Formato del digitalizador\n");
	fprintf(archivo, "# FORMATO_2_MODO_2\t 'M_1280X960_Y'\n");
	fprintf(archivo, "# FORMATO_1_MODO_5\t 'M_1024X768_Y'\n");
	fprintf(archivo, "# FORMATO_1_MODO_2\t 'M_800X600_Y'\n");
	fprintf(archivo, "# FORMATO_0_MODO_5\t 'M_640X480_Y'\n\n");

	fprintf(archivo, "formato_digitalizador = %s\n", param->Cam.formatoDig);
	fprintf(archivo, "ancho_imagen = %ld\n", param->Cam.anchoImagen);
	fprintf(archivo, "alto_imagen = %ld\n\n", param->Cam.altoImagen);

	fprintf(archivo, "formato_digitalizador_e = %s\n", param->Cam.formatoDig_e);
	fprintf(archivo, "ancho_imagen_e = %ld\n", param->Cam.anchoImagen_e);
	fprintf(archivo, "alto_imagen_e = %ld\n\n", param->Cam.altoImagen_e);

	
	fprintf(archivo, "# Parámetros relativos a la cámara.\n");
	fprintf(archivo, "brillo_camara =%4ld	  \t# Brillo.\n", param->Cam.brillo);
	fprintf(archivo, "ganancia_camara =%4ld	  \t# Ganancia.\n", param->Cam.ganancia);

	fclose(archivo);

	return 0;
}

