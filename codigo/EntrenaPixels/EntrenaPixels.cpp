// EntrenaPixels.cpp: implementation of the CEntrenaPixels class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "EntrenaPixels.h"
#include <math.h>
#include <algorithm> //for std::sort
//#include "Analisis.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


// PARAMETROS APRENDIZAJE
// solo se usaran estas opciones si el archivo de zonas contiene pixels y no zonas (si la extension es .dat y no .pix)
static const bool bGenerarDatosClasificacion = true; //indica si generamos datos sinteticos a partir de la moda y los percentiles a la hora de calcular la matriz de covarianza
    static const bool BOOL_IGUALAR_DATOS = true; //forzamos que el numero de datos para cada mineral sea el mismo. generamos hasta alcanzar el valor
        static const int MAXIMO_DATOS = 1000; // no permitimos mas valores de datos por cada mineral. generamos hasta alcanzar este valor
        static const int MIN_GENERADOS_POR_ZONA_CLASIFICACION = 1; // si estamos igualando datos y sobran, no hace falta generar, pero si este valor es mayor que 0, forzamos que se generen (aun desestimando aun mas datos originales)
    static const int DATOS_GENERADOS_POR_ZONA_CLASIFICACION = 20; // datos por cada zona. solo si BOOL_IGUALAR_DATOS es false. se ha visto experimentalmente que mas de 20 no aporta mejora y entre 10 y 20 la mejora es despreciable
    static const bool ALEATORIZAR_BANDAS_INDEPENDIENTEMENTE = 1; //1 // Al generar valores sinteticos, se generan valores aleatorios por cada una de las bandas (si 0, todas las bandas se mueven a la vez) 


extern bool ComparadorMinerales(CMineral* mineral1,CMineral* mineral2);
//using namespace std;

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

// Funcion auxiliar visible solo a este modulo/fichero (static)
// genera un dato aleatoriamente segun una distribucion normal con media y desviaciones dadas
static double GenerarDatoDistribucion(const double dMedia, const double dDesviacionInf, const double dDesviacionSup)
{
    return CorregirDistribucion(dMedia,dDesviacionInf,dDesviacionSup,GenerarDistribucionNormal());
}


// HACER BORRAR, esto solo para version ZONAS static const int HOLGURA_INDICES_BUSQUEDA = 20; //en niveles de gris, numero que se suma/resta a los percentiles
/* ESTO SOLO PARA EntrenaZonas
// Cargar lista de zonas de fichero
bool CMinerales::CargarZonas(CString csFichero, bool bPruebas)
{
    // lee todas las variables del fichero y las guarda en variables globales
	if ( LoadVars(csFichero) == NULL) {
		return false;
	}

    int num_datos;
    LOADINT(num_datos);
    ASSERT(GetNumSections() == num_datos);

    //Ordenamos los minerales por abreviatura
//AfxMessageBox("Si no, comando sort en DEBUG (pero sin debugear) por algun motivo impide el programa"); // HACER QUITAR
    CMineral::m_enumOrden = ORDEN_ABREVIATURA; // m_enumOrden es static
    std::sort(m_list.begin(),m_list.end(),m_minerales.ComparadorMinerales);

    for (int i=0; i <GetNumSections(); i++)
    {
        
        // Leer info
        char* Abreviatura;
        CString csAbreviatura;
        Abreviatura = csAbreviatura.GetBuffer(100);
	    LOADSTRSECTION(Abreviatura,i);
        csAbreviatura.ReleaseBuffer(-1);
        csAbreviatura.MakeLower();

        char* Calidad;
        CString csCalidad;
        Calidad = csCalidad.GetBuffer(100);
	    LOADSTRSECTION(Calidad,i);
        csCalidad.ReleaseBuffer(-1);

        // Encontrar el mineral en la lista de minerales
        CMineral* pMineral = BuscarPorAbreviatura(csAbreviatura);

        if (pMineral!=NULL && csCalidad != CALIDAD_E) //No consideramos minerales con baja calidad
        {
            CMineralZona* pMineral_zona = new CMineralZona;

            pMineral_zona->m_nZona = i+1;

            if (!bPruebas)
                // asignar a lista valida (luego se separaran los de prueba)
                pMineral->m_list_zonas.push_back(pMineral_zona);
            else
                // asignar a lista pruebas
                pMineral->m_list_zonas_test.push_back(pMineral_zona);

            char* Comentario;
            Comentario = pMineral_zona->m_csComentario.GetBuffer(100);
	        LOADSTRSECTION(Comentario,i);
            pMineral_zona->m_csComentario.ReleaseBuffer(-1);

            char* Procedencia;
            Procedencia = pMineral_zona->m_csProcedencia.GetBuffer(100);
	        LOADSTRSECTION(Procedencia,i);
            pMineral_zona->m_csProcedencia.ReleaseBuffer(-1);

            //espectros
            double* Espectros;
            Espectros = pMineral_zona->m_arrReflectancias;
            LOADVDBLSECTION(Espectros, m_nBandas,i); 

            //percentiles
            double* Percentil05;
            Percentil05 = pMineral_zona->m_arrPercentil05;
            LOADVDBLSECTION(Percentil05, m_nBandas,i);
            double* Percentil95;
            Percentil95 = pMineral_zona->m_arrPercentil95;
            LOADVDBLSECTION(Percentil95, m_nBandas,i);

            //Trabajamos internamente en valores de gris, asi que hay que convertir
            for (int r=0;r<m_nBandas;r++)
            {
                pMineral_zona->m_arrReflectancias[r] = pMineral_zona->m_arrReflectancias[r] / theApp.m_ParamIni.escala;
                pMineral_zona->m_arrPercentil05[r] = pMineral_zona->m_arrPercentil05[r] / theApp.m_ParamIni.escala;
                pMineral_zona->m_arrPercentil95[r] = pMineral_zona->m_arrPercentil95[r] / theApp.m_ParamIni.escala;
            }
        }

    }
    
    return true;
}
// Cargar lista de zonas de prueba de fichero
bool CMinerales::CargarZonasPruebas(CString csFichero)
{
    return CargarZonas(csFichero,true);
}
*/

// Limita los pixeles de entrenamiento cargados
void CEntrenaPixels::LimitarPixels(int nLimitePixels)
{
    if (nLimitePixels > 0)
    {
        fprintf(m_log,"\n   Limitamos pixles a %d\n",nLimitePixels);
        // limitar pixels a un maximo aleatoreamente independientemente del area
        vector<CMineral*>::iterator it;
        for (it=m_minerales.m_list.begin(); it!=m_minerales.m_list.end();it++)
        {
            int nPixelsA_Dejar = nLimitePixels;
            int nPixels = (*it)->m_nTotalPixels;
            fprintf(m_log,"%s\t %d\n",(*it)->m_csNombre,nPixels);
#if _DEBUG
            if (nPixels == 0)
            {
                AfxMessageBox("Atencion: No hay pixels para " + (*it)->m_csNombre);
            }
            else 
#endif
                if (nPixels > nPixelsA_Dejar)
            {
                vector<CMineralZona*>::iterator it_zonas;
                for (it_zonas = (*it)->m_list_zonas.begin(); it_zonas!=(*it)->m_list_zonas.end();it_zonas++)
                {
                    for (int p = 0; p<(*it_zonas)->m_nNumPixels; ++p)
                    {
                        if (((double)rand()/(double)RAND_MAX) < ((double)nPixelsA_Dejar / (double)nPixels))
                        {
                            // dejar en lista
                            nPixelsA_Dejar--; // nos aseguramos que haya la proporcion indicada de zonas (si por probabilidad no fuese asi, se ajusta hacia el final de la lista)
                        }
                        else
                        {
                            // dejamos valores a 0 como indicacion de que no se usará este pixel
                            memset((*it_zonas)->m_arrPixels[p],0,NUM_BANDAS); // segundo se elimina 
                            (*it_zonas)->m_nNumPixelsNoExcluidos--; //actualizamos el numero total de pixels utiles en la zona
                        }
                        nPixels--;// nos aseguramos que haya la proporcion indicada de zonas de prueba (si por probabilidad no fuese asi, se ajusta hacia el final de la lista)
                    }
                }
                (*it)->m_nTotalPixels = nLimitePixels;
            }
        }
        fprintf(m_log,"\n\n\n");
    }

}

// Calculamos las matrices de covarianza, reflectancias medias e indices de busqueda
bool CEntrenaPixels::Entrenar()
{
    int i;

    // CALCULAR PROMEDIO ZONAS: Calcular datos de clasificacion (promedio, matriz de covarianzas, etc)
    // Tiene que ser en un bucle independiente para asegurarnos que no influya la aleatoriedad del calculo de las matrices de covarianza en otras metricas
    vector<CMineral*>::iterator it;
//Calculamos y guardamos color RGB de los minerales
//FILE* ficherito;
//ficherito=fopen("D:\\Proyectos\\Minerales\\codigo\\Analisis\\log_colores.txt","wt");
    for (it=m_minerales.m_list.begin(); it!=m_minerales.m_list.end();++it)
    {
        if ((*it)->m_list_zonas.size() != 0)
        {
            CalcularPromedioZonas((*it));

            // guardamos valores de refelectancia y desviacion tipica en el log
            fprintf(m_log,"%s\t",(*it)->m_csNombre);
            for (i=0;i<m_nNumBandas;i++)
            {
                fprintf(m_log,"%.2lf\t",(*it)->m_arrReflectancias[i]);
            }
fprintf(m_log,"\t\t\t\t");
for (i=0;i<m_nNumBandas;i++)
{
    fprintf(m_log,"%.2lf\t",(*it)->m_desviacion_tipica[i]);
}
            fprintf(m_log,"\n\n");
        }
//if(ficherito){                
//int b=((*it).m_arrReflectancias[0] + (*it).m_arrReflectancias[1])/2;
//int g=((*it).m_arrReflectancias[2] + (*it).m_arrReflectancias[3])/2;
//int r=((*it).m_arrReflectancias[4] + (*it).m_arrReflectancias[5])/2;
//fprintf(ficherito,"%2d %10s RGB: %3d\t%3d\t%3d\n",(*it).m_nId,(*it).m_csAbreviatura,r,g,b);
//}                
    }
    fprintf(m_log,"\n\n\n");
//if (ficherito) fclose(ficherito);


/*
    // ****** PRUEBAS : distancias mahalanobis medias de cada mineral con los demas
    int j;
    double difVar[NUM_BANDAS];
    double acum;
    int numMinerales = m_minerales.GetCount();
    double** matrizDistancias = new double*[numMinerales];
    for (int m=0;m<m_numMinerales;m++)
    {
        matrizDistancias[m] = new double[numMinerales];
        memset(matrizDistancias[m],0,sizeof(double)*numMinerales);
    }

    int min_act = 0;
    for (it=m_list.begin(); it!=m_list.end();it++)
    {
        int nPixels = (*it)->m_nTotalPixels;

        vector<CMineralZona*>::iterator it_zonas;
        for (it_zonas = (*it)->m_list_zonas.begin(); it_zonas!=(*it)->m_list_zonas.end();it_zonas++)
        {
            for (int p=0; p < (*it_zonas)->m_nNumPixels; ++p)
            {
                if ((*it_zonas)->m_arrPixels[p][0] != 0) //si es 0 significa que el pixel esta excluido
                {
                    vector<CMineral>::iterator it2;
                    int indice_mineral2 = 0;
                    for (it2=m_list.begin(); it2!=m_list.end();it2++)
                    {
                        // IDENTIFICAR
                        double dDistanciaCuadrado = 0;
                        double* vectorReflectancias  = (*it2)->m_arrReflectancias;
                        Matriz* matrizCovInv = (Matriz*)((*it2)->m_matrizCovarianzaInv);
                        // Calcular distancia Mahalanobis
                        for (i = NUM_BANDAS - 1; i >= 0; i--) // comparar con 0 es mas eficiente, por eso se hace al reves
                            difVar[i] = (*it_zonas)->m_arrPixels[p][i] - vectorReflectancias[i];//fabs

                        // distancia = [difVar] * [matrizCovarianzaInv] * [difVar]
                        for (i = NUM_BANDAS - 1; i >= 0; i--) 
                        {
                            acum = 0;
                            for (int j = NUM_BANDAS - 1; j >= 0; j--)
                                acum += difVar[j] * (*matrizCovInv)[j][i];
                            dDistanciaCuadrado += acum * difVar[i];
                        }
                        matrizDistancias[min_act][indice_mineral2] += dDistanciaCuadrado;
                        indice_mineral2++;
                    }
                }
            }
        }
        //promediar e imprimir
        fprintf(m_log,"%s\t",(*it).m_csAbreviatura);
        for (m=0;m<numMinerales;m++)
        {
            matrizDistancias[min_act][m] = matrizDistancias[min_act][m] / nPixels;
            fprintf(m_log,"%.2lf\t",matrizDistancias[min_act][m]);
        }
        fprintf(m_log,"\n");
        min_act++;
    }

    for (m=0;m<numMinerales;m++)
        delete [] matrizDistancias[m];
    delete [] matrizDistancias;

    // ************* FIN PRUEBAS
*/

    return true;
}
// A partir de la lista de zonas (solo m_list_zonas) se calcula los valores medios de 
// reflectancia (y su desviacion) y "dispersion media" (valor aproximado de la desviacion calculada a partir de los percentiles)
// Tambien se calcula la matriz de covarianza
// que se usaran como referencia para este mineral
void CEntrenaPixels::CalcularPromedioZonas(CMineral* pMineral)
{
    int i = 0;
    int nZona = 0;
    int nPixel = 0;

    int numZonas = pMineral->m_list_zonas.size();
    // reset valores
    memset(pMineral->m_arrReflectancias,0,NUM_BANDAS*sizeof(double)); //importante que no tenga valores si numZonas == 0

    if (numZonas > 0)
    {
        vector<CMineralZona*>::iterator it;

        int nDatosPorZona = DATOS_GENERADOS_POR_ZONA_CLASIFICACION; //por defecto
        int nDatos = -1; //numero total de datos para este mineral
        if (pMineral->m_nTotalPixels > 0)
        {
            nDatosPorZona = -1; // no aplica
            nDatos = pMineral->m_nTotalPixels;
        }
        else if (bGenerarDatosClasificacion && BOOL_IGUALAR_DATOS)//si hay que igualar datos, hay que calcular cuantos datos por zona
        {
            nDatosPorZona =  max(MIN_GENERADOS_POR_ZONA_CLASIFICACION, (int)ceil((double)MAXIMO_DATOS / (double)numZonas)); // o bien el minimo (y se descartaran zonas) o las necesarias para asegurarnos que llegamos al maximo
            nDatos = MAXIMO_DATOS;
        }
        else if (bGenerarDatosClasificacion)
        {
            nDatos = numZonas*nDatosPorZona;
        }
        else
        {
            nDatos = numZonas;
        }
        double**          datosCovarianza;
        datosCovarianza = new double *[nDatos]; // auxiliar para MATRIZ COVARIANZA (*2 porque se usará el percentil inf y sup)
        for (i=0;i<nDatos;i++)
            datosCovarianza[i] = new double [m_nNumBandas];

        // - Primero calcular promedio de reflectancias y diferencias entre cada percentil y cada reflectancia
        // - Preparar datos para covarianza
        nZona = 0;
        nPixel = 0;
        double desviacionInf;
        double desviacionSup; 
        pMineral->m_reflectanciaMediaMin = INT_MAX;
        bool bMaximoAlcanzado = false; // en caso BOOL_IGUALAR_DATOS, si ya hemos llegado al maximo, se dejan de procesar zonas
        for (it = pMineral->m_list_zonas.begin(); it!=pMineral->m_list_zonas.end() && !bMaximoAlcanzado;it++) //para cada zona
        {
            if (pMineral->m_nTotalPixels > 0) // hay informacion de reflectancias de pixels
            {
                for (int p=0;p<(*it)->m_nNumPixels;++p) //para cada pixel
                {
                    if ((*it)->m_arrPixels[p][0] != 0) //si es 0, el pixel fue excluido para alcanzar MAXIMO_PIXELS
                    {
                        double ref_media = 0; //auxiliar para calcular m_reflectanciaMediaMin y m_reflectanciaMediaMax
                        for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
                        {
                            ref_media += (*it)->m_arrPixels[p][i];
                            (*it)->m_arrReflectancias[i] += (*it)->m_arrPixels[p][i]; // usado para comprobar que ninguna zona sea muy diferente al promedio del mineral
                            pMineral->m_arrReflectancias[i] += (*it)->m_arrPixels[p][i];
                            datosCovarianza[nPixel][i] = (*it)->m_arrPixels[p][i];
                        }
                        pMineral->m_reflectanciaMediaMin = min(pMineral->m_reflectanciaMediaMin,ref_media/m_nNumBandas); //necesario para la busqueda en funcion de la reflectancia media
                        pMineral->m_reflectanciaMediaMax = max(pMineral->m_reflectanciaMediaMax,ref_media/m_nNumBandas); //necesario para la busqueda en funcion de la reflectancia media
                        ++nPixel;
                    }
                }
                for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
                    (*it)->m_arrReflectancias[i] =(*it)->m_arrReflectancias[i] / (*it)->m_nNumPixelsNoExcluidos; // usado para comprobar que ninguna zona sea muy diferente al promedio del mineral
            }
            else //hay informacion de reflectancias de zonas
            {
                for (i=0; i<m_nNumBandas;i++) //para cada valor espectral
                {
                    pMineral->m_arrReflectancias[i] += (*it)->m_arrReflectancias[i];
                    pMineral->m_reflectanciaMediaMin += (*it)->m_arrPercentil05[i]; //necesario para la busqueda en funcion de la reflectancia media
                    pMineral->m_reflectanciaMediaMax += (*it)->m_arrPercentil95[i]; //necesario para la busqueda en funcion de la reflectancia media
                }

                if (bGenerarDatosClasificacion)
                {
                    bool bMaximoAlcanzadoSintesis = false; // en caso BOOL_IGUALAR_DATOS, si ya hemos llegado al maximo, se dejan de sintetizar datos
                    for (int j=0;j<nDatosPorZona && !bMaximoAlcanzadoSintesis;++j) // SINTESIS
                    {
                        double dAleatorio;
                        if (!ALEATORIZAR_BANDAS_INDEPENDIENTEMENTE)
                            dAleatorio = GenerarDistribucionNormal(); // dato aleatorio de una distrubucion normal, comun para todas las bandas (para conservar la covarianza entre bandas)
                        for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
                        {
                            if (ALEATORIZAR_BANDAS_INDEPENDIENTEMENTE)
                                dAleatorio = GenerarDistribucionNormal(); // dato aleatorio de una distrubucion normal, independiente para cada banda
                            desviacionInf = fabs((*it)->m_arrPercentil05[i] - (*it)->m_arrReflectancias[i]) / 1.6448;// Dispersiones: Se resta la media de CADA ZONA
                            desviacionSup = fabs((*it)->m_arrPercentil95[i] - (*it)->m_arrReflectancias[i]) / 1.6448;// 1.6448 es la desviacion estandar al percentil 5% y 95% en una N(0,1)
                            datosCovarianza[nZona*nDatosPorZona + j][i] = CorregirDistribucion((*it)->m_arrReflectancias[i],desviacionInf,desviacionSup,dAleatorio);
                        }
                        if (BOOL_IGUALAR_DATOS && nZona*nDatosPorZona + j+1>= MAXIMO_DATOS)
                            bMaximoAlcanzadoSintesis = true;
                    }
                }
                ++nZona;
                if (BOOL_IGUALAR_DATOS)
                    bMaximoAlcanzado = nZona*nDatosPorZona >= MAXIMO_DATOS;
            }
        }
        //calculamos reflectancia media de todas las bandas
        pMineral->m_reflectanciaMedia = 0;
        if (pMineral->m_nTotalPixels > 0) //informacion de reflectancia de pixels
        {
            for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
            {
                pMineral->m_arrReflectancias[i] = pMineral->m_arrReflectancias[i] / pMineral->m_nTotalPixels;   
                pMineral->m_reflectanciaMedia += pMineral->m_arrReflectancias[i];
            }
        }
        else //informacion de reflectancia de zonas
        {
            for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
            {
                pMineral->m_arrReflectancias[i] = pMineral->m_arrReflectancias[i] / numZonas;   
                pMineral->m_reflectanciaMedia += pMineral->m_arrReflectancias[i];
            }
            pMineral->m_reflectanciaMediaMin = pMineral->m_reflectanciaMediaMin / (numZonas*m_nNumBandas);
            pMineral->m_reflectanciaMediaMax = pMineral->m_reflectanciaMediaMax / (numZonas*m_nNumBandas);
        }
        pMineral->m_reflectanciaMedia = pMineral->m_reflectanciaMedia / m_nNumBandas;
memset(pMineral->m_desviacion_tipica,0,sizeof(double)*NUM_BANDAS);
        // terminar de preparamos datos covarianza, restar la media
        for (int j=0;j<nDatos;++j)
        {
            for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
            {
                datosCovarianza[j][i] = /*fabs*/(datosCovarianza[j][i] - pMineral->m_arrReflectancias[i]);// Covarianza: Se resta la media de TODAS LAS ZONAS
pMineral->m_desviacion_tipica[i] += datosCovarianza[j][i]*datosCovarianza[j][i];
            }
        }
for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
{
pMineral->m_desviacion_tipica[i] = sqrt(pMineral->m_desviacion_tipica[i] / nDatos);
}

    // comprobar que ninguna zona sea muy diferente al promedio del mineral
    for (it = pMineral->m_list_zonas.begin(); it!=pMineral->m_list_zonas.end();it++) //para cada zona
    {
        for (i=0; i<m_nNumBandas;++i) //para cada valor espectral
        {
            double dDistancia = fabs((*it)->m_arrReflectancias[i] - pMineral->m_arrReflectancias[i]);
            if (dDistancia > pMineral->m_desviacion_tipica[i]*MULT_DESV_TIPICA_TEST_ZONAS)
                fprintf(m_log,"\n ERROR: El area %s tiene una distancia muy grande (%lf) a su mineral: %s en la banda %d (desviacion tipica %lf)\n",(*it)->m_csProcedencia, dDistancia, pMineral->m_csNombre,i, pMineral->m_desviacion_tipica[i]);
        }

    }

/*
// terminar de preparamos datos covarianza, restar la media
int nZonasProcesadas = nZona; //en caso BOOL_IGUALAR_DATOS, igual no se han procesado todas las zonas
nZona = 0;
for (it = pMineral->m_list_zonas.begin(); it!=pMineral->m_list_zonas.end() && nZona<nZonasProcesadas; it++) //para acada zona
{
    for (i=0; i<NUM_BANDAS;++i) //para cada valor espectral
    {
        if (bGenerarDatosClasificacion)
        {
            bool bMaximoAlcanzadoSintesis = false; // en caso BOOL_IGUALAR_DATOS, si ya hemos llegado al maximo, se dejan de sintetizar datos
            for (int j=0;j<nDatosPorZona && !bMaximoAlcanzadoSintesis;++j)
            {
                datosCovarianza[nZona*nDatosPorZona + j][i] = (datosCovarianza[nZona*nDatosPorZona + j][i] - pMineral->m_arrReflectancias[i]);// Covarianza: Se resta la media de TODAS LAS ZONAS
                if (BOOL_IGUALAR_DATOS && nZona*nDatosPorZona + j+1>= MAXIMO_DATOS)
                    bMaximoAlcanzadoSintesis = true;
            }
        }
        else
        {
            datosCovarianza[nZona][i] = ((*it).m_arrReflectancias[i] - m_arrReflectancias[i]); // Covarianza: Se resta la media de TODAS LAS ZONAS
        }
    }
    ++nZona;
}
*/
        // CALCULO MATRIZ COVARIANZA E INVERSION
        MatrizCovarianza(datosCovarianza, pMineral->m_matrizCovarianzaInv, m_nNumBandas, nDatos );
        // Invertimos la matriz
        if (!InvierteMatriz(pMineral->m_matrizCovarianzaInv, m_nNumBandas))
        {
            // Ha habido un error al invertir, probablemente porque no hay suficientes datos o son muy homogeneos
            // No se puede usar este mineral
            CString csErrorMessage;
            csErrorMessage.Format("Advertencia: no hay suficientes datos de %s para poder incluirlo en el entrenamiento",pMineral->m_csNombre);
            AfxMessageBox(csErrorMessage);
            pMineral->m_reflectanciaMedia = -1; //de esta manera indicamos que no hay datos para este mineral
        }

        for (i=0;i<nDatos;i++)
            delete datosCovarianza[i];
        delete datosCovarianza;
    } //if numZonas>0
}

// Guardamos datos entrenamiento
void CEntrenaPixels::GuardarDatosEntrenamiento(CString csFicheroSalidaMahalanobis)
{
    int i,j;
    FILE* fMahalanobis;

    //El orden del fichero de entrenamiento es en orden de identificador
    CMineral::m_enumOrden = ORDEN_IDENTIFICADOR; // m_enumOrden es static
    std::sort(m_minerales.m_list.begin(),m_minerales.m_list.end(),ComparadorMinerales);

    fMahalanobis=fopen(csFicheroSalidaMahalanobis,"wt");
//            int list_size = m_list.size();
//            fwrite(&list_size,sizeof(int),1,fMahalanobis);
    
    if (fMahalanobis == NULL)
    {
        AfxMessageBox("No se puede abrir el fichero de entrenamiento " + csFicheroSalidaMahalanobis);
        return;
    }
    fprintf(fMahalanobis,"%d\n",m_nNumBandas);

    vector<CMineral*>::iterator it;
    for (it=m_minerales.m_list.begin(); it!=m_minerales.m_list.end();++it)
    {
        fprintf(fMahalanobis,"[%s]\n",(*it)->m_csAbreviatura);
        __int8 nHayZonas;
        if ((*it)->m_list_zonas.size() != 0)
        {
            nHayZonas = 1;
            fprintf(fMahalanobis,"%d\n",nHayZonas);
            for(i=0;i<m_nNumBandas;++i)
            {
                fprintf(fMahalanobis,"%.2lf\t",(*it)->m_arrReflectancias[i]);
            }
            fprintf(fMahalanobis,"\n");
            fprintf(fMahalanobis,"%.2lf\t%.2lf\t%.2lf\n",(*it)->m_reflectanciaMedia, (*it)->m_reflectanciaMediaMin, (*it)->m_reflectanciaMediaMax);
            for(i=0;i<NUM_BANDAS;++i)
            {
                for(j=0;j<m_nNumBandas;++j)
                {
                    fprintf(fMahalanobis,"%lf\t",(*it)->m_matrizCovarianzaInv[i][j]);
                }
                fprintf(fMahalanobis,"\n");
            }
        }
        else
        {
            nHayZonas = 0;
            fprintf(fMahalanobis,"%d\n",nHayZonas);
        }
    }
    fclose(fMahalanobis);
}

/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es el directorio donde buscar (recursivamente) los ficheros de entrada
	 - Segundo es el nombre (y el camino) del fichero de salida
*****************************************************************************/
bool procesa_argumentos(int argc, TCHAR* argv[], CString& csFicheroMinerales,CString& csFicheroEntrada, CString& csFicheroSalida, int& nLimitePixels)
{

	if (argc > 4)  
	    nLimitePixels = atoi(argv[4]);
    else
        nLimitePixels = 0; //por defecto no se limitaran pixels

	if (argc > 3)  
	    csFicheroSalida = argv[3];
    else
        csFicheroSalida = FICHERO_DEFECTO_MAHALANOBIS;

	if (argc > 2)  
	    csFicheroEntrada = argv[2];
    else
        csFicheroEntrada = FICHERO_DEFECTO_ENTRENAMIENTO;

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
    CString csFicheroEntrada;
    CString csFicheroSalida;
    int nLimitePixels;

	if ( procesa_argumentos(argc, argv,csFicheroMinerales,csFicheroEntrada,csFicheroSalida, nLimitePixels) == false)		
		return 1;

    CEntrenaPixels entrena;
#ifndef _DEBUG
    try
    {
#endif
        // cargar nombres y abreviaturas de minerales
        entrena.m_minerales.CargarMinerales(csFicheroMinerales);
        entrena.m_nNumBandas = entrena.m_minerales.CargarPixels(csFicheroEntrada, false, entrena.m_log);
entrena.m_nNumBandas = 13;
        entrena.LimitarPixels(nLimitePixels);
        entrena.Entrenar();
//        if (csFicheroSalida == "")
//            csFicheroSalida = csFicheroPixels.Left(csFicheroPixels.GetLength() - 3) + "mah";
        entrena.GuardarDatosEntrenamiento(csFicheroSalida);
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
