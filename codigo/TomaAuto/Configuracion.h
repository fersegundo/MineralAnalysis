
#if !defined(CONFIGURACION_H_)
#define CONFIGURACION_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define FICHERO_CONFIGURACION "parametros_TomaAuto.ini"

#include "defines.h"

//using namespace std;

#define MIN_ESCALA_REFLECTANCIA 0.01
#define MAX_ESCALA_REFLECTANCIA 999 //desactivamos proteccion

class CConfiguracion 
{
public:
    CString m_csDirectorioBarrido;
    CString m_csDirectorioToma;
    int     m_nDimension_barrido_x;
    int     m_nDimension_barrido_y;
    int     m_nDimension_toma_x;
    int     m_nDimension_toma_y;
    double  m_ratio_inicial; //de la imagen
    double  m_dEscalaReflectancia; //escala relectancia-nivel de gris con la que se guardaran las imagenes adquiridas
    int     m_nBits; //bits de salida de las imagenes adquiridas
    int     m_nImagenAcum; // numero de veces que se promedia cada campo en la adquisición

public:
	CConfiguracion();
	virtual ~CConfiguracion();
    bool Cargar();
    bool Guardar();
};

#endif 
