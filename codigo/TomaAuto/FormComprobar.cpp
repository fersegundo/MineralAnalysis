// CFormComprobar.cpp : implementation file
//

#include "stdafx.h"
#include "tomaauto.h"
#include "FormComprobar.h"
#include "defines.h"
#include "TomaAutoDlg.h"
//#include "folder_dialog.h"
#include "..\librerias\ControlDisp\control_camara.h"
#include "..\librerias\LibBarrido\control_barrido.h"
#include "..\librerias\ControlMicroscopio\control_microscopio.h" //para mspWhereP en el thread de Posicion

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define UMBRAL_ERROR_MAXIMO 1.0 //diferencia del porcentaje de reflentacia teorica del patron y el porcentaje de reflectancia encontrado

extern CTomaAutoApp theApp;
extern HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)

/////////////////////////////////////////////////////////////////////////////
// CFormComprobar dialog


CFormComprobar::CFormComprobar(CWnd* pParent /*=NULL*/)
	: CFormComun(CFormComprobar::IDD, pParent)
{
    m_pPadre                = NULL;
	//{{AFX_DATA_INIT(CFormComprobar)
	//}}AFX_DATA_INIT

}


void CFormComprobar::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormComprobar)
	DDX_Control(pDX, IDC_STATIC_RECALIBRAR, m_staticRecalibrar);
	DDX_Control(pDX, IDC_STATIC_OK, m_staticOk);
	DDX_Control(pDX, IDC_ICON_EXCLAMATION, m_icon_exclamation);
	DDX_Control(pDX, IDC_ICON_CHECK, m_icon_check);
	DDX_Control(pDX, IDC_STATIC_ERROR, m_staticError);
	DDX_Control(pDX, IDC_STATIC_FILTRO, m_staticFiltro);
	DDX_Control(pDX, IDC_STATIC_REFL, m_staticRefl);
	DDX_Control(pDX, IDC_STATIC_REFL_TEORICA, m_staticReflTeorica);
	DDX_Control(pDX, IDC_STATIC_RESULTADOS, m_statucResultados);
	DDX_Control(pDX, IDC_STATIC_953, m_static953);
	DDX_Control(pDX, IDC_STATIC_952, m_static952);
	DDX_Control(pDX, IDC_STATIC_951, m_static951);
	DDX_Control(pDX, IDC_STATIC_950, m_static950);
	DDX_Control(pDX, IDC_STATIC_903, m_static903);
	DDX_Control(pDX, IDC_STATIC_902, m_static902);
	DDX_Control(pDX, IDC_STATIC_901, m_static901);
	DDX_Control(pDX, IDC_STATIC_900, m_static900);
	DDX_Control(pDX, IDC_STATIC_853, m_static853);
	DDX_Control(pDX, IDC_STATIC_852, m_static852);
	DDX_Control(pDX, IDC_STATIC_851, m_static851);
	DDX_Control(pDX, IDC_STATIC_850, m_static850);
	DDX_Control(pDX, IDC_STATIC_803, m_static803);
	DDX_Control(pDX, IDC_STATIC_802, m_static802);
	DDX_Control(pDX, IDC_STATIC_801, m_static801);
	DDX_Control(pDX, IDC_STATIC_800, m_static800);
	DDX_Control(pDX, IDC_STATIC_753, m_static753);
	DDX_Control(pDX, IDC_STATIC_752, m_static752);
	DDX_Control(pDX, IDC_STATIC_751, m_static751);
	DDX_Control(pDX, IDC_STATIC_750, m_static750);
	DDX_Control(pDX, IDC_STATIC_703, m_static703);
	DDX_Control(pDX, IDC_STATIC_702, m_static702);
	DDX_Control(pDX, IDC_STATIC_701, m_static701);
	DDX_Control(pDX, IDC_STATIC_700, m_static700);
	DDX_Control(pDX, IDC_STATIC_653, m_static653);
	DDX_Control(pDX, IDC_STATIC_652, m_static652);
	DDX_Control(pDX, IDC_STATIC_651, m_static651);
	DDX_Control(pDX, IDC_STATIC_650, m_static650);
	DDX_Control(pDX, IDC_STATIC_603, m_static603);
	DDX_Control(pDX, IDC_STATIC_602, m_static602);
	DDX_Control(pDX, IDC_STATIC_601, m_static601);
	DDX_Control(pDX, IDC_STATIC_600, m_static600);
	DDX_Control(pDX, IDC_STATIC_553, m_static553);
	DDX_Control(pDX, IDC_STATIC_552, m_static552);
	DDX_Control(pDX, IDC_STATIC_551, m_static551);
	DDX_Control(pDX, IDC_STATIC_550, m_static550);
	DDX_Control(pDX, IDC_STATIC_503, m_static503);
	DDX_Control(pDX, IDC_STATIC_502, m_static502);
	DDX_Control(pDX, IDC_STATIC_501, m_static501);
	DDX_Control(pDX, IDC_STATIC_500, m_static500);
	DDX_Control(pDX, IDC_STATIC_453, m_static453);
	DDX_Control(pDX, IDC_STATIC_452, m_static452);
	DDX_Control(pDX, IDC_STATIC_451, m_static451);
	DDX_Control(pDX, IDC_STATIC_450, m_static450);
	DDX_Control(pDX, IDC_STATIC_403, m_static403);
	DDX_Control(pDX, IDC_STATIC_402, m_static402);
	DDX_Control(pDX, IDC_STATIC_401, m_static401);
	DDX_Control(pDX, IDC_STATIC_400, m_static400);
	DDX_Control(pDX, IDC_STATIC_3003, m_static3003);
	DDX_Control(pDX, IDC_STATIC_3002, m_static3002);
	DDX_Control(pDX, IDC_STATIC_3001, m_static3001);
	DDX_Control(pDX, IDC_STATIC_3000, m_static3000);
	DDX_Control(pDX, IDC_STATIC_2003, m_static2003);
	DDX_Control(pDX, IDC_STATIC_2002, m_static2002);
	DDX_Control(pDX, IDC_STATIC_2001, m_static2001);
	DDX_Control(pDX, IDC_STATIC_2000, m_static2000);
	DDX_Control(pDX, IDC_STATIC_103, m_static103);
	DDX_Control(pDX, IDC_STATIC_102, m_static102);
	DDX_Control(pDX, IDC_STATIC_101, m_static101);
	DDX_Control(pDX, IDC_STATIC_1003, m_static1003);
	DDX_Control(pDX, IDC_STATIC_1002, m_static1002);
	DDX_Control(pDX, IDC_STATIC_1001, m_static1001);
	DDX_Control(pDX, IDC_STATIC_1000, m_static1000);
	DDX_Control(pDX, IDC_STATIC_100, m_static100);
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


BEGIN_MESSAGE_MAP(CFormComprobar, CDialog)
	//{{AFX_MSG_MAP(CFormComprobar)
	ON_BN_CLICKED(IDC_COMPROBAR, OnComprobar)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_ESCALA_VENTANA, OnKillfocusEscalaVentana)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
//  message handlers

BOOL CFormComprobar::OnInitDialog() 
{
	CFormComun::OnInitDialog();
	
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

    // Ocultar completamente los filtros que no usemos
    for (int i=0; i< theApp.m_ParamIni.nBandas; i++)
    {
        if (theApp.m_ParamIni.Rueda.espectro[i] != 0) //pancromatico no
            MostrarFiltroResultado(theApp.m_ParamIni.filtro[i]);
    }

    // Inicializamos los fonts
    LOGFONT lf;
    CFont * pFont = m_static100.GetFont(); //cualquier CStatic vale
    pFont->GetLogFont( &lf ); // esto lo hacemos para obtener temporalmente el estilo por defecto

    VERIFY(m_FontNormal.CreateFontIndirect(&lf));  // create the font

    // modificamos el estilo del font
    lf.lfWeight = FW_BOLD;
    VERIFY(m_FontNegrita.CreateFontIndirect(&lf));  // create the font

	return TRUE;  // return TRUE unless you set the focus to a control
	              // EXCEPTION: OCX Property Pages should return FALSE
}

void CFormComprobar::OnDestroy() 
{
	CDialog::OnDestroy();

    m_FontNormal.DeleteObject();
    m_FontNegrita.DeleteObject();
	
    fin_toma();
}

void CFormComprobar::OnShowWindow(BOOL bShow, UINT nStatus) 
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

            m_staticOk.ShowWindow(SW_HIDE);
            m_icon_check.ShowWindow(SW_HIDE);
            m_staticRecalibrar.ShowWindow(SW_HIDE);
            m_icon_exclamation.ShowWindow(SW_HIDE);

            for (int i=0; i< theApp.m_ParamIni.nBandas; i++)
                if (theApp.m_ParamIni.Rueda.espectro[i] != 0) //pancromatico no es necesario
                    ResultadoFiltro(i,-1, -1); //deshabilitamos, y borramos los datos de reflectancia
        }

    }
}

// Necesario para que [ENTER] y [ESC] no cierren el dialogo
BOOL CFormComprobar::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// Mostrar explicitamente la posicion dada en el dialogo
void CFormComprobar::MostrarPosicion(double x, double y, double z)
{
    CString csValue;
    csValue.Format("%.2f",x);
    m_pos_x.SetWindowText((LPCTSTR)csValue);
    csValue.Format("%.2f",y);
    m_pos_y.SetWindowText((LPCTSTR)csValue);
    csValue.Format("%.2f",z);
    m_pos_z.SetWindowText((LPCTSTR)csValue);
}

// Mostrar explicitamente el filtro dado en el dialogo
void CFormComprobar::MostrarFiltro(int nFiltro)
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
void CFormComprobar::CalculaZoom(long anchoDisplay,long altoDisplay)
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
double CFormComprobar::ActualizaZoom()
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

void CFormComprobar::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);

    UPDATE_EASYSIZE;
    if (::IsWindow(m_imagen.m_hWnd))
    {
        ActualizaZoom(); //Actualiza theApp.m_configuracion.m_ratio_inicial
        CString csRatio;
        csRatio.Format("%.2f",theApp.m_configuracion.m_ratio_inicial);
        m_ratio.SetWindowText(csRatio);
    }
    Invalidate(TRUE);	
}

// se actualiza el ratio de la imagen despues de editarlo
void CFormComprobar::OnKillfocusEscalaVentana() 
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

int CFormComprobar::GetAnchoResto()
{
    return m_anchoResto + m_anchoTitulo;
}

int CFormComprobar::GetAltoResto()
{
    return m_altoResto + m_altoTitulo;
}

HWND CFormComprobar::GetControlImagen()
{
    return (HWND) m_imagen;
}

// Controles para redimensionar los elementos del dialogo
BEGIN_EASYSIZE_MAP(CFormComprobar)
    EASYSIZE(IDC_IMAGE                        ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)

    EASYSIZE( IDC_STATIC_ESCALA_VENTANA       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESCALA_VENTANA              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_POSICION             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSX                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSY                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSZ                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POS_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_FILTRO_ACTUAL               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FILTRO_ACTUAL        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_LONGITUD_ONDA        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_NM                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_INSTRUCCIONES        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_COMPROBAR                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE( IDC_STATIC_RESULTADOS           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_FILTRO               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_REFL                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_REFL_TEORICA         ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_ERROR                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_100                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_101                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_102                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_103                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_400                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_401                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_402                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_403                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_450                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_451                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_452                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_453                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_500                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_501                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_502                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_503                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_550                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_551                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_552                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_553                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_600                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_601                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_602                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_603                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_650                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_651                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_652                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_653                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_700                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_701                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_702                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_703                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_750                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_751                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_752                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_753                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_800                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_801                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_802                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_803                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_850                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_851                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_852                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_853                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_900                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_901                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_902                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_903                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_950                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_951                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_952                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_953                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_1000                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_1001                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_1002                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_1003                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_2000                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_2001                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_2002                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_2003                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_3000                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_3001                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_3002                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_3003                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_ICON_CHECK                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_OK                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_ICON_EXCLAMATION            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_RECALIBRAR           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

END_EASYSIZE_MAP


/////////////////////////////////////////////////////////////////////////////
// CFormComprobar message handlers

// Estructura para pasar a los thread 
typedef struct PARAM_THREAD
{
    CFormComprobar*   pDialogo;
} PARAM_THREAD;


void CFormComprobar::OnComprobar() 
{
    // Ocultamos avisos por si ya se ejecutó
    m_staticOk.ShowWindow(SW_HIDE);
    m_icon_check.ShowWindow(SW_HIDE);
    m_staticRecalibrar.ShowWindow(SW_HIDE);
    m_icon_exclamation.ShowWindow(SW_HIDE);

    //Finaliza el thread de posicion 
    m_pPadre->FinalizarThreadPosicion();

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        theApp.Rueda.zObservacion = mspWhere(Z_); //puede que el usuario haya enfocado

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnBarrer") );
    }

    PARAM_THREAD* paramThread = new PARAM_THREAD; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pDialogo = this;

    AfxBeginThread(ComprobarThread, (void *)paramThread);
}

UINT CFormComprobar::ComprobarThread( LPVOID Param )
{
    PARAM_THREAD* paramThread = (PARAM_THREAD*)Param;

    //Tomar Imagenes (en M_banda[])
    captura_bandas( theApp.m_ParamIni, 1/*1 << (theApp.m_ParamIni.Cam.profundidad - BITS_CAMARA)*/,theApp.Rueda);
    //Lanzamos el thread de Actualizacion de Posicion
    paramThread->pDialogo->m_pPadre->LanzarThreadPosicion();

    // Obtener valores de reflectacia medios
	long moda;
    double reflectancia;
    double error;
    long histo_temp[65536];
    long histo_filtrado[256];
    MIL_ID M_histo16;
    memset(histo_temp,0,65536*sizeof(long));
	MimAllocResult(M_sistema, 65536, M_HIST_LIST, &M_histo16);

    paramThread->pDialogo->m_staticOk.ShowWindow(SW_SHOW);
    paramThread->pDialogo->m_icon_check.ShowWindow(SW_SHOW);
    for (int i = 0; i<theApp.m_ParamIni.nBandas;i++)
    {
        if (theApp.m_ParamIni.Rueda.espectro[i] != 0) //pancromatico no
        {
	        MimHistogram(M_banda[i], M_histo16);
	        MimGetResult(M_histo16, M_VALUE, histo_temp);
            CString csMensaje;

            // Filtramos el histograma para evitar el efecto "peine"
            memset(histo_filtrado, 0, 256 * sizeof(long));
            for (int j=0; j<65536; j++)
                histo_filtrado[j/256] += histo_temp[j];
            moda = moda_histograma(histo_filtrado, 0, 255);

		    reflectancia = moda * theApp.m_ParamIni.escala; //calculamos el nivel de reflectancia

            error = fabs(reflectancia - theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[i]);
            paramThread->pDialogo->ResultadoFiltro(i,reflectancia, error); //habilitamos, y rellenamos con el dato obtenido de reflectancia

            if (error > UMBRAL_ERROR_MAXIMO)
            {
                paramThread->pDialogo->m_staticOk.ShowWindow(SW_HIDE);
                paramThread->pDialogo->m_icon_check.ShowWindow(SW_HIDE);
                paramThread->pDialogo->m_staticRecalibrar.ShowWindow(SW_SHOW);
                paramThread->pDialogo->m_icon_exclamation.ShowWindow(SW_SHOW);
            }
        }
    }

    return 0;
}

void CFormComprobar::ResultadoFiltro(int nFiltro, double dReflectancia, double dError)
{
    CString csReflectancia;
    CString csError;
    BOOL bEnable;
    CFont* pFont; //puntero al font a aplicar

    pFont = &m_FontNormal;
    if (dReflectancia == -1) // clear
    {
        bEnable = FALSE;
        csReflectancia.Empty();
        csError.Empty();
    }
    else
    {
        bEnable = TRUE;
        csReflectancia.Format("%.2lf",dReflectancia);
        csError.Format("%.2lf",dError);
        if (dError > UMBRAL_ERROR_MAXIMO)
            pFont = &m_FontNegrita;
    }

    //pFont->CreateFontIndirect(&lf);

    if (nFiltro == 0)
    {
        m_static100.EnableWindow(bEnable);
        m_static101.EnableWindow(bEnable);
        m_static102.EnableWindow(bEnable);
        m_static103.EnableWindow(bEnable);

        m_static101.SetWindowText(csReflectancia);
        m_static103.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static103.SetWindowText(csError);
    }
    else if(nFiltro == 1)
    {
        m_static400.EnableWindow(bEnable);
        m_static401.EnableWindow(bEnable);
        m_static402.EnableWindow(bEnable);
        m_static403.EnableWindow(bEnable);

        m_static401.SetWindowText(csReflectancia);
        m_static403.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static403.SetWindowText(csError);
    }
    else if(nFiltro == 2)
    {
        m_static450.EnableWindow(bEnable);
        m_static451.EnableWindow(bEnable);
        m_static452.EnableWindow(bEnable);
        m_static453.EnableWindow(bEnable);

        m_static451.SetWindowText(csReflectancia);
        m_static453.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static453.SetWindowText(csError);
    }
    else if(nFiltro == 3)
    {
        m_static500.EnableWindow(bEnable);
        m_static501.EnableWindow(bEnable);
        m_static502.EnableWindow(bEnable);
        m_static503.EnableWindow(bEnable);

        m_static501.SetWindowText(csReflectancia);
        m_static503.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static503.SetWindowText(csError);
    }
    else if(nFiltro == 4)
    {
        m_static550.EnableWindow(bEnable);
        m_static551.EnableWindow(bEnable);
        m_static552.EnableWindow(bEnable);
        m_static553.EnableWindow(bEnable);

        m_static551.SetWindowText(csReflectancia);
        m_static553.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static553.SetWindowText(csError);
    }
    else if(nFiltro == 5)
    {
        m_static600.EnableWindow(bEnable);
        m_static601.EnableWindow(bEnable);
        m_static602.EnableWindow(bEnable);
        m_static603.EnableWindow(bEnable);

        m_static601.SetWindowText(csReflectancia);
        m_static603.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static603.SetWindowText(csError);
    }
    else if(nFiltro == 6)
    {
        m_static650.EnableWindow(bEnable);
        m_static651.EnableWindow(bEnable);
        m_static652.EnableWindow(bEnable);
        m_static653.EnableWindow(bEnable);

        m_static651.SetWindowText(csReflectancia);
        m_static653.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static653.SetWindowText(csError);
    }
    else if(nFiltro == 7)
    {
        m_static700.EnableWindow(bEnable);
        m_static701.EnableWindow(bEnable);
        m_static702.EnableWindow(bEnable);
        m_static703.EnableWindow(bEnable);

        m_static701.SetWindowText(csReflectancia);
        m_static703.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static703.SetWindowText(csError);
    }
    else if(nFiltro == 8)
    {
        m_static750.EnableWindow(bEnable);
        m_static751.EnableWindow(bEnable);
        m_static752.EnableWindow(bEnable);
        m_static753.EnableWindow(bEnable);

        m_static751.SetWindowText(csReflectancia);
        m_static753.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static753.SetWindowText(csError);
    }
    else if(nFiltro == 9)
    {
        m_static800.EnableWindow(bEnable);
        m_static801.EnableWindow(bEnable);
        m_static802.EnableWindow(bEnable);
        m_static803.EnableWindow(bEnable);

        m_static801.SetWindowText(csReflectancia);
        m_static803.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static803.SetWindowText(csError);
    }
    else if(nFiltro == 10 )
    {
        m_static850.EnableWindow(bEnable);
        m_static851.EnableWindow(bEnable);
        m_static852.EnableWindow(bEnable);
        m_static853.EnableWindow(bEnable);

        m_static851.SetWindowText(csReflectancia);
        m_static853.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static853.SetWindowText(csError);
    }
    else if(nFiltro == 11)
    {
        m_static900.EnableWindow(bEnable);
        m_static901.EnableWindow(bEnable);
        m_static902.EnableWindow(bEnable);
        m_static903.EnableWindow(bEnable);

        m_static901.SetWindowText(csReflectancia);
        m_static903.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static903.SetWindowText(csError);
    }
    else if(nFiltro == 12)
    {
        m_static950.EnableWindow(bEnable);
        m_static951.EnableWindow(bEnable);
        m_static952.EnableWindow(bEnable);
        m_static953.EnableWindow(bEnable);

        m_static951.SetWindowText(csReflectancia);
        m_static953.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static953.SetWindowText(csError);
    }
    else if(nFiltro == 13)
    {
        m_static1000.EnableWindow(bEnable);
        m_static1001.EnableWindow(bEnable);
        m_static1002.EnableWindow(bEnable);
        m_static1003.EnableWindow(bEnable);

        m_static1001.SetWindowText(csReflectancia);
        m_static1003.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static1003.SetWindowText(csError);
    }
    else if(nFiltro == 14)
    {
        m_static2000.EnableWindow(bEnable);
        m_static2001.EnableWindow(bEnable);
        m_static2002.EnableWindow(bEnable);
        m_static2003.EnableWindow(bEnable);

        m_static2001.SetWindowText(csReflectancia);
        m_static2003.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static2003.SetWindowText(csError);
    }
    else if(nFiltro == 15)
    {
        m_static3000.EnableWindow(bEnable);
        m_static3001.EnableWindow(bEnable);
        m_static3002.EnableWindow(bEnable);
        m_static3003.EnableWindow(bEnable);

        m_static3001.SetWindowText(csReflectancia);
        m_static3003.SetFont(pFont, FALSE); // TRUE to redraw window
        m_static3003.SetWindowText(csError);
    }

}

void CFormComprobar::MostrarFiltroResultado(int nFiltro) 
{
    CString csTexto;

    if (nFiltro == 0)
    {
        m_static100.ShowWindow(SW_SHOW);
        m_static101.ShowWindow(SW_SHOW);
        m_static102.ShowWindow(SW_SHOW);
        m_static103.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static100.SetWindowText(csTexto);
        m_static101.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static102.SetWindowText(csTexto);
        m_static103.SetWindowText("");
    }
    else if(nFiltro == 1)
    {
        m_static400.ShowWindow(SW_SHOW);
        m_static401.ShowWindow(SW_SHOW);
        m_static402.ShowWindow(SW_SHOW);
        m_static403.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static400.SetWindowText(csTexto);
        m_static401.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static402.SetWindowText(csTexto);
        m_static403.SetWindowText("");
    }
    else if(nFiltro == 2)
    {
        m_static450.ShowWindow(SW_SHOW);
        m_static451.ShowWindow(SW_SHOW);
        m_static452.ShowWindow(SW_SHOW);
        m_static453.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static450.SetWindowText(csTexto);
        m_static451.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static452.SetWindowText(csTexto);
        m_static453.SetWindowText("");
    }
    else if(nFiltro == 3)
    {
        m_static500.ShowWindow(SW_SHOW);
        m_static501.ShowWindow(SW_SHOW);
        m_static502.ShowWindow(SW_SHOW);
        m_static503.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static500.SetWindowText(csTexto);
        m_static501.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static502.SetWindowText(csTexto);
        m_static503.SetWindowText("");
    }
    else if(nFiltro == 4)
    {
        m_static550.ShowWindow(SW_SHOW);
        m_static551.ShowWindow(SW_SHOW);
        m_static552.ShowWindow(SW_SHOW);
        m_static553.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static550.SetWindowText(csTexto);
        m_static551.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static552.SetWindowText(csTexto);
        m_static553.SetWindowText("");
    }
    else if(nFiltro == 5)
    {
        m_static600.ShowWindow(SW_SHOW);
        m_static601.ShowWindow(SW_SHOW);
        m_static602.ShowWindow(SW_SHOW);
        m_static603.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static600.SetWindowText(csTexto);
        m_static601.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static602.SetWindowText(csTexto);
        m_static603.SetWindowText("");
    }
    else if(nFiltro == 6)
    {
        m_static650.ShowWindow(SW_SHOW);
        m_static651.ShowWindow(SW_SHOW);
        m_static652.ShowWindow(SW_SHOW);
        m_static653.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static650.SetWindowText(csTexto);
        m_static651.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static652.SetWindowText(csTexto);
        m_static653.SetWindowText("");
    }
    else if(nFiltro == 7)
    {
        m_static700.ShowWindow(SW_SHOW);
        m_static701.ShowWindow(SW_SHOW);
        m_static702.ShowWindow(SW_SHOW);
        m_static703.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static700.SetWindowText(csTexto);
        m_static701.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static702.SetWindowText(csTexto);
        m_static703.SetWindowText("");
    }
    else if(nFiltro == 8)
    {
        m_static750.ShowWindow(SW_SHOW);
        m_static751.ShowWindow(SW_SHOW);
        m_static752.ShowWindow(SW_SHOW);
        m_static753.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static750.SetWindowText(csTexto);
        m_static751.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static752.SetWindowText(csTexto);
        m_static753.SetWindowText("");
    }
    else if(nFiltro == 9)
    {
        m_static800.ShowWindow(SW_SHOW);
        m_static801.ShowWindow(SW_SHOW);
        m_static802.ShowWindow(SW_SHOW);
        m_static803.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static800.SetWindowText(csTexto);
        m_static801.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static802.SetWindowText(csTexto);
        m_static803.SetWindowText("");
    }
    else if(nFiltro == 10 )
    {
        m_static850.ShowWindow(SW_SHOW);
        m_static851.ShowWindow(SW_SHOW);
        m_static852.ShowWindow(SW_SHOW);
        m_static853.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static850.SetWindowText(csTexto);
        m_static851.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static852.SetWindowText(csTexto);
        m_static853.SetWindowText("");
    }
    else if(nFiltro == 11)
    {
        m_static900.ShowWindow(SW_SHOW);
        m_static901.ShowWindow(SW_SHOW);
        m_static902.ShowWindow(SW_SHOW);
        m_static903.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static900.SetWindowText(csTexto);
        m_static901.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static902.SetWindowText(csTexto);
        m_static903.SetWindowText("");
    }
    else if(nFiltro == 12)
    {
        m_static950.ShowWindow(SW_SHOW);
        m_static951.ShowWindow(SW_SHOW);
        m_static952.ShowWindow(SW_SHOW);
        m_static953.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static950.SetWindowText(csTexto);
        m_static951.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static952.SetWindowText(csTexto);
        m_static953.SetWindowText("");
    }
    else if(nFiltro == 13)
    {
        m_static1000.ShowWindow(SW_SHOW);
        m_static1001.ShowWindow(SW_SHOW);
        m_static1002.ShowWindow(SW_SHOW);
        m_static1003.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static1000.SetWindowText(csTexto);
        m_static1001.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static1002.SetWindowText(csTexto);
        m_static1003.SetWindowText("");
    }
    else if(nFiltro == 14)
    {
        m_static2000.ShowWindow(SW_SHOW);
        m_static2001.ShowWindow(SW_SHOW);
        m_static2002.ShowWindow(SW_SHOW);
        m_static2003.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static2000.SetWindowText(csTexto);
        m_static2001.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static2002.SetWindowText(csTexto);
        m_static2003.SetWindowText("");
    }
    else if(nFiltro == 15)
    {
        m_static3000.ShowWindow(SW_SHOW);
        m_static3001.ShowWindow(SW_SHOW);
        m_static3002.ShowWindow(SW_SHOW);
        m_static3003.ShowWindow(SW_SHOW);

        csTexto.Format("%d",theApp.m_ParamIni.Rueda.espectro[nFiltro]);
        m_static3000.SetWindowText(csTexto);
        m_static3001.SetWindowText("");
        csTexto.Format("%.2lf",theApp.m_ParamIni.Rueda.reflectancia_nominal_pat_cl[nFiltro]);
        m_static3002.SetWindowText(csTexto);
        m_static3003.SetWindowText("");
    }
}

