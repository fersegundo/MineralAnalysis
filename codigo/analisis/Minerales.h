// Minerales.h: interface for the CMineral and CMinerales class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(MINERALES_H_)
#define MINERALES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define TIMEBENCH //log de tiempos de ejecucion para analizar la optimizacion

#include "defines.h"
#include "matrices.h"
#include <vector>
#include <list>
//#include "analisis.h"

//PARAMETROS CLASIFICACION
#define NUM_INDICES_BUSQUEDA  256
#define USAR_INDICES_BUSQUEDA  true //ordenamos la lista de minerales por reflectancias medias (de todas las bandas) y usamos m_arrInicioBusqueda y m_arrFinBusqueda para acotar busqueda
#define MIN_MINERALES_BUSQUEDA  3   //por cada entrada de m_arrInicioBusqueda y m_arrFinBusqueda, forzamos el numero de minerales considerados
#define HOLGURA_INDICES_BUSQUEDA 5  //unidades de reflectancia extra que permitimos de holgura dese la reflectancia media minima / maxima al construir los indices de busqueda

#define FICHERO_DEFECTO_MINERALES       "minerales.dat"
#define FICHERO_DEFECTO_PIXELS          "recopilacion_pixels.pix"
#define FICHERO_DEFECTO_ENTRENAMIENTO   "recopilacion_pixels_entrenamiento.pix"
#define FICHERO_DEFECTO_PRUEBAS         "recopilacion_pixels_pruebas.pix"
#define FICHERO_DEFECTO_MAHALANOBIS     "recopilacion_pixels_entrenamiento.mah"
using namespace std;

// Clase necesaria para construir la lista de resultados ordenados para la identificacion de una zona
class CMineralClasificado
{
public:
    int m_nIndex; //indice del mineral identificado
    int m_nCount; //numero de pixels en la imagen identificados
    double m_dDistancia; //distancia promedio de todos los pixeles para este mineral
    double m_dConfiabilidad; //confiabilidad promedio de todos los pixeles para este mineral
    HTREEITEM m_hTree; //handle asociado con la entrada de la lista de minerales clasificados (tree view), para poder pintar de colores en el callback OnCustomdrawClasificados

    CMineralClasificado()
    {
        m_nIndex = -1;
        m_nCount = -1;
        m_dDistancia = -1;
        m_dConfiabilidad = -1;
        m_hTree  = NULL;
    } 

    virtual ~CMineralClasificado(){}

    CMineralClasificado& operator =(const CMineralClasificado& rhs)
    {
        if (&rhs != this)
        {
            m_nIndex = rhs.m_nIndex;
            m_nCount = rhs.m_nCount;
            m_dDistancia = rhs.m_dDistancia;
            m_dConfiabilidad = rhs.m_dConfiabilidad;
            m_hTree = rhs.m_hTree;
        }

        return *this;
    }

    bool operator==(const CMineralClasificado& rhs) const
    {
        return (m_nIndex == rhs.m_nIndex) ? true : false;
    }

    bool operator<(const CMineralClasificado& rhs) const
    {
        return (m_nCount > rhs.m_nCount) ? true : false;
    }    

    bool operator>(const CMineralClasificado& rhs) const
    {
        return (m_nCount < rhs.m_nCount) ? true : false;
    }

};

// Clase necesaria para construir la lista de resultados ordenados para la identificacion de un espectro
class CResultadoIdentificacion
{
private:

public:

    int             m_nIndice;
    double          m_dProbabilidad;

    CResultadoIdentificacion()
    {
        m_nIndice = -1;
        m_dProbabilidad = -1;
    } 

    virtual ~CResultadoIdentificacion(){}

    CResultadoIdentificacion& operator =(const CResultadoIdentificacion& rhs)
    {
        if (&rhs != this)
        {
            m_nIndice = rhs.m_nIndice;
            m_dProbabilidad = rhs.m_dProbabilidad;
        }

        return *this;
    }

    bool operator==(const CResultadoIdentificacion& rhs) const
    {
        return (m_nIndice == rhs.m_nIndice) ? true : false;
    }

    bool operator<(const CResultadoIdentificacion& rhs) const
    {
        return (m_dProbabilidad < rhs.m_dProbabilidad) ? true : false;
    }    

    bool operator>(const CResultadoIdentificacion& rhs) const
    {
        return (m_dProbabilidad > rhs.m_dProbabilidad) ? true : false;
    }
};

// clase con los detalles de una zona de una toma de una muestra (habrá muchas para un mismo mineral)
class CMineralZona
{
private:

public:

    int             m_nZona; //identificador unico del fichero de zonas
    CString         m_csCalidad;
    CString         m_csComentario;
    CString         m_csProcedencia;
    double          m_arrReflectancias[NUM_BANDAS]; // - o bien hay informacion de reflectancia moda y percentiles           
    double          m_arrPercentil05[NUM_BANDAS]; 
    double          m_arrPercentil95[NUM_BANDAS]; 
    int             m_nNumPixels;
    // si MAXIMO_PIXELS>0, se pueden excluir cierto numero de pixels que se pondrá su reflectancia a 0 (para no reordenar el array).
    // Es necesario llevar la cuenta de los pixels no excluidos para contabilidad en caso de que haya que separar areas para purebas
    int             m_nNumPixelsNoExcluidos; 
    double          (*m_arrPixels)[NUM_BANDAS];

    CMineralZona()
    {
        m_nZona = -1;
        m_csCalidad.Empty();
        m_csComentario.Empty();
        m_csProcedencia.Empty();
        memset(m_arrReflectancias,0,NUM_BANDAS*sizeof(double));
        memset(m_arrPercentil05,0,NUM_BANDAS*sizeof(double));
        memset(m_arrPercentil95,0,NUM_BANDAS*sizeof(double));
        m_nNumPixels = 0;
        m_nNumPixelsNoExcluidos = 0;
        m_arrPixels = NULL;
    } 

    CMineralZona(CMineralZona const& rhs)
    {
        if (&rhs != this)
        {
            m_nZona = rhs.m_nZona;
            m_csCalidad = rhs.m_csCalidad;
            m_csComentario = rhs.m_csComentario;
            m_csProcedencia = rhs.m_csProcedencia;
            for (int i = 0; i< NUM_BANDAS;i++)
            {
                m_arrReflectancias[i] = rhs.m_arrReflectancias[i];
                m_arrPercentil05[i] = rhs.m_arrPercentil05[i];
                m_arrPercentil95[i] = rhs.m_arrPercentil95[i];
            }
            //copiando matriz pixeles
            m_nNumPixels = rhs.m_nNumPixels;
            m_nNumPixelsNoExcluidos = rhs.m_nNumPixelsNoExcluidos;
            m_arrPixels = new double [m_nNumPixels][NUM_BANDAS];
            memset(m_arrPixels,0,sizeof(double) * m_nNumPixels * NUM_BANDAS);
            for (int j=0; j < m_nNumPixels; j++)
            {
                for (int b=0; b<NUM_BANDAS; b++)
                    m_arrPixels[j][b] = rhs.m_arrPixels[j][b];
            }
        }
    }

    virtual ~CMineralZona()
    {
        if (m_arrPixels != NULL)
            delete [] m_arrPixels;
        // OJO puede haber memory leak HACER
    }

    CMineralZona& operator =(const CMineralZona& rhs)
    {
        if (&rhs != this)
        {
            m_nZona = rhs.m_nZona;
            m_csCalidad = rhs.m_csCalidad;
            m_csComentario = rhs.m_csComentario;
            m_csProcedencia = rhs.m_csProcedencia;
            for (int i = 0; i< NUM_BANDAS;i++)
            {
                m_arrReflectancias[i] = rhs.m_arrReflectancias[i];
                m_arrPercentil05[i] = rhs.m_arrPercentil05[i];
                m_arrPercentil95[i] = rhs.m_arrPercentil95[i];
            }
            //copiando matriz pixeles
            m_nNumPixels = rhs.m_nNumPixels;
            m_nNumPixelsNoExcluidos = rhs.m_nNumPixelsNoExcluidos;
            m_arrPixels = new double [m_nNumPixels][NUM_BANDAS];
            memset(m_arrPixels,0,sizeof(double) * m_nNumPixels * NUM_BANDAS);
            for (int j=0; j < m_nNumPixels; j++)
            {
                for (int b=0; b<NUM_BANDAS; b++)
                    m_arrPixels[j][b] = rhs.m_arrPixels[j][b];
            }
        }

        return *this;
    }

    bool operator==(const CMineralZona& rhs) const
    {
        return (m_nZona == rhs.m_nZona) ? true : false;
    }
};

//////////////////////////////////////////////////////////////////////
//
// CMineral - Descripcion mineral y contenedor zonas
//
//////////////////////////////////////////////////////////////////////

enum enum_orden_mineral{
    ORDEN_IDENTIFICADOR              = 0,
    ORDEN_ABREVIATURA                   ,
    ORDEN_REFLECTANCIA
} ;

class CListaAsociaciones;
class CMineral
{
private:

public:
    bool                    m_bActivo; //indica si se tiene en cuenta el mineral a la hora de entrenar
    bool                    m_bClasificable; //indica si se tiene en cuenta el mineral a la hora de clasificar
    static enum_orden_mineral     m_enumOrden; //si false, se ordenará por abreviatura, static implica que solo hay una copia para todos los minerales
    CString                 m_csAbreviatura;
    CString                 m_csNombre;
    int                     m_nId; //identificador unico que se lee de fichero
    int                     m_nIndiceColor; //indice de la paleta de colores. se lee de fichero
    int                     m_RGB[3]; // color para mostrar a la hora de clasificar
    double                  m_arrReflectancias[NUM_BANDAS];           // promedio de las reflectancias de todas las zonas
double                  m_desviacion_tipica[NUM_BANDAS];           // desviacion tipica necesaria para comprobacion en proyecto EntrenaPixels
    double                  m_reflectanciaMedia;                      // reflectancia media de todas las bandas (usado para ordenar). Si -1 indicará que no hay datos en la base de datos y por tanto si se usará en clasificación
    double                  m_reflectanciaMediaMin;                      // reflectancia media de todas las bandas (usado para ordenar)
    double                  m_reflectanciaMediaMax;                      // reflectancia media de todas las bandas (usado para ordenar)
    Matriz                  m_matrizCovarianzaInv; // MATRIZ COVARIANZA calculada con los valores de las zonas
    vector<CMineralZona*>   m_list_zonas; //listado con detalles de todas las zonas de tomas/barridos de muestra (muchas entradas por mineral). Leido del archivo de recopilacion de datos
    vector<CMineralZona*>   m_list_zonas_test; //listado con detalles de zonas para pruebas. Leido del archivo de recopilacion de datos
    int                     m_nTotalPixels; //numero total de informacion de reflectancia de pixeles NO EXCLUIDOS POR LIMITE en todas las zonas (0 si solo hay informacion de zonas)
    CListaAsociaciones*     m_pAsociaciones; //lista de asociaciones minerales a la que pertenece este mineral
    bool                    m_bPerteneceAsociacionUsuario; //si el mineral ha sido incluido por el usuario en la asociacion personalizada

    CMineral()
    {
        m_bActivo = true;
        m_bClasificable = true;
        m_enumOrden = ORDEN_ABREVIATURA;
        m_nId = -1;
        memset(m_RGB,-1,sizeof(int)*3);
        m_reflectanciaMedia = -1;
        m_reflectanciaMediaMin = 0;
        m_reflectanciaMediaMax = 0;
        m_csAbreviatura.Empty();
        m_csNombre.Empty();
        memset(m_arrReflectancias,0,NUM_BANDAS*sizeof(double));
        memset(m_matrizCovarianzaInv,0,NUM_BANDAS*NUM_BANDAS*sizeof(double));
        m_list_zonas.clear();
        m_list_zonas_test.clear();
        m_nTotalPixels = 0;
        m_pAsociaciones = NULL;
        m_bPerteneceAsociacionUsuario = false;
    } 

    CMineral(CMineral const& rhs)
    {
        if (&rhs != this)
        {
            m_bActivo = rhs.m_bActivo;
            m_bClasificable = rhs.m_bClasificable;
            m_enumOrden = rhs.m_enumOrden;
            m_nId = rhs.m_nId;
            for (int i=2;i>=0;--i)
                m_RGB[i] = rhs.m_RGB[i] ; 
            m_csNombre = rhs.m_csNombre;
            m_csAbreviatura = rhs.m_csAbreviatura;
            m_reflectanciaMedia = rhs.m_reflectanciaMedia;
            m_reflectanciaMediaMin = rhs.m_reflectanciaMediaMin;
            m_reflectanciaMediaMax = rhs.m_reflectanciaMediaMax;
            for (i = 0; i< NUM_BANDAS;i++)
            {
                m_arrReflectancias[i] = rhs.m_arrReflectancias[i];
                for (int j=0; j< NUM_BANDAS;j++)
                    m_matrizCovarianzaInv[i][j] = rhs.m_matrizCovarianzaInv[i][j];
            }
            m_list_zonas.clear();
            m_list_zonas_test.clear();
            vector<CMineralZona*>::const_iterator it;
            for (it = rhs.m_list_zonas.begin(); it!=rhs.m_list_zonas.end();it++)
            {
                CMineralZona* pMineralesZonas = new CMineralZona;
                *pMineralesZonas = *(*it); //copiamos el objeto CMineralZona
                m_list_zonas.push_back(pMineralesZonas);
            }
            for (it = rhs.m_list_zonas_test.begin(); it!=rhs.m_list_zonas_test.end();it++)
            {
                CMineralZona* pMineralesZonas = new CMineralZona;
                *pMineralesZonas = *(*it);
                m_list_zonas_test.push_back(pMineralesZonas);
            }

            m_nTotalPixels = rhs.m_nTotalPixels;
            m_pAsociaciones = rhs.m_pAsociaciones;
            m_bPerteneceAsociacionUsuario = rhs.m_bPerteneceAsociacionUsuario;
        }
    }

    virtual ~CMineral()
    {
        vector<CMineralZona*>::iterator it;
        for (it = m_list_zonas.begin(); it!=m_list_zonas.end();it++)
        {
            delete (*it);
        }
        m_list_zonas.clear();

        for (it = m_list_zonas_test.begin(); it!=m_list_zonas_test.end();it++)
        {
            delete (*it);
        }
        m_list_zonas_test.clear();
    }

    CMineral& operator =(const CMineral& rhs)
    {
        if (&rhs != this)
        {
            m_bActivo = rhs.m_bActivo;
            m_bClasificable = rhs.m_bClasificable;
            m_enumOrden = rhs.m_enumOrden;
            m_nId = rhs.m_nId;
            for (int i=2;i>=0;--i)
                m_RGB[i] = rhs.m_RGB[i] ; 
            m_csNombre = rhs.m_csNombre;
            m_csAbreviatura = rhs.m_csAbreviatura;
            m_reflectanciaMedia = rhs.m_reflectanciaMedia;
            m_reflectanciaMediaMin = rhs.m_reflectanciaMediaMin;
            m_reflectanciaMediaMax = rhs.m_reflectanciaMediaMax;
            for (i = 0; i< NUM_BANDAS;i++)
            {
                m_arrReflectancias[i] = rhs.m_arrReflectancias[i];
                for (int j=0; j< NUM_BANDAS;j++)
                    m_matrizCovarianzaInv[i][j] = rhs.m_matrizCovarianzaInv[i][j];
            }
            m_list_zonas.clear();
            m_list_zonas_test.clear();
            vector<CMineralZona*>::const_iterator it;
            for (it = rhs.m_list_zonas.begin(); it!=rhs.m_list_zonas.end();it++)
            {
                CMineralZona* pMineralesZonas = new CMineralZona;
                *pMineralesZonas = *(*it); //copiamos el objeto CMineralZona
                m_list_zonas.push_back(pMineralesZonas);
            }
            for (it = rhs.m_list_zonas_test.begin(); it!=rhs.m_list_zonas_test.end();it++)
            {
                CMineralZona* pMineralesZonas = new CMineralZona;
                *pMineralesZonas = *(*it);
                m_list_zonas_test.push_back(pMineralesZonas);
            }

            m_nTotalPixels = rhs.m_nTotalPixels;
            m_pAsociaciones = rhs.m_pAsociaciones;
            m_bPerteneceAsociacionUsuario = rhs.m_bPerteneceAsociacionUsuario;
        }

        return *this;
    }

    bool operator==(const CMineral& rhs) const
    {
        return (m_nId == rhs.m_nId) ? true : false;
    }

    bool operator<(const CMineral& rhs) const
    {
        if (m_enumOrden == ORDEN_REFLECTANCIA)
            return (m_reflectanciaMedia < rhs.m_reflectanciaMedia) ? true : false;
        else if (m_enumOrden == ORDEN_ABREVIATURA)
            return (m_csAbreviatura < rhs.m_csAbreviatura) ? true : false;
        else
            return (m_nId < rhs.m_nId) ? true : false;
    }    

    bool operator>(const CMineral& rhs) const
    {
        if (m_enumOrden == ORDEN_REFLECTANCIA)
            return (m_reflectanciaMedia > rhs.m_reflectanciaMedia) ? true : false;
        else if (m_enumOrden == ORDEN_ABREVIATURA)
            return (m_csAbreviatura > rhs.m_csAbreviatura) ? true : false;
        else
            return (m_nId > rhs.m_nId) ? true : false;
    }
};

//////////////////////////////////////////////////////////////////////
//
// CMinerales - Contenedor de minerales e interfaz
//
//////////////////////////////////////////////////////////////////////

class CMinerales 
{
    friend class CEntrenaPixels;
public:
	BYTE                m_lut_clasificacion[256][3]; //valores RGB para cada indice de mineral para mostrar en pantalla
    vector<CMineral*>   m_list; //listado de minerales (una entrada por mineral). Lista de minerales leido de archivo de minerales y espectros calculados a partir de m_list_zonas
    bool                m_bAlgunMineralClasificable; // Si hay algun mineral seleccionado para clasificacion (en el dialogo de asociaciones minerales) 
private:
    int m_arrInicioBusqueda[NUM_INDICES_BUSQUEDA]; //por cada valor de gris, indice de mineral (ordenado por reflectancia media) minimo por el que buscar en clasificacion
    int m_arrFinBusqueda[NUM_INDICES_BUSQUEDA];    //por cada valor de gris, indice de mineral (ordenado por reflectancia media) maximo por el que buscar en clasificacion
    int m_nBandas; //se leera del propio fichero de entrenamiento
public:
	CMinerales();
	virtual ~CMinerales();
    bool CargarMinerales(CString csFichero = "");
    bool GuardarMinerales(CString csFichero = "");
    int CargarPixels(CString csFicheroPixels, bool bPruebas, FILE* log);
    bool CargarEntrenamiento(CString csFicheroEntrenamiento, int nBandasConfiguradas);
    void CMinerales::CalcularIndicesBusqueda();
    int GetCount();
    CString GetNombre(int nIndex);
    int GetId(int nIndex = -1);
    int GetIndiceColor(int nIndex);
    CString GetAbreviatura(int nIndex);
    CString GetAbreviatura(CString csNombre);
    CMineral* BuscarPorAbreviatura(CString csAbreviatura);
    int GetIndex(int nId);
    int GetIndexConIndiceColor(int nId);
    int CMinerales::VerificaIndiceColor(int nIndiceColorOriginal, int nIdPropio = -1);
    void Identificar(double* arrReflectancias, list<CResultadoIdentificacion>& list_resultados) ;
    void Identificar(double* arrReflectancias, CResultadoIdentificacion& primero, CResultadoIdentificacion& segundo);
//    bool ComparadorMinerales(CMineral* mineral1,CMineral* mineral2);
    double MinimumDistance(double* arrObjetivo, double* arrReferencia); //publico para comprobar zonas en CEntrenaPixels::CalcularPromedioZonas
void Pruebas();
void CMinerales::ImprimirMinerales();
private:
//    bool CargarZonas(CString csFichero, bool bPruebas = false);
    bool CargarZonasPruebas(CString csFichero);
    double SpectralAngleMapper(double* arrObjetivo, double* arrReferencia);
    double Similarity(double* arrObjetivo, double* arrReferencia);
    double MahalanobisMatriz(double* arrObjetivo, double* arrReferencia, Matriz matrizCovarianza);
    double Mahalanobis(double* arrObjetivo, double* arrReferencia, double* arrDesviacionInf, double* arrDesviacionSup, bool bLaterales);
    double Mahalanobis2(double* arrObjetivo, double* arrReferencia, double* arrVarianza);
    void ImprimirResultadoIdentificacion(
        FILE* archivo, 
        CString csAbreviatura, 
        int nCountZonas, 
        int nZona,       //identificador unico de zona (independiente de mineral)
        int nCountDato, 
        bool bCorrecto, 
        list<CResultadoIdentificacion>::iterator it_begin, 
        list<CResultadoIdentificacion>::iterator it_end);
};


#endif // !defined(AFX_AREAS_H__63BC2382_C54E_4277_85DC_BE8E2E2D41A4__INCLUDED_)
