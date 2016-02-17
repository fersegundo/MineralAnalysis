// TomaAutoDlg.h : header file
//

#if !defined(AFX_TOMAAUTODLG_H__20E67564_36A4_4684_93AB_184A87A80611__INCLUDED_)
#define AFX_TOMAAUTODLG_H__20E67564_36A4_4684_93AB_184A87A80611__INCLUDED_


#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include "FormTomaAuto.h"
#include "FormTomaUnica.h"
#include "FormPatron.h"
#include "FormCorreccionGeometrica.h"
#include "FormCota.h"
#include "FormComprobar.h"
#include "OpcionesConfiguracionDlg.h"
#include "MoverDlg.h"

// Estructura para pasar al thread de ActualizaPosicion
typedef struct PARAM_THREAD_POS
{
    CTomaAutoDlg*       pThis;
} PARAM_THREAD_POS;

/////////////////////////////////////////////////////////////////////////////
// CTomaAutoDlg dialog
class CTomaAutoDlg : public CDialog
{
private:
    CMenu                           m_Menu;
    CFormTomaAuto                   m_formTomaAuto;
    CFormTomaUnica                  m_formTomaUnica;
    CFormPatron                     m_formPatron;
    CFormCorreccionGeometrica       m_formCorreccionGeometrica;
    CFormCota                       m_formCota;
    CFormComprobar                  m_formComprobar;
    CFormComun*                     m_formActivo;
                                    
    CWinThread*                     m_threadPos;    // HANDLE al thread de posicion. Necesario para poder esperar correctamente el thread al finalizar
    PARAM_THREAD_POS*               m_paramThreadPos;
    HANDLE                          m_StopThread;        // COMUNICACION para finalizar el thread
  
	COpcionesConfiguracionDlg       m_OpcionesConfiguracionDlg;
	CMoverDlg                       m_MoverDlg;

// Construction
public:
	CTomaAutoDlg(CWnd* pParent = NULL);	// standard constructor
    void SetSize(int nAncho, int nAlto);

    void CTomaAutoDlg::MostrarFiltro(int nFiltro);
    void CTomaAutoDlg::HayCorreccion(bool bHayCorreccion, bool bCambiarBarrido = false);

    void CTomaAutoDlg::LanzarThreadPosicion();
    void CTomaAutoDlg::FinalizarThreadPosicion();

    void Autoenfocar();
    void CTomaAutoDlg::Mover();

// Dialog Data
	//{{AFX_DATA(CTomaAutoDlg)
	enum { IDD = IDD_TOMAAUTO_DIALOG };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CTomaAutoDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	afx_msg void OnAccionesCambiarobjetivo1();
	afx_msg void OnAccionesCambiarobjetivo2();
	afx_msg void OnAccionesCambiarobjetivo3();
	afx_msg void OnAccionesCambiarobjetivo4();
	afx_msg void OnAccionesCambiarobjetivo5();
	afx_msg void OnAccionesCambiarobjetivo6();
	//{{AFX_MSG(CTomaAutoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnAccionesCambiarcubos1();
	afx_msg void OnAccionesCambiarcubos2();
	afx_msg void OnAccionesCambiarcubos3();
	afx_msg void OnAccionesCambiarcubos4();
	afx_msg void OnAccionesAutoenfocar();
	afx_msg void OnClose();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnModoTomaunica();
	afx_msg void OnModoTomaautomatica();
	afx_msg void OnModoCalibracionAlta();
	afx_msg void OnModoCalibracionBaja();
	afx_msg void OnModoCorreccionGeometrica();
	afx_msg void OnModoCota();
	afx_msg void OnModoComprobar();
	afx_msg void OnOpcionesConfiguracion();
	afx_msg void OnMover();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()

private:
    // funcion para ejecutar el thread
    static UINT ActualizarPosicion (LPVOID param);
    void CTomaAutoDlg::MostrarPosicion(point p);

    void CTomaAutoDlg::CambiarCubo(int nCubo);
    void CTomaAutoDlg::CambiarObjetivo(int nObjetivo);

    void CTomaAutoDlg::Autoenfoque();
    static UINT AutoenfocarThread( LPVOID Param );
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_TOMAAUTODLG_H__20E67564_36A4_4684_93AB_184A87A80611__INCLUDED_)
