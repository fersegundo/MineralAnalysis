// ClasificaBarridoDlg.h : header file
//

#include "resourceClasificaBarrido.h"
#include "AsociacionesBarridoDlg.h"

#if !defined(AFX_CLASIFICABARRIDODLG_H__AC51A6BE_A27B_4CAC_BCBF_685BF869A486__INCLUDED_)
#define AFX_CLASIFICABARRIDODLG_H__AC51A6BE_A27B_4CAC_BCBF_685BF869A486__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define WM_UPDATEFIELDS WM_APP + 1 //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)

// clase necesaria para ordenar el resultado de la clasificacion por numero de pixels encontrados
class CResultadoClasificacionBarrido
{
public:
    CString m_csName;
    unsigned int     m_nPixels;
    double  m_dPorcentaje;

    CResultadoClasificacionBarrido()
    {
        m_nPixels       = -1;;
        m_dPorcentaje   = -1;
    } 

    CResultadoClasificacionBarrido(CResultadoClasificacionBarrido const& rhs)
    {
        if (&rhs != this)
        {
            m_csName = rhs.m_csName;
            m_nPixels = rhs.m_nPixels;
            m_dPorcentaje = rhs.m_dPorcentaje;
        }
    }

    virtual ~CResultadoClasificacionBarrido()
    {
    }

    CResultadoClasificacionBarrido& operator =(const CResultadoClasificacionBarrido& rhs)
    {
        if (&rhs != this)
        {
            m_csName = rhs.m_csName;
            m_nPixels = rhs.m_nPixels;
            m_dPorcentaje = rhs.m_dPorcentaje;
        }

        return *this;
    }

    bool operator==(const CResultadoClasificacionBarrido& rhs) const
    {
        return (m_csName == rhs.m_csName) ? true : false;
    }

    bool operator<(const CResultadoClasificacionBarrido& rhs) const
    {
        return (m_nPixels > rhs.m_nPixels) ? true : false;
    }    

    bool operator>(const CResultadoClasificacionBarrido& rhs) const
    {
        return (m_nPixels < rhs.m_nPixels) ? true : false;
    }
};

/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoDlg dialog

class CClasificaBarridoDlg : public CDialog
{
private:
    CMenu m_Menu;
    CAsociacionesBarridoDlg m_asociacionesDlg;

// Construction
public:
	CClasificaBarridoDlg(CWnd* pParent = NULL);	// standard constructor
    void CClasificaBarridoDlg::ActualizaProgresionMinerales(unsigned int* arrParcial,unsigned int* arrAcum,  
                                                            unsigned int nCountOscuros, unsigned int nCountNoClasificados, unsigned int nCountOscurosAcum, unsigned int nCountNoClasificadosAcum,
                                                            int nCamposTerminados, bool bUltimoCampo,
                                                            FILE* fichero);

// Dialog Data
	//{{AFX_DATA(CClasificaBarridoDlg)
	enum { IDD = IDD_CLASIFICABARRIDO_DIALOG };
	CButton	m_buttonExaminar;
	CTreeCtrl	m_progreso_no_acum;
	CTreeCtrl	m_progreso_no;
	CTreeCtrl	m_progreso_acum;
	CStatic	m_static_x;
	CButton	m_buttonSegundaPasada;
	CTreeCtrl	m_progreso;
	CEdit	m_Directorio;
	CString	m_csMuestra;
	CString	m_csPorcentajeAceptados;
	CString	m_csCampoActual;
	CString	m_csCamposTotal;
	CString	m_csCamposX;
	CString	m_csCamposY;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CClasificaBarridoDlg)
	public:
	virtual BOOL PreTranslateMessage(MSG* pMsg);
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CClasificaBarridoDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
    afx_msg LRESULT UpdateDisplay(UINT wParam, LONG lParam);//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	afx_msg void OnSeleccionarDirectorio();
	afx_msg void OnKillfocusDirecorio();
	afx_msg void OnSegundaPasada();
	afx_msg void OnClose();
	afx_msg void OnConfiguracionOpciones();
	afx_msg void OnConfiguracionAsociacionesminerales();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    bool MostrarInfoBarrido();
    void CClasificaBarridoDlg::PermitirNuevaClasificacion();
    void TratarDirectorio();
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CLASIFICABARRIDODLG_H__AC51A6BE_A27B_4CAC_BCBF_685BF869A486__INCLUDED_)
