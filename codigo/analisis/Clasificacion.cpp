#include "stdafx.h"
//#include <math.h> //para floor
//#include <sys/stat.h> //para stat

#include "Clasificacion.h"
#include <sys/stat.h> //para stat

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


Clasificacion::Clasificacion()
{
    //Inicilizacion
    m_pMinerales            = NULL;
    M_lut_clasificacion     = M_NULL;

    // punteros a buffers de MIL controlados por ControlImagenes
    M_pClasificacion             = NULL;
    M_pConfiabilidad             = NULL;
    M_pDistancia                 = NULL;

    // buffer para las imagenes para procesamiento
    m_buffImagenesClasificacion = NULL;

    // buffers para imagenes en el display (opcional)
    m_bufClasificacion      = NULL;
    m_bufConfiabilidad      = NULL;
    m_bufDistancia          = NULL;
    m_nNumBandas            = -1;
    m_nAnchoImagen          = -1;
    m_nAltoImagen           = -1;
}

Clasificacion::~Clasificacion()
{
    Liberar();

    // CLASIFICACION
    if(M_lut_clasificacion!=M_NULL)
        MbufFree(M_lut_clasificacion);
}

//libera los buffers
void Clasificacion::Liberar()
{
    if (m_bufClasificacion != NULL)
    {
        delete[] m_bufClasificacion;
        m_bufClasificacion = NULL;
    }
    if (m_bufConfiabilidad != NULL)
    {
        delete[] m_bufConfiabilidad;
        m_bufConfiabilidad = NULL;
    }
    if (m_bufDistancia != NULL)
    {
        delete[] m_bufDistancia;
        m_bufDistancia = NULL;
    }
    if (m_buffImagenesClasificacion != NULL)
    {
        for (int b=0;b<m_nNumBandas;b++)
        {
            if (m_buffImagenesClasificacion[b] != NULL)
            {
                delete [] m_buffImagenesClasificacion[b];//free(m_buffImagenesClasificacion[b]);
                m_buffImagenesClasificacion[b] = NULL;
            }
        }
        delete [] m_buffImagenesClasificacion;
        m_buffImagenesClasificacion = NULL;
    }
    m_nNumBandas            = -1;
    m_nAnchoImagen          = -1;
    m_nAltoImagen           = -1;
}

void Clasificacion::SetMinerales(CMinerales* pMinerales)
{
    m_pMinerales = pMinerales;
}

// Carga las imagenes (o un subconjunto de ellas dado por pRect) de una imagen multiespectral dada por nombre
// - arrImagenesMIL: referencia a puntero de array de imagenes (con memoria previamente reservada para las imagenes (se pasa por optimizacion))
// - csNombreMuestra: string con el nombre de la muestra (incluido el campo) que cargar
// - nNumBandas: numero de bandas a cargar (estará ya descontado el pancromatico)
// - arrBandas: array con los filtros que hay que cargar de fichero (tantos como nNumBandas)
// - pRect: NULL si se quiere cargar las imagenes completas
// Devuelve false si hay algun error
bool Clasificacion::CargarImagenes(MIL_ID*& arrImagenesMIL, CString csNombreMuestra, int* arrBandas, int nNumBandas, CRect* pRect)
{
    bool bRet;
    CString csNombreImagen;
	struct stat stFileInfo;
    int i;
    bool bReservarMemoria = false; //para saber si reservar memoria

    if (arrImagenesMIL == NULL) //primera vez (primer campo)
    {
        bReservarMemoria = true;
        arrImagenesMIL = new MIL_ID [nNumBandas];
    }

    for (i = 0; i < nNumBandas; i++)
    {
        csNombreImagen.Format("%s_%02d.tif",csNombreMuestra,arrBandas[i]);
	    // Comprobaciones
	    if (stat(csNombreImagen,&stFileInfo) != 0)
		    return false;

        if (bReservarMemoria)
        {
            // Reservar memoria solo la primera vez
            long lSizeX, lSizeY;
            MbufDiskInquire(csNombreImagen.GetBuffer(500),M_SIZE_X, &lSizeX);
            MbufDiskInquire(csNombreImagen.GetBuffer(500),M_SIZE_Y, &lSizeY);
            MbufAlloc2d(M_DEFAULT_HOST, lSizeX,lSizeY, 16 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &arrImagenesMIL[i]);
        }
        if (MbufImport(csNombreImagen.GetBuffer(500),M_TIFF,M_LOAD,M_NULL,&arrImagenesMIL[i]) == M_NULL)
//        if (MbufImport(csNombreImagen.GetBuffer(500),M_TIFF,M_RESTORE,M_DEFAULT_HOST,&arrImagenesMIL[i]) == M_NULL)
            return false;
        if (arrImagenesMIL[i] == NULL)
            return false;
    }

    bRet = CargarImagenes(arrImagenesMIL, nNumBandas, pRect);

    return bRet;
}

//HACER: tenemos informacion de que filtros se usaron en el barrido (arrBandas), pero no tenemos informacion de que filtros se usaron en entrenamiento
//       Si añadimos esta info en entrenamiento, podemos tenerlo en cuenta a la hora de clasificar
bool Clasificacion::CargarImagenes(MIL_ID* M_Muestra, int nNumBandas, CRect* pRect)
{
    long nAnchoImagen, nAltoImagen, nAnchoxAltoImagen;
    MIL_ID M_AreaAux[MAX_NUM_IMAGENES];

    m_nNumBandas = nNumBandas;
    if (pRect !=NULL)
    {
        nAnchoImagen = pRect->right - pRect->left;
        nAltoImagen = pRect->bottom - pRect->top;
        for (int b=0;b<nNumBandas;b++) 
        {
            if (M_Muestra[b]!=M_NULL)
	            MbufChild2d(M_Muestra[b], pRect->left, pRect->top,pRect->right - pRect->left, pRect->bottom - pRect->top, &M_AreaAux[b]);
        }
    }
    else
    {
        MbufInquire(M_Muestra[1],M_SIZE_X,&nAnchoImagen);
        MbufInquire(M_Muestra[1],M_SIZE_Y,&nAltoImagen);
        for (int b=0;b<nNumBandas;b++)
        {
            M_AreaAux[b] = M_Muestra[b];
        }
    }
    m_nAnchoImagen = nAnchoImagen;
    m_nAltoImagen = nAltoImagen;
    nAnchoxAltoImagen = nAnchoImagen*nAltoImagen;

    if (m_buffImagenesClasificacion == NULL)
    {
        m_buffImagenesClasificacion = new unsigned short *[nNumBandas ]; //valores espectrales.
        for (int b=0;b<nNumBandas;b++) 
        {
            m_buffImagenesClasificacion[b] = new unsigned short [nAnchoxAltoImagen];
            memset(m_buffImagenesClasificacion[b],0,sizeof(unsigned short) * nAnchoxAltoImagen);
        }
        //memset(m_buffImagenesClasificacion,0,sizeof(unsigned short) * nNumBandas);
    }

    for (int b=0;b<nNumBandas;b++) 
    {
        MbufGet(M_AreaAux[b], m_buffImagenesClasificacion[b]);
        //liberar M_AreaAux si se reservo memoria mediante MbufChild
        if (M_AreaAux[b] != M_Muestra[b]) //alternativamente if (pRect != NULL)
            MbufFree(M_AreaAux[b]);

    }

    return true;
}

// Devuelve el ancho de la imagen (previamente cargada con CargarImagenes)
int Clasificacion::GetAnchoImagen()
{
    return m_nAnchoImagen;
}

// Devuelve el alto de la imagen (previamente cargada con CargarImagenes)
int Clasificacion::GetAltoImagen()
{
    return m_nAltoImagen;
}

// devuelve false, si ha habido un error o si ya se habia clasificado y unicamente se ha mostrado la imagen ya clasificada
bool Clasificacion::InicializaDisplay(CStatic*	pDisplay, MIL_ID& Mildisplay, MIL_ID& M_Clasificacion,MIL_ID& M_Confiabilidad,MIL_ID& M_Distancia)
{
    if (pDisplay == NULL || m_nAnchoImagen < 0 || m_nAltoImagen < 0)
        return false;

    if (M_Clasificacion != M_NULL) // no hace falta clasificar de nuevo, ya se hizo con anterioridad
    {
        MdispSelectWindow(Mildisplay, M_Clasificacion, *pDisplay);
        MbufControl(M_Clasificacion,M_ASSOCIATED_LUT,M_lut_clasificacion);
        return false;
    }

    // se ha cambiado de imagen
    if (m_nAnchoImagen == -1 || m_nAltoImagen == -1) //si no se ha llamado a CargarImagen todavia
    {
        int nAnchoImagen,nAltoImagen;
        MdispInquire(Mildisplay,M_WINDOW_SIZE_X,&nAnchoImagen);
        MdispInquire(Mildisplay,M_WINDOW_SIZE_X,&nAltoImagen);
        m_nAnchoImagen = nAnchoImagen;
        m_nAltoImagen = nAltoImagen;
    }
    int nAnchoxAltoImagen;
    nAnchoxAltoImagen = m_nAnchoImagen * m_nAltoImagen;
    MbufAlloc2d(M_DEFAULT_HOST, m_nAnchoImagen,m_nAltoImagen, 8 + M_UNSIGNED, M_IMAGE + M_DISP, &M_Clasificacion);
    MbufAlloc2d(M_DEFAULT_HOST, m_nAnchoImagen,m_nAltoImagen, 8 + M_UNSIGNED, M_IMAGE + M_DISP, &M_Confiabilidad);
    MbufAlloc2d(M_DEFAULT_HOST, m_nAnchoImagen,m_nAltoImagen, 8 + M_UNSIGNED, M_IMAGE + M_DISP, &M_Distancia);

    M_pClasificacion        = &M_Clasificacion;
    M_pConfiabilidad        = &M_Confiabilidad;
    M_pDistancia            = &M_Distancia;

    MbufClear(M_Clasificacion,0);
    MdispSelectWindow(Mildisplay, M_Clasificacion, *pDisplay);
    MbufControl(M_Clasificacion,M_ASSOCIATED_LUT,M_DEFAULT ); //desasociamos para que se pinte linea a linea (se asociará de nuevo al final)

    if (M_lut_clasificacion == M_NULL)
    {
        // primera vez que se ejecuta, reservar e inicializar lut
	    MbufAllocColor(M_DEFAULT_HOST, 3L, 256L, 1L, 8L+M_UNSIGNED, M_LUT, &M_lut_clasificacion);
	    MbufPutColor2d(M_lut_clasificacion, M_RGB24+M_PACKED, M_ALL_BAND, 0, 0,
		    256, 1, &m_pMinerales->m_lut_clasificacion);
    }

    return true;
}
///////////////////////////////////////////////////////////////////////////////////////////////////
// Devuelve true si se ha realizado la clasificacion, false si no se ha realizado (porque ya se realizó antes)
// listClasificados     resultado con la lista de clasificados
// nCountOscuros        resultado de pixeles no clasificados por ser demasiado oscuros
// dEscalaReflectancia  escala necesaria para convertir los valores de gris a reflectancia
// nBitsProfundidad     profundidad en bits necesaria para convertir los valores de gris a reflectancia
// dRefMin              Reflectancia minima para que un pixel sea clasificado, si no será clasificado como demasiado oscuro
bool Clasificacion::Clasificar(list<CMineralClasificado> &listClasificados, unsigned int& nCountOscuros, double dEscalaReflectancia, int nBitsProfundidad, double dRefMin)
{
    int i, j, offset;

    if (m_nNumBandas < 1 || m_nAnchoImagen < 0 || m_nAltoImagen < 0 || m_buffImagenesClasificacion == NULL || m_pMinerales == NULL)
    {
        ASSERT(FALSE);
        return false;
    }

    // memoria buffers auxiliares
    double* m_arrReflectanciasClasificacion      = new double[m_nNumBandas]; //valores espectrales de un pixel para identificar.
    int nNumMinerales = m_pMinerales->GetCount();
    int* arrCountMinerales = new int[nNumMinerales];                //lista para contabilizar el numero de veces que se ha identificado cada mineral en la imagen
    double* arrDistanciaMinerales = new double[nNumMinerales];      //lista para almacenar la distancia media de cada mineral en la imagen
    double* arrConfiabilidadMinerales = new double[nNumMinerales];  //lista para almacenar la confiabilidad de cada mineral en la imagen

    int nAnchoxAltoImagen;
    nAnchoxAltoImagen = m_nAnchoImagen * m_nAltoImagen;
    if (m_bufClasificacion == NULL)
        m_bufClasificacion      = new BYTE[nAnchoxAltoImagen];//buffer para la imagen de salida (clasificada) 
    memset(m_bufClasificacion, 0, nAnchoxAltoImagen);
    if (m_bufConfiabilidad == NULL)
        m_bufConfiabilidad      = new BYTE[nAnchoxAltoImagen];//buffer para los valores de confiabilidad en cada pixel
    memset(m_bufConfiabilidad, 0, nAnchoxAltoImagen*sizeof(BYTE));
    if (m_bufDistancia == NULL)
        m_bufDistancia      = new BYTE[nAnchoxAltoImagen];//buffer para los valores de distancia en cada pixel 
    memset(m_bufDistancia, 0, nAnchoxAltoImagen*sizeof(BYTE));

    // resultados indentificacion
    listClasificados.clear();
    CResultadoIdentificacion primero;
    CResultadoIdentificacion segundo;


    //inicializacion buffers auxiliares
    memset(arrCountMinerales, 0, nNumMinerales*sizeof(int));
    memset(arrDistanciaMinerales, 0, nNumMinerales*sizeof(double));
    memset(arrConfiabilidadMinerales, 0, nNumMinerales*sizeof(double));

//    MbufClear(M_overlay_normal, TRANSPARENTE );
    nCountOscuros = 0;
    offset = 0;
    for (j=0; j<m_nAltoImagen; j++)
    {
        for (i=0; i<m_nAnchoImagen; offset++, i++)
        {
            double dRefMedia = 0; // para pixels demasiado oscuros (ref minima)
            for (int b=0;b<m_nNumBandas; b++)
            {
                //nuevo valor para la linea espectral
                 // CONVERTIMOS de valor de gris en la profundidad que este la imagen a 8 bit (al representarlo en double, conservamos toda la informacion)
                if (nBitsProfundidad != 8)
                    m_arrReflectanciasClasificacion[b] = (double)m_buffImagenesClasificacion[b][offset] / (1<<(nBitsProfundidad-8));
                else
                    m_arrReflectanciasClasificacion[b] = m_buffImagenesClasificacion[b][offset];
                m_arrReflectanciasClasificacion[b] = m_arrReflectanciasClasificacion[b] * dEscalaReflectancia; // CONVERTIMOS a reflectancia (desde valor de gris en 8 bit)
                if (m_arrReflectanciasClasificacion[b] > 100)
                    m_arrReflectanciasClasificacion[b] = 100; //a veces dEscalaReflectancia es ligeramente superior (redondeos del procesador)

                dRefMedia += m_arrReflectanciasClasificacion[b];// para pixels demasiado oscuros (ref minima)
            }

            dRefMedia = dRefMedia / m_nNumBandas;
            if (dRefMedia < dRefMin)
            {
                nCountOscuros++;
                m_bufClasificacion[offset] = 0; //m_listaColor[nColorIndex];
                m_bufConfiabilidad[offset] = 0; //m_listaColor[nColorIndex];
                m_bufDistancia[offset] = 0; //m_listaColor[nColorIndex];
            }
            else
            {
                // identificamos el mineral (con probabilidades) a partir de su espectro
                m_pMinerales->Identificar(m_arrReflectanciasClasificacion, primero, segundo);
                if (primero.m_nIndice>=0)
                {
                    ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador para despues añadirlo a la leyenda
                    arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                    double dConfiabilidad = segundo.m_dProbabilidad / primero.m_dProbabilidad;
                    arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
    //                m_pMinerales->GetAbreviatura(m_primero_clasificacion.m_nIndice);
                    m_bufClasificacion[offset] = m_pMinerales->GetIndiceColor(primero.m_nIndice);
                    //PRUEBAS: CALCULOS IMAGEN DE CONFIABILIDAD Y DISTANCIA
                    dConfiabilidad = 50 * (dConfiabilidad);
                    if (dConfiabilidad > 250)
                        dConfiabilidad = 250; //unificamos en 250 todas las distancias menores a 0.5
                    m_bufConfiabilidad[offset] = (BYTE)dConfiabilidad;
                    //double dA = (1.0/primero.m_dProbabilidad);
                    //double dB = ( dA/ 2.0);
                    //double dC = ceil(dB*255);
                    double dDistancia = 250 - 2 * primero.m_dProbabilidad;//ceil(((1.0/primero.m_dProbabilidad) / 2.0)*255); // 0 ... inf, 0 mucha distancia, 1 poquisima (0.5)
                    if (dDistancia > 255)
                        dDistancia = 255; //unificamos en 255 todas las distancias menores a 0.5
                    else if (dDistancia < 0)
                        dDistancia = 0;
                    m_bufDistancia[offset] = (BYTE)dDistancia;
                }
                else
                {
                    // ERROR: No se ha podido identificar porque no hay ningun mineral entrenado
                    ASSERT(FALSE);
                    m_bufClasificacion[offset] = 0; //m_listaColor[nColorIndex];
                    m_bufConfiabilidad[offset] = 0; //m_listaColor[nColorIndex];
                    m_bufDistancia[offset] = 0; //m_listaColor[nColorIndex];
                    
                }
            }
        }
        if (M_pClasificacion != NULL && *M_pClasificacion != M_NULL) // hay display
        {
            // dibuja la linea en el display
            MbufPut2d(*M_pClasificacion, 0, j, m_nAnchoImagen, 1, &m_bufClasificacion[offset-m_nAnchoImagen]);
        }
    }

    //Pruebas
    int total = 0;
    for (int g=0;g<nNumMinerales;g++)
        total += arrCountMinerales[g];
    ASSERT(total+nCountOscuros == m_nAnchoImagen*m_nAltoImagen);

    //añadimos cada mineral identificado y cuantos pixeles han sido clasificados
    for (i=0;i<nNumMinerales;++i)
    {
        if (arrCountMinerales[i] > 0)
        {
            CMineralClasificado mineral;
            mineral.m_nIndex = i;
            mineral.m_nCount = arrCountMinerales[i];
            mineral.m_dConfiabilidad = arrConfiabilidadMinerales[i] / arrCountMinerales[i]; // se guarda la media
            mineral.m_dDistancia = arrDistanciaMinerales[i] / arrCountMinerales[i];         // se guarda la media
            listClasificados.push_back(mineral);
        }
    }

    listClasificados.sort(); //se ordenan por m_nCount (ver "operator<" de CMineralClasificado)

    delete[] m_arrReflectanciasClasificacion;
    delete[] arrCountMinerales;
    delete[] arrDistanciaMinerales;
    delete[] arrConfiabilidadMinerales;

    if (M_pClasificacion != NULL && *M_pClasificacion != M_NULL) // hay display
    {
    //    MbufPut2d(*M_pClasificacion, 0, 0, nAnchoImagen, nAltoImagen, m_bufClasificacion); //si dibujamos por lineas, no hace falta esto
        MbufControl(*M_pClasificacion,M_ASSOCIATED_LUT,M_lut_clasificacion);
        MbufPut2d(*M_pConfiabilidad, 0, 0, m_nAnchoImagen, m_nAltoImagen, m_bufConfiabilidad); 
        MbufPut2d(*M_pDistancia, 0, 0, m_nAnchoImagen, m_nAltoImagen, m_bufDistancia); 
    }

    return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// VERSION SEGUNDA PASADA
// Devuelve true si se ha realizado la clasificacion, false si no se ha realizado (porque ya se realizó antes)
// arrMinerales         [RESULTADO] lista de pixeles encontrados por cada mineral
// arrMineralesAcum     [RESULTADO] lista de pixeles encontrados por cada mineral. Acumulado
// nCountOscuros        [RESULTADO] numero de pixeles no clasificados debido a que son demasiado oscuros
// nCountNoClasificados [RESULTADO] numero de pixeles no clasificados debido a que no cumplen umbrales
// arrClasificados      valores de los pixels de la imagen de clasificacion
// dRefMin              Reflectancia minima para que un pixel sea clasificado, si no será clasificado como demasiado oscuro
bool Clasificacion::Clasificar(unsigned int* arrMinerales, unsigned int* arrMineralesAcum, unsigned int& nCountOscuros, unsigned int& nCountNoClasificados,
                               BYTE* arrClasificados, 
                               double dDistancia, double dConfiabilidad,
                               double dEscalaReflectancia, int nBitsProfundidad, 
                               double dRefMin)
{
    int i, j, offset;
    int nNumBandasOscuras = 0;
//char mensaje[512];
    if (m_nNumBandas < 1 || m_nAnchoImagen < 0 || m_nAltoImagen < 0 || m_buffImagenesClasificacion == NULL || m_pMinerales == NULL)
    {
        ASSERT(FALSE);
        return false;
    }

    // memoria buffers auxiliares
    double* m_arrReflectanciasClasificacion      = new double[m_nNumBandas]; //valores espectrales de un pixel para identificar.
    int nNumMinerales = m_pMinerales->GetCount();

    nCountNoClasificados    = 0;
    nCountOscuros           = 0;

    if (M_pClasificacion != NULL && *M_pClasificacion != M_NULL) // hay display
    {
        int nAnchoxAltoImagen;
        nAnchoxAltoImagen = m_nAnchoImagen * m_nAltoImagen;
        if (m_bufClasificacion == NULL)
            m_bufClasificacion      = new BYTE[nAnchoxAltoImagen];//buffer para la imagen de salida (clasificada) 
        memset(m_bufClasificacion, 0, nAnchoxAltoImagen);
        if (m_bufConfiabilidad == NULL)
            m_bufConfiabilidad      = new BYTE[nAnchoxAltoImagen];//buffer para los valores de confiabilidad en cada pixel
        memset(m_bufConfiabilidad, 0, nAnchoxAltoImagen*sizeof(BYTE));
        if (m_bufDistancia == NULL)
            m_bufDistancia      = new BYTE[nAnchoxAltoImagen];//buffer para los valores de distancia en cada pixel 
        memset(m_bufDistancia, 0, nAnchoxAltoImagen*sizeof(BYTE));
    }

    // resultados indentificacion
    CResultadoIdentificacion primero;
    CResultadoIdentificacion segundo;


//    MbufClear(M_overlay_normal, TRANSPARENTE );
    offset = 0;
    for (j=0; j<m_nAltoImagen; j++)
    {
        for (i=0; i<m_nAnchoImagen; offset++, i++)
        {

//sprintf(mensaje, " Procesando pixel %d, %d \t", i, j);
//sendlog_timestamp(theApp.m_log, mensaje);
            if (true) // hay que reclasificar todos porque algunos no se tuvieron en cuenta al FiltrarAsociaciones
            //if (arrClasificados[offset] == PIXEL_NO_CUMPLE_CONFIABILIDAD            ||
            //    arrClasificados[offset] == PIXEL_NO_CUMPLE_DISTANCIA                ||
            //    arrClasificados[offset] == PIXEL_NO_CUMPLE_DISTANCIA_CONFIABILIDAD)
            {
//sprintf(mensaje, " NO CUMPLIÓ (valor:%d)",arrClasificados[offset]);
//sendlog_timestamp(theApp.m_log, mensaje);
                // Obviamos la comparacion con oscuros (se supone que en la primera pasada ya se filtró

                double dRefMedia = 0; // para pixels demasiado oscuros (ref minima)
                for (int b=0;b<m_nNumBandas && nNumBandasOscuras != m_nNumBandas; b++)
                {
                    //nuevo valor para la linea espectral
                     // CONVERTIMOS de valor de gris en la profundidad que este la imagen a 8 bit (al representarlo en double, conservamos toda la informacion)
                    if (nBitsProfundidad != 8)
                        m_arrReflectanciasClasificacion[b] = (double)m_buffImagenesClasificacion[b][offset] / (1<<(nBitsProfundidad-8));
                    else
                        m_arrReflectanciasClasificacion[b] = m_buffImagenesClasificacion[b][offset];
                    m_arrReflectanciasClasificacion[b] = m_arrReflectanciasClasificacion[b] * dEscalaReflectancia; // CONVERTIMOS a reflectancia (desde valor de gris en 8 bit)
                    if (m_arrReflectanciasClasificacion[b] > 100)
                        m_arrReflectanciasClasificacion[b] = 100; //a veces dEscalaReflectancia es ligeramente superior (redondeos del procesador)

                    dRefMedia += m_arrReflectanciasClasificacion[b];// para pixels demasiado oscuros (ref minima)
                }

                dRefMedia = dRefMedia / m_nNumBandas;
                if (dRefMedia < dRefMin)
                {
                    nCountOscuros++;
                    continue;
                }

                m_pMinerales->Identificar(m_arrReflectanciasClasificacion, primero, segundo);

                if (primero.m_dProbabilidad < dDistancia)
                {
//sprintf(mensaje, " cumple distancia:%f < %f ",primero.m_dProbabilidad, dDistancia);
//sendlog_timestamp(theApp.m_log, mensaje);
                    if (segundo.m_dProbabilidad / primero.m_dProbabilidad > dConfiabilidad)
                    {
//sprintf(mensaje, " cumple confi:%f > %f, lo clasificamos color:%d\n",segundo.m_dProbabilidad /primero.m_dProbabilidad, dConfiabilidad,m_pMinerales->GetIndiceColor(primero.m_nIndice));
//sendlog_timestamp(theApp.m_log, mensaje);
                        //Clasificamos itAnterior
                        arrClasificados[offset] = m_pMinerales->GetIndiceColor(primero.m_nIndice);
                        arrMinerales[primero.m_nIndice]++; //incrementamos el contador
                        arrMineralesAcum[primero.m_nIndice]++; //incrementamos el contador
                    }
                    else
                    {
//sprintf(mensaje, " NO cumple confi:%f < %f",segundo.m_dProbabilidad /primero.m_dProbabilidad, dConfiabilidad);
//sendlog_timestamp(theApp.m_log, mensaje);
                        // No cumple confiabilidad
                        arrClasificados[offset] = PIXEL_NO_CUMPLE_CONFIABILIDAD;
                        nCountNoClasificados++;
                    }
                }
                else
                {
//sprintf(mensaje, " NO cumple distancia:%f > %f ",(*itAnterior).m_dProbabilidad, dDistancia);
//sendlog_timestamp(theApp.m_log, mensaje);
                    if (segundo.m_dProbabilidad / primero.m_dProbabilidad > dConfiabilidad)
                        arrClasificados[offset] = PIXEL_NO_CUMPLE_DISTANCIA;
                    else
                        arrClasificados[offset] = PIXEL_NO_CUMPLE_DISTANCIA_CONFIABILIDAD;
                     nCountNoClasificados++;
               }
            } //if no cumplió umbrales en primera pasada
        } // for i
        if (M_pClasificacion != NULL && *M_pClasificacion != M_NULL) // hay display
        {
            // dibuja la linea en el display
            MbufPut2d(*M_pClasificacion, 0, j, m_nAnchoImagen, 1, &m_bufClasificacion[offset-m_nAnchoImagen]);
        }
    } //for j

//sprintf(mensaje, "\n  finalizando clasificacion\n");
//sendlog_timestamp(theApp.m_log, mensaje);

    delete[] m_arrReflectanciasClasificacion;

    if (M_pClasificacion != NULL && *M_pClasificacion != M_NULL) // hay display
    {
    //    MbufPut2d(*M_pClasificacion, 0, 0, nAnchoImagen, nAltoImagen, m_bufClasificacion); //si dibujamos por lineas, no hace falta esto
        MbufControl(*M_pClasificacion,M_ASSOCIATED_LUT,M_lut_clasificacion);
        MbufPut2d(*M_pConfiabilidad, 0, 0, m_nAnchoImagen, m_nAltoImagen, m_bufConfiabilidad); //si dibujamos por lineas, no hace falta esto
        MbufPut2d(*M_pDistancia, 0, 0, m_nAnchoImagen, m_nAltoImagen, m_bufDistancia); //si dibujamos por lineas, no hace falta esto
    }
//sprintf(mensaje, "\n  FIN CLASIFICACION\n");
//sendlog_timestamp(theApp.m_log, mensaje);

    return true;
}

// Calcula la imagen de clasificacion selectiva (con colores segun los umbrales de distancia y confiabilidad
// Version sin informacion umbralizada
// - bufPruebas: imagen resultado con la clasificacion selectiva
void Clasificacion::ClasificacionSelectiva(double dUmbralConfiabilidad,double dUmbralDistancia,
                                           int nInfDer,int nInfIzq,int nSupDer,int nSupIzq,
                                           BYTE* bufPruebas)
{
    ClasificacionSelectiva( dUmbralConfiabilidad,dUmbralDistancia,
                             nInfDer,nInfIzq,nSupDer,nSupIzq,
                             NULL,NULL,NULL,NULL,
                             bufPruebas);
}

// Calcula la informacion umbralizada
// Version sin imagen de clasificacion
// - arrInfDer: array de contadores de cuantos pixels cumplen tanto la condicion de distancia como la de confiabilidad para cada mineral
// - arrInfIzq: array de contadores de cuantos pixels cumplen solo la condicion de distancia para cada mineral
// - arrSupDer: array de contadores de cuantos pixels cumplen solo la condicion de confiabilidad para cada mineral
// - arrSupIzq: array de contadores de cuantos pixels no cumplen ninguna condicion para cada mineral
void Clasificacion::ClasificacionSelectiva(double dUmbralConfiabilidad,double dUmbralDistancia,
                                           unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq)
{
    ClasificacionSelectiva( dUmbralConfiabilidad,dUmbralDistancia,
                             false,false,false,false,
                             arrInfDer, arrInfIzq,arrSupDer,arrSupIzq,
                             NULL);
}

// Calcula la imagen de clasificacion selectiva (con colores segun los umbrales de distancia y confiabilidad
// Version con informacion umbralizada:
// - nInfDer, nInfIzq, nSupDer, nSupIzq: valores a asignar a la imagen resultado en caso de que se cumpla cada caso especifico condicional. Si -1, se asigna el mineral
// - arrInfDer: array de contadores de cuantos pixels cumplen tanto la condicion de distancia como la de confiabilidad para cada mineral
// - arrInfIzq: array de contadores de cuantos pixels cumplen solo la condicion de distancia para cada mineral
// - arrSupDer: array de contadores de cuantos pixels cumplen solo la condicion de confiabilidad para cada mineral
// - arrSupIzq: array de contadores de cuantos pixels no cumplen ninguna condicion para cada mineral
// - bufPruebas: imagen resultado con la clasificacion selectiva
void Clasificacion::ClasificacionSelectiva(double dUmbralConfiabilidad,double dUmbralDistancia,
                                           int nInfDer,int nInfIzq,int nSupDer,int nSupIzq,
                                           unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq,
                                           BYTE* bufPruebas)
{
    if (bufPruebas!=NULL)
        memset(bufPruebas,0,m_nAnchoImagen*m_nAltoImagen);

    for (int i=0;i<m_nAnchoImagen*m_nAltoImagen;++i)
    {
        if (m_bufClasificacion[i] == 0)
        {
            // No hacer nada, se deja a 0
        }
        else 
        {

            int indice;
            if (arrInfDer != NULL)
            {
                indice = m_pMinerales->GetIndexConIndiceColor(m_bufClasificacion[i]);
//                arrTotal[indice]++;
            }

            if (m_bufConfiabilidad[i]/50 > dUmbralConfiabilidad)
            {
                if ((250 - m_bufDistancia[i])/2 < dUmbralDistancia)
                {
                    // -----  Confiabilidad ALTA bien
                    // | | |  Distancia     BAJA bien
                    // |-|-|
                    // | |x|
                    // -----  x Muy probable

                    if (bufPruebas!=NULL)
                    {
                        if (nInfDer == -1)
                            bufPruebas[i] = m_bufClasificacion[i];
                        else
                            bufPruebas[i] = nInfDer;
                    }

                    if (arrInfDer != NULL)
                        arrInfDer[indice]++;
                }
                else
                {
                    // -----  Confiabilidad ALTA bien
                    // | |x|  Distancia     alta
                    // |-|-|
                    // | | |
                    // -----  @ Unica opcion poco clara

                    if (bufPruebas!=NULL)
                    {
                        if (nSupDer == -1)
                            bufPruebas[i] = m_bufClasificacion[i];
                        else
                            bufPruebas[i] = nSupDer;
                    }
                    if (arrSupDer != NULL)
                        arrSupDer[indice]++;
                }
            }
            else
            {
                if ((250 - m_bufDistancia[i])/2 < dUmbralDistancia)
                {
                    // -----  Confiabilidad baja 
                    // | | |  Distancia     BAJA bien
                    // |-|-|
                    // |x| |
                    // -----  xxxx Muchos muy posibles minerales

                    if (bufPruebas!=NULL)
                    {
                        if (nInfIzq == -1)
                            bufPruebas[i] = m_bufClasificacion[i];
                        else
                            bufPruebas[i] = nInfIzq;
                    }
                    if (arrInfIzq != NULL)
                        arrInfIzq[indice]++;
                }
                else
                {
                    // -----  Confiabilidad baja 
                    // |x| |  Distancia     alta
                    // |-|-|
                    // | | |
                    // -----  @@@@ improbable

                    if (bufPruebas!=NULL)
                    {
                        if (nSupIzq == -1)
                            bufPruebas[i] = m_bufClasificacion[i];
                        else
                            bufPruebas[i] = nSupIzq;
                    }
                    if (arrSupIzq != NULL)
                        arrSupIzq[indice]++;
                }
            }
        }
    } //for
}

// Guardar un fichero de texto con la información de clasificacion selectiva 
void Clasificacion::ClasificacionSelectivaLog(double dUmbralConfiabilidad,double dUmbralDistancia,
                                                list<CMineralClasificado> &listClasificados, unsigned int nCountOscuros,
                                                unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq,
                                                CString csFicheroLog)    
{
    if (!csFicheroLog.IsEmpty())
    {
        FILE* log=fopen(csFicheroLog,"wt");
        if (log == NULL) return;

        int nTotal       = 0; // SIN contar MUY_OSCUROS
        int nTotalInfDer = 0;
        int nTotalSupDer = 0;
        int nTotalInfIzq = 0;
        int nTotalSupIzq = 0;
        int nIndice = 0;

        fprintf(log," Informacion de clasificacion de una imagen multiespectral\n");
        fprintf(log," Umbral distancia: %lf Umbral confiabilidad: %lf\n\n",dUmbralDistancia,dUmbralConfiabilidad);
        fprintf(log," NUMERO TOTAL DE PIXELES %d PIXELES MUY OSCUROS (NO CLASIFICABLES): %d\n\n",m_nAnchoImagen*m_nAltoImagen,nCountOscuros);
        fprintf(log," Mineral\t\tTotal\tBien\tD_alta\tC_baja\tMal\n");

        list<CMineralClasificado>::iterator it;
        for (it=listClasificados.begin();it!=listClasificados.end();it++)
        {
            nIndice = (*it).m_nIndex;
            if (nIndice!=-1)
            {
                CString csNombre = m_pMinerales->GetNombre(nIndice);
                csNombre.Replace(' ','_');
                fprintf(log,"%s\t%d\t%d\t%d\t%d\t%d\n",  csNombre,
                                                                (*it).m_nCount,
                                                                arrInfDer[nIndice],
                                                                arrSupDer[nIndice],
                                                                arrInfIzq[nIndice],
                                                                arrSupIzq[nIndice] );
                nTotal += (*it).m_nCount;
                nTotalInfDer += arrInfDer[nIndice];
                nTotalSupDer += arrSupDer[nIndice];
                nTotalInfIzq += arrInfIzq[nIndice];
                nTotalSupIzq += arrSupIzq[nIndice];
            }
        }
        fprintf(log,"%s\t%d\t%d\t%d\t%d\t%d\n",  "TOTAL",
                                                        nTotal,
                                                        nTotalInfDer,
                                                        nTotalSupDer,
                                                        nTotalInfIzq,
                                                        nTotalSupIzq );
        fclose(log);
    }

}

// Guardar un fichero de texto con la información de clasificacion selectiva 
// VERSION SIN listClasificados, SIN ORDENAR
void Clasificacion::ClasificacionSelectivaLog(double dUmbralConfiabilidad,double dUmbralDistancia,
                                                unsigned int nCountOscuros,
                                                unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq,
                                                CString csFicheroLog)    
{
    if (!csFicheroLog.IsEmpty())
    {
        FILE* log=fopen(csFicheroLog,"wt");
        if (log == NULL) return;

        int nTotal       = 0; // SIN contar MUY_OSCUROS
        int nTotalInfDer = 0;
        int nTotalSupDer = 0;
        int nTotalInfIzq = 0;
        int nTotalSupIzq = 0;
        int nIndice      = 0;
        int nTodos       = 0;

        fprintf(log," Informacion de clasificacion de una imagen multiespectral\n");
        fprintf(log," Umbral distancia: %.2lf Umbral confiabilidad: %.2lf\n\n",dUmbralDistancia,dUmbralConfiabilidad);
        fprintf(log," Dimensiones de las imagenes: %dx%d\n",m_nAnchoImagen,m_nAltoImagen);
        fprintf(log," NUMERO TOTAL DE PIXELES MUY OSCUROS (NO CLASIFICABLES): %d\n\n",nCountOscuros);
        fprintf(log," Mineral             \tTotal\tBien\tD_alta\tC_baja\tMal\n");

        for (nIndice=0; nIndice<m_pMinerales->GetCount();nIndice++)
        {
            nTodos = 0;
            if (arrInfDer != NULL)
                nTodos += arrInfDer[nIndice];
            if (arrSupDer != NULL)
                nTodos += arrSupDer[nIndice];
            if (arrInfIzq != NULL)
                nTodos += arrInfIzq[nIndice];
            if (arrSupIzq != NULL)
                nTodos += arrSupIzq[nIndice];
            if (nTodos > 0)
            {
                CString csNombre = m_pMinerales->GetNombre(nIndice);
                csNombre.Replace(' ','_');
                fprintf(log,"%-20s\t%d\t",  csNombre, nTodos);
                if (arrInfDer != NULL)
                {
                    fprintf(log,"%d\t",                      arrInfDer[nIndice]);
                    nTotalInfDer += arrInfDer[nIndice];
                }
                else
                    fprintf(log,"%d\t",                      0);
                if (arrSupDer != NULL)
                {
                    fprintf(log,"%d\t",                      arrSupDer[nIndice]);
                    nTotalSupDer += arrSupDer[nIndice];
                }
                else
                    fprintf(log,"%d\t",                      0);
                if (arrInfIzq != NULL)
                {
                    fprintf(log,"%d\t",                      arrInfIzq[nIndice]);
                    nTotalInfIzq += arrInfIzq[nIndice];
                }
                else
                    fprintf(log,"%d\t",                      0);
                if (arrSupIzq != NULL)
                {
                    fprintf(log,"%d\t",                      arrSupIzq[nIndice]);
                    nTotalSupIzq += arrSupIzq[nIndice];
                }
                else
                    fprintf(log,"%d\t",                      0);
                fprintf(log,"\n");

                nTotal += nTodos;
            }
        }
        fprintf(log,"%-20s\t%d\t%d\t%d\t%d\t%d\n",  "TOTAL",
                                                        nTotal,
                                                        nTotalInfDer,
                                                        nTotalSupDer,
                                                        nTotalInfIzq,
                                                        nTotalSupIzq );
        fclose(log);
    }

}

// guarda la imagen de minerales clasificados en el fichero dado
void Clasificacion::GuardarClasificar(CString csNombreFichero)
{
    if (M_pClasificacion != NULL && *M_pClasificacion != M_NULL)
        MbufExport((char*)(LPCTSTR)csNombreFichero,M_TIFF,*M_pClasificacion);
}

