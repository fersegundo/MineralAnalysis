// GestionMineralesDlg.h : header file
//

#if !defined(AFX_GESTIONMINERALESDLG_H__731E49D2_FB58_4EBF_8F16_D827BAC9CCB3__INCLUDED_)
#define AFX_GESTIONMINERALESDLG_H__731E49D2_FB58_4EBF_8F16_D827BAC9CCB3__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#include <vector>
#include "NuevoMineralDlg.h"

/////////////////////////////////////////////////////////////////////////////
// CGestionMineralesDlg dialog

class CGestionMineralesDlg : public CDialog
{
private:
    CRect               m_rect;      // dimensiones del dialogo sin añadir ningun control. Necesario para añadir nuevos controles
    vector<CButton*>    m_vecCheck;  // vector de controles check box de cada mineral
    vector<CButton*>    m_vecEdit;   // vector de botones para editar cada mineral
    vector<CButton*>    m_vecDelete; // vector de botones para eliminar cada mineral
    //vector<int>         m_vecMap;    // vector de indices originales para poder trazar el control presionado

    CBitmap m_bitmapEditar;
    CBitmap m_bitmapBorrar;

    NuevoMineralDlg m_edicionDlg;

// Construction
public:
	CGestionMineralesDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CGestionMineralesDlg)
	enum { IDD = IDD_GESTIONMINERALES_DIALOG };
		// NOTE: the ClassWizard will add data members here
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CGestionMineralesDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	virtual BOOL OnCommand(WPARAM wParam, LPARAM lParam);
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CGestionMineralesDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnNuevoMineral();
	virtual void OnOK();
	virtual void OnCancel();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    void CGestionMineralesDlg::Liberar();

    void CGestionMineralesDlg::NuevoGrupoControles(int nIndice) ;

    void CGestionMineralesDlg::ProcesarEliminacion(int nIndice) ;
    void CGestionMineralesDlg::ProcesarEdicion(int nIndice) ;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_GESTIONMINERALESDLG_H__731E49D2_FB58_4EBF_8F16_D827BAC9CCB3__INCLUDED_)
