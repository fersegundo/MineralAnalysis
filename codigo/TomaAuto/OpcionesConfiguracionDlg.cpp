// OpcionesConfiguracion.cpp : implementation file
//

#include "stdafx.h"
#include "TomaAuto.h"
#include "OpcionesConfiguracionDlg.h"

#include "folder_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// COpcionesConfiguracionDlg dialog

extern CTomaAutoApp theApp;

COpcionesConfiguracionDlg::COpcionesConfiguracionDlg(CWnd* pParent /*=NULL*/)
	: CDialog(COpcionesConfiguracionDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(COpcionesConfiguracionDlg)
	m_csRefMax = _T("");
	//}}AFX_DATA_INIT
}


void COpcionesConfiguracionDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(COpcionesConfiguracionDlg)
	DDX_Control(pDX, IDC_CAMPOS_ACUMULAR, m_campos_acumular);
	DDX_Control(pDX, IDC_COMBO_BITS, m_combo_bits);
	DDX_Control(pDX, IDC_ESCALA_REFLECTANCIA, m_escala_reflectancia);
	DDX_Text(pDX, IDC_REF_MAX, m_csRefMax);
	//}}AFX_DATA_MAP
}


BEGIN_MESSAGE_MAP(COpcionesConfiguracionDlg, CDialog)
	//{{AFX_MSG_MAP(COpcionesConfiguracionDlg)
	ON_EN_CHANGE(IDC_ESCALA_REFLECTANCIA, OnChangeEscalaReflectancia)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// COpcionesConfiguracionDlg message handlers

//! Inicializacion del dialogo
BOOL COpcionesConfiguracionDlg::OnInitDialog() 
{
	CDialog::OnInitDialog();
	
    // Establecemos la escala de reflectancia 
    CString csAux;
    csAux.Format("%.4lf",theApp.m_configuracion.m_dEscalaReflectancia);
    m_escala_reflectancia.SetWindowText(csAux);

    m_csRefMax.Format("%.2lf",theApp.m_configuracion.m_dEscalaReflectancia * 256);

    // Establecemos los bits de salida
    csAux.Format("%d",theApp.m_configuracion.m_nBits);
    m_combo_bits.SetCurSel(m_combo_bits.FindString(0,csAux));

    // Establecemos los campos a acumular
    csAux.Format("%d",theApp.m_configuracion.m_nImagenAcum);
    m_campos_acumular.SetWindowText(csAux);
    

    UpdateData(FALSE);

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

// Para controlar el formato de la escala y actualizar la reflectancia maxima
void COpcionesConfiguracionDlg::OnChangeEscalaReflectancia() 
{
    //Obtenemos la escala del dialogo
    CString csEscala;
    CString csAux;
    m_escala_reflectancia.GetWindowText(csEscala);
    double dEscala = atof((LPCSTR)csEscala);

    // No permitimos caracteres no decimales
    int nLength = csEscala.GetLength();
    if (nLength > 0)
    {
        if (csEscala[nLength-1]==',')
        {
            //cambiar , por .
            csEscala.SetAt(nLength-1, '.');
            m_escala_reflectancia.SetWindowText((LPCTSTR)csEscala);
            m_escala_reflectancia.SetSel(nLength,nLength); //para que se desplace el cursor al final
        }
        if (csEscala[nLength-1]=='.')
        {
            //no actualizar, esperar a los decimales
        }
        else if (csEscala[nLength-1] > 47 && csEscala[nLength-1]<58) // 0-9
        {
            // Actualizar reflectancia maxima
            m_csRefMax.Format("%.2lf",dEscala * 256);
        }
        else
        {
            // No permitimos el caracter, asi que lo eliminamos
            csEscala.Delete(nLength-1);
            m_escala_reflectancia.SetWindowText((LPCTSTR)csEscala);
            m_escala_reflectancia.SetSel(nLength,nLength); //para que se desplace el cursor al final
        }
    }
    else
    {
        m_csRefMax.Empty();
    }

    UpdateData(FALSE);
}

//! Acciones al presionar el boton OK
void COpcionesConfiguracionDlg::OnOK() 
{
    //Actualizamos la escala
    CString csAux;
    m_escala_reflectancia.GetWindowText(csAux);
    theApp.m_configuracion.m_dEscalaReflectancia = atof((LPCSTR)csAux); 
    //Comprobamos limites
    if (theApp.m_configuracion.m_dEscalaReflectancia < MIN_ESCALA_REFLECTANCIA)
    {
        theApp.m_configuracion.m_dEscalaReflectancia = MIN_ESCALA_REFLECTANCIA;
//        csAux.Format("%.3f",dEscala);
        csAux.Format("%.3f", theApp.m_configuracion.m_dEscalaReflectancia);
        m_escala_reflectancia.SetWindowText((LPCTSTR)csAux);
    }
    if (theApp.m_configuracion.m_dEscalaReflectancia > MAX_ESCALA_REFLECTANCIA)
    {
        theApp.m_configuracion.m_dEscalaReflectancia = MAX_ESCALA_REFLECTANCIA;
//        csAux.Format("%.3f",dEscala);
        csAux.Format("%.3f", theApp.m_configuracion.m_dEscalaReflectancia);
        m_escala_reflectancia.SetWindowText((LPCTSTR)csAux);
    }

    //Actualizamos los bits de salida
    m_combo_bits.GetWindowText(csAux);
    theApp.m_configuracion.m_nBits = atoi((LPCSTR)csAux); 

    //Actualizamos los campos a acumular
    m_campos_acumular.GetWindowText(csAux);
    theApp.m_configuracion.m_nImagenAcum = atoi((LPCSTR)csAux); 
    //Comprobamos limites

	CDialog::OnOK();
}

