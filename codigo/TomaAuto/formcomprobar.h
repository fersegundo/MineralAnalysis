#if !defined(AFX_CFormComprobar_H__BBF699CF_5E25_4EC7_86A5_CDEBA92EB0A7__INCLUDED_)
#define AFX_CFormComprobar_H__BBF699CF_5E25_4EC7_86A5_CDEBA92EB0A7__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormComun.h"
#include "EasySize.h"

/////////////////////////////////////////////////////////////////////////////
// CFormComprobar dialog
/////////////////////////////////////////////////////////////////////////////
class CFormComprobar : public CFormComun
{
DECLARE_EASYSIZE
private:
    long                m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    long                m_anchoTitulo, m_altoTitulo; //Necesario para dimensionar padre (dialogo con menu y barra)
    CFont m_FontNormal;
    CFont m_FontNegrita;
// Construction
public:
	CFormComprobar(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	CStatic	m_nanometros;
	CStatic	m_longitud_onda;
	CStatic	m_filtro_actual;
	//{{AFX_DATA(CFormComprobar)
	enum { IDD = IDD_FORM_COMPROBAR };
	CStatic	m_staticRecalibrar;
	CStatic	m_staticOk;
	CStatic	m_icon_exclamation;
	CStatic	m_icon_check;
	CStatic	m_staticError;
	CStatic	m_staticFiltro;
	CStatic	m_staticRefl;
	CStatic	m_staticReflTeorica;
	CButton	m_statucResultados;
	CStatic	m_static953;
	CStatic	m_static952;
	CStatic	m_static951;
	CStatic	m_static950;
	CStatic	m_static903;
	CStatic	m_static902;
	CStatic	m_static901;
	CStatic	m_static900;
	CStatic	m_static853;
	CStatic	m_static852;
	CStatic	m_static851;
	CStatic	m_static850;
	CStatic	m_static803;
	CStatic	m_static802;
	CStatic	m_static801;
	CStatic	m_static800;
	CStatic	m_static753;
	CStatic	m_static752;
	CStatic	m_static751;
	CStatic	m_static750;
	CStatic	m_static703;
	CStatic	m_static702;
	CStatic	m_static701;
	CStatic	m_static700;
	CStatic	m_static653;
	CStatic	m_static652;
	CStatic	m_static651;
	CStatic	m_static650;
	CStatic	m_static603;
	CStatic	m_static602;
	CStatic	m_static601;
	CStatic	m_static600;
	CStatic	m_static553;
	CStatic	m_static552;
	CStatic	m_static551;
	CStatic	m_static550;
	CStatic	m_static503;
	CStatic	m_static502;
	CStatic	m_static501;
	CStatic	m_static500;
	CStatic	m_static453;
	CStatic	m_static452;
	CStatic	m_static451;
	CStatic	m_static450;
	CStatic	m_static403;
	CStatic	m_static402;
	CStatic	m_static401;
	CStatic	m_static400;
	CStatic	m_static3003;
	CStatic	m_static3002;
	CStatic	m_static3001;
	CStatic	m_static3000;
	CStatic	m_static2003;
	CStatic	m_static2002;
	CStatic	m_static2001;
	CStatic	m_static2000;
	CStatic	m_static103;
	CStatic	m_static102;
	CStatic	m_static101;
	CStatic	m_static1003;
	CStatic	m_static1002;
	CStatic	m_static1001;
	CStatic	m_static1000;
	CStatic	m_static100;
	CEdit	m_ratio;
	CStatic	m_pos_x;
	CStatic	m_pos_z;
	CStatic	m_pos_y;
	CStatic	m_imagen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormComprobar)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormComprobar)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEscalaVentana();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnComprobar();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    static UINT ComprobarThread( LPVOID Param );
    void CalculaZoom(long anchoDisplay,long altoDisplay);
    double ActualizaZoom();

    void CFormComprobar::MostrarFiltroResultado(int nFiltro);
    void CFormComprobar::ResultadoFiltro(int nFiltro, double dReflectancia, double dError);
public:
    void MostrarPosicion(double x, double y, double z);
    void MostrarFiltro(int nFiltro);
    void ActualizarEscala();

    int GetAnchoResto();
    int GetAltoResto();
    HWND GetControlImagen();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CFormComprobar_H__BBF699CF_5E25_4EC7_86A5_CDEBA92EB0A7__INCLUDED_)
