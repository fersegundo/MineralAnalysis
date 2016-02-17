// FormTomaAuto.cpp : implementation file
//

#include "stdafx.h"
#include "TomaAuto.h"
#include "defines.h"
#include "FormTomaAuto.h"
#include "TomaAutoDlg.h"
#include "Progreso.h"
#include "folder_dialog.h"
#include "..\librerias\ControlDisp\control_camara.h"
#include "..\librerias\LibBarrido\calibracion.h" //para get_objetivo_corregido y ESCALA_TEORICA

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CTomaAutoApp theApp;
extern HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)
extern limitesCam	limCam; // declarado en la aplicacion llamante y usado aqui

/////////////////////////////////////////////////////////////////////////////
// CFormTomaAuto dialog


CFormTomaAuto::CFormTomaAuto(CWnd* pParent /*=NULL*/)
	: CFormComun(CFormTomaAuto::IDD, pParent)
{
    m_bCambiadoManualmente  = false;
//    m_enumEsquinaActiva     = ESQUINA_1;
    m_bEsquina1Marcada      = false;
    m_bEsquina2Marcada      = false;
    m_pPadre                = NULL;

    //{{AFX_DATA_INIT(CFormTomaAuto)
	m_nRadio_cobertura_espaciado = 0; //0 es 100
	m_nRadio_cobertura = 0; //0 es cobertura, 1 espaciado
	m_static_campos_x = _T("");
	m_static_campos_y = _T("");
	m_static_dimension_x = _T("");
	m_static_dimension_y = _T("");
	m_bAutoenfoque = FALSE;
	m_csProgreso_actual = _T("");
	m_csProgreso_total = _T("");
	//}}AFX_DATA_INIT
}

//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
LRESULT CFormTomaAuto::UpdateDisplay(WPARAM wParam, LPARAM lParam)
{
    UpdateData((BOOL)wParam);
    return 0;
}


void CFormTomaAuto::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormTomaAuto)
	DDX_Control(pDX, IDC_PROGRESO_ACTUAL, m_progreso_actual);
	DDX_Control(pDX, IDC_AUTOENFOCAR, m_boton_autoenfocar);
	DDX_Control(pDX, IDC_STATIC_CAMPOS, m_progreso_static_campos);
	DDX_Control(pDX, IDC_PROGRESO_TOTAL, m_progreso_total);
	DDX_Control(pDX, IDC_STATIC_PROGRESO, m_progreso_static);
	DDX_Control(pDX, IDC_STATIC_BARRA, m_progreso_barra);
	DDX_Control(pDX, IDC_ESCALA_VENTANA, m_ratio);
	DDX_Control(pDX, IDC_PROGRESO, m_progreso);
	DDX_Control(pDX, IDC_EDIT_PASO_Y, m_paso_y);
	DDX_Control(pDX, IDC_EDIT_PASO_X, m_paso_x);
	DDX_Control(pDX, IDC_DIMENSION_MAX_Y, m_dimension_max_y);
	DDX_Control(pDX, IDC_DIMENSION_MAX_X, m_dimension_max_x);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_X, m_dimension_x);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_Y, m_dimension_y);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_MICRAS_Y, m_dimension_micras_y);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_MICRAS_X, m_dimension_micras_x);
	DDX_Control(pDX, IDC_RADIO_COBERTURA_100, m_radio_cobertura);
	DDX_Control(pDX, IDC_RADIO_COBERTURA, m_radio_cobertura_espaciado);
	DDX_Control(pDX, IDC_EDIT_ESCALA, m_escala);
	DDX_Control(pDX, IDC_BUTTON_MOSAICO, m_boton_mosaico);
	DDX_Control(pDX, IDC_STATIC_POS_X, m_pos_x);
	DDX_Control(pDX, IDC_STATIC_POS_Z, m_pos_z);
	DDX_Control(pDX, IDC_STATIC_POS_Y, m_pos_y);
	DDX_Control(pDX, IDC_CANCELAR, m_cancelar);
	DDX_Control(pDX, IDC_EDIT_NOMBRE, m_nombre);
	DDX_Control(pDX, IDC_STATIC_ESQUINA2_Z, m_esquina2_z);
	DDX_Control(pDX, IDC_STATIC_ESQUINA1_Z, m_esquina1_z);
	DDX_Control(pDX, IDC_EDIT_COBERTURA, m_edit_cobertura);
	DDX_Control(pDX, IDC_INTERMEDIOS, m_intermedios);
	DDX_Control(pDX, IDC_BARRER, m_barrer);
	DDX_Control(pDX, IDC_ENFOCADO, m_boton_enfocado);
	DDX_Control(pDX, IDC_ESQUINA2, m_boton_esquina2);
	DDX_Control(pDX, IDC_ESQUINA, m_boton_esquina);
	DDX_Control(pDX, IDC_STATIC_ESQUINA2_Y, m_esquina2_y);
	DDX_Control(pDX, IDC_STATIC_ESQUINA2_X, m_esquina2_x);
	DDX_Control(pDX, IDC_STATIC_ESQUINA1_Y, m_esquina1_y);
	DDX_Control(pDX, IDC_STATIC_ESQUINA1_X, m_esquina1_x);
	DDX_Control(pDX, IDC_IMAGE, m_imagen);
	DDX_Control(pDX, IDC_EDIT_DIRECTORIO, m_Directorio);
	DDX_Radio(pDX, IDC_RADIO_COBERTURA, m_nRadio_cobertura_espaciado);
	DDX_Radio(pDX, IDC_RADIO_COBERTURA_100, m_nRadio_cobertura);
	DDX_Text(pDX, IDC_STATIC_CAMPOS_X, m_static_campos_x);
	DDX_Text(pDX, IDC_STATIC_CAMPOS_Y, m_static_campos_y);
	DDX_Text(pDX, IDC_STATIC_DIMENSION_X, m_static_dimension_x);
	DDX_Text(pDX, IDC_STATIC_DIMENSION_Y, m_static_dimension_y);
	DDX_Check(pDX, IDC_AUTOENFOQUE, m_bAutoenfoque);
	DDX_Text(pDX, IDC_PROGRESO_ACTUAL, m_csProgreso_actual);
	DDX_Text(pDX, IDC_PROGRESO_TOTAL, m_csProgreso_total);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_INICIO_X,   m_x[0]);
	DDX_Control(pDX, IDC_STATIC_INICIO_Y,   m_y[0]);
	DDX_Control(pDX, IDC_STATIC_INICIO_Z,   m_z[0]);
	DDX_Control(pDX, IDC_STATIC_SC_X,       m_x[1]);
	DDX_Control(pDX, IDC_STATIC_SC_Y,       m_y[1]);
	DDX_Control(pDX, IDC_STATIC_SC_Z,       m_z[1]);
	DDX_Control(pDX, IDC_STATIC_SD_X,       m_x[2]);
	DDX_Control(pDX, IDC_STATIC_SD_Y,       m_y[2]);
	DDX_Control(pDX, IDC_STATIC_SD_Z,       m_z[2]);
	DDX_Control(pDX, IDC_STATIC_CI_X,       m_x[3]);
	DDX_Control(pDX, IDC_STATIC_CI_Y,       m_y[3]);
	DDX_Control(pDX, IDC_STATIC_CI_Z,       m_z[3]);
	DDX_Control(pDX, IDC_STATIC_C_X,        m_x[4]);
	DDX_Control(pDX, IDC_STATIC_C_Y,        m_y[4]);
	DDX_Control(pDX, IDC_STATIC_C_Z,        m_z[4]);
	DDX_Control(pDX, IDC_STATIC_CD_X,       m_x[5]);
	DDX_Control(pDX, IDC_STATIC_CD_Y,       m_y[5]);
	DDX_Control(pDX, IDC_STATIC_CD_Z,       m_z[5]);
	DDX_Control(pDX, IDC_STATIC_II_X,       m_x[6]);
	DDX_Control(pDX, IDC_STATIC_II_Y,       m_y[6]);
	DDX_Control(pDX, IDC_STATIC_II_Z,       m_z[6]);
	DDX_Control(pDX, IDC_STATIC_IC_X,       m_x[7]);
	DDX_Control(pDX, IDC_STATIC_IC_Y,       m_y[7]);
	DDX_Control(pDX, IDC_STATIC_IC_Z,       m_z[7]);
	DDX_Control(pDX, IDC_STATIC_FIN_X,      m_x[8]);
	DDX_Control(pDX, IDC_STATIC_FIN_Y,      m_y[8]);
	DDX_Control(pDX, IDC_STATIC_FIN_Z,      m_z[8]);
	DDX_Control(pDX, IDC_CUADRO_SI, m_cuadro[0]);
	DDX_Control(pDX, IDC_CUADRO_SC, m_cuadro[1]);
	DDX_Control(pDX, IDC_CUADRO_SD, m_cuadro[2]);
	DDX_Control(pDX, IDC_CUADRO_CI, m_cuadro[3]);
	DDX_Control(pDX, IDC_CUADRO_C, m_cuadro[4]);
	DDX_Control(pDX, IDC_CUADRO_CD, m_cuadro[5]);
	DDX_Control(pDX, IDC_CUADRO_II, m_cuadro[6]);
	DDX_Control(pDX, IDC_CUADRO_IC, m_cuadro[7]);
	DDX_Control(pDX, IDC_CUADRO_ID, m_cuadro[8]);
	DDX_Control(pDX, IDC_STATIC_PI, m_static[0]);
	DDX_Control(pDX, IDC_STATIC_SC, m_static[1]);
	DDX_Control(pDX, IDC_STATIC_SD, m_static[2]);
	DDX_Control(pDX, IDC_STATIC_CI, m_static[3]);
	DDX_Control(pDX, IDC_STATIC_C, m_static[4]);
	DDX_Control(pDX, IDC_STATIC_CD, m_static[5]);
	DDX_Control(pDX, IDC_STATIC_II, m_static[6]);
	DDX_Control(pDX, IDC_STATIC_IC, m_static[7]);
	DDX_Control(pDX, IDC_STATIC_PF, m_static[8]);

	DDX_Control(pDX, IDC_STATIC_NM, m_nanometros);
	DDX_Control(pDX, IDC_STATIC_LONGITUD_ONDA, m_longitud_onda);
	DDX_Control(pDX, IDC_FILTRO_ACTUAL, m_filtro_actual);
}


BEGIN_MESSAGE_MAP(CFormTomaAuto, CDialog)
	//{{AFX_MSG_MAP(CFormTomaAuto)
	ON_BN_CLICKED(IDC_DIRECTORIO, OnExaminar)
	ON_EN_CHANGE(IDC_EDIT_DIRECTORIO, OnChangeEditDirectorio)
	ON_BN_CLICKED(IDC_ESQUINA, OnEsquina)
	ON_BN_CLICKED(IDC_ESQUINA2, OnEsquina2)
	ON_BN_CLICKED(IDC_INTERMEDIOS, OnIntermedios)
	ON_BN_CLICKED(IDC_ENFOCADO, OnEnfocado)
	ON_BN_CLICKED(IDC_BARRER, OnBarrer)
	ON_BN_CLICKED(IDC_RADIO_COBERTURA, OnRadioCobertura)
	ON_BN_CLICKED(IDC_RADIO_ESPACIADO, OnRadioEspaciado)
	ON_EN_CHANGE(IDC_EDIT_NOMBRE, OnChangeEditNombre)
	ON_BN_CLICKED(IDC_CANCELAR, OnCancelar)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_X, OnChangeEditDimensionX)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_Y, OnChangeEditDimensionY)
	ON_EN_CHANGE(IDC_EDIT_COBERTURA, OnChangeEditCobertura)
	ON_BN_CLICKED(IDC_RADIO_COBERTURA_30, OnRadioCobertura30)
	ON_BN_CLICKED(IDC_RADIO_COBERTURA_50, OnRadioCobertura50)
	ON_BN_CLICKED(IDC_RADIO_COBERTURA_100, OnRadioCobertura100)
	ON_BN_CLICKED(IDC_RADIO_COBERTURA_EDIT, OnRadioCoberturaEdit)
	ON_BN_CLICKED(IDC_BUTTON_MOSAICO, OnButtonMosaico)
	ON_EN_CHANGE(IDC_EDIT_ESCALA, OnChangeEditEscala)
    ON_MESSAGE(WM_UPDATEFIELDS, UpdateDisplay) //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_MICRAS_X, OnChangeEditDimensionMicrasX)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_MICRAS_Y, OnChangeEditDimensionMicrasY)
	ON_EN_CHANGE(IDC_EDIT_PASO_X, OnChangeEditPasoX)
	ON_EN_CHANGE(IDC_EDIT_PASO_Y, OnChangeEditPasoY)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_ESCALA_VENTANA, OnKillfocusEscalaVentana)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_AUTOENFOCAR, OnAutoenfocar)
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_GOTO_XY, OnMover)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormTomaAuto message handlers

BOOL CFormTomaAuto::OnInitDialog() 
{
	CFormComun::OnInitDialog();
	
    // Por defecto no se autoenfoca en el barrido
    m_bAutoenfoque = FALSE;
    UpdateData(FALSE);

    // Inicializacion variables
    m_Directorio.SetWindowText(theApp.m_configuracion.m_csDirectorioBarrido);

    CString csDimension;
    if (theApp.m_configuracion.m_nDimension_barrido_x > 0)
        csDimension.Format("%d",theApp.m_configuracion.m_nDimension_barrido_x);
    else
        csDimension.Empty();
    m_dimension_x.SetWindowText(csDimension);
    if (theApp.m_configuracion.m_nDimension_barrido_y > 0)
        csDimension.Format("%d",theApp.m_configuracion.m_nDimension_barrido_y);
    else
        csDimension.Empty();
    m_dimension_y.SetWindowText(csDimension);

    // Creamos la pantalla de progreso
    m_pProgreso = new CProgreso;

    // Cramos el dialogo de progreso (se mostrará cuando sea necesario)
    m_progreso_mosaico.Create(IDD_PROGRESO_MOSAICO, this);


CString csAux;
csAux.Format("%d",6);
m_progreso_total.SetWindowText(csAux);



    INIT_EASYSIZE;

    // Ancho y alto del display actualmente (solo como referencia para redimensionar el dialogo)
    RECT Rect;
    m_imagen.GetClientRect(&Rect);
    long anchoImagen = abs(Rect.right);
    long altoImagen = abs(Rect.bottom);
    // Ancho y alto del dialogo  actualmente
    GetClientRect(&Rect);
    long anchoDialogo = abs(Rect.right);
    long altoDialogo = abs(Rect.bottom);

    // Ancho y alto de todo menos el display actualmente
    m_anchoResto = anchoDialogo - anchoImagen;
    m_altoResto = altoDialogo - altoImagen;

    // Calculamos el valor fijo del tamaño del menu y barra de titulo
    GetWindowRect(&Rect);
    m_anchoTitulo = abs(Rect.right) - anchoDialogo;
    m_altoTitulo = abs(Rect.bottom) - altoDialogo;

    // Leemos el ratio configurado y lo aplicamos
    CString csRatio;
    csRatio.Format("%.2f",theApp.m_configuracion.m_ratio_inicial);
    m_ratio.SetWindowText(csRatio);

    // El display tiene que ajustarse al tamaño de imagen y el zoom actual
    anchoImagen = (long)floor(theApp.m_ParamIni.Cam.anchoImagen * theApp.m_configuracion.m_ratio_inicial  + 0.5);
    altoImagen = (long)floor(theApp.m_ParamIni.Cam.altoImagen * theApp.m_configuracion.m_ratio_inicial + 0.5);
    //Ajustamos la ventana principal tambien
    m_imagen.SetWindowPos(NULL,0,0,anchoImagen,altoImagen, SWP_NOMOVE | SWP_NOZORDER); //primero imagen para que OnSize funciones correctamente
    SetWindowPos(NULL,0,0,anchoImagen+m_anchoResto,altoImagen+m_altoResto, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);
    m_pPadre->SetSize(anchoImagen+m_anchoResto+m_anchoTitulo,altoImagen+m_altoResto+m_altoTitulo);
	
	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFormTomaAuto::OnDestroy() 
{
	CDialog::OnDestroy();
	
    m_pProgreso->Fin(); //libera memoria asociada con el display de progreso
    m_progreso_mosaico.DestroyWindow();
    delete m_pProgreso;

    fin_toma();
}

//Acciones necesarias al mostrar la ventana
void CFormTomaAuto::OnShowWindow(BOOL bShow, UINT nStatus) 
{
	CDialog::OnShowWindow(bShow, nStatus);
	
    if (nStatus == 0)
    {
        if (bShow == TRUE)
        {
            MdispSelectWindow(M_display_normal,M_imagen1,m_imagen);

            // Leemos el ratio configurado y lo aplicamos
            CString csRatio;
            csRatio.Format("%.2f",theApp.m_configuracion.m_ratio_inicial);
            m_ratio.SetWindowText(csRatio);
            
            MostrarFiltro(theApp.Rueda.GetFilter()); //Actualizamos el filtro actual
            ActualizarEscala(); //en dialogo usando theApp.m_CorreccionGeometrica.m_dEscalaObjetivo. Esto se hace aqui y no en OnInit porque se puede haber modificado en correccion geometrica

            // El display tiene que ajustarse al tamaño de imagen y el zoom actual
            long anchoImagen = (long)floor(theApp.m_ParamIni.Cam.anchoImagen * theApp.m_configuracion.m_ratio_inicial  + 0.5);
            long altoImagen = (long)floor(theApp.m_ParamIni.Cam.altoImagen * theApp.m_configuracion.m_ratio_inicial + 0.5);
            //Ajustamos la ventana principal tambien
            m_imagen.SetWindowPos(NULL,0,0,anchoImagen,altoImagen, SWP_NOMOVE | SWP_NOZORDER); //primero imagen para que OnSize funciones correctamente
            //Ajustamos la ventana principal tambien
            SetWindowPos(NULL,0,0,anchoImagen+m_anchoResto,altoImagen+m_altoResto, SWP_NOMOVE | SWP_NOZORDER | SWP_NOSENDCHANGING | SWP_NOACTIVATE);

            // Borramos el mosaico de progreso
            m_pProgreso->Borrar();
        }
        else
        {
            MdispDeselect(M_display_normal,M_imagen1);
        }
    }
}

// Ncesario para que [ENTER] y [ESC] no cierren el dialogo
BOOL CFormTomaAuto::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// Mostrar explicitamente la posicion dada en el dialogo
void CFormTomaAuto::MostrarPosicion(double x, double y, double z)
{
    point point_dummy;

    CString csValue;
    csValue.Format("%.2f",x);
    m_pos_x.SetWindowText((LPCTSTR)csValue);
    csValue.Format("%.2f",y);
    m_pos_y.SetWindowText((LPCTSTR)csValue);
    csValue.Format("%.2f",z);
    m_pos_z.SetWindowText((LPCTSTR)csValue);

    // Actualizamos info barrido en tiempo real solo si se esta definiendo una esquina
    // si ya se han definido las dos, no
    if (m_bEsquina1Marcada && m_bEsquina2Marcada == false)
    {
        // Se esta definiendo la segunda esquina: Establecemos el punto actual como fin temporalmente 
        barrido.SetFin(point_dummy);
        ActualizarInfoBarrido();
    }
    else if (m_bEsquina1Marcada == false && m_bEsquina2Marcada)
    {
        // Se esta definiendo la primera esquina: Establecemos el punto actual como inicio temporalmente 
        barrido.SetInicio(point_dummy);
        ActualizarInfoBarrido();
    }

}

// Mostrar explicitamente el filtro dado en el dialogo
void CFormTomaAuto::MostrarFiltro(int nFiltro)
{
    CString csValue;
    csValue.Format("%d", nFiltro+1);
    m_filtro_actual.SetWindowText((LPCTSTR)csValue);
    if (theApp.m_ParamIni.Rueda.espectro[nFiltro] == 0)
    {
        csValue.Format("visible");
        m_nanometros.ShowWindow(SW_HIDE);
    }
    else
    {
        csValue.Format("%d", theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_nanometros.ShowWindow(SW_SHOW);
    }
    m_longitud_onda.SetWindowText((LPCTSTR)csValue);
}

// Incrementa en 1 y muestra el campo que se esta barriendo
void CFormTomaAuto::InicializaProgresion(int nCamposTotales)
{
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0); //Update variables from display, in case something has changed
    if (nCamposTotales < 1)
    {
        m_csProgreso_actual.Empty();
        m_csProgreso_total.Empty();
//        m_progreso_total.SetWindowText("");
    }
    else
    {
        m_csProgreso_actual.Format("%d",0);
        m_csProgreso_total.Format("%d",nCamposTotales);
//        CString csAux;
//        csAux.Format("%d",nCamposTotales);
//        m_progreso_total.SetWindowText(csAux);
    }
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display
}

// Incrementa en 1 y muestra el campo que se esta barriendo
void CFormTomaAuto::IncrementaProgresion()
{
    int nCampoActual = atoi(m_csProgreso_actual);

    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0); //Update variables from display, in case something has changed
    m_csProgreso_actual.Format("%d",nCampoActual+1);
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display
}



////////////////////////////////////////////////////////////////////////////////////
//
//      CONFIGURACION DEL BARRIDO
//
////////////////////////////////////////////////////////////////////////////////////

// Cambia el directorio de trabajo
void CFormTomaAuto::OnExaminar() 
{
	CFolderDialog dlg(&theApp.m_configuracion.m_csDirectorioBarrido);
	if (dlg.DoModal() == IDOK)
	{
		// pathSelected contain the selected folder.
		//theApp.m_configuracion.m_csDirectorioBarrido = pathSelected;
        m_Directorio.SetWindowText(theApp.m_configuracion.m_csDirectorioBarrido);
	}  
}

//Para actualizar el directorio de trabajo si se modifica manualmente
void CFormTomaAuto::OnChangeEditDirectorio() 
{
    LPTSTR temp = theApp.m_configuracion.m_csDirectorioBarrido.GetBuffer( 1024 );
	m_Directorio.GetLine(0,temp);	
    theApp.m_configuracion.m_csDirectorioBarrido.ReleaseBuffer();  // Surplus memory released, temp is now invalid.

    // Nos aseguramos de que el ultimo caracter sea '\'
    CString csUltimoCaracter = theApp.m_configuracion.m_csDirectorioBarrido.Right(1);
    if (csUltimoCaracter != "\\" && csUltimoCaracter != "/")
        theApp.m_configuracion.m_csDirectorioBarrido += _T("\\");
}

void CFormTomaAuto::OnChangeEditNombre() 
{
    TCHAR temp[MAX_PATH];

    //LPTSTR temp = theApp.csNombreToma.GetBuffer( 1024 );
	m_nombre.GetLine(0,temp);
    //theApp.csNombreToma.ReleaseBuffer();  // Surplus memory released, temp is now invalid.
    theApp.csNombreToma = temp;
}

// El usuario cambia la opcion Cobertura/Espaciado a Cobertura
void CFormTomaAuto::OnRadioCobertura() 
{

    GetDlgItem(IDC_RADIO_COBERTURA_30)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_50)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_100)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_EDIT)->EnableWindow(TRUE);
    m_edit_cobertura.EnableWindow(TRUE);
    m_paso_x.EnableWindow(FALSE);
    m_paso_y.EnableWindow(FALSE);
	
    ActualizarBarrido();
}

void CFormTomaAuto::OnRadioCobertura30() 
{
    ActualizarBarrido();	
}

void CFormTomaAuto::OnRadioCobertura50() 
{
    ActualizarBarrido();	
}

void CFormTomaAuto::OnRadioCobertura100() 
{
    ActualizarBarrido();
}
void CFormTomaAuto::OnRadioCoberturaEdit() 
{
    ActualizarBarrido();	
}

// El usuario cambia la opcion Cobertura/Paso a Paso
void CFormTomaAuto::OnRadioEspaciado() 
{
    GetDlgItem(IDC_RADIO_COBERTURA_30)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO_COBERTURA_50)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO_COBERTURA_100)->EnableWindow(FALSE);
    GetDlgItem(IDC_RADIO_COBERTURA_EDIT)->EnableWindow(FALSE);
    m_edit_cobertura.EnableWindow(FALSE);
    m_paso_x.EnableWindow(TRUE);
    m_paso_y.EnableWindow(TRUE);

    ActualizarBarrido();
}

void CFormTomaAuto::OnChangeEditCobertura() 
{
    ActualizarBarrido();	
}

void CFormTomaAuto::OnChangeEditPasoX() 
{
    ActualizarBarrido();	
}

void CFormTomaAuto::OnChangeEditPasoY() 
{
    ActualizarBarrido();	
}

void CFormTomaAuto::OnChangeEditDimensionX() 
{
    //Obtenemos la dimension del dialogo
    CString csAux;
    m_dimension_x.GetWindowText(csAux);
    if (!m_bCambiadoManualmente)
    {
        theApp.m_configuracion.m_nDimension_barrido_x = atoi((LPCSTR)csAux);
        if (theApp.m_configuracion.m_nDimension_barrido_x > limCam.anchoDig)
        {
            theApp.m_configuracion.m_nDimension_barrido_x = limCam.anchoDig;
            csAux.Format("%d",theApp.m_configuracion.m_nDimension_barrido_x);
            m_bCambiadoManualmente = true;
            m_dimension_x.SetWindowText((LPCTSTR)csAux);
        }

        if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_barrido_x > 0)
        {
            //Actualizar la dimension en micras (dejamos la escala constante)
            csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_barrido_x * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
            m_bCambiadoManualmente = true;
            m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
            ActualizarBarrido();
        }
        else if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0) //theApp.m_configuracion.m_nDimension == 0
        {
            // Borrar dimension en micras
            m_bCambiadoManualmente = true;
            m_dimension_micras_x.SetWindowText("");
            ActualizarBarrido();
        }
        else if (theApp.m_configuracion.m_nDimension_barrido_x > 0) // theApp.m_CorreccionGeometrica.m_dEscalaObjetivo == 0
        {
            m_dimension_micras_x.GetWindowText(csAux);
            if (csAux.IsEmpty == FALSE)
            {
                //Actualizar Escala
                theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_barrido_x;
                csAux.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                m_bCambiadoManualmente = true;
                m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
                ActualizarBarrido();
            }
        }
        else //theApp.m_CorreccionGeometrica.m_dEscalaObjetivo == 0 && theApp.m_configuracion.m_nDimension == 0
        {
            // Nada
        }
    
    }
    else
    {
        m_bCambiadoManualmente = false;
    }
}

void CFormTomaAuto::OnChangeEditDimensionY() 
{
    //Obtenemos la dimension del dialogo
    CString csAux;
    m_dimension_y.GetWindowText(csAux);
    if (!m_bCambiadoManualmente)
    {
        theApp.m_configuracion.m_nDimension_barrido_y = atoi((LPCSTR)csAux);
        if (theApp.m_configuracion.m_nDimension_barrido_y > limCam.altoDig)
        {
            theApp.m_configuracion.m_nDimension_barrido_y = limCam.altoDig;
            csAux.Format("%d",theApp.m_configuracion.m_nDimension_barrido_y);
            m_bCambiadoManualmente = true;
            m_dimension_y.SetWindowText((LPCTSTR)csAux);
        }

        if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_barrido_y > 0)
        {
            //Actualizar la dimension en micras (dejamos la escala constante)
            csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_barrido_y * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
            m_bCambiadoManualmente = true;
            m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
            ActualizarBarrido();
        }
        else if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0) //theApp.m_configuracion.m_nDimension == 0
        {
            // Borrar dimension en micras
            m_bCambiadoManualmente = true;
            m_dimension_micras_y.SetWindowText("");
            ActualizarBarrido();
        }
        else if (theApp.m_configuracion.m_nDimension_barrido_y > 0) // theApp.m_CorreccionGeometrica.m_dEscalaObjetivo == 0
        {
            m_dimension_micras_y.GetWindowText(csAux);
            if (csAux.IsEmpty == FALSE)
            {
                //Actualizar Escala
                theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_barrido_y;
                csAux.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                m_bCambiadoManualmente = true;
                m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
                ActualizarBarrido();
            }
        }
        else //theApp.m_CorreccionGeometrica.m_dEscalaObjetivo == 0 && theApp.m_configuracion.m_nDimension == 0
        {
            // Nada
        }
    
    }
    else
    {
        m_bCambiadoManualmente = false;
    }
	
}

void CFormTomaAuto::OnChangeEditDimensionMicrasX() 
{
    if (!m_bCambiadoManualmente)
    {
        //Actualizar la dimension en pixels (dejamos la escala constante)
        CString csAux;
        m_dimension_micras_x.GetWindowText(csAux);

        // No permitimos caracteres no decimales
        int nLength = csAux.GetLength();
        if (csAux.IsEmpty())
        {
            if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_barrido_x > 0)
            {
                // Borrar dimension en pixels
                m_bCambiadoManualmente = true;
                theApp.m_configuracion.m_nDimension_barrido_x = 0;
                m_dimension_x.SetWindowText("");
                ActualizarBarrido();
            }
            else
            {
                //Nada
            }
        }
        else
        {
            if (csAux[nLength-1]==',')
            {
                //cambiar , por .
                csAux.SetAt(nLength-1, '.');
                m_bCambiadoManualmente = true;
                m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
                m_dimension_micras_x.SetSel(nLength,nLength); //para que se desplace el cursor al final
            }
            if (csAux[nLength-1]=='.')
            {
                //no actualizar, esperar a los decimales
            }
            else if (csAux[nLength-1] > 47 && csAux[nLength-1]<58) // 0-9
            {
                if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 )
                {
                    theApp.m_configuracion.m_nDimension_barrido_x = (int)(atof((LPCSTR)csAux) / theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    csAux.Format("%d",theApp.m_configuracion.m_nDimension_barrido_x);
                    m_bCambiadoManualmente = true;
                    m_dimension_x.SetWindowText((LPCTSTR)csAux);
                    ActualizarBarrido();
                }
                else
                {
                    if (theApp.m_configuracion.m_nDimension_barrido_x > 0)
                    {
                        //Actualizar Escala
                        theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_barrido_x;
                        csAux.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                        m_bCambiadoManualmente = true;
                        m_escala.SetWindowText((LPCTSTR)csAux);
                        ActualizarBarrido();
                    }
                }
            }
            else
            {
                // No permitimos el caracter, asi que lo eliminamos
                csAux.Delete(nLength-1);
                m_bCambiadoManualmente = true;
                m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
                m_dimension_micras_x.SetSel(nLength,nLength); //para que se desplace el cursor al final
            }
        }
    }
    else
    {
        m_bCambiadoManualmente = false;
    }
}

void CFormTomaAuto::OnChangeEditDimensionMicrasY() 
{
    if (!m_bCambiadoManualmente)
    {
        //Actualizar la dimension en pixels (dejamos la escala constante)
        CString csAux;
        m_dimension_micras_y.GetWindowText(csAux);

        // No permitimos caracteres no decimales
        int nLength = csAux.GetLength();
        if (csAux.IsEmpty())
        {
            if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_barrido_y > 0)
            {
                // Borrar dimension en pixels
                m_bCambiadoManualmente = true;
                theApp.m_configuracion.m_nDimension_barrido_y = 0;
                m_dimension_y.SetWindowText("");
                ActualizarBarrido();

            }
            else
            {
                //Nada
            }
        }
        else
        {
            if (csAux[nLength-1]==',')
            {
                //cambiar , por .
                csAux.SetAt(nLength-1, '.');
                m_bCambiadoManualmente = true;
                m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
                m_dimension_micras_y.SetSel(nLength,nLength); //para que se desplace el cursor al final
            }
            if (csAux[nLength-1]=='.')
            {
                //no actualizar, esperar a los decimales
            }
            else if (csAux[nLength-1] > 47 && csAux[nLength-1]<58) // 0-9
            {
                if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 )
                {
                    theApp.m_configuracion.m_nDimension_barrido_y = (int)(atof((LPCSTR)csAux) / theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    csAux.Format("%d",theApp.m_configuracion.m_nDimension_barrido_y);
                    m_bCambiadoManualmente = true;
                    m_dimension_y.SetWindowText((LPCTSTR)csAux);
                    ActualizarBarrido();
                }
                else
                {
                    if (theApp.m_configuracion.m_nDimension_barrido_y > 0)
                    {
                        //Actualizar Escala
                        theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_barrido_y;
                        csAux.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                        m_bCambiadoManualmente = true;
                        m_escala.SetWindowText((LPCTSTR)csAux);
                        ActualizarBarrido();
                    }
                }
            }
            else
            {
                // No permitimos el caracter, asi que lo eliminamos
                csAux.Delete(nLength-1);
                m_bCambiadoManualmente = true;
                m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
                m_dimension_micras_y.SetSel(nLength,nLength); //para que se desplace el cursor al final
            }
        }
    }
    else
    {
        m_bCambiadoManualmente = false;
    }
	
}

void CFormTomaAuto::OnChangeEditEscala() 
{
    if (!m_bCambiadoManualmente)
    {
        //Obtenemos la escala del dialogo
        CString csAux;
        m_escala.GetWindowText(csAux);
    
        // No permitimos caracteres no decimales
        int nLength = csAux.GetLength();
        if (nLength == 0)
        {
            theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = 0;
            // X
            if (theApp.m_configuracion.m_nDimension_barrido_x > 0)
            {
                // Si no hay informacion de escala, no puede haberla de dimension en micras
                m_bCambiadoManualmente = true;
                m_dimension_micras_x.SetWindowText("");

            }
            // Y
            if (theApp.m_configuracion.m_nDimension_barrido_y > 0)
            {
                // Si no hay informacion de escala, no puede haberla de dimension en micras
                m_bCambiadoManualmente = true;
                m_dimension_micras_y.SetWindowText("");
            }
            //Borramos las dimensiones maximas
            m_dimension_max_x.SetWindowText("");
            m_dimension_max_y.SetWindowText("");

            ActualizarBarrido();	
        }
        else
        {
            if (csAux[nLength-1]==',')
            {
                //cambiar , por .
                csAux.SetAt(nLength-1, '.');
                m_bCambiadoManualmente = true;
                m_escala.SetWindowText((LPCTSTR)csAux);
                m_escala.SetSel(nLength,nLength); //para que se desplace el cursor al final
            }
            if (csAux[nLength-1]=='.')
            {
                //no actualizar, esperar a los decimales
            }
            else if (csAux[nLength-1] > 47 && csAux[nLength-1]<58) // 0-9
            {
                theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof((LPCSTR)csAux); //actualizamos
                if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo < MIN_ESCALA_REFLECTANCIA)
                {
                    theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = MIN_ESCALA_REFLECTANCIA;
                    csAux.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_escala.SetWindowText((LPCTSTR)csAux);
                }
                if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > MAX_ESCALA_REFLECTANCIA)
                {
                    theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = MAX_ESCALA_REFLECTANCIA;
                    csAux.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_escala.SetWindowText((LPCTSTR)csAux);
                }
                // X
                if (theApp.m_configuracion.m_nDimension_barrido_x > 0)
                {
                    //Actualizar la dimension en micras (dejamos la dimension en pixels constante)
                    csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_barrido_x * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
                    ActualizarBarrido();	

                }
                // Y
                if (theApp.m_configuracion.m_nDimension_barrido_y > 0)
                {
                    //Actualizar la dimension en micras (dejamos la dimension en pixels constante)
                    csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_barrido_y * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
                    ActualizarBarrido();	

                }
                //Actualizamos las dimensiones maximas (que unicamente dependen de la escala)
                csAux.Format("%d",theApp.CalcularAnchoMax());
                m_dimension_max_x.SetWindowText((LPCTSTR)csAux);
                csAux.Format("%d",theApp.CalcularAltoMax());
                m_dimension_max_y.SetWindowText((LPCTSTR)csAux);

                // No actualizamos dimension en pixels nunca
            }
            else
            {
                // No permitimos el caracter, asi que lo eliminamos
                csAux.Delete(nLength-1);
                m_bCambiadoManualmente = true;
                m_escala.SetWindowText((LPCTSTR)csAux);
                m_escala.SetSel(nLength,nLength); //para que se desplace el cursor al final
            }
        }
    }
    else
    {
        m_bCambiadoManualmente = false;
    }
}

//en dialogo usando theApp.m_CorreccionGeometrica.m_dEscalaObjetivo
void CFormTomaAuto::ActualizarEscala()
{
    CString csEscala;
    csEscala.Format("%.5f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
    m_escala.SetWindowText(csEscala);
}

void CFormTomaAuto::ActualizarBarrido() 
{
    //Actualizar la informacion de numero de campos y tamaño total de barrido 
    //SOLO SI esta definida el area de barrido
    if (m_barrer.IsWindowVisible() == TRUE || 
        m_intermedios.IsWindowEnabled() == TRUE ||
        m_boton_enfocado.IsWindowEnabled() == TRUE)
    {
        //Actualizar la informacion de numero de campos y tamaño total de barrido 
        bool bInfoDimension = ActualizarInfoBarrido();


        //Habilitar boton "Barrer" si:
        // - hay informacion de dimension 
        // - ya se ha definido completamente el plano de regresion
        if (bInfoDimension == true)
        {
            if (m_barrer.IsWindowVisible()  == TRUE)
                m_barrer.EnableWindow(TRUE);
        }
        else
            m_barrer.EnableWindow(FALSE);
    }
}

// AUXILIAR
// A partir de los campos de cobertura/espaciado y Dimension, calculamos 
// y presentamos la info de numero de campos y dimensiones de barrido 
// Devuelve true si hay info de Dimension y se ha podido actualizar
bool CFormTomaAuto::ActualizarInfoBarrido() 
{
    bool bRet = false;

    if (theApp.m_configuracion.m_nDimension_barrido_x > 0 && theApp.m_configuracion.m_nDimension_barrido_y > 0 && theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0)
    {

        //Obtenemos la cobertura / espaciado
        int nCobertura = -1;
        double dPaso_x = -1;
        double dPaso_y = -1;
        ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0);
        //UpdateData(TRUE);
        if (m_nRadio_cobertura_espaciado == 0)
        {
            if(m_nRadio_cobertura == 0)
                nCobertura = 100;
            else if(m_nRadio_cobertura == 1)
                nCobertura = 50;
            else if(m_nRadio_cobertura == 2)
                nCobertura = 30;
            else if(m_nRadio_cobertura == 3)
            {
                CString csCobertura;
                m_edit_cobertura.GetWindowText(csCobertura);
                nCobertura = atoi((LPCSTR)csCobertura);
                if (nCobertura <= 0)
                {
                    //No hay info de cobertura
                    m_static_campos_x.Empty();
                    m_static_campos_y.Empty();
                    m_static_dimension_x.Empty();
                    m_static_dimension_y.Empty();

                    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display
                    return false;
                }
            }
        }
        else
        {
            CString csEspaciado;
            m_paso_x.GetWindowText(csEspaciado);
            dPaso_x = atof((LPCSTR)csEspaciado);
            m_paso_y.GetWindowText(csEspaciado);
            dPaso_y = atof((LPCSTR)csEspaciado);
            if (dPaso_x <= 0 || dPaso_y <= 0)
            {
                //No hay info de cobertura
                m_static_campos_x.Empty();
                m_static_campos_y.Empty();
                m_static_dimension_x.Empty();
                m_static_dimension_y.Empty();

                ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display
                return false;
            }
        }

        // Calculamos el numero de campos y dimensiones de barrido
        int nCampos_x = -1;
        int nCampos_y = -1;
        double dAnchoTotal = -1;
        double dAltoTotal = -1;
        barrido.CalcularCampos(nCobertura,dPaso_x,dPaso_y,nCampos_x,nCampos_y,dAnchoTotal,dAltoTotal);

        //Mostramos la info en el dialogo
        m_static_campos_x.Format("%d",nCampos_x);
        m_static_campos_y.Format("%d",nCampos_y);
        m_static_dimension_x.Format("%.2f",dAnchoTotal);
        m_static_dimension_y.Format("%.2f",dAltoTotal);

        bRet = true;
    }
    else
    {
        //Mostramos la info en el dialogo
        m_static_campos_x.Empty();
        m_static_campos_y.Empty();
        m_static_dimension_x.Empty();
        m_static_dimension_y.Empty();

        bRet = false;
    }

    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display

    return bRet;
}

// AUXILIAR
// Se vacian los valores de las coordenadas de los puntos intermedios
void CFormTomaAuto::BorrarIntermedios() 
{
    for (int i= 0;i<NUM_PUNTOS_PLANO;i++)
    {
        m_x[i].SetWindowText("");
        m_y[i].SetWindowText("");
        m_z[i].SetWindowText("");
    }
}





////////////////////////////////////////////////////////////////////////////////////
//
//      BOTONES DE BARRIDO
//
////////////////////////////////////////////////////////////////////////////////////


// se establece una esquina 1 del barrido
void CFormTomaAuto::OnEsquina() 
{
    point pActual;

    // Establecemos el punto actual como inicio temporalmente (cuando esten las dos esquinas
    // seleccionadas se recacularan inicio y fin)
    // Request ownership of mutex.
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        bool bSinIntermedios = barrido.SetInicio(pActual); //obtiene pActual

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnEsquina"));

        CString csValue;
        csValue.Format("%.2f",pActual.coord[X_]);
	    m_esquina1_x.SetWindowText((LPCTSTR)csValue);
        csValue.Format("%.2f",pActual.coord[Y_]);
	    m_esquina1_y.SetWindowText((LPCTSTR)csValue);
        csValue.Format("%.2f",pActual.coord[Z_]);
	    m_esquina1_z.SetWindowText((LPCTSTR)csValue);

        // si la otra esquina se ha definido, se actualiza la info de barrido y se habilita el boton de toma
        if (m_bEsquina2Marcada)
        {
            bool bInfoDimension = ActualizarInfoBarrido();
            if (bSinIntermedios)
            {
                m_barrer.ShowWindow(SW_SHOW);
                if (bInfoDimension)
                    m_barrer.EnableWindow(TRUE); //procedemos directamente al barrido
                m_intermedios.ShowWindow(SW_HIDE);
                m_intermedios.EnableWindow(FALSE); //por si se habia habilitado antes (el usuario estableció las dos esquinas con distinta Z, pero luego las igualó)
            }
            else
            {
                m_barrer.ShowWindow(SW_HIDE);
                m_barrer.EnableWindow(FALSE); //por si se habia habilitado antes (el usuario estableció las dos esquinas con la misma Z, pero luego cambió el foco)
                m_intermedios.ShowWindow(SW_SHOW);
                m_intermedios.EnableWindow(TRUE);
            }
        }
        m_bEsquina1Marcada = true;
    }
    else
        sendlog("OnEsquina","Fallo al esperar por el MutexMicroscopio. TIMEOUT");
}

// se establece una esquina 2 del barrido
void CFormTomaAuto::OnEsquina2() 
{
    point pActual;

    // Establecemos el punto actual como fin temporalmente (cuando esten las dos esquinas
    // seleccionadas se recacularan inicio y fin)
    // Request ownership of mutex.
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        bool bSinIntermedios = barrido.SetFin(pActual);
        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnEsquina2") );
        CString csValue;
        csValue.Format("%.2f",pActual.coord[X_]);
	    m_esquina2_x.SetWindowText((LPCTSTR)csValue);
        csValue.Format("%.2f",pActual.coord[Y_]);
	    m_esquina2_y.SetWindowText((LPCTSTR)csValue);	
        csValue.Format("%.2f",pActual.coord[Z_]);
	    m_esquina2_z.SetWindowText((LPCTSTR)csValue);	

        // si la otra esquina se ha definido, se actualiza la info de barrido y se habilita el boton de toma
        if (m_bEsquina1Marcada)
        {
            bool bInfoDimension = ActualizarInfoBarrido();
            if (bSinIntermedios)
            {
                m_barrer.ShowWindow(SW_SHOW);
                if (bInfoDimension)
                    m_barrer.EnableWindow(TRUE); //procedemos directamente al barrido
                m_intermedios.ShowWindow(SW_HIDE);
                m_intermedios.EnableWindow(FALSE); //por si se habia habilitado antes (el usuario estableció las dos esquinas con distinta Z, pero luego las igualó)
            }
            else
            {
                m_intermedios.ShowWindow(SW_SHOW);
                m_intermedios.EnableWindow(TRUE);
                m_barrer.ShowWindow(SW_HIDE);
                m_barrer.EnableWindow(FALSE); //por si se habia habilitado antes (el usuario estableció las dos esquinas con la misma Z, pero luego cambió el foco)
            }
        }
        m_bEsquina2Marcada = true;
    }
    else
        sendlog("OnEsquina","Fallo al esperar por el MutexMicroscopio. TIMEOUT");
}

// El usuario presiona este boton cuando se han definido las esquinas que definen el barrido
void CFormTomaAuto::OnIntermedios() 
{
    int i;
    CString csValue = "";

    //  recalcula las posiciones del inicio y fin (en m_arrPuntosControl)
    barrido.Intermedios();
    
    // borramos valores esquinas
	m_esquina1_x.SetWindowText((LPCTSTR)csValue);
	m_esquina1_y.SetWindowText((LPCTSTR)csValue);
	m_esquina1_z.SetWindowText((LPCTSTR)csValue);
	m_esquina2_x.SetWindowText((LPCTSTR)csValue);
	m_esquina2_y.SetWindowText((LPCTSTR)csValue);
	m_esquina2_z.SetWindowText((LPCTSTR)csValue);
    

    //ponemos valores inicio, fin y puntos intermedios tambien
    for (i=0;i<NUM_PUNTOS_PLANO;i++)
    {
        csValue.Format("%.2f",barrido.m_arrPuntosControl[i].coord[X_]);
	    m_x[i].SetWindowText((LPCTSTR)csValue);
        csValue.Format("%.2f",barrido.m_arrPuntosControl[i].coord[Y_]);
	    m_y[i].SetWindowText((LPCTSTR)csValue);	
        if (barrido.m_arrPuntosControl[i].coord[Z_] != -1)
        {
            csValue.Format("%.3f",barrido.m_arrPuntosControl[i].coord[Z_]);
	        m_z[i].SetWindowText((LPCTSTR)csValue);	
        }
    }

    // Preparamos el dialogo para el enfoque de los puntos intermedios
    m_intermedios.EnableWindow(FALSE);
    m_intermedios.ShowWindow(SW_HIDE);
    m_boton_esquina.EnableWindow(FALSE);
    m_boton_esquina2.EnableWindow(FALSE);
    m_boton_esquina.ShowWindow(SW_HIDE);
    m_boton_esquina2.ShowWindow(SW_HIDE);
    m_boton_enfocado.EnableWindow(TRUE);
    m_boton_enfocado.ShowWindow(SW_SHOW);

    //Mostramos puntos intermedios
    for (i= 0;i<NUM_PUNTOS_PLANO;i++)
    {
        m_x[i].ShowWindow(SW_SHOW);
        m_y[i].ShowWindow(SW_SHOW);
        m_z[i].ShowWindow(SW_SHOW);
        m_cuadro[i].ShowWindow(SW_SHOW);
        m_static[i].ShowWindow(SW_SHOW);
    }

    //Ocultamos progreso
    m_progreso.ShowWindow(SW_HIDE);
    m_progreso_barra.ShowWindow(SW_HIDE);
    m_progreso_static.ShowWindow(SW_HIDE);
    m_progreso_actual.ShowWindow(SW_HIDE);
    m_progreso_total.ShowWindow(SW_HIDE);
    m_progreso_static_campos.ShowWindow(SW_HIDE);
    m_boton_mosaico.ShowWindow(SW_HIDE);

    // Iniciamos enfoque de puntos intermedios
    // Request ownership of mutex.
    int nIndicePuntoSiguiente = -1;
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        int nDummy = -1;
        nIndicePuntoSiguiente = barrido.EnfoquePuntosIntermedios(nDummy);
        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnIntermedios") );
    }

    // Indicacion visual de punto a enfocar
    m_cuadro[nIndicePuntoSiguiente].ShowWindow(SW_SHOW);
}

void CFormTomaAuto::OnAutoenfocar() 
{
    m_pPadre->Autoenfocar();
}

void CFormTomaAuto::OnMover() 
{
    m_pPadre->Mover();
   	
}

// El usuario ha enfocado un punto intermedio 
void CFormTomaAuto::OnEnfocado() 
{
    int nIndicePuntoEnfocado = -1;

    // Request ownership of mutex.
    int nIndicePuntoSiguiente = -1;
    if (WaitForSingleObject(g_hMutexMicroscopio,500L) != WAIT_OBJECT_0)// half-second time-out interval
    {
        AfxMessageBox("Microscopio en uso!");
        return;
    }

    nIndicePuntoSiguiente = barrido.EnfoquePuntosIntermedios(nIndicePuntoEnfocado);
    // Release ownership of the mutex object.
    if (! ReleaseMutex(g_hMutexMicroscopio))
        ASSERT(AfxMessageBox("ReleaseMutex error al OnEnfocado") );

    // Quitamos indicacion visual de punto enfocado
    m_cuadro[nIndicePuntoEnfocado].ShowWindow(SW_HIDE);    
    if (nIndicePuntoSiguiente == -1)
    {
        //No hay mas puntos a enfocar
        m_boton_enfocado.EnableWindow(FALSE);
        m_barrer.ShowWindow(SW_SHOW);
        // Si no se ha rellenado el campo de Dimension o Cobertura editable o Espaciado(y por tanto no hay informacion 
        // de numero de campos), no se habilita el boton de barrer 
        //(se habilitará al rellenar el campo)
// if (m_static_campos_x.IsEmpty() == FALSE && m_static_campos_y.IsEmpty() == FALSE)
        if (theApp.m_configuracion.m_nDimension_barrido_x > 0 && theApp.m_configuracion.m_nDimension_barrido_y > 0 && theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0)
            m_barrer.EnableWindow(TRUE);
    }
    else
    {
        // Indicacion visual de punto a enfocar
        m_cuadro[nIndicePuntoSiguiente].ShowWindow(SW_SHOW);    
    }

    //Actualizar x,y, z en el dialogo
    CString csValueX,csValueY,csValueZ = "";
    csValueX.Format("%.2f",barrido.m_arrPuntosControl[nIndicePuntoEnfocado].coord[X_]);
    csValueY.Format("%.2f",barrido.m_arrPuntosControl[nIndicePuntoEnfocado].coord[Y_]);
    csValueZ.Format("%.3f",barrido.m_arrPuntosControl[nIndicePuntoEnfocado].coord[Z_]);

    m_x[nIndicePuntoEnfocado].SetWindowText((LPCTSTR)csValueX);	
    m_y[nIndicePuntoEnfocado].SetWindowText((LPCTSTR)csValueY);	
    m_z[nIndicePuntoEnfocado].SetWindowText((LPCTSTR)csValueZ);	
}

//Se han definido los puntos intermedios y se procede a barrer
void CFormTomaAuto::OnBarrer() 
{
    UpdateData(TRUE);

    //Finaliza el thread de posicion 
    m_pPadre->FinalizarThreadPosicion();

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        if (get_objetivo_corregido() == 0) // Comprobar si el thread de correccion finalizó correctamente y se pueden corregir imagenes
        {
            AfxMessageBox("Imposible efectuar barrido: no hay informacion de correccion - Reinicie la aplicacion");
        }
        //Comprobamos que las dimensiones estan dentro de los limites maximos calculados (con correccion incluida)
        else if (theApp.CalcularAnchoMax() < theApp.m_configuracion.m_nDimension_barrido_x)
        {
            AfxMessageBox("Imposible efectuar barrido: Ancho de imagen mayor que maximo - Revise los valores de Tamaño");
        }
        else if (theApp.CalcularAltoMax() < theApp.m_configuracion.m_nDimension_barrido_y  ) 
        {
            AfxMessageBox("Imposible efectuar barrido: Alto de imagen mayor que maximo - Revise los valores de Tamaño");
        }
        else
        {
            if (barrido.m_arrPuntosControl[0].coord[Z_] == barrido.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_])
            {
                // No se estan usando puntos intermedios (para el plano de regresion)

                // desactivamos los botones y valores de las esquinas
	            m_esquina1_x.SetWindowText("");
	            m_esquina1_y.SetWindowText("");
	            m_esquina1_z.SetWindowText("");
	            m_esquina2_x.SetWindowText("");
	            m_esquina2_y.SetWindowText("");
	            m_esquina2_z.SetWindowText("");
                m_boton_esquina.EnableWindow(FALSE);
                m_boton_esquina2.EnableWindow(FALSE);
                m_boton_esquina.ShowWindow(SW_HIDE);
                m_boton_esquina2.ShowWindow(SW_HIDE);

                //Establecemos inicio y fin (recalculando las posiciones del inicio y fin (en m_arrPuntosControl))
                barrido.Intermedios();

                CString csValue = "";
                csValue.Format("%.2f",barrido.m_arrPuntosControl[0].coord[X_]);
	            m_x[0].SetWindowText((LPCTSTR)csValue);
                csValue.Format("%.2f",barrido.m_arrPuntosControl[0].coord[Y_]);
	            m_y[0].SetWindowText((LPCTSTR)csValue);	
                if (barrido.m_arrPuntosControl[0].coord[Z_] != -1) //no deberia ocurrir nunca lo contrario
                {
                    csValue.Format("%.3f",barrido.m_arrPuntosControl[0].coord[Z_]);
	                m_z[0].SetWindowText((LPCTSTR)csValue);	
                }
                csValue.Format("%.2f",barrido.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[X_]);
	            m_x[NUM_PUNTOS_PLANO-1].SetWindowText((LPCTSTR)csValue);
                csValue.Format("%.2f",barrido.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Y_]);
	            m_y[NUM_PUNTOS_PLANO-1].SetWindowText((LPCTSTR)csValue);	
                if (barrido.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_] != -1)
                {
                    csValue.Format("%.3f",barrido.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_]);
	                m_z[NUM_PUNTOS_PLANO-1].SetWindowText((LPCTSTR)csValue);	
                }
            }
            else
            {
                //borramos puntos intermedios del plano (dejando solo inicio y fin)
                for (int i=1;i<NUM_PUNTOS_PLANO-1;i++)
                {
	                m_x[i].SetWindowText("");
	                m_y[i].SetWindowText("");	
	                m_z[i].SetWindowText("");	
                }
            }

            m_pProgreso->Inicializar(barrido.GetCamposX(),barrido.GetCamposY(),m_progreso);
            if (barrido.Barrer(this))    //THREAD. <---------
            {
                // Preparamos el dialogo para el barrido
                m_barrer.ShowWindow(SW_HIDE);
                m_barrer.EnableWindow(FALSE);
                m_cancelar.EnableWindow(TRUE);
                m_cancelar.ShowWindow(SW_SHOW);
                m_boton_autoenfocar.EnableWindow(FALSE);

                //Deshabilitamos la configuracion del barrido (tamaño, escala, cobertura,...)
                m_escala.EnableWindow(FALSE);
                m_dimension_x.EnableWindow(FALSE);
                m_dimension_y.EnableWindow(FALSE);
                m_dimension_micras_x.EnableWindow(FALSE);
                m_dimension_micras_y.EnableWindow(FALSE);
                m_edit_cobertura.EnableWindow(FALSE);
                m_paso_x.EnableWindow(FALSE);
                m_paso_y.EnableWindow(FALSE);
                m_nombre.EnableWindow(FALSE);
                m_Directorio.EnableWindow(FALSE);
                m_radio_cobertura_espaciado.EnableWindow(FALSE);
                GetDlgItem(IDC_RADIO_COBERTURA_30)->EnableWindow(FALSE);
                GetDlgItem(IDC_RADIO_COBERTURA_50)->EnableWindow(FALSE);
                m_radio_cobertura.EnableWindow(FALSE);
                GetDlgItem(IDC_RADIO_COBERTURA_100)->EnableWindow(FALSE);
                GetDlgItem(IDC_RADIO_COBERTURA_EDIT)->EnableWindow(FALSE);

                //Ocultamos puntos intermedios
                for (int i= 0;i<NUM_PUNTOS_PLANO;i++)
                {
                    m_x[i].ShowWindow(SW_HIDE);
                    m_y[i].ShowWindow(SW_HIDE);
                    m_z[i].ShowWindow(SW_HIDE);
                    m_cuadro[i].ShowWindow(SW_HIDE);
                    m_static[i].ShowWindow(SW_HIDE);
                }

                //Mostramos progreso
                m_progreso.ShowWindow(SW_SHOW);
                m_progreso_barra.ShowWindow(SW_SHOW);
                m_progreso_static.ShowWindow(SW_SHOW);
                m_progreso_actual.ShowWindow(SW_SHOW);
                m_progreso_total.ShowWindow(SW_SHOW);
                m_progreso_static_campos.ShowWindow(SW_SHOW);
                m_boton_mosaico.ShowWindow(SW_SHOW);
            }
        }

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnBarrer") );
    }
}

// Si falla la preparacion de la correccion (por ejemplo porque no hay patrones), hay que evitar que se pueda barrer y capturar
void CFormTomaAuto::HayCorreccion(bool bHayCorreccion)
{
    m_pPadre->HayCorreccion(bHayCorreccion);
}

// LLamado por el thread de Barrido al finalizar
void CFormTomaAuto::PermitirNuevoBarrido(bool bCancelado) 
{
    if (!bCancelado)
    {
        m_boton_mosaico.EnableWindow(TRUE);
    }
    //Preparamos dialogo inicial
    m_cancelar.EnableWindow(FALSE);
    m_cancelar.ShowWindow(SW_HIDE);
    m_intermedios.ShowWindow(SW_SHOW);
    m_boton_enfocado.ShowWindow(FALSE);
    m_boton_esquina.ShowWindow(SW_SHOW);
    m_boton_esquina2.ShowWindow(SW_SHOW);
    InicializaProgresion(-1);
    m_boton_autoenfocar.EnableWindow(TRUE);

    //Habilitamos la configuracion del barrido (tamaño, escala, cobertura,...)
    m_escala.EnableWindow(TRUE);
    m_dimension_x.EnableWindow(TRUE);
    m_dimension_y.EnableWindow(TRUE);
    m_dimension_micras_x.EnableWindow(TRUE);
    m_dimension_micras_y.EnableWindow(TRUE);
    m_edit_cobertura.EnableWindow(TRUE);
    m_paso_x.EnableWindow(TRUE);
    m_paso_y.EnableWindow(TRUE);
    m_nombre.EnableWindow(TRUE);
    m_Directorio.EnableWindow(TRUE);
    m_radio_cobertura.EnableWindow(TRUE);
    m_radio_cobertura_espaciado.EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_30)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_50)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_100)->EnableWindow(TRUE);
    GetDlgItem(IDC_RADIO_COBERTURA_EDIT)->EnableWindow(TRUE);

    m_static_campos_x.Empty();
    m_static_campos_y.Empty();
    m_static_dimension_x.Empty();
    m_static_dimension_y.Empty();

    BorrarIntermedios();

    m_boton_esquina.EnableWindow(TRUE);
    m_boton_esquina2.EnableWindow(TRUE);
    m_bEsquina1Marcada = false;
    m_bEsquina2Marcada = false;

    //Lanzamos el thread de Actualizacion de Posicion
    m_pPadre->LanzarThreadPosicion();
}

// Se cancela el barrido
void CFormTomaAuto::OnCancelar() 
{
    //Preparamos dialogo inicial (tambien se hará al Cancelar->PermitirNuevoBarrido)
    m_cancelar.EnableWindow(FALSE);
    m_cancelar.ShowWindow(SW_HIDE);
    m_intermedios.ShowWindow(SW_SHOW);
    m_boton_enfocado.ShowWindow(FALSE);
    m_boton_esquina.EnableWindow(FALSE); //hasta que no termine el thread no permitimos el nuevo barrido
    m_boton_esquina2.EnableWindow(FALSE);//hasta que no termine el thread no permitimos el nuevo barrido
    m_boton_esquina.ShowWindow(SW_SHOW);
    m_boton_esquina2.ShowWindow(SW_SHOW);

    BorrarIntermedios();

	barrido.Cancelar();
}

// Genera el mosaico a partir de las imagenes guardadas en disco
// Estructura para pasar al thread de barrido
typedef struct PARAM_THREAD_PROGRESO
{
    CFormTomaAuto*   pDialogo;
} PARAM_THREAD_PROGRESO;
void CFormTomaAuto::OnButtonMosaico() 
{
    m_boton_mosaico.EnableWindow(FALSE);

    PARAM_THREAD_PROGRESO* paramThread = new PARAM_THREAD_PROGRESO; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pDialogo = this;

    AfxBeginThread(GenerarMosaicoThread, (void *)paramThread);

}
// THREAD
UINT CFormTomaAuto::GenerarMosaicoThread( LPVOID Param )
{
    PARAM_THREAD_PROGRESO* paramThread = (PARAM_THREAD_PROGRESO*)Param;

    paramThread->pDialogo->m_progreso_mosaico.ShowWindow(SW_SHOW);
    paramThread->pDialogo->m_progreso_mosaico.RedrawWindow(); //Necesario para que aparezca el texto modificado en el dialogo de progreso
    paramThread->pDialogo->barrido.GenerarMosaicos(paramThread->pDialogo);
    paramThread->pDialogo->m_progreso_mosaico.ShowWindow(SW_HIDE);

    delete paramThread;

    return 0;
}

// Se llama desde barrido::GenerarMosaicos para incrementar la indicacion de progreso de generacio ndel mosaico
void CFormTomaAuto::IncrementaProgresionMosaico() 
{
    m_progreso_mosaico.IncrementarProgreso();
}







/////////////////////////////////////////////////////////////////////////////////////
//
//  ZOOM - ESCALA
//
/////////////////////////////////////////////////////////////////////////////////////

//AUXILIAR
//Calcula los factores de zoom entre el tamaño del display y la imagen para poder
//mostrarla posteriormente correctamente redimensionada en el display
void CFormTomaAuto::CalculaZoom(long anchoDisplay,long altoDisplay)
{
    double factorZoomX = (double)anchoDisplay / theApp.m_ParamIni.Cam.anchoImagen; //theApp.m_configuracion.m_nDimension_barrido_x;
    double factorZoomY = (double)altoDisplay / theApp.m_ParamIni.Cam.altoImagen; //theApp.m_configuracion.m_nDimension_barrido_y;

    theApp.m_configuracion.m_ratio_inicial  = __min(factorZoomX,factorZoomY);
    // SI VERSION MIL ES 8.0, factorZoom debe dejarse en coma flotante
    // SI VERSION MIL ES MENOR, factorZoom debe ser [-16 a -1] o [1 a 16]
    if (M_MIL_CURRENT_VERSION < 8.0)
    {
        if ( theApp.m_configuracion.m_ratio_inicial > 16)
            theApp.m_configuracion.m_ratio_inicial = 16;
        if ( theApp.m_configuracion.m_ratio_inicial > 1)
            theApp.m_configuracion.m_ratio_inicial = floor(theApp.m_configuracion.m_ratio_inicial);
        else if (theApp.m_configuracion.m_ratio_inicial < ((double)1/16))
            theApp.m_configuracion.m_ratio_inicial = -16;
        else
            theApp.m_configuracion.m_ratio_inicial = -ceil(1 / theApp.m_configuracion.m_ratio_inicial);
    }
    else
    {
        if (theApp.m_configuracion.m_ratio_inicial>1)
            theApp.m_configuracion.m_ratio_inicial = 1; // No aumentamos el tamaño de la imagen
        else if (theApp.m_configuracion.m_ratio_inicial < 0.5)
            theApp.m_configuracion.m_ratio_inicial = 0.5; // Tamaño minimo del display, la mitad de la imagen
    }
}

// Actualiza y aplica al display el factor de zoom segun el tamaño del display 
// pero con limite el tamaño de la imagen
// Devuelve el zoom o ratio aplicado
double CFormTomaAuto::ActualizaZoom()
{
    // Ancho y alto maximo posible
    RECT Rect;
    m_imagen.GetClientRect(&Rect);
    long anchoDisplay = abs(Rect.right);
    long altoDisplay = abs(Rect.bottom);

    // calcula el zoom optimo (maximo) dentro de lo posible
    // el resultado esta en m_factorZoom
    CalculaZoom(anchoDisplay,altoDisplay);

    //Ajustar la ventana de display al zoom calculado
    long alto,ancho;
    if (theApp.m_configuracion.m_ratio_inicial < 0)
    {
        ancho = (long)floor(theApp.m_ParamIni.Cam.anchoImagen /*theApp.m_configuracion.m_nDimension_barrido_x*/ * -1.0/theApp.m_configuracion.m_ratio_inicial + 0.5);
        alto = (long)floor(theApp.m_ParamIni.Cam.altoImagen/*theApp.m_configuracion.m_nDimension_barrido_y*/ * -1.0/theApp.m_configuracion.m_ratio_inicial + 0.5);
    }
    else 
    {
        ancho = (long)floor(theApp.m_ParamIni.Cam.anchoImagen/*theApp.m_configuracion.m_nDimension_barrido_x*/ * theApp.m_configuracion.m_ratio_inicial + 0.5);
        alto = (long)floor(theApp.m_ParamIni.Cam.altoImagen/*theApp.m_configuracion.m_nDimension_barrido_y*/ * theApp.m_configuracion.m_ratio_inicial + 0.5);
    }

    m_imagen.SetWindowPos(NULL,0,0,ancho,alto,SWP_NOMOVE | SWP_NOZORDER);

    MdispZoom(M_display_normal, theApp.m_configuracion.m_ratio_inicial, theApp.m_configuracion.m_ratio_inicial);

    return theApp.m_configuracion.m_ratio_inicial;
}

void CFormTomaAuto::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
    UPDATE_EASYSIZE;
    if (::IsWindow(m_imagen.m_hWnd))
    {
        ActualizaZoom();
        CString csRatio;
        csRatio.Format("%.2f",theApp.m_configuracion.m_ratio_inicial);
        m_ratio.SetWindowText(csRatio);
    }
    Invalidate(TRUE);	
}

// se actualiza el ratio de la imagen despues de editarlo
void CFormTomaAuto::OnKillfocusEscalaVentana() 
{
    // Leemos el ratio que se quiere aplicar
    CString csRatio;
    m_ratio.GetWindowText(csRatio);
    double ratio = atof((LPCSTR)csRatio);

    if (ratio >= 0.5 && ratio <= 1)
    {
        CString csRatioActual;
        csRatioActual.Format("%.2f",theApp.m_configuracion.m_ratio_inicial);

        // No actuar si se ha modificado el ratio al cambiar el tamaño de la ventana
        // Es necesario comparar en CString para que los redondeos sean equiparables
        if (m_anchoResto != -1 && m_altoResto!= -1 && csRatioActual!=csRatio) 
        {
            // Ancho y alto correcto del display 
            long anchoDisplay = (long)floor(theApp.m_ParamIni.Cam.anchoImagen * ratio  + 0.5);
            long altoDisplay = (long)floor(theApp.m_ParamIni.Cam.altoImagen * ratio + 0.5);
    //        long anchoDisplay = (long)floor(theApp.m_configuracion.m_nDimension_barrido_x * ratio  + 0.5);
    //        long altoDisplay = (long)floor(theApp.m_configuracion.m_nDimension_barrido_y * ratio + 0.5);
            this->SetWindowPos(NULL,0,0,anchoDisplay+m_anchoResto,altoDisplay+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);
            if (m_pPadre)
                m_pPadre->SetSize(anchoDisplay+m_anchoResto+m_anchoTitulo,altoDisplay+m_altoResto+m_altoTitulo);

            // Se redimensionara el display en funcion del display (limitandolo al tamaño de la imagen 
            // y forzando a que sea cuadrado)
            ActualizaZoom();
        }
    }
}

int CFormTomaAuto::GetAnchoResto()
{
    return m_anchoResto + m_anchoTitulo;
}

int CFormTomaAuto::GetAltoResto()
{
    return m_altoResto + m_altoTitulo;
}

HWND CFormTomaAuto::GetControlImagen()
{
    return (HWND) m_imagen;
}



// Controles para redimensionar los elementos del dialogo
BEGIN_EASYSIZE_MAP(CFormTomaAuto)
//    EASYSIZE(IDC_IMAGE_GROUP           ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
    EASYSIZE(IDC_IMAGE                        ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
// IDC_AREA_DERECHA

    EASYSIZE( IDC_FILTRO_ACTUAL               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FILTRO_ACTUAL        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_LONGITUD_ONDA        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_NM                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_EDIT_COBERTURA              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_EDIT_DIMENSION_X            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_DIMENSION_X          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_DIMENSION_Y          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CAMPOS_X             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CAMPOS_Y             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESQUINA                     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_INTERMEDIOS                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_INICIO_X             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FIN_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_INICIO_Y             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FIN_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA1_X           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA1_Y           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA2_X           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA2_Y           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESQUINA2                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_RADIO_COBERTURA             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_RADIO_ESPACIADO             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_RADIO_COBERTURA_100         ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_RADIO_COBERTURA_50          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_RADIO_COBERTURA_30          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_PASO_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_PROGRESO                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_RADIO_COBERTURA_EDIT        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SC_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SC_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SD_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SD_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CI_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CI_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_C_X                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_C_Y                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CD_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CD_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_II_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_II_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_IC_X                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_IC_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_AUTOENFOCAR                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ENFOCADO                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_BARRER                      ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_INICIO_Z             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SC_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SD_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CI_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_C_Z                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CD_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_II_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_IC_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FIN_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA1_Z           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA2_Z           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CANCELAR                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_GOTO_XY                     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_BUTTON_MOSAICO              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_DIMENSION_MICRAS_X     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_ESCALA                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_DIMENSION_MAX_X             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_DIMENSION_Y            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_DIMENSION_MAX_Y             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_DIMENSION_MICRAS_Y     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_PASO_Y                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_SI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_SC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_SD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_CI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_C                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_CD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_II                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_IC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_ID                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_AUTOENFOQUE                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_PROGRESO_ACTUAL             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_PROGRESO_TOTAL              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESCALA_VENTANA              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POSICION             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
//    EASYSIZE( IDC_STATIC_TOMA                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_DIMENSIONES          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSX                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSY                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSZ                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_BARRIDO              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_MICRAS1              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_MICRAS2              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_MAX                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_x1                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_TAMPIXELS            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_TAMMICRAS            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_ESCALA               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_XB                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_YB                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_ZB                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_PI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_SC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_SD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_CI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_C                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_CD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_II                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_IC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_PF                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_DIMTOT               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CAMPOSTOT            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_X2                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_X3                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_x4                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_x5                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_x6                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_PROGRESO             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CAMPOS               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_BARRA                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESCALA_VENTANA       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
END_EASYSIZE_MAP

