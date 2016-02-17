
#if !defined(CLASIFICACION_BARRIDO_H)
#define CLASIFICACION_BARRIDO_H

#include "Clasificacion.h"

class CClasificaBarridoDlg;
class CAsociacionesBarridoDlg;
class ClasificacionBarrido
{
    BYTE*             m_bufBarrido;   // imagen clasificada
    Clasificacion     m_clasificacion;
    unsigned int*     m_arrCount; // contador de pixels identificados por cada mineral
    unsigned int*     m_arrCountAcum; // contador de pixels identificados por cada mineral. Acumulados

    // buffers mil necesarios para mostrar o guardar las imagenes en falso color (usados en ambas pasadas)
    MIL_ID M_Clasificacion    ;
    MIL_ID M_lut_clasificacion;

public:
    ClasificacionBarrido();
    virtual ~ClasificacionBarrido();
    ClasificacionBarrido::Init();

    void ClasificacionBarrido::PrimeraPasada(double dUmbralDistancia, double dUmbralConfiabilidad,
                                         CString csMuestra, 
                                         CAsociacionesBarridoDlg* pDialogo = NULL);
    void ClasificacionBarrido::SegundaPasada(double dUmbralDistancia, double dUmbralConfiabilidad, double dRefMin,
                                         CString csMuestra,
                                         CClasificaBarridoDlg* pDialogo = NULL);

private:
    void ClasificacionBarrido::LiberarMemoria();
    static UINT ClasificacionBarrido::PrimeraPasadaThread( LPVOID Param );
    static UINT ClasificacionBarrido::SegundaPasadaThread( LPVOID Param );
};

#endif 
