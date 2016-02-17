#if !defined(AFX_FORMTOMAAUTO_H__946DAF49_1968_4BEA_B74C_DFB51037998E__INCLUDED_)
#define AFX_FORMTOMAAUTO_H__946DAF49_1968_4BEA_B74C_DFB51037998E__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// FormTomaAuto.h : header file
//

#include "FormComun.h"
#include "EasySize.h"
#include "Barrido.h"
#include "resource.h"		// main symbols
#include "ProgresoMosaico.h"

/*
enum ENUM_ESQUINA_ACTIVA
{
    NINGUNA         = 0,
    ESQUINA_1       = 1,
    ESQUINA_2       = 2,
};
*/


/////////////////////////////////////////////////////////////////////////////
// CFormTomaAuto dialog
class CProgreso;
class CFormTomaAuto : public CFormComun
{
DECLARE_EASYSIZE
public:
    CProgreso* m_pProgreso; //se accede desde barrido

private:
    CBarrido            barrido;
  
    bool                m_bCambiadoManualmente;            //para evitar que se procesen cambios "manuales" (mediante codigo) al editar 
    bool                m_bEsquina1Marcada;
    bool                m_bEsquina2Marcada;
    long                m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    long                m_anchoTitulo, m_altoTitulo; //Necesario para dimensionar padre (dialogo con menu y barra)
    ProgresoMosaico     m_progreso_mosaico; //dialogo de progreso de construccion del mosaico
// Construction
public:
	CFormTomaAuto(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
    CStatic	m_x[NUM_PUNTOS_PLANO];
	CStatic	m_y[NUM_PUNTOS_PLANO];
	CStatic	m_z[NUM_PUNTOS_PLANO];
	CButton	m_cuadro[NUM_PUNTOS_PLANO];
	CButton	m_static[NUM_PUNTOS_PLANO];
	CStatic	m_nanometros;
	CStatic	m_longitud_onda;
	CStatic	m_filtro_actual;
	//{{AFX_DATA(CFormTomaAuto)
	enum { IDD = IDD_FORM_TOMAAUTO };
	CStatic	m_progreso_actual;
	CButton	m_boton_autoenfocar;
	CStatic	m_progreso_static_campos;
	CStatic	m_progreso_total;
	CStatic	m_progreso_acutal;
	CStatic	m_progreso_static;
	CStatic	m_progreso_barra;
	CEdit	m_ratio;
	CStatic	m_progreso;
	CEdit	m_paso_y;
	CEdit	m_paso_x;
	CStatic	m_dimension_max_y;
	CStatic	m_dimension_max_x;
	CEdit	m_dimension_x;
	CEdit	m_dimension_y;
	CEdit	m_dimension_micras_y;
	CEdit	m_dimension_micras_x;
	CButton	m_radio_cobertura;
	CButton	m_radio_cobertura_espaciado;
	CEdit	m_escala;
	CButton	m_boton_mosaico;
	CStatic	m_pos_x;
	CStatic	m_pos_z;
	CStatic	m_pos_y;
	CButton	m_cancelar;
	CEdit	m_nombre;
	CStatic	m_esquina2_z;
	CStatic	m_esquina1_z;
	CEdit	m_edit_cobertura;
	CButton	m_intermedios;
	CButton	m_barrer;
	CButton	m_boton_enfocado;
	CButton	m_boton_esquina2;
	CButton	m_boton_esquina;
	CStatic	m_esquina2_y;
	CStatic	m_esquina2_x;
	CStatic	m_esquina1_y;
	CStatic	m_esquina1_x;
	CStatic	m_imagen;
	CEdit	m_Directorio;
	int		m_nRadio_cobertura_espaciado;
	int		m_nRadio_cobertura;
	CString	m_static_campos_x;
	CString	m_static_campos_y;
	CString	m_static_dimension_x;
	CString	m_static_dimension_y;
	BOOL	m_bAutoenfoque;
	CString	m_csProgreso_actual;
	CString	m_csProgreso_total;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CFormTomaAuto)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
    virtual BOOL PreTranslateMessage(MSG* pMsg);
	//}}AFX_VIRTUAL

// Implementation
protected:

	// Generated message map functions
	//{{AFX_MSG(CFormTomaAuto)
	virtual BOOL OnInitDialog();
	afx_msg void OnExaminar();
	afx_msg void OnChangeEditDirectorio();
	afx_msg void OnEsquina();
	afx_msg void OnEsquina2();
	afx_msg void OnIntermedios();
	afx_msg void OnEnfocado();
	afx_msg void OnBarrer();
	afx_msg void OnRadioCobertura();
	afx_msg void OnRadioEspaciado();
	afx_msg void OnChangeEditNombre();
	afx_msg void OnCancelar();
	afx_msg void OnChangeEditDimensionX();
	afx_msg void OnChangeEditDimensionY();
	afx_msg void OnChangeEditCobertura();
	afx_msg void OnRadioCobertura30();
	afx_msg void OnRadioCobertura50();
	afx_msg void OnRadioCobertura100();
	afx_msg void OnRadioCoberturaEdit();
	afx_msg void OnButtonMosaico();
	afx_msg void OnChangeEditEscala();
    afx_msg LRESULT UpdateDisplay(UINT wParam, LONG lParam);//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	afx_msg void OnChangeEditDimensionMicrasX();
	afx_msg void OnChangeEditDimensionMicrasY();
	afx_msg void OnChangeEditPasoX();
	afx_msg void OnChangeEditPasoY();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnKillfocusEscalaVentana();
	afx_msg void OnDestroy();
	afx_msg void OnShowWindow(BOOL bShow, UINT nStatus);
	afx_msg void OnAutoenfocar();
	afx_msg void OnMover();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    // funcion para ejecutar el thread
    static UINT GenerarMosaicoThread( LPVOID Param );

    void BorrarIntermedios();
    void ActualizarBarrido();
    bool ActualizarInfoBarrido();
    void CalculaZoom(long anchoDisplay,long altoDisplay);
    double ActualizaZoom();
public:
    void PermitirNuevoBarrido(bool bCancelado);
    void HayCorreccion(bool bHayCorreccion);
    void MostrarPosicion(double x, double y, double z);
    void MostrarFiltro(int nFiltro);
    void InicializaProgresion(int nCamposTotales);
    void IncrementaProgresion();
    void IncrementaProgresionMosaico();
    void ActualizarEscala();

    int GetAnchoResto();
    int GetAltoResto();
    HWND GetControlImagen();

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_FORMTOMAAUTO_H__946DAF49_1968_4BEA_B74C_DFB51037998E__INCLUDED_)
