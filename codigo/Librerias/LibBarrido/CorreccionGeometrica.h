
#if !defined(CORRECCION_GEOMETRICA_H_)
#define CORRECCION_GEOMETRICA_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "..\ProcesoAux\estructuras.h"

#define FICH_PARAM_CORRECCION_GEOMETRICA			"parametros_correccion_geometrica"
#define ESCALA_TEORICA_20x   0.324 //escala que deben tener las imagenes con un objetivo 20x (si es otro se corregirá en base a este valor). Se escribe en el fichero al calibrar. Al corregir se usa la del fichero

// Polka dot
#define NUM_CUADRADOS                           4
#define DISTANCIA_ENTRE_CUADRADOS               150
#define PORCENTAJE_CUADRADOS_RESPECTO_FONDO     50
#define AREA_MINIMA_CUADRADO                    1200 //En pixels (1600 seria aproximadamente el area de un cuadrado con un objetivo de 2.5x)
#define PIXELS_BORDE_EXCLUSION                  0   //pixels de holgura en los extremos de la imagen a la hora de excluir blobs que esten demasiado cerca del borde de la imagen
#define BANDA_PARA_ROTACION                     4

class DtaRevolver;
class CCorreccionGeometrica 
{
public:
    double  m_dEscalaObjetivo;
private:
    double* m_arrPosicionX;
    double* m_arrPosicionY;
    double* m_arrEscala;
    double  m_dRotacion; // en grados
    int     m_nNumCuadradosX;
public:
	CCorreccionGeometrica();
	virtual ~CCorreccionGeometrica();
    int CCorreccionGeometrica::CalcularAnchoMax(int nAnchoImagen, int nAltoImagen, int nBandas, bool bRotacion = true);
    int CCorreccionGeometrica::CalcularAltoMax(int nAnchoImagen, int nAltoImagen, int nBandas, bool bRotacion = true);
    bool ProcesarPolka(parametros* pParamIni,DtaRevolver& Rueda, MIL_ID overlay = NULL);
    bool CCorreccionGeometrica::ProcesarEscalaTranslacion(parametros* pParamIni,DtaRevolver& Rueda, MIL_ID overlay);
    bool CCorreccionGeometrica::ProcesarRotacion(parametros* pParamIni,DtaRevolver& Rueda, MIL_ID overlay);
    bool Guardar(parametros& paramIni);
    bool Leer(parametros& paramIni);
    bool CorregirImagenes(MIL_ID* M_arrImagenes, parametros &param, bool bCorregirRotacion = true);
private:
    bool ProcesarPolkaCampo(parametros* pParamIni,
                            int nAnchoImagen, int nAltoImagen,
                            double arrCentroX[],
                            double arrCentroY[],
                            double arrEscala[],
                            int nBandas,
                            /*OUT*/ int& nNumCuadrados, 
                            MIL_ID overlay = NULL);
    void CCorreccionGeometrica::ObtenerCuadradoProximoA(double dInicioX, double dInicioY,
                                 double& dCercanoX, double& dCercanoY);
    long CCorreccionGeometrica::EncontrarBlobs(int nBanda, MIL_ID MilBlobResult);
    bool CalcularCentrosCuadrados(int nBanda, 
                                 double x[], double y[], 
                                 double &dCentroX, double &dCentroY, 
                                 double& dEscalaAproximada, 
                                 MIL_ID overlay = NULL);
};

#endif 
