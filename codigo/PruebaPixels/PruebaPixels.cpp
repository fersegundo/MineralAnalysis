// EntrenaPixels.cpp: implementation of the CEntrenaPixels class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "PruebaPixels.h"
#include <math.h>
#include <algorithm> //for std::sort
//#include "Analisis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

static const double MIN_REFLECTANCIA_CLASIFICACION = 1; //si alguna reflectancia de cualquier banda es menor que este valor, se considerará ganga

// Guarda los pixeles correspondientes al porcentaje configurado en un nuevo fichero de pruebas
// dUmbralDistancia, dUmbralConfiabilidad. Si -1, no aplicamos clasificacion condicional. 
//  Si 0, aplicamos la "caja" con varias clasificaciones condicionales, el fichero de zonas de pruebas lo dejamos sin aplicar umbrales pero generamos todos los ficheros de estadisticas (combinaciones de la caja)
//  Si >0, aplicamos los umbrales dados
bool CPruebaPixels::Pruebas(CString csFicheroPruebas,double dUmbralDistancia, double dUmbralConfiabilidad)
{
    int d,c;
    SYSTEMTIME st;
    GetLocalTime(&st);

    // guardamos las zonas de prueba clasificadas
    FILE* fich = fopen(csFicheroPruebas,"wt");
    if (fich==NULL)
        return false;
    // Dejar espacio para resumen posterior
    fseek(fich,94 + 7*2,SEEK_SET);

    // Estadisticas clasificacion condicional
    #define NUM_DISTANCIAS      5
    #define NUM_CONFIABILIDADES 8
    int nNumDistancias;     //para usar en los bucles
    int nNumConfiabilidades;

    FILE* fich_cond[NUM_DISTANCIAS][NUM_CONFIABILIDADES];    // fichero para las estadisticas de la clasificacion condicional
    double arrDistancias[NUM_DISTANCIAS]            ;
    double arrConfiabilidades[NUM_CONFIABILIDADES]  ;
    int nAceptadosCorrectos[NUM_DISTANCIAS][NUM_CONFIABILIDADES]   = {0};     // pixels totales aceptados (que cumplen distancia y confiabilidad) clasificados como el mineral correcto
    int nAceptadosOtros[NUM_DISTANCIAS][NUM_CONFIABILIDADES]       = {0};     // pixels totales aceptados (que cumplen distancia y confiabilidad) clasificados como otro mineral
    int nRechazadosCorrectos[NUM_DISTANCIAS][NUM_CONFIABILIDADES]  = {0};     // pixels totales rechazados (que no cumplen distancia y confiabilidad) clasificados como el mineral correcto
    int nRechazadosOtros[NUM_DISTANCIAS][NUM_CONFIABILIDADES]      = {0};     // pixels totales aceptados rechazados (que no cumplen distancia y confiabilidad) clasificados como otro mineral
    if (dUmbralDistancia>0 && dUmbralConfiabilidad>0)
    {
        nNumDistancias      = 1;
        nNumConfiabilidades = 1;
        arrDistancias[0] = dUmbralDistancia           ;
        arrConfiabilidades[0] = dUmbralConfiabilidad ;
    }
    else if (dUmbralDistancia==0)
    {
        nNumDistancias      = 5;
        nNumConfiabilidades = 8;
        arrDistancias[0]        = 20  ;
        arrDistancias[1]        = 30  ;
        arrDistancias[2]        = 40  ;
        arrDistancias[3]        = 50  ;
        arrDistancias[4]        = 80  ;
        arrConfiabilidades[0]   = 3.0 ;
        arrConfiabilidades[1]   = 2.0 ;
        arrConfiabilidades[2]   = 1.7 ;
        arrConfiabilidades[3]   = 1.5 ;
        arrConfiabilidades[4]   = 1.4 ;
        arrConfiabilidades[5]   = 1.3 ;
        arrConfiabilidades[6]   = 1.2 ;
        arrConfiabilidades[7]   = 1.1 ;
    }
    else
    {
        nNumDistancias      = 0;
        nNumConfiabilidades = 0;
    }

    // abrimos la matriz de ficheros de estadisticas de clasificacion condicional
    CString csFicheroCond;
    for ( d=0;d<nNumDistancias;d++)
    {
        for ( c=0; c<nNumConfiabilidades;c++)
        {
            csFicheroCond.Format("%s_D%d_C%d_EstadisticasClasificacionCondicional.txt",csFicheroPruebas.Left(csFicheroPruebas.GetLength()-4),int(arrDistancias[d]),int(arrConfiabilidades[c]*10));
            fich_cond[d][c] = fopen(csFicheroCond,"wt");
            if (fich_cond[d][c]==NULL)
                return false;
            fprintf(fich_cond[d][c], "Estadisticas de clasificacion de pixels condicional. Umbral distancia: %f, Umbral confiabilidad: %f. \nGenerado por PruebaPixels\n",arrDistancias[d],arrConfiabilidades[c]);
            // Añadir fecha
            fprintf(fich_cond[d][c],"# %2d del %2d de %4d a las %2d:%2d\n\n\n" ,st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute);  //31 + 1*2
            // Encabezado
            fprintf(fich_cond[d][c], "Mineral\tVerdaderosPositivos\tFalsosPositivos\tFalsosNegativos\tVerdaderosNegativos\t               Total\n");
            fprintf(fich_cond[d][c], "--------------------------------------------------------------------------------------------------------------\n");
        }
    }

    // variables necesarias para la identificacion
    list<CMineralClasificado> listClasificados; //por cada zona
    CResultadoIdentificacion primero;
    CResultadoIdentificacion segundo;
    int nNumMinerales = m_minerales.GetCount();

    // listas usadas por cada zona
    int*    arrCountMinerales           = new int[nNumMinerales];     //lista para contabilizar el numero de veces que se ha identificado cada mineral en cada zona.
    double* arrDistanciaMinerales       = new double[nNumMinerales];  //lista para almacenar la distancia media de cada mineral en cada zona
    double* arrConfiabilidadMinerales   = new double[nNumMinerales];  //lista para almacenar la confiabilidad de cada mineral en cada zona
    int nCountOscuros = 0; //contador de pixeles que no se procesan porque son demasiado oscuros


    CMineral* pMineral = NULL;

    // Contadores globales a toda la prueba
    int nZonasPruebaActual = 1;


    for (int i = 0; i<nNumMinerales; i++)
    {
        pMineral = m_minerales.m_list[i];

        if (pMineral->m_reflectanciaMedia == -1) // si no hay valores (por ejemplo porque no habia suficientes datos para entrenar)
            continue; // no procesar este mineral

        vector<CMineralZona*>::iterator it;
        // resultados indentificacion

        memset(nAceptadosCorrectos,     0,NUM_DISTANCIAS*NUM_CONFIABILIDADES*sizeof(int));
        memset(nAceptadosOtros,         0,NUM_DISTANCIAS*NUM_CONFIABILIDADES*sizeof(int));
        memset(nRechazadosCorrectos,    0,NUM_DISTANCIAS*NUM_CONFIABILIDADES*sizeof(int));
        memset(nRechazadosOtros,        0,NUM_DISTANCIAS*NUM_CONFIABILIDADES*sizeof(int));

        for (it = pMineral->m_list_zonas_test.begin(); it!=pMineral->m_list_zonas_test.end();it++)
        {
            // PARA CADA ZONA DE PRUEBA
            //inicializacion buffers auxiliares
            memset(arrCountMinerales,           0, nNumMinerales*sizeof(int));
            memset(arrDistanciaMinerales,       0, nNumMinerales*sizeof(double));
            memset(arrConfiabilidadMinerales,   0, nNumMinerales*sizeof(double));
            nCountOscuros = 0;

            listClasificados.clear();

            for (int p=0; p < ((*it)->m_nNumPixels) ; ++p)
            {
                if ((*it)->m_arrPixels[p][0] > 0 && (*it)->m_arrPixels[p][0] < MIN_REFLECTANCIA_CLASIFICACION)
                {
                    nCountOscuros++;
                }
                else if ((*it)->m_arrPixels[p][0] > 0)
                {
                    // identificamos el mineral (con probabilidades) a partir de su espectro
                    m_minerales.Identificar((*it)->m_arrPixels[p], primero, segundo);
                    ASSERT(primero.m_nIndice!=-1);
                    if (primero.m_nIndice != -1)
                    {
                        double dConfiabilidad = segundo.m_dProbabilidad / primero.m_dProbabilidad;
                        if (primero.m_nIndice == i) // clasificado correctamente
                        {
                            if (dUmbralDistancia ==-1)// no se aplica clasificacion condicional, se contabilizan todos
                            {
                                ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador
                                arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                                arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
                            }
                            else if (dUmbralDistancia == 0)// se aplica clasificacion condicional, pero se contabilizan todos
                            {
                                ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador
                                arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                                arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
                                for ( d=0;d<nNumDistancias;d++)
                                    for ( c=0; c<nNumConfiabilidades;c++)
                                        if (primero.m_dProbabilidad < arrDistancias[d] && dConfiabilidad > arrConfiabilidades[c]) // Aceptado, pasa los dos umbrales
                                            ++nAceptadosCorrectos[d][c];
                                        else
                                            ++nRechazadosCorrectos[d][c];
                            }
                            else if (primero.m_dProbabilidad < dUmbralDistancia && dConfiabilidad > dUmbralConfiabilidad) // Aceptado, pasa los dos umbrales
                            {
                                ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador
                                arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                                arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
                                // Estadisticas clasificacion condicional
                                ++nAceptadosCorrectos[0][0];
                            }
                            else
                            {
                                ++nRechazadosCorrectos[0][0];
                            }
                        }
                        else // otros
                        {
                            if (dUmbralDistancia ==-1)// no se aplica clasificacion condicional, se contabilizan todos
                            {
                                ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador
                                arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                                arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
                            }
                            else if (dUmbralDistancia == 0)// se aplica clasificacion condicional, pero se contabilizan todos
                            {
                                ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador
                                arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                                arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
                                for ( d=0;d<nNumDistancias;d++)
                                    for ( c=0; c<nNumConfiabilidades;c++)
                                        if (primero.m_dProbabilidad < arrDistancias[d] && dConfiabilidad > arrConfiabilidades[c]) // Aceptado, pasa los dos umbrales
                                            ++nAceptadosOtros[d][c];
                                        else
                                            ++nRechazadosOtros[d][c];
                            }
                            else if (primero.m_dProbabilidad < dUmbralDistancia && dConfiabilidad > dUmbralConfiabilidad) // Aceptado, pasa los dos umbrales
                            {
                                ++arrCountMinerales[primero.m_nIndice]; //añadimos el mineral identificado a su contador
                                arrDistanciaMinerales[primero.m_nIndice] += primero.m_dProbabilidad;
                                arrConfiabilidadMinerales[primero.m_nIndice] += dConfiabilidad;
                                // Estadisticas clasificacion condicional
                                ++nAceptadosOtros[0][0];
                            }
                            else
                            {
                                ++nRechazadosOtros[0][0];
                            }
                        }
                    }
                }
                // else, no hay informacion de reflectancias porque fue excluido al limitar el numero de pixels de entrenamiento
            }
            //añadimos cada mineral identificado y cuantos pixeles han sido clasificados
            for (int m=0;m<nNumMinerales;++m)
            {
                if (arrCountMinerales[m] > 0)
                {
                    CMineralClasificado mineral;
                    mineral.m_nIndex = m;
                    mineral.m_nCount = arrCountMinerales[m];
                    mineral.m_dConfiabilidad = arrConfiabilidadMinerales[m] / arrCountMinerales[m]; // se guarda la media
                    mineral.m_dDistancia = arrDistanciaMinerales[m] / arrCountMinerales[m];         // se guarda la media
                    listClasificados.push_back(mineral);
                }
            }

            //añadimos como un mineral mas el numero de pixeles no clasificados por ser MUY OSCUROS
            if (nCountOscuros > 0)
            {
                CMineralClasificado mineral;
                mineral.m_nIndex = -1; //MUY OSCUROS
                mineral.m_nCount = nCountOscuros;
                listClasificados.push_back(mineral);
            }

            listClasificados.sort(); //se ordenan por m_nCount (ver "operator<" de CMineralClasificado)

            // Escribir datos a fichero de salida
            fprintf(fich, "[%d]\n", nZonasPruebaActual);
            fprintf(fich, "Abreviatura   = %s\n", pMineral->m_csAbreviatura);
            fprintf(fich, "Mineral       = %s\n", pMineral->m_csNombre);
            fprintf(fich, "Calidad       = %s\n", (*it)->m_csCalidad);
            fprintf(fich, "Comentario    = %s\n", (*it)->m_csComentario);
            //  fprintf(fich, "Procedencia   = %s campo:%d fila:%d\n", csMuestra, campo, fila);
            fprintf(fich, "Procedencia   = %s\n", (*it)->m_csProcedencia);

            list<CMineralClasificado>::iterator it_clasificado;
            fprintf(fich, "Clasificado   = ");
            for (it_clasificado=listClasificados.begin();it_clasificado!=listClasificados.end();it_clasificado++)
            {
                CString csNombre;
                if ((*it_clasificado).m_nIndex != -1)
                    csNombre = m_minerales.GetNombre((*it_clasificado).m_nIndex);
                else
                    csNombre = "MUY_OSCUROS";
	            fprintf(fich, "%-19s ",csNombre);
            }
	        fprintf(fich, "\n");

            fprintf(fich, "Pixels        = ");
            for (it_clasificado=listClasificados.begin();it_clasificado!=listClasificados.end();it_clasificado++)
            {
                fprintf(fich,"%-19d ",(*it_clasificado).m_nCount);
            }

	        fprintf(fich, "\n");
            fprintf(fich, "Distancia     = ");
            for (it_clasificado=listClasificados.begin();it_clasificado!=listClasificados.end();it_clasificado++)
            {
	            fprintf(fich, "%-19lf ",(*it_clasificado).m_dDistancia);
            }
	        fprintf(fich, "\n");
            fprintf(fich, "Confiabilidad = ");
            for (it_clasificado=listClasificados.begin();it_clasificado!=listClasificados.end();it_clasificado++)
            {
	            fprintf(fich, "%-19lf ",(*it_clasificado).m_dConfiabilidad);
            }
	        fprintf(fich, "\n\n");

           
            ++nZonasPruebaActual;
        } // por cada zona

        // guardamos estadisticas clasificacion condicional
        for ( d=0;d<nNumDistancias;d++)
            for ( c=0; c<nNumConfiabilidades;c++)
                if (fich_cond[d][c] != NULL)
                    fprintf(fich_cond[d][c], "% 7s\t% 19d\t% 15d\t% 15d\t% 19d\t%20d\n", pMineral->m_csAbreviatura,nAceptadosCorrectos[d][c], nAceptadosOtros[d][c], nRechazadosCorrectos[d][c], nRechazadosOtros[d][c], pMineral->m_nTotalPixels);

    } //por cada mineral

    delete[] arrDistanciaMinerales;
    delete[] arrConfiabilidadMinerales;
    delete[] arrCountMinerales;

    // Añadir resumen a archivo de salida
    fseek(fich,0,SEEK_SET);
    fprintf(fich, "# Resultado de clasificacion de areas de prueba\n"); //47 + 1*2
    // Añadir fecha
    fprintf(fich,"# %2d del %2d de %4d a las %2d:%2d\n" ,st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute);  //31 + 1*2
    fprintf(fich, "\n\n"); //2*2
    fprintf(fich, "num_datos = %4d\n",nZonasPruebaActual-1); //16 + 1*2
    fprintf(fich, "\n\n");//2*2

    // liberamos matriz de estadisticas de clasificacion condicional
    fclose(fich);
    for ( d=0;d<nNumDistancias;d++)
        for ( c=0; c<nNumConfiabilidades;c++)
            if (fich_cond[d][c] != NULL)
                fclose(fich_cond[d][c]);

    return true;
}

/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es el directorio donde buscar (recursivamente) los ficheros de entrada
	 - Segundo es el nombre (y el camino) del fichero de salida
*****************************************************************************/
bool procesa_argumentos(int argc, TCHAR* argv[], CString& csFicheroMinerales, CString& csFicheroMahalanobis, CString& csFicheroEntrada, CString& csFicheroSalida, double& dDistancia, double& dConfiabilidad)
{

    if (argc < 1 || argc>7)
    {
        printf("Error: numero incorrecto de argumentos.\n");
        return false;
    }

	if (argc > 6)  
	    dConfiabilidad = atof(argv[6]);
    else
        dConfiabilidad = -1;

	if (argc > 5)  
	    dDistancia = atof(argv[5]);
    else
        dDistancia = -1;

	if (argc > 4)  
	    csFicheroSalida = argv[4];
    else
        csFicheroSalida = csFicheroEntrada.Left(csFicheroEntrada.GetLength()-4) + "_clasificacion.txt";

	if (argc > 3)  
	    csFicheroEntrada = argv[3];
    else
        csFicheroEntrada = FICHERO_DEFECTO_PRUEBAS;

	if (argc > 2)  
	    csFicheroMahalanobis = argv[2];
    else
        csFicheroMahalanobis = FICHERO_DEFECTO_MAHALANOBIS;

	if (argc > 1)  
	    csFicheroMinerales = argv[1];
    else
        csFicheroMinerales = FICHERO_DEFECTO_MINERALES;

    return true;
}

/**********************************************************************
// MAIN
***********************************************************************/
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    CString csFicheroMinerales;
    CString csFicheroMahalanobis;
    CString csFicheroEntrada;
    CString csFicheroSalida;

    double dDistancia;
    double dConfiabilidad;

	if ( procesa_argumentos(argc, argv,csFicheroMinerales,csFicheroMahalanobis,csFicheroEntrada,csFicheroSalida,dDistancia, dConfiabilidad) == false)		
		return 1;

    CPruebaPixels prueba;
#ifndef _DEBUG
    try
    {
#endif
        // cargar nombres y abreviaturas de minerales
        prueba.m_minerales.CargarMinerales(csFicheroMinerales);
        if (prueba.m_minerales.CargarEntrenamiento(csFicheroMahalanobis,prueba.m_nNumBandas) == false)
        {
            printf("Error al cargar entrenamiento");
            return FALSE;
        }
        prueba.m_nNumBandas = prueba.m_minerales.CargarPixels(csFicheroEntrada, true, prueba.m_log); // true indica que se almacenan en m_list_zonas_test
        prueba.Pruebas(csFicheroSalida, dDistancia, dConfiabilidad);
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
