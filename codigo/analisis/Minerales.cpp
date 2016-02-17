// Minerales.cpp: implementation of the CMinerales class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Minerales.h"
#include "..\librerias\ProcesoAux\myVars.h" //LoadVars
#include "..\librerias\ProcesoAux\myUtilIO.h" //BuscaString

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// PARAMETROS APRENDIZAJE
#define RAND_SEED 2 // semilla inicial de numeros aleatorios

// PARAMETROS PRUEBAS
static const int NUM_METODOS = 1; //AÑADIR METODO AQUI!!!!!!!!
static const int DETALLADO = 0; // Si 1 resultados detallados (con valores del metodo para varios minerales)
    static const int NUM_RESULTADOS_PRUEBAS = 5;//400000; 
static const bool bGenerarDatosPruebas = false; //indica si generamos datos sinteticos a partir de la moda y los percentiles a la hora de hacer las pruebas
    static const int DATOS_GENERADOS_POR_ZONA_PRUEBAS = 10; // en vez de usar la moda de la zona, generamos sinteticamente datos para probar
    static const bool ALEATORIZAR_BANDAS_INDEPENDIENTEMENTE_PRUEBAS = 0; //0 // Al generar valores sinteticos, se generan valores aleatorios por cada una de las bandas (si 0, todas las bandas se mueven a la vez) 

using namespace std;

extern FILE* g_log;

enum_orden_mineral CMineral::m_enumOrden = ORDEN_ABREVIATURA; //declaracion porque m_enumOrden es static

// funcion que compara dos punteros a CMineral. Usado como predicado en std::sort
bool ComparadorMinerales(CMineral* mineral1,CMineral* mineral2)
{
    if (CMineral::m_enumOrden == ORDEN_REFLECTANCIA)
        return (mineral1->m_reflectanciaMedia < mineral2->m_reflectanciaMedia) ? true : false;
    else if (CMineral::m_enumOrden == ORDEN_ABREVIATURA)
        return (mineral1->m_csAbreviatura < mineral2->m_csAbreviatura) ? true : false;
    else
        return (mineral1->m_nId < mineral2->m_nId) ? true : false;
}

//////////////////////////////////////////////////////////////////////
//
// CMinerales - Contenedor de minerales e interfaz
//
//////////////////////////////////////////////////////////////////////
CMinerales::CMinerales()
{
    srand(RAND_SEED); //inicializamos con un valor para asegurarnos el determinismo

    m_nBandas = -1;
    memset(m_lut_clasificacion,255,256*3); //inicializamos a blanco para que se vea bien en caso de que erroneamente se mostrase en pantalla algun no-mineral
    m_bAlgunMineralClasificable = false;
}

CMinerales::~CMinerales()
{
    vector<CMineral*>::iterator it;
    for (it=m_list.begin(); it!=m_list.end();it++)
        if ((*it) != NULL)
            delete (*it);
}

// Cargar lista de minerales de fichero
bool CMinerales::CargarMinerales(CString csFichero)
{
    int i;

    if (csFichero == "")
        csFichero = FICHERO_DEFECTO_MINERALES;
    // lee todas las variables del fichero y las guarda en variables globales
	if ( LoadVars(csFichero) == NULL) {
		return false;
	}

    m_list.clear();
    for (i=0; i <GetNumSections(); i++)
    {
        int nDummy1, nDummy2;
        char* pAbreviatura;
        pAbreviatura = FindSection(i,nDummy1,nDummy2);
        if (pAbreviatura != NULL)
        {
            CMineral* pMineral = new CMineral;

            pMineral->m_csAbreviatura = pAbreviatura;
            pMineral->m_csAbreviatura.MakeLower();

            int activo;
	        LOADINTSECTION(activo,i);
            pMineral->m_bActivo = activo==1;
            ASSERT(activo == 0 || activo == 1);

            int identificador;
	        LOADINTSECTION(identificador,i);
            pMineral->m_nId = identificador;
            ASSERT(identificador >= 0 && identificador <= 256);

            char* nombre;
            nombre = pMineral->m_csNombre.GetBuffer(100);
	        LOADSTRSECTION(nombre,i);
            pMineral->m_csNombre.ReleaseBuffer(-1);

            int indice;
	        LOADINTSECTION(indice,i);
            pMineral->m_nIndiceColor = indice;
            ASSERT(indice >= 0 && indice <= 256);

            // color a la hora de clasificar
            int* RGB;
            RGB = pMineral->m_RGB;
	        LOADVINTSECTION(RGB,3,i);
            // Establece los valores RGB del indice de este mineral para mostrar en pantalla
		    m_lut_clasificacion[indice][0] = (BYTE) (RGB[2]);
            m_lut_clasificacion[indice][1] = (BYTE) (RGB[1]);
            m_lut_clasificacion[indice][2] = (BYTE) (RGB[0]);

            m_list.push_back(pMineral);
        }
    }

    //reservar color negro para ganga
	m_lut_clasificacion[0][0] = (BYTE) (0);
    m_lut_clasificacion[0][1] = (BYTE) (0);
    m_lut_clasificacion[0][2] = (BYTE) (0);
//reservar colores especiales (no clasificados)
m_lut_clasificacion[PIXEL_NO_CUMPLE_DISTANCIA_CONFIABILIDAD][0] = (BYTE) (255);
m_lut_clasificacion[PIXEL_NO_CUMPLE_DISTANCIA_CONFIABILIDAD][1] = (BYTE) (0);
m_lut_clasificacion[PIXEL_NO_CUMPLE_DISTANCIA_CONFIABILIDAD][2] = (BYTE) (255);

m_lut_clasificacion[PIXEL_NO_CUMPLE_DISTANCIA][0]               = (BYTE) (255);
m_lut_clasificacion[PIXEL_NO_CUMPLE_DISTANCIA][1]               = (BYTE) (0);
m_lut_clasificacion[PIXEL_NO_CUMPLE_DISTANCIA][2]               = (BYTE) (0);

m_lut_clasificacion[PIXEL_NO_CUMPLE_CONFIABILIDAD][0]           = (BYTE) (0);
m_lut_clasificacion[PIXEL_NO_CUMPLE_CONFIABILIDAD][1]           = (BYTE) (0);
m_lut_clasificacion[PIXEL_NO_CUMPLE_CONFIABILIDAD][2]           = (BYTE) (255);

m_lut_clasificacion[PIXEL_CUMPLE][0]                            = (BYTE) (0);
m_lut_clasificacion[PIXEL_CUMPLE][1]                            = (BYTE) (255);
m_lut_clasificacion[PIXEL_CUMPLE][2]                            = (BYTE) (0);

    return true;
}
// Guarda lista de minerales en fichero
bool CMinerales::GuardarMinerales(CString csFichero)
{
    vector<CMineral*>::iterator it;

	FILE*   archivo;
	if ((archivo = fopen(csFichero, "wt")) == NULL)
		return false;

    fprintf(archivo,"# Archivo de descripción de minerales. Usar ""GestionMinerales"" para su edición.\n\n");
    for (it=m_list.begin(); it!=m_list.end();it++)
    {
        if ((*it) != NULL)
        {
            fprintf(archivo,"[%s]\n", (*it)->m_csAbreviatura);
            fprintf(archivo,"activo = %d\n", (BOOL)(*it)->m_bActivo);
            fprintf(archivo,"identificador = %d\n", (*it)->m_nId);
            fprintf(archivo,"nombre = %s\n", (*it)->m_csNombre);
            fprintf(archivo,"indice = %d\n", (*it)->m_nIndiceColor);
            fprintf(archivo,"RGB = %d %d %d\n", (*it)->m_RGB[0],(*it)->m_RGB[1],(*it)->m_RGB[2]);
            fprintf(archivo,"\n");
        }
    }

    return true;
}

// Busqueda dicotomica suponiendo que los minerales estan ordenados por abreviatura
CMineral* CMinerales::BuscarPorAbreviatura(CString csAbreviatura)
{
    int nIndiceMin = 0;
    int nIndiceMax = GetCount()-1;
    int nIndice = -1;

    ASSERT(nIndiceMax > 0);

    // Comprobamos que el orden sea por abreviatura
    if (CMineral::m_enumOrden != ORDEN_ABREVIATURA)
        return NULL;

    do
    {
        nIndice = floor((nIndiceMin + nIndiceMax) / 2);

        int res_comp = _stricmp(m_list[nIndice]->m_csAbreviatura,csAbreviatura);
        if (res_comp > 0)
        {
            nIndiceMax = max(nIndice - 1, nIndiceMin); // max necesario para cuando quedan dos posibles valores contiguos y el primero es mayor que la compraracion
        }
        else if (res_comp <0)
        {
            nIndiceMin = min(nIndice + 1,nIndiceMax); // min necesario para cuando quedan dos posibles valores contiguos y el segundo es menor que la compraracion
        }
        else 
            // ENCONTRADO!!!!
            return m_list[nIndice];
    } while (nIndiceMin != nIndiceMax);

    // puede ser que sea justo nIndiceMin == nIndiceMax
    if (m_list[nIndiceMin]->m_csAbreviatura == csAbreviatura)
        // ENCONTRADO!!!!
        return m_list[nIndiceMin];
    else
        // No encontrado
        return NULL;
}

// Cargar lista de zonas y pixels de fichero
int CMinerales::CargarPixels(CString csFichero, bool bPruebas, FILE* log)
{
    int eof;
    char* pString;
    ULONG pos;
    ULONG ulNumLineasDummy = 0;
    int nNumBandas; //devolveremos este valor

	FILE*   archivo;
	if ((archivo = fopen(csFichero, "rt")) == NULL)
    {
        fprintf(log,"No se puede abrir el fichero de pixels %s\n",csFichero);
		return -1;
    }

    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(archivo,"num_areas =",ulNumLineasDummy);
    int num_areas;
    if (pos!=-1)
    {
        pString = ReadLine(archivo,eof);
        num_areas = atoi(pString);
    }
    else
    {
        fprintf(log,"Error al buscar num_areas en el fichero %s\n",csFichero);
        return -1;
    }

    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(archivo,"num_bandas =",ulNumLineasDummy);
    if (pos!=-1)
    {
        pString = ReadLine(archivo,eof);
        nNumBandas = atoi(pString);
    }
    else
    {
        fprintf(log,"Error al buscar num_bandas en el fichero %s\n",csFichero);
        return -1;
    }

    fprintf(log,"Leemos fichero de pixels %s con %d areas y %d bandas\n",csFichero, num_areas, nNumBandas);

    //Asegurarnos que los minerales esten ordenados por abreviatura (para que funcione luego BuscarPorAbreviatura()
    CMineral::m_enumOrden = ORDEN_ABREVIATURA; // m_enumOrden es static
    std::sort(m_list.begin(),m_list.end(),ComparadorMinerales);

    for (int i=0; i <num_areas; i++)
    {
        // Procedencia
        pos = BuscaString(archivo,"Procedencia = ",ulNumLineasDummy);
        CString csProcedencia;
        if (pos!=-1)
            csProcedencia = LTrim(RTrim(ReadLine(archivo,eof)));

        // Abreviatura
        pos = BuscaString(archivo," = ",ulNumLineasDummy);
        CString csAbreviatura;
        if (pos!=-1)
        {
            csAbreviatura = LTrim(RTrim(ReadLine(archivo,eof)));
            csAbreviatura.MakeLower();
        }

         // Calidad
        pos = BuscaString(archivo," = ",ulNumLineasDummy);
        CString csCalidad;
        if (pos!=-1)
            csCalidad = LTrim(RTrim(ReadLine(archivo,eof)));

        // Encontrar el mineral en la lista de minerales
        CMineral* pMineral = BuscarPorAbreviatura(csAbreviatura);

        if (pMineral!=NULL && csCalidad != CALIDAD_E) //No consideramos zonas con baja calidad
        {
            CMineralZona* pMineral_zona = new CMineralZona;

            pMineral_zona->m_nZona = i+1;

            pMineral_zona->m_csProcedencia = csProcedencia;
            pMineral_zona->m_csCalidad = csCalidad;

            // Comentario
            pos = BuscaString(archivo," = ",ulNumLineasDummy);
            if (pos!=-1)
                pMineral_zona->m_csComentario = LTrim(RTrim(ReadLine(archivo,eof)));

            // num pixels
            pos = BuscaString(archivo," = ",ulNumLineasDummy);
            int num_pixels;
            if (pos!=-1)
            {
                pString = ReadLine(archivo,eof);
                num_pixels = atoi(pString);
            }
            pMineral_zona->m_nNumPixels = num_pixels;
            pMineral_zona->m_nNumPixelsNoExcluidos = num_pixels;

            pMineral_zona->m_arrPixels = new double [num_pixels][NUM_BANDAS];
            memset(pMineral_zona->m_arrPixels,0,sizeof(double) * num_pixels * NUM_BANDAS);
            double valor;
            for (int j=0; j < num_pixels; j++)
            {
                for (int b=0; b<nNumBandas; b++)
                {
                    fscanf(archivo,"%lf",&valor);
                    pMineral_zona->m_arrPixels[j][b] = valor;
                }
            }

            if ( !bPruebas)
                // asignar a lista valida (luego se separaran los de prueba)
                pMineral->m_list_zonas.push_back(pMineral_zona);
            else 
                // asignar a lista pruebas
                pMineral->m_list_zonas_test.push_back(pMineral_zona);
            pMineral->m_nTotalPixels += pMineral_zona->m_nNumPixels;
        }
    }

    fprintf(log,"Leidas %d areas fichero de pixels %s \n",i,csFichero);

    fclose(archivo);

    // Ordenamos por reflectancia media (de todas las bandas) para usar arrays de busqueda (para ahorrar tiempo en la clasificacion)
    CMineral::m_enumOrden = ORDEN_REFLECTANCIA; //m_enumOrden es static
    std::sort(m_list.begin(),m_list.end(),ComparadorMinerales);

    return nNumBandas;
}

// - nBandasConfiguradas: numero de bandas en fichero de configuracion, para comparar y comprobar con las bandas del fichero de entrenamiento
//                        si -1, se obviará la comprobación
bool CMinerales::CargarEntrenamiento(CString csFicheroEntrenamiento, int nBandasConfiguradas)
{
    int i,j;
    int nNumBandas;
    FILE* fMahalanobis;
// HACER: esto del fichero "f" es un truco para eliminar un problema raro que da un problema en RELEASE
//FILE* f;
//f=fopen("log.txt","wt");

    //El orden del fichero de entrenamiento es en orden de identificador
    CMineral::m_enumOrden = ORDEN_IDENTIFICADOR; // m_enumOrden es static
    std::sort(m_list.begin(),m_list.end(),ComparadorMinerales);

    if (csFicheroEntrenamiento == "")
        csFicheroEntrenamiento = FICHERO_DEFECTO_MAHALANOBIS;

    fMahalanobis=fopen(csFicheroEntrenamiento,"rt");

    if (fMahalanobis == NULL)
    {
        AfxMessageBox("No se puede abrir el fichero de entrenamiento " + csFicheroEntrenamiento);
        return false;
    }

    fscanf(fMahalanobis,"%d\n",&nNumBandas);
    if (nNumBandas<=0 || nNumBandas > NUM_BANDAS)
    {
        CString csError;
        csError.Format("Error al cargar entrenamiento. Valor incorrecto de numero de bandas: %d",nNumBandas);
        AfxMessageBox(csError);
        ASSERT(FALSE);
        return false;
    }
    else if (nBandasConfiguradas!=-1 && nNumBandas<nBandasConfiguradas)
    {
        CString csError;
        csError.Format("Advertencia: No hay informacion de entrenamiento (%d bandas) para todas las bandas configuradasse (%d)",nNumBandas,nBandasConfiguradas);
        AfxMessageBox(csError);
        ASSERT(FALSE);
    }
    else if (nBandasConfiguradas!=-1 && nNumBandas>nBandasConfiguradas)
    {
        CString csError;
        csError.Format("Error al cargar entrenamiento. Hay mas bandas de entrenamiento (%d) que las configuradas (%d)",nNumBandas,nBandasConfiguradas);
        AfxMessageBox(csError);
        ASSERT(FALSE);
        return false;
    }
    m_nBandas = nNumBandas;

/*
    for(i=0;i<NUM_INDICES_BUSQUEDA;++i)
    {
        if (fscanf(fMahalanobis,"%d\t%d\n",&m_arrInicioBusqueda[i],&m_arrFinBusqueda[i]) != 2 
            || m_arrInicioBusqueda[i] >= m_arrFinBusqueda[i] 
            || m_arrInicioBusqueda[i]<0 
            || m_arrFinBusqueda[i]<0 
            || m_arrInicioBusqueda[i]>=m_list.size() 
            || m_arrFinBusqueda[i]>=m_list.size() )
        {
            CString csError;
            csError.Format("Error al cargar entrenamiento. Formato incorrecto de lista de indices");
            AfxMessageBox(csError);
            ASSERT(FALSE);
            return false;
        }
    }
*/
    vector<CMineral*>::iterator it;
    for (it=m_list.begin(); it!=m_list.end();++it)
    {
        char* szAbreviatura[32];
        fscanf(fMahalanobis,"%s\n",szAbreviatura);
        int nHayZonas;
        fscanf(fMahalanobis,"%d\n",&nHayZonas);
//CString mes0;
//mes0.Format("mineral %s %d     .... numbandas:%d",(*it)->m_csNombre,nHayZonas,nNumBandas);
//AfxMessageBox(mes0);
//fprintf(f,"%s\n",mes0);
        if (nHayZonas == 1)
        {
            //reflectancias espectrales
            for(i=0;i<nNumBandas;++i)
            {
                if(fscanf(fMahalanobis,"%lf\t",&((*it)->m_arrReflectancias[i]))!=1 
                    || (*it)->m_arrReflectancias[i]<0 || (*it)->m_arrReflectancias[i]>255)
                {
                    CString csError;
                    csError.Format("Error al cargar entrenamiento. Formato incorrecto de reflectancias en mineral %s y banda %d",(*it)->m_csNombre,i);
                    AfxMessageBox(csError);
                    ASSERT(FALSE);
                    return false;
                }
//fprintf(f,"%lf\t",(*it)->m_arrReflectancias[i]);
            }
//fprintf(f,"\n");

            //refletancia media para usar indices
            if(fscanf(fMahalanobis,"%lf\t%lf\t%lf\n",&((*it)->m_reflectanciaMedia),&((*it)->m_reflectanciaMediaMin),&((*it)->m_reflectanciaMediaMax))!=3 
                || (*it)->m_reflectanciaMedia<0 || (*it)->m_reflectanciaMedia>255
                || (*it)->m_reflectanciaMediaMin<0 || (*it)->m_reflectanciaMediaMin>255
                || (*it)->m_reflectanciaMediaMax<0 || (*it)->m_reflectanciaMediaMax>255)
            {
                ASSERT(FALSE);
                CString csError;
                csError.Format("Error al cargar entrenamiento. Formato incorrecto de reflectancias medias en mineral %s",(*it)->m_csNombre);
                AfxMessageBox(csError);
                return false;
            }
//fprintf(f,"%lf\t%lf\t%lf\n",(*it)->m_reflectanciaMedia,(*it)->m_reflectanciaMediaMin,(*it)->m_reflectanciaMediaMax);

            // matriz invertida de mahalanobis
            for(i=0;i<nNumBandas;++i)
            {
                for(j=0;j<nNumBandas;++j)
                {
                    if (fscanf(fMahalanobis,"%lf\t",&((*it)->m_matrizCovarianzaInv[i][j])) != 1)
                    {
                        CString csError;
                        csError.Format("Error al cargar entrenamiento. Faltan datos leyendo de la matriz leyendo %s, linea %d, columna %d",(*it)->m_csNombre,i,j);
                        AfxMessageBox(csError);
                        ASSERT(FALSE);
                        return false;
                    }
//fprintf(f,"%lf\t",(*it)->m_matrizCovarianzaInv[i][j]);
                }
//fprintf(f,"\n");
            }
        }
        else if (nHayZonas != 0)
        {
            CString csError;
            csError.Format("Error al cargar entrenamiento. Formato incorrecto de hay_zonas, se ha leido: (%d)",nHayZonas);
            AfxMessageBox(csError);
            ASSERT(FALSE);
            return false;
        }
    }
    fclose(fMahalanobis);

    // Ordenamos por reflectancia media (de todas las bandas) para usar arrays de busqueda (para ahorrar tiempo en la clasificacion)
    CMineral::m_enumOrden = ORDEN_REFLECTANCIA; //m_enumOrden es static
    std::sort(m_list.begin(),m_list.end(),ComparadorMinerales);
//fclose(f);
    return true;
}

// calcula las tablas de indices de busqueda para optimizar la velocidad de clasificacion
void CMinerales::CalcularIndicesBusqueda()
{
    int i;
    vector<CMineral*>::iterator it;
    int numMinerales = GetCount();

    memset(m_arrInicioBusqueda,CHAR_MAX,sizeof(int)*NUM_INDICES_BUSQUEDA);
    memset(m_arrFinBusqueda,-1,sizeof(int)*NUM_INDICES_BUSQUEDA);

    // Ordenamos por reflectancia media (de todas las bandas) para rellenar arrays de busqueda (para ahorrar tiempo en la clasificacion)
    CMineral::m_enumOrden = ORDEN_REFLECTANCIA; //m_enumOrden es static
    std::sort(m_list.begin(),m_list.end(),ComparadorMinerales);
    int nCount = 0;
    for (it=m_list.begin(); it!=m_list.end();it++,nCount++)
    {
        if ((*it)->m_reflectanciaMedia != -1) //solo si hay datos
        {
            int nIndiceArrayMin = (*it)->m_reflectanciaMediaMin - HOLGURA_INDICES_BUSQUEDA; //restamos la holgura
            if (nIndiceArrayMin <0)
                nIndiceArrayMin = 0;
            nIndiceArrayMin = (int)floor(nIndiceArrayMin * ((double)NUM_INDICES_BUSQUEDA/100.0)); //convertimos de valor de reflectancia a indice de busqueda
            int nIndiceArrayMax = (*it)->m_reflectanciaMediaMax + HOLGURA_INDICES_BUSQUEDA; //sumamos la holgura
            if (nIndiceArrayMax >= NUM_INDICES_BUSQUEDA)
                nIndiceArrayMax = NUM_INDICES_BUSQUEDA-1;
            nIndiceArrayMax = (int)ceil(nIndiceArrayMax * ((double)NUM_INDICES_BUSQUEDA/100.0)); //convertimos de valor de gris a indice de busqueda
            for (i=nIndiceArrayMin;i<nIndiceArrayMax;i++)
            {
                m_arrInicioBusqueda[i] = min(m_arrInicioBusqueda[i], nCount);
                m_arrFinBusqueda[i] = max(m_arrFinBusqueda[i], nCount);
            }
        }
    }
    // No permitimos entradas de los arrays sin rellenar, asi que repasamos la lista y propagamos los valores adyacentes
    int nValorMin = 0;
    for (i=0;i<NUM_INDICES_BUSQUEDA;i++)
    {
        if (m_arrInicioBusqueda[i] > numMinerales-1) // no hay valor
        {
            m_arrInicioBusqueda[i] = nValorMin;
        }
        else
            nValorMin = m_arrInicioBusqueda[i];
    }
    int nValorMax = numMinerales-1;
    for (i=NUM_INDICES_BUSQUEDA-1;i>=0;i--)
    {
        if (m_arrFinBusqueda[i] < 0) // no hay valor
        {
            m_arrFinBusqueda[i] = nValorMax;
        }
        else
            nValorMax = m_arrFinBusqueda[i];
    }
    // Hacemos una pasada forzando las entradas de los arrays comprendan al menos MIN_MINERALES_BUSQUEDA minerales
    // Si no, buscamos el mineral mas cercano (principio o fin) y ampliamos el rango (en lo que sea)
    // De tal manera que una iteracion nos asegura una diferencia de 2 o mas, dos iteraciones una diferencia de 3 o mas, etc
    bool bTerminado = MIN_MINERALES_BUSQUEDA <= 1;
    int nIteraciones = 0;
    bool bEncontrado = false; //para parar midiendo en cada entrada de mineral cuando un mineral que amplie rango se ha encontrado
    int arrInicioAux[NUM_INDICES_BUSQUEDA]; //para almacenar los nuevos valores sin afectar a las demas entradas
    int arrFinAux[NUM_INDICES_BUSQUEDA];
    while (bTerminado == false)
    {
        bTerminado = true; // por defecto terminado, en cuanto haya una entrada que no cumpla, lo pondremos a no terminado
        for (i=0;i<NUM_INDICES_BUSQUEDA;i++)
        {
            arrInicioAux[i] = m_arrInicioBusqueda[i]; //inicializacion
            arrFinAux[i] = m_arrFinBusqueda[i];       //inicializacion
            if (m_arrFinBusqueda[i] - m_arrInicioBusqueda[i] + 1 < MIN_MINERALES_BUSQUEDA)
            {
                bTerminado = false; // hay que iterar otra vez
                bEncontrado = false;
                int j=1; //distancia a entrada actual (i)
                while(!bEncontrado || (i-j < 0 && i+j >= NUM_INDICES_BUSQUEDA))
                {
                    if (i-j >= 0)
                    {
                        if (m_arrInicioBusqueda[i-j] < m_arrInicioBusqueda[i])
                        {
                            bEncontrado = true;
                            arrInicioAux[i] = min(arrInicioAux[i],m_arrInicioBusqueda[i-j]); //min por si por el otro lado tambien se encuentra un valor que amplie rango
                        }
                        if (m_arrFinBusqueda[i-j] > m_arrFinBusqueda[i])
                        {
                            bEncontrado = true;
                            arrFinAux[i] = max(arrFinAux[i],m_arrFinBusqueda[i-j]); //min por si por el otro lado tambien se encuentra un valor que amplie rango
                        }
                    }
                    if (i+j < NUM_INDICES_BUSQUEDA)
                    {
                        if (m_arrInicioBusqueda[i+j] < m_arrInicioBusqueda[i])
                        {
                            bEncontrado = true;
                            arrInicioAux[i] = min(arrInicioAux[i],m_arrInicioBusqueda[i+j]); //min por si por el otro lado tambien se encuentra un valor que amplie rango
                        }
                        if (m_arrFinBusqueda[i+j] > m_arrFinBusqueda[i])
                        {
                            bEncontrado = true;
                            arrFinAux[i] = max(arrFinAux[i],m_arrFinBusqueda[i+j]); //min por si por el otro lado tambien se encuentra un valor que amplie rango
                        }
                    }
                    ++j;
                }
                if (!bEncontrado)
                    //ERROR: para la entrada i no se ha encontrado ningun rango de minerales que cumpla MIN_MINERALES_BUSQUEDA!!
                    AfxMessageBox("ERROR: para la entrada i no se ha encontrado ningun rango de minerales que cumpla MIN_MINERALES_BUSQUEDA!!");
            }
        }
        //copiamos los valores
        CopiaVector(m_arrInicioBusqueda,arrInicioAux,NUM_INDICES_BUSQUEDA);
        CopiaVector(m_arrFinBusqueda,arrFinAux,NUM_INDICES_BUSQUEDA);

        ++nIteraciones;
        bTerminado = bTerminado || (MIN_MINERALES_BUSQUEDA <= nIteraciones + 1); // si no ha habido ninguna entrada fallida o se asegura el rango por el numero de iteraciones
    }

    // Log
    fprintf(g_log, "INDICES DE BUSQUEDA\n");
    for(i=0; i<NUM_INDICES_BUSQUEDA;i++)
    {
        fprintf(g_log, "%d: %d %d\n", i, m_arrInicioBusqueda[i],m_arrFinBusqueda[i]);
    }
}

void CMinerales::ImprimirMinerales()
{
    FILE* f;
    int i = 1;
    f=fopen("log_minerales_orden.txt","wt");
    vector<CMineral*>::iterator it;
    for (it=m_list.begin(); it!=m_list.end();++it,i++)
    {
        fprintf(f,"%d: %s - %lf\n",i,(*it)->m_csAbreviatura,(*it)->m_reflectanciaMedia);
    }
    fclose(f);
}

int CMinerales::GetCount()
{
    return m_list.size();
}

// Obtiene el identificador de mineral a partir del indice 
// si nIndex es -1, devuelve el proximo indice disponible
int CMinerales::GetId(int nIndex)
{
    if (nIndex == -1)
    {
        //Devolver proximo disponible
        int nUltimoIndexValido = GetCount() -1;
        while(m_list[nUltimoIndexValido] == NULL || nUltimoIndexValido<0)
            nUltimoIndexValido--;
        if (nUltimoIndexValido < 0)
            return 1; //lista vacia
        else
            return m_list[nUltimoIndexValido]->m_nId + 1; //sumar 1 al id del ultimo mineral
    }
    else if (nIndex <0 || nIndex>m_list.size()-1)
        return -1;
    else
        return m_list[nIndex]->m_nId;
}

//Devuelve el indice de la paleta de colores de este mineral
int CMinerales::GetIndiceColor(int nIndex)
{
    if (nIndex <0 || nIndex>m_list.size()-1)
        return -1;
    else
        return m_list[nIndex]->m_nIndiceColor;
}

CString CMinerales::GetNombre(int nIndex)
{
    if (nIndex <0 || nIndex>m_list.size()-1)
        return "ERROR";
    else
        return m_list[nIndex]->m_csNombre;
}

CString CMinerales::GetAbreviatura(int nIndex)
{
    if (nIndex <0 || nIndex>m_list.size()-1)
        return "ERROR";
    else
        return m_list[nIndex]->m_csAbreviatura;
}

CString CMinerales::GetAbreviatura(CString csNombre)
{
    vector<CMineral*>::iterator it;

    for (it = m_list.begin(); it != m_list.end();it++)
    {
        if ((*it) != NULL && (*it)->m_csNombre == csNombre)
        {
            break;
        }
    }

    if (it == m_list.end())
        return SELECCIONE_MINERAL_ABR;
    else
        return (*it)->m_csAbreviatura;
}

int CMinerales::GetIndex(int nId)
{
    vector<CMineral*>::iterator it;

    int nIndex = 0;

    for (it = m_list.begin(); it != m_list.end();it++)
    {
        if ((*it) != NULL && (*it)->m_nId == nId)
        {
            break;
        }
        ++nIndex;
    }

    if (it == m_list.end())
        return -1;
    else
        return nIndex;
}

int CMinerales::GetIndexConIndiceColor(int nIndiceColor)
{
    vector<CMineral*>::iterator it;

    int nIndex = 0;

    for (it = m_list.begin(); it != m_list.end();it++)
    {
        if ((*it) != NULL && (*it)->m_nIndiceColor == nIndiceColor)
        {
            break;
        }
        ++nIndex;
    }

    if (it == m_list.end())
        return -1;
    else
        return nIndex;
}

// Verifica si el indice de color del mineral dado esta repetido y en tal caso devuelve el proximo mas cercano
// nIdPropio - si diferente de -1, se excluirá el mineral correspondiente de la comprobación
int CMinerales::VerificaIndiceColor(int nIndiceColorOriginal, int nIdPropio)
{
    int nIndiceColor = nIndiceColorOriginal;
    int nNumMin = GetCount();
    if (nIndiceColor <0 || nIndiceColor>=255)
        return -1;

    vector<CMineral*>::iterator it;
    bool bVerificado = false;
    int incremento = 1;

    while(!bVerificado)
    {
        for (it = m_list.begin(); it != m_list.end();it++)
        {
            if ((*it) != NULL && nIdPropio != (*it)->m_nId && (*it)->m_nIndiceColor == nIndiceColor)
            {
                nIndiceColor = nIndiceColorOriginal + incremento; //probar con el siguiente valor
                if (nIndiceColor < 0 || nIndiceColor >= 255)
                    return -1; //hemos llegado al un limite
                //preparamos el proximo incremento ... +1 -1 +2 -2 +3 -3 ...
                if (incremento < 0)
                    incremento = -incremento + 1;
                else
                    incremento = -incremento;
                break; // volver a iniciar el bucle
            }
        }
        if (it == m_list.end())
            bVerificado = true; //no hay ningun indice de color igual
    }

    return nIndiceColor;
}

#ifdef TIMEBENCH
extern FILE*   log_file;
#endif

// Calcula la probabilidad de que un perfil de reflectancias dado sea cada uno de los minerales
// Se devuelven los indices de todos los minerales ordenados de mayor a menor probabilidad
// No se devuelven en la lista los minerales que no tengan informacion de reflectancia
// OBSOLETO
/*
void CMinerales::Identificar(double* arrReflectancias, list<CResultadoIdentificacion>& list_resultados, int nMetodo ) 
{
    CResultadoIdentificacion primero, segundo;
    Identificar(arrReflectancias,primero,segundo);
    list_resultados.clear();
    list_resultados.push_back(primero);
    list_resultados.push_back(segundo);

//    Identificar(arrReflectancias,list_resultados);




    int nCurrentIndex = 0;
#ifdef TIMEBENCH
double dInicio = myTime();
double dInicio2 = myTime();
double dActual;
#endif
    list_resultados.clear();

    vector<CMineral*>::iterator it;
    for (it = m_list.begin(); it != m_list.end();it++)
    {
#ifdef TIMEBENCH
dActual = myTime();
fprintf(log_file,"%f  " ,dActual - dInicio);
fprintf(log_file,"    mineral\n");
dInicio = dActual;
#endif
        if ((*it)->m_arrReflectancias != NULL && (*it)->m_arrReflectancias[0] != 0)
        {
            CResultadoIdentificacion res;
            res.m_nIndice = nCurrentIndex;
            if (nMetodo == 1)
            {
                res.m_dProbabilidad = MahalanobisMatriz(arrReflectancias, (*it)->m_arrReflectancias,(*it)->m_matrizCovarianzaInv) ;
            }
            else if (nMetodo == 2)
                res.m_dProbabilidad = MinimumDistance(arrReflectancias, (*it)->m_arrReflectancias);
            else if (nMetodo == 3)
                res.m_dProbabilidad = SpectralAngleMapper(arrReflectancias, (*it)->m_arrReflectancias) * 100 / 90;
            else if (nMetodo == 4)
                res.m_dProbabilidad = Similarity(arrReflectancias, (*it)->m_arrReflectancias) ;
            else
                //DEFAULT
                res.m_dProbabilidad = MahalanobisMatriz(arrReflectancias, (*it)->m_arrReflectancias, (*it)->m_matrizCovarianzaInv) ;

            list_resultados.push_back(res);
        }

        nCurrentIndex++;
    }

#ifdef TIMEBENCH
dActual = myTime();
fprintf(log_file,"%f  " ,dActual - dInicio);
fprintf(log_file,"    mineral\n");
dInicio = dActual;
#endif
    // Ordenar de menor a mayor probabilidad
    list_resultados.sort();
#ifdef TIMEBENCH
fprintf(log_file,"%f  " ,myTime() - dInicio);
fprintf(log_file,"    sort\n");
#endif
}
*/

// Calcula la probabilidad de que un perfil de reflectancias dado sea cada uno de los minerales
// Se devuelven los indices de todos los minerales ordenados de mayor a menor probabilidad
// No se devuelven en la lista los minerales que no tengan informacion de reflectancia
void CMinerales::Identificar(double* arrReflectancias, list<CResultadoIdentificacion>& list_resultados) 
{
    int i, j;
    double dDistanciaCuadrado;

    double *vectorReflectancias;
    Matriz* matrizCovInv;
//    double (**matrizCovInv)[][NUM_BANDAS];
    double difVar[NUM_BANDAS];
    double acum;
    CResultadoIdentificacion res;
#ifdef TIMEBENCH
double dInicio = myTime();
double dInicio2 = myTime();
double dActual;
#endif
    list_resultados.clear();

//    vector<CMineral*>::iterator it;

    int nCurrentIndex;
    int nIndiceFin;
    if (USAR_INDICES_BUSQUEDA)
    {
        double nRefMedia = 0;
        for (i = m_nBandas - 1; i >= 0; i--) // comparar con 0 es mas eficiente, por eso se hace al reves
        {
            nRefMedia += arrReflectancias[i];
        }
        nRefMedia = nRefMedia / m_nBandas;

        int nIndice = (int)floor(nRefMedia * ((double)NUM_INDICES_BUSQUEDA/100) + 0.5);

        nCurrentIndex = m_arrInicioBusqueda[nIndice]; //convertimos de valor de gris a indice de busqueda
        nIndiceFin = m_arrFinBusqueda[nIndice]; //convertimos de valor de gris a indice de busqueda
        ASSERT(nIndiceFin >= nCurrentIndex); // si no, no hay ningun mineral con esta reflectancia media
    }
    else 
    {
        nCurrentIndex = 0;
        nIndiceFin = GetCount()-1;
    }
    for (; nCurrentIndex <= nIndiceFin; nCurrentIndex++)
    {
CString csNombre = m_list[nCurrentIndex]->m_csNombre;
        if (m_list[nCurrentIndex]->m_reflectanciaMedia != -1 /*si -1 es que no hay datos*/ && m_list[nCurrentIndex]->m_bActivo /*si false es que no se quiere entrenar*/ &&
            (m_list[nCurrentIndex]->m_bClasificable /*si false es que no se quiere clasificar*/ || !m_bAlgunMineralClasificable /* ningun mineral clasificable es que se usan todos*/)) 
        {
            dDistanciaCuadrado = 0;
            vectorReflectancias  = m_list[nCurrentIndex]->m_arrReflectancias;
            matrizCovInv = (Matriz*)(m_list[nCurrentIndex]->m_matrizCovarianzaInv);
#ifdef TIMEBENCH
dActual = myTime();
fprintf(log_file,"%f  " ,dActual - dInicio);
fprintf(log_file,"    mineral\n");
dInicio = dActual;
#endif
            // Calcular distancia Mahalanobis
            for (i = m_nBandas - 1; i >= 0; i--) // comparar con 0 es mas eficiente, por eso se hace al reves
                difVar[i] = arrReflectancias[i] - vectorReflectancias[i];//fabs

            // distancia = [difVar] * [matrizCovarianzaInv] * [difVar]
            for (i = m_nBandas - 1; i >= 0; i--) 
            {
                acum = 0;
                for (j = m_nBandas - 1; j >= 0; j--)
                    acum += difVar[j] * (*matrizCovInv)[j][i];
                dDistanciaCuadrado += acum * difVar[i];
            }
            res.m_dProbabilidad = dDistanciaCuadrado;
            res.m_nIndice = nCurrentIndex;

            list_resultados.push_back(res);
        }
    }
#ifdef TIMEBENCH
dActual = myTime();
fprintf(log_file,"%f  " ,dActual - dInicio);
fprintf(log_file,"    mineral\n");
dInicio = dActual;
#endif
    // Ordenar de menor a mayor probabilidad
    list_resultados.sort();
#ifdef TIMEBENCH
fprintf(log_file,"%f  " ,myTime() - dInicio);
fprintf(log_file,"    sort\n");
#endif
}

// Calcula la probabilidad de que un perfil de reflectancias dado sea cada uno de los minerales
// Se devuelven los indices de todos los minerales ordenados de mayor a menor probabilidad
// No se devuelven en la lista los minerales que no tengan informacion de reflectancia
// ATENCION. si USAR_INDICES_BUSQUEDA, la lista de minerales ha de estar ordenada por reflectancia
void CMinerales::Identificar(double* arrReflectancias, CResultadoIdentificacion& primero, CResultadoIdentificacion& segundo) 
{
    int i, j;
    double dDistanciaCuadrado;

    double *vectorReflectancias;
    Matriz* matrizCovInv;
//    double (**matrizCovInv)[][NUM_BANDAS];
    double difVar[NUM_BANDAS];
    double acum;

    int nIndicePrimero = -1, nIndiceSegundo = -1;
    double dDistanciaPrimero = 1e10;
    double dDistanciaSegundo = 1e10;

    int nCurrentIndex = 0;
    int nIndiceFin;
    if (USAR_INDICES_BUSQUEDA)
    {
        double dRefMedia = 0;
        for (i = m_nBandas - 1; i >= 0; i--) // comparar con 0 es mas eficiente, por eso se hace al reves
        {
            dRefMedia += arrReflectancias[i];
        }
        dRefMedia = dRefMedia / m_nBandas;

        int nIndice = (int)floor(dRefMedia * ((double)NUM_INDICES_BUSQUEDA/100) + 0.5);
        if (nIndice > 255)
        {
            ASSERT(FALSE);
            nIndice = 255; //codigo defensivo (no deberia ocurrir nunca)
        }
        nCurrentIndex = m_arrInicioBusqueda[nIndice]; //convertimos de valor de gris a indice de busqueda
        nIndiceFin = m_arrFinBusqueda[nIndice]; //convertimos de valor de gris a indice de busqueda
        ASSERT(nIndiceFin >= nCurrentIndex); // si no, no hay ningun mineral con esta reflectancia media
    }
    else 
    {
        nCurrentIndex = 0;
        nIndiceFin = GetCount()-1;
    }
#ifdef TIMEBENCH
double dInicio = myTime();
double dActual;
#endif

//fprintf(g_log,"de %d a %d\n",nCurrentIndex,nIndiceFin);
    for (; nCurrentIndex <= nIndiceFin; nCurrentIndex++)
    {
//fprintf(g_log,"\ncomparando con %s con ref %lf\n",m_list[nCurrentIndex]->m_csNombre,m_list[nCurrentIndex]->m_reflectanciaMedia);
        if (m_list[nCurrentIndex]->m_reflectanciaMedia != -1 /*si -1 es que no hay datos*/ && m_list[nCurrentIndex]->m_bActivo /*si false es que no se quiere entrenar*/ &&
            (m_list[nCurrentIndex]->m_bClasificable /*si false es que no se quiere clasificar*/ || !m_bAlgunMineralClasificable /* ningun mineral clasificable es que se usan todos*/)) 
        {
            dDistanciaCuadrado = 0;
            vectorReflectancias  = m_list[nCurrentIndex]->m_arrReflectancias;
            matrizCovInv = (Matriz*)(m_list[nCurrentIndex]->m_matrizCovarianzaInv);
            // Calcular distancia Mahalanobis
            for (i = m_nBandas - 1; i >= 0; i--) // comparar con 0 es mas eficiente, por eso se hace al reves
            {
                difVar[i] = arrReflectancias[i] - vectorReflectancias[i];//fabs
//fprintf(g_log," .. difVar %lf = %lf * %lf \n",difVar[i], arrReflectancias[i] , vectorReflectancias[i]);
            }

            // distancia = [difVar] * [matrizCovarianzaInv] * [difVar]
            for (i = m_nBandas - 1; i >= 0; i--) 
            {
                acum = 0;
                for (j = m_nBandas - 1; j >= 0; j--)
                    acum += difVar[j] * (*matrizCovInv)[j][i];
                dDistanciaCuadrado += acum * difVar[i];
//fprintf(g_log," .. acum %lf * difVar %lf = %lf .. \n",acum,difVar[i], dDistanciaCuadrado);
                if (dDistanciaCuadrado > dDistanciaSegundo)
                {
//fprintf(g_log,"la distancia %lf a este mineral ya es mayor que el primero %lf y segundo %lf \n",dDistanciaCuadrado,dDistanciaPrimero,dDistanciaSegundo);
                    goto siguiente_mineral; // la distancia a este mineral ya es mayor que el primero y segundo
                }
            }
//fprintf(g_log,"  distancia: %lf\n",dDistanciaCuadrado);

            if (dDistanciaCuadrado < dDistanciaPrimero)
            {
//fprintf(g_log,"  .. y lo ponemos como primero con indice %d\n",nCurrentIndex);
                dDistanciaSegundo = dDistanciaPrimero;
                dDistanciaPrimero = dDistanciaCuadrado;
                nIndiceSegundo = nIndicePrimero;
                nIndicePrimero = nCurrentIndex;
            }
            else if (dDistanciaCuadrado < dDistanciaSegundo)
            {
                dDistanciaSegundo = dDistanciaCuadrado;
                nIndiceSegundo = nCurrentIndex;
            }
    siguiente_mineral: //esta etiqueta sirve para ahorrarse las comparaciones anteriores
#ifdef TIMEBENCH
dActual = myTime();
fprintf(log_file,"%lf  " ,dActual - dInicio);
fprintf(log_file,"    mineral\n");
dInicio = dActual;
#endif
            continue;
        }
    }

    primero.m_dProbabilidad = dDistanciaPrimero;
    primero.m_nIndice       = nIndicePrimero;
    segundo.m_dProbabilidad = dDistanciaSegundo;
    segundo.m_nIndice       = nIndiceSegundo;
//fprintf(g_log,"\n finalmente el primer mineral es: %d\n",primero.m_nIndice);
}

//////////////////////////////////////////////////////////////////////
//
// Distancia Mahalanobis
//
//////////////////////////
// Mahalanobis - usando  matriz de covarianza (calculada en CalcularPromedioZonas)
// Se obvia hacer la raiz cuadrada por optimizacion (el resultado es el cuadrado de la distancia Mahalanobis)
// OBSOLETO
double CMinerales::MahalanobisMatriz(double* arrObjetivo, double* arrReferencia,  Matriz matrizCovarianzaInv)
{
//double dInicio = myTime();

    int i, j;
    double dDistancia = 0;

    double difVar[NUM_BANDAS];
    double acum;

    for (i = m_nBandas - 1; i >= 0; i--) // comparar con 0 es mas eficiente, por eso se hace al reves
        difVar[i] = arrObjetivo[i] - arrReferencia[i];//fabs
//fprintf(log_file,"%f  " ,myTime() - dInicio);
//fprintf(log_file,"        Resta&new\n");
//dInicio = myTime();

    // distancia = [difVar] * [matrizCovarianzaInv] * [difVar]
    for (i = m_nBandas - 1; i >= 0; i--) 
    {
        acum = 0;
        for (j = m_nBandas - 1; j >= 0; j--)
            acum += difVar[j] * matrizCovarianzaInv[j][i];
        dDistancia += acum * difVar[i];
    }

    //fprintf(log_file,"%f  " ,myTime() - dInicio);
//fprintf(log_file,"        MultVectorMatriz\n");
//dInicio = myTime();

//fprintf(log_file,"%f  " ,myTime() - dInicio);
//fprintf(log_file,"        MultVectorVector&delete\n");

    return dDistancia;
}

// SAM
// Metodo para calcular lo parecido que son dos "coordenadas multiespectrales" segun
// el angulo que forman sus vectores (en un espacio multiespectral)
// Se devuelve en grados (0..90). Si 0, mas parecidas son
double CMinerales::SpectralAngleMapper(double* arrObjetivo, double* arrReferencia)
{
    double dCos;

    double dProducto    = 0;
    double dObjCuadrado = 0;
    double dRefCuadrado = 0;
    for (int i = 0; i<m_nBandas;i++)
    {

        dProducto    += arrObjetivo[i] * arrReferencia[i];       
        dObjCuadrado += arrObjetivo[i] * arrObjetivo[i];
        dRefCuadrado += arrReferencia[i] * arrReferencia[i];

    }

    dCos = dProducto / sqrt(dObjCuadrado*dRefCuadrado);
    return acos(dCos) * 360 / (2*PI);
}

// MOMI
// Metodo para calcular la similaridad entre dos perfiles multiespectrales por
// minimos cuadrados de los ratios entre cada banda
// Se devuelve en valores de similaridad (0...). Si 0, mas parecidas son
double CMinerales::Similarity(double* arrObjetivo, double* arrReferencia)
{
    double dS = 0;

    double dQ    = 0;
    double dSumQ    = 0;
    double dSumQ2    = 0;
    for (int i = 0; i<m_nBandas;i++)
    {
        if (arrObjetivo[i] == 0) //reflectancia 0 poco probable, pero por si acaso
            dQ = 0; 
        else
            dQ = arrReferencia[i] / arrObjetivo[i];
        dSumQ    += dQ;       
        dSumQ2   += dQ * dQ;
    }

    dS = sqrt((dSumQ2-dSumQ*dSumQ/(m_nBandas)) / (m_nBandas - 1)) 
        / 
        (dSumQ/(m_nBandas));

    return dS*1000;
}

// Minimum Distance
// Metodo para calcular la diferencia entre dos perfiles multiespectrales por
// la distancia entre las coordenadas multiespectrales
// Se devuelve en distancia, cuanto mas pequeña mejor
double CMinerales::MinimumDistance(double* arrObjetivo, double* arrReferencia)
{
    double dDistancia = 0;

    double dAux    = 0;
    for (int i = 0; i<m_nBandas;i++)
    {
        dAux = arrReferencia[i] - arrObjetivo[i];
        dDistancia += dAux*dAux;
    }

    return sqrt(dDistancia);
}

//////////////////////////////
// Pruebas
/////////////////////
// Auxiliar a Pruebas para imprimir una linea de resultados de identificacion (sea una zona o un dato generado)
void CMinerales::ImprimirResultadoIdentificacion(FILE* archivo, 
                                                 CString csAbreviatura, 
                                                 int nCountZonas, //numero de zona del mineral
                                                 int nZona,       //identificador unico de zona (independiente de mineral)
                                                 int nCountDato,
                                                 bool bCorrecto, 
                                                 list<CResultadoIdentificacion>::iterator it_pos, 
                                                 list<CResultadoIdentificacion>::iterator it_end)
{
	fprintf(archivo, "%7d\t",nZona);
	fprintf(archivo, "%6s\t",csAbreviatura);
	fprintf(archivo, "%4d\t",nCountZonas);//,(*it).m_csProcedencia);
	fprintf(archivo, "%4d\t",nCountDato);
    // Calcular confiabilidad
    it_pos++; // segundo mineral
    double dConfiabilidad = (*it_pos).m_dProbabilidad;
    it_pos--; //primer mineral
    dConfiabilidad = dConfiabilidad / (*it_pos).m_dProbabilidad;
    if (bCorrecto)
	        fprintf(archivo, " bien\t");
    else
	        fprintf(archivo, "  MAL\t");

//	                    fprintf(archivo, "    metodo: %s\n",NombreMetodo[m-1]);

	fprintf(archivo, "%2.2lf\t",dConfiabilidad);//,GetNombre((*it_res).m_nIndice));
    for (int j=0; j<NUM_RESULTADOS_PRUEBAS && it_pos!=it_end;) 
    {
       // --it_res;
        if ((*it_pos).m_dProbabilidad != -1) // no mostrar si no hay valor de reflectancia
        {
	        fprintf(archivo, "%6s\t%2.2f\t",GetAbreviatura((*it_pos).m_nIndice),(*it_pos).m_dProbabilidad);
            j++;
        }
        it_pos++;
    }
	fprintf(archivo, "\n");
}


// ESTA FUNCION ESTA EN ENTRENAPIXELS.CPP Y SOLO ESTA AQUI POR LAS PRUEBAS CON ZONAS
// Funcion auxiliar visible solo a este modulo/fichero (static)
// genera un dato aleatoriamente segun una distribucion pseudo-normal
static double GenerarDistribucionNormal()
{
    const int NUM_RAND = 20;
    double dValor = 0;

    // la suma de distribuciones tipo "caja" es una distribucion pseudo-normal
    for (int i=0;i<NUM_RAND;i++)
    {
        dValor += (double)rand()/(double)RAND_MAX; // 0..1 -> 0..NUM_RAND
    }
    
    dValor = (dValor - NUM_RAND/2); // -NUM_RAND/2 .. NUM_RAND/2
    //normalizamos a N(0,1). 
    //sqrt(NUM_RAND/12) es la desviacion de la suma 
    //(si NUM_RAND = 12 la desviacion ya es 1 por construccion)
    dValor = dValor / sqrt((double)NUM_RAND/(double)12); //-3*NUM_RAND .. 3*NUM_REAND

    return dValor;
}

// ESTA FUNCION ESTA EN ENTRENAPIXELS.CPP Y SOLO ESTA AQUI POR LAS PRUEBAS CON ZONAS
// Funcion auxiliar visible solo a este modulo/fichero (static)
// Corrige un valor de una distribucion pseudo-normal (dValor) aplicando la media y desviaciones dadas
static double CorregirDistribucion(const double dMedia, const double dDesviacionInf, const double dDesviacionSup,const double dValor)
{
    double dRes;

    // corregimos con la desviacion deseada (teniando en cuenta si esta por encima o por debajo de la media)
    if (dValor>0)
        dRes = dValor * dDesviacionSup + dMedia;
    else
        dRes = dValor * dDesviacionInf + dMedia;

    return dRes;
}

void CMinerales::Pruebas()
{
    double arrReflectanciasGenerado[NUM_BANDAS];           // array auxiliar para almacenar reflectacias sinteticas

    int m = 0; //auxiliar bucles metodos
    int nTotalMinerales = 0; // minerales con alguna zona de prueba
    int nTotalZonas = 0; // zonas totales de prueba de todos los minerales
    CString NombreMetodo[NUM_METODOS];
    int MineralesAcertados[NUM_METODOS];
    int MineralesCasiAcertados[NUM_METODOS];
    int ZonasAcertadas[NUM_METODOS]; //reutilizado en cada mineral
    int ZonasAcertadasTotal[NUM_METODOS]; //total de minerales para imprimir resumen al final

    //AÑADIR METODO AQUI!!!!!!!!
    NombreMetodo[0] = "Mahalanobis";
//    NombreMetodo[1] = "D.Euclidea ";
//    NombreMetodo[2] = "  S.A.M.   ";
//    NombreMetodo[3] = "  M.O.M.I. ";

    memset(ZonasAcertadas,0,NUM_METODOS*sizeof(int));
    memset(ZonasAcertadasTotal,0,NUM_METODOS*sizeof(int));
    memset(MineralesAcertados,0,NUM_METODOS*sizeof(int));
    memset(MineralesCasiAcertados,0,NUM_METODOS*sizeof(int));

	FILE*   archivo;
	if ((archivo = fopen("resultados_pruebas.txt", "wt")) == NULL)
		return;
/* PRUEBAS TIMEBENCH
log_file = fopen("log_file.txt", "wt");
*/
    // Dejar espacio para resumen posterior
    int tam_caracteres_nombres_metodos = 0;
    for (m = 0;m<NUM_METODOS;m++)
        tam_caracteres_nombres_metodos += NombreMetodo[m].GetLength();
    fseek(archivo, 34 + 19 + tam_caracteres_nombres_metodos + (51+2)*NUM_METODOS + 6*2,SEEK_SET);


    vector<CMineralZona*>::iterator it;
    list<CResultadoIdentificacion> listIdentificacion;
    list<CResultadoIdentificacion>::iterator it_res;
    int nCountZonas = 0;

    // Cabecera de resumen de minerales
    if (DETALLADO)
    {
	    fprintf(archivo, "# RESULTADOS ZONAS DE PRUEBA\n\n");
	    fprintf(archivo, "Id_Zona\tAbrev.\tZona\tDato\tbien?\tConf.\t");
        for (int i= 0; i<NUM_RESULTADOS_PRUEBAS; i++)
	        fprintf(archivo, "Abrev.\tDist.\t");
    }
    else
    {
//        fprintf(archivo,"Simb.\n");
        for (m = 0;m<NUM_METODOS;m++)
            fprintf(archivo,"   ");
        fprintf(archivo,"Mineral");
        fprintf(archivo," ");
        fprintf(archivo,"Zonas");
        fprintf(archivo," ");
        for (m=0;m<NUM_METODOS;m++) // METODO
	        fprintf(archivo, "  %s ",NombreMetodo[m]);
    }
    fprintf(archivo,"\n");
    fprintf(archivo,"-------------------------------------------------------------------------------------------------------------------------\n");

    // Para cada mineral
    for (int i = 0; i<GetCount(); i++)
    {
        if (m_list[i]->m_list_zonas_test.size() != 0)
        {
            nTotalMinerales++;
            memset(ZonasAcertadas,0,NUM_METODOS*sizeof(int)); //reseteamos el contador de zonas (se reutiliza en cada mineral)
            // por cada zona de prueba
            nCountZonas = 0;
            for (it = m_list[i]->m_list_zonas_test.begin(); it!=m_list[i]->m_list_zonas_test.end();it++)
            {
//                if (bGenerarDatosPruebas)
//                    nCountZonas = nCountZonas + DATOS_GENERADOS_POR_ZONA_PRUEBAS;
//                else
                    nCountZonas++;

                // identificamos el mineral (con probabilidades) a partir de su espectro
                for (m=1;m<=NUM_METODOS;m++) // METODO
                {
                    if (bGenerarDatosPruebas)
                    {
                        for (int j=0;j<DATOS_GENERADOS_POR_ZONA_PRUEBAS;j++)
                        {
                            double dAleatorio;
                            if (!ALEATORIZAR_BANDAS_INDEPENDIENTEMENTE_PRUEBAS)
                                dAleatorio = GenerarDistribucionNormal(); // dato aleatorio de una distrubucion normal, comun para todas las bandas (para conservar la covarianza entre bandas)
                            for (int r=0;r<m_nBandas;r++)
                            {
                                if (ALEATORIZAR_BANDAS_INDEPENDIENTEMENTE_PRUEBAS)
                                    dAleatorio = GenerarDistribucionNormal(); // dato aleatorio de una distrubucion normal, independiente para cada banda (funciona mejor)
                                double desviacionInf = fabs((*it)->m_arrPercentil05[r] - (*it)->m_arrReflectancias[r]) / 1.6448;// Dispersiones: Se resta la media de CADA ZONA
                                double desviacionSup = fabs((*it)->m_arrPercentil95[r] - (*it)->m_arrReflectancias[r]) / 1.6448;// 1.6448 es la desviacion estandar al percentil 5% y 95% en una N(0,1)
                                arrReflectanciasGenerado[r] = CorregirDistribucion((*it)->m_arrReflectancias[r],desviacionInf,desviacionSup,dAleatorio);
                            }
                            Identificar(arrReflectanciasGenerado, listIdentificacion);
                            it_res = listIdentificacion.begin();
                            bool bCorrecto = GetAbreviatura((*it_res).m_nIndice) == m_list[i]->m_csAbreviatura;
                            if (bCorrecto)
                                ZonasAcertadas[m-1]++;

                            if (DETALLADO)
                                ImprimirResultadoIdentificacion(archivo, m_list[i]->m_csAbreviatura,nCountZonas,(*it)->m_nZona, j+1,bCorrecto,it_res,listIdentificacion.end()); //calcula confiabilidad e imprime datos
                        }
                    }
                    else
                    {
                        Identificar((*it)->m_arrReflectancias, listIdentificacion);
                        it_res = listIdentificacion.begin();
                        bool bCorrecto = GetAbreviatura((*it_res).m_nIndice) == m_list[i]->m_csAbreviatura;
                        if (bCorrecto)
                            ZonasAcertadas[m-1]++;

                        if (DETALLADO)
                            ImprimirResultadoIdentificacion(archivo, m_list[i]->m_csAbreviatura,nCountZonas,(*it)->m_nZona,0,bCorrecto,it_res,listIdentificacion.end()); //calcula confiabilidad e imprime datos
                    }
                }
            }

            if (bGenerarDatosPruebas)
                nCountZonas= nCountZonas*DATOS_GENERADOS_POR_ZONA_PRUEBAS;
            // preparamos el resumen total de zonas
            nTotalZonas += nCountZonas;

            if (!DETALLADO)
            {
	            fprintf(archivo, "\n");
                // Resumen simbolico
                for (m=0;m<NUM_METODOS;m++) // METODO
                {
                    if (ZonasAcertadas[m] == nCountZonas)
	                    fprintf(archivo, "   "); // TODAS ACERTADAS
                    else if (ZonasAcertadas[m] > 0)
	                    fprintf(archivo, "+  "); // ALGUNA NO ACERTADA
                    else
	                    fprintf(archivo, "#  "); // NINGUNA ACERTADA
                }
	            fprintf(archivo, "%6s ",m_list[i]->m_csAbreviatura);
	            fprintf(archivo, " %3d ",nCountZonas);
            }


            for (m=0;m<NUM_METODOS;m++) // METODO
            {
                // Imprimir info resumen mineral 
                if (!DETALLADO)
                {
                    fprintf(archivo, "         ");//para dejar espacio para el menu superior
	                fprintf(archivo, " %3d",ZonasAcertadas[m]);
                }
                // Contabilidad minerales acertados
                if (ZonasAcertadas[m] == nCountZonas)
                    MineralesAcertados[m]++;
                else if (ZonasAcertadas[m] > 0)
                    MineralesCasiAcertados[m]++;

                // preparamos el resumen total de zonas
                ZonasAcertadasTotal[m] += ZonasAcertadas[m]; //zonas acertadas para este metodo = zonas acertadas para este metodo y este mineral
            }

        } // if zonas!=0
    }

    // Resumen zonas
    fprintf(archivo,"\n------------------------------------------------------------------------------------------\n");
    if (DETALLADO)
	    fprintf(archivo, " RESUMEN ZONAS: ");
    else
        for (m = 0;m<NUM_METODOS;m++)
            fprintf(archivo,"   ");
    fprintf(archivo,"  TOTAL ");
    fprintf(archivo," ");
    fprintf(archivo,"%3d",nTotalZonas);
    fprintf(archivo," ");
    for (m=0;m<NUM_METODOS;m++) // METODO
    {
        if (DETALLADO)
	        fprintf(archivo, " %s:",NombreMetodo[m]);
        else
            fprintf(archivo, "         "); //para dejar espacio para el menu superior
	    fprintf(archivo, "%3d",ZonasAcertadasTotal[m]);
    }

    // Resumen general
    fseek(archivo,0,SEEK_SET); //al principio del fichero (se dejó el espacio necesario anteriormente)
	fprintf(archivo, "### RESUMEN GENERAL DE METODOS ###\n");
	fprintf(archivo, "%3d minerales TOTAL\n\n", nTotalMinerales);
    for (m=0;m<NUM_METODOS;m++) // METODO
    {
	    fprintf(archivo, " %s: %3d minerales perfectos, %3d a medias, %3d nunca\n",
            NombreMetodo[m], 
            MineralesAcertados[m], 
            MineralesCasiAcertados[m], 
            nTotalMinerales - MineralesAcertados[m] - MineralesCasiAcertados[m]);
    }
	fprintf(archivo, "\n\n\n");

    fclose(archivo);
/* PRUEBAS TIMEBENCH
fclose(log_file);
*/
}
