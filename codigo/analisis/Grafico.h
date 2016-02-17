// Grafico.h: interface for the CGrafico class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_GRAFICO_H__D019E130_ED1F_4241_AF67_D781908EAF3F__INCLUDED_)
#define AFX_GRAFICO_H__D019E130_ED1F_4241_AF67_D781908EAF3F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "PGL.h" //PlotGraphicLibrary

#define LINE_WIDTH 0.5
#define LINE_WIDTH_VER 5

using namespace Gdiplus;

class CGrafico  
{
    CWnd*       m_parentWindow;
    CPGLGraph*  m_pGraph; //grafico
    int         m_nPoints; // numero de puntos del eje x
    int         m_nX_Min; // valor mas bajo eje x
    double      m_dX_Step; // distancia entre valores eje x
    // puntos para la linea que necesitamos unicamente para que el grafico tome las dimensiones adecuadas
    double*     m_pY_Dummy; UINT m_id_Dummy;     
    int         m_id_LineaVertical; //identificador de la linea vertical que indica la banda seleccionada

	gfxinterface::CGDIpInterface m_GDIpInterface; //necesario para repintar el grafico

public:
	CGrafico();
	virtual ~CGrafico();
    void Inicializar(CWnd* parentWindow, int nPoints, int nY_Max, int nX_Min, double nX_Step, char* csTitle, char* csLabelX, char* csLabelY );

    void MuestraGrafico();
    UINT NuevaLinea(double arrValores[],int r, int g, int b);
    bool EliminarLinea(UINT nId);
    void NuevaLineaVertical(double dX,int r, int g, int b);
    void EliminarLineaVertical();
    void EliminarLineas();
    void MostrarLinea(UINT nId, bool bMostrar);
    void MostrarTodas(bool bMostrar);
};

#endif // !defined(AFX_GRAFICO_H__D019E130_ED1F_4241_AF67_D781908EAF3F__INCLUDED_)
