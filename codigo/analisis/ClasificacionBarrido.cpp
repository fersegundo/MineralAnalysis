#include "stdafx.h"
#include "ClasificacionBarrido.h"
#include "Clasificacion.h"
#include "ClasificaBarrido.h"
#include "ClasificaBarridoDlg.h" //para acualizacion progreso
//#include "AsociacionesDlg.h" //para acualizacion progreso
#include "Minerales.h"
#include "..\librerias\LibTiff\InterfazLibTiff.h" // control imagenes tiff (escala reflectancia-gris)

// PRUEBAS BORRAR
#include "..\librerias\procesoaux\gestion_mensajes.h"
char fichLog[LONGITUD_TEXTO];

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClasificaBarridoApp theApp;

ClasificacionBarrido::ClasificacionBarrido()
{
    m_bufBarrido        = NULL;
    m_arrCount          = NULL;
    m_arrCountAcum      = NULL;

    // buffers mil necesarios para mostrar o guardar las imagenes en falso color (usados en ambas pasadas)
    M_Clasificacion      = M_NULL;
    M_lut_clasificacion  = M_NULL;
}

ClasificacionBarrido::~ClasificacionBarrido()
{
    LiberarMemoria();

    if (m_arrCount != NULL)
        delete [] m_arrCount;
    if (m_arrCountAcum != NULL)
        delete [] m_arrCountAcum;

    if(M_lut_clasificacion!=M_NULL)
        MbufFree(M_lut_clasificacion);
    if (M_Clasificacion != M_NULL)
        MbufFree(M_Clasificacion);
}

void ClasificacionBarrido::LiberarMemoria()
{
    if (m_bufBarrido != NULL)
    {
        /*
	    for(int i = 0; i < theApp.m_nCampos_y; i++)
        {
	        for(int j = 0; j < theApp.m_nCampos_x; j++)
            {
		        delete [] m_bufBarrido[i][j];
		        m_bufBarrido[i][j] = NULL;
            }
		    delete [] m_bufBarrido[i];
            m_bufBarrido[i] = NULL;
        }
        */
	    delete [] m_bufBarrido;
        m_bufBarrido = NULL;
    }
}

ClasificacionBarrido::Init()
{
    //Inicilizacion
    m_clasificacion.SetMinerales(&theApp.m_minerales);

    int nMin = theApp.m_minerales.GetCount();
    m_arrCount  = new unsigned int[nMin];
    m_arrCountAcum  = new unsigned int[nMin];

    // reservar e inicializar lut
	MbufAllocColor(M_DEFAULT_HOST, 3L, 256L, 1L, 8L+M_UNSIGNED, M_LUT, &M_lut_clasificacion);
	MbufPutColor2d(M_lut_clasificacion, M_RGB24+M_PACKED, M_ALL_BAND, 0, 0,
		            256, 1, &theApp.m_minerales.m_lut_clasificacion);

    //No se puede reservar memoria para m_bufBarrido aqui porque aun no se han leido las dimensiones del barrido. Se hará en PrimeraPasada
}

// Estructura para pasar al thread de barrido (PrimeraPasada: filtrar asociaciones)
typedef struct PARAM_THREAD_ASOCIACIONES
{
    ClasificacionBarrido*       pThis;
    //HWND                        hDialogo;
    CAsociacionesBarridoDlg*           pDialogo;
    double                      dUmbralDistancia;
    double                      dUmbralConfiabilidad;
    CString                     csMuestra;
} PARAM_THREAD_ASOCIACIONES;

// Estructura para pasar al thread de barrido (SegundaPasada: clasificacion)
typedef struct PARAM_THREAD_CLASIFICACION
{
    ClasificacionBarrido*       pThis;
    //HWND                        hDialogo;
    CClasificaBarridoDlg*       pDialogo;
    double                      dUmbralDistancia;
    double                      dUmbralConfiabilidad;
    double                      dRefMin;
    CString                     csMuestra;
} PARAM_THREAD_CLASIFICACION;


// Realiza la clasificacion de todo el barrido en la primera pasada
// pDialogo - necesario si hay que actualizar el progreso de clasificacion de minerales
void ClasificacionBarrido::PrimeraPasada(double dUmbralDistancia, double dUmbralConfiabilidad,
                                         CString csMuestra, 
                                         CAsociacionesBarridoDlg* pDialogo)
{
    PARAM_THREAD_ASOCIACIONES* paramThread = new PARAM_THREAD_ASOCIACIONES; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pThis = this;
    //paramThread->hDialogo = pDialogo->m_hWnd;
    paramThread->pDialogo = pDialogo;
    paramThread->dUmbralDistancia = dUmbralDistancia;
    paramThread->dUmbralConfiabilidad = dUmbralConfiabilidad;
    paramThread->csMuestra = csMuestra;

    AfxBeginThread(PrimeraPasadaThread, (void *)paramThread);
}

// Realiza la clasificacion de todo el barrido en la segunda pasada
// pDialogo - necesario si hay que actualizar el progreso de clasificacion de minerales
void ClasificacionBarrido::SegundaPasada(double dUmbralDistancia, double dUmbralConfiabilidad, double dRefMin,
                                         CString csMuestra,
                                         CClasificaBarridoDlg* pDialogo)
{
    PARAM_THREAD_CLASIFICACION* paramThread = new PARAM_THREAD_CLASIFICACION; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pThis                  = this;
    //paramThread->hDialogo = pDialogo->m_hWnd;
    paramThread->pDialogo               = pDialogo;
    paramThread->dUmbralDistancia       = dUmbralDistancia;
    paramThread->dUmbralConfiabilidad   = dUmbralConfiabilidad;
    paramThread->dRefMin                = dRefMin;
    paramThread->csMuestra              = csMuestra;

    AfxBeginThread(SegundaPasadaThread, (void *)paramThread);
}


UINT ClasificacionBarrido::PrimeraPasadaThread( LPVOID Param )
{
    PARAM_THREAD_ASOCIACIONES* paramThread = (PARAM_THREAD_ASOCIACIONES*)Param;

sendlog_timestamp(theApp.m_log,"Inicio PrimeraPasada\n");
    bool bPrimerCampo               = true;
    int nAnchoxAltoImagen       = -1;
    CString csCampo;
    list<CMineralClasificado> listClasificados; // lista de minerales clasificados
    unsigned int nCountOscuros           = 0; // numero de pixels demasiado oscuros (no clasificable). Resultado de clasificacion
    unsigned int nCountOscurosTotal      = 0; // numero de pixels demasiado oscuros (no clasificable). Total
    MIL_ID* arrImagenesMIL = NULL; // variable a reusar en CargarImagenes (se reserva la primera vez que se llama a CargarImagenes pero se libera al final de esta funcion)

    // variables de informacion relativa a la umbralizacion por cada mineral (TOTAL)
    // m_arrInfDer es global (para usarlo en segunda pasada)
    unsigned int* arrInfIzq;
    unsigned int* arrSupDer;
    unsigned int* arrSupIzq;

    int numMin = theApp.m_minerales.GetCount();

    arrInfIzq           = new unsigned int[numMin];
    arrSupDer           = new unsigned int[numMin];
    arrSupIzq           = new unsigned int[numMin];

    memset(paramThread->pThis->m_arrCountAcum,0,numMin*sizeof(unsigned int));
    memset(arrInfIzq,0,numMin*sizeof(unsigned int));
    memset(arrSupDer,0,numMin*sizeof(unsigned int));
    memset(arrSupIzq,0,numMin*sizeof(unsigned int));

    // Si ya se hizo una pasada, hay que liberar primero por si el numero de campos es diferente
    if (paramThread->pThis->m_bufBarrido != NULL) 
        paramThread->pThis->LiberarMemoria(); 
    //paramThread->pThis->m_bufBarrido = new BYTE**[theApp.m_nCampos_y];//buffer para la imagen de salida (clasificada selectivamente) 

    bool bEscalaCargada = false;
    for (int i=0; i< theApp.m_nCampos_y;i++)
    {
//        paramThread->pThis->m_bufBarrido[i] = new BYTE* [theApp.m_nCampos_x];

        for (int j=0; j< theApp.m_nCampos_x;j++)
        {
sendlog_timestamp(theApp.m_log,".. Otro campo ..\n");
            //Componemos el nombre y cargamos las imagenes en el modulo de clasificacion
            if (theApp.m_bBarrido2d) //barrido de 1 dimension
                csCampo.Format("%s%s_%03d_%03d",theApp.m_configuracion.m_csDefaultPath, paramThread->csMuestra,i+1,j+1); 
            else
                csCampo.Format("%s%s_%03d",theApp.m_configuracion.m_csDefaultPath, paramThread->csMuestra,j+1); 

            // Leer informacion TIF (escala reflectancia y profundidad)
            bool bError = false;
            try
            {
                CString csNombreImagen;
                csNombreImagen.Format("%s_%02d.tif",csCampo,theApp.m_arrFiltros[1]);
                INTERFAZ_LIBTIF::LeerTags(csNombreImagen,theApp.m_nBitsProfundidad,theApp.m_dEscalaReflectancia); //Leemos de la imagen la profundidad en bits y la escala reflectancia-gris
                if (theApp.m_nBitsProfundidad < 0 )
                    theApp.m_nBitsProfundidad = 8; // por defecto las imagenes estan en 8 bit
                if (theApp.m_dEscalaReflectancia < 0)
                    bError = true;
            }
            catch(...)
            {
                bError = true;
            }
            if (bError)
            {
                AfxMessageBox("No se ha podido leer la escala de reflectancia-gris de la imagen, se usará 0.4");
                theApp.m_dEscalaReflectancia = 0.4;
            }
            else
                bEscalaCargada = true;

            if (!paramThread->pThis->m_clasificacion.CargarImagenes(arrImagenesMIL, csCampo, &theApp.m_arrFiltros[1], theApp.m_nBandas-1, NULL)) //Nos saltamos el pancromatico 
            {
                CString csError;
                csError.Format("Error al cargar el campo %s",csCampo);
                AfxMessageBox(csError);
                ASSERT(FALSE);
                continue;
            }

sendlog_timestamp(theApp.m_log," .. imagenes cargadas ..\n");
            if (bPrimerCampo)
            {
                bPrimerCampo = false;
                // solo en el primer campo
                theApp.m_nAnchoImagen = paramThread->pThis->m_clasificacion.GetAnchoImagen();
                theApp.m_nAltoImagen = paramThread->pThis->m_clasificacion.GetAltoImagen();
                nAnchoxAltoImagen = theApp.m_nAnchoImagen * theApp.m_nAltoImagen;
                if(nAnchoxAltoImagen <= 1)
                {
                    AfxMessageBox("Error al obtener ancho de la imagen");
                    continue;
                }
                MbufAlloc2d(M_DEFAULT_HOST, theApp.m_nAnchoImagen,theApp.m_nAltoImagen, 8 + M_UNSIGNED, M_IMAGE, &paramThread->pThis->M_Clasificacion);
                MbufControl(paramThread->pThis->M_Clasificacion,M_ASSOCIATED_LUT,paramThread->pThis->M_lut_clasificacion);

                paramThread->pThis->m_bufBarrido = new BYTE [nAnchoxAltoImagen];

sendlog_timestamp(theApp.m_log,"primer campo!\n");
            }
            //paramThread->pThis->m_bufBarrido[i][j] = new BYTE [nAnchoxAltoImagen];
            memset(paramThread->pThis->m_bufBarrido, 0, nAnchoxAltoImagen*sizeof(BYTE));

            //CLASIFICACION --------------- Obtenemos los resultados de clasificacion
            listClasificados.clear();
            if (!paramThread->pThis->m_clasificacion.Clasificar(listClasificados,nCountOscuros, theApp.m_dEscalaReflectancia, theApp.m_nBitsProfundidad))
            {
                CString csError;
                csError.Format("Error al clasificar el campo %s",csCampo);
                AfxMessageBox(csError);
                ASSERT(FALSE);
                continue;
            }
            nCountOscurosTotal += nCountOscuros;
sendlog_timestamp(theApp.m_log,"clasificado\n");

            // Lo sumamos al total
            // CLASIFICACION ---------- calculamos la informacion de clasificacion selectiva
            paramThread->pThis->m_clasificacion.ClasificacionSelectiva(paramThread->dUmbralConfiabilidad,paramThread->dUmbralDistancia, 
                                                    -1,3,2,1, //asignamos el valor 255 a lo que no cumpla condiciones y el valor de mineral a lo que si
                                                    paramThread->pThis->m_arrCountAcum, arrInfIzq,arrSupDer,arrSupIzq,
//                                                    paramThread->pThis->m_bufBarrido[i][j]);
                                                    paramThread->pThis->m_bufBarrido);

            //Actualiza progresion dialogo
            //CClasificaBarridoDlg* pDialogo = (CClasificaBarridoDlg*) CWnd::FromHandle(paramThread->hDialogo);
            if (paramThread->pDialogo)
                paramThread->pDialogo->ActualizaProgresionMinerales(paramThread->pThis->m_arrCountAcum, i*theApp.m_nCampos_x+j+1);

sendlog_timestamp(theApp.m_log,"clasificacion selectiva\n");
            // Salvar imagen de clasificacion selectiva
//            MbufPut2d(paramThread->pThis->M_Clasificacion, 0, 0, theApp.m_nAnchoImagen, theApp.m_nAltoImagen, paramThread->pThis->m_bufBarrido[i][j]); //si dibujamos por lineas, no hace falta esto
            MbufPut2d(paramThread->pThis->M_Clasificacion, 0, 0, theApp.m_nAnchoImagen, theApp.m_nAltoImagen, paramThread->pThis->m_bufBarrido); //si dibujamos por lineas, no hace falta esto
            csCampo += "_PRIMERA_PASADA.tif";
            MbufExport((char*)(LPCTSTR)csCampo,M_TIFF,paramThread->pThis->M_Clasificacion);
sendlog_timestamp(theApp.m_log,"exportado\n");
        }
    }
sendlog_timestamp(theApp.m_log,"fin barrido\n");

    // CLASIFICACION ---------- Guardamos el log TOTAL
    CString csFicheroLog;
    csFicheroLog.Format("%s%s_PRIMERA_PASADA.txt",theApp.m_configuracion.m_csDefaultPath, paramThread->csMuestra);
    paramThread->pThis->m_clasificacion.ClasificacionSelectivaLog(  paramThread->dUmbralConfiabilidad,paramThread->dUmbralDistancia, 
                                                nCountOscurosTotal,
                                                paramThread->pThis->m_arrCountAcum, arrInfIzq,arrSupDer,arrSupIzq,
                                                csFicheroLog);

sendlog_timestamp(theApp.m_log,"fin pasada\n\n");

    delete [] arrInfIzq;
    delete [] arrSupDer;
    delete [] arrSupIzq;

    // Liberamos la memoria para CargarImagenes
    for (i = 0; i < theApp.m_nBandas-1; i++) //m_nBandas-1 porque en CargarImagenes nos saltamos el pancromatico
    {
        if (arrImagenesMIL[i] != M_NULL)
        {
            MbufFree(arrImagenesMIL[i]);
            arrImagenesMIL[i] = M_NULL;
        }
    }
    delete [] arrImagenesMIL;

    // Filtramos los minerales posibles segun compatibilidad con asociaciaciones    
    int* arrAsocCompatibles = new int[theApp.m_asociaciones.m_nAsociaciones]; // posibilidades de compatibilidad con los minerales originales calculadas para cada asociacion
    theApp.m_asociaciones.FiltrarAsociaciones(paramThread->pThis->m_arrCountAcum, arrAsocCompatibles);

    if (paramThread->pDialogo)
        paramThread->pDialogo->FinFiltrar(arrAsocCompatibles);
    delete [] arrAsocCompatibles;

    delete paramThread;
    return 0;
}

UINT ClasificacionBarrido::SegundaPasadaThread( LPVOID Param )
{
    int i;
    int nAnchoxAlto                 = -1;
    bool bPrimerCampo               = true;
    PARAM_THREAD_CLASIFICACION* paramThread = (PARAM_THREAD_CLASIFICACION*)Param;

sendlog_timestamp(theApp.m_log,"Inicio SegundaPasada\n");
    CString csCampo;
    unsigned int nCountOscuros               = 0;
    unsigned int nCountNoClasificados        = 0;
    unsigned int nCountOscurosAcum           = 0;
    unsigned int nCountNoClasificadosAcum    = 0;
    MIL_ID* arrImagenesMIL = NULL; // variable a reusar en CargarImagenes (se reserva la primera vez que se llama a CargarImagenes pero se libera al final de esta funcion)

    CString csFicheroLog;
    csFicheroLog.Format("%s%s_CLASIFICACION_BARRIDO.txt",theApp.m_configuracion.m_csDefaultPath, paramThread->csMuestra);
    FILE* ficheroResultadoBarrido = fopen(csFicheroLog,"wt");
    //Reservamos memoria para m_bufBarrido si no ha sido reservada en primer barrido (cuando se seleccionan las asociaciones manualmente)
    if (paramThread->pThis->m_bufBarrido != NULL) 
        paramThread->pThis->LiberarMemoria(); //si ya se hizo una pasada, hay que volver a reservar por si el numero de campos es diferente
    //paramThread->pThis->m_bufBarrido = new BYTE**[theApp.m_nCampos_y];//buffer para la imagen de salida (clasificada selectivamente) 

    int nMinerales = theApp.m_minerales.GetCount();
    memset(paramThread->pThis->m_arrCount, 0, nMinerales*sizeof(int)); // Porque vamos a reclasificarlos todos
    memset(paramThread->pThis->m_arrCountAcum, 0, nMinerales*sizeof(int)); // Porque vamos a reclasificarlos todos

    // Para cada mineral, comprobamos que sea compatible con las asociaciones, si no, lo desactivamos
    theApp.m_asociaciones.ActualizarMineralesActivosPorAsociaciones();

    bool bEscalaCargada = false;
    for (i=0; i< theApp.m_nCampos_y;i++)
    {
//        paramThread->pThis->m_bufBarrido[i] = new BYTE* [theApp.m_nCampos_x];

        for (int j=0; j< theApp.m_nCampos_x;j++)
        {
sendlog_timestamp(theApp.m_log,".. Otro campo ..\n");
            //Componemos el nombre y cargamos las imagenes en el modulo de clasificacion
            if (theApp.m_bBarrido2d) //barrido de 1 dimension
                csCampo.Format("%s%s_%03d_%03d",theApp.m_configuracion.m_csDefaultPath, paramThread->csMuestra,i+1,j+1); 
            else
                csCampo.Format("%s%s_%03d",theApp.m_configuracion.m_csDefaultPath, paramThread->csMuestra,j+1); 

            // Leer informacion TIF (escala reflectancia y profundidad)
            bool bError = false;
            try
            {
                CString csNombreImagen;
                csNombreImagen.Format("%s_%02d.tif",csCampo,theApp.m_arrFiltros[1]);
                INTERFAZ_LIBTIF::LeerTags(csNombreImagen,theApp.m_nBitsProfundidad,theApp.m_dEscalaReflectancia); //Leemos de la imagen la profundidad en bits y la escala reflectancia-gris
                if (theApp.m_nBitsProfundidad < 0 )
                    theApp.m_nBitsProfundidad = 8; // por defecto las imagenes estan en 8 bit
                if (theApp.m_dEscalaReflectancia < 0)
                    bError = true;
            }
            catch(...)
            {
                bError = true;
            }
            if (bError)
            {
                AfxMessageBox("No se ha podido leer la escala de reflectancia-gris de la imagen, se usará 0.4");
                theApp.m_dEscalaReflectancia = 0.4;
            }
            else
                bEscalaCargada = true;

            if (!paramThread->pThis->m_clasificacion.CargarImagenes(arrImagenesMIL, csCampo, &theApp.m_arrFiltros[1], theApp.m_nBandas-1, NULL)) //Nos saltamos el pancromatico 
            {
                CString csError;
                csError.Format("Error al cargar el campo %s",csCampo);
                AfxMessageBox(csError);
                ASSERT(FALSE);
                continue;
            }
sendlog_timestamp(theApp.m_log," .. imagenes cargadas ..\n");

            if (bPrimerCampo)
            {
                bPrimerCampo = false;
                // Hay que obtener ancho y alto y reservar M_Clasificacion, pero solo en el primer campo
                theApp.m_nAnchoImagen = paramThread->pThis->m_clasificacion.GetAnchoImagen();
                theApp.m_nAltoImagen = paramThread->pThis->m_clasificacion.GetAltoImagen();
                nAnchoxAlto = theApp.m_nAnchoImagen * theApp.m_nAltoImagen;
                MbufAlloc2d(M_DEFAULT_HOST, theApp.m_nAnchoImagen,theApp.m_nAltoImagen, 8 + M_UNSIGNED, M_IMAGE, &paramThread->pThis->M_Clasificacion);
                MbufControl(paramThread->pThis->M_Clasificacion,M_ASSOCIATED_LUT,paramThread->pThis->M_lut_clasificacion);

                paramThread->pThis->m_bufBarrido = new BYTE [nAnchoxAlto];

                // Cabecera del fichero de resultados
                fprintf(ficheroResultadoBarrido," Informacion de clasificacion de una imagen multiespectral\n");
                fprintf(ficheroResultadoBarrido," Umbral distancia: %.2lf Umbral confiabilidad: %.2lf\n", paramThread->dUmbralDistancia, paramThread->dUmbralConfiabilidad);
                fprintf(ficheroResultadoBarrido," Dimensiones de las imagenes: %dx%d\n\n",theApp.m_nAnchoImagen, theApp.m_nAltoImagen);
            }
            ASSERT(paramThread->pThis->M_Clasificacion != M_NULL);                  //deberia estar reservado en primera pasada 
            ASSERT(paramThread->pThis->M_lut_clasificacion != M_NULL);              //deberia estar reservado en primera pasada
            ASSERT(paramThread->pThis->m_bufBarrido != M_NULL);                     //deberia estar reservado en primera pasada
            ASSERT(theApp.m_nAnchoImagen != -1);                //deberia estar reservado en primera pasada
            ASSERT(theApp.m_nAltoImagen != -1);                 //deberia estar reservado en primera pasada

//            paramThread->pThis->m_bufBarrido[i][j] = new BYTE [nAnchoxAlto];
            memset(paramThread->pThis->m_bufBarrido, 0, nAnchoxAlto*sizeof(BYTE));

            //CLASIFICACION --------------- Obtenemos los resultados de clasificacion
            memset(paramThread->pThis->m_arrCount, 0, nMinerales*sizeof(int)); // Reseteamos el contador de cada campo (no acumulado)
            if (!paramThread->pThis->m_clasificacion.Clasificar(paramThread->pThis->m_arrCount,paramThread->pThis->m_arrCountAcum, nCountOscuros, nCountNoClasificados,
//                                                                paramThread->pThis->m_bufBarrido[i][j],
                                                                paramThread->pThis->m_bufBarrido,
                                                                paramThread->dUmbralDistancia, paramThread->dUmbralConfiabilidad,
                                                                theApp.m_dEscalaReflectancia, theApp.m_nBitsProfundidad,
                                                                paramThread->dRefMin))
            {
                CString csError;
                csError.Format("Error al clasificar el campo %s",csCampo);
                AfxMessageBox(csError);
                ASSERT(FALSE);
                continue;
            }
sendlog_timestamp(theApp.m_log,"clasificado\n");

            //Actualiza progresion dialogo
            nCountOscurosAcum           += nCountOscuros;
            nCountNoClasificadosAcum    += nCountNoClasificados;
            
            // Actualiza en pantalla y guarda en fichero
            paramThread->pDialogo->ActualizaProgresionMinerales( paramThread->pThis->m_arrCount, paramThread->pThis->m_arrCountAcum,  
                                                                nCountOscuros, nCountNoClasificados,nCountOscurosAcum, nCountNoClasificadosAcum,
                                                                i*theApp.m_nCampos_x+j+1, i+1 == theApp.m_nCampos_y && j+1 == theApp.m_nCampos_x,
                                                                ficheroResultadoBarrido);

sendlog_timestamp(theApp.m_log,"clasificacion selectiva\n");
            // Salvar imagen de clasificacion selectiva
//            MbufPut2d(paramThread->pThis->M_Clasificacion, 0, 0, theApp.m_nAnchoImagen, theApp.m_nAltoImagen, paramThread->pThis->m_bufBarrido[i][j]); //si dibujamos por lineas, no hace falta esto
            MbufPut2d(paramThread->pThis->M_Clasificacion, 0, 0, theApp.m_nAnchoImagen, theApp.m_nAltoImagen, paramThread->pThis->m_bufBarrido); //si dibujamos por lineas, no hace falta esto
            csCampo += "_SEGUNDA_PASADA.tif";
            MbufExport((char*)(LPCTSTR)csCampo,M_TIFF,paramThread->pThis->M_Clasificacion);
sendlog_timestamp(theApp.m_log,"exportado\n");
        }
    } 
sendlog_timestamp(theApp.m_log,"fin barrido\n");

    // Liberamos la memoria para CargarImagenes
    for (i = 0; i < theApp.m_nBandas-1; i++)//m_nBandas-1 porque en CargarImagenes nos saltamos el pancromatico
    {
        if (arrImagenesMIL[i] != M_NULL)
        {
            MbufFree(arrImagenesMIL[i]);
            arrImagenesMIL[i] = M_NULL;
        }
    }
    delete [] arrImagenesMIL;
    arrImagenesMIL = NULL;

    if (ficheroResultadoBarrido != NULL)
        fclose(ficheroResultadoBarrido);

    delete paramThread;
    return 0;
}

