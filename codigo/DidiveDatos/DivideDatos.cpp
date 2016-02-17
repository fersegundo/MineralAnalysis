// DivideDatos.cpp: implementation of the CDivideDatos class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "DivideDatos.h"
#include <math.h>
#include <algorithm> //for std::sort
//#include "Analisis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

// Separa zonas de prueba segun el porcentaje configurado para pruebas posteriores
void CDivideDatos::SepararAleatoreamentePruebas(int nPorcentajeZonas)
{
    m_total_entrenamiento = 0;
    m_total_pruebas = 0;

    CMineral* pMineral = NULL;
    for (int i = 0; i<m_minerales.m_list.size(); i++)
    {
        pMineral = m_minerales.m_list[i];
        // deja el porcentaje indicado de zonas en la lista de zonas (m_list_zonas) 
        // moviendo las restantes a la lista de pruebas (m_list_zonas_test)
        ASSERT(nPorcentajeZonas >0);
        int nZonas = pMineral->m_list_zonas.size();

        vector<CMineralZona*>::iterator it;

        int nZonasA_Dejar = ceil(nPorcentajeZonas * nZonas / 100);

        for (it = pMineral->m_list_zonas.begin(); it!=pMineral->m_list_zonas.end();it++)
        {
            if (((double)rand()/(double)RAND_MAX) < ((double)nZonasA_Dejar / (double)nZonas))
            {
                // dejar en lista
                nZonasA_Dejar--; // nos aseguramos que haya la proporcion indicada de zonas (si por probabilidad no fuese asi, se ajusta hacia el final de la lista)
                m_total_entrenamiento ++;
            }
            else
            {
                pMineral->m_list_zonas_test.push_back(*it); // primero se añade el puntero

//                pMineral->m_nTotalPixels -= (*it)->m_nNumPixelsNoExcluidos; //restamos los pixeles NO EXCLUIDOS de esta zona del total de pixeles de entrenamiento
                m_total_pruebas ++;

                it = pMineral->m_list_zonas.erase(it); // segundo se elimina el puntero
                it--; // se incrementará en la siguiente iteracion (y ahora it apunta al proximo elemento)
            }
            nZonas--;// nos aseguramos que haya la proporcion indicada de zonas de prueba (si por probabilidad no fuese asi, se ajusta hacia el final de la lista)
        }
    }

    fprintf(m_log,"\nSeparadas pruebas. total zonas entrenamiento: %d, total zonas pruebas: %d \n",m_total_entrenamiento,m_total_pruebas);

}

void CDivideDatos::GuardaPixels(CString csFicheroEntrenamiento, int nPorcentajeZonas, bool bEntrenamiento)
{
	FILE*   archivo;
	if ((archivo = fopen(csFicheroEntrenamiento, "wt")) == NULL)
    {
        printf("Error no se ha podido abrir archivo %d",csFicheroEntrenamiento);
        ASSERT(FALSE);
		return;
    }
    
    fprintf(archivo, "# Información de reflectancia ");
    fprintf(m_log, "# Información de reflectancia ");
    int porcentaje;
    int nSize;

    if (bEntrenamiento)
    {
        nSize = m_total_entrenamiento;
        fprintf(archivo,"para entrenamiento ");
        fprintf(m_log,"para entrenamiento ");
        porcentaje = nPorcentajeZonas;
    }
    else
    {
        nSize = m_total_pruebas;
        fprintf(archivo,"para pruebas ");
        fprintf(m_log,"para pruebas ");
        porcentaje = 100 - nPorcentajeZonas;
    }
    fprintf(archivo, "de todos los PIXELES del %d%% de las areas\n", porcentaje);
    fprintf(m_log, "de todos los PIXELES del %d%% de las areas\n", porcentaje);
    // Añadir fecha
    SYSTEMTIME st;
    GetSystemTime(&st);
    fprintf(archivo,"# %2d del %2d de %4d a las %2d:%2d\n" ,st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute);
    fprintf(archivo, "\n");
    fprintf(archivo, "num_areas = %4d\n",nSize);
    fprintf(archivo, "num_bandas = %2d\n",m_nNumBandas);
    fprintf(archivo, "\n");
    fprintf(m_log,"# %2d del %2d de %4d a las %2d:%2d\n" ,st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute);
    fprintf(m_log, "\n");
    fprintf(m_log, "num_areas = %4d\n",nSize);
    fprintf(m_log, "num_bandas = %2d\n",m_nNumBandas);
    fprintf(m_log, "\n");

    CMineral* pMineral = NULL;

    int nZona = 1;
    for (int i = 0; i<m_minerales.m_list.size(); i++)
    {
        pMineral = m_minerales.m_list[i];

        vector<CMineralZona*>::iterator it_begin;
        vector<CMineralZona*>::iterator it_end;
        if (bEntrenamiento)
        {
            it_begin = pMineral->m_list_zonas.begin();
            it_end = pMineral->m_list_zonas.end();
        }
        else
        {
            it_begin = pMineral->m_list_zonas_test.begin();
            it_end = pMineral->m_list_zonas_test.end();
        }

        vector<CMineralZona*>::iterator it;
        for (it = it_begin; it!=it_end;it++)
        {
	        fprintf(archivo, "\n[%d]\n", nZona);
            nZona++;
            fprintf(archivo, "Procedencia = %s\n",(*it)->m_csProcedencia);
            fprintf(archivo, "Abreviatura = %s\n",pMineral->m_csAbreviatura);
            fprintf(archivo, "Calidad     = %s\n",(*it)->m_csCalidad);
            fprintf(archivo, "Comentario  = %s\n",(*it)->m_csComentario);
            fprintf(archivo, "Pixels      = %d\n",(*it)->m_nNumPixels);

            for (int p=0; p<(*it)->m_nNumPixels;p++)
            {
                for (int b=0; b < m_nNumBandas; b++) //pancromatico no
		            fprintf(archivo, "%.2lf ", (*it)->m_arrPixels[p][b]);
	            fprintf(archivo, "\n");
            }
        }
    }

    fclose(archivo);
}


/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es el directorio donde buscar (recursivamente) los ficheros de entrada
	 - Segundo es el nombre (y el camino) del fichero de salida
*****************************************************************************/
bool procesa_argumentos(int argc, TCHAR* argv[], int& nPorcentajeZonas, CString& csFicheroMinerales, CString& csFicheroPixels)
{

    if (argc < 2 || argc>4)
    {
        printf("Error: numero incorreto de argumentos. \nHace falta especificar al menos el porcentaje de zonas que se usaran para entrenar\n");
        return false;
    }

	if (argc > 3)  
	    csFicheroPixels = argv[3];
    else
        csFicheroPixels = FICHERO_DEFECTO_PIXELS;

	if (argc > 2)  
	    csFicheroMinerales = argv[2];
    else
        csFicheroMinerales = FICHERO_DEFECTO_MINERALES;

    nPorcentajeZonas = atoi(argv[1]);

    if (nPorcentajeZonas < 0 || nPorcentajeZonas>100)
    {
        printf("Error: valor incorrecto de porcentaje zonas, valor encontrado %d\n", nPorcentajeZonas);
        return false;
    }

    return true;
}

/**********************************************************************
// MAIN
***********************************************************************/
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    CString csFicheroMinerales;
    CString csFicheroPixels;
    CString csFicheroPruebas;
    CString csFicheroEntrenamiento;
    int nPorcentajeZonas;

	if ( procesa_argumentos(argc, argv,nPorcentajeZonas, csFicheroMinerales, csFicheroPixels) == false)		
		return 1;

    CDivideDatos divide;
#ifndef _DEBUG
    try
    {
#endif
        // cargar nombres y abreviaturas de minerales
        divide.m_minerales.CargarMinerales(csFicheroMinerales);
        divide.m_nNumBandas = divide.m_minerales.CargarPixels(csFicheroPixels, false, divide.m_log); 
        divide.SepararAleatoreamentePruebas(nPorcentajeZonas);
        csFicheroEntrenamiento = csFicheroPixels.Left(csFicheroPixels.GetLength()-4) + "_entrenamiento" + csFicheroPixels.Right(4);
        csFicheroPruebas = csFicheroPixels.Left(csFicheroPixels.GetLength()-4) + "_pruebas" + csFicheroPixels.Right(4);
        divide.GuardaPixels(csFicheroEntrenamiento, nPorcentajeZonas,true);
        divide.GuardaPixels(csFicheroPruebas,nPorcentajeZonas,false);
#ifndef _DEBUG
    } 
    catch (CMemoryException* e)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion de memoria al cargar minerales, es posible que el numero de datos a cargar sea excesivo para la memoria disponible");
        e->Delete();
        return FALSE;
    }
    catch (...)
    {
        // Probablemente se trate de una excepcion de memoria porque hay demasiados datos de pixels que cargar
        AfxMessageBox("Excepcion general al cargar minerales");
        return FALSE;
    }
#endif

	return 0;
}
