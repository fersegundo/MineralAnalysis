
#if !defined(ASOCIACIONES_MINERALES_H)
#define ASOCIACIONES_MINERALES_H

#define FICHERO_DEFECTO_ASOCIACIONES     "asociaciones_def.csv"

// ASOCIACIONES MINERALES, valores aparicion y porcentajes
#define AS_MIN_NUM                              6 //numero total de valores de posibilidad
#define AS_MIN_AUSENTE                          0
#define AS_MIN_ABUNDANTE                        1
#define AS_MIN_ABUNDANTE_CODIFICADO            11 // numero primo, para codificar en un solo entero diferentes cantidades
#define AS_MIN_ABUNDANTE_PORCENTAJE_MIN        0.15
#define AS_MIN_PRINCIPAL                        2
#define AS_MIN_PRINCIPAL_CODIFICADO            7// numero primo, para codificar en un solo entero diferentes cantidades
#define AS_MIN_PRINCIPAL_PORCENTAJE_MIN        0.05
#define AS_MIN_ACCESORIO                        3
#define AS_MIN_ACCESORIO_CODIFICADO            5// numero primo, para codificar en un solo entero diferentes cantidades
#define AS_MIN_ACCESORIO_PORCENTAJE_MIN        0.005
#define AS_MIN_ESCASO                           4
#define AS_MIN_ESCASO_CODIFICADO               3// numero primo, para codificar en un solo entero diferentes cantidades
#define AS_MIN_ESCASO_PORCENTAJE_MIN           0.001
#define AS_MIN_ESPORADICO                       5
#define AS_MIN_ESPORADICO_CODIFICADO           2// numero primo, para codificar en un solo entero diferentes cantidades
#define AS_MIN_ESPORADICO_PORCENTAJE_MIN       0.0

// Entrada (fila) de la tabla de asociaciones, es decir lista de asociaciones para un mineral
class CListaAsociaciones
{
public:
    unsigned __int8*        m_lista;

    CListaAsociaciones::CListaAsociaciones()
    {
        m_lista    = NULL;
    }

    CListaAsociaciones::~CListaAsociaciones()
    {
        if (m_lista)
            delete [] m_lista;
    }
};

class CMinerales;
class CAsociacionesMinerales
{
public:
    int                     m_nAsociaciones;
    CString*                m_arrNombresAsociaciones; //lista con el nombre de las asociaciones
    CString*                m_arrDescripcionAsociaciones; //lista con la descricion detallada de cada asociacion
    int*                    m_arrCompatibilidadAsociaciones; //lista con el estado de compatibilidad del barrido actual (0 si no es compatible)
    bool                    m_bAsociacionUsuario;            // estado de compatibilidad de la asociacion personalizada por el usuario

private:
    CMinerales*             m_pMinerales; //para manejar la lista comun de minerales y actualizar el puntero a la lista de asociaciones

public:
    CAsociacionesMinerales();
    virtual ~CAsociacionesMinerales();

    void CAsociacionesMinerales::Init(CMinerales* pMinerales); //retiene el puntero a la clase Minerales
    void CAsociacionesMinerales::Liberar(); //llamar cuando no se usen mas las asociaciones. Libera las asociaciones minerales y desasocia los punteros de la clase Minerales
    bool CAsociacionesMinerales::CargarFichero(CString csFichero);
    bool CAsociacionesMinerales::CargarFicheroAsociacionesSeleccionadas(CString csFichero);
    bool CAsociacionesMinerales::GuardarFicheroAsociacionesSeleccionadas(CString csFichero);
    bool CAsociacionesMinerales::FiltrarAsociaciones(unsigned int* arrOriginal, int* arrAsocCompatibles);
    void CAsociacionesMinerales::ActualizarMineralesActivosPorAsociaciones();

private:
    unsigned __int8 CAsociacionesMinerales::dividir (unsigned int *arrOriginal, unsigned int* arrOrdenado, int inicio, int fin);
    void CAsociacionesMinerales::quicksort( unsigned int *arrOriginal, unsigned int* arrOrdenado, int inicio, int fin)  ;

};

#endif 
