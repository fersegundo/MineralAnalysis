// Areas.h: interface for the CAreas class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_AREAS_H__63BC2382_C54E_4277_85DC_BE8E2E2D41A4__INCLUDED_)
#define AFX_AREAS_H__63BC2382_C54E_4277_85DC_BE8E2E2D41A4__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include <list>
#include "defines.h"
#include "analisis.h"
#include "minerales.h"

extern CAnalisisApp theApp;

using namespace std;

class CArea
{
private:

public:

    int             m_nId;
    int             m_nIdEspectros;  // id linea espectros
    // valores espectrales para esta area (no pancromatico)
    double          m_arrEspectros[MAX_NUM_IMAGENES]; 
    // valores de percentil para esta area (no pancromatico)
    double          m_arrPercentilInf[MAX_NUM_IMAGENES]; 
    double          m_arrPercentilSup[MAX_NUM_IMAGENES]; 
    // histogramas (uno por cada espectro, no pancromatico). Necesario para salvar estado.
    double          m_arrHistogramas[MAX_NUM_IMAGENES][NUM_VALORES_HISTO]; 
    // id lineas graficas histograma (tantas como filtros maximos)
    //Ojo: se usa MAX_NUM_IMAGENES pero puede que haya menos filtros.
    //En ese caso se quedará inicializado a 0 y no se usarán
    int             m_arrIdHistograma[MAX_NUM_IMAGENES]; 
    CString         m_csNombre;
    CRect           m_rect;
    CString         m_csMineral;
    CString         m_csMineralAbreviatura;
    CString         m_csCalidad;
    CString         m_csComentario;
    int             m_nColorIndex;
    list<CResultadoIdentificacion> m_listIdentificacion; //para mantener la lista de minerales identificados

    CArea()
    {
        m_nId = -1;
        m_nIdEspectros = -1;
        for (int i = 0; i< MAX_NUM_IMAGENES;i++)
        {
            m_arrIdHistograma[i] = 0;
            m_arrEspectros[i]    = 0;
            m_arrPercentilInf[i]    = 0;
            m_arrPercentilSup[i]    = 0;
            for (int j=0;j<NUM_VALORES_HISTO;j++)
                m_arrHistogramas[i][j] = 0;
        }
        m_csNombre.Empty();
        m_csMineralAbreviatura = SELECCIONE_MINERAL_ABR;
        m_csMineral.Format(SELECCIONE_MINERAL);
        m_csCalidad.Format(CALIDAD_SIN_VALORAR);

        m_csComentario.Empty();
        m_rect.left           = 0;
        m_rect.top            = 0;
        m_rect.right          = 0;
        m_rect.bottom         = 0;
        m_nColorIndex       = 0;
    } 

    // Compone el nombre del area (a partir de la abreviatura y la calidad)
    // almacenandolo y devolviendolo
    CString CArea::ComponerNombre()
    {
        m_csNombre = m_csMineralAbreviatura;
        m_csNombre += " ";
        if (m_csCalidad ==  CALIDAD_A)
            m_csNombre += "A";
        else if (m_csCalidad ==  CALIDAD_B)
            m_csNombre += "B";
        else if (m_csCalidad ==  CALIDAD_C)
            m_csNombre += "C";
        else if (m_csCalidad ==  CALIDAD_D)
            m_csNombre += "D";
        else if (m_csCalidad ==  CALIDAD_E)
            m_csNombre += "E";
        else 
            m_csNombre += CALIDAD_SIN_VALORAR_ABR;

        return m_csNombre;
    }

    void CArea::CopiarEspectro(double* arrEspectro)
    {
        for (int i = 0; i<theApp.m_ParamIni.nBandas-1;i++)//valores espectrales. pancromatico no
            m_arrEspectros[i] = arrEspectro[i];

        // El resto de filtros no usados a 0
        for (int j = i; j<MAX_NUM_IMAGENES;j++)
            m_arrEspectros[j] = 0;
    }

    void CArea::CopiarPercentilInf(double* arrPercentilInf)
    {
        for (int i = 0; i<theApp.m_ParamIni.nBandas-1;i++)//valores espectrales. pancromatico no
            m_arrPercentilInf[i] = arrPercentilInf[i];

        // El resto de filtros no usados a 0
        for (int j = i; j<MAX_NUM_IMAGENES;j++)
            m_arrPercentilInf[j] = 0;
    }

    void CArea::CopiarPercentilSup(double* arrPercentilSup)
    {
        for (int i = 0; i<theApp.m_ParamIni.nBandas-1;i++)//valores espectrales. pancromatico no
            m_arrPercentilSup[i] = arrPercentilSup[i];

        // El resto de filtros no usados a 0
        for (int j = i; j<MAX_NUM_IMAGENES;j++)
            m_arrPercentilSup[j] = 0;
    }

    void CArea::GetEspectro(double* arrEspectro)
    {
        ASSERT(MAX_NUM_IMAGENES >= theApp.m_ParamIni.nBandas-1);
        for (int i = 0; i<theApp.m_ParamIni.nBandas-1;i++)//valores espectrales. pancromatico no
            arrEspectro[i] = m_arrEspectros[i];
    }

    void CArea::GetPercentilInf(double* arrPercentilInf)
    {
        ASSERT(MAX_NUM_IMAGENES >= theApp.m_ParamIni.nBandas-1);
        for (int i = 0; i<theApp.m_ParamIni.nBandas-1;i++)//valores espectrales. pancromatico no
            arrPercentilInf[i] = m_arrPercentilInf[i];
    }

    void CArea::GetPercentilSup(double* arrPercentilSup)
    {
        ASSERT(MAX_NUM_IMAGENES >= theApp.m_ParamIni.nBandas-1);
        for (int i = 0; i<theApp.m_ParamIni.nBandas-1;i++)//valores espectrales. pancromatico no
            arrPercentilSup[i] = m_arrPercentilSup[i];
    }

    // nEspectro = indice [0...nBandas-2] (no incluye pancromatico)
    void CArea::CopiarHistograma(double* arrHistograma, int nEspectro)
    {
        int i = 0;

        if (arrHistograma != NULL)
        {
            for (i = 0; i<NUM_VALORES_HISTO;i++)
                m_arrHistogramas[nEspectro][i] = arrHistograma[i];
        }
        else
        {
            // indicar que este espectro no esta disponible
            m_arrHistogramas[nEspectro][0] = -1;
        }

    }

    // nEspectro = indice [0...nBandas-2] (no incluye pancromatico)
    void CArea::GetHistograma(double* arrHistograma, int nEspectro)
    {
        int i = 0;

        if (arrHistograma != NULL)
        {
            if (m_arrHistogramas[nEspectro][0] != -1)
            {
                for (i = 0; i<NUM_VALORES_HISTO;i++)
                    arrHistograma[i] = m_arrHistogramas[nEspectro][i];
            }
            else
            {
                // indicar que este espectro no esta disponible
                arrHistograma = NULL;
            }

        }
    }

    void CArea::CopiarIdHistogramas(int* arrIdHistogramas)
    {
        for (int i = 0; i< theApp.m_ParamIni.nBandas;i++) 
            m_arrIdHistograma[i] = arrIdHistogramas[i];

        // El resto de filtros no usados a 0
        for (int j = i; j<MAX_NUM_IMAGENES;j++)
            m_arrIdHistograma[j] = 0;
    }

    void CArea::SetIdEspectros(int nIdEspectros)
    {
        m_nIdEspectros = nIdEspectros;
    }

    // copy constructor
    CArea(const CArea& rhs)
        : m_nId(rhs.m_nId), m_csNombre(rhs.m_csNombre), m_csMineral(rhs.m_csMineral), 
        m_csCalidad(rhs.m_csCalidad), m_csComentario(rhs.m_csComentario), 
        m_csMineralAbreviatura(rhs.m_csMineralAbreviatura)
    {
        m_rect.left = rhs.m_rect.left;
        m_rect.top = rhs.m_rect.top;
        m_rect.right = rhs.m_rect.right;
        m_rect.bottom = rhs.m_rect.bottom;
        m_nColorIndex = rhs.m_nColorIndex;
        m_nIdEspectros = rhs.m_nIdEspectros;
        for (int i = 0; i< MAX_NUM_IMAGENES;i++)
        {
            m_arrIdHistograma[i] = rhs.m_arrIdHistograma[i];
            m_arrEspectros[i] = rhs.m_arrEspectros[i];
            m_arrPercentilInf[i] = rhs.m_arrPercentilInf[i];
            m_arrPercentilSup[i] = rhs.m_arrPercentilSup[i];

            for (int j=0;j<NUM_VALORES_HISTO;j++)
                m_arrHistogramas[i][j] = rhs.m_arrHistogramas[i][j];
        }

    }

    virtual ~CArea(){}

    CArea& operator =(const CArea& rhs)
    {
        if (&rhs != this)
        {
            m_nId = rhs.m_nId;
            m_csNombre = rhs.m_csNombre;
            m_csMineral = rhs.m_csMineral;
            m_csCalidad = rhs.m_csCalidad;
            m_csComentario = rhs.m_csComentario;
            m_csMineralAbreviatura = rhs.m_csMineralAbreviatura;
            m_rect.left = rhs.m_rect.left;
            m_rect.top = rhs.m_rect.top;
            m_rect.right = rhs.m_rect.right;
            m_rect.bottom = rhs.m_rect.bottom;
            m_nColorIndex = rhs.m_nColorIndex;
            m_nIdEspectros = rhs.m_nIdEspectros;
            for (int i = 0; i< MAX_NUM_IMAGENES;i++)
            {
                m_arrIdHistograma[i] = rhs.m_arrIdHistograma[i];
                m_arrEspectros[i] = rhs.m_arrEspectros[i];
                m_arrPercentilInf[i] = rhs.m_arrPercentilInf[i];
                m_arrPercentilSup[i] = rhs.m_arrPercentilSup[i];
                for (int j=0;j<NUM_VALORES_HISTO;j++)
                    m_arrHistogramas[i][j] = rhs.m_arrHistogramas[i][j];
            }
        }

        return *this;
    }

    bool operator==(const CArea& rhs) const
    {
        return (m_nId == rhs.m_nId) ? true : false;
    }

    bool operator<(const CArea& rhs) const
    {
        return (m_csNombre < rhs.m_csNombre) ? true : false;
    }    

    bool operator>(const CArea& rhs) const
    {
        return (m_csNombre > rhs.m_csNombre) ? true : false;
    }
};

enum enum_tipo_fichero{
    AREAS_INFO              = 0,
    AREAS_CLASIFICACION,
    AREAS_PIXELS
} ;

class ControlImagenes;
class CAnalisisDlg;
class CAreas 
{
    int m_CurrentColorIndex;
    long listaR[NUM_COLORES];
    long listaG[NUM_COLORES];
    long listaB[NUM_COLORES];
    list<CArea> m_list;
public:
	CAreas();
	virtual ~CAreas();
    CArea* NuevaArea(CRect& rect);
    void GetCurrentColor(long& r, long& g, long& b);
    int GetCurrentColorIndex();
    CRect Eliminar(int nId);
    void EliminarTodas();
    CArea* GetArea(int nId);
    void GetRGB(CArea& area, long& r, long& g, long& b);
    void GetRGB(int nId, long& r, long& g, long& b);
    void SetText(int nId,TCHAR* pszText);
    bool Guardar(CString csCampo, bool bFormato2d, ControlImagenes& controlImagenes);
    void GuardarReducido(FILE* archivo, int nCampo);
    bool Cargar(CString csCampo, CAnalisisDlg* pDialogo);
private:
    void GuardarArea(CArea& area, FILE* archivo);
    bool Guardar(CString csCampo, CString csMuestra, CString csMuestraCampo, CString csFila, ControlImagenes& controlImagenes, enum_tipo_fichero enumTipoFichero);
    void GuardarClasificacionArea(CArea& area, FILE* archivo, ControlImagenes& controlImagenes);
    void GuardarPixelsArea(CArea& area, FILE* archivo, ControlImagenes& controlImagenes);
    void CargarArea(char * strCount, CArea& area, int numEspectros, int numValHisto);
};

#endif // !defined(AFX_AREAS_H__63BC2382_C54E_4277_85DC_BE8E2E2D41A4__INCLUDED_)
