// FormPatron.cpp : implementation file
//

#include "stdafx.h"
#include "tomaauto.h"
#include "FormPatron.h"
#include "defines.h"
#include "TomaAutoDlg.h"
#include "Progreso.h"
#include "..\librerias\ControlDisp\control_camara.h"
#include "..\librerias\ControlMicroscopio\control_microscopio.h" //para mspWhereP en el thread de Posicion
#include "..\librerias\LibBarrido\calibracion.h" //para get_objetivo_corregido y ESCALA_TEORICA

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CTomaAutoApp theApp;
extern HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)

/////////////////////////////////////////////////////////////////////////////
// CFormPatron dialog


CFormPatron::CFormPatron(CWnd* pParent /*=NULL*/)
	: CFormComun(CFormPatron::IDD, pParent)
{
    m_bCambiadoManualmente  = false;
//    m_enumEsquinaActiva     = ESQUINA_1;
    m_bEsquina1Marcada      = false;
    m_bEsquina2Marcada      = false;
    m_pPadre                = NULL;
    m_bTipoPatronClaro      = true;
    m_bExposicionAjustada   = false;

    //{{AFX_DATA_INIT(CFormPatron)
	m_csProgreso_actual = _T("");
	m_csProgreso_total = _T("");
	//}}AFX_DATA_INIT
}

//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
LRESULT CFormPatron::UpdateDisplay(WPARAM wParam, LPARAM lParam)
{
    UpdateData((BOOL)wParam);
    return 0;
}


void CFormPatron::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormPatron)
	DDX_Control(pDX, IDC_PROGRESO_ACTUAL, m_progreso_actual);
	DDX_Control(pDX, IDC_EXPOSICION, m_exposicion);
	DDX_Control(pDX, IDC_STATIC_CAMPOS, m_progreso_static_campos);
	DDX_Control(pDX, IDC_PROGRESO_TOTAL, m_progreso_total);
	DDX_Control(pDX, IDC_STATIC_PROGRESO, m_progreso_static);
	DDX_Control(pDX, IDC_STATIC_BARRA, m_progreso_barra);
	DDX_Control(pDX, IDC_ESCALA_VENTANA, m_ratio);
	DDX_Control(pDX, IDC_PROGRESO, m_progreso);
	DDX_Control(pDX, IDC_STATIC_POS_X, m_pos_x);
	DDX_Control(pDX, IDC_STATIC_POS_Z, m_pos_z);
	DDX_Control(pDX, IDC_STATIC_POS_Y, m_pos_y);
	DDX_Control(pDX, IDC_STATIC_ESQUINA2_Z, m_esquina2_z);
	DDX_Control(pDX, IDC_STATIC_ESQUINA1_Z, m_esquina1_z);
	DDX_Control(pDX, IDC_INTERMEDIOS, m_intermedios);
	DDX_Control(pDX, IDC_BARRER, m_barrer);
	DDX_Control(pDX, IDC_CANCELAR, m_cancelar);
	DDX_Control(pDX, IDC_ENFOCADO, m_boton_enfocado);
	DDX_Control(pDX, IDC_ESQUINA2, m_boton_esquina2);
	DDX_Control(pDX, IDC_ESQUINA, m_boton_esquina);
	DDX_Control(pDX, IDC_STATIC_ESQUINA2_Y, m_esquina2_y);
	DDX_Control(pDX, IDC_STATIC_ESQUINA2_X, m_esquina2_x);
	DDX_Control(pDX, IDC_STATIC_ESQUINA1_Y, m_esquina1_y);
	DDX_Control(pDX, IDC_STATIC_ESQUINA1_X, m_esquina1_x);
	DDX_Control(pDX, IDC_IMAGE, m_imagen);
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


BEGIN_MESSAGE_MAP(CFormPatron, CDialog)
	//{{AFX_MSG_MAP(CFormPatron)
	ON_BN_CLICKED(IDC_ESQUINA, OnEsquina)
	ON_BN_CLICKED(IDC_ESQUINA2, OnEsquina2)
	ON_BN_CLICKED(IDC_INTERMEDIOS, OnIntermedios)
	ON_BN_CLICKED(IDC_ENFOCADO, OnEnfocado)
	ON_BN_CLICKED(IDC_BARRER, OnBarrer)
	ON_BN_CLICKED(IDC_CANCELAR, OnCancelar)
    ON_MESSAGE(WM_UPDATEFIELDS, UpdateDisplay) //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_ESCALA_VENTANA, OnKillfocusEscalaVentana)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_EXPOSICION, OnExposicion)
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormPatron message handlers

BOOL CFormPatron::OnInitDialog() 
{
	CFormComun::OnInitDialog();
	
    UpdateData(FALSE);

    // Inicializacion variables

    // Creamos la pantalla de progreso
    m_pProgreso = new CProgreso;

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

void CFormPatron::OnDestroy() 
{
	CDialog::OnDestroy();
	
    m_pProgreso->Fin(); //libera memoria asociada con el display de progreso
    delete m_pProgreso;

    fin_toma();
}

//Acciones necesarias al mostrar la ventana
void CFormPatron::OnShowWindow(BOOL bShow, UINT nStatus) 
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

            //Actualizamos el filtro actual
            MostrarFiltro(theApp.Rueda.GetFilter());

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
BOOL CFormPatron::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

//Establece el tipo de patron a tomar. true si patron claro, false si patron oscuro
void CFormPatron::SetTipoPatron(bool bTipoPatronClaro)
{
    m_bTipoPatronClaro = bTipoPatronClaro;

    if (bTipoPatronClaro)
        m_exposicion.EnableWindow(TRUE);
    else
        m_exposicion.EnableWindow(FALSE);
}

//Establece el tipo de patron a tomar. true si patron claro, false si patron oscuro
bool CFormPatron::GetTipoPatron()
{
    return m_bTipoPatronClaro;
}

// Mostrar explicitamente la posicion dada en el dialogo
void CFormPatron::MostrarPosicion(double x, double y, double z)
{
    CString csValue;

    csValue.Format("%.2f",x);
    m_pos_x.SetWindowText((LPCTSTR)csValue);

    csValue.Format("%.2f",y);
    m_pos_y.SetWindowText((LPCTSTR)csValue);

    csValue.Format("%.2f",z);
    m_pos_z.SetWindowText((LPCTSTR)csValue);
}

// Mostrar explicitamente la posicion SOLO DE Z dada en el dialogo
void CFormPatron::MostrarPosicionZ(double z)
{
    CString csValue;
    csValue.Format("%.2f",z);
    m_pos_z.SetWindowText((LPCTSTR)csValue);
}

// Mostrar explicitamente el filtro dado en el dialogo
void CFormPatron::MostrarFiltro(int nFiltro)
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
void CFormPatron::InicializaProgresion(int nCamposTotales)
{
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0); //Update variables from display, in case something has changed
    if (nCamposTotales < 1)
    {
        m_csProgreso_actual.Empty();
        m_csProgreso_total.Empty();
    }
    else
    {
        m_csProgreso_actual.Format("%d",0);
        m_csProgreso_total.Format("%d",nCamposTotales);
    }
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display
}

// Incrementa en 1 y muestra el campo que se esta barriendo
void CFormPatron::IncrementaProgresion()
{
    int nCampoActual = atoi(m_csProgreso_actual);

    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0); //Update variables from display, in case something has changed
    m_csProgreso_actual.Format("%d",nCampoActual+1);
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display
}



////////////////////////////////////////////////////////////////////////////////////
//
//      BARRIDO
//
////////////////////////////////////////////////////////////////////////////////////

// AUXILIAR
// Se vacian los valores de las coordenadas de los puntos intermedios
void CFormPatron::BorrarIntermedios() 
{
    for (int i= 0;i<NUM_PUNTOS_PLANO;i++)
    {
        m_x[i].SetWindowText("");
        m_y[i].SetWindowText("");
        m_z[i].SetWindowText("");
    }
}



// se establece una esquina 1 del barrido
void CFormPatron::OnEsquina() 
{
    point pActual;

    // Establecemos el punto actual como inicio temporalmente (cuando esten las dos esquinas
    // seleccionadas se recacularan inicio y fin)
    // Request ownership of mutex.
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        bool bSinIntermedios = m_barridoPatron.SetInicio(pActual); //obtiene pActual

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
            if (bSinIntermedios)
            {
                m_barrer.ShowWindow(SW_SHOW);
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
void CFormPatron::OnEsquina2() 
{
    point pActual;

    // Establecemos el punto actual como fin temporalmente (cuando esten las dos esquinas
    // seleccionadas se recacularan inicio y fin)
    // Request ownership of mutex.
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        bool bSinIntermedios = m_barridoPatron.SetFin(pActual);
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
            if (bSinIntermedios)
            {
                m_barrer.ShowWindow(SW_SHOW);
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
void CFormPatron::OnIntermedios() 
{
    int i;
    CString csValue = "";

    //  recalcula las posiciones del inicio y fin (en m_arrPuntosControl)
    m_barridoPatron.Intermedios();
    
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
        csValue.Format("%.2f",m_barridoPatron.m_arrPuntosControl[i].coord[X_]);
	    m_x[i].SetWindowText((LPCTSTR)csValue);
        csValue.Format("%.2f",m_barridoPatron.m_arrPuntosControl[i].coord[Y_]);
	    m_y[i].SetWindowText((LPCTSTR)csValue);	
        if (m_barridoPatron.m_arrPuntosControl[i].coord[Z_] != -1)
        {
            csValue.Format("%.3f",m_barridoPatron.m_arrPuntosControl[i].coord[Z_]);
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

    // Iniciamos enfoque de puntos intermedios
    // Request ownership of mutex.
    int nIndicePuntoSiguiente = -1;
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        int nDummy = -1;
        nIndicePuntoSiguiente = m_barridoPatron.EnfoquePuntosIntermedios(nDummy);
        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnIntermedios") );
    }

    // Indicacion visual de punto a enfocar
    m_cuadro[nIndicePuntoSiguiente].ShowWindow(SW_SHOW);
}

// El usuario ha enfocado un punto intermedio 
void CFormPatron::OnEnfocado() 
{
    int nIndicePuntoEnfocado = -1;

    // Request ownership of mutex.
    int nIndicePuntoSiguiente = -1;
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        nIndicePuntoSiguiente = m_barridoPatron.EnfoquePuntosIntermedios(nIndicePuntoEnfocado);
        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnEnfocado") );
    }

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
        m_barrer.EnableWindow(TRUE);
    }
    else
    {
        // Indicacion visual de punto a enfocar
        m_cuadro[nIndicePuntoSiguiente].ShowWindow(SW_SHOW);    
    }

    //Actualizar x,y, z en el dialogo
    CString csValueX,csValueY,csValueZ = "";
    csValueX.Format("%.2f",m_barridoPatron.m_arrPuntosControl[nIndicePuntoEnfocado].coord[X_]);
    csValueY.Format("%.2f",m_barridoPatron.m_arrPuntosControl[nIndicePuntoEnfocado].coord[Y_]);
    csValueZ.Format("%.3f",m_barridoPatron.m_arrPuntosControl[nIndicePuntoEnfocado].coord[Z_]);

    m_x[nIndicePuntoEnfocado].SetWindowText((LPCTSTR)csValueX);	
    m_y[nIndicePuntoEnfocado].SetWindowText((LPCTSTR)csValueY);	
    m_z[nIndicePuntoEnfocado].SetWindowText((LPCTSTR)csValueZ);	
}

//Se han definido los puntos intermedios y se procede a barrer
void CFormPatron::OnBarrer() 
{
    if (m_bTipoPatronClaro && !m_bExposicionAjustada)
    {
        if (AfxMessageBox("Atención! Es necesario ajustar la exposición de la cámara para cada filtro antes de tomar el patrón de alta reflectancia. ¿Esta seguro de querer tomar el patrón sin ajustar las exposiciones?", MB_ICONQUESTION | MB_YESNO | MB_DEFBUTTON2) == IDNO)
            return;
    }

    //Finaliza el thread de posicion 
    m_pPadre->FinalizarThreadPosicion();

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        if (m_barridoPatron.m_arrPuntosControl[0].coord[Z_] == m_barridoPatron.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_])
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
            m_barridoPatron.Intermedios();

            CString csValue = "";
            csValue.Format("%.2f",m_barridoPatron.m_arrPuntosControl[0].coord[X_]);
	        m_x[0].SetWindowText((LPCTSTR)csValue);
            csValue.Format("%.2f",m_barridoPatron.m_arrPuntosControl[0].coord[Y_]);
	        m_y[0].SetWindowText((LPCTSTR)csValue);	
            if (m_barridoPatron.m_arrPuntosControl[0].coord[Z_] != -1) //no deberia ocurrir nunca lo contrario
            {
                csValue.Format("%.3f",m_barridoPatron.m_arrPuntosControl[0].coord[Z_]);
	            m_z[0].SetWindowText((LPCTSTR)csValue);	
            }
            csValue.Format("%.2f",m_barridoPatron.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[X_]);
	        m_x[NUM_PUNTOS_PLANO-1].SetWindowText((LPCTSTR)csValue);
            csValue.Format("%.2f",m_barridoPatron.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Y_]);
	        m_y[NUM_PUNTOS_PLANO-1].SetWindowText((LPCTSTR)csValue);	
            if (m_barridoPatron.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_] != -1)
            {
                csValue.Format("%.3f",m_barridoPatron.m_arrPuntosControl[NUM_PUNTOS_PLANO-1].coord[Z_]);
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

        m_pProgreso->Inicializar(theApp.m_ParamIni.BarridoPatrones.camposEjeX ,theApp.m_ParamIni.BarridoPatrones.camposEjeY, m_progreso);
        if (m_barridoPatron.Barrer(this))    //THREAD. <---------
        {
            // Preparamos el dialogo para el barrido
            m_barrer.ShowWindow(SW_HIDE);
            m_barrer.EnableWindow(FALSE);
            m_cancelar.EnableWindow(TRUE);
            m_cancelar.ShowWindow(SW_SHOW);

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
        }

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnBarrer") );
    }
}

// Si falla la preparacion de la correccion (por ejemplo porque no hay patrones), hay que evitar que se pueda barrer y capturar
void CFormPatron::HayCorreccion(bool bHayCorreccion)
{
    m_pPadre->HayCorreccion(bHayCorreccion);
}

// LLamado por el thread de Barrido al finalizar
void CFormPatron::PermitirNuevoBarrido(bool bCancelado) 
{
    //Preparamos dialogo inicial
    m_cancelar.EnableWindow(FALSE);
    m_cancelar.ShowWindow(SW_HIDE);
    m_intermedios.ShowWindow(SW_SHOW);
    m_boton_enfocado.ShowWindow(FALSE);
    m_boton_esquina.ShowWindow(SW_SHOW);
    m_boton_esquina2.ShowWindow(SW_SHOW);
    InicializaProgresion(-1);

    BorrarIntermedios();

    m_boton_esquina.EnableWindow(TRUE);
    m_boton_esquina2.EnableWindow(TRUE);
    m_bEsquina1Marcada = false;
    m_bEsquina2Marcada = false;

    //Lanzamos el thread de Actualizacion de Posicion
    m_pPadre->LanzarThreadPosicion();
}

// Se cancela el barrido
void CFormPatron::OnCancelar() 
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

	m_barridoPatron.Cancelar();
}





/////////////////////////////////////////////////////////////////////////////////////
//
//  ZOOM - ESCALA
//
/////////////////////////////////////////////////////////////////////////////////////

//AUXILIAR
//Calcula los factores de zoom entre el tamaño del display y la imagen para poder
//mostrarla posteriormente correctamente redimensionada en el display
void CFormPatron::CalculaZoom(long anchoDisplay,long altoDisplay)
{
    double factorZoomX = (double)anchoDisplay / theApp.m_ParamIni.Cam.anchoImagen; //theApp.m_configuracion.m_nDimension_x;
    double factorZoomY = (double)altoDisplay / theApp.m_ParamIni.Cam.altoImagen; //theApp.m_configuracion.m_nDimension_y;

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
double CFormPatron::ActualizaZoom()
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
        ancho = (long)floor(theApp.m_ParamIni.Cam.anchoImagen /*theApp.m_configuracion.m_nDimension_x*/ * -1.0/theApp.m_configuracion.m_ratio_inicial + 0.5);
        alto = (long)floor(theApp.m_ParamIni.Cam.altoImagen/*theApp.m_configuracion.m_nDimension_y*/ * -1.0/theApp.m_configuracion.m_ratio_inicial + 0.5);
    }
    else 
    {
        ancho = (long)floor(theApp.m_ParamIni.Cam.anchoImagen/*theApp.m_configuracion.m_nDimension_x*/ * theApp.m_configuracion.m_ratio_inicial + 0.5);
        alto = (long)floor(theApp.m_ParamIni.Cam.altoImagen/*theApp.m_configuracion.m_nDimension_y*/ * theApp.m_configuracion.m_ratio_inicial + 0.5);
    }

    m_imagen.SetWindowPos(NULL,0,0,ancho,alto,SWP_NOMOVE | SWP_NOZORDER);

    MdispZoom(M_display_normal, theApp.m_configuracion.m_ratio_inicial, theApp.m_configuracion.m_ratio_inicial);

    return theApp.m_configuracion.m_ratio_inicial;
}

void CFormPatron::OnSize(UINT nType, int cx, int cy) 
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
void CFormPatron::OnKillfocusEscalaVentana() 
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
    //        long anchoDisplay = (long)floor(theApp.m_configuracion.m_nDimension_x * ratio  + 0.5);
    //        long altoDisplay = (long)floor(theApp.m_configuracion.m_nDimension_y * ratio + 0.5);
            this->SetWindowPos(NULL,0,0,anchoDisplay+m_anchoResto,altoDisplay+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);
            if (m_pPadre)
                m_pPadre->SetSize(anchoDisplay+m_anchoResto+m_anchoTitulo,altoDisplay+m_altoResto+m_altoTitulo);

            // Se redimensionara el display en funcion del display (limitandolo al tamaño de la imagen 
            // y forzando a que sea cuadrado)
            ActualizaZoom();
        }
    }
}

int CFormPatron::GetAnchoResto()
{
    return m_anchoResto + m_anchoTitulo;
}

int CFormPatron::GetAltoResto()
{
    return m_altoResto + m_altoTitulo;
}

HWND CFormPatron::GetControlImagen()
{
    return (HWND) m_imagen;
}


// Controles para redimensionar los elementos del dialogo
BEGIN_EASYSIZE_MAP(CFormPatron)
    EASYSIZE(IDC_IMAGE                        ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)

    EASYSIZE( IDC_STATIC_ESCALA_VENTANA       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESCALA_VENTANA              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_POSICION             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POS_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POSX                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSY                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSZ                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_FILTRO_ACTUAL               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FILTRO_ACTUAL        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_LONGITUD_ONDA        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_NM                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_INSTRUCCIONES        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_INSTRUCCIONES2       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_INSTRUCCIONES3       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_INSTRUCCIONES4       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_EXPOSICION                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_STATIC_BARRIDO              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_ENFOCADO                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_BARRER                      ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_INTERMEDIOS                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CANCELAR                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_XB                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_YB                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_ZB                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_ESQUINA                     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESQUINA2                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA1_X           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA1_Y           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA1_Z           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA2_X           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA2_Y           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_ESQUINA2_Z           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_INICIO_X             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_INICIO_Y             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_INICIO_Z             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_PI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_SC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_SD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_CI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_C                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_CD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_II                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_IC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_PF                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
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
    EASYSIZE( IDC_STATIC_SC_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_SD_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CI_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_C_Z                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CD_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_II_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_IC_Z                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FIN_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FIN_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FIN_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_SI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_SC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_SD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_CI                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_C                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_CD                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_II                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_IC                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_CUADRO_ID                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)  
    
    EASYSIZE( IDC_PROGRESO                    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_PROGRESO_ACTUAL             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_PROGRESO_TOTAL              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_PROGRESO             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_CAMPOS               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_BARRA                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
END_EASYSIZE_MAP


void CFormPatron::OnExposicion() 
{
    //Finaliza el thread de posicion 
    m_pPadre->FinalizarThreadPosicion();

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        m_barridoPatron.AjusteAutomaticoExposicion(this);

        m_bExposicionAjustada = true;

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnBarrer") );
    }
}

void CFormPatron::FinExposicion()
{
    //Lanzamos el thread de Actualizacion de Posicion
    m_pPadre->LanzarThreadPosicion();

	if (crea_paramRevolver(theApp.m_ParamIni.raiz_patrones, &theApp.m_ParamIni) )  
		AfxMessageBox("Error al intentar guardar los parametros modificados", MB_ICONERROR);
}

