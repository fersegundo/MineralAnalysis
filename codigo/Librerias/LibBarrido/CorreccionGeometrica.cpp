
#include "stdafx.h"
#include <mil.h>
#include <math.h>
#include "CorreccionGeometrica.h"
#include "..\ControlMicroscopio\control_microscopio.h"
#include "..\ProcesoAux\gestion_mensajes.h"
#include "..\ProcesoAux\control_proceso_imagenes.h"
#include "..\ProcesoAux\myVars.h"
#include "..\ProcesoAux\funciones_graficas.h" //muestra_punto
#include "control_barrido.h"
#include "..\ControlRevolver\control_revolver_dta.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CCorreccionGeometrica::CCorreccionGeometrica()
{
    m_arrPosicionX = NULL;
    m_arrPosicionY = NULL;
    m_arrEscala = NULL;
    m_dRotacion = -1;
    m_nNumCuadradosX = -1; //se calcula en ProcesarEscalaTranslacion (en ProcesarPolkaCampo)

    m_dEscalaObjetivo = ESCALA_TEORICA_20x; //en ProcesaPolka se corregirá si se usa otro objetivo

}

CCorreccionGeometrica::~CCorreccionGeometrica()
{
    if (m_arrPosicionX != NULL)
        free(m_arrPosicionX);
    if (m_arrPosicionY != NULL)
        free(m_arrPosicionY);
    if (m_arrEscala != NULL)
        free(m_arrEscala);
}

// INTERFAZ
bool CCorreccionGeometrica::ProcesarPolka(parametros* pParamIni,DtaRevolver& Rueda, MIL_ID overlay)
{
    if (!ProcesarEscalaTranslacion(pParamIni, Rueda, overlay))
        return false;
    if (!ProcesarRotacion(pParamIni,Rueda, overlay))
        return false;

    // guardamos en fichero
    Guardar(*pParamIni);

    return true;
}


// INTERFAZ
bool CCorreccionGeometrica::ProcesarEscalaTranslacion(parametros* pParamIni,DtaRevolver& Rueda, MIL_ID overlay)
{
    //corregimos la escala objetivo segun los aumentos en los que estemos
    int nObjetivo = mspGetObjetivo(); //1..MAX_OBJETIVO
    m_dEscalaObjetivo = ESCALA_TEORICA_20x * 20 / pParamIni->objetivos[nObjetivo-1].dAumento; // si 10x --> x2, si 5x --> x4, si 2.5x --> x8

    if (m_arrPosicionX == NULL)
        m_arrPosicionX = (double *)malloc(pParamIni->nBandas*sizeof(double));
    if (m_arrPosicionY == NULL)
        m_arrPosicionY = (double *)malloc(pParamIni->nBandas*sizeof(double));
    if (m_arrEscala == NULL)
        m_arrEscala = (double *)malloc(pParamIni->nBandas*sizeof(double));


    // CAPTURA. Resultado en M_banda[banda] en 8 o 16bit
    captura_bandas(*pParamIni, 1 /*imagenes a acumular*/, Rueda);

    // Calculamos centros y escalas para este campo.
    if (ProcesarPolkaCampo(pParamIni, pParamIni->Cam.anchoImagen, pParamIni->Cam.altoImagen,
        m_arrPosicionX,m_arrPosicionY,m_arrEscala,
        pParamIni->nBandas,
        m_nNumCuadradosX, 
        overlay) == false)
        return false;
    ASSERT(m_nNumCuadradosX > 0);

    return true;
}

// INTERFAZ
// Es necesario haber llamado a ProcesarEscalaTranslacion antes de llamar a esta funcion
bool CCorreccionGeometrica::ProcesarRotacion(parametros* pParamIni,DtaRevolver& Rueda, MIL_ID overlay)
{
    ASSERT(m_nNumCuadradosX>0);// Es necesario haber llamado a ProcesarEscalaTranslacion antes de llamar a esta funcion
    ASSERT(m_arrPosicionX != NULL);// Es necesario haber llamado a ProcesarEscalaTranslacion antes de llamar a esta funcion
    ASSERT(m_arrPosicionY != NULL);// Es necesario haber llamado a ProcesarEscalaTranslacion antes de llamar a esta funcion
    ASSERT(m_arrEscala != NULL);// Es necesario haber llamado a ProcesarEscalaTranslacion antes de llamar a esta funcion

    if (overlay != NULL)
        MbufClear(overlay, TRANSPARENTE);

    // Calculamos angulo de rotacion (promediando entre los dos pares de cuadrados)
    //   <--------- Movimiento platina en x
    //
    //   Antes              Despues
    //
    //  1+ - - - 3         1 - - - 3+
    //               -->   
    //  2* - - - 4         2 - - - 4*
    //
    double dCuadroDerechoX_Antes = -1;
    double dCuadroDerechoY_Antes = -1;
    double dCuadroDerechoX_Despues = -1;
    double dCuadroDerechoY_Despues = -1;
    // Obtenemos el cuadrado mas a la izquierda que este centrado en la imagen (para asegurarnos que no se salga de la imagen al desplazarnos)
    ObtenerCuadradoProximoA(0, //a la izquierda del todo
                            pParamIni->Cam.altoImagen/2, // centrado en Y 
                            dCuadroDerechoX_Antes, dCuadroDerechoY_Antes);

    if (overlay != NULL)
        muestra_punto((int)dCuadroDerechoX_Antes,(int)dCuadroDerechoY_Antes,5,VERDE,0);

    // Mover hacia atras un cuadrado para calcular la posicion aproximada en Y del cuadro despues del movimiento completo
    mspGoCheck(X_,mspWhere(X_)-DISTANCIA_ENTRE_CUADRADOS); 

    // CAPTURA. Resultado en M_banda[banda] en 8 o 16bit
    int filtro_ant = Rueda.GetFilter();
    captura_banda(BANDA_PARA_ROTACION,*pParamIni, 1, Rueda, filtro_ant);

    ObtenerCuadradoProximoA(dCuadroDerechoX_Antes + (DISTANCIA_ENTRE_CUADRADOS / m_dEscalaObjetivo), //el cuadrado estará tanto a la derecha como pixeles nos hayamos desplazado
                            dCuadroDerechoY_Antes, // lo buscamos donde estaba antes el otro (no se habrá desplazado mucho en Y)
                            dCuadroDerechoX_Despues, dCuadroDerechoY_Despues);
    if (overlay != NULL)
        muestra_punto((int)dCuadroDerechoX_Despues,(int)dCuadroDerechoY_Despues,3,VERDE,0);

    if (m_nNumCuadradosX > 1) 
    {
        // Completar el movimiento
        mspGoCheck(X_,mspWhere(X_) - DISTANCIA_ENTRE_CUADRADOS * (m_nNumCuadradosX-1)); //-1 porque ya nos hemos desplazado un cuadrado antes 

        // CAPTURA. Resultado en M_banda[banda] en 8 o 16bit
        captura_banda(BANDA_PARA_ROTACION,*pParamIni, 1, Rueda, filtro_ant);

        //Buscar el mismo cuadrado (que ahora estará a la derecha del todo)
        ObtenerCuadradoProximoA(pParamIni->Cam.anchoImagen-1, //a la derecha del todo
                                dCuadroDerechoY_Antes + (dCuadroDerechoY_Despues-dCuadroDerechoY_Antes)*m_nNumCuadradosX,      // se habrá desplazado en Y tanto como se haya desplazado en el movimiento anterior, pero multiplicado por el numero de cuadrados
                                dCuadroDerechoX_Despues, 
                                dCuadroDerechoY_Despues);
    }

    //*nNumCuadradosX para el calculo del angulo (los cuadrados extremos izquierdos serán los cuadrados extremos derechos despues del movimiento)

    if (dCuadroDerechoX_Despues == dCuadroDerechoX_Antes)
        return false;
    m_dRotacion = atan((dCuadroDerechoY_Despues-dCuadroDerechoY_Antes)/(dCuadroDerechoX_Despues-dCuadroDerechoX_Antes));
    m_dRotacion *= 180 / PI; // Radianes a grados
    // NOTA: atan devuelve el valor en radianes

    if (overlay != NULL)
    {
        muestra_punto((int)dCuadroDerechoX_Despues,(int)dCuadroDerechoY_Despues,5,VERDE,0);

        //dibujamos el triangulo
	    MgraColor(M_DEFAULT, AZUL);
	    MgraLine(M_DEFAULT, overlay, (int)dCuadroDerechoX_Antes, (int)dCuadroDerechoY_Antes, (int)dCuadroDerechoX_Despues, (int)dCuadroDerechoY_Despues);
        Sleep(500);
	    MgraLine(M_DEFAULT, overlay, (int)dCuadroDerechoX_Antes, (int)dCuadroDerechoY_Antes, (int)dCuadroDerechoX_Despues, (int)dCuadroDerechoY_Antes);
	    MgraLine(M_DEFAULT, overlay, (int)dCuadroDerechoX_Despues, (int)dCuadroDerechoY_Antes, (int)dCuadroDerechoX_Despues, (int)dCuadroDerechoY_Despues);
        char rot[12];
        sprintf(rot,"%.2lf grados",m_dRotacion);
	    MgraFont(M_DEFAULT, M_FONT_DEFAULT_LARGE);
        dibuja_texto(overlay, rot, (int)dCuadroDerechoX_Antes, (int)dCuadroDerechoY_Antes, 0);
	    MgraFont(M_DEFAULT, M_FONT_DEFAULT);
    }
    
    //	Una vez capturadas las imágenes, vuelvo a la posición de enfoque (filtro predeterminado)
	if ( filtro_ant != pParamIni->Rueda.posFiltro )  {
		if (Rueda.ChangeFilter(pParamIni->Rueda.posFiltro) == false)
			return false;
	}

    ASSERT(m_dRotacion != -1);
    return (m_dRotacion != -1);

}

// probando colorcitos
int g_arrR[14];
int g_arrG[14];
int g_arrB[14];



//Se calculan los centros de gravedad y escala para el campo actual
// nNumCuadrados - aqui se devuelve el numero de cuadrados (mejor dicho distancias entre cuadrados) en X
bool CCorreccionGeometrica::ProcesarPolkaCampo(parametros* pParamIni,
                                               int nAnchoImagen, int nAltoImagen,
                                               double arrCentroX[],
                                               double arrCentroY[],
                                               double arrEscala[],
                                               int nBandas,
                                               /*OUT*/ int& nNumCuadrados,
                                               MIL_ID overlay)
{
//Visible
g_arrB[0] = 128;
g_arrG[0] = 128;
g_arrR[0] = 128;

//400
g_arrB[1] = 0;
g_arrG[1] = 0;
g_arrR[1] = 255;

//450
g_arrB[2] = 0;
g_arrG[2] = 255;
g_arrR[2] = 0;

//500
g_arrB[3] = 255;
g_arrG[3] = 0;
g_arrR[3] = 0;

//550
g_arrB[4] = 0;
g_arrG[4] = 255;
g_arrR[4] = 255;

//600
g_arrB[5] = 255;
g_arrG[5] = 0;
g_arrR[5] = 255;

//650
g_arrB[6] = 255;
g_arrG[6] = 255;
g_arrR[6] = 0;

//700
g_arrB[7] = 255;
g_arrG[7] = 255;
g_arrR[7] = 255;

//750
g_arrB[8] = 192;
g_arrG[8] = 192;
g_arrR[8] = 192;

//800
g_arrB[9] = 0;
g_arrG[9] = 0;
g_arrR[9] = 192;

//850
g_arrB[10] = 0;
g_arrG[10] = 192;
g_arrR[10] = 0;

//900
g_arrB[11] = 192;
g_arrG[11] = 0;
g_arrR[11] = 0;

//950
g_arrB[12] = 0;
g_arrG[12] = 192;
g_arrR[12] = 192;

//1000
g_arrB[13] = 192;
g_arrG[13] = 0;
g_arrR[13] = 192;

    double x[NUM_CUADRADOS] = {0}; //almacenaremos los centros de gravedad de los cuadrados
    double y[NUM_CUADRADOS] = {0};

    double dEscalaAproximada = -1; //necesaria para calcular la escala exacta porque no sabemos cuantos cuadrados hay entre los cuadrados que hacen esquina
    double arrDistanciasSumadas[4] = {0};
    double dNumDistancias = -1;

    // Calculos previos al calculo del centro de gravedad corrigiendo la escala
    double dCentroImagenX = nAnchoImagen/2;
    double dCentroImagenY = nAltoImagen/2;

    for (int i=0;i<nBandas;i++)
    {
        if (CalcularCentrosCuadrados(i, x, y, arrCentroX[i], arrCentroY[i], dEscalaAproximada, overlay) == false)
            return false;

        // Calcular escala
        // Calcular cuantos cuadrados hay, utilizando los de abajo, luego los de arriba, luego los de la derecha y luego los de la izquierda (por separado y luego promediamos)
        arrDistanciasSumadas[0] = sqrt((x[2] - x[0]) * (x[2] - x[0]) + (y[2] - y[0]) * (y[2] - y[0]));
        arrDistanciasSumadas[1] = sqrt((x[3] - x[1]) * (x[3] - x[1]) + (y[3] - y[1]) * (y[3] - y[1]));
//        arrDistanciasSumadas[2] = y[1] - y[0];
//        arrDistanciasSumadas[3] = y[3] - y[2];
        arrEscala[i] = 0;
        for (int d=1;d>=0;d--) //al reves porque es mas eficiente y para que el ultimo calculo sea el de x[2] - x[0] 
        {
            // NumDistancias es el numero de distancias inter-cuadrado que hay sumadas en entre cada una de los cuatro cuadrados extremos (por ejemplo si es 4x3, seria 10)
            dNumDistancias = arrDistanciasSumadas[d] * m_dEscalaObjetivo / DISTANCIA_ENTRE_CUADRADOS;
            nNumCuadrados = (int)floor(dNumDistancias+0.5);

            arrEscala[i] += (nNumCuadrados * DISTANCIA_ENTRE_CUADRADOS) / arrDistanciasSumadas[d] ;
        }
        arrEscala[i] = arrEscala[i] / 2;

        // Calcular centro de gravedad de los cuadrados corrigiendo la escala
        // Es decir, la media de los centros de gravedad si no hubiese efecto de cambio de escala (los centros pueden variar debido a la escala y nosotros queremos caracterizar unicamente la traslacion)
        arrCentroX[i] = ((arrCentroX[i] - dCentroImagenX) * m_dEscalaObjetivo / arrEscala[i]) + dCentroImagenX;
        arrCentroY[i] = ((arrCentroY[i] - dCentroImagenY) * m_dEscalaObjetivo / arrEscala[i]) + dCentroImagenY;

        if (overlay != NULL)
        {
            char onda[12];
            if (i == 0)
                sprintf(onda,"Visible");
            else
                sprintf(onda,"%4d nm",pParamIni->Rueda.espectro[i]);
            dibuja_cruz(M_overlay_normal,(int)arrCentroX[i],(int)arrCentroY[i],M_RGB888(g_arrR[i],g_arrG[i],g_arrB[i]), 15, 0);
            MgraColor(M_DEFAULT, M_RGB888(g_arrR[i],g_arrG[i],g_arrB[i]));
	        MgraFont(M_DEFAULT, M_FONT_DEFAULT_LARGE);
            dibuja_texto(overlay, onda, 410, 328+32*i, 0);
	        MgraFont(M_DEFAULT, M_FONT_DEFAULT);

        }

        // En este punto nNumCuadrados tiene el numero de inter-cuadrados en X
    }

    return true;
}

void CCorreccionGeometrica::ObtenerCuadradoProximoA(double dInicioX, double dInicioY,
                             double& dCercanoX, double& dCercanoY)
{
    // Calculos MIL blobs y centros de gravedad
    //
    MIL_ID MilBlobResult;   
    // Allocate a blob result buffer. 
    MblobAllocResult(M_DEFAULT_HOST, &MilBlobResult); 
    long TotalBlobs = EncontrarBlobs(BANDA_PARA_ROTACION, MilBlobResult);
 
    double *CogX, *CogY; // centros de gravedad
    if ((CogX = (double *)malloc(TotalBlobs*sizeof(double))) && 
        (CogY = (double *)malloc(TotalBlobs*sizeof(double)))
        )
    { 
        // Get the results. 
        memset(CogX,0,TotalBlobs*sizeof(double));
        memset(CogY,0,TotalBlobs*sizeof(double));
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_X, CogX); 
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_Y, CogY); 

        int nIndiceEncontrado = 0;
        double dDistanciaEncontrado = CogX[nIndiceEncontrado] + CogY[nIndiceEncontrado];
        double dSumaInicio = dInicioX + dInicioY; //por ahorrar en calculo
        for (int n=1; n<TotalBlobs;n++)
        {
            double dDistancia = fabs(CogX[n] - dInicioX) + fabs(CogY[n] - dInicioY); //Distancia manhatan por ahorrar en calculo
            if (dDistancia < dDistanciaEncontrado) 
            {
                nIndiceEncontrado = n;
                dDistanciaEncontrado = dDistancia;
            }
        }

        dCercanoX = CogX[nIndiceEncontrado];
        dCercanoY = CogY[nIndiceEncontrado];

        free(CogX);
        free(CogY);
    }

    MblobFree(MilBlobResult);
}

// Binariza y calcula centros de gravedad, limites y areas por cada blob. 
// Obvia los blobs menores de AREA_MINIMA_CUADRADO y los que esten en contacto con el borde
// para la imagen de banda nBanda
// Devuelve en numero de Blobs encontrados
long CCorreccionGeometrica::EncontrarBlobs(int nBanda, MIL_ID MilBlobResult)
{
     
    int nAnchoImagen = MbufInquire(M_banda[nBanda], M_SIZE_X, M_NULL);
    int nAltoImagen  = MbufInquire(M_banda[nBanda], M_SIZE_Y, M_NULL);

    // Calculos MIL centros de gravedad
    //
    MIL_ID MilBlobFeatureList;   

    // Allocate a binary image buffer for fast processing.
    MIL_ID MilBinImage;
    MbufAlloc2d(M_DEFAULT_HOST, nAnchoImagen, nAltoImagen, 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);

//    MimBinarize(M_banda[nBanda], MilBinImage, M_LESS_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son negros y el fondo blanco
    MimBinarize(M_banda[nBanda], MilBinImage, M_GREATER_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son blancos y el fondo gris o negro
//    MimBinarize(M_banda[nBanda], MilBinImage, M_IN_RANGE,70,255); //para  cuando los cuadrados son blanco y el fondo gris o negro
//MbufExport("D:\\binarizado.tif",M_TIFF,MilBinImage);
    // Allocate a feature list. 
    MblobAllocFeatureList(M_DEFAULT_HOST, &MilBlobFeatureList); 
    // Enable the Area and Center Of Gravity feature calculation. 
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY_X + M_SORT1_UP ); // ORDENADO POR X
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY_Y ); 
    MblobSelectFeature(MilBlobFeatureList, M_AREA); 
    MblobSelectFeature(MilBlobFeatureList, M_BOX); 

    MblobControl(MilBlobResult, M_IDENTIFIER_TYPE, M_BINARY ); //Faster
    MblobControl(MilBlobResult, M_SAVE_RUNS, M_DISABLE  ); //Faster -- cannot use MblobFill(), MblobGetLabel(), MblobGetRuns(),or MblobLabel()
    // Calculate selected features for each blob.
    MblobCalculate(MilBinImage, M_NULL, MilBlobFeatureList, MilBlobResult); 
    MblobFree(MilBlobFeatureList);

    // Eliminar blobs pequeños
    MblobSelect(MilBlobResult,M_DELETE,M_AREA,M_LESS,AREA_MINIMA_CUADRADO,M_NULL);

    //Eliminar blobs que toquen el borde
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_X_MIN,M_LESS_OR_EQUAL,PIXELS_BORDE_EXCLUSION,M_NULL);
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_X_MAX,M_GREATER_OR_EQUAL,nAnchoImagen-1-PIXELS_BORDE_EXCLUSION,M_NULL);
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_Y_MIN,M_LESS_OR_EQUAL,PIXELS_BORDE_EXCLUSION,M_NULL);
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_Y_MAX,M_GREATER_OR_EQUAL,nAltoImagen-1-PIXELS_BORDE_EXCLUSION,M_NULL);

    MbufFree(MilBinImage);

    // Get the total number of selected blobs. 
    long TotalBlobs;
    MblobGetNumber(MilBlobResult, &TotalBlobs); 

    return TotalBlobs;

}

// VERSION VALIDA PARA OBJETIVOS MAS PEQUEÑOS DE 20x (PARA CUANDO HAY MAS DE 4 CUADRADOS COMPLETOS)
// AUXILIAR
// Calcula los centros de gravedad de hasta NUM_CUADRADOS cuadrados y devuelve sus coordenadas en x, y
// Se devuelven en orden ascendente
//  -------->
// |  1   3
// |  2   4
// \/
// Para la banda nBanda
bool CCorreccionGeometrica::CalcularCentrosCuadrados(int nBanda, 
                                                     double x[], double y[], 
                                                     double &dCentroX, double &dCentroY, 
                                                     double& dEscalaAproximada, 
                                                     MIL_ID overlay)
{
    int n;

    //Inicializamos resultado
    memset(x,0,sizeof(double)*NUM_CUADRADOS);
    memset(y,0,sizeof(double)*NUM_CUADRADOS);

    // Calculos MIL centros de gravedad
    //
    MIL_ID MilBlobResult;   
    // Allocate a blob result buffer. 
    MblobAllocResult(M_DEFAULT_HOST, &MilBlobResult); 
    long TotalBlobs = EncontrarBlobs(nBanda, MilBlobResult);

    double *CogX, *CogY; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    double *Areas; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    // Read and print the blob's center of gravity. 
    if ((CogX = (double *)malloc(TotalBlobs*sizeof(double))) && 
        (CogY = (double *)malloc(TotalBlobs*sizeof(double))) &&
        (Areas = (double *)malloc(TotalBlobs*sizeof(double)))
        )
    { 
        // Get the results. 
        memset(CogX,0,TotalBlobs*sizeof(double));
        memset(CogY,0,TotalBlobs*sizeof(double));
        memset(Areas,0,TotalBlobs*sizeof(double));
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_X, CogX); 
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_Y, CogY); 
        MblobGetResult(MilBlobResult, M_AREA, Areas); 

        if (TotalBlobs < NUM_CUADRADOS)
        {
            CString mensaje;
            mensaje.Format("Se han encontrado solo %d objetos, cuando se esperaba %d",TotalBlobs,NUM_CUADRADOS);
            error_fatal("ProcesarPolka",(LPCSTR)mensaje,0);
            return false;
        }

        //Calculo aproximado de la escala: sabemos el PORCENTAJE_CUADRADOS_RESPECTO_FONDO del Polka Dot y la DISTANCIA_ENTRE_CUADRADOS, asi que podemos calcular el area teorica de un cuadrado:
        double dAreaCuadradoY_Fondo = DISTANCIA_ENTRE_CUADRADOS * DISTANCIA_ENTRE_CUADRADOS;
        double dAreaCuadrado = dAreaCuadradoY_Fondo * PORCENTAJE_CUADRADOS_RESPECTO_FONDO / 100;
        //La escala será:
        dEscalaAproximada = sqrt(dAreaCuadrado / Areas[0]);

        //Buscamos el cuadrado de "arriba a la izquierda", el de "abajo a la izquierda", el de "abajo a la izquierda" y el de "arriba a la derecha"
        // Lo haremos encontrando el de menor X + Y, mayor X + Y, mayor X - Y y menor X - Y
        int nIndiceArribaIzquierda  = 0; //menor X+Y
        double dMinArribaIzquierda  = CogX[0] + CogY[0]; //menor X+Y
        int nIndiceAbajoDerecha     = 0; //mayor X+Y
        double dMaxAbajoDerecha     = CogX[0] + CogY[0]; //mayor X+Y
        int nIndiceAbajoIzquierda   = 0; //menor X-Y
        double dMinAbajoIzquierda   = CogX[0] - CogY[0]; //menor X-Y
        int nIndiceArribaDerecha    = 0; //mayor X-Y
        double dMaxArribaDerecha    = CogX[0] - CogY[0]; //mayor X-Y
//        if (overlay != NULL)
//            dibuja_cruz(M_overlay_normal,(int)CogX[0],(int)CogY[0],M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]), 5, 0);
        for (n=1; n<TotalBlobs;n++)
        {
//            if (overlay != NULL)
//                dibuja_cruz(M_overlay_normal,(int)CogX[n],(int)CogY[n],M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]), 5, 0);

            if (CogX[n] + CogY[n] > dMaxAbajoDerecha)
            {
                dMaxAbajoDerecha = CogX[n] + CogY[n];
                nIndiceAbajoDerecha = n;
            }
            if (CogX[n] + CogY[n] < dMinArribaIzquierda)
            {
                dMinArribaIzquierda = CogX[n] + CogY[n];
                nIndiceArribaIzquierda = n;
            }
            if (CogX[n] - CogY[n] > dMaxArribaDerecha)
            {
                dMaxArribaDerecha = CogX[n] - CogY[n];
                nIndiceArribaDerecha = n;
            }
            if (CogX[n] - CogY[n] < dMinAbajoIzquierda)
            {
                dMinAbajoIzquierda = CogX[n] - CogY[n];
                nIndiceAbajoIzquierda = n;
            }
        }

        //  -------->
        // |  1   3
        // |  2   4
        // \/
        x[0] = CogX[nIndiceArribaIzquierda];
        y[0] = CogY[nIndiceArribaIzquierda];
        x[1] = CogX[nIndiceAbajoIzquierda];
        y[1] = CogY[nIndiceAbajoIzquierda];
        x[2] = CogX[nIndiceArribaDerecha];
        y[2] = CogY[nIndiceArribaDerecha];
        x[3] = CogX[nIndiceAbajoDerecha];
        y[3] = CogY[nIndiceAbajoDerecha];

        if (overlay != NULL)
        {
            dibuja_aspa(M_overlay_normal,(int)x[0],(int)y[0],M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]), 10, 0);
            dibuja_aspa(M_overlay_normal,(int)x[1],(int)y[1],M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]), 10, 0);
            dibuja_aspa(M_overlay_normal,(int)x[2],(int)y[2],M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]), 10, 0);
            dibuja_aspa(M_overlay_normal,(int)x[3],(int)y[3],M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]), 10, 0);
        }

        // Calcular centro de gravedad de los cuadrados
        dCentroX = 0;
        dCentroY = 0;
        for(n=0; n < NUM_CUADRADOS; n++)
        {
            dCentroX += x[n];
            dCentroY += y[n];
        }
        dCentroX = dCentroX / NUM_CUADRADOS;
        dCentroY = dCentroY / NUM_CUADRADOS;

//        if (overlay != NULL)
//            muestra_punto((int)dCentroX,(int)dCentroY,3,M_RGB888(g_arrR[nBanda],g_arrG[nBanda],g_arrB[nBanda]),0);
        
        free(CogX);
        free(CogY);
        free(Areas);
    }
    else
    {
        error_leve("ProcesarPolka","No hay memoria suficiente");
        return false;
    }

    MblobFree(MilBlobResult);

    return true;
}

/*
// VERSION VALIDA PARA OBJETIVOS MAS PEQUEÑOS DE 20x (PARA CUANDO HAY MAS DE 4 CUADRADOS COMPLETOS)
// AUXILIAR
// Calcula los centros de gravedad de hasta NUM_CUADRADOS cuadrados y devuelve sus coordenadas en x, y
// Se devuelven en orden ascendente
//  -------->
// |  1   3
// |  2   4
// \/
// Para la banda nBanda
bool CCorreccionGeometrica::CalcularCentrosCuadrados(int nBanda, 
                                                     int nAnchoImagen, int nAltoImagen, 
                                                     double x[], double y[], 
                                                     double &dCentroX, double &dCentroY, 
                                                     double& dEscalaAproximada)
{
    //Inicializamos resultado
    memset(x,0,sizeof(double)*NUM_CUADRADOS);
    memset(y,0,sizeof(double)*NUM_CUADRADOS);

    // Calculos MIL centros de gravedad
    //
    MIL_ID MilBlobResult, MilBlobFeatureList;   

    // Allocate a binary image buffer for fast processing.
    MIL_ID MilBinImage;
    MbufAlloc2d(M_DEFAULT_HOST,
    MbufInquire(M_banda[nBanda], M_SIZE_X, M_NULL),
    MbufInquire(M_banda[nBanda], M_SIZE_Y, M_NULL), 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);

//    MimBinarize(M_banda[nBanda], MilBinImage, M_LESS_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son negros y el fondo blanco
    MimBinarize(M_banda[nBanda], MilBinImage, M_GREATER_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son blancos y el fondo gris o negro
//    MimBinarize(M_banda[nBanda], MilBinImage, M_IN_RANGE,70,255); //para  cuando los cuadrados son blanco y el fondo gris o negro
//MbufExport("D:\\binarizado.tif",M_TIFF,MilBinImage);
    // Allocate a feature list. 
    MblobAllocFeatureList(M_DEFAULT_HOST, &MilBlobFeatureList); 
    // Enable the Area and Center Of Gravity feature calculation. 
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY_X + M_SORT1_UP ); // ORDENADO POR X
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY_Y ); 
    MblobSelectFeature(MilBlobFeatureList, M_AREA); 
    MblobSelectFeature(MilBlobFeatureList, M_BOX); 
    // Allocate a blob result buffer. 
    MblobAllocResult(M_DEFAULT_HOST, &MilBlobResult); 
    MblobControl(MilBlobResult, M_IDENTIFIER_TYPE, M_BINARY ); //Faster
    MblobControl(MilBlobResult, M_SAVE_RUNS, M_DISABLE  ); //Faster -- cannot use MblobFill(), MblobGetLabel(), MblobGetRuns(),or MblobLabel()
    // Calculate selected features for each blob.
    MblobCalculate(MilBinImage, M_NULL, MilBlobFeatureList, MilBlobResult); 
    MblobFree(MilBlobFeatureList);

    // Eliminar blobs pequeños
    MblobSelect(MilBlobResult,M_DELETE,M_AREA,M_LESS,AREA_MINIMA_CUADRADO,M_NULL);

    //Eliminar blobs que toquen el borde
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_X_MIN,M_EQUAL,0,M_NULL);
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_X_MAX,M_EQUAL,nAnchoImagen-1,M_NULL);
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_Y_MIN,M_EQUAL,0,M_NULL);
    MblobSelect(MilBlobResult,M_DELETE,M_BOX_Y_MAX,M_EQUAL,nAltoImagen-1,M_NULL);

    // Get the total number of selected blobs. 
    long TotalBlobs;
    MblobGetNumber(MilBlobResult, &TotalBlobs); 
    double *CogX, *CogY; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    double *Areas; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    // Read and print the blob's center of gravity. 
    if ((CogX = (double *)malloc(TotalBlobs*sizeof(double))) && 
        (CogY = (double *)malloc(TotalBlobs*sizeof(double))) &&
        (Areas = (double *)malloc(TotalBlobs*sizeof(double)))
        )
    { 
        // Get the results. 
        memset(CogX,0,TotalBlobs*sizeof(double));
        memset(CogY,0,TotalBlobs*sizeof(double));
        memset(Areas,0,TotalBlobs*sizeof(double));
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_X, CogX); 
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_Y, CogY); 
        MblobGetResult(MilBlobResult, M_AREA, Areas); 

        if (TotalBlobs < NUM_CUADRADOS)
        {
            CString mensaje;
            mensaje.Format("Se han encontrado solo %d objetos, cuando se esperaba %d",TotalBlobs,NUM_CUADRADOS);
            error_leve("ProcesarPolka",(LPCSTR)mensaje);
            return false;
        }

        // variables necesarias por si 0 o TotalBlobs-1 es un cuadrado no entero
        int nPrimerBlob = 0;
        int nUltimoBlob = TotalBlobs -1;

        // Primero, eliminamos los blobs que no sean lo suficientemente grandes
        // SUPONEMOS QUE EL CUADRADO DE ARRIBA A LA IZQUIERDA ESTA ENTERO
        double dAreaReferencia = max(Areas[0],Areas[1]); // como no sabemos si el 0 es el de arriba a la izquierda o el de abajo a la izquierda (que puede no estar entero), pues tomamos como area de referencia (cuadrado entero) el 0 o el siguiente (de la misma columna necesariamente)
        
        //Calculo aproximado de la escala: sabemos el PORCENTAJE_CUADRADOS_RESPECTO_FONDO del Polka Dot y la DISTANCIA_ENTRE_CUADRADOS, asi que podemos calcular el area teorica de un cuadrado:
        double dAreaCuadradoY_Fondo = DISTANCIA_ENTRE_CUADRADOS * DISTANCIA_ENTRE_CUADRADOS;
        double dAreaCuadrado = dAreaCuadradoY_Fondo * PORCENTAJE_CUADRADOS_RESPECTO_FONDO / 100;

        //La escala será:
        dEscalaAproximada = sqrt(dAreaCuadrado / dAreaReferencia);

        double HOLGURA_AREA = 0.8; //en procentaje, cuanto menor puede ser un cuadrado que se considera entero
        for (int n=0; n<TotalBlobs;n++)
        {
            if (Areas[n] < dAreaReferencia * HOLGURA_AREA)
            {
                // eliminamos el area
                Areas[n] = -1;
                CogX[n] = -1;
                CogY[n] = -1;
                if (n==0)
                    nPrimerBlob = 1;
                else if (n==TotalBlobs-1)
                    nUltimoBlob = TotalBlobs-2;
            }
        }

        // Cojer los cuadrados de las esquinas

        //encontrar el grupo con menor X (con cierta holgura porque no estarán exactamente en el mismo lugar)
        // aproximadamente 232 pixels entre centros de gravedad
        // Ordenar en orden ascendente
        //
        //  -------->
        // |  1   3
        // |  2   4
        // \/

        int HOLGURA = 50;
        n = 1; //primer blob que puede ser de la misma columna que el primero
        while(CogX[n] < CogX[nPrimerBlob] + HOLGURA)
        {
            if (CogX[n] == -1)
            {
                // hemos llegado a un cuadrado no entero, esto podemos considerarlo el fin de la columna
                n --; 
                break; 
            }

            n++; //el blob n es de la misma columna, probar con el siguiente
        }
        n--; // nuestro cuadrado será el ultimo de la primera columna
        if (CogX[n] == -1)
        {
            // hemos llegado a un cuadrado no entero, esto podemos considerarlo el fin de la columna
            n --; 
        }
    // el punto nPrimerBlob y n son los dos extremos izquierdos, pero no sabemos cual es el de arriba y cual el de abajo
        if(CogY[nPrimerBlob] < CogY[n])
        {
            // el nPrimerBlob es el de arriba a la izquierda
            x[0] = CogX[nPrimerBlob];
            y[0] = CogY[nPrimerBlob];
            x[1] = CogX[n];
            y[1] = CogY[n];
        }
        else
        {
            // el n es el de arriba a la izquierda
            x[1] = CogX[nPrimerBlob];
            y[1] = CogY[nPrimerBlob];
            x[0] = CogX[n];
            y[0] = CogY[n];
        }

        n = nUltimoBlob-1; //ultimo blob que puede ser de la misma columna que el ultimo
        while(CogX[n] > (CogX[nUltimoBlob] - HOLGURA))
        {
            if (CogX[n] == -1)
            {
                // hemos llegado a un cuadrado no entero, esto podemos considerarlo el fin de la columna
                n ++; 
                break; 
            }
            n--; //el blob n es de la misma columna, probar con el anterior
        }
        n++; //nuestro cuadrado será el primero de la ultima columna 
        if (CogX[n] == -1)
        {
            // el ultimo cuadrado de la columna que queremos es no-entero, hay que coger el siguiente
            n ++; 
        }
        // el punto nUltimoBlob y n son los dos extremos derechos, pero no sabemos cual es el de arriba y cual el de abajo
        if(CogY[nUltimoBlob] < CogY[n])
        {
            x[2] = CogX[nUltimoBlob];
            y[2] = CogY[nUltimoBlob];
            x[3] = CogX[n];
            y[3] = CogY[n];
        }
        else
        {
            x[3] = CogX[nUltimoBlob];
            y[3] = CogY[nUltimoBlob];
            x[2] = CogX[n];
            y[2] = CogY[n];
        }

        // Calcular centro de gravedad de los cuadrados (para calculo posterior de angulo)
        dCentroX = 0;
        dCentroY = 0;
        for(n=0; n < NUM_CUADRADOS; n++)
        {
            dCentroX += x[n];
            dCentroY += y[n];
        }
        // Ordenar en orden ascendente
        //
        //  -------->
        // |  1   3
        // |  2   4
        // \/
        dCentroX = dCentroX / NUM_CUADRADOS;
        dCentroY = dCentroY / NUM_CUADRADOS;
      
        free(CogX);
        free(CogY);
        free(Areas);
    }
    else
    {
        error_leve("ProcesarPolka","No hay memoria suficiente");
        return false;
    }

    MblobFree(MilBlobResult);
    MbufFree(MilBinImage);

    return true;
}
*/
/*
// VERSION UNICAMENTE PARA OBJETIVOS MAS PEQUEÑOS DE 20x (PARA CUANDO HAY MAS DE 4 CUADRADOS COMPLETOS)
// AUXILIAR
// Calcula los centros de gravedad de hasta NUM_CUADRADOS cuadrados y devuelve sus coordenadas en x, y
// Se devuelven en orden ascendente
//  -------->
// |  1   3
// |  2   4
// \/
// Para la banda nBanda
bool CCorreccionGeometrica::CalcularCentrosCuadrados(int nBanda, double x[], double y[], double &dCentroX, double &dCentroY, double& dEscalaAproximada)
{
    //Inicializamos resultado
    memset(x,0,sizeof(double)*NUM_CUADRADOS);
    memset(y,0,sizeof(double)*NUM_CUADRADOS);

    // Calculos MIL centros de gravedad
    //
    MIL_ID MilBlobResult,                  // Blob result buffer identifier.     
    MilBlobFeatureList;             // Feature list identifier.           
    // Allocate a binary image buffer for fast processing.
    MIL_ID MilBinImage;
    MbufAlloc2d(M_DEFAULT_HOST,
        MbufInquire(M_banda[nBanda], M_SIZE_X, M_NULL),
        MbufInquire(M_banda[nBanda], M_SIZE_Y, M_NULL), 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);
//    MimBinarize(M_banda[nBanda], MilBinImage, M_LESS_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son negro y el fondo blanco
    MimBinarize(M_banda[nBanda], MilBinImage, M_GREATER_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son blanco y el fondo gris o negro
//    MimBinarize(M_banda[nBanda], MilBinImage, M_IN_RANGE,70,255); //para  cuando los cuadrados son blanco y el fondo gris o negro
//MbufExport("D:\\binarizado.tif",M_TIFF,MilBinImage);
    // Allocate a feature list. 
    MblobAllocFeatureList(M_DEFAULT_HOST, &MilBlobFeatureList); 
    // Enable the Area and Center Of Gravity feature calculation. 
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY_X + M_SORT1_UP ); // ORDENADO POR X
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY_Y ); 
    MblobSelectFeature(MilBlobFeatureList, M_AREA); 
    // Allocate a blob result buffer. 
    MblobAllocResult(M_DEFAULT_HOST, &MilBlobResult); 
    MblobControl(MilBlobResult, M_IDENTIFIER_TYPE, M_BINARY ); //Faster
    MblobControl(MilBlobResult, M_SAVE_RUNS, M_DISABLE  ); //Faster -- cannot use MblobFill(), MblobGetLabel(), MblobGetRuns(),or MblobLabel()
    // Calculate selected features for each blob.
    MblobCalculate(MilBinImage, M_NULL, MilBlobFeatureList, MilBlobResult); 
    MblobFree(MilBlobFeatureList);
    // Get the total number of selected blobs. *
    long TotalBlobs;
    MblobGetNumber(MilBlobResult, &TotalBlobs); 
    double *CogX, *CogY; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    double *Areas; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    // Read and print the blob's center of gravity. 
    if ((CogX = (double *)malloc(TotalBlobs*sizeof(double))) && 
        (CogY = (double *)malloc(TotalBlobs*sizeof(double))) &&
        (Areas = (double *)malloc(TotalBlobs*sizeof(double)))
        )
    { 
        // Get the results. 
        memset(CogX,0,TotalBlobs*sizeof(double));
        memset(CogY,0,TotalBlobs*sizeof(double));
        memset(Areas,0,TotalBlobs*sizeof(double));
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_X, CogX); 
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_Y, CogY); 
        MblobGetResult(MilBlobResult, M_AREA, Areas); 

        if (TotalBlobs < NUM_CUADRADOS)
        {
            CString mensaje;
            mensaje.Format("Se han encontrado solo %d objetos, cuando se esperaba %d",TotalBlobs,NUM_CUADRADOS);
            error_leve("ProcesarPolka",(LPCSTR)mensaje);
            return false;
        }

        // variables necesarias por si 0 o TotalBlobs-1 es un cuadrado no entero
        int nPrimerBlob = 0;
        int nUltimoBlob = TotalBlobs -1;

        // Primero, eliminamos los blobs que no sean lo suficientemente grandes
        // SUPONEMOS QUE EL CUADRADO DE ARRIBA A LA IZQUIERDA ESTA ENTERO
        double dAreaReferencia = max(Areas[0],Areas[1]); // como no sabemos si el 0 es el de arriba a la izquierda o el de abajo a la izquierda (que puede no estar entero), pues tomamos como area de referencia (cuadrado entero) el 0 o el siguiente (de la misma columna necesariamente)
        
        //Calculo aproximado de la escala: sabemos el PORCENTAJE_CUADRADOS_RESPECTO_FONDO del Polka Dot y la DISTANCIA_ENTRE_CUADRADOS, asi que podemos calcular el area teorica de un cuadrado:
        double dAreaCuadradoY_Fondo = DISTANCIA_ENTRE_CUADRADOS * DISTANCIA_ENTRE_CUADRADOS;
        double dAreaCuadrado = dAreaCuadradoY_Fondo * PORCENTAJE_CUADRADOS_RESPECTO_FONDO / 100;

        //La escala será:
        dEscalaAproximada = sqrt(dAreaCuadrado / dAreaReferencia);

        double HOLGURA_AREA = 0.8; //en procentaje, cuanto menor puede ser un cuadrado que se considera entero
        for (int n=0; n<TotalBlobs;n++)
        {
            if (Areas[n] < dAreaReferencia * HOLGURA_AREA)
            {
                // eliminamos el area
                Areas[n] = -1;
                CogX[n] = -1;
                CogY[n] = -1;
                if (n==0)
                    nPrimerBlob = 1;
                else if (n==TotalBlobs-1)
                    nUltimoBlob = TotalBlobs-2;
            }
        }

        // Cojer los cuadrados de las esquinas

        //encontrar el grupo con menor X (con cierta holgura porque no estarán exactamente en el mismo lugar)
        // aproximadamente 232 pixels entre centros de gravedad
        // Ordenar en orden ascendente
        //
        //  -------->
        // |  1   3
        // |  2   4
        // \/

        int HOLGURA = 50;
        n = 1; //primer blob que puede ser de la misma columna que el primero
        while(CogX[n] < CogX[nPrimerBlob] + HOLGURA)
        {
            if (CogX[n] == -1)
            {
                // hemos llegado a un cuadrado no entero, esto podemos considerarlo el fin de la columna
                n --; 
                break; 
            }

            n++; //el blob n es de la misma columna, probar con el siguiente
        }
        n--; // nuestro cuadrado será el ultimo de la primera columna
        if (CogX[n] == -1)
        {
            // hemos llegado a un cuadrado no entero, esto podemos considerarlo el fin de la columna
            n --; 
        }
    // el punto nPrimerBlob y n son los dos extremos izquierdos, pero no sabemos cual es el de arriba y cual el de abajo
        if(CogY[nPrimerBlob] < CogY[n])
        {
            // el nPrimerBlob es el de arriba a la izquierda
            x[0] = CogX[nPrimerBlob];
            y[0] = CogY[nPrimerBlob];
            x[1] = CogX[n];
            y[1] = CogY[n];
        }
        else
        {
            // el n es el de arriba a la izquierda
            x[1] = CogX[nPrimerBlob];
            y[1] = CogY[nPrimerBlob];
            x[0] = CogX[n];
            y[0] = CogY[n];
        }

        n = nUltimoBlob-1; //ultimo blob que puede ser de la misma columna que el ultimo
        while(CogX[n] > (CogX[nUltimoBlob] - HOLGURA))
        {
            if (CogX[n] == -1)
            {
                // hemos llegado a un cuadrado no entero, esto podemos considerarlo el fin de la columna
                n ++; 
                break; 
            }
            n--; //el blob n es de la misma columna, probar con el anterior
        }
        n++; //nuestro cuadrado será el primero de la ultima columna 
        if (CogX[n] == -1)
        {
            // el ultimo cuadrado de la columna que queremos es no-entero, hay que coger el siguiente
            n ++; 
        }
        // el punto nUltimoBlob y n son los dos extremos derechos, pero no sabemos cual es el de arriba y cual el de abajo
        if(CogY[nUltimoBlob] < CogY[n])
        {
            x[2] = CogX[nUltimoBlob];
            y[2] = CogY[nUltimoBlob];
            x[3] = CogX[n];
            y[3] = CogY[n];
        }
        else
        {
            x[3] = CogX[nUltimoBlob];
            y[3] = CogY[nUltimoBlob];
            x[2] = CogX[n];
            y[2] = CogY[n];
        }

        // Calcular centro de gravedad de los cuadrados (para calculo posterior de angulo)
        dCentroX = 0;
        dCentroY = 0;
        for(n=0; n < NUM_CUADRADOS; n++)
        {
            dCentroX += x[n];
            dCentroY += y[n];
        }
        // Ordenar en orden ascendente
        //
        //  -------->
        // |  1   3
        // |  2   4
        // \/
        dCentroX = dCentroX / NUM_CUADRADOS;
        dCentroY = dCentroY / NUM_CUADRADOS;
      
        free(CogX);
        free(CogY);
        free(Areas);
    }
    else
    {
        error_leve("ProcesarPolka","No hay memoria suficiente");
        return false;
    }

    MblobFree(MilBlobResult);
    MbufFree(MilBinImage);

    return true;
}
*/
/*
/// AUXILIAR
// Calcula los centros de gravedad de hasta NUM_CUADRADOS cuadrados y devuelve sus coordenadas en x, y
// Se devuelven en orden ascendente
//  -------->
// |  1   3
// |  2   4
// \/
// Para la banda nBanda
bool CCorreccionGeometrica::CalcularCentrosCuadrados(int nBanda, double x[], double y[], double &dCentroX, double &dCentroY)
{
    //Inicializamos resultado
    memset(x,0,sizeof(double)*NUM_CUADRADOS);
    memset(y,0,sizeof(double)*NUM_CUADRADOS);

    // Calculos MIL centros de gravedad
    //
    MIL_ID MilBlobResult,                  // Blob result buffer identifier.     
    MilBlobFeatureList;             // Feature list identifier.           
    // Allocate a binary image buffer for fast processing.
    MIL_ID MilBinImage;
    MbufAlloc2d(M_DEFAULT_HOST,
        MbufInquire(M_banda[nBanda], M_SIZE_X, M_NULL),
        MbufInquire(M_banda[nBanda], M_SIZE_Y, M_NULL), 1+M_UNSIGNED, M_IMAGE+M_PROC, &MilBinImage);
//    MimBinarize(M_banda[nBanda], MilBinImage, M_LESS_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son negro y el fondo blanco
    MimBinarize(M_banda[nBanda], MilBinImage, M_GREATER_OR_EQUAL,M_DEFAULT,M_NULL); //para  cuando los cuadrados son blanco y el fondo gris o negro
//    MimBinarize(M_banda[nBanda], MilBinImage, M_IN_RANGE,70,255); //para  cuando los cuadrados son blanco y el fondo gris o negro
//MbufExport("D:\\binarizado.tif",M_TIFF,MilBinImage);
    // Allocate a feature list. 
    MblobAllocFeatureList(M_DEFAULT_HOST, &MilBlobFeatureList); 
    // Enable the Area and Center Of Gravity feature calculation. 
    MblobSelectFeature(MilBlobFeatureList, M_CENTER_OF_GRAVITY); 
    MblobSelectFeature(MilBlobFeatureList, M_AREA + M_SORT1_DOWN ); // ORDENADO POR AREA
    // Allocate a blob result buffer. 
    MblobAllocResult(M_DEFAULT_HOST, &MilBlobResult); 
    MblobControl(MilBlobResult, M_IDENTIFIER_TYPE, M_BINARY ); //Faster
    MblobControl(MilBlobResult, M_SAVE_RUNS, M_DISABLE  ); //Faster -- cannot use MblobFill(), MblobGetLabel(), MblobGetRuns(),or MblobLabel()
    // Calculate selected features for each blob.
    MblobCalculate(MilBinImage, M_NULL, MilBlobFeatureList, MilBlobResult); 
    MblobFree(MilBlobFeatureList);
    // Get the total number of selected blobs. 
    long TotalBlobs;
    MblobGetNumber(MilBlobResult, &TotalBlobs); 
    double *CogX, *CogY; // variables auxiliares necesarias en caso de que que no coincidan el numero de blobs presentes con los esperados
    // Read and print the blob's center of gravity. 
    if ((CogX = (double *)malloc(TotalBlobs*sizeof(double))) && 
        (CogY = (double *)malloc(TotalBlobs*sizeof(double)))
        )
    { 
        // Get the results. 
        memset(CogX,0,TotalBlobs*sizeof(double));
        memset(CogY,0,TotalBlobs*sizeof(double));
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_X, CogX); 
        MblobGetResult(MilBlobResult, M_CENTER_OF_GRAVITY_Y, CogY); 

        if (TotalBlobs < NUM_CUADRADOS)
        {
            CString mensaje;
            mensaje.Format("Se han encontrado solo %d objetos, cuando se esperaba %d",TotalBlobs,NUM_CUADRADOS);
            error_leve("ProcesarPolka",(LPCSTR)mensaje);
            return false;
        }

        // Cojer los primeros NUM_CUADRADOS (para tolerar blobs pequeños).
        dCentroX = 0;
        dCentroY = 0;
        for(int n=0; n < NUM_CUADRADOS; n++)
        {
            dCentroX += CogX[n];
            dCentroY += CogY[n];
        }
        // Ordenar en orden ascendente
        //
        //  -------->
        // |  1   3
        // |  2   4
        // \/
        dCentroX = dCentroX / NUM_CUADRADOS;
        dCentroY = dCentroY / NUM_CUADRADOS;
        for(n=0; n < NUM_CUADRADOS; n++)
        {
            if ((CogX[n] <= dCentroX) && (CogY[n] <= dCentroY))
            {
                ASSERT(x[0] == 0);
                ASSERT(y[0] == 0);
                x[0] = CogX[n];
                y[0] = CogY[n];
            }
            else if ((CogX[n] <= dCentroX) && (CogY[n] > dCentroY))
            {
                ASSERT(x[1] == 0);
                ASSERT(y[1] == 0);
                x[1] = CogX[n];
                y[1] = CogY[n];
            }
            else if ((CogX[n] > dCentroX) && (CogY[n] <= dCentroY))
            {
                ASSERT(x[2] == 0);
                ASSERT(y[2] == 0);
                x[2] = CogX[n];
                y[2] = CogY[n];
            }
            else if ((CogX[n] > dCentroX) && (CogY[n] > dCentroY))
            {
                ASSERT(x[3] == 0);
                ASSERT(y[3] == 0);
                x[3] = CogX[n];
                y[3] = CogY[n];
            }
            else
            {
                // Imposible
                ASSERT(FALSE);
                return false;
            }
        }
       
        free(CogX);
        free(CogY);
    }
    else
    {
        error_leve("ProcesarPolka","No hay memoria suficiente");
        return false;
    }

    MblobFree(MilBlobResult);
    MbufFree(MilBinImage);

    return true;
}
*/

// Guarda los parametros de la correccion geometrica calculada en fichero
// INTERFAZ
bool CCorreccionGeometrica::Guardar(parametros& paramIni)
{
	char nom_fich[512];
	FILE  *archivo;
    int i;

	sprintf(nom_fich, "%s%s%s", paramIni.raiz_patrones, FICH_PARAM_CORRECCION_GEOMETRICA, ".ini");

	if ((archivo = fopen(nom_fich, "wt")) == NULL)
		return false;

	fprintf(archivo, "# PARÁMETROS DE CORRECCION GEOMETRICA OBTENIDOS MEDIANTE POLKA-DOT\n\n");

	fprintf(archivo, "# Centros de gravedad para cada filtro\n");
	fprintf(archivo, "posicion_x = ");
	for (i=0; i < paramIni.nBandas; i++)
		fprintf(archivo, "%.5f ", m_arrPosicionX[i]);
	fprintf(archivo, "\n");
	fprintf(archivo, "posicion_y = ");
	for (i=0; i < paramIni.nBandas; i++)
		fprintf(archivo, "%.5f ", m_arrPosicionY[i]);
	fprintf(archivo, "\n\n");
	fprintf(archivo, "# Escalas para cada filtro\n");
	fprintf(archivo, "escala = ");
	for (i=0; i < paramIni.nBandas; i++)
		fprintf(archivo, "%.5f ", m_arrEscala[i]);
	fprintf(archivo, "\n\n");
	fprintf(archivo, "# Angulo de rotacion (en grados)\n");
	fprintf(archivo, "rotacion = %.5f #Grados. Negativo significa que la camara esta rotada en el sentido del reloj\n\n",m_dRotacion);

	fprintf(archivo, "# Escala efectiva del objetivo \n");
	fprintf(archivo, "escala_objetivo = %.5f\n",m_dEscalaObjetivo);

    fclose(archivo);
    return true;
}


// Carga los parametros de la correccion geometrica calculada desde fichero
bool CCorreccionGeometrica::Leer(parametros& paramIni)
{
	char  nom_fich[512];
	char	*fn = "CCorreccionGeometrica::Leer";

	//	Carga del fichero
	sprintf(nom_fich, "%s%s%s", paramIni.raiz_patrones, FICH_PARAM_CORRECCION_GEOMETRICA, ".ini");
	if ( LoadVars(nom_fich) == NULL) {
		error_fatal(nom_fich, "El fichero indicado no se encuentra", 0);
		return false;
	}

    if (
        ( m_arrPosicionX = (double *)malloc(paramIni.nBandas*sizeof(double)) ) && 
        ( m_arrPosicionY = (double *)malloc(paramIni.nBandas*sizeof(double)) ) &&
        ( m_arrEscala = (double *)malloc(paramIni.nBandas*sizeof(double)) )
       )
    { 
        double* posicion_x = NULL;
	    posicion_x = m_arrPosicionX;
	    LOADVDBL(posicion_x, paramIni.nBandas);

	    double* posicion_y = NULL;
	    posicion_y = m_arrPosicionY;
	    LOADVDBL(posicion_y, paramIni.nBandas);

	    double* escala = NULL;
	    escala = m_arrEscala;
	    LOADVDBL(escala, paramIni.nBandas);

	    double rotacion;
	    LOADDBL(rotacion);
        m_dRotacion = rotacion;

	    double escala_objetivo;
	    LOADDBL(escala_objetivo);
        if (escala_objetivo > 0)
            m_dEscalaObjetivo = escala_objetivo;

        return true;
    }
    else
    {
        error_leve("CCorreccionGeometrica::Leer","No hay memoria suficiente");
        return false;
    }
}


// correcciones (posicion, escala, rotacion)
// M_arrImagenes - array de imagenes a corregir
// bCorregirRotacion - [opcional, por defecto true] - true si se quiere corregir rotacion ademas la posicion y escala
bool CCorreccionGeometrica::CorregirImagenes(MIL_ID* M_arrImagenes, parametros &param, bool bCorregirRotacion)
{
    MIL_ID M_aux;                       // Buffer auxiliar para realizar la corrección geométrica
    MIL_ID M_WarpParam1, M_WarpParam2;  // Arrays de coeficientes de warping [3x3]
    
    if (m_arrPosicionX == NULL || m_arrPosicionY == NULL || m_arrEscala == NULL)
        return false;

    // Buffer auxiliar de las mismas dimensiones que el buffer origen. Más adelante se recortará a las dimensiones de destino
	MbufAlloc2d(M_sistema, param.Cam.anchoImagen, param.Cam.altoImagen, param.Cam.profundidad+M_UNSIGNED,
		M_IMAGE+M_PROC, &M_aux); 

    // Arrays auxiliares para calcular los coeficientes de warping polinómico
    MbufAlloc2d(M_sistema, 3, 3, 32+M_FLOAT, M_ARRAY, &M_WarpParam1);
	MbufAlloc2d(M_sistema, 3, 3, 32+M_FLOAT, M_ARRAY, &M_WarpParam2);

	for (int f=0; f < param.nBandas; f++)  
    {
        // WARP 1: Desplazamos la imagen de partida para llevar el centro de la imagen al punto (0,0) 
        // y al mismo tiempo corregir el error de centrado entre la banda actual y la de referencia
        MgenWarpParameter(M_NULL, M_WarpParam2, M_NULL, M_WARP_POLYNOMIAL, M_TRANSLATE, 
            - (param.Cam.anchoImagen/2 + m_arrPosicionX[f] - m_arrPosicionX[param.filtro[param.Rueda.posEnfoque]]), 
            - (param.Cam.altoImagen/2 + m_arrPosicionY[f] - m_arrPosicionY[param.filtro[param.Rueda.posEnfoque]]));

        // WARP 2: Corregimos la escala de la imagen respecto a su centro
        MgenWarpParameter(M_WarpParam2, M_WarpParam1, M_NULL, M_WARP_POLYNOMIAL, M_SCALE, 
            m_arrEscala[f] / m_dEscalaObjetivo, m_arrEscala[f] / m_dEscalaObjetivo);

        if (bCorregirRotacion)
        {
            // WARP 3: Rotamos la imagen respecto a su centro
            MgenWarpParameter(M_WarpParam1, M_WarpParam2, M_NULL, M_WARP_POLYNOMIAL, M_ROTATE, 
                m_dRotacion, M_NULL);
        }
        else
        {
            MbufCopy(M_WarpParam1, M_WarpParam2);
        }

        // WARP 4: Devolvemos el punto (0,0) al centro de la imagen
        MgenWarpParameter(M_WarpParam2, M_WarpParam1, M_NULL, M_WARP_POLYNOMIAL, M_TRANSLATE, 
            param.Cam.anchoImagen/2, param.Cam.altoImagen/2);

        // Aplicamos todas las transformaciones lineales anteriores combinadas
        MimWarp(M_arrImagenes[f], M_aux, M_WarpParam1, M_NULL, M_WARP_POLYNOMIAL, M_BILINEAR + M_OVERSCAN_CLEAR);
        MbufCopy(M_aux, M_arrImagenes[f]);
    }

	MbufFree(M_WarpParam2);
	MbufFree(M_WarpParam1);
	MbufFree(M_aux);

    return true;
}

// Calcula y devuelve el ancho maximo que pueden tener las imagenes de destino teniendo en cuenta la escala y los factores de correccion de imagen
// bRotacion - indica si se aplica el angulo de rotacion en el calculo del maximo
int CCorreccionGeometrica::CalcularAnchoMax(int nAnchoImagen, int nAltoImagen, int nBandas, bool bRotacion)
{
    double dDimensionMax;

    if (bRotacion)
        // Primero restamos el error por rotacion de la camara (importante que sea lo primero porque se usa la altura tambien)
        dDimensionMax = (double)nAnchoImagen/(cos(m_dRotacion*PI / 180) + fabs(sin(m_dRotacion*PI / 180))*(double)nAnchoImagen/(double)nAltoImagen);
    else
        dDimensionMax = nAnchoImagen;

    // Luego restamos el error por desplazamiento
    double max_desplazamiento   = 0;
    double min_desplazamiento   = INT_MAX;
    double min_escala           = INT_MAX;
    
	for (int f=0; f < nBandas; f++)  
    {
        min_desplazamiento  = min(min_desplazamiento,m_arrPosicionX[f]);
        max_desplazamiento  = max(max_desplazamiento,m_arrPosicionX[f]);
        min_escala          = min(min_escala,m_arrEscala[f]);
    }

    dDimensionMax = dDimensionMax - 2*(max_desplazamiento - min_desplazamiento);

    // Luego restamos el error por escala
    dDimensionMax = dDimensionMax* min_escala / m_dEscalaObjetivo;

    if (dDimensionMax > nAnchoImagen)
        return nAnchoImagen;

    return (int)floor(dDimensionMax);
}

// Calcula y devuelve el alto maximo que pueden tener las imagenes de destino teniendo en cuenta la escala y los factores de correccion de imagen
// bRotacion - indica si se aplica el angulo de rotacion en el calculo del maximo
int CCorreccionGeometrica::CalcularAltoMax(int nAnchoImagen, int nAltoImagen, int nBandas, bool bRotacion)
{
    double dDimensionMax;

    if (bRotacion)
        // Primero restamos el error por rotacion de la camara (importante que sea lo primero porque se usa la altura tambien)
        dDimensionMax = (double)nAltoImagen/(cos(m_dRotacion*PI / 180) + fabs(sin(m_dRotacion*PI / 180))*(double)nAnchoImagen/(double)nAltoImagen);
    else
        dDimensionMax = nAltoImagen;
   
    // Luego restamos el error por desplazamiento
    double max_desplazamiento   = 0;
    double min_desplazamiento   = INT_MAX;
    double min_escala           = INT_MAX;
    
	for (int f=0; f < nBandas; f++)  
    {
        min_desplazamiento  = min(min_desplazamiento,m_arrPosicionY[f]);
        max_desplazamiento  = max(max_desplazamiento,m_arrPosicionY[f]);
        min_escala          = min(min_escala,m_arrEscala[f]);
    }

    dDimensionMax = dDimensionMax - 2*(max_desplazamiento - min_desplazamiento);

    // Luego restamos el error por escala
    dDimensionMax = dDimensionMax* min_escala / m_dEscalaObjetivo;

    if (dDimensionMax > nAltoImagen)
        return nAltoImagen;

    return (int)floor(dDimensionMax);
}
