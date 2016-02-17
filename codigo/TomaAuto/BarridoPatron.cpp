/*
	adquisicion_imagenPat.cpp
*/

#include "stdafx.h"


#include <math.h>
#include <conio.h>
#include <stdio.h>

#include "BarridoPatron.h"
#include "FormPatron.h"
#include "TomaAuto.h"
#include "Progreso.h"

#include "..\Librerias\ProcesoAux\control_proceso_imagenes.h"
#include "..\Librerias\ControlDisp\control_digitalizador.h" //PROFUNDIDAD_ACUMULACION
#include "..\Librerias\ControlDisp\control_camara.h"
#include "..\Librerias\ControlMicroscopio\control_microscopio.h"

#include "..\Librerias\LibBarrido\calibracion.h"
#include "..\Librerias\LibBarrido\control_barrido.h"
#include "..\librerias\ControlMicroscopio\control_objetivos.h"
#include <direct.h> //mkdir
#include <errno.h> //mkdir
    

#define CAMPOS_PATRONES		 (CAMPOS_EJE_X*CAMPOS_EJE_Y)

#define SALTO_DIBUJO		20
#define SEMILADO_RECT		 7

#define PROFUNDIDAD_CONTADOR    16

extern CTomaAutoApp theApp;
extern limitesCam	limCam; // declarado en la aplicacion llamante y usado aqui

// Estructura para pasar al thread de barrido
typedef struct PARAM_THREAD
{
    CBarridoPatron*       pThis;
    CFormPatron*   pDialogo;
} PARAM_THREAD;



CBarridoPatron::CBarridoPatron()
{
    m_bCancelarBarrido = false;    // su true, se cancela el thread de barrido

    //inicializamos puntos de enfoque
    InitPuntos();

    // Reservamos histrogramas auxiliares
	MimAllocResult(M_sistema, NUMVAL, M_HIST_LIST, &M_histo12);
	MimAllocResult(M_sistema, 65536, M_HIST_LIST, &M_histo16);
}

CBarridoPatron::~CBarridoPatron()
{
	MimFree(M_histo12);
	MimFree(M_histo16);
}

//inicializamos puntos de enfoque
void CBarridoPatron::InitPuntos()
{
    for (int j=0;j<NUM_PUNTOS_PLANO;j++)
    {
        for (int i=X_;i<=Z_;i++)
            m_arrPuntosControl[j].coord[i] = -1;
        m_arrPuntosControl[j].peso = 1;
    }
}


/**************************  ini_TomaPatrones  ******************************
	Reserva de memoria para tomar una serie de imagenes promediadas.
*****************************************************************************/
int CBarridoPatron::ini_TomaPatrones(PARAM_THREAD* paramThread)
{
	int i;

    //Acumuladores
	for (i=0; i < theApp.m_ParamIni.nBandas; i++)  
    {
	    MbufAlloc2d(M_sistema, theApp.m_ParamIni.Cam.anchoImagen, theApp.m_ParamIni.Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &paramThread->pThis->M_banda_acum[i]);
	    MbufAlloc2d(M_sistema, theApp.m_ParamIni.Cam.anchoImagen, theApp.m_ParamIni.Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &paramThread->pThis->M_banda_enmascarada_acum[i]);
	    MbufClear(paramThread->pThis->M_banda_acum[i], 0);
	    MbufClear(paramThread->pThis->M_banda_enmascarada_acum[i], 0);

        //Contador
	    MbufAlloc2d(M_sistema, theApp.m_ParamIni.Cam.anchoImagen, theApp.m_ParamIni.Cam.altoImagen, 8+M_UNSIGNED, //necesario 32+M_FLOAT para realizar operaciones en coma flotante
		    M_IMAGE+M_PROC, &paramThread->pThis->M_cont_campos[i]);
	    MbufClear(paramThread->pThis->M_cont_campos[i], 0);

	}

    //Buffers auxiliares
	MbufAlloc2d(M_sistema, theApp.m_ParamIni.Cam.anchoImagen, theApp.m_ParamIni.Cam.altoImagen, 8+M_UNSIGNED,
		M_IMAGE+M_PROC, &paramThread->pThis->M_mascara_aux);
	MbufClear(paramThread->pThis->M_mascara_aux, 0);

	MbufAlloc2d(M_sistema, theApp.m_ParamIni.Cam.anchoImagen, theApp.m_ParamIni.Cam.altoImagen, PROFUNDIDAD_ACUMULACION+M_UNSIGNED,
		M_IMAGE+M_PROC+M_GRAB, &paramThread->pThis->M_acum_aux);
	MbufClear(paramThread->pThis->M_acum_aux, 0);

	return 0;
}


/**************************  fin_TomaPatrones  ******************************
*****************************************************************************/
int CBarridoPatron::fin_TomaPatrones(PARAM_THREAD* paramThread)
{
	int i;

	for (i=0; i < theApp.m_ParamIni.nBandas; i++)
    {
		MbufFree(paramThread->pThis->M_banda_enmascarada_acum[i]);
		MbufFree(paramThread->pThis->M_banda_acum[i]);
        MbufFree(paramThread->pThis->M_cont_campos[i]);
    }
    MbufFree(paramThread->pThis->M_mascara_aux);
    MbufFree(paramThread->pThis->M_acum_aux);

    return 0;
}

//Establece una esquina de barrido. Se guarda temporalmente en inicio y fin
//Devuelve true si la Z del punto establecido es identica a la establecida para el punto final (para saltarse el plano de regresion)
bool CBarridoPatron::SetInicio(point& pActual)
{
    pActual = mspWhereP();
    for (int i=X_;i<=Z_;i++)
    {
        m_arrPuntosControl[0].coord[i] = pActual.coord[i];
    }

//    pActual = m_pActual;

    return (m_arrPuntosControl[0].coord[Z_] == m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_]);
}

//Establece una esquina de barrido. Se guarda temporalmente en inicio y fin
//Devuelve true si la Z del punto establecido es identica a la establecida para el punto inicial (para saltarse el plano de regresion)
bool CBarridoPatron::SetFin(point& pActual)
{
    pActual = mspWhereP();
    for (int i=X_;i<=Z_;i++)
    {
        m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[i] = pActual.coord[i];
    }

//    pActual = m_pActual;

    return (m_arrPuntosControl[0].coord[Z_] == m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_]);
}

// Recalcula las 4 esquinas (puntos principales de enfoque) 
// a partir de los puntos definidos por el usuario
void CBarridoPatron::Intermedios()
{
    // guardamos temporalmente las presuntas MinMin y MaxMax
    point temp_inicio;
    point temp_fin;
    for (int i=X_;i<=Z_;i++)
    {
        temp_inicio.coord[i] = m_arrPuntosControl[0].coord[i];
        temp_fin.coord[i] = m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[i];
    }

    // Reordenamos para que las esquinas esten en orden
    m_arrPuntosControl[0].coord[X_] = min(temp_inicio.coord[X_],temp_fin.coord[X_]);
    m_arrPuntosControl[0].coord[Y_] = min(temp_inicio.coord[Y_],temp_fin.coord[Y_]);

    m_arrPuntosControl[2].coord[X_] = max(temp_inicio.coord[X_],temp_fin.coord[X_]);
    m_arrPuntosControl[2].coord[Y_] = min(temp_inicio.coord[Y_],temp_fin.coord[Y_]);

    m_arrPuntosControl[6].coord[X_] = min(temp_inicio.coord[X_],temp_fin.coord[X_]);
    m_arrPuntosControl[6].coord[Y_] = max(temp_inicio.coord[Y_],temp_fin.coord[Y_]);

    m_arrPuntosControl[8].coord[X_] = max(temp_inicio.coord[X_],temp_fin.coord[X_]);
    m_arrPuntosControl[8].coord[Y_] = max(temp_inicio.coord[Y_],temp_fin.coord[Y_]);

    // Asignamos coordenadas de los puntos intermedios
    //  -------> (x)
    // | 0 1 2
    // | 3 4 5
    // | 6 7 8
    // \/
    // (y)

    m_arrPuntosControl[1].coord[X_] = (m_arrPuntosControl[0].coord[X_] + m_arrPuntosControl[2].coord[X_]) / 2;
    m_arrPuntosControl[1].coord[Y_] = m_arrPuntosControl[0].coord[Y_];

    m_arrPuntosControl[3].coord[X_] = m_arrPuntosControl[0].coord[X_];
    m_arrPuntosControl[3].coord[Y_] = (m_arrPuntosControl[0].coord[Y_] + m_arrPuntosControl[6].coord[Y_]) / 2;

    m_arrPuntosControl[4].coord[X_] = m_arrPuntosControl[1].coord[X_];
    m_arrPuntosControl[4].coord[Y_] = m_arrPuntosControl[3].coord[Y_];

    m_arrPuntosControl[5].coord[X_] = m_arrPuntosControl[2].coord[X_];
    m_arrPuntosControl[5].coord[Y_] = m_arrPuntosControl[3].coord[Y_];

    m_arrPuntosControl[7].coord[X_] = m_arrPuntosControl[1].coord[X_];
    m_arrPuntosControl[7].coord[Y_] = m_arrPuntosControl[6].coord[Y_];


    //reasignamos la Z de las dos esquinas ya enfocadas por el usuario
    if (temp_inicio.coord[Z_] == temp_fin.coord[Z_])
    {
        // Sin plano de regresion, todas las Z iguales 
        m_arrPuntosControl[0].coord[Z_] = temp_inicio.coord[Z_];
        m_arrPuntosControl[2].coord[Z_] = temp_inicio.coord[Z_];
        m_arrPuntosControl[6].coord[Z_] = temp_inicio.coord[Z_];
        m_arrPuntosControl[8].coord[Z_] = temp_inicio.coord[Z_];
    }
    else
    {
        if ((temp_inicio.coord[X_] < temp_fin.coord[X_]) && (temp_inicio.coord[Y_] < temp_fin.coord[Y_]))
        {
            m_arrPuntosControl[0].coord[Z_] = temp_inicio.coord[Z_];
            m_arrPuntosControl[2].coord[Z_] = -1;
            m_arrPuntosControl[6].coord[Z_] = -1;
            m_arrPuntosControl[8].coord[Z_] = temp_fin.coord[Z_];
        }
        else if ((temp_inicio.coord[X_] < temp_fin.coord[X_]) && (temp_inicio.coord[Y_] > temp_fin.coord[Y_]))
        {
            m_arrPuntosControl[0].coord[Z_] = -1;
            m_arrPuntosControl[2].coord[Z_] = temp_inicio.coord[Z_];
            m_arrPuntosControl[6].coord[Z_] = temp_fin.coord[Z_];
            m_arrPuntosControl[8].coord[Z_] = -1;
        }
        else if ((temp_inicio.coord[X_] > temp_fin.coord[X_]) && (temp_inicio.coord[Y_] < temp_fin.coord[Y_]))
        {
            m_arrPuntosControl[0].coord[Z_] = -1;
            m_arrPuntosControl[2].coord[Z_] = temp_fin.coord[Z_];
            m_arrPuntosControl[6].coord[Z_] = temp_inicio.coord[Z_];
            m_arrPuntosControl[8].coord[Z_] = -1;
        }    
        else if ((temp_inicio.coord[X_] > temp_fin.coord[X_]) && (temp_inicio.coord[Y_] > temp_fin.coord[Y_]))
        {
            m_arrPuntosControl[0].coord[Z_] = temp_fin.coord[Z_];
            m_arrPuntosControl[2].coord[Z_] = -1;
            m_arrPuntosControl[6].coord[Z_] = -1;
            m_arrPuntosControl[8].coord[Z_] = temp_inicio.coord[Z_];
        }
        else
        {
            //ERROR: los puntos no forman un rectangulo, sino una recta!!
        }
    }
}

// El usuario ha enfocado un punto intermedio 
// Devuelve el indice del punto enfocado para que se actualice el valor de z en el dialogo como parametro
// Devuelve el indice del siguiente punto a enfocar o -1 si no hay mas puntos a enfocar
int CBarridoPatron::EnfoquePuntosIntermedios(int& nIndicePuntoEnfocado)
{
    static int nIndicePuntoActual = -1;
    nIndicePuntoEnfocado = nIndicePuntoActual; //devolvemos el punto enfocado
    bool bPuntoEncontrado = false;

    if (nIndicePuntoActual != -1)
    {
        // Guardar punto (no solo Z porque el usuario ha podido moverse un poco)
        m_arrPuntosControl[nIndicePuntoActual].coord[X_]= mspWhere(X_);
        m_arrPuntosControl[nIndicePuntoActual].coord[Y_]= mspWhere(Y_);
        m_arrPuntosControl[nIndicePuntoActual].coord[Z_]= mspWhere(Z_);
//        m_arrPuntosControl[nIndicePuntoActual].coord[X_]= m_pActual.coord[X_];
//        m_arrPuntosControl[nIndicePuntoActual].coord[Y_]= m_pActual.coord[Y_];
//        m_arrPuntosControl[nIndicePuntoActual].coord[Z_]= m_pActual.coord[Z_];
    }

    // Ir al siguiente punto a enfocar (si hay)
    for (int i=nIndicePuntoActual+1;i<NUM_PUNTOS_PLANO;i++)
    {
        if (m_arrPuntosControl[i].coord[Z_] == -1)
        {
            // Punto no enfocado encontrado

            // Ir al punto
            mspGoCheck(X_,m_arrPuntosControl[i].coord[X_]);
            mspGoCheck(Y_,m_arrPuntosControl[i].coord[Y_]);

            //Guardar punto a enfocar
            nIndicePuntoActual = i;

            //hay mas puntos a enfocar
            return nIndicePuntoActual;
        }
    }

    //No hay mas puntos a enfocar
    nIndicePuntoActual = -1; //preparado para un proximo barrido
    return -1;
}

// pDialogo: puntero al dialogo necesario para que el thread indique su final
bool CBarridoPatron::Barrer(CFormPatron* pDialogo)
{
    if (m_arrPuntosControl[0].coord[Z_] != m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_])
    {
        //Calculamos el plano de regresion
        CString ErrorMessage;
	    pointlist plist;
	    dim_pointlist(NUM_PUNTOS_PLANO,plist);		// Dimensionamos la lista de puntos.

        for (int i=0;i<NUM_PUNTOS_PLANO;i++)
            put_pointlist(plist,i,m_arrPuntosControl[i]);
	    
	    if (get_plane_rg(plist, m_plano) == false)		// Calculamos el plano de regresión
        {
            ErrorMessage.Format("Error en el calculo del plano de regresion");
		    AfxMessageBox(ErrorMessage);
            return false;
        }

        // verificamos el error del plano
        double Zplano, errorZ;
        double dMaxDistPlano = theApp.m_ParamIni.objetivos[mspGetObjetivo()-1].dMaxDistPlano;
        for (i=0; i<NUM_PUNTOS_PLANO; i++)
	    {
            Zplano = m_plano.A * plist.pointn[i].coord[X_] + m_plano.B * plist.pointn[i].coord[Y_] + m_plano.C;
            errorZ = fabs(plist.pointn[i].coord[Z_] - Zplano);
            if (errorZ > dMaxDistPlano || errorZ < 0)
            {
                ErrorMessage.Format("Error en el calculo del plano de regresion, error: %f", errorZ);
		        AfxMessageBox(ErrorMessage);
                return false;
            }

            #if _DEBUG
	            char mensaje[LONGITUD_TEXTO];
                sprintf(mensaje,"punto %d: [%.2lf,%.2lf,%.2lf]  Zplano:%.2lf  errorZ: %.2lf", i+1, plist.pointn[i].coord[X_],
                    plist.pointn[i].coord[Y_], plist.pointn[i].coord[Z_], Zplano, errorZ);
                sendlog("Barrer", mensaje);
            #endif
        }

        free_pointlist(plist); // Liberamos la memoria empleada
    }
    else
    {
        m_plano.A = -1;
        m_plano.B = -1;
        m_plano.C = -1;
    }

    // Comprobar si el usuario ha cambiado de OBJETIVO externamente o mediante la aplicacion
    bool bHayCambioManual; //si se ha cambiado de objetivo externamente, la configuracion estará obsoleta
    if (!actualizar_configuracion_objetivo(&theApp.m_ParamIni, bHayCambioManual))
        return false;
    // Si se ha cambiado de objetivo con la aplicacion, la configuracion estara actualizada (incluyendo la geometrica) pero no la correccion
    if (bHayCambioManual || get_objetivo_corregido() != mspGetObjetivo())
    {
        if (bHayCambioManual)
        {
            //Actualizamos exposicion y base de exposicion
            int nFiltroActual = theApp.Rueda.GetFilter();
	        ModificaExposicionSleep(theApp.m_ParamIni.Rueda.exposicion[nFiltroActual]);//modificar la exposicion correspondiente al filtro seleccionado
            ModificaBaseExposicion(theApp.m_ParamIni.Rueda.base_exp[nFiltroActual]);// Modificar la base de exposicion para este nuevo filtro
        }
    }

    //Crear directorio si no esta ya creado
    if (mkdir(theApp.m_ParamIni.raiz_patrones) != 0)
	{
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
		    AfxMessageBox("No se puede crear el directorio de patrones");
            return false;
        }
	}

    PARAM_THREAD* paramThread = new PARAM_THREAD; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pThis = this;
    paramThread->pDialogo = pDialogo;

    pDialogo->InicializaProgresion(theApp.m_ParamIni.BarridoPatrones.camposEjeX*theApp.m_ParamIni.BarridoPatrones.camposEjeY);
    m_bCancelarBarrido = false;
    AfxBeginThread(BarrerThread, (void *)paramThread);

    return true;
}

// avisa al thread para que termine
void CBarridoPatron::Cancelar()
{
    m_bCancelarBarrido = true;
    //inicializamos puntos de enfoque
    InitPuntos();
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// THREAD BARRIDO para que el dialogo este vivo
// Devuelve 0 si todo ha ido bien, 1 en otro caso
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
UINT CBarridoPatron::BarrerThread( LPVOID Param )
{
#if _DEBUG
	static char mensaje[1024];
#endif
    PARAM_THREAD* paramThread = (PARAM_THREAD*)Param;

    int i,j;
    char nombre_fich[512];
    double x,y,z = -1;
    z = paramThread->pThis->m_arrPuntosControl[0].coord[Z_]; // Z por defecto para el caso de que no se use plano de regresion
    theApp.Rueda.zObservacion = z;                           // y actualizar la z del filtro predefinido 

	//	Resevo memoria para los buffers que se utilizarán en el proceso de acumulación
	//	de imágenes de las diferentes bandas. 
	paramThread->pThis->ini_TomaPatrones(paramThread);

	// Determino el salto del movimiento
	double saltoX = 0, saltoY = 0;
    if (theApp.m_ParamIni.BarridoPatrones.camposEjeX == 1)
        saltoX = 0;
    else
	    saltoX = fabs(paramThread->pThis->m_arrPuntosControl[0].coord[X_] - paramThread->pThis->m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[X_]) 
                / (theApp.m_ParamIni.BarridoPatrones.camposEjeX-1);
    if (theApp.m_ParamIni.BarridoPatrones.camposEjeY == 1)
        saltoY = 0;
    else
	    saltoY = fabs(paramThread->pThis->m_arrPuntosControl[0].coord[Y_] - paramThread->pThis->m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Y_]) 
                / (theApp.m_ParamIni.BarridoPatrones.camposEjeY-1);

	// Recorrido de la región, tomando imágenes de cada banda en "ZIG-ZAG"
	for(j =0; j < theApp.m_ParamIni.BarridoPatrones.camposEjeY; j++)  
    {
        // Ir al punto
        y = paramThread->pThis->m_arrPuntosControl[0].coord[Y_] + j*saltoY;
		mspGoCheck(Y_, y);
		for(i=0; i < theApp.m_ParamIni.BarridoPatrones.camposEjeX; i++)  
        {
            if (paramThread->pThis->avanza_y_toma_campo(paramThread,
                                    x, y, z, 
                                    j, i, saltoX) == -1)
            {
				paramThread->pThis->fin_TomaPatrones(paramThread);
				return -1;
            }
            if (paramThread->pThis->m_bCancelarBarrido == true)
            {
	            //	Libero la memoria MIL reservada en esta función
	            paramThread->pThis->fin_TomaPatrones(paramThread);

                paramThread->pThis->InitPuntos(); //reseteamos para permitir un nuevo barrido
                paramThread->pDialogo->PermitirNuevoBarrido(true);
                delete paramThread;
                return 1;
            }
		}
		j++;
        if (j>=theApp.m_ParamIni.BarridoPatrones.camposEjeY) //No hay que "volver"
            break;
        y = paramThread->pThis->m_arrPuntosControl[0].coord[Y_] + j*saltoY;
		mspGoCheck(Y_, y);
		for(i=theApp.m_ParamIni.BarridoPatrones.camposEjeX-1; i >= 0; i--)  
        {
            if (paramThread->pThis->avanza_y_toma_campo(paramThread,
                                    x, y, z, 
                                    j, i, saltoX) == -1)
            {
				paramThread->pThis->fin_TomaPatrones(paramThread);
				return -1;
            }
            if (paramThread->pThis->m_bCancelarBarrido == true)
            {
	            //	Libero la memoria MIL reservada en esta función
	            paramThread->pThis->fin_TomaPatrones(paramThread);

                paramThread->pThis->InitPuntos(); //reseteamos para permitir un nuevo barrido
                paramThread->pDialogo->PermitirNuevoBarrido(true);
                delete paramThread;
                return 1;
            }
		}
	}

    //Factor por el que hay que multiplicar una unica toma (sin acumular) para alcanzar
    //la profundidad deseada. Si 8bit -> 1. Si 12bit-> 16
    ASSERT(theApp.m_ParamIni.Cam.profundidad >= BITS_CAMARA);
    int factorCamara = 1 << (theApp.m_ParamIni.Cam.profundidad - BITS_CAMARA); //Si 8bit -> 1. Si 12bit-> 16
    // Factor por el que hay que multiplicar  la imagen acumulada para alcanzar la profundidad deseada teniendo en cuenta la acumulacion pero no el promediado de campos
    double factorMult = (double)factorCamara / (double)theApp.m_ParamIni.BarridoPatrones.nImagenAcum ;
  
    long histo_temp[NUMVAL];
        // M_banda_acum[i] -> promediado -> M_banda[i]
	for (i=0; i < theApp.m_ParamIni.nBandas; i++)  
    {
        if (paramThread->pDialogo->GetTipoPatron()) // PATRON REFLECTANCIA ALTA
        {
	        MimHistogram(paramThread->pThis->M_cont_campos[i], paramThread->pThis->M_histo12);
	        MimGetResult(paramThread->pThis->M_histo12, M_VALUE, histo_temp);
            if (histo_temp[0]!=0) // Existen píxeles que no tienen valor acumulado
            {
                // Ponemos el valor acumulado sin enmascarar en los píxeles que no han acumulado ningún valor
                MimBinarize(paramThread->pThis->M_cont_campos[i], paramThread->pThis->M_mascara_aux, M_EQUAL, 0, M_NULL);
                MimShift(paramThread->pThis->M_mascara_aux, paramThread->pThis->M_mascara_aux, -7);
    		    MimArith(paramThread->pThis->M_banda_acum[i], paramThread->pThis->M_mascara_aux, paramThread->pThis->M_banda_acum[i], M_MULT);
       		    MimArith(paramThread->pThis->M_banda_acum[i], paramThread->pThis->M_banda_enmascarada_acum[i], paramThread->pThis->M_banda_enmascarada_acum[i], M_ADD);
                // Corregimos el contador en los píxeles que no han acumulado ningún valor
                MimArith(paramThread->pThis->M_mascara_aux, theApp.m_ParamIni.BarridoPatrones.camposEjeX * theApp.m_ParamIni.BarridoPatrones.camposEjeY, paramThread->pThis->M_mascara_aux, M_MULT_CONST);
       		    MimArith(paramThread->pThis->M_mascara_aux, paramThread->pThis->M_cont_campos[i], paramThread->pThis->M_cont_campos[i], M_ADD);
            }
            // Calculamos el valor promedio de los valores válidos para cada píxel
		    MimArith(paramThread->pThis->M_banda_enmascarada_acum[i], paramThread->pThis->M_cont_campos[i], paramThread->pThis->M_banda_enmascarada_acum[i], M_DIV); //promediado
            //El contador de campos validos (para cada pixel) se convertirá en el factor de division para promediar
            MimArith(paramThread->pThis->M_banda_enmascarada_acum[i], factorMult, paramThread->pThis->M_banda_enmascarada_acum[i], M_MULT_CONST);
        }
        else //PATRON REFLECTANCIA BAJA
        {
		    MimArith(paramThread->pThis->M_banda_enmascarada_acum[i], factorMult / (theApp.m_ParamIni.BarridoPatrones.camposEjeX * theApp.m_ParamIni.BarridoPatrones.camposEjeY), 
                paramThread->pThis->M_banda_enmascarada_acum[i], M_MULT_CONST); //promediado
        }

		MbufCopy(paramThread->pThis->M_banda_enmascarada_acum[i], M_banda[i]); //conversion a 16 bit o 8 bit (segun BITS_CAMARA)
    }

    //	Una vez obtenida la imagen promedio se guarda esta en disco con
	//	la extensión y nombre del patrón seleccionado inicialmente. 
	for (i=0; i < theApp.m_ParamIni.nBandas; i++)  
    {
        if (paramThread->pDialogo->GetTipoPatron())
		    sprintf(nombre_fich, "%s%s_%02d%s", theApp.m_ParamIni.raiz_patrones, NOM_PATRON_CL, theApp.m_ParamIni.filtro[i]+1, EXT_PAT);
        else
		    sprintf(nombre_fich, "%s%s_%02d%s", theApp.m_ParamIni.raiz_patrones, NOM_PATRON_OS, theApp.m_ParamIni.filtro[i]+1, EXT_PAT);
		graba_imagen_campo(nombre_fich, M_banda[i]);
	}

    // preparar correccion con nuevos patrones
	if (prepara_correccion(&theApp.m_ParamIni) != 0) // HACER Thread?
	{
		AfxMessageBox("Fallo en prepara_correccion");
        paramThread->pDialogo->HayCorreccion(false); //no permitir barrido
        return false;
    }
    paramThread->pDialogo->HayCorreccion(true); //permitir barrido

	//	Limpio el buffer de overlay
	borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);

    // Dejo la rueda en el filtro por defecto
    theApp.Rueda.ChangeFilter(theApp.m_ParamIni.Rueda.posFiltro);

    paramThread->pThis->InitPuntos(); //reseteamos para permitir un nuevo barrido
    paramThread->pDialogo->PermitirNuevoBarrido(false);

	//	Libero la memoria MIL reservada en esta función
	paramThread->pThis->fin_TomaPatrones(paramThread);

    delete paramThread;

    return 0;
}

/**************************  avanza_y_toma_campo  *********************************
Funcion creada unicamente por reutilizacion de codigo
AUXILIAR a haz_movimiento
*****************************************************************************/
int CBarridoPatron::avanza_y_toma_campo(PARAM_THREAD* paramThread,
                        double& x, double& y, double& z, 
                        int j, int i, double saltoX)
{
#if _DEBUG
	char mensaje[LONGITUD_TEXTO];
#endif
    
    x = paramThread->pThis->m_arrPuntosControl[0].coord[X_] + i * saltoX;
    mspGoCheck(X_, x);
    if (paramThread->pThis->m_plano.A != -1)
    {
        // z de enfoque (para el filtro de enfoque) segun el plano de regresion
        z = paramThread->pThis->m_plano.A * x + paramThread->pThis->m_plano.B * y + paramThread->pThis->m_plano.C;
        // Hay que actualizar la z del filtro predefinido (a partir de la Z del filtro de enfoque)
// ESTO SOLO SI SE ENFOCA EN VERDE!        theApp.Rueda.zObservacion = z + theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] - 
// ESTO SOLO SI SE ENFOCA EN VERDE!                                theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.filtro[BANDA_REFERENCIA_Z]]; 
        // Hay que actualizar la z del filtro predefinido 
        // actualizo la z absoluta del filtro predefinido 
        theApp.Rueda.zObservacion = z; 
	    mspGoCheck(Z_, z + BACKSLASHZ);
	    mspGoCheck(Z_, z);
    }

    // Actualizar posicion y progreso en dialogo
    paramThread->pDialogo->IncrementaProgresion();
    paramThread->pDialogo->MostrarPosicion(x,y,z);

#if _DEBUG
	sprintf(mensaje, "Barriendo Patron a: %f  %f  %f", x,y,z);
    sendlog("\nBarrerThread",mensaje);
#endif

//    muestra_tomaPat(M_overlay_normal, i, j, false);
    if (toma_campoPat(paramThread)) 
	    return -1;
    
    //Añadimos campo a la imagen de progreso (se usa M_banda[])
    paramThread->pDialogo->m_pProgreso->NuevoCampo(i,j);
        
    return 0;
}

/**************************  CalcularMascara  *********************************
Toma una imagen y calcula los valores de su histograma que son demasiado oscuros
y genera la mascara con ceros y unos correspondiente
AUXILIAR a toma_campoPat
*****************************************************************************/
void CBarridoPatron::CalcularMascara(PARAM_THREAD* paramThread, int nBanda)
{
    // Hallar moda y sigma inferior
	long moda;
    // M_acum_aux es de 32 bit, pero como suponemos que nunca alcanzará mas de 16bit reales
    long histo[65536];//4294967296  pow(2,profundidad)
    memset(histo,0,65536*sizeof(long));
    // No podemos usar evalua_histograma porque no esta preparado para buffers de 16 bit
	MimHistogram(paramThread->pThis->M_acum_aux, M_histo16);
	MimGetResult(M_histo16, M_VALUE, histo);
	moda = moda_histograma(histo, 0, 65536-1);

    // Calcular mascara
    int nUmbral;
    nUmbral =(int)(moda - theApp.m_ParamIni.BarridoPatrones.rango_util[nBanda]*theApp.m_ParamIni.BarridoPatrones.nImagenAcum);
//    MimBinarize(M_acum_aux,M_mascara_aux,M_GREATER_OR_EQUAL,nUmbral,M_NULL);//pixeles validos valdran 0xff
    MimBinarize(paramThread->pThis->M_acum_aux,paramThread->pThis->M_mascara_aux,M_IN_RANGE,nUmbral,(NUMVAL*theApp.m_ParamIni.BarridoPatrones.nImagenAcum) - 3);//pixeles validos valdran 0xff, -3 porque la camara no da el valor maximo, si no el anterior y porque M_IN_RANGE es inclusivo

    MimShift(paramThread->pThis->M_mascara_aux,paramThread->pThis->M_mascara_aux,-7); //pixeles validos valdran 1

    //Sumar contador
    MimArith(paramThread->pThis->M_mascara_aux, paramThread->pThis->M_cont_campos[nBanda], paramThread->pThis->M_cont_campos[nBanda], M_ADD);

    //Marcar campos en overlay
    MbufClear(M_overlay_normal, TRANSPARENTE);
    MimArith(M_overlay_normal,1,M_overlay_normal,M_SUB_CONST);
    MimArith(M_overlay_normal,paramThread->pThis->M_mascara_aux,M_overlay_normal,M_ADD);

}

/**************************  toma_campoPat  *********************************
AUXILIAR a avanza_y_toma_campo
*****************************************************************************/
int CBarridoPatron::toma_campoPat(PARAM_THREAD* paramThread)
{
	int i;

    int filtro_antiguo = theApp.Rueda.GetFilter();

    ASSERT(theApp.m_ParamIni.BarridoPatrones.nImagenAcum > 0);

	for (i=0; i < theApp.m_ParamIni.nBandas; i++) 
    {
		// Cambio de filtro si es necesario
		if ( theApp.m_ParamIni.filtro[i] !=  filtro_antiguo)  
        {
			if (theApp.Rueda.ChangeFilter(theApp.m_ParamIni.filtro[i]) == false)
				return -1;
            filtro_antiguo = theApp.m_ParamIni.filtro[i];
            toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo. 
            toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
		}

        MimShift(M_imagen1,M_banda[i], 4); //Necesario para que luego funcione la progresion

        toma2_acumulada(paramThread->pThis->M_acum_aux, theApp.m_ParamIni.BarridoPatrones.nImagenAcum);

        MimArith(paramThread->pThis->M_acum_aux, paramThread->pThis->M_banda_acum[i], paramThread->pThis->M_banda_acum[i], M_ADD); //Acumulamos la imagen completa

        if (paramThread->pDialogo->GetTipoPatron()) // PATRON REFLECTANCIA ALTA
        {
            // Despues de acumular, calculamos la mascara para esta banda
            CalcularMascara(paramThread,i);

            MimArith(paramThread->pThis->M_acum_aux, paramThread->pThis->M_mascara_aux, paramThread->pThis->M_acum_aux, M_MULT); //se ponen a 0 los pixeles no validos
        }

		MimArith(M_acum_aux, paramThread->pThis->M_banda_enmascarada_acum[i], paramThread->pThis->M_banda_enmascarada_acum[i], M_ADD); //Acumulamos la imagen enmascarada

		toma2_continua(M_imagen1);
	}

	return 0;
}

void CBarridoPatron::AjusteAutomaticoExposicion(CFormPatron* pDialogo)
{
    PARAM_THREAD* paramThread = new PARAM_THREAD; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pThis = this;
    paramThread->pDialogo = pDialogo;

    AfxBeginThread(AjusteAutomaticoExposicionThread, (void *)paramThread);
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// THREAD 
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
UINT CBarridoPatron::AjusteAutomaticoExposicionThread( LPVOID Param )
{
	long  histograma[NUMVAL];	// Vector auxiliar para cálculo de histogramas.
	double media_histograma = 0,sigma_histograma = 0;
	double gris_teorico = 0;
	int exposicion_min = limCam.MinExposicion, exposicion_max = limCam.MaxExposicion; //limites auxiliares para acotar aproximaciones iterativas
	double UMBRAL_DIFERENCIA_EXPOSICION = 2; //si la diferencia entre el minimo y maximo es menor que este valor se para
	double mejor_error = INT_MAX;//necesario para elegir entre las ultimas dos exposiciones 
	int    mejor_exposicion = 0; //necesario para elegir entre las ultimas dos exposiciones
	bool bFinBucle = false;
    unsigned __int8 nFiltro;

    PARAM_THREAD* paramThread = (PARAM_THREAD*)Param;

    // OJO: esto se hace porque puede ser que el usuario haya enfocado manualmente antes
    // El enfoque se puede haber producido en cualquier filtro
    theApp.Rueda.zObservacion = mspWhere(Z_) + theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] - 
                                        theApp.m_ParamIni.Rueda.diferencia_z[theApp.Rueda.GetFilter()]; 

    for (int i = 0; i<theApp.m_ParamIni.nBandas;i++) //al cambiar NUM_BANDAS veces de filtro, el filtro se quedará en el que estaba
	{
        nFiltro = theApp.m_ParamIni.filtro[i];

		//calculamos el nivel de gris para este filtro
        if (BITS_CAMARA == 12) 
		    gris_teorico = theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro] / (theApp.m_ParamIni.escala/16);//convertimos REFLECTANCIA -> Nivel de gris 
        else 
		    gris_teorico = theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro] / theApp.m_ParamIni.escala;//convertimos REFLECTANCIA -> Nivel de gris 

		if (gris_teorico != 0) // Si reflectancia es 0, no se debe ajustar la exposicion para este filtro
		{
		    if (theApp.Rueda.ChangeFilter(nFiltro) == false)
            {
                ASSERT(FALSE);
                CString message;
                message.Format("Error al cambiar el filtro %d",nFiltro);
                AfxMessageBox(message);
			    return -1;
            }
			bFinBucle = false;
			exposicion_min = limCam.MinExposicion;// reiniciamos limites auxiliares para acotar aproximaciones iterativas
			exposicion_max = limCam.MaxExposicion; 
			mejor_error = INT_MAX;
			mejor_exposicion = 0;
			do //APROXIMACIONES ITERATIVAS
			{
				// Calculamos el histograma actual y la media
				toma2_unica(M_imagen1);		// Tomo una imagen única para su procesado
	            MimHistogram(M_imagen1, paramThread->pThis->M_histo12);
	            MimGetResult(paramThread->pThis->M_histo12, M_VALUE, histograma);
				procesa_histograma(histograma,&media_histograma,&sigma_histograma);

				//Nos quedamos con la mejor exposicion para decidir al final entre los dos mejores
				if (fabs(media_histograma - gris_teorico) < mejor_error 
					&& histograma[NUMVAL-1] == 0) // esto ultimo para evitar saturacion
				{
					mejor_error = fabs(media_histograma - gris_teorico);
					mejor_exposicion = theApp.m_ParamIni.Rueda.exposicion[nFiltro];
				}

				//Acotamos
				if (media_histograma < gris_teorico // subir nivel gris -> subir exposicion
					&& histograma[NUMVAL-1] == 0) // esto ultimo para evitar saturacion
				{
					exposicion_min = theApp.m_ParamIni.Rueda.exposicion[nFiltro]; //exposicion mas alta de la que tenemos ahora
				}
				else
				{
					// Si hay que bajar la exposicion O HAY SATURACION
					exposicion_max = theApp.m_ParamIni.Rueda.exposicion[nFiltro]; //exposicion mas alta de la que tenemos ahora
				}

				// modifica la exposicion para este filtro
				ASSERT(int((exposicion_max + exposicion_min+0.5)/2) >= limCam.MinExposicion);
				ASSERT(int((exposicion_max + exposicion_min+0.5)/2) <= limCam.MaxExposicion);
                theApp.m_ParamIni.Rueda.exposicion[nFiltro] = int((exposicion_max + exposicion_min+0.5)/2);
				ModificaExposicion(theApp.m_ParamIni.Rueda.exposicion[nFiltro]);
                toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
                toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
				Sleep(TIEMPO_AJUSTE_EXPOSICION); //hay que dar tiempo a que se estabilice la exposicion

				if (exposicion_max - exposicion_min < UMBRAL_DIFERENCIA_EXPOSICION)
				{
					bFinBucle = true;
					//Hay que decidir entre la exposicion actual y la del otro limite (que puede ser mejor)
					if (fabs(media_histograma - gris_teorico) > mejor_error)
					{
						//Es mejor la exposicion del otro limite
                        theApp.m_ParamIni.Rueda.exposicion[nFiltro] = mejor_exposicion;
				        ModificaExposicion(mejor_exposicion);
                        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
                        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
					}
				}
			} while (!bFinBucle);
		}
        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
	}

    // Dejar en filto predefinido
	theApp.Rueda.ChangeFilter(theApp.m_ParamIni.Rueda.posFiltro);

	toma2_continua(M_imagen1);	// Continuo tomando imágenes continuas.

    paramThread->pDialogo->FinExposicion();

    delete paramThread;

    return 0;
}

