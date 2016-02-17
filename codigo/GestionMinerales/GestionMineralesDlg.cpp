// GestionMineralesDlg.cpp : implementation file
//

#include "stdafx.h"
#include "GestionMinerales.h"
#include "GestionMineralesDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CGestionMineralesApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CGestionMineralesDlg dialog

CGestionMineralesDlg::CGestionMineralesDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGestionMineralesDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CGestionMineralesDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CGestionMineralesDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CGestionMineralesDlg)
		// NOTE: the ClassWizard will add DDX and DDV calls here
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CGestionMineralesDlg, CDialog)
	//{{AFX_MSG_MAP(CGestionMineralesDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_NUEVO_MINERAL, OnNuevoMineral)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CGestionMineralesDlg message handlers

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CGestionMineralesDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CGestionMineralesDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}


BOOL CGestionMineralesDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
    //Reservamos memoria para los vectores con los minerales actuales
    int nNumMinerales = theApp.m_minerales.GetCount();
//    m_vecCheck.reserve(nNumMinerales);
//    m_vecEdit.reserve(nNumMinerales);
//    m_vecDelete.reserve(nNumMinerales);

    this->GetClientRect(m_rect);
    //int nAlto = rect.bottom - rect.top;

    m_bitmapEditar.LoadBitmap(IDB_EDIT);
    m_bitmapBorrar.LoadBitmap(IDB_BORRAR);

    for (int i= 0; i<nNumMinerales;i++)
    {
        NuevoGrupoControles(i);
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CGestionMineralesDlg::Liberar()
{
    //Liberamos memoria
    for (int i= 0; i<theApp.m_minerales.GetCount();i++)
    {
        if (m_vecDelete[i] != NULL)
            delete m_vecDelete[i];
        if (m_vecEdit[i] != NULL)
            delete m_vecEdit[i];
        if (m_vecCheck[i] != NULL)
            delete m_vecCheck[i];
    }
}

void CGestionMineralesDlg::NuevoGrupoControles(int nIndice) 
{
    const unsigned __int8 nFilasMax = 20;
    const unsigned __int8 nTamCheck = 15;
    const unsigned __int8 nEspaciado = 25;
    const unsigned __int8 nDistanciaColumnas = 200;
    const unsigned __int8 nSangriaInicial = 40;
    const unsigned __int8 nTamBotones = 15;
    const unsigned __int8 nEspaciadoBotones = 25;

    // lista para enlazar los indices iniciales con los reales de los vectores
    //m_vecMap.push_back(nIndice);
    
    unsigned int fila, columna;
    CButton* pButton;
    CString csInfo;

    HBITMAP hBitmapEditar= (HBITMAP) m_bitmapEditar.GetSafeHandle();
    HBITMAP hBitmapBorrar= (HBITMAP) m_bitmapBorrar.GetSafeHandle();

    fila = nIndice % nFilasMax;
    columna = nIndice / nFilasMax;
    csInfo.Format("%-4s - %s",theApp.m_minerales.GetAbreviatura(nIndice), theApp.m_minerales.GetNombre(nIndice));

    // Delete
    pButton = new CButton;
    pButton->Create("x",WS_CHILD | WS_VISIBLE |BS_BITMAP/*| BS_LEFTTEXT*/, 
        CRect(nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2), 
              m_rect.bottom + nEspaciado*fila, 
              nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2) + nTamBotones, 
              m_rect.bottom + nEspaciado*fila + nTamBotones ), this, CONTROLES_DINAMICOS_MIN + nIndice*3 +2);
    pButton->SetBitmap(hBitmapBorrar);
    m_vecDelete.push_back(pButton);

    //Edit
    pButton = new CButton;
    pButton->Create("e",WS_CHILD | WS_VISIBLE |BS_BITMAP  /*| BS_LEFTTEXT*/, 
        CRect(nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2)+ nEspaciadoBotones, 
              m_rect.bottom + nEspaciado*fila, 
              nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2)+ nEspaciadoBotones + nTamBotones, 
              m_rect.bottom + nTamBotones + nEspaciado*fila), this, CONTROLES_DINAMICOS_MIN + nIndice*3 +1);
    pButton->SetBitmap(hBitmapEditar);
    m_vecEdit.push_back(pButton);

    //Check
    pButton = new CButton;
    pButton->Create(csInfo,WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX /*| BS_LEFTTEXT*/, 
        CRect(nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2)+ nEspaciadoBotones*2, 
              m_rect.bottom + nEspaciado*fila, 
              nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2)+ nEspaciadoBotones*2 + nDistanciaColumnas, 
              m_rect.bottom + nTamCheck + nEspaciado*fila), this, CONTROLES_DINAMICOS_MIN + nIndice*3);
    pButton->SetCheck(theApp.m_minerales.m_list[nIndice]->m_bActivo);
    m_vecCheck.push_back(pButton);

    this->SetWindowPos(NULL,0,0,max(nSangriaInicial + columna*(nDistanciaColumnas+nEspaciadoBotones*2)+ nEspaciadoBotones*2 + nDistanciaColumnas + 50, m_rect.right),
                                m_rect.bottom + nTamCheck + nEspaciado*nFilasMax + 50, SWP_NOMOVE | SWP_NOZORDER); //+50 para el menu y barra de titulo
}

BOOL CGestionMineralesDlg::OnCommand(WPARAM wParam, LPARAM lParam) 
{
    int nNumMinerales = theApp.m_minerales.GetCount();

    WORD nId = LOWORD(wParam);
    if (HIWORD(wParam) == BN_CLICKED)
    {
        if (nId >= CONTROLES_DINAMICOS_MIN && (nId <= CONTROLES_DINAMICOS_MIN + (nNumMinerales-1)*3 + 2))
        {
            // Se trata de un control de minerales
            int nIndice = (nId - CONTROLES_DINAMICOS_MIN) / 3;
            int nAccion = (nId - CONTROLES_DINAMICOS_MIN) % 3;
            if (nAccion == 2)
            {
                // Delete
                ProcesarEliminacion(nIndice);
                return TRUE; // al haber eliminado el control no se puede llamar a CDialog::OnCommand
            }
            else if (nAccion == 1)
                // Edit
                ProcesarEdicion(nIndice);
            // else Check. No hacer nada

        }
    }
	
	return CDialog::OnCommand(wParam, lParam);
}

void CGestionMineralesDlg::ProcesarEliminacion(int nIndice) 
{
	if (IDYES == AfxMessageBox("Esta seguro que desea eliminar completamente " + theApp.m_minerales.m_list[nIndice]->m_csNombre, MB_YESNO | MB_ICONQUESTION))
    {
        // eliminamos del dialogo
        BOOL bOk;
        bOk = m_vecDelete[nIndice]->DestroyWindow();
        ASSERT(bOk);
        bOk = m_vecEdit[nIndice]->DestroyWindow();
        ASSERT(bOk);
        bOk = m_vecCheck[nIndice]->DestroyWindow();
        ASSERT(bOk);

        delete m_vecDelete[nIndice];
        delete m_vecEdit[nIndice];
        delete m_vecCheck[nIndice];
        m_vecDelete[nIndice]    = NULL;
        m_vecEdit[nIndice]      = NULL;
        m_vecCheck[nIndice]     = NULL;
        //m_vecDelete.erase(m_vecDelete.begin() + nIndice);
        //m_vecEdit.erase(m_vecEdit.begin() + nIndice);
        //m_vecCheck.erase(m_vecCheck.begin() + nIndice);

        // eliminamos de la lista
        delete theApp.m_minerales.m_list[nIndice];
        theApp.m_minerales.m_list[nIndice] = NULL;
        //theApp.m_minerales.m_list.erase(theApp.m_minerales.m_list.begin() + nIndice);
    }
}
void CGestionMineralesDlg::ProcesarEdicion(int nIndice) 
{
    //Indicamos al objeto m_edicionDlg que mineral se va a editar
    m_edicionDlg.m_pMineral = theApp.m_minerales.m_list[nIndice];

    int nIndiceColor = theApp.m_minerales.GetIndiceColor(nIndice);

	int nResponse = m_edicionDlg.DoModal();
	if (nResponse == IDOK)
	{
        //Modificar nombre check en caso de que haya sido modificado
        CString csInfo;
        csInfo.Format("%-4s - %s",theApp.m_minerales.GetAbreviatura(nIndice), theApp.m_minerales.GetNombre(nIndice));
        m_vecCheck[nIndice]->SetWindowText(csInfo);
	}
	else if (nResponse == IDCANCEL)
	{
	}	
}

void CGestionMineralesDlg::OnNuevoMineral() 
{
    // Crear mineral en minerales
    CMineral* pMineral = new CMineral;
    pMineral->m_nId = theApp.m_minerales.GetId(); //obtiene proximo identificador

    //Indicamos al objeto m_edicionDlg que mineral se va a editar
    m_edicionDlg.m_pMineral = pMineral;

	int nResponse = m_edicionDlg.DoModal();
	if (nResponse == IDOK)
	{
        //Añadirlo a la lista
        theApp.m_minerales.m_list.push_back(pMineral);
        //Despues, añadirlo al dialogo
        NuevoGrupoControles(theApp.m_minerales.GetCount()-1);
	}
	else if (nResponse == IDCANCEL)
	{
        // eliminar el mineral temporal
        delete pMineral;
	}	
}


void CGestionMineralesDlg::OnOK() 
{
    // Guardamos los checks
    for (int i= 0; i<theApp.m_minerales.GetCount();i++)
    {
        if (theApp.m_minerales.m_list[i] != NULL)
            theApp.m_minerales.m_list[i]->m_bActivo = m_vecCheck[i]->GetCheck() == TRUE;
    }

    Liberar();
	
	CDialog::OnOK();
}
/*
void CGestionMineralesDlg::OnClose() 
{
    Liberar();

	CDialog::OnClose();
}
*/
void CGestionMineralesDlg::OnCancel() 
{
	Liberar();
	
	CDialog::OnCancel();
}
