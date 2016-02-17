#if !defined(AFX_FORMTOMAUNICA_H_INCLUDED_)
#define AFX_FORMTOMAUNICA_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FormTomaAuto.h : header file
//

#include "FormComun.h"
#include "EasySize.h"

/////////////////////////////////////////////////////////////////////////////
// CFormTomaUnica dialog
class CFormTomaUnica : public CFormComun
{
DECLARE_EASYSIZE
public:

private:
    bool                m_bCambiadoManualmente;            //para evitar que se procesen cambios "manuales" (mediante codigo) al editar 
    long                m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    long                m_anchoTitulo, m_altoTitulo; //Necesario para dimensionar padre (dialogo con menu y barra)
    int                 m_nCampo;                      //contador campo toma única (hasta que se modifique el nombre de campo)
// Construction
public:
	CFormTomaUnica(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	CStatic	m_nanometros;
	CStatic	m_longitud_onda;
	CStatic	m_filtro_actual;
	//{{AFX_DATA(CFormTomaUnica)
	enum { IDD = IDD_FORM_TOMAUNICA };
	CEdit	m_ratio;
	CStatic	m_dimension_max_y;
	CStatic	m_dimension_max_x;
	CEdit	m_dimension_x;
	CEdit	m_dimension_y;
	CEdit	m_dimension_micras_y;
	CEdit	m_dimension_micras_x;
	CEdit	m_escala;
	CStatic	m_pos_x;
	CStatic	m_pos_z;
	CStatic	m_pos_y;
	CEdit	m_nombre;
	CStatic	m_imagen;
	CEdit	m_Directorio;
	CString	m_static_dimension_x;
	CString	m_static_dimension_y;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormTomaUnica)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormTomaUnica)
	virtual BOOL OnInitDialog();
	afx_msg void OnExaminar();
	afx_msg void OnChangeEditDirectorio();
	afx_msg void OnChangeEditDimensionX();
	afx_msg void OnChangeEditDimensionY();
	afx_msg void OnChangeEditEscala();
	afx_msg void OnChangeEditDimensionMicrasX();
	afx_msg void OnChangeEditDimensionMicrasY();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEscalaVentana();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnToma();
	afx_msg void OnAutoenfocar();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    static UINT TomaThread( LPVOID Param );

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
