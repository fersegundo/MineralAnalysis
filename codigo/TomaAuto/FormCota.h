#if !defined(AFX_FORM_COTA_)
#define AFX_FORM_COTA_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormComun.h"
#include "EasySize.h"
#include "BarridoPatron.h"
#include "resource.h"		// main symbols

class CFormCota : public CFormComun
{
DECLARE_EASYSIZE
public:

private:
    bool                m_bCambiadoManualmente;            //para evitar que se procesen cambios "manuales" (mediante codigo) al editar 
    long                m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    long                m_anchoTitulo, m_altoTitulo; //Necesario para dimensionar padre (dialogo con menu y barra)
// Construction
public:
	CFormCota(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    CStatic	m_x[NUM_PUNTOS_PLANO];
	CStatic	m_y[NUM_PUNTOS_PLANO];
	CStatic	m_z[NUM_PUNTOS_PLANO];
	CButton	m_cuadro[NUM_PUNTOS_PLANO];
	CButton	m_static[NUM_PUNTOS_PLANO];
	CStatic	m_nanometros;
	CStatic	m_longitud_onda;
	CStatic	m_filtro_actual;
	//{{AFX_DATA(CFormCota)
	enum { IDD = IDD_FORM_COTA };
	CButton	m_cota;
	CEdit	m_ratio;
	CStatic	m_pos_x;
	CStatic	m_pos_z;
	CStatic	m_pos_y;
	CStatic	m_imagen;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormCota)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormCota)
	virtual BOOL OnInitDialog();
    afx_msg LRESULT UpdateDisplay(UINT wParam, LONG lParam);//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEscalaVentana();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnCota();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
    void MostrarPosicion(double x, double y, double z);
    void MostrarPosicionZ(double z);
    void MostrarFiltro(int nFiltro);

    int GetAnchoResto();
    int GetAltoResto();
    HWND GetControlImagen();

    void FinCota();
private:
    void CalculaZoom(long anchoDisplay,long altoDisplay);
    double ActualizaZoom();
    static UINT AjusteAutomaticoZThread( LPVOID Param );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif 
