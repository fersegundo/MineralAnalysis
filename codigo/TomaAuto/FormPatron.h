#if !defined(AFX_FormPatron_H__8668093E_9E59_414C_83F0_40BFF63D1CD9__INCLUDED_)
#define AFX_FormPatron_H__8668093E_9E59_414C_83F0_40BFF63D1CD9__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormComun.h"
#include "EasySize.h"
#include "BarridoPatron.h"
#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CFormPatron dialog
class CProgreso;
class CFormPatron : public CFormComun
{
DECLARE_EASYSIZE
public:
    CProgreso* m_pProgreso; //se accede desde barrido

private:
    CBarridoPatron      m_barridoPatron;
    bool                m_bTipoPatronClaro; // true si patron claro, false si patron oscuro
    bool                m_bExposicionAjustada; //para avisar si no se ha realizado al barrer

    bool                m_bCambiadoManualmente;            //para evitar que se procesen cambios "manuales" (mediante codigo) al editar 
    bool                m_bEsquina1Marcada;
    bool                m_bEsquina2Marcada;
    long                m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    long                m_anchoTitulo, m_altoTitulo; //Necesario para dimensionar padre (dialogo con menu y barra)
// Construction
public:
	CFormPatron(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    CStatic	m_x[NUM_PUNTOS_PLANO];
	CStatic	m_y[NUM_PUNTOS_PLANO];
	CStatic	m_z[NUM_PUNTOS_PLANO];
	CButton	m_cuadro[NUM_PUNTOS_PLANO];
	CButton	m_static[NUM_PUNTOS_PLANO];
	CStatic	m_nanometros;
	CStatic	m_longitud_onda;
	CStatic	m_filtro_actual;
	//{{AFX_DATA(CFormPatron)
	enum { IDD = IDD_FORM_PATRON };
	CStatic	m_progreso_actual;
	CButton	m_exposicion;
	CStatic	m_progreso_static_campos;
	CStatic	m_progreso_total;
	CStatic	m_progreso_acutal;
	CStatic	m_progreso_static;
	CStatic	m_progreso_barra;
	CEdit	m_ratio;
	CStatic	m_progreso;
	CStatic	m_pos_x;
	CStatic	m_pos_z;
	CStatic	m_pos_y;
	CStatic	m_esquina2_z;
	CStatic	m_esquina1_z;
	CButton	m_intermedios;
	CButton	m_barrer;
	CButton	m_cancelar;
	CButton	m_boton_enfocado;
	CButton	m_boton_esquina2;
	CButton	m_boton_esquina;
	CStatic	m_esquina2_y;
	CStatic	m_esquina2_x;
	CStatic	m_esquina1_y;
	CStatic	m_esquina1_x;
	CStatic	m_imagen;
	CString	m_csProgreso_actual;
	CString	m_csProgreso_total;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormPatron)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormPatron)
	virtual BOOL OnInitDialog();
	afx_msg void OnEsquina();
	afx_msg void OnEsquina2();
	afx_msg void OnIntermedios();
	afx_msg void OnEnfocado();
	afx_msg void OnBarrer();
	afx_msg void OnCancelar();
    afx_msg LRESULT UpdateDisplay(UINT wParam, LONG lParam);//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEscalaVentana();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnExposicion();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    void BorrarIntermedios();
    void CalculaZoom(long anchoDisplay,long altoDisplay);
    double ActualizaZoom();
public:
    void SetTipoPatron(bool bTipoPatronClaro);
    bool GetTipoPatron();
    void PermitirNuevoBarrido(bool bCancelado);
    void HayCorreccion(bool bHayCorreccion);
    void MostrarPosicion(double x, double y, double z);
    void MostrarPosicionZ(double z);
    void MostrarFiltro(int nFiltro);
    void InicializaProgresion(int nCamposTotales);
    void IncrementaProgresion();

    int GetAnchoResto();
    int GetAltoResto();
    HWND GetControlImagen();

    void CFormPatron::FinExposicion();
    void CFormPatron::FinCota();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMTOMAAUTO_H__946DAF49_1968_4BEA_B74C_DFB51037998E__INCLUDED_)
