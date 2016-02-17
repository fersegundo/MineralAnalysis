#if !defined(AFX_FORM_CORRECCION_GEOMETRICA)
#define AFX_FORM_CORRECCION_GEOMETRICA

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormComun.h"
#include "EasySize.h"

/////////////////////////////////////////////////////////////////////////////
class CFormCorreccionGeometrica : public CFormComun
{
DECLARE_EASYSIZE
public:

private:
    long                m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    long                m_anchoTitulo, m_altoTitulo; //Necesario para dimensionar padre (dialogo con menu y barra)
// Construction
public:
	CFormCorreccionGeometrica(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	CStatic	m_nanometros;
	CStatic	m_longitud_onda;
	CStatic	m_filtro_actual;
	//{{AFX_DATA(CFormCorreccionGeometrica)
	enum { IDD = IDD_FORM_GEOMETRICA };
	CButton	m_proceder2;
	CButton	m_proceder;
	CEdit	m_ratio;
	CStatic	m_pos_x;
	CStatic	m_pos_z;
	CStatic	m_pos_y;
	CStatic	m_imagen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormCorreccionGeometrica)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormCorreccionGeometrica)
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEscalaVentana();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnProceder();
	afx_msg void OnProceder2();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    static UINT ProcederThread( LPVOID Param );
    static UINT Proceder2Thread( LPVOID Param );

    void CalculaZoom(long anchoDisplay,long altoDisplay);
    double ActualizaZoom();
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

#endif
