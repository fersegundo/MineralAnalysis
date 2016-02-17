#if !defined(CLASIFICACION_H)
#define CLASIFICACION_H

#include "defines.h"
#include "mil.h"
#include "minerales.h"

class CMinerales;
class CMineralClasificado;
class Clasificacion
{
    CMinerales*     m_pMinerales;
    MIL_ID M_lut_clasificacion;// Establece los valores RGB para cada indice de mineral para mostrar en pantalla


    MIL_ID* M_pClasificacion      ;
    MIL_ID* M_pConfiabilidad;
    MIL_ID* M_pDistancia;

    unsigned short** m_buffImagenesClasificacion; //las imagenes a cargar

    BYTE* m_bufClasificacion;   //buffer para la imagen de salida (clasificada)
    BYTE* m_bufConfiabilidad;   //buffer para los valores de confiabilidad en cada pixel
    BYTE* m_bufDistancia;       //buffer para los valores de distancia en cada pixel
    int m_nNumBandas;
    int m_nAnchoImagen;
    int m_nAltoImagen ;
public:
    Clasificacion();
    virtual ~Clasificacion();
    void Liberar();

    void SetMinerales(CMinerales* pMinerales);
    bool Clasificar(list<CMineralClasificado> &listClasificados, unsigned int& nCountOscuros, double dEscalaReflectancia, int nBitsProfundidad, double dRefMin = 0);
    bool Clasificacion::Clasificar(unsigned int* arrMinerales,  unsigned int* arrMineralesAcum, unsigned int& nCountOscuros, unsigned int& nCountNoClasificados, 
                                    BYTE* arrClasificados, 
                                    double dDistancia, double dConfiabilidad,
                                    double dEscalaReflectancia, int nBitsProfundidad,
                                    double dRefMin = 0);
    void ClasificacionSelectiva(double dUmbralConfiabilidad,double dUmbralDistancia,
                                               int nInfDer,int nInfIzq,int nSupDer,int nSupIzq,
                                               BYTE* bufPruebas);
    void ClasificacionSelectiva(double dUmbralConfiabilidad,double dUmbralDistancia,
                                               unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq);
    void ClasificacionSelectiva(double dUmbralConfiabilidad,double dUmbralDistancia,
                                               int nInfDer,int nInfIzq,int nSupDer,int nSupIzq,
                                               unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq,
                                               BYTE* bufPruebas);
    void ClasificacionSelectivaLog(double dUmbralConfiabilidad,double dUmbralDistancia,
                                                    list<CMineralClasificado> &listClasificados, unsigned int nCountOscuros,
                                                    unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq,
                                                    CString csFicheroLog);    
    void ClasificacionSelectivaLog(double dUmbralConfiabilidad,double dUmbralDistancia,
                                                    unsigned int nCountOscuros,
                                                    unsigned int* arrInfDer, unsigned int* arrInfIzq,unsigned int* arrSupDer,unsigned int* arrSupIzq,
                                                    CString csFicheroLog)    ;
    bool InicializaDisplay(CStatic*	pDisplay, MIL_ID& Mildisplay, MIL_ID& M_Clasificacion,MIL_ID& M_Confiabilidad,MIL_ID& M_Distancia);
    bool CargarImagenes(MIL_ID*& arrImagenesMIL, CString csNombreMuestra, int* arrBandas, int nNumBandas, CRect* pRect);
    bool CargarImagenes(MIL_ID* M_Muestra, int nNumBandas, CRect* pRect);
    int Clasificacion::GetAnchoImagen();
    int Clasificacion::GetAltoImagen();
    void GuardarClasificar(CString csNombreFichero);

private:

};

#endif 
