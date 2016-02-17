
#include "stdafx.h"
#include "TomaAuto.h"
#include "Barrido.h"
#include "FormTomaAuto.h" //necesario para callback de thread de barrido 
#include "Progreso.h"
#include "..\librerias\include\procesoAux.h"
#include "..\librerias\ControlMicroscopio\control_microscopio.h"
#include "..\librerias\ControlMicroscopio\control_objetivos.h"
#include "..\librerias\LibBarrido\autoenfoque.h" //autoenfoque
#include "..\librerias\LibBarrido\control_barrido.h" //captura_bandas
#include "..\librerias\LibBarrido\calibracion.h"
#include "..\librerias\ControlDisp\control_camara.h" //ModificaExposicion
#include <math.h>
#include <direct.h> //mkdir
#include <errno.h> //mkdir

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CTomaAutoApp theApp;

// Estructura para pasar al thread de barrido
typedef struct PARAM_THREAD
{
    CBarrido*       pThis;
    CFormTomaAuto*   pDialogo;
} PARAM_THREAD;


//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CBarrido::CBarrido()
{
    m_nCampos_x             = -1;       // numero de campos del barrido
    m_nCampos_y             = -1;
    m_dDimensionReal_x      = -1;       // pixels entre campos (sumando lo que ocupa el propio campo y la separacion entre campos)
    m_dDimensionReal_y      = -1;       // pixels entre campos (sumando lo que ocupa el propio campo y la separacion entre campos)
    m_bCancelarBarrido      = false;    // su true, se cancela el thread de barrido
    
    //inicializamos puntos de enfoque
    InitPuntos();
}

CBarrido::~CBarrido()
{

}

//inicializamos puntos de enfoque
void CBarrido::InitPuntos()
{
    for (int j=0;j<NUM_PUNTOS_PLANO;j++)
    {
        for (int i=X_;i<=Z_;i++)
            m_arrPuntosControl[j].coord[i] = -1;
        m_arrPuntosControl[j].peso = 1;
    }
}


// Actualiza variable interna con la posicion 
// Mientras se define el barrido, se llamará periodicamente por el thread de posicion (en el dialogo)
void CBarrido::ActualizaPosicion(point& pActual)
{
    pActual = mspWhereP();
//    m_pActual = pActual;
}

//Establece una esquina de barrido. Se guarda temporalmente en inicio y fin
//Devuelve true si la Z del punto establecido es identica a la establecida para el punto final (para saltarse el plano de regresion)
bool CBarrido::SetInicio(point& pActual)
{
    m_pActual = mspWhereP();
    for (int i=X_;i<=Z_;i++)
    {
        m_arrPuntosControl[0].coord[i] = m_pActual.coord[i];
    }

    pActual = m_pActual;

    return (m_arrPuntosControl[0].coord[Z_] == m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_]);
}

//Establece una esquina de barrido. Se guarda temporalmente en inicio y fin
//Devuelve true si la Z del punto establecido es identica a la establecida para el punto inicial (para saltarse el plano de regresion)
bool CBarrido::SetFin(point& pActual)
{
    m_pActual = mspWhereP();
    for (int i=X_;i<=Z_;i++)
    {
        m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[i] = m_pActual.coord[i];
    }

    pActual = m_pActual;

    return (m_arrPuntosControl[0].coord[Z_] == m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_]);
}

// Recalcula las 4 esquinas (puntos principales de enfoque) 
// a partir de los puntos definidos por el usuario
void CBarrido::Intermedios()
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
int CBarrido::EnfoquePuntosIntermedios(int& nIndicePuntoEnfocado)
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

// El numero de campos estará definido o bien por el pocentaje de cobertura o por el espaciado 
// nCobertura - porcentaje de cobertura (puede ser mayor de 100). Si -1, usar espaciado
// nEspaciado - espaciado entre campos a usar. Puede ser negativo o 0
void CBarrido::CalcularCampos(int nCobertura, double dPaso_x, double dPaso_y, int& nCampos_x, int& nCampos_y, double& dAnchoTotal, double& dAltoTotal)
{
    ASSERT(nCobertura != 0);

    if (nCobertura != -1)
    {
        m_dDimensionReal_x = (theApp.m_configuracion.m_nDimension_barrido_x * 100/nCobertura) * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo;
        m_dDimensionReal_y = (theApp.m_configuracion.m_nDimension_barrido_y * 100/nCobertura) * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo;
    }
    else
    {
        m_dDimensionReal_x = dPaso_x;
        m_dDimensionReal_y = dPaso_y;
    }

    if (theApp.m_configuracion.m_nDimension_barrido_x!=0 && theApp.m_configuracion.m_nDimension_barrido_y!=0)
    {
        nCampos_x = (int)ceil(fabs(m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[X_] - m_arrPuntosControl[0].coord[X_]) / m_dDimensionReal_x); 
        nCampos_y = (int)ceil(fabs(m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Y_] - m_arrPuntosControl[0].coord[Y_]) / m_dDimensionReal_y);
        
        // Sumamos 1 porque el inicio y fin son los centros del primer y ultimo campo, de tal manera que cabe uno mas
        // (ejemplo: si el campo ocupa 100 y el barrido esta definido con inicio en 0 y fin en 100,
        //  cabrán dos campos: uno con centro en 0 y otro con centro en 100)
        nCampos_x ++;
        nCampos_y ++;

        dAnchoTotal = nCampos_x * m_dDimensionReal_x;
        dAltoTotal = nCampos_y * m_dDimensionReal_y;
        m_nCampos_x = nCampos_x;
        m_nCampos_y = nCampos_y;
    }
}
int CBarrido::GetCamposX()
{
    return m_nCampos_x;
}

int CBarrido::GetCamposY()
{
    return m_nCampos_y;
}

// pDialogo: puntero al dialogo necesario para que el thread indique su final
bool CBarrido::Barrer(CFormTomaAuto* pDialogo)
{
    if (m_arrPuntosControl[0].coord[Z_] != m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_])
    {
        //Calculamos el plano de regresion
        CString ErrorMessage;
	    pointlist plist;
	    dim_pointlist(NUM_PUNTOS_PLANO,plist);		// Dimensionamos la lista de puntos.

        ASSERT (m_nCampos_x != -1);
        ASSERT (m_nCampos_y != -1);

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
            if ((pDialogo->m_bAutoenfoque == FALSE && errorZ > dMaxDistPlano ) || errorZ < 0)
            {
                ErrorMessage.Format("Error: la predicción de Z (mediante el plano de regresión calculado) del punto intermedio %d esta a %f micras de su enfoque real", i+1, errorZ);
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

        free_pointlist(plist);				// Liberamos la memoria empleada
    }
    else
    {
        m_plano.A = -1;
        m_plano.B = -1;
        m_plano.C = -1;
    }


    //Crear directorio si no esta ya creado
    if (mkdir(theApp.m_configuracion.m_csDirectorioBarrido) != 0)
	{
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
		    AfxMessageBox("NO se puede crear el directorio");
            return false;
        }
	}

    // Guardar fichero de informacion de barrido
    CString csFicheroInfo = theApp.m_configuracion.m_csDirectorioBarrido + theApp.csNombreToma + "_INFO_BARRIDO.txt";
    FILE* f_info_barrido = fopen(csFicheroInfo, "wt");

    fprintf(f_info_barrido,"# Informacion relativa al barrido de la muestra %s en este directorio\n\n",theApp.csNombreToma);
    fprintf(f_info_barrido,"%dx%d\n\n",m_nCampos_x,m_nCampos_y);
    fprintf(f_info_barrido,"%d\n",theApp.m_ParamIni.nBandas);
    for (int b=0;b<theApp.m_ParamIni.nBandas;b++)
        fprintf(f_info_barrido,"%d\t",theApp.m_ParamIni.filtro[b]+1);
    fprintf(f_info_barrido,"\n");
    fclose(f_info_barrido);

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

            // Cargar correccion geometrica
            theApp.ActualizarCorreccionGeometrica();
            pDialogo->ActualizarEscala(); //actualizamos la escala en el dialogo
        }
        // preparar correccion con nuevos patrones
	    if (prepara_correccion(&theApp.m_ParamIni) != 0) // HACER Thread?
	    {
            pDialogo->HayCorreccion(false);
		    AfxMessageBox("Fallo en prepara_correccion");
            return false;
        }
        pDialogo->HayCorreccion(true);

    }

    PARAM_THREAD* paramThread = new PARAM_THREAD; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pThis = this;
    paramThread->pDialogo = pDialogo;

    pDialogo->InicializaProgresion(m_nCampos_x*m_nCampos_y);
    m_bCancelarBarrido = false;
    AfxBeginThread(BarrerThread, (void *)paramThread);

    return true;
}

// avisa al thread para que termine
void CBarrido::Cancelar()
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
UINT CBarrido::BarrerThread( LPVOID Param )
{
	static char mensaje[1024];

    PARAM_THREAD* paramThread = (PARAM_THREAD*)Param;

	char nombre_fich[512];
    ASSERT(theApp.m_ParamIni.Cam.anchoImagen >= theApp.m_configuracion.m_nDimension_barrido_x);
    ASSERT(theApp.m_ParamIni.Cam.altoImagen >= theApp.m_configuracion.m_nDimension_barrido_y);
    long nChildInicioX    = (long)(theApp.m_ParamIni.Cam.anchoImagen - theApp.m_configuracion.m_nDimension_barrido_x)/2;
    int nChildInicioY     = (long)(theApp.m_ParamIni.Cam.altoImagen - theApp.m_configuracion.m_nDimension_barrido_y)/2;
    double x, y, z = -1;
	double xReal, yReal, zReal;
    double margenX, margenY;

    // Calculamos los márgenes que hay que aplicar a la zona de barrido para que ésta quede centrada en el mosaico.
    margenX = ( (paramThread->pThis->m_nCampos_x - 1) * paramThread->pThis->m_dDimensionReal_x -
        fabs(paramThread->pThis->m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[X_] - paramThread->pThis->m_arrPuntosControl[0].coord[X_]) ) / 2.0;
    margenY = ( (paramThread->pThis->m_nCampos_y - 1) * paramThread->pThis->m_dDimensionReal_y -
        fabs(paramThread->pThis->m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Y_] - paramThread->pThis->m_arrPuntosControl[0].coord[Y_]) ) / 2.0;
    // Redondeamos los márgenes a la cincuentena de micras
    margenX = 50 * floor(margenX / 50);
    margenY = 50 * floor(margenY / 50);

    z = paramThread->pThis->m_arrPuntosControl[0].coord[Z_]; //Z por defecto para el caso de que no se use plano de regresion

/*
    for (int i=0; i<paramThread->pThis->m_nCampos_x; i++)
    {
        // Desplazamiento a la línea j + 1
        x = paramThread->pThis->m_arrPuntosControl[0].coord[X_] - margenX + i * paramThread->pThis->m_dDimensionReal_x;
        mspGoCheck(X_, x);
        sprintf(mensaje, "Comenzamos columna %d", i + 1);
        sendlog("\nBarrerThread", mensaje);
        for (int j=0; j<paramThread->pThis->m_nCampos_y; j++)
        {
			// Desplanamiento a la columna i + 1
            y = paramThread->pThis->m_arrPuntosControl[0].coord[Y_] - margenY + j * paramThread->pThis->m_dDimensionReal_y;
            mspGoCheck(Y_, y);
            sprintf(mensaje, "Campo %d,%d  Posición teórica (%.2f, %.2f)", j + 1, i + 1, x, y);
            sendlog("\nBarrerThread", mensaje);
			yReal = mspWhere(Y_);
			xReal = mspWhere(X_);
            sprintf(mensaje, "Campo %d,%d  Posición real (%.2f, %.2f)", j + 1, i + 1, xReal, yReal);
            sendlog("BarrerThread", mensaje);
			// Determinamos la z correspondiente al plano de regresión
            if (paramThread->pThis->m_plano.A != -1)
            {
                z = paramThread->pThis->m_plano.A * x + paramThread->pThis->m_plano.B * y + paramThread->pThis->m_plano.C;
	            //mspGoCheck(Z_, z + BACKSLASHZ);
	            //mspGoCheck(Z_, z);
            }
            // Hay que actualizar la z del filtro predefinido 
            // actualizo la z absoluta del filtro predefinido 
            theApp.Rueda.zObservacion = z; 
	        sprintf(mensaje, "Posición de enfoque teórico: %.2f)", z);
            sendlog("BarrerThread",mensaje);

            // Actualizar posicion y progreso en dialogo
            paramThread->pDialogo->IncrementaProgresion();
			zReal = mspWhere(Z_);
            paramThread->pDialogo->MostrarPosicion(xReal,yReal,zReal);

            // Autoenfoque
            ::SendMessage(paramThread->pDialogo->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0);
            if (paramThread->pDialogo->m_bAutoenfoque)
            {

                // Se ha de enfocar (tanto manual como automaticamente)
                // con el filtro "de enfoque" (o "de referencia", es decir el monocromatico)
                theApp.Rueda.ChangeFilter(theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]);

                z =  z - theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] + 
                         theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]]; //actualizamos la z al valor correcto para el filtro de enfoque

                double contraste;
			    if (autoenfoque(z,z,&z,&contraste, theApp.m_ParamIni.objetivos[mspGetObjetivo()-1].dAumento, true, paramThread->pDialogo->GetControlImagen()) == -1)
                {
                    // ERROR
                    AfxMessageBox("Barrido - No es posible realizar autoenfoque");
                    z = theApp.Rueda.zObservacion;
                }
                else
                {
                    // dejamos "z" en el punto de enfoque del filtro "predefinido" (es decir, el de observacion) para que se actualice luego "Rueda.zPredefinido"
                    z = z + theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] - 
                            theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]]; 
                    theApp.Rueda.zObservacion = z; 
          	        sprintf(mensaje, "Posición final de autoenfoque: %.2f)", z);
                    sendlog("BarrerThread",mensaje);
                }
            }

            // Actualizar posicion en dialogo
            paramThread->pDialogo->MostrarPosicion(x,y,z);

    
	        sprintf(mensaje, "Adquiriendo imagen %d,%d  Posición (%.2f, %.2f, %.4f)", j + 1, i + 1, x, y, z);
            sendlog("BarrerThread",mensaje);
            
            //Tomar Imagenes (en M_banda[])
            captura_bandas( theApp.m_ParamIni, theApp.m_configuracion.m_nImagenAcum,theApp.Rueda, theApp.m_configuracion.m_dEscalaReflectancia);
            if (paramThread->pThis->m_bCancelarBarrido == true)
            {
                paramThread->pThis->InitPuntos(); //reseteamos para permitir un nuevo barrido
                paramThread->pDialogo->PermitirNuevoBarrido(true);
                delete paramThread;
                return 1;
            }
            // correcciones (posicion, escala, etc)
            theApp.CorregirImagenes(); // las imagenes corregidas estarán en M_banda
            // HACER?: que esto sea un thread. esperar al thread anterior (para que no acumulen (problema de acceso a mismas variables?))

            //Añadimos campo a la imagen de progreso
            paramThread->pDialogo->m_pProgreso->NuevoCampo(i,j);

		    //	Una vez capturadas y corregidas las imágenes, se guardan en disco 
		    for (int f=0; f < theApp.m_ParamIni.nBandas; f++)  
            {
                // Guardamos en el tamaño de salida definido por el usuario
                MIL_ID Mchild;
        	    MbufChild2d(M_banda[f], 
                    nChildInicioX, 
                    nChildInicioY, 
                    theApp.m_configuracion.m_nDimension_barrido_x, 
                    theApp.m_configuracion.m_nDimension_barrido_y, 
                    &Mchild);
			    sprintf(nombre_fich, "%s%s_%03d_%03d_%02d%s", theApp.m_configuracion.m_csDirectorioBarrido, theApp.csNombreToma, j+1, i+1, theApp.m_ParamIni.filtro[f]+1, EXT_IMAG);
			    graba_imagen_campo_bits(theApp.m_ParamIni, nombre_fich, Mchild, theApp.m_configuracion.m_nBits, theApp.m_configuracion.m_dEscalaReflectancia); 
                MbufFree(Mchild);
            }

        } //for (campos y)
    } // for (campos x) 
*/
    
    for (int j=0; j<paramThread->pThis->m_nCampos_y; j++)
    {
        // Desplazamiento a la línea j + 1
        y = paramThread->pThis->m_arrPuntosControl[0].coord[Y_] - margenY + j * paramThread->pThis->m_dDimensionReal_y;
        //mspGoCheck(Y_, y);
        sprintf(mensaje, "Comenzamos fila %d", j + 1);
        sendlog("\nBarrerThread", mensaje);
        for (int i=0; i<paramThread->pThis->m_nCampos_x; i++)
        {
			// Desplazamiento a la columna i + 1
            x = paramThread->pThis->m_arrPuntosControl[0].coord[X_] - margenX + i * paramThread->pThis->m_dDimensionReal_x;
            mspGoCheckXY(x, y);
            sprintf(mensaje, "Campo %d,%d  Posición teórica (%.2f, %.2f)", j + 1, i + 1, x, y);
            sendlog("\nBarrerThread", mensaje);
			yReal = mspWhere(Y_);
			xReal = mspWhere(X_);
            sprintf(mensaje, "Campo %d,%d  Posición real (%.2f, %.2f)", j + 1, i + 1, xReal, yReal);
            sendlog("BarrerThread", mensaje);
			// Determinamos la z correspondiente al plano de regresión
            if (paramThread->pThis->m_plano.A != -1)
            {
                z = paramThread->pThis->m_plano.A * x + paramThread->pThis->m_plano.B * y + paramThread->pThis->m_plano.C;
	            //mspGoCheck(Z_, z + BACKSLASHZ);
	            //mspGoCheck(Z_, z);
            }
            // Hay que actualizar la z del filtro predefinido 
            // actualizo la z absoluta del filtro predefinido 
            theApp.Rueda.zObservacion = z; 
	        sprintf(mensaje, "Posición de enfoque teórico: %.2f)", z);
            sendlog("BarrerThread",mensaje);

            // Actualizar posicion y progreso en dialogo
            paramThread->pDialogo->IncrementaProgresion();
			zReal = mspWhere(Z_);
            paramThread->pDialogo->MostrarPosicion(xReal,yReal,zReal);

            // Autoenfoque
            ::SendMessage(paramThread->pDialogo->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0);
            if (paramThread->pDialogo->m_bAutoenfoque)
            {

                // Se ha de enfocar (tanto manual como automaticamente)
                // con el filtro "de enfoque" (o "de referencia", es decir el monocromatico)
                theApp.Rueda.ChangeFilter(theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]);

                z =  z - theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] + 
                         theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]]; //actualizamos la z al valor correcto para el filtro de enfoque

                double contraste;
			    if (autoenfoque(z,z,&z,&contraste, theApp.m_ParamIni.objetivos[mspGetObjetivo()-1].dAumento, true, paramThread->pDialogo->GetControlImagen()) == -1)
                {
                    // ERROR
                    AfxMessageBox("Barrido - No es posible realizar autoenfoque");
                    z = theApp.Rueda.zObservacion;
                }
                else
                {
                    // dejamos "z" en el punto de enfoque del filtro "predefinido" (es decir, el de observacion) para que se actualice luego "Rueda.zPredefinido"
                    z = z + theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] - 
                            theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]]; 
                    theApp.Rueda.zObservacion = z; 
          	        sprintf(mensaje, "Posición final de autoenfoque: %.2f)", z);
                    sendlog("BarrerThread",mensaje);
                }
            }

            // Actualizar posicion en dialogo
            paramThread->pDialogo->MostrarPosicion(x,y,z);

    
	        sprintf(mensaje, "Adquiriendo imagen %d,%d  Posición (%.2f, %.2f, %.4f)", j + 1, i + 1, x, y, z);
            sendlog("BarrerThread",mensaje);
            
            //Tomar Imagenes (en M_banda[])
            captura_bandas( theApp.m_ParamIni, theApp.m_configuracion.m_nImagenAcum,theApp.Rueda, theApp.m_configuracion.m_dEscalaReflectancia);
            if (paramThread->pThis->m_bCancelarBarrido == true)
            {
                paramThread->pThis->InitPuntos(); //reseteamos para permitir un nuevo barrido
                paramThread->pDialogo->PermitirNuevoBarrido(true);
                delete paramThread;
                return 1;
            }
            // correcciones (posicion, escala, etc)
            theApp.CorregirImagenes(); // las imagenes corregidas estarán en M_banda
            // HACER?: que esto sea un thread. esperar al thread anterior (para que no acumulen (problema de acceso a mismas variables?))

            //Añadimos campo a la imagen de progreso
            paramThread->pDialogo->m_pProgreso->NuevoCampo(i,j);

		    //	Una vez capturadas y corregidas las imágenes, se guardan en disco 
		    for (int f=0; f < theApp.m_ParamIni.nBandas; f++)  
            {
                // Guardamos en el tamaño de salida definido por el usuario
                MIL_ID Mchild;
        	    MbufChild2d(M_banda[f], 
                    nChildInicioX, 
                    nChildInicioY, 
                    theApp.m_configuracion.m_nDimension_barrido_x, 
                    theApp.m_configuracion.m_nDimension_barrido_y, 
                    &Mchild);
			    sprintf(nombre_fich, "%s%s_%03d_%03d_%02d%s", theApp.m_configuracion.m_csDirectorioBarrido, theApp.csNombreToma, j+1, i+1, theApp.m_ParamIni.filtro[f]+1, EXT_IMAG);
			    graba_imagen_campo_bits(theApp.m_ParamIni, nombre_fich, Mchild, theApp.m_configuracion.m_nBits, theApp.m_configuracion.m_dEscalaReflectancia); 
                MbufFree(Mchild);
            }

        } //for (campos y)
    } // for (campos x)

    paramThread->pThis->InitPuntos(); //reseteamos para permitir un nuevo barrido
    paramThread->pDialogo->PermitirNuevoBarrido(false);

    delete paramThread;

    return 0;
}

// A partir de las imagenes guardadas en disco, genera la imagen mosaico
// pDialogo - necesario para avanzar el progreso de la generaciond e mosaicos
void CBarrido::GenerarMosaicos(CFormTomaAuto* pDialogo)
{
	char nombre_fich[512];
    MIL_ID M_aux;
    MIL_ID M_mosaico;
	MbufAlloc2d(M_sistema, theApp.m_configuracion.m_nDimension_barrido_x, theApp.m_configuracion.m_nDimension_barrido_y, 8/*theApp.m_ParamIni.Cam.profundidad*/+M_UNSIGNED,
		M_IMAGE+M_PROC, &M_aux);
	MbufAlloc2d(M_sistema, theApp.m_configuracion.m_nDimension_barrido_x*m_nCampos_x, theApp.m_configuracion.m_nDimension_barrido_y*m_nCampos_y, 8/*theApp.m_ParamIni.Cam.profundidad*/+M_UNSIGNED,
		M_IMAGE+M_PROC, &M_mosaico);
	for (int f=0; f < theApp.m_ParamIni.nBandas; f++)  
    {
        pDialogo->IncrementaProgresionMosaico(); // por cada banda
        MbufClear(M_mosaico,0);
        for (int i=0;i<m_nCampos_x;i++)
        {
            for (int j=0;j<m_nCampos_y;j++)
            {
		        sprintf(nombre_fich, "%s%s_%03d_%03d_%02d%s", theApp.m_configuracion.m_csDirectorioBarrido, theApp.csNombreToma, j+1, i+1, theApp.m_ParamIni.filtro[f]+1, EXT_IMAG);
		        //carga_imagen_campo8bit(nombre_fich, M_aux, theApp.m_ParamIni.Cam.profundidad); 
		         carga_imagen_campo_bits(nombre_fich, M_aux, 8); 

                MbufCopyClip(M_aux,M_mosaico,i*theApp.m_configuracion.m_nDimension_barrido_x,j*theApp.m_configuracion.m_nDimension_barrido_y);
            }
        }
		sprintf(nombre_fich, "%s%s_%02d%s", theApp.m_configuracion.m_csDirectorioBarrido, theApp.csNombreToma, theApp.m_ParamIni.filtro[f]+1, EXT_IMAG);
		graba_imagen_campo(nombre_fich, M_mosaico); 

    }
	MbufFree(M_aux);
	MbufFree(M_mosaico);
}


