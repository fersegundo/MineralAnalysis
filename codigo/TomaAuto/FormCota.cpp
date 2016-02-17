// FormPatron.cpp : implementation file
//

#include "stdafx.h"
#include "tomaauto.h"
#include "FormCota.h"
#include "defines.h"
#include "TomaAutoDlg.h"
#include "..\librerias\ControlDisp\control_camara.h"
#include "..\librerias\ControlMicroscopio\control_microscopio.h" //para mspWhereP en el thread de Posicion
#include "..\librerias\LibBarrido\autoenfoque.h" //autoenfoque

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CTomaAutoApp theApp;
extern HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)

/////////////////////////////////////////////////////////////////////////////
// CFormPatron dialog


CFormCota::CFormCota(CWnd* pParent /*=NULL*/)
	: CFormComun(CFormCota::IDD, pParent)
{
    m_bCambiadoManualmente  = false;
    m_pPadre                = NULL;

    //{{AFX_DATA_INIT(CFormCota)
	//}}AFX_DATA_INIT
}

//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
LRESULT CFormCota::UpdateDisplay(WPARAM wParam, LPARAM lParam)
{
    UpdateData((BOOL)wParam);
    return 0;
}


void CFormCota::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormCota)
	DDX_Control(pDX, IDC_COTA, m_cota);
	DDX_Control(pDX, IDC_ESCALA_VENTANA, m_ratio);
	DDX_Control(pDX, IDC_STATIC_POS_X, m_pos_x);
	DDX_Control(pDX, IDC_STATIC_POS_Z, m_pos_z);
	DDX_Control(pDX, IDC_STATIC_POS_Y, m_pos_y);
	DDX_Control(pDX, IDC_IMAGE, m_imagen);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_NM, m_nanometros);
	DDX_Control(pDX, IDC_STATIC_LONGITUD_ONDA, m_longitud_onda);
	DDX_Control(pDX, IDC_FILTRO_ACTUAL, m_filtro_actual);
}


BEGIN_MESSAGE_MAP(CFormCota, CDialog)
	//{{AFX_MSG_MAP(CFormCota)
    ON_MESSAGE(WM_UPDATEFIELDS, UpdateDisplay) //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_ESCALA_VENTANA, OnKillfocusEscalaVentana)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_GETMINMAXINFO()
	ON_BN_CLICKED(IDC_COTA, OnCota)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormCota message handlers

BOOL CFormCota::OnInitDialog() 
{
	CFormComun::OnInitDialog();
	
    UpdateData(FALSE);

    // Inicializacion variables

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

void CFormCota::OnDestroy() 
{
	CDialog::OnDestroy();
	
    fin_toma();
}

//Acciones necesarias al mostrar la ventana
void CFormCota::OnShowWindow(BOOL bShow, UINT nStatus) 
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
        }
        else
        {
            MdispDeselect(M_display_normal,M_imagen1);
        }
    }
}

// Ncesario para que [ENTER] y [ESC] no cierren el dialogo
BOOL CFormCota::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}


// Mostrar explicitamente la posicion dada en el dialogo
void CFormCota::MostrarPosicion(double x, double y, double z)
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
void CFormCota::MostrarPosicionZ(double z)
{
    CString csValue;
    csValue.Format("%.2f",z);
    m_pos_z.SetWindowText((LPCTSTR)csValue);
}

// Mostrar explicitamente el filtro dado en el dialogo
void CFormCota::MostrarFiltro(int nFiltro)
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


/////////////////////////////////////////////////////////////////////////////////////
//
//  ZOOM - ESCALA
//
/////////////////////////////////////////////////////////////////////////////////////

//AUXILIAR
//Calcula los factores de zoom entre el tamaño del display y la imagen para poder
//mostrarla posteriormente correctamente redimensionada en el display
void CFormCota::CalculaZoom(long anchoDisplay,long altoDisplay)
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
double CFormCota::ActualizaZoom()
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

void CFormCota::OnSize(UINT nType, int cx, int cy) 
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
void CFormCota::OnKillfocusEscalaVentana() 
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

int CFormCota::GetAnchoResto()
{
    return m_anchoResto + m_anchoTitulo;
}

int CFormCota::GetAltoResto()
{
    return m_altoResto + m_altoTitulo;
}

HWND CFormCota::GetControlImagen()
{
    return (HWND) m_imagen;
}


// Controles para redimensionar los elementos del dialogo
BEGIN_EASYSIZE_MAP(CFormCota)
    EASYSIZE(IDC_IMAGE                        ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)

    EASYSIZE( IDC_STATIC_ESCALA_VENTANA       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESCALA_VENTANA              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_FILTRO_ACTUAL               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FILTRO_ACTUAL        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_LONGITUD_ONDA        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_NM                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_INSTRUCCIONES        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_INSTRUCCIONES2       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_STATIC_POSICION             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POS_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POSX                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSY                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSZ                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_COTA                        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

END_EASYSIZE_MAP

// Estructura para pasar al thread de barrido
typedef struct PARAM_THREAD
{
    CFormCota*       pThis;
} PARAM_THREAD;

void CFormCota::OnCota() 
{
    //Finaliza el thread de posicion 
    m_pPadre->FinalizarThreadPosicion();

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        PARAM_THREAD* paramThread = new PARAM_THREAD; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
        paramThread->pThis = this;

        AfxBeginThread(AjusteAutomaticoZThread, (void *)paramThread);

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnBarrer") );
    }
}

//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
// THREAD 
//////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////
UINT CFormCota::AjusteAutomaticoZThread( LPVOID Param )
{
	double contraste;
    double z, z_predefinido; //esta es la z de referencia
    unsigned __int8 nFiltro;
    const int MAX_BASE_EXP_AUTOENFOQUE = 200;
    double dAumentos = theApp.m_ParamIni.objetivos[mspGetObjetivo()-1].dAumento;

    PARAM_THREAD* paramThread = (PARAM_THREAD*)Param;

    z = mspWhere(Z_);
	if (autoenfoque(z,z,&z_predefinido,&contraste, dAumentos, true, (HWND)paramThread->pThis->m_imagen) == -1)
        return -1;
	mspGo(Z_,z_predefinido); 
    theApp.Rueda.zObservacion = z_predefinido;
    z = z_predefinido;
    theApp.m_ParamIni.Rueda.diferencia_z[0] = 0;
	for (int i = 1; i<theApp.m_ParamIni.nBandas;i++) 
	{
        nFiltro = theApp.m_ParamIni.filtro[i];
		if (theApp.Rueda.ChangeFilter(nFiltro) == false)
        {
            ASSERT(FALSE);
			return -1;
        }
        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo
        toma2_unica(M_imagen1); //nos aseguramos que el cambio de exposicion se haya hecho efectivo

        if (theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro] != 0) // Si reflectancia es 0, no se debe ajustar la z para este filtro
        {
            if (theApp.m_ParamIni.Rueda.base_exp[nFiltro]>MAX_BASE_EXP_AUTOENFOQUE) //tiempo de exposicion demasiado grande para autoenfocar en movimiento
            {
		        if (autoenfoque(z,z,&z,&contraste, dAumentos, false, (HWND)paramThread->pThis->m_imagen) == -1)
                    return -1;
            }
            else
            {
		        if (autoenfoque(z,z,&z,&contraste, dAumentos, true, (HWND)paramThread->pThis->m_imagen) == -1)
                    return -1;
            }
            theApp.m_ParamIni.Rueda.diferencia_z[nFiltro] = z - z_predefinido;
		    mspGo(Z_,z); //si ha enfocado bien, el proximo filtro estará mejor enfocado
            paramThread->pThis->MostrarPosicionZ(z); //solo se actualiza z
        }
	}

    // Dejar en filto predefinido
	theApp.Rueda.ChangeFilter(theApp.m_ParamIni.Rueda.posFiltro);

  	toma2_continua(M_imagen1);	// Continuo tomando imágenes continuas.

    paramThread->pThis->FinCota();

    delete paramThread;

    return 0;
}

void CFormCota::FinCota()
{
    //Lanzamos el thread de Actualizacion de Posicion
    m_pPadre->LanzarThreadPosicion();

	if (crea_paramRevolver(theApp.m_ParamIni.raiz_patrones, &theApp.m_ParamIni) )  
		AfxMessageBox("Error al intentar guardar los parametros modificados", MB_ICONERROR);
}

