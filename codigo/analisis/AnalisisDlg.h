// AnalisisDlg.h : header file
//

#if !defined(AFX_ANALISISDLG_H__63B2236E_8388_40FA_BC07_7F70B8C48411__INCLUDED_)
#define AFX_ANALISISDLG_H__63B2236E_8388_40FA_BC07_7F70B8C48411__INCLUDED_

#include "ControlImagenes.h"
#include "Clasificacion.h"
#include "Areas.h"
#include "Grafico.h"
#include <vector>
#include "ColorStatic.h"
#include "ColorEdit.h"
#include "EasySize.h"
#include "AsociacionesDlg.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

using namespace std ;

/////////////////////////////////////////////////////////////////////////////
// CAnalisisDlg dialog

class CAnalisisDlg : public CDialog
{
DECLARE_EASYSIZE
private:
    CAsociacionesDlg    m_asociacionesDlg;
// Construction
public:
	CAnalisisDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	//{{AFX_DATA(CAnalisisDlg)
	enum { IDD = IDD_ANALISIS_DIALOG };
	CButton	m_checkInfIzq;
	CButton	m_checkInfDer;
	CButton	m_checkSupDer;
	CButton	m_checkSupIzq;
	CButton	m_botonDistancia;
	CButton	m_botonConfiabilidad;
	CTreeCtrl	m_no_clasificados;
	CButton	m_ButtonRGB;
	CButton	m_ButtonClasificar;
	CButton	m_ButtonGuardarClasificar;
	CTreeCtrl	m_clasificados;
	CButton	m_BotonSuperior;
	CButton	m_BotonInferior;
	CEdit	m_ratio;
	CButton	m_ButtonGuardarRGB;
	CButton	m_BotonSiguiente;
	CButton	m_BotonAnterior;
	CButton	m_guardar;
    CBitmap m_guardar_bitmap;
    CBitmap m_anterior_bitmap;
    CBitmap m_siguiente_bitmap;
    CBitmap m_superior_bitmap;
    CBitmap m_inferior_bitmap;
	CStatic	m_mineral_static;
	CStatic	m_comentario_static;
	CEdit	m_comentario;
	CComboBox	m_combo_mineral;
	CComboBox	m_combo_calidad;
	CStatic	m_calidad_static;
	CStatic	m_histograma_frame;
	CStatic	m_espectros_frame;
	CTreeCtrl	m_tree_areas;
	CButton	m_control_group;
	CColorStatic 	m_TotalR;
	CColorStatic 	m_TotalG;
	CColorStatic 	m_TotalB;
	CStatic	m_static400;
	CStatic	m_static450;
	CStatic	m_static500;
	CStatic	m_static550;
	CStatic	m_static600;
	CStatic	m_static650;
	CStatic	m_static700;
	CStatic	m_static750;
	CStatic	m_static800;
	CStatic	m_static850;
	CStatic	m_static900;
	CStatic	m_static950;
	CStatic	m_static1000;
	CEdit	m_edit450r;
	CEdit	m_edit450g;
	CEdit	m_edit450b;
	CEdit	m_edit400r;
	CEdit	m_edit400g;
	CEdit	m_edit400b;
	CEdit	m_edit550r;
	CEdit	m_edit550g;
	CEdit	m_edit550b;
	CEdit	m_edit500r;
	CEdit	m_edit500g;
	CEdit	m_edit500b;
   	CEdit	m_edit650r;
	CEdit	m_edit650g;
	CEdit	m_edit650b;
	CEdit	m_edit600r;
	CEdit	m_edit600g;
	CEdit	m_edit600b;
    CEdit	m_edit750r;
	CEdit	m_edit750g;
	CEdit	m_edit750b;
	CEdit	m_edit700r;
	CEdit	m_edit700g;
	CEdit	m_edit700b;
    CEdit	m_edit850r;
	CEdit	m_edit850g;
	CEdit	m_edit850b;
	CEdit	m_edit800r;
	CEdit	m_edit800g;
	CEdit	m_edit800b;
    CEdit	m_edit950r;
	CEdit	m_edit950g;
	CEdit	m_edit950b;
	CEdit	m_edit900r;
	CEdit	m_edit900g;
	CEdit	m_edit900b;
    CEdit	m_edit1000r;
	CEdit	m_edit1000g;
	CEdit	m_edit1000b;
	CTabCtrl	m_tabControl;
	CStatic	m_control;
	BOOL	m_bTodosMinerales;
	CString	m_csUmbralConfiabilidad;
	CString	m_csUmbralDistancia;
	CString	m_dMinRef;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAnalisisDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;
	// Generated message map functions
	//{{AFX_MSG(CAnalisisDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnChangeEdit400B();
	afx_msg void OnChangeEdit450B();
	afx_msg void OnChangeEdit500B();
	afx_msg void OnChangeEdit550B();
	afx_msg void OnChangeEdit600B();
	afx_msg void OnChangeEdit650B();
	afx_msg void OnChangeEdit700B();
	afx_msg void OnChangeEdit750B();
	afx_msg void OnChangeEdit800B();
	afx_msg void OnChangeEdit850B();
	afx_msg void OnChangeEdit900B();
	afx_msg void OnChangeEdit950B();
	afx_msg void OnChangeEdit1000B();
	afx_msg void OnChangeEdit400G();
	afx_msg void OnChangeEdit450G();
	afx_msg void OnChangeEdit500G();
	afx_msg void OnChangeEdit550G();
	afx_msg void OnChangeEdit600G();
	afx_msg void OnChangeEdit650G();
	afx_msg void OnChangeEdit700G();
	afx_msg void OnChangeEdit750G();
	afx_msg void OnChangeEdit800G();
	afx_msg void OnChangeEdit850G();
	afx_msg void OnChangeEdit900G();
	afx_msg void OnChangeEdit950G();
	afx_msg void OnChangeEdit1000G();
	afx_msg void OnChangeEdit400R();
	afx_msg void OnChangeEdit450R();
	afx_msg void OnChangeEdit500R();
	afx_msg void OnChangeEdit550R();
	afx_msg void OnChangeEdit600R();
	afx_msg void OnChangeEdit650R();
	afx_msg void OnChangeEdit700R();
	afx_msg void OnChangeEdit750R();
	afx_msg void OnChangeEdit800R();
	afx_msg void OnChangeEdit850R();
	afx_msg void OnChangeEdit900R();
	afx_msg void OnChangeEdit950R();
	afx_msg void OnChangeEdit1000R();
	afx_msg void OnAbrirTodas();
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnDestroy();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSelchangedAreas(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnKeydownAreas(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnEndlabeleditAreas(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnCustomdrawAreas ( NMHDR* pNMHDR, LRESULT* pResult );
    afx_msg void OnCustomdrawClasificados ( NMHDR* pNMHDR, LRESULT* pResult );
	afx_msg void OnDummy();
	afx_msg void OnClickAreas(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnTimer(UINT nIDEvent);
	afx_msg void OnChangeComentario();
	afx_msg void OnSelchangeComboMineral();
	afx_msg void OnSelchangeComboCalidad();
	afx_msg void OnGuardar();
	afx_msg void OnGenerar();
	afx_msg void OnAnterior();
	afx_msg void OnSiguiente();
	afx_msg void OnGuardarRgb();
	afx_msg void OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI);
	afx_msg void OnChangeRatio();
	afx_msg void OnSuperior();
	afx_msg void OnInferior();
	afx_msg void OnKillfocusRatio();
	afx_msg void OnIdentificar();
	afx_msg void OnGuardarClasificar();
	afx_msg void OnClasificar();
	afx_msg void OnRgb();
	afx_msg void OnConfiabilidad();
	afx_msg void OnDistancia();
	afx_msg void OnDropdownComboMineral();
	afx_msg void OnSupIzq();
	afx_msg void OnSupDer();
	afx_msg void OnInfIzq();
	afx_msg void OnInfDer();
	afx_msg void OnChangeUmbralConfiabilidad();
	afx_msg void OnChangeUmbralDistancia();
	afx_msg void OnTodosMinerales();
	afx_msg void OnConfiguracinAsociacionesminerales();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
private:
    CMenu m_Menu;
    CGrafico    m_histograma;
    CGrafico    m_espectros;
    CGrafico    prueba;
    ControlImagenes controlImagenes;
    CAreas          m_areas;
    int m_nNumTab;
    vector<int> filterOfTab;
    int m_nCampoActual;   // indica el campo de trabajo actual, para ayudar en la navegacion
    int m_nFilaActual;   // indica el campo de trabajo actual, para ayudar en la navegacion
    int m_nCampoAnterior; // indica el campo inmediatamente anterior disponible, para gestionar botones
    int m_nCampoSiguiente;
    int m_nCampoSuperior; // indica el campo inmediatamente superior disponible, para gestionar botones
    int m_nCampoInferior;
    CString m_csNombreBanda; // indica el fichero espectral correspondiente al campo actual
    long m_anchoResto, m_altoResto; //necesario para limitar las dimensiones maximas en OnGetMinMaxInfo
    bool m_bFormato2d; //si las imagenes a tratar vienen de un barrido y por tanto tienen el formato NOMBRE_FILA_CAMPO_14.tif
    int m_nUltimoFiltro; //ultimo filtro selecionado (en el tabulador o RGB (-1) para controlar el caso de clasificacion
    list<CMineralClasificado> m_listClasificados; // lista de minerales clasificados, necesario que sea atributo para usarlo en el callback OnCustomdrawClasificados
    unsigned int m_nCountOscuros; // numero de pixels demasiado oscuros (no clasificable). Resultado de clasificacion
    bool m_bRGB_Modificado; //para saber si hay que recalcular la imagen RGB
private:
    int  ExtraerNumFiltro(char* nomFich);
    int  TabOfFilter(int numFiltro);
    int BuscaTabLibre();
    void HabilitarControlesFiltro(int numFiltro,BOOL bEstado);
    void DeshabilitarRGB();
    void DeshabilitarPropiedadesArea();
    int GetFilterOfWavelength(int nWavelength);
    int ActualizarTotalB();
    int ActualizarTotalG();
    int ActualizarTotalR();
    void NuevaImagen(char* FileName, int numFiltro, bool bMostrar);
    int BuscarCampoSiguiente();
    int BuscarCampoAnterior();
    int BuscarCampoSuperior();
    int BuscarCampoInferior();
    void AbrirTodas();
    void CerrarTodas();
    void CrearArea(CRect& rect);
    void DefinirArea(CArea& area);
    int SelectStringAnywhere(CComboBox& comboBox, CString& csToFind);
    void RellenarComboMinerales(CArea* pArea);

public:
    void DibujarArea(CArea& area);
void GetOpcionPruebas(double& dUmbralConfiabilidad, double& dUmbralDistancia,bool& bInfDer,bool& bInfIzq,bool& bSupDer,bool& bSupIzq);
void ClasificacionSelectiva(CString csFicheroLog) ;

};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_ANALISISDLG_H__63B2236E_8388_40FA_BC07_7F70B8C48411__INCLUDED_)
