/*
	adquisicion_imagenPat.cpp
*/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#if !defined (__BORLANDC__)
#include <math.h>
#include <conio.h>
#endif
#include <stdio.h>

#include "..\Librerias\include\procesoAux.h"
#include "..\Librerias\include\controlDisp.h"
#include "..\Librerias\include\LibBarrido.h"
#include "..\Librerias\ControlRevolver\control_revolver_dta.h"

#include "adquisicion_imagenPat.h"

#define CAMPOS_PATRONES		 (CAMPOS_EJE_X*CAMPOS_EJE_Y)

#define SALTO_DIBUJO		20
#define SEMILADO_RECT		 7

//	Declaración de variables externas a este módulo. 

/*	Vectores de imágenes, para el almacenamiento de los sucesivos
	campos adquiridos de los patrones. */
MIL_ID M_banda_acum[MAX_FILTROS_DTA]; //acumulacion de campos validos
MIL_ID M_banda_enmascarada_acum[MAX_FILTROS_DTA];

extern TomaAutomatica_T TomaAutomatica;
extern DtaRevolver Rueda;           // Objeto revolver para el control de la rueda de filtros

// factor de multiplicacion de la sigma, que se restará a la moda para calcular el umbral en CalcularMascara. 
double UMBRAL_MULT_SIGMA[MAX_BANDAS];
// el nivel de gris que se restará a la moda para calcular el umbral en CalcularMascara. Solo si UMBRAL_MULT_SIGMA es 0
double UMBRAL_DIF[MAX_BANDAS];

#define PROFUNDIDAD_CONTADOR    16
MIL_ID M_cont_campos[MAX_BANDAS]; // Por cada pixel, cuantos campos fueron validos en el barrido. Comun para todas las bandas
MIL_ID M_mascara_aux; // Mascara auxiliar con 0 y 1 por cada campo que se usará internamente
MIL_ID M_acum_aux; // Acumulador auxiliar que se usará internamente


//Declaracion de funciones auxuliares
#if !defined (__BORLANDC__)
int definePlanoRegresion(parametros& param,	plane&	plano);
int pideP(point *p);
int captura_imagenAcum(parametros *param, DtaRevolver& Rueda);
#endif
int haz_movimiento(parametros *param, ppoint inicio, ppoint final, plane* planoRegresion, char* tipoPat);
int avanza_y_toma_campo(int& campoPat, 
                        ppoint inicio,
                        double& X, double& Y, double& Z, 
                        int i, int j, int dirX, double saltoX,
                        parametros* param, plane* planoRegresion, char* tipoPat);
int toma_campoPat(parametros *param, char* tipoPat);
void muestra_tomaPat(MIL_ID M_fb, int x, int y, bool solido);

// Declaración de variables globales declaradas en este módulo
int campoPat;

#if defined (__BORLANDC__)
bool TomaPatCancelada(void); //esta funcion esta declarada en el programa de borland
#endif

/**************************  lee_paramTomaPat  ******************************
OBSOLETO
	Método para leer las variables relativas a la presentación de imágenes
	en pantalla y el funcionamiento del mtb (apagado de la lámpara al
	finalizar el programa).
int lee_paramTomaPat(char *raiz, int *LutColor, bool *aLamp)
{
	char nomFich[256];
	int aplicaLutColor, apagaLamp;

	// Inicializo las variables por defecto
	*LutColor = 0;
	*aLamp = false;

	// Composición del nombre del fichero
	sprintf(nomFich, "%sparam_TomaPat%s", raiz, EXT_INI);

	// Lectura de las variables
	LoadVars(nomFich);

	LOADINT(aplicaLutColor);
	*LutColor = aplicaLutColor;

	LOADINT(apagaLamp);
	if (apagaLamp)	*aLamp = true;
	else			*aLamp = false;

	return 0;
}
*****************************************************************************/


/**************************  ini_TomaPatrones  ******************************
	Reserva de memoria para tomar una serie de imagenes promediadas.
*****************************************************************************/
int ini_TomaPatrones(parametros *param)
{
	int i;

    //Acumuladores
	for (i=0; i < param->nBandas; i++)  {
	    MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &M_banda_acum[i]);
	    MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &M_banda_enmascarada_acum[i]);
	    MbufClear(M_banda_acum[i], 0);
	    MbufClear(M_banda_enmascarada_acum[i], 0);

        //Contador
	    MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, 8+M_UNSIGNED, //necesario 32+M_FLOAT para realizar operaciones en coma flotante
		    M_IMAGE+M_PROC, &M_cont_campos[i]);
	    MbufClear(M_cont_campos[i], 0);

	}

    //Buffers auxiliares
	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, 8+M_UNSIGNED,
		M_IMAGE+M_PROC, &M_mascara_aux);
	MbufClear(M_mascara_aux, 0);

	MbufAlloc2d(M_sistema, param->Cam.anchoImagen, param->Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		M_IMAGE+M_PROC+M_GRAB, &M_acum_aux);
	MbufClear(M_acum_aux, 0);

	return 0;
}


/**************************  fin_TomaPatrones  ******************************
*****************************************************************************/
int fin_TomaPatrones(int num_bandas)
{
	int i;

	for (i=0; i < num_bandas; i++)
    {
		MbufFree(M_banda_enmascarada_acum[i]);
		MbufFree(M_banda_acum[i]);
        MbufFree(M_cont_campos[i]);
    }
    MbufFree(M_mascara_aux);
    MbufFree(M_acum_aux);

	return 0;
}


#if !defined (__BORLANDC__)
/**************************  adquierePat  ***********************************
Menu previo a la funcion haz_movimiento. Se pide el plano de regresion y los extremos del barrido
Devuelve '-1' si se ha cancelado la toma
    bExposicionAjustada - si false, habrá que ajustar la exposicion justo antes de comenzar el barrido
INTERFAZ
SOLO VISUAL
*****************************************************************************/
int adquierePat(parametros *param, char *tipoPat, bool bExposicionAjustada)
{
    int nRes;
	char mensaje[LONGITUD_TEXTO*2], linea[LONGITUD_TEXTO/2];
	point inicio, final;
	plane planoRegresion;
	plane* pPlanoRegresion = NULL; //si es NULL, no hay plano de regresion definido


	// Primero se pide que se enfoque las cuatro esquinas del patron y se calcula el plano de regresion
	if (definePlanoRegresion(*param, planoRegresion) == 0)
    {
		pPlanoRegresion = &planoRegresion;
	    sprintf(mensaje,"\n+-  DEFINE PUNTOS DE ADQUISICIÓN   ---------------------------------------+");
	    strcat(mensaje, "\n| Seleccione (NO ES NECESARIO ENFOCAR) los puntos extremos de una región en la que    |");
    }
    else
    {
	    sprintf(mensaje,"\n+-  DEFINE PUNTOS DE ADQUISICIÓN   ---------------------------------------+");
	    strcat(mensaje, "\n| Seleccione (ENFOCANDO) los puntos extremos de una región en la que      |");
    }

	sprintf(linea, "\n| desee adquirir las imágenes del patrón. Se recorrerán %d campos.        |", 
        param->BarridoPatrones.camposEjeX*param->BarridoPatrones.camposEjeY);
	strcat(mensaje, linea);
	sprintf(linea,  "\n| Se capturan las imágenes correspondientes al patrón: %s             |",
		tipoPat);
	strcat(mensaje, linea);
	strcat(mensaje, "\n+-------------------------------------------------------------------------+");
	CharToOem(mensaje,mensaje);
	printf(mensaje);

    if (TomaAutomatica.bAutomatica == true)
    {
        // Caso para realizar la toma de patrones sin iteraccion del usuario
	    return haz_movimiento(param, &TomaAutomatica.Puntos[0], &TomaAutomatica.Puntos[3], pPlanoRegresion, tipoPat);
    }
    else
    {
        if (pPlanoRegresion == NULL)
        {
            // El usuario debe enfocar
            // Se ha de enfocar con el filtro "de enfoque" (o "de referencia", es decir el monocromatico)
            Rueda.ChangeFilter(param->filtro[param.Rueda.posEnfoque]);
        }

	    sprintf(mensaje, "Sitúese en un extremo de la región del patrón");
	    sendop(mensaje);
	    printf("\n");

        nRes = pideP(&inicio);
        // OJO: esto se hace porque puede ser que el usuario haya enfocado manualmente antes
        // Estamos en el filtro "de enfoque" y sin embargo zPredefinido debe ser calculado para el pancromatico
        Rueda.zPredefinido = mspWhere(Z_) + param->Rueda.diferencia_z[param->Rueda.posFiltro] - param->Rueda.diferencia_z[param->filtro[param.Rueda.posEnfoque]] ;
	    if ( nRes < 0)
        {
            // se ha punsado ESC
            if (pPlanoRegresion == NULL) // si hay plano de regresion, el punto de enfoque nos da igual 
                // Regresamos al filtro predefinido (o de usuario)
                Rueda.ChangeFilter(param->Rueda.posFiltro);
		    return -1;
        }

	    sprintf(mensaje, "Sitúese en el extremo opuesto de la región del patrón\n");
	    sendop(mensaje);

        nRes = pideP(&final);
        // OJO: esto se hace porque puede ser que el usuario haya enfocado manualmente antes
        // Estamos en el filtro "de enfoque" y sin embargo zPredefinido debe ser calculado para el pancromatico
        Rueda.zPredefinido = mspWhere(Z_) + param->Rueda.diferencia_z[param->Rueda.posFiltro] - param->Rueda.diferencia_z[param->filtro[param.Rueda.posEnfoque]] ;
	    if ( nRes < 0)
        {
            // se ha punsado ESC
            if (pPlanoRegresion == NULL)
                // Regresamos al filtro predefinido (o de usuario)
                Rueda.ChangeFilter(param->Rueda.posFiltro);
		    return -1;
        }

	    printf("\n");
        if (pPlanoRegresion == NULL)
        {
            // El usuario debe enfocar
            // Regresamos al filtro predefinido (o de usuario)
            Rueda.ChangeFilter(param->Rueda.posFiltro);
        }

        if (tipoPat == "PAT_CL" && bExposicionAjustada == false)
        {
            //Ajustamos las exposiciones en el primer extremo antes de barrer 
            if (pPlanoRegresion != NULL)
            {
                // Z de enfoque (para el filtro de enfoque) segun el plano de regresion
                inicio.coord[Z_] = pPlanoRegresion->A * inicio.coord[X_] + pPlanoRegresion->B * inicio.coord[Y_] + pPlanoRegresion->C;
            }
            inicio.coord[Z_] += BACKSLASHZ;
	        mspGoP(inicio); //BACKSLASHZ
            inicio.coord[Z_] -= BACKSLASHZ;
	        mspGoP(inicio);
		    sendop ("Es obligatorio realizar el ajuste automatico de exposicion al menos una vez. Ajustando exposiciones ...");
            AjusteAutomaticoExposicion(M_digitalizador, param);
		    sendop ("Exposiciones ajustadas. Salvamos las nuevas exposiciones en el archivo de configuracion.");

		    if (crea_paramRevolver(param->raiz_patrones, param) )  {
			    error_fatal("control_parametros", "ERROR: No se puede crear el fichero de configuracion. Continuamos con la toma", 0);
		    }
        }

	    return haz_movimiento(param, &inicio, &final, pPlanoRegresion, tipoPat);
    }

}

/**************************************************************************
Paso previo a haz_movimiento para pedir al usuario que enfoque cuatro esquinas del patron
para poder calcular un plano de regresion. Paso opcional.
Devuelve 0 si el plano ha sido definido y -1 si el usuario ha optado por no definirlo
AUXILIAR
SOLO VISUAL
*****************************************************************************/
int definePlanoRegresion(parametros& param,	plane&	plano)
{
	char mensaje[LONGITUD_TEXTO*2];
	point esquina;
	pointlist plist;

    // Se ha de enfocar con el filtro "de enfoque" (o "de referencia", es decir el monocromatico)
    Rueda.ChangeFilter(param.filtro[param.Rueda.posEnfoque]);

	sprintf(mensaje,"\n+-  DEFINE PLANO DE REGRESION   ------------------------------------------+");
	strcat(mensaje, "\n| Posiciónese en cada esquina del patrón para poder enfocar correctamente |");
	strcat(mensaje, "\n| Enfoque y presione [ENTER]                                              |");
	strcat(mensaje, "\n| [Esc] - Para omitir este paso                                           |");
	strcat(mensaje, "\n+-------------------------------------------------------------------------+\n");
	CharToOem(mensaje,mensaje);
	printf(mensaje);

	dim_pointlist(PUNTOS_PLANO_REGRESION,plist);		// Dimensionamos la lista de puntos.

	for (int i=0;i<PUNTOS_PLANO_REGRESION;i++)
	{
        if (TomaAutomatica.bAutomatica == true)
        {
            put_pointlist(plist,i,TomaAutomatica.Puntos[i]);
        }
        else
        {
		    if ( pideP(&esquina) < 0)
		    {
			    // Se ha pulsado [Esc], no se define plano de regresion
			    free_pointlist(plist);					// Liberamos la memoria empleada

                // OJO: esto se hace porque puede ser que el usuario haya enfocado manualmente antes
                // Estamos en el filtro "de enfoque" y sin embargo zPredefinido debe ser calculado para el pancromatico
                double zPancromatico = mspWhere(Z_) + param.Rueda.diferencia_z[param.Rueda.posFiltro] - param.Rueda.diferencia_z[param.filtro[param.Rueda.posEnfoque]] ;
                Rueda.zPredefinido = zPancromatico; //actualizo el valor absoluto de z del filtro predefinido
                // Regresamos al filtro predefinido (o de usuario)
                Rueda.ChangeFilter(param.Rueda.posFiltro);
			    return -1;
		    }
		    put_pointlist(plist, i, esquina);	// Se guarda su posición en la lista.
		    printf("\n");
        }
	}
	
	get_plane_rg(plist, plano);		// Calculamos el plano de regresión


    double Zplano, errorZ;
	for (i=0; i<PUNTOS_PLANO_REGRESION; i++)
	{
        Zplano = plano.A * plist.pointn[i].coord[X_] + plano.B * plist.pointn[i].coord[Y_] + plano.C;
        errorZ = plist.pointn[i].coord[Z_] - Zplano;
        printf("punto %d: [%.2lf,%.2lf,%.2lf]  Zplano:%.2lf  errorZ: %.2lf\n", i+1, plist.pointn[i].coord[X_],
            plist.pointn[i].coord[Y_], plist.pointn[i].coord[Z_], Zplano, errorZ);
    }

	free_pointlist(plist);				// Liberamos la memoria empleada

    // OJO: esto se hace porque puede ser que el usuario haya enfocado manualmente antes
    // Estamos en el filtro "de enfoque" y sin embargo zPredefinido debe ser calculado para el pancromatico
    double zPancromatico = mspWhere(Z_) + param.Rueda.diferencia_z[param.Rueda.posFiltro] - param.Rueda.diferencia_z[param.filtro[param.Rueda.posEnfoque]] ;
    Rueda.zPredefinido = zPancromatico; //actualizo el valor absoluto de z del filtro predefinido
    // Regresamos al filtro predefinido (o de usuario)
    Rueda.ChangeFilter(param.Rueda.posFiltro);

    return 0;
}
#endif


/*************************  haz_movimiento  *********************************
Sin plano de regresion y sin ajuste de exposicion
INTERFAZ (para BORLAND)
*****************************************************************************/
int haz_movimiento(parametros *param, ppoint inicio, ppoint final)
{
	return haz_movimiento(param,inicio, final,NULL, "PAT_CL");
}
/*************************  haz_movimiento  *********************************
Efectuamos el barrido automático desde "inicio" y desplazandonos 
en "meandro" hasta alcanzar "fin"
    planoRegresion - será NULL si no hay plano de regresion definido
    bExposicionAjustada - si false, habrá que ajustar la exposicion justo antes de comenzar el barrido
Devuelve '-1' si se ha cancelado la toma
AUXILIAR
*****************************************************************************/
int haz_movimiento(parametros *param, ppoint inicio, ppoint final, plane* planoRegresion,char* tipoPat)
{
	int dirX, dirY;
	double saltoX = 0, saltoY = 0, X = 0, Y = 0;
    double Z = 0;
	char mensaje[LONGITUD_TEXTO], *fn="haz_movimiento";

	//	Resevo memoria para los buffers que se utilizarán en el proceso de acumulación
	//	de imágenes de las diferentes bandas. 
	ini_TomaPatrones(param);

	double cota;
	if (planoRegresion == NULL)
	{
		// Si no hay plano de regresion, establecemos una cota media y posicionamos la coordenada Z
		cota = (inicio->coord[Z_] + final->coord[Z_]) / 2;
        mspGoCheck(Z_, cota + BACKSLASHZ);
		mspGoCheck(Z_, cota);
        Rueda.zPredefinido = cota;
	}
	
	// Determino el salto y sentido del movimiento
    if (param->BarridoPatrones.camposEjeX == 1)
    {
        saltoX = 0;
    }
    else
    {
	    saltoX = abs(inicio->coord[X_] - final->coord[X_]) / (param->BarridoPatrones.camposEjeX-1);
    }
    if (param->BarridoPatrones.camposEjeY == 1)
    {
        saltoY = 0;
    }
    else
    {
	    saltoY = abs(inicio->coord[Y_] - final->coord[Y_]) / (param->BarridoPatrones.camposEjeY-1);
    }
	sprintf(mensaje, "-Salto X: %.2lf  -SaltoY: %.2lf", saltoX, saltoY);
	sendlog(fn, mensaje);

	dirX = (inicio->coord[X_] > final->coord[X_])? -1 : 1;
	dirY = (inicio->coord[Y_] > final->coord[Y_])? -1 : 1;
	sprintf(mensaje, "-DirX: %d  -DirY: %d", dirX, dirY);
	sendlog(fn, mensaje);

	// Recorrido de la región, tomando imágenes de cada banda en "ZIG-ZAG"
	campoPat = 0;
	for(int i =0; i < param->BarridoPatrones.camposEjeY; i++)  {
		Y = inicio->coord[Y_] + dirY * i * saltoY;
		mspGoCheck(Y_, Y);
		for(int j=0; j < param->BarridoPatrones.camposEjeX; j++)  {
            if (avanza_y_toma_campo(campoPat, 
                                    inicio,
                                    X, Y, Z, 
                                    i, j, dirX, saltoX,
                                    param, planoRegresion, tipoPat) == -1)
            {
				fin_TomaPatrones(param->nBandas);
				return -1;
            }
		}
		i++;
        if (i>=param->BarridoPatrones.camposEjeY) //No hay que "volver"
            break;
		Y = inicio->coord[Y_] + dirY * i * saltoY;
		mspGoCheck(Y_, Y);
		for(j=param->BarridoPatrones.camposEjeX-1; j >= 0; j--)  {
            if (avanza_y_toma_campo(campoPat, 
                                    inicio,
                                    X, Y, Z, 
                                    i, j, dirX, saltoX,
                                    param, planoRegresion, tipoPat) == -1)
            {
				fin_TomaPatrones(param->nBandas);
				return -1;
            }
		}
	}

    //Factor por el que hay que multiplicar una unica toma (sin acumular) para alcanzar
    //la profundidad deseada. Si 8bit -> 1. Si 12bit-> 16
    ASSERT(param->Cam.profundidad >= BITS_CAMARA);
    int factorCamara = 1 << (param->Cam.profundidad - BITS_CAMARA); //Si 8bit -> 1. Si 12bit-> 16
    // Factor por el que hay que dividir la imagen acumulada para alcanzar la profundidad deseada teniendo en cuenta la acumulacion pero no el promediado de campos
    double factorMult = (double)factorCamara / (double)param->BarridoPatrones.nImagenAcum;
  
    long histo_temp[4096];
        // M_banda_acum[i] -> promediado -> M_banda[i]
	for (i=0; i < param->nBandas; i++)  
    {
        if (strcmp(tipoPat,"PAT_CL")==0) // PATRON REFLECTANCIA ALTA
        {
            evalua_histograma(M_cont_campos[i], histo_temp);
            if (histo_temp[0]!=0) // Existen píxeles que no tienen valor acumulado
            {
                // Ponemos el valor acumulado sin enmascarar en los píxeles que no han acumulado ningún valor
                MimBinarize(M_cont_campos[i], M_mascara_aux, M_EQUAL, 0, M_NULL);
                MimShift(M_mascara_aux, M_mascara_aux, -7);
    		    MimArith(M_banda_acum[i], M_mascara_aux, M_banda_acum[i], M_MULT);
       		    MimArith(M_banda_acum[i], M_banda_enmascarada_acum[i], M_banda_enmascarada_acum[i], M_ADD);
                // Corregimos el contador en los píxeles que no han acumulado ningún valor
                MimArith(M_mascara_aux, param->BarridoPatrones.camposEjeX * param->BarridoPatrones.camposEjeY, M_mascara_aux, M_MULT_CONST);
       		    MimArith(M_mascara_aux, M_cont_campos[i], M_cont_campos[i], M_ADD);
            }
            // Calculamos el valor promedio de los valores válidos para cada píxel
		    MimArith(M_banda_enmascarada_acum[i], M_cont_campos[i], M_banda_enmascarada_acum[i], M_DIV); //promediado
            //El contador de campos validos (para cada pixel) se convertirá en el factor de division para promediar
            MimArith(M_banda_enmascarada_acum[i], factorMult, M_banda_enmascarada_acum[i], M_MULT_CONST);
        }
        else //PATRON REFLECTANCIA BAJA
        {
		    MimArith(M_banda_enmascarada_acum[i], factorMult / (param->BarridoPatrones.camposEjeX * param->BarridoPatrones.camposEjeY), 
                M_banda_enmascarada_acum[i], M_MULT_CONST); //promediado
        }

		MbufCopy(M_banda_enmascarada_acum[i], M_banda[i]); //conversion a 16 bit o 8 bit (segun BITS_CAMARA)
    }

    if (param->Cam.profundidad == 16)
    {
        // El display estaba preparado para mostrar buffers de 12bit
        // ahora hay que configurarlo para mostrar buffers de 16bit
        MdispControl(M_display_normal,M_VIEW_BIT_SHIFT,8); //despreciamos los 8 bits menos significativos
    }

	//	Limpio el buffer de overlay
	borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);

    // Dejo la rueda en el filtro por defecto
    Rueda.ChangeFilter(param->Rueda.posFiltro);

	//	Libero la memoria MIL reservada en esta función
	fin_TomaPatrones(param->nBandas);

	return 0;
}

/**************************  avanza_y_toma_campo  *********************************
Funcion creada unicamente por reutilizacion de codigo
AUXILIAR a haz_movimiento
*****************************************************************************/
int avanza_y_toma_campo(int& campoPat, 
                        ppoint inicio,
                        double& X, double& Y, double& Z, 
                        int i, int j, int dirX, double saltoX,
                        parametros* param, plane* planoRegresion, char* tipoPat)
{
    campoPat++;
    X = inicio->coord[X_] + dirX * j * saltoX;
    mspGoCheck(X_, X);
    if (planoRegresion != NULL)
    {
        // Z de enfoque (para el filtro de enfoque) segun el plano de regresion
        Z = planoRegresion->A * X + planoRegresion->B * Y + planoRegresion->C;
        // Hay que actualizar la z del filtro predefinido (a partir de la Z del filtro de enfoque)
        // actualizo la z absoluta del filtro predefinido (calculandola a partir del filtro de enfoque)
        Rueda.zPredefinido = Z + param->Rueda.diferencia_z[param->Rueda.posFiltro] - 
                                 param->Rueda.diferencia_z[param->filtro[param.Rueda.posEnfoque]]; 
	    mspGoCheck(Z_, Z + BACKSLASHZ);
	    mspGoCheck(Z_, Z);
    }
    muestra_tomaPat(M_overlay_normal, j, i, false);
    if (toma_campoPat(param, tipoPat)) 
	    return -1;

    muestra_tomaPat(M_overlay_normal, j, i, true);
    #if !defined (__BORLANDC__)
    if (getKey() == K_ESC)  {
    #else
    if (TomaPatCancelada())  {
    #endif
	    return -1;
    }

    return 0;
}

#if !defined (__BORLANDC__)
/**************************  pideP  *********************************
Espera a que el usuario pulse ENTER y devuelve el punto (X,Y,Z) actual en 'p'
Devuelve 0 o -1 si el usuario pulso ESC
AUXILIAR
SOLO VISUAL
*****************************************************************************/
int pideP(point *p)
{
	int t;
	char mensaje[LONGITUD_TEXTO];

	do {
		*p = mspWhereP();
		p->peso=1;
		sprintf(mensaje, "X: %8.2lf Y: %8.2lf Z: %8.2lf",
			p->coord[X_], p->coord[Y_], p->coord[Z_]);
		sendcont(mensaje);
		t = getKey();
		if (t == K_ESC)			return -1;
		Sleep(200);
	}while (t == 0);		// Espera hasta que no pulse una tecla

	return 0;
}
#endif

/**************************  CalcularMascara  *********************************
Toma una imagen y calcula los valores de su histograma que son demasiado oscuros
y genera la mascara con ceros y unos correspondiente
AUXILIAR a toma_campoPat
*****************************************************************************/
void CalcularMascara(parametros *param, int nBanda, MIL_ID buffer)
{
    // Hallar moda y sigma inferior
	long moda;
	double sigmaSup, sigmaInf;
    // buffer es M_acum_aux, que es de 32 bit, pero como suponemos que nunca alcanzará mas de 16bit reales
    long histo[65536];//4294967296  pow(2,profundidad)
    memset(histo,0,65536*sizeof(long));
    // No podemos usar evalua_histograma porque no esta preparado para buffers de 16 bit
    MIL_ID M_histo16;
	MimAllocResult(M_sistema, 65536, M_HIST_LIST, &M_histo16);
	MimHistogram(buffer, M_histo16);
	MimGetResult(M_histo16, M_VALUE, histo);
	moda = moda_histograma(histo, 0, 65536-1);
	asimetria_histograma(histo, moda, &sigmaInf, &sigmaSup);
	MimFree(M_histo16);

    // Calcular mascara
    int nUmbral;
    nUmbral =(int)(moda - param->BarridoPatrones.rango_util[nBanda]*param->BarridoPatrones.nImagenAcum);
//    MimBinarize(buffer,M_mascara_aux,M_GREATER_OR_EQUAL,nUmbral,M_NULL);//pixeles validos valdran 0xff
    MimBinarize(buffer,M_mascara_aux,M_IN_RANGE,nUmbral,(4096*param->BarridoPatrones.nImagenAcum) - 3);//pixeles validos valdran 0xff, -3 porque la camara no da el valor maximo, si no el anterior y porque M_IN_RANGE es inclusivo

    MimShift(M_mascara_aux,M_mascara_aux,-7); //pixeles validos valdran 1

    //Sumar contador
    MimArith(M_mascara_aux, M_cont_campos[nBanda], M_cont_campos[nBanda], M_ADD);

//Marcar campos en overlay
//HACER: BORRAR o repintar campos recorridos
MbufClear(M_overlay_normal, TRANSPARENTE);
MimArith(M_overlay_normal,1,M_overlay_normal,M_SUB_CONST);
MimArith(M_overlay_normal,M_mascara_aux,M_overlay_normal,M_ADD);

    long histo_temp[256];
	evalua_histograma(M_mascara_aux, histo_temp);

	char texto[LONGITUD_TEXTO];
    sprintf(texto, "Mascara campo: %d moda:%d sigma %8.2lf Umbral: %d ceros: %d unos: %d\n",
			campoPat, moda,sigmaInf,nUmbral ,histo_temp[0],histo_temp[1] );
	sendcont(texto);

}

/**************************  toma_campoPat  *********************************
AUXILIAR a avanza_y_toma_campo
*****************************************************************************/
int toma_campoPat(parametros *param, char* tipoPat)
{
	int i;
	char texto[LONGITUD_TEXTO];

    int filtro_antiguo = Rueda.GetFilter();


	for (i=0; i < param->nBandas; i++) 
    {

#if !defined (__BORLANDC__)
		if (getKey() == K_ESC)
#else
        if (TomaPatCancelada())
#endif
			return -1;

		// Cambio de filtro si es necesario
		if ( param->filtro[i] !=  filtro_antiguo)  {
			if (Rueda.ChangeFilter(param->filtro[i]) == false)
				return -1;
            filtro_antiguo = param->filtro[i];
            toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
            toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
		}

        toma2_acumulada(M_acum_aux, param->BarridoPatrones.nImagenAcum);

		MimArith(M_acum_aux, M_banda_acum[i], M_banda_acum[i], M_ADD); //Acumulamos la imagen completa

        if (strcmp(tipoPat,"PAT_CL")==0) // PATRON REFLECTANCIA ALTA
        {
            // Despues de acumular, calculamos la mascara para esta banda
            CalcularMascara(param,i,M_acum_aux);

            MimArith(M_acum_aux, M_mascara_aux, M_acum_aux, M_MULT); //se ponen a 0 los pixeles no validos
        }

		MimArith(M_acum_aux, M_banda_enmascarada_acum[i], M_banda_enmascarada_acum[i], M_ADD); //Acumulamos la imagen enmascarada


		sprintf(texto, "Campo: %d Banda: %d  Filtro: %d  (%8.2lf, %8.2lf, %8.2lf) zPred: %8.2lf\n",
			campoPat, i, param->filtro[i]+1, mspWhere(X_), mspWhere(Y_), mspWhere(Z_), Rueda.zPredefinido);
		sendcont(texto);
		toma2_continua(M_imagen1);
	}

	return 0;
}


/**************************  muestra_TomaPat  *******************************
AUXILIAR a avanza_y_toma_campo
*****************************************************************************/
void muestra_tomaPat(MIL_ID M_fb, int x, int y, bool solido)
{
	long alto, ancho;

	MbufInquire(M_fb, M_SIZE_X, &ancho);
	MbufInquire(M_fb, M_SIZE_Y, &alto);

	int iniX = ancho/2 - (3*SALTO_DIBUJO) / 2;
	int iniY = alto/2 - (3*SALTO_DIBUJO) / 2;

	int pX = iniX + SALTO_DIBUJO * x;
	int pY = iniY + SALTO_DIBUJO * y;

	if (solido)  {
		MgraColor(M_DEFAULT, MAGENTA_OSC);
		MgraRectFill(M_DEFAULT, M_overlay_normal, pX-SEMILADO_RECT, pY-SEMILADO_RECT,
			pX+SEMILADO_RECT, pY+SEMILADO_RECT);
	}
	else  {
		MgraColor(M_DEFAULT, MAGENTA);
		MgraRect(M_DEFAULT, M_overlay_normal, (pX-SEMILADO_RECT)-1, (pY-SEMILADO_RECT)-1,
			(pX+SEMILADO_RECT)+1, (pY+SEMILADO_RECT)+1);
	}
}


/**************************  captura_imagenAcum  ****************************
#if !defined (__BORLANDC__)
	Función para tomar 'n' imagenes del patron con los diferentes filtros
	y promediarla.
    AUXILIAR
    SOLO VISUAL
    OBSOLETO BORRAR SIN USO
*****************************************************************************
int captura_imagenAcum(parametros *param, DtaRevolver& Rueda)
{
	int i, n, m, tecla;
    int filtro_antiguo; // para saber si hay que cambiar el enfoque (z) en el bucle
	char mensaje[1024], linea[256], texto[256];
	double voltActLamp;		// Variable para comprobar la posición actual de la lámpara
	CString posActFiltro;	// Variable para averiguar la posición actual de los filtros

	//	Resevo memoria para los buffers que se utilizarán en el proceso de acumulación
	//	de imágenes de las diferentes bandas.
	ini_TomaPatrones(param);

	MbufClear(M_imagen_acum, 0);	// Vacio el buffer de acumulación.

	sprintf(mensaje, "Toma imágenes del patron para determinar su imagen promedio.\n");
	sprintf(linea, "   - [Intro] para tomar una imagen.\n");
	strcat(mensaje, linea);
	sprintf(linea, "   - [Esc] para acabar.\n");
	strcat(mensaje, linea);
	sendop(mensaje);

	n = 0;					// Inicializo el contador del número de imágenes.
	sprintf(mensaje, "Número de imágenes tomadas: %d \t\t\t", n);
	sendcont(mensaje);		// Muesto en pantalla en número de imágenes que promedio
	do  {
		tecla = getKey();
		switch(tecla)  {
			case K_CR:

				//	Compruebo la intensidad de la lámpara antes de almacenar la imagen.
				voltActLamp = mspGetLamp();
				if ( fabs(param->Mtb.voltLamp - voltActLamp ) > 0.1)  {
					sprintf(mensaje, "voltaje actual de la lámpara: %.2lf", voltActLamp);
					error_leve("captura_imagen", mensaje);
					sendop("Fije la lámpara en el voltaje deseado.\n");
					do  {
						voltActLamp = mspGetLamp(0);
						sprintf(mensaje, "Volt. prefijado: %.2lf -- Volt. actual: %.2lf",
							param->Mtb.voltLamp, voltActLamp);
						sendcont(mensaje);
						Sleep(100);
					} while ( fabs(param->Mtb.voltLamp - voltActLamp) > 0.1);
				putch('\a');				// Emito un pitido
				}

				//	Captura de la imagen, acumulando las imágenes obtenidas
				//	en el buffer de 16bits asociado a cada banda. 
				n++;						// Contador
				sprintf(mensaje, "Número de imágenes tomadas: %d \t\t\t ", n);
				sendcont(mensaje);

                filtro_antiguo = Rueda.GetFilter();
				for (i=0; i < param->nBandas; i++) {
					// Cambio de filtro si es necesario
					if ( param->filtro[i] !=  filtro_antiguo)  {
						if (Rueda.ChangeFilter(param->filtro[i]) == false)
							return -1;
                        filtro_antiguo = param->filtro[i];
					}
					printf("\n- Filtro %d", param->filtro[i]);

					for (m=0; m < param->BarridoPatrones.nImagenAcum; m++)  {
						toma2_unica(M_imagen1);
						MimArith(M_imagen1, M_banda_enmascarada_acum[i], M_banda_enmascarada_acum[i], M_ADD);
					}
					sprintf(texto, "BANDA %d  FILTRO %d", i, param->filtro[i]);
					sendop(texto);
					toma2_continua(M_imagen1);
				}

				// Devuelvo la posición de la rueda de filtros a la posición inicial
				if ( param->Rueda.posFiltro !=  filtro_antiguo )  {
					printf("\nCambio al filtro inicial %d...", param->Rueda.posFiltro);
					if (Rueda.ChangeFilter(param->Rueda.posFiltro) == false)
						return -1;
				}
				break;
			case K_ESC:						// Condición de salida
				fin_toma();					// Detenemos la adquisición continua
				break;
			case 0:							// Condición de tecla no pulsada
				Sleep(100);
				break;
			default:						// Tecla incorrecta
				putch('\a');
		}
	}while (tecla != K_ESC);

	if (n == 0)  {	// Sin tomar imágenes devuelve 0
		fin_TomaPatrones(param->nBandas);
		return 0;
	}

	//	Una vez realizada la acumulación de imágenes en el buffer auxiliar,
	//	procedo a dividirlo por el número de imágenes tomadas, redondeando.
	//	La imagen promediada es almacenada en los buffers reservados para cada banda.
	for (i=0; i < param->nBandas; i++)  {
		MimArith(M_banda_enmascarada_acum[i], (n*param->BarridoPatrones.nImagenAcum)/2, M_banda_enmascarada_acum[i], M_ADD_CONST);
		MimArith(M_banda_enmascarada_acum[i], n*param->BarridoPatrones.nImagenAcum, M_banda[i], M_DIV_CONST);
	}

	//	Libero la memoria reservada en esta función (M_bandas_acum). 
	fin_TomaPatrones(param->nBandas);

	return n;			// Devuelvo el número de imágenes tomadas
}
#endif
*/


