// AnalisisDlg.cpp : implementation file
//

#include "stdafx.h"
#include "Analisis.h"
#include "AnalisisDlg.h"
#include "Minerales.h"
#include <direct.h> //chdir pruebas - automatizacion
#include "..\librerias\LibTiff\InterfazLibTiff.h" // control imagenes tiff (escala reflectancia-gris)
#include "AsociacionesMinerales.h" // para Liberar las asociaciones
//#include "..\RecopilaDatosPixels\FileFinder.h"//Uso alternativo automatizado por linea de comando (shlwapi.lib, FileFinder.h y Path.h)
#include "FileFinder.h"//Uso alternativo automatizado por linea de comando (shlwapi.lib, FileFinder.h y Path.h)

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MAX_EDIT_BYTES 3 //numero de caracteres admisibles en cada campo de porcentajes RGB de cada filtro (max 100%)
static const LINEA_VERTICAL_R = 200; //color de la linea vertical que se presenta en el grafico de espectros
static const LINEA_VERTICAL_G = 200;
static const LINEA_VERTICAL_B = 256;

extern CAnalisisApp theApp;
extern bool ComparadorMinerales(CMineral* mineral1,CMineral* mineral2); //definido en Minerales.cpp, necesario para ordenar la lista de minerales

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CAnalisisDlg dialog

CAnalisisDlg::CAnalisisDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CAnalisisDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CAnalisisDlg)
	m_bTodosMinerales = FALSE;
	m_csUmbralConfiabilidad = _T("");
	m_csUmbralDistancia = _T("");
	m_dMinRef = _T("");
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    
    m_nNumTab = 0;
    //memset(filterOfTab,-1,MAX_NUM_IMAGENES*sizeof(int));

    m_nCampoActual = -1;
    m_nFilaActual = -1;
    m_nCampoAnterior = -1;
    m_nCampoSiguiente = -1;
    m_nCampoSuperior = -1;
    m_nCampoInferior = -1;

    m_anchoResto = -1;
    m_altoResto = -1;
    
    m_nUltimoFiltro = 2;

    m_bRGB_Modificado = true;

    m_nCountOscuros = 0;
}

void CAnalisisDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAnalisisDlg)
	DDX_Control(pDX, IDC_InfIzq, m_checkInfIzq);
	DDX_Control(pDX, IDC_InfDer, m_checkInfDer);
	DDX_Control(pDX, IDC_SupDer, m_checkSupDer);
	DDX_Control(pDX, IDC_SupIzq, m_checkSupIzq);
	DDX_Control(pDX, IDC_DISTANCIA, m_botonDistancia);
	DDX_Control(pDX, IDC_CONFIABILIDAD, m_botonConfiabilidad);
	DDX_Control(pDX, IDC_NO_CLASIFICADOS, m_no_clasificados);
	DDX_Control(pDX, IDC_RGB, m_ButtonRGB);
	DDX_Control(pDX, IDC_CLASIFICAR, m_ButtonClasificar);
	DDX_Control(pDX, IDC_GUARDAR_CLASIFICAR, m_ButtonGuardarClasificar);
	DDX_Control(pDX, IDC_CLASIFICADOS, m_clasificados);
	DDX_Control(pDX, IDC_SUPERIOR, m_BotonSuperior);
	DDX_Control(pDX, IDC_INFERIOR, m_BotonInferior);
	DDX_Control(pDX, IDC_RATIO, m_ratio);
	DDX_Control(pDX, IDC_GUARDAR_RGB, m_ButtonGuardarRGB);
	DDX_Control(pDX, IDC_SIGUIENTE, m_BotonSiguiente);
	DDX_Control(pDX, IDC_ANTERIOR, m_BotonAnterior);
	DDX_Control(pDX, IDC_GUARDAR, m_guardar);
	DDX_Control(pDX, IDC_MINERAL_STATIC, m_mineral_static);
	DDX_Control(pDX, IDC_COMENTARIO_STATIC, m_comentario_static);
	DDX_Control(pDX, IDC_COMENTARIO, m_comentario);
	DDX_Control(pDX, IDC_COMBO_MINERAL, m_combo_mineral);
	DDX_Control(pDX, IDC_COMBO_CALIDAD, m_combo_calidad);
	DDX_Control(pDX, IDC_CALIDAD_STATIC, m_calidad_static);
	DDX_Control(pDX, IDC_HISTOGRAMA, m_histograma_frame);
	DDX_Control(pDX, IDC_ESPECTROS, m_espectros_frame);
	DDX_Control(pDX, IDC_AREAS, m_tree_areas);
	DDX_Control(pDX, IDC_IMAGE_GROUP, m_control_group);
	DDX_Control(pDX, IDC_TOTAL_R, m_TotalR);
	DDX_Control(pDX, IDC_TOTAL_G, m_TotalG);
	DDX_Control(pDX, IDC_TOTAL_B, m_TotalB);
	DDX_Control(pDX, IDC_STATIC_400, m_static400);
	DDX_Control(pDX, IDC_STATIC_450, m_static450);
	DDX_Control(pDX, IDC_STATIC_500, m_static500);
	DDX_Control(pDX, IDC_STATIC_550, m_static550);
	DDX_Control(pDX, IDC_STATIC_600, m_static600);
	DDX_Control(pDX, IDC_STATIC_650, m_static650);
	DDX_Control(pDX, IDC_STATIC_700, m_static700);
	DDX_Control(pDX, IDC_STATIC_750, m_static750);
	DDX_Control(pDX, IDC_STATIC_800, m_static800);
	DDX_Control(pDX, IDC_STATIC_850, m_static850);
	DDX_Control(pDX, IDC_STATIC_900, m_static900);
	DDX_Control(pDX, IDC_STATIC_950, m_static950);
	DDX_Control(pDX, IDC_STATIC_1000, m_static1000);
	DDX_Control(pDX, IDC_EDIT_450_R, m_edit450r);
	DDX_Control(pDX, IDC_EDIT_450_G, m_edit450g);
	DDX_Control(pDX, IDC_EDIT_450_B, m_edit450b);
	DDX_Control(pDX, IDC_EDIT_400_R, m_edit400r);
	DDX_Control(pDX, IDC_EDIT_400_G, m_edit400g);
	DDX_Control(pDX, IDC_EDIT_400_B, m_edit400b);
	DDX_Control(pDX, IDC_EDIT_550_R, m_edit550r);
	DDX_Control(pDX, IDC_EDIT_550_G, m_edit550g);
	DDX_Control(pDX, IDC_EDIT_550_B, m_edit550b);
	DDX_Control(pDX, IDC_EDIT_500_R, m_edit500r);
	DDX_Control(pDX, IDC_EDIT_500_G, m_edit500g);
	DDX_Control(pDX, IDC_EDIT_500_B, m_edit500b);
	DDX_Control(pDX, IDC_EDIT_650_R, m_edit650r);
	DDX_Control(pDX, IDC_EDIT_650_G, m_edit650g);
	DDX_Control(pDX, IDC_EDIT_650_B, m_edit650b);
	DDX_Control(pDX, IDC_EDIT_600_R, m_edit600r);
	DDX_Control(pDX, IDC_EDIT_600_G, m_edit600g);
	DDX_Control(pDX, IDC_EDIT_600_B, m_edit600b);
	DDX_Control(pDX, IDC_EDIT_750_R, m_edit750r);
	DDX_Control(pDX, IDC_EDIT_750_G, m_edit750g);
	DDX_Control(pDX, IDC_EDIT_750_B, m_edit750b);
	DDX_Control(pDX, IDC_EDIT_700_R, m_edit700r);
	DDX_Control(pDX, IDC_EDIT_700_G, m_edit700g);
	DDX_Control(pDX, IDC_EDIT_700_B, m_edit700b);
	DDX_Control(pDX, IDC_EDIT_850_R, m_edit850r);
	DDX_Control(pDX, IDC_EDIT_850_G, m_edit850g);
	DDX_Control(pDX, IDC_EDIT_850_B, m_edit850b);
	DDX_Control(pDX, IDC_EDIT_800_R, m_edit800r);
	DDX_Control(pDX, IDC_EDIT_800_G, m_edit800g);
	DDX_Control(pDX, IDC_EDIT_800_B, m_edit800b);
	DDX_Control(pDX, IDC_EDIT_950_R, m_edit950r);
	DDX_Control(pDX, IDC_EDIT_950_G, m_edit950g);
	DDX_Control(pDX, IDC_EDIT_950_B, m_edit950b);
	DDX_Control(pDX, IDC_EDIT_900_R, m_edit900r);
	DDX_Control(pDX, IDC_EDIT_900_G, m_edit900g);
	DDX_Control(pDX, IDC_EDIT_900_B, m_edit900b);
	DDX_Control(pDX, IDC_EDIT_1000_R, m_edit1000r);
	DDX_Control(pDX, IDC_EDIT_1000_G, m_edit1000g);
	DDX_Control(pDX, IDC_EDIT_1000_B, m_edit1000b);
	DDX_Control(pDX, IDC_TAB1, m_tabControl);
	DDX_Control(pDX, IDC_IMAGE, m_control);
	DDX_Check(pDX, IDC_TODOS_MINERALES, m_bTodosMinerales);
	DDX_Text(pDX, IDC_UMBRAL_CONFIABILIDAD, m_csUmbralConfiabilidad);
	DDX_Text(pDX, IDC_UMBRAL_DISTANCIA, m_csUmbralDistancia);
	DDX_Text(pDX, IDC_MAX_REF, m_dMinRef);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAnalisisDlg, CDialog)
	//{{AFX_MSG_MAP(CAnalisisDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_NOTIFY(TCN_SELCHANGE, IDC_TAB1, OnSelchangeTab1)
	ON_EN_CHANGE(IDC_EDIT_400_B, OnChangeEdit400B)
	ON_EN_CHANGE(IDC_EDIT_450_B, OnChangeEdit450B)
	ON_EN_CHANGE(IDC_EDIT_500_B, OnChangeEdit500B)
	ON_EN_CHANGE(IDC_EDIT_550_B, OnChangeEdit550B)
	ON_EN_CHANGE(IDC_EDIT_600_B, OnChangeEdit600B)
	ON_EN_CHANGE(IDC_EDIT_650_B, OnChangeEdit650B)
	ON_EN_CHANGE(IDC_EDIT_700_B, OnChangeEdit700B)
	ON_EN_CHANGE(IDC_EDIT_750_B, OnChangeEdit750B)
	ON_EN_CHANGE(IDC_EDIT_800_B, OnChangeEdit800B)
	ON_EN_CHANGE(IDC_EDIT_850_B, OnChangeEdit850B)
	ON_EN_CHANGE(IDC_EDIT_900_B, OnChangeEdit900B)
	ON_EN_CHANGE(IDC_EDIT_950_B, OnChangeEdit950B)
	ON_EN_CHANGE(IDC_EDIT_1000_B, OnChangeEdit1000B)
	ON_EN_CHANGE(IDC_EDIT_400_G, OnChangeEdit400G)
	ON_EN_CHANGE(IDC_EDIT_450_G, OnChangeEdit450G)
	ON_EN_CHANGE(IDC_EDIT_500_G, OnChangeEdit500G)
	ON_EN_CHANGE(IDC_EDIT_550_G, OnChangeEdit550G)
	ON_EN_CHANGE(IDC_EDIT_600_G, OnChangeEdit600G)
	ON_EN_CHANGE(IDC_EDIT_650_G, OnChangeEdit650G)
	ON_EN_CHANGE(IDC_EDIT_700_G, OnChangeEdit700G)
	ON_EN_CHANGE(IDC_EDIT_750_G, OnChangeEdit750G)
	ON_EN_CHANGE(IDC_EDIT_800_G, OnChangeEdit800G)
	ON_EN_CHANGE(IDC_EDIT_850_G, OnChangeEdit850G)
	ON_EN_CHANGE(IDC_EDIT_900_G, OnChangeEdit900G)
	ON_EN_CHANGE(IDC_EDIT_950_G, OnChangeEdit950G)
	ON_EN_CHANGE(IDC_EDIT_1000_G, OnChangeEdit1000G)
	ON_EN_CHANGE(IDC_EDIT_400_R, OnChangeEdit400R)
	ON_EN_CHANGE(IDC_EDIT_450_R, OnChangeEdit450R)
	ON_EN_CHANGE(IDC_EDIT_500_R, OnChangeEdit500R)
	ON_EN_CHANGE(IDC_EDIT_550_R, OnChangeEdit550R)
	ON_EN_CHANGE(IDC_EDIT_600_R, OnChangeEdit600R)
	ON_EN_CHANGE(IDC_EDIT_650_R, OnChangeEdit650R)
	ON_EN_CHANGE(IDC_EDIT_700_R, OnChangeEdit700R)
	ON_EN_CHANGE(IDC_EDIT_750_R, OnChangeEdit750R)
	ON_EN_CHANGE(IDC_EDIT_800_R, OnChangeEdit800R)
	ON_EN_CHANGE(IDC_EDIT_850_R, OnChangeEdit850R)
	ON_EN_CHANGE(IDC_EDIT_900_R, OnChangeEdit900R)
	ON_EN_CHANGE(IDC_EDIT_950_R, OnChangeEdit950R)
	ON_EN_CHANGE(IDC_EDIT_1000_R, OnChangeEdit1000R)
	ON_BN_CLICKED(IDC_ABRIR_TODAS, OnAbrirTodas)
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_SIZE()
	ON_WM_DESTROY()
	ON_WM_MOUSEMOVE()
	ON_NOTIFY(TVN_SELCHANGED, IDC_AREAS, OnSelchangedAreas)
	ON_NOTIFY(TVN_KEYDOWN, IDC_AREAS, OnKeydownAreas)
	ON_NOTIFY(TVN_ENDLABELEDIT, IDC_AREAS, OnEndlabeleditAreas)
    ON_NOTIFY ( NM_CUSTOMDRAW, IDC_AREAS, OnCustomdrawAreas )
    ON_NOTIFY ( NM_CUSTOMDRAW, IDC_CLASIFICADOS, OnCustomdrawClasificados )
	ON_BN_CLICKED(IDC_DUMMY, OnDummy)
	ON_NOTIFY(NM_CLICK, IDC_AREAS, OnClickAreas)
	ON_WM_TIMER()
	ON_EN_CHANGE(IDC_COMENTARIO, OnChangeComentario)
	ON_CBN_SELCHANGE(IDC_COMBO_MINERAL, OnSelchangeComboMineral)
	ON_CBN_SELCHANGE(IDC_COMBO_CALIDAD, OnSelchangeComboCalidad)
	ON_BN_CLICKED(IDC_GUARDAR, OnGuardar)
	ON_BN_CLICKED(IDC_ANTERIOR, OnAnterior)
	ON_BN_CLICKED(IDC_SIGUIENTE, OnSiguiente)
	ON_BN_CLICKED(IDC_GUARDAR_RGB, OnGuardarRgb)
	ON_WM_GETMINMAXINFO()
	ON_EN_CHANGE(IDC_RATIO, OnChangeRatio)
	ON_BN_CLICKED(IDC_SUPERIOR, OnSuperior)
	ON_BN_CLICKED(IDC_INFERIOR, OnInferior)
	ON_EN_KILLFOCUS(IDC_RATIO, OnKillfocusRatio)
	ON_BN_CLICKED(IDC_GUARDAR_CLASIFICAR, OnGuardarClasificar)
	ON_BN_CLICKED(IDC_CLASIFICAR, OnClasificar)
	ON_BN_CLICKED(IDC_RGB, OnRgb)
	ON_BN_CLICKED(IDC_CONFIABILIDAD, OnConfiabilidad)
	ON_CBN_DROPDOWN(IDC_COMBO_MINERAL, OnDropdownComboMineral)
	ON_BN_CLICKED(IDC_SupIzq, OnSupIzq)
	ON_BN_CLICKED(IDC_SupDer, OnSupDer)
	ON_BN_CLICKED(IDC_InfIzq, OnInfIzq)
	ON_BN_CLICKED(IDC_InfDer, OnInfDer)
	ON_EN_CHANGE(IDC_UMBRAL_CONFIABILIDAD, OnChangeUmbralConfiabilidad)
	ON_EN_CHANGE(IDC_UMBRAL_DISTANCIA, OnChangeUmbralDistancia)
	ON_BN_CLICKED(IDC_TODOS_MINERALES, OnTodosMinerales)
	ON_COMMAND(ID_CONFIGURACIN_ASOCIACIONESMINERALES, OnConfiguracinAsociacionesminerales)
	ON_BN_CLICKED(IDC_DISTANCIA, OnDistancia)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

// Controles para redimensionar los elementos del dialogo
BEGIN_EASYSIZE_MAP(CAnalisisDlg)      //  left        top      right     bottom
    EASYSIZE(IDC_IMAGE_GROUP           ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
    EASYSIZE(IDC_AREAS                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE(IDC_AREA_STATIC           ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_TODOS_MINERALES ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_COMENTARIO_STATIC ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_MINERAL_STATIC    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_CALIDAD_STATIC    ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_COMENTARIO        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_COMBO_MINERAL     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
        EASYSIZE(IDC_COMBO_CALIDAD     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    //IDC_DUMMY_POS y IDC_DUMMY_POS_IN se usan unicamente para que IDC_HISTO y IDC_HISTO2 se estiren adecuadamente
	EASYSIZE(IDC_DUMMY_POS             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_BORDER,0)
	EASYSIZE(IDC_DUMMY_POS_IN          ,IDC_DUMMY_POS,IDC_DUMMY_POS,IDC_DUMMY_POS,IDC_DUMMY_POS,ES_VCENTER)
    EASYSIZE(IDC_ESPECTROS             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,IDC_DUMMY_POS_IN,0)
    EASYSIZE(IDC_HISTOGRAMA            ,ES_KEEPSIZE,IDC_DUMMY_POS_IN,ES_BORDER,ES_BORDER,0)
    EASYSIZE(IDC_TAB1                  ,ES_BORDER,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)

    EASYSIZE(IDC_CLASIFICADOS          ,ES_BORDER,ES_BORDER,ES_KEEPSIZE,ES_BORDER,0)
    EASYSIZE(IDC_STATIC_RATIO          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE(IDC_RATIO                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
END_EASYSIZE_MAP



/////////////////////////////////////////////////////////////////////////////
// CAnalisisDlg message handlers


BOOL CAnalisisDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

    // Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

    m_asociacionesDlg.Init(&theApp.m_asociaciones, &theApp.m_minerales);
    // Para cada mineral, comprobamos que sea compatible con las asociaciones, si no, lo desactivamos
    theApp.m_asociaciones.ActualizarMineralesActivosPorAsociaciones();	

    //Inicializar el menú
    m_Menu.LoadMenu(IDR_MENU);
    SetMenu(&m_Menu);

    // Deshabilitar menu asociaciones minerales
    m_Menu.EnableMenuItem(0,MF_BYPOSITION | MF_GRAYED); 
    DrawMenuBar(); 

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    // Scroll minerales clasificados demasiado oscuros
    //m_no_clasificados.ShowScrollBar(SB_VERT);

    // Icono de guardar
    //m_guardar.SetBitmap( (HBITMAP) LoadImage(0, "save.jpg",IMAGE_BITMAP,0,0,LR_LOADFROMFILE) );
    m_guardar_bitmap.LoadBitmap(IDB_GUARDAR);
    HBITMAP hBitmap= (HBITMAP) m_guardar_bitmap.GetSafeHandle();
    m_guardar.SetBitmap(hBitmap);
    /*
    // resize buton for bitmap
    BITMAP bm;
    m_guardar_bitmap.GetBitmap(&bm);
    int width = bm.bmWidth;
    int height = bm.bmHeight;
    CRect r;
    m_guardar.GetWindowRect(&r); 
    ScreenToClient(&r); // MoveWindow needs coordinates in parent window 
    r.right = r.left + width;
    r.bottom = r.top + height;
    m_guardar.MoveWindow(&r);
*/
    // Bitmap de boton "campo siguiente"
    m_siguiente_bitmap.LoadBitmap(IDB_SIGUIENTE);
    hBitmap= (HBITMAP) m_siguiente_bitmap.GetSafeHandle();
    m_BotonSiguiente.SetBitmap(hBitmap);
    // Bitmap de boton "campo anterior"
    m_anterior_bitmap.LoadBitmap(IDB_ANTERIOR);
    hBitmap= (HBITMAP) m_anterior_bitmap.GetSafeHandle();
    m_BotonAnterior.SetBitmap(hBitmap);

    // Bitmap de boton "campo inferior"
    m_inferior_bitmap.LoadBitmap(IDB_INFERIOR);
    hBitmap= (HBITMAP) m_inferior_bitmap.GetSafeHandle();
    m_BotonInferior.SetBitmap(hBitmap);

    // Bitmap de boton "campo superior"
    m_superior_bitmap.LoadBitmap(IDB_SUPERIOR);
    hBitmap= (HBITMAP) m_superior_bitmap.GetSafeHandle();
    m_BotonSuperior.SetBitmap(hBitmap);

    // botones inicialmente deshabilitados
//    m_BotonCerrar.EnableWindow(FALSE);
    m_BotonAnterior.EnableWindow(FALSE);
    m_BotonSiguiente.EnableWindow(FALSE);
    m_BotonSuperior.EnableWindow(FALSE);
    m_BotonInferior.EnableWindow(FALSE);
    

    //todos los controles de RGB deben estar deshabilidatos
    DeshabilitarRGB();

    DeshabilitarPropiedadesArea();

    // Las propiedades de area deben estar deshabilitadas
    
	m_mineral_static.EnableWindow(FALSE);
	m_comentario_static.EnableWindow(FALSE);
	m_calidad_static.EnableWindow(FALSE);
	m_combo_mineral.EnableWindow(FALSE);
	m_combo_calidad.EnableWindow(FALSE);
	m_comentario.EnableWindow(FALSE);

    m_combo_mineral.AddString(SELECCIONE_MINERAL);

    // Inicializamos editables a partir de lo configurado
    m_csUmbralConfiabilidad.Format("%.2lf", theApp.m_configuracion.m_dConfiabilidad);
    m_csUmbralDistancia.Format("%.2lf", theApp.m_configuracion.m_dDistancia);
    m_dMinRef.Format("%.2lf", theApp.m_configuracion.m_dMinRef);
    UpdateData(FALSE);

    // cargar nombres y abreviaturas de minerales
    for (int i = 0; i<theApp.m_minerales.GetCount();i++)
        m_combo_mineral.AddString(theApp.m_minerales.GetNombre(i));
    
    m_combo_calidad.AddString(CALIDAD_SIN_VALORAR);
    m_combo_calidad.AddString(CALIDAD_A);
    m_combo_calidad.AddString(CALIDAD_B);
    m_combo_calidad.AddString(CALIDAD_C);
    m_combo_calidad.AddString(CALIDAD_D);
    m_combo_calidad.AddString(CALIDAD_E);

    //establecemos un maximo numero de caractes para los campos de porcentajes (max 100%)
    m_edit400b.SetLimitText(MAX_EDIT_BYTES);
    m_edit400g.SetLimitText(MAX_EDIT_BYTES);
    m_edit400r.SetLimitText(MAX_EDIT_BYTES);
    m_edit450b.SetLimitText(MAX_EDIT_BYTES);
    m_edit450g.SetLimitText(MAX_EDIT_BYTES);
    m_edit450r.SetLimitText(MAX_EDIT_BYTES);
    m_edit500b.SetLimitText(MAX_EDIT_BYTES);
    m_edit500g.SetLimitText(MAX_EDIT_BYTES);
    m_edit500r.SetLimitText(MAX_EDIT_BYTES);
    m_edit550b.SetLimitText(MAX_EDIT_BYTES);
    m_edit550g.SetLimitText(MAX_EDIT_BYTES);
    m_edit550r.SetLimitText(MAX_EDIT_BYTES);
    m_edit600b.SetLimitText(MAX_EDIT_BYTES);
    m_edit600g.SetLimitText(MAX_EDIT_BYTES);
    m_edit600r.SetLimitText(MAX_EDIT_BYTES);
    m_edit650b.SetLimitText(MAX_EDIT_BYTES);
    m_edit650g.SetLimitText(MAX_EDIT_BYTES);
    m_edit650r.SetLimitText(MAX_EDIT_BYTES);
    m_edit700b.SetLimitText(MAX_EDIT_BYTES);
    m_edit700g.SetLimitText(MAX_EDIT_BYTES);
    m_edit700r.SetLimitText(MAX_EDIT_BYTES);
    m_edit750b.SetLimitText(MAX_EDIT_BYTES);
    m_edit750g.SetLimitText(MAX_EDIT_BYTES);
    m_edit750r.SetLimitText(MAX_EDIT_BYTES);
    m_edit800b.SetLimitText(MAX_EDIT_BYTES);
    m_edit800g.SetLimitText(MAX_EDIT_BYTES);
    m_edit800r.SetLimitText(MAX_EDIT_BYTES);
    m_edit850b.SetLimitText(MAX_EDIT_BYTES);
    m_edit850g.SetLimitText(MAX_EDIT_BYTES);
    m_edit850r.SetLimitText(MAX_EDIT_BYTES);
    m_edit900b.SetLimitText(MAX_EDIT_BYTES);
    m_edit900g.SetLimitText(MAX_EDIT_BYTES);
    m_edit900r.SetLimitText(MAX_EDIT_BYTES);
    m_edit950b.SetLimitText(MAX_EDIT_BYTES);
    m_edit950g.SetLimitText(MAX_EDIT_BYTES);
    m_edit950r.SetLimitText(MAX_EDIT_BYTES);
    m_edit1000b.SetLimitText(MAX_EDIT_BYTES);
    m_edit1000g.SetLimitText(MAX_EDIT_BYTES);
    m_edit1000r.SetLimitText(MAX_EDIT_BYTES);

    // Cargamos los porcentajes
    if (theApp.m_configuracion.m_bCargado)
    {
        // Actualizamos los valores en el dialogo
        char dummy[4];
        m_edit400r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(400)], dummy, 10));
        m_edit400g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(400)], dummy, 10));
        m_edit400b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(400)], dummy, 10));

        m_edit450r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(450)], dummy, 10));
        m_edit450g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(450)], dummy, 10));
        m_edit450b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(450)], dummy, 10));

        m_edit500r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(500)], dummy, 10));
        m_edit500g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(500)], dummy, 10));
        m_edit500b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(500)], dummy, 10));

        m_edit550r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(550)], dummy, 10));
        m_edit550g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(550)], dummy, 10));
        m_edit550b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(550)], dummy, 10));

        m_edit600r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(600)], dummy, 10));
        m_edit600g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(600)], dummy, 10));
        m_edit600b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(600)], dummy, 10));

        m_edit650r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(650)], dummy, 10));
        m_edit650g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(650)], dummy, 10));
        m_edit650b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(650)], dummy, 10));

        m_edit700r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(700)], dummy, 10));
        m_edit700g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(700)], dummy, 10));
        m_edit700b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(700)], dummy, 10));

        m_edit750r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(750)], dummy, 10));
        m_edit750g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(750)], dummy, 10));
        m_edit750b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(750)], dummy, 10));

        m_edit800r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(800)], dummy, 10));
        m_edit800g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(800)], dummy, 10));
        m_edit800b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(800)], dummy, 10));

        m_edit850r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(850)], dummy, 10));
        m_edit850g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(850)], dummy, 10));
        m_edit850b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(850)], dummy, 10));

        m_edit900r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(900)], dummy, 10));
        m_edit900g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(900)], dummy, 10));
        m_edit900b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(900)], dummy, 10));

        m_edit950r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(950)], dummy, 10));
        m_edit950g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(950)], dummy, 10));
        m_edit950b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(950)], dummy, 10));

        m_edit1000r.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(1000)], dummy, 10));
        m_edit1000g.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(1000)], dummy, 10));
        m_edit1000b.SetWindowText(itoa(theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(1000)], dummy, 10));
    }
    INIT_EASYSIZE;

    // Leemos el ratio configurado y lo aplicamos
    double ratio_inicial = theApp.m_configuracion.m_ratio_inicial;
    CString csRatio;
    csRatio.Format("%.2f",ratio_inicial);
    m_ratio.SetWindowText(csRatio);

    // Ancho y alto del display actualmente (solo como referencia para redimensionar el dialogo)
    RECT Rect;
    m_control_group.GetClientRect(&Rect);
    long anchoDisplay = abs(Rect.right);
    long altoDisplay = abs(Rect.bottom);

    // Ancho y alto del dialogo  actualmente
    this->GetWindowRect(&Rect);
    long anchoDialogo = abs(Rect.right);
    long altoDialogo = abs(Rect.bottom);

    // Ancho y alto de todo menos el display actualmente
    m_anchoResto = anchoDialogo - anchoDisplay;
    m_altoResto = altoDialogo - altoDisplay;

    // Ancho y alto correcto del display 
    controlImagenes.SetAnchoImagen(theApp.m_ParamIni.Cam.anchoImagen);
    controlImagenes.SetAltoImagen(theApp.m_ParamIni.Cam.anchoImagen);
    anchoDisplay = (long)floor(theApp.m_ParamIni.Cam.anchoImagen * ratio_inicial + 0.5);
    altoDisplay = (long)floor(theApp.m_ParamIni.Cam.anchoImagen * ratio_inicial + 0.5);
    this->SetWindowPos(NULL,0,0,anchoDisplay+m_anchoResto,altoDisplay+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);

    // Se redimensionara el display en funcion del display (limitandolo al tamaño de la imagen 
    // y forzando a que sea cuadrado)
    controlImagenes.ActualizaZoom(m_control_group, m_control); // Inicializacion MIL necesaria dependiente del dialogo

    //Inicialializa los graficos con la ventana padre, el numero de valores del eje X, 
    //el valor inicial maximo del eje Y, el titulo, y los textos de los ejes
    m_histograma.Inicializar(&m_histograma_frame,NUM_VALORES_HISTO, 100, 0, 100.0 / NUM_VALORES_HISTO, "TITULO","Reflectancia","Frecuencia relativa");
    m_espectros.Inicializar(&m_espectros_frame, theApp.m_ParamIni.nBandas-1, 100, 400, 50, "TITULO","Longitud de Onda (nm)","Reflectancia");
    this->Invalidate(FALSE);

    // Uso alternativo de Analisis por linea de comandos necesaria para regenerar automaticamente los .pix de todas las imagenes de un directorio dado
    if (theApp.m_lpCmdLine[0] != _T('\0'))
    {
        // Ejecucion automatica
    
        // Buscar todos los archivos de datos
        CFileFinder	_finder;
	    CFileFinder::CFindOpts	opts;
	    // Set CFindOpts object
	    opts.sBaseFolder = theApp.m_lpCmdLine;
	    opts.sFileMask.Format("*_01.tif");
	    opts.bSubfolders = true;
	    opts.FindNormalFiles();
	    _finder.RemoveAll();
	    _finder.Find(opts);
        // Fin de la busqueda

        // Para cada archivo, leer la informacion y volcarla al fichero general
        int nValidImageCount = 0;
        for (int i=0 ; i<_finder.GetFileCount();i++)
        {
            m_csNombreBanda = _finder.GetFilePath(i);
            CString csDirectorio = m_csNombreBanda.Left(m_csNombreBanda.ReverseFind('\\'));
            chdir(csDirectorio);
            //Extrae campo actual
            m_nCampoActual = atoi((LPCTSTR )m_csNombreBanda.Mid(m_csNombreBanda.GetLength() - 10,3));
            m_nFilaActual = atoi((LPCTSTR )m_csNombreBanda.Mid(m_csNombreBanda.GetLength() - 14,3));

            if (m_nFilaActual == 0 || m_csNombreBanda.GetLength() < 15 || m_csNombreBanda[m_csNombreBanda.GetLength()-15] != '_')
            {
                m_bFormato2d = false;
            }
            else
            {
                m_bFormato2d = true;
            }

            AbrirTodas();
            OnGuardar();
        }
        CString csMessage; 
        csMessage.Format("Se han regenerado ficheros .pix para %d imagenes en %s",i,theApp.m_lpCmdLine);
        AfxMessageBox(csMessage);

        exit(0);
    }

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CAnalisisDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// Acciones al cerrar el dialogo
void CAnalisisDlg::OnDestroy() 
{
    CString csRatio;
    m_ratio.GetWindowText(csRatio);
    theApp.m_configuracion.m_ratio_inicial = atof((LPCSTR)csRatio);

    // Guardamos editables en configuracion
    UpdateData(TRUE);
    theApp.m_configuracion.m_dConfiabilidad = atof(m_csUmbralConfiabilidad);
    theApp.m_configuracion.m_dDistancia     = atof(m_csUmbralDistancia);  
    theApp.m_configuracion.m_dMinRef        = atof(m_dMinRef);              
    theApp.m_configuracion.Guardar();

    // Hay que liberar la memoria de asociaciones minerales (no se puede hacer en su destructor porque tenemos que asegurarnos de que se haga antes de liberar Minerales)
    theApp.m_asociaciones.Liberar();
    m_asociacionesDlg.Liberar();

//    CDialog::OnDestroy();
}

void CAnalisisDlg::OnPaint() 
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
        m_espectros.MuestraGrafico();
        m_histograma.MuestraGrafico();

	    CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CAnalisisDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}





///////////////////////////////////////////////////////////////////////////////////////////////////
//
// FUNCIONES AUXILIARES GENERALES
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Devuelve el numero de filtro [0..MAX_NUM_IMAGENES-1] cuya longitud de onda configurada sea nWavelength
// Si no se encuentra, se devuelve -1
int CAnalisisDlg::GetFilterOfWavelength(int nWavelength) 
{
    for (int i = 0;i<MAX_NUM_IMAGENES;i++)
    {
        if (theApp.m_ParamIni.Rueda.espectro[i] == nWavelength)
            return i;
    }

    ASSERT(FALSE);
    return -1;
}









///////////////////////////////////////////////////////////////////////////////////////////////////
//
// ABRIR / CERRAR / CONTROL IMAGENES
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// carga la imagen correspondiente al campo anterior al actual
void CAnalisisDlg::OnAnterior() 
{
    // Comprobar que no hay cambios en areas
    if (m_guardar.IsWindowEnabled() == TRUE)
    {
        int nMessageRes = AfxMessageBox("¿Desea guardar modificaciones en las areas?", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION) ;
        if (nMessageRes == IDYES)
            OnGuardar();
        else if (nMessageRes == IDCANCEL)
            return;
        else
            m_guardar.EnableWindow(FALSE);
    }


    // Componer nuevo nombre de fichero para la siguiente banda 
    if (m_bFormato2d)
    {
        // Version 2 dimensiones
        m_csNombreBanda.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                    m_nFilaActual,m_nCampoAnterior,
                                                    m_csNombreBanda.Right(7));
    }
    else
    {
        // version 1d
        m_csNombreBanda.Format("%s%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-10),
                                                        m_nCampoAnterior,
                                                        m_csNombreBanda.Right(10-3));
    }

    // Actualizar navegacion
    m_BotonSiguiente.EnableWindow(TRUE);
    m_nCampoSiguiente = m_nCampoActual;
    m_nCampoActual = m_nCampoAnterior;
    m_nCampoAnterior = BuscarCampoAnterior();
    if (m_nCampoAnterior == -1)
        m_BotonAnterior.EnableWindow(FALSE);
    else
        m_BotonAnterior.EnableWindow(TRUE);

    AbrirTodas();
	
}

// carga la imagen correspondiente al campo siguiente al actual
void CAnalisisDlg::OnSiguiente() 
{
    // Comprobar que no hay cambios en areas
    if (m_guardar.IsWindowEnabled() == TRUE)
    {
        int nMessageRes = AfxMessageBox("¿Desea guardar modificaciones en las areas?", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION) ;
        if (nMessageRes == IDYES)
            OnGuardar();
        else if (nMessageRes == IDCANCEL)
            return;
        else
            m_guardar.EnableWindow(FALSE);
    }

    // Componer nuevo nombre de fichero para la siguiente banda 
    if (m_bFormato2d)
    {
        // Version 2 dimensiones
        m_csNombreBanda.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                    m_nFilaActual,m_nCampoSiguiente,
                                                    m_csNombreBanda.Right(7));
    }
    else
    {
        // version 1d
        m_csNombreBanda.Format("%s%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-10),
                                                        m_nCampoSiguiente,
                                                        m_csNombreBanda.Right(10-3));
    }

    // Actualizar navegacion
    m_BotonAnterior.EnableWindow(TRUE);
    m_nCampoAnterior = m_nCampoActual;
    m_nCampoActual = m_nCampoSiguiente;
    m_nCampoSiguiente = BuscarCampoSiguiente();
    if (m_nCampoSiguiente == -1)
        m_BotonSiguiente.EnableWindow(FALSE);
    else
        m_BotonSiguiente.EnableWindow(TRUE);

    AbrirTodas();
}

void CAnalisisDlg::OnSuperior() 
{
    // Comprobar que no hay cambios en areas
    if (m_guardar.IsWindowEnabled() == TRUE)
    {
        int nMessageRes = AfxMessageBox("¿Desea guardar modificaciones en las areas?", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION) ;
        if (nMessageRes == IDYES)
            OnGuardar();
        else if (nMessageRes == IDCANCEL)
            return;
        else
            m_guardar.EnableWindow(FALSE);
    }

    // Componer nuevo nombre de fichero para la siguiente banda 
    m_csNombreBanda.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                        m_nCampoSuperior,m_nCampoActual,
                                                        m_csNombreBanda.Right(7));
    /* version 1d
    m_csNombreBanda.Format("%s%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-10),
                                                            m_nCampoAnterior,
                                                            m_csNombreBanda.Right(10-3));
*/

    // Actualizar navegacion
    m_BotonInferior.EnableWindow(TRUE);
    m_nCampoInferior = m_nFilaActual;
    m_nFilaActual = m_nCampoSuperior;
    m_nCampoSuperior = BuscarCampoSuperior();
    if (m_nCampoSuperior == -1)
        m_BotonSuperior.EnableWindow(FALSE);
    else
        m_BotonSuperior.EnableWindow(TRUE);

    AbrirTodas();
}

void CAnalisisDlg::OnInferior() 
{
    // Comprobar que no hay cambios en areas
    if (m_guardar.IsWindowEnabled() == TRUE)
    {
        int nMessageRes = AfxMessageBox("¿Desea guardar modificaciones en las areas?", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION) ;
        if (nMessageRes == IDYES)
            OnGuardar();
        else if (nMessageRes == IDCANCEL)
            return;
        else
            m_guardar.EnableWindow(FALSE);
    }

    // Componer nuevo nombre de fichero para la siguiente banda 
    m_csNombreBanda.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                        m_nCampoInferior,m_nCampoActual,
                                                        m_csNombreBanda.Right(7));
    /* version 1d
    m_csNombreBanda.Format("%s%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-10),
                                                            m_nCampoAnterior,
                                                            m_csNombreBanda.Right(10-3));
*/

    // Actualizar navegacion
    m_BotonSuperior.EnableWindow(TRUE);
    m_nCampoSuperior = m_nFilaActual;
    m_nFilaActual = m_nCampoInferior;
    m_nCampoInferior = BuscarCampoInferior();
    if (m_nCampoInferior == -1)
        m_BotonInferior.EnableWindow(FALSE);
    else
        m_BotonInferior.EnableWindow(TRUE);

    AbrirTodas();
}

// Busca el campo disponible inmediatamente siguiente
// csNombreBanda - string con el nombre completo de un fichero de imagen espectral
//                 correspondiente al campo actual
int CAnalisisDlg::BuscarCampoSiguiente()
{

    int nCamposInexistentes = 0;

    CString csSiguieteFichero;
    int nPresuntoCampo = m_nCampoActual;


    while(nCamposInexistentes < MAX_CAMPOS_INEXISTENTES)
    {
        nPresuntoCampo++;
        if (m_bFormato2d)
        {
            // Version 2 dimensiones
            csSiguieteFichero.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                                m_nFilaActual,nPresuntoCampo,
                                                                m_csNombreBanda.Right(7));
        }
        else
        {
            // version 1d
            csSiguieteFichero.Format("%s%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-10),
                                                                nPresuntoCampo,
                                                                m_csNombreBanda.Right(10-3));
        }
        // Comprueba si existe el fichero (usando el mismo espectro que el propocionado)
        if (GetFileAttributes(csSiguieteFichero) != 0xFFFFFFFF)
            return nPresuntoCampo;
        else
            nCamposInexistentes++;
    } //while

    //No hay ningun campo siguiente
    return -1;
}

// Busca el campo disponible inmediatamente anterior
// csNombreBanda - string con el nombre completo de un fichero de imagen espectral
//                 correspondiente al campo actual
int CAnalisisDlg::BuscarCampoAnterior()
{

    int nCamposInexistentes = 0;

    CString csSiguieteFichero;
    int nPresuntoCampo = m_nCampoActual;

    while(nCamposInexistentes < MAX_CAMPOS_INEXISTENTES && nPresuntoCampo != 1)
    {
        nPresuntoCampo--;
        if (m_bFormato2d)
        {
            // Version 2 dimensiones
            csSiguieteFichero.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                                m_nFilaActual,nPresuntoCampo,
                                                                m_csNombreBanda.Right(7));
        }
        else
        {
            // version 1d
            csSiguieteFichero.Format("%s%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-10),
                                                                nPresuntoCampo,
                                                                m_csNombreBanda.Right(10-3));
        }
        // Comprueba si existe el fichero (usando el mismo espectro que el propocionado)
        if (GetFileAttributes(csSiguieteFichero) != 0xFFFFFFFF)
            return nPresuntoCampo;
        else
            nCamposInexistentes++;
    } //while

    //No hay ningun campo siguiente
    return -1;
}

// Busca el campo disponible inmediatamente superior
int CAnalisisDlg::BuscarCampoSuperior()
{

    int nCamposInexistentes = 0;

    CString csSiguieteFichero;
    int nPresuntaFila = m_nFilaActual;


    while(nCamposInexistentes < MAX_CAMPOS_INEXISTENTES && nPresuntaFila != 1)
    {
        nPresuntaFila--;
        // Version 2 dimensiones
        csSiguieteFichero.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                            nPresuntaFila,m_nCampoActual,
                                                            m_csNombreBanda.Right(7));
        // Comprueba si existe el fichero (usando el mismo espectro que el propocionado)
        if (GetFileAttributes(csSiguieteFichero) != 0xFFFFFFFF)
            return nPresuntaFila;
        else
            nCamposInexistentes++;
    } //while

    //No hay ningun campo siguiente
    return -1;
}

// Busca el campo disponible inmediatamente inferior
int CAnalisisDlg::BuscarCampoInferior()
{

    int nCamposInexistentes = 0;

    CString csSiguieteFichero;
    int nPresuntaFila = m_nFilaActual;


    while(nCamposInexistentes < MAX_CAMPOS_INEXISTENTES)
    {
        nPresuntaFila++;
        // Version 2 dimensiones
        csSiguieteFichero.Format("%s%03d_%03d%s",m_csNombreBanda.Left( m_csNombreBanda.GetLength()-14),
                                                            nPresuntaFila,m_nCampoActual,
                                                            m_csNombreBanda.Right(7));
        // Comprueba si existe el fichero (usando el mismo espectro que el propocionado)
        if (GetFileAttributes(csSiguieteFichero) != 0xFFFFFFFF)
            return nPresuntaFila;
        else
            nCamposInexistentes++;
    } //while

    //No hay ningun campo siguiente
    return -1;
}

// Seleccion Tab
void CAnalisisDlg::OnSelchangeTab1(NMHDR* pNMHDR, LRESULT* pResult) 
{
	int numeroFiltro;

    int currTab = m_tabControl.GetCurSel();

    if (currTab != -1)
    {
        numeroFiltro = filterOfTab[currTab];

        //Se muestra las lineas adecuadas de todas las areas en HISTOGRAMA
        HTREEITEM item = m_tree_areas.GetFirstVisibleItem();
        if (item != NULL)
        {
            m_histograma.MostrarTodas(false); // borra todas las lineas de todas las areas 
            do //por cada area
            {
                if (m_tree_areas.GetCheck(item) == TRUE) //mostrar linea de area solo si seleccionado su checkbox
                {
                    CArea* area = m_areas.GetArea((int)item);
                    //muestra el histograma para este area y este espectro
                    m_histograma.MostrarLinea(area->m_arrIdHistograma[numeroFiltro-1],true); 
                }
            } while ((item = m_tree_areas.GetNextVisibleItem(item)) != NULL);
        }
        // Añadimos linea vertical
        m_espectros.NuevaLineaVertical(theApp.m_ParamIni.Rueda.espectro[numeroFiltro-1],LINEA_VERTICAL_R, LINEA_VERTICAL_G, LINEA_VERTICAL_B);

        this->Invalidate(FALSE); //actualiza m_control y m_espectros
        controlImagenes.MostrarImagen(numeroFiltro, m_control);
        //"Deseleccionamos" el boton RGB y clasificar
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        m_ButtonClasificar.SetCheck(FALSE);
        m_ButtonClasificar.EnableWindow(TRUE); // habilitado
        m_botonDistancia.SetCheck(FALSE); // "no hundido"
        m_botonConfiabilidad.SetCheck(FALSE); // "no hundido"
        m_clasificados.EnableWindow(FALSE);            // deshabilitamos lista de clasificados
        m_no_clasificados.EnableWindow(FALSE);            // deshabilitamos lista de clasificados
    }
    else
    {
        //No hay ningun tab seleccionado. Puede ser que se hayan deseleccionado por codigo (llamandose a esta funcion)
    }

	*pResult = 0;
}

// Funcion auxiliar comun a OnAbrir y OnAbrirTodas para cargar una nueva imagen dado su numero de filtro y su nombre
//  bMostrar indica si debe de mostrarse la imagen en pantalla o solo cargar la imagen en memoria
void CAnalisisDlg::NuevaImagen(char* FileName, int numFiltro, bool bMostrar)
{
    CString error;
    if ( controlImagenes.CargarImagen(FileName, m_control, numFiltro, bMostrar)) //Ojo: CargarImagen deja el zoom en 0.5 siempre
    {
        //Añadir un tab
        TCITEM tcItem;
        tcItem.mask = TCIF_TEXT;
        TCHAR drive[_MAX_DRIVE];
        TCHAR dir[_MAX_DIR];
        TCHAR fname[_MAX_FNAME];
        TCHAR ext[_MAX_EXT];                
        _tsplitpath(FileName, drive, dir, fname, ext );               
        tcItem.pszText = _T(&fname[strlen(fname)-2]); //Nombre del tab sera solo el numero de filtro

        HabilitarControlesFiltro(numFiltro,TRUE);

        filterOfTab.push_back(numFiltro);
        m_tabControl.InsertItem(m_nNumTab, &tcItem);
        m_nNumTab++;

		//Ponemos nombre de campo en la ventana
		fname[strlen(fname)-3] = 0; // forzamos a que termine despues del nombre de la serie
		SetWindowText(fname);

		if (bMostrar)
		{
			// Seleccionar nuevo tab 
			m_tabControl.SetCurSel(m_nNumTab-1); //No provoca llamada a OnSelchangeTab1
            // El display tiene que ajustarse al tamaño de imagen y el zoom actual
            double dZoom = controlImagenes.GetFactorZoom();
            double anchoImagen = (long)floor(controlImagenes.GetAnchoImagen() * dZoom  + 0.5);
            double altoImagen = (long)floor(controlImagenes.GetAltoImagen() * dZoom + 0.5);
            m_control_group.SetWindowPos(NULL,0,0,anchoImagen,altoImagen, SWP_NOMOVE | SWP_NOZORDER);
            //Ajustamos la ventana principal tambien
            this->SetWindowPos(NULL,0,0,anchoImagen+m_anchoResto,altoImagen+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);
//			UPDATE_EASYSIZE; //necesario? - alternativa a SetWindowPos?

            // TRUCO para evitar que los rectangulos se pinten con un reborde negro. Pero la carga de imagenes es mas lenta
//			controlImagenes.ActualizaZoom(m_control_group, m_control);

            this->Invalidate(FALSE);
		}
    }
    else
    {
       error.Format("Error: No se puede cargar %s. Compruebe el formato", FileName);
       AfxMessageBox(error, MB_ICONERROR | MB_OK);
    }
}

// Abrir todos las imagenes correspondientes a todos los espectros a partir
// de un fichero elegido, cerrando previamente todas las imagenes abiertas
void CAnalisisDlg::OnAbrirTodas() 
{
    static char BASED_CODE szFilter[] = 
    "TIFF Image (*.tif)|*.tif|All Files (*.*)|*.*||";
    CString error;
  
    // Comprobar que no hay cambios en areas
    if (m_guardar.IsWindowEnabled() == TRUE)
    {
        int nMessageRes = AfxMessageBox("¿Desea guardar modificaciones en las areas?", MB_YESNOCANCEL | MB_DEFBUTTON3 | MB_ICONQUESTION) ;
        if (nMessageRes == IDYES)
            OnGuardar();
        else if (nMessageRes == IDCANCEL)
            return;
        else
            m_guardar.EnableWindow(FALSE);
    }

    CFileDialog d( TRUE, "tif", NULL, OFN_HIDEREADONLY |
	     OFN_OVERWRITEPROMPT , szFilter, NULL );
    d.m_ofn.lpstrInitialDir= theApp.m_configuracion.m_csDefaultPath; 

    if (d.DoModal()==IDOK) 
    {
        m_csNombreBanda = d.GetPathName(); //solo se puede seleccionar un fichero
        theApp.m_configuracion.m_csDefaultPath = m_csNombreBanda.Left(m_csNombreBanda.ReverseFind('\\')+1);

        //Extrae campo actual
        m_nCampoActual = atoi((LPCTSTR )m_csNombreBanda.Mid(m_csNombreBanda.GetLength() - 10,3));
        m_nFilaActual = atoi((LPCTSTR )m_csNombreBanda.Mid(m_csNombreBanda.GetLength() - 14,3));

        if (m_nFilaActual == 0 || m_csNombreBanda.GetLength() < 15 || m_csNombreBanda[m_csNombreBanda.GetLength()-15] != '_')
        {
            m_bFormato2d = false;
            m_BotonSuperior.EnableWindow(FALSE);
            m_BotonInferior.EnableWindow(FALSE);
        }
        else
        {
            m_bFormato2d = true;
            m_nCampoSuperior = BuscarCampoSuperior();
            if (m_nCampoSuperior == -1)
                m_BotonSuperior.EnableWindow(FALSE);
            else
                m_BotonSuperior.EnableWindow(TRUE);
            m_nCampoInferior = BuscarCampoInferior();
            if (m_nCampoInferior == -1)
                m_BotonInferior.EnableWindow(FALSE);
            else
                m_BotonInferior.EnableWindow(TRUE);

        }

        // Inicializar botones campo anterior, siguiete, inferior y superior
        m_nCampoSiguiente = BuscarCampoSiguiente();
        if (m_nCampoSiguiente == -1)
            m_BotonSiguiente.EnableWindow(FALSE);
        else
            m_BotonSiguiente.EnableWindow(TRUE);

        m_nCampoAnterior = BuscarCampoAnterior();
        if (m_nCampoAnterior == -1)
            m_BotonAnterior.EnableWindow(FALSE);
        else
            m_BotonAnterior.EnableWindow(TRUE);
        
        // Cargar opciones asociaciones
	    if (fopen(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, "rt") == NULL)
            // No hay fichero asociaciones especifico. copiar y usar el general
            CopyFile(theApp.m_configuracion.m_csPathEjecutable+FICHERO_ASOCIACIONES_SELECCIONADAS,theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, FALSE);
        else
            // Hay fichero especifico, sobreescribir general para que siempre sea igual al especifico actual
            CopyFile(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, theApp.m_configuracion.m_csPathEjecutable+FICHERO_ASOCIACIONES_SELECCIONADAS,FALSE);
        theApp.m_asociaciones.CargarFicheroAsociacionesSeleccionadas(theApp.m_configuracion.m_csDefaultPath + FICHERO_ASOCIACIONES_SELECCIONADAS);

        // Habilitar menu asociaciones minerales
        m_Menu.EnableMenuItem(0,MF_BYPOSITION | MF_ENABLED); 
        DrawMenuBar(); 


        AbrirTodas();
    } // if (d.DoModal()==IDOK)
}

// Abrir todos las imagenes correspondientes a todos los espectros a partir
// de un fichero elegido, cerrando previamente todas las imagenes abiertas
void CAnalisisDlg::AbrirTodas() 
{
    char FileName[_MAX_PATH];
    int numFiltro = -1;
    bool bMostrar = true; //cargaremos todos los filtros pero solo hay que mostrar (con la inicializacion de display, overlay, etc necesaria) uno de ellos

    strcpy (FileName, LPCTSTR(m_csNombreBanda));

    int currTab = m_tabControl.GetCurSel();
    int nNumFiltro;
    if (currTab != -1)
        nNumFiltro = filterOfTab[currTab];
    else if (m_ButtonRGB.GetState() == 1)
        nNumFiltro = -1; // RGB
    else
        nNumFiltro = m_nUltimoFiltro; // Por defecto, mostrar el que estuviera

    CerrarTodas();

    // Cargamos las imagenes de todos los espectros
    bool bEscalaCargada = false;
    for (int i = 0; i<theApp.m_ParamIni.nBandas;i++)
    {
        if (theApp.m_ParamIni.Rueda.espectro[i] != 0) //no usar filtro pancromatico
        {
            char arrNumFiltro[2];
            //Obtenemos el filtro del espectro
            numFiltro = theApp.m_ParamIni.filtro[i]+1;//+1 porque filtro[] devuelve [0 .. NUM_FILTROS-1]
            itoa(numFiltro,arrNumFiltro,10);
            //Asegurarnos que sea un formato de dos caracteres 01, 02 , ...
            if (strlen(arrNumFiltro)==1)
            {
                arrNumFiltro[1] = arrNumFiltro[0];
                arrNumFiltro[0] = '0';
            }
            // sobreescribimos los dos caracteres con los del filtro en cuestion
            FileName[strlen(FileName)-1 -3/*EXT*/-1/*.*/ ] = arrNumFiltro[1];
            FileName[strlen(FileName)-1 -3/*EXT*/-1/*.*/ -1 ] = arrNumFiltro[0];

			NuevaImagen(FileName,numFiltro, bMostrar);
            bMostrar = false;

            //Suponemos que todos las imagenes tienen la misma escala reflectancia-gris, por lo que leemos solo de la primera imagen
            if (!bEscalaCargada)
            {
                bool bError = false;
                try
                {
                    INTERFAZ_LIBTIF::LeerTags(FileName,theApp.m_nBitsProfundidad,theApp.m_dEscalaReflectancia);
                    if (theApp.m_nBitsProfundidad < 0 )
                        theApp.m_nBitsProfundidad = 8; // por defecto las imagenes estan en 8 bit
                    controlImagenes.DisplayBits(theApp.m_nBitsProfundidad); //para que el display muestre las imagenes de la profundidad leida
                    if (theApp.m_dEscalaReflectancia <= 0)
                        bError = true;
                }
                catch(...)
                {
                    bError = true;
                }
                if (bError)
                {
                    sendlog("CAnalisisDlg::AbrirTodas", "No se ha podido leer la escala de reflectancia-gris de la imagen, se usará la escala de patrones");
                    theApp.m_dEscalaReflectancia = theApp.m_ParamIni.escala;
                }
                else
                    bEscalaCargada = true;
            }
        }
    }

    if (nNumFiltro == -1 ) // RGB
    {
        OnRgb();
    }
    else //cualquier tab
        // Añadimos linea vertical
        m_espectros.NuevaLineaVertical(theApp.m_ParamIni.Rueda.espectro[nNumFiltro-1],LINEA_VERTICAL_R, LINEA_VERTICAL_G, LINEA_VERTICAL_B);


	// Cargamos areas de fichero (en caso de que haya)
    CString csCampo;
    GetWindowText(csCampo);
    m_areas.Cargar(csCampo, this);

    m_ButtonRGB.EnableWindow(TRUE);
    m_ButtonClasificar.EnableWindow(TRUE);

}

// Libera memoria y cierra el tab de todas las imagenes
void CAnalisisDlg::CerrarTodas() 
{
    // Si hay elemento seleccionado, hay que matar el timer de parpadeo
    HTREEITEM item = m_tree_areas.GetSelectedItem();
    if (item != NULL)
    {
        CWnd::KillTimer((int)item); // evidentemente este area esta seleccionada y ya no lo estará
    }

    m_ButtonRGB.EnableWindow(FALSE);               // deshabilitamos RGB
    m_ButtonClasificar.EnableWindow(FALSE);        // deshabilitamos Clasificar
    m_ButtonGuardarClasificar.EnableWindow(FALSE); // deshabilitamos Guardar Clasificar
    m_clasificados.EnableWindow(FALSE);            // deshabilitamos lista de clasificados
    m_no_clasificados.EnableWindow(FALSE);            // deshabilitamos lista de clasificados
    m_botonDistancia.EnableWindow(FALSE); 
    m_botonConfiabilidad.EnableWindow(FALSE); 
    m_checkInfDer.EnableWindow(FALSE); 
    m_checkInfIzq.EnableWindow(FALSE); 
    m_checkSupDer.EnableWindow(FALSE); 
    m_checkSupIzq.EnableWindow(FALSE); 
    int nOk = m_clasificados.DeleteAllItems();     // borramos la lista de clasificados
    ASSERT(nOk);
    nOk = m_no_clasificados.DeleteAllItems();     // borramos la lista de clasificados
    ASSERT(nOk);

    // Deshabilitar botones 
//    m_BotonCerrar.EnableWindow(FALSE);
    DeshabilitarRGB();
    DeshabilitarPropiedadesArea();

    //"Deseleccionamos" el boton RGB y Clasificar
    m_ButtonRGB.SetCheck(FALSE);
    m_ButtonClasificar.SetCheck(FALSE);

    // Quitar todos los tabs
    m_nNumTab = 0;
    filterOfTab.clear();
    m_tabControl.DeleteAllItems();

    //borrar espectros e histogramas
    m_espectros.EliminarLineas();
    m_histograma.EliminarLineas();


    //Borrar lista interna de areas
    m_areas.EliminarTodas();

	// Borrar lista de minerales y la seleccion que estuviese hecha
    m_combo_mineral.ResetContent();

    //borrar lista de areas de TreeCtrl
    // Debe hacerse despues de  borrar m_areas porque al borrar todas se selecciona involuntariamente
    // el area siguiente y esto provoca llamada a OnSelchangedAreas, que dará resultados incorrectos 
    // si no estan borradas antes las areas
    nOk = m_tree_areas.DeleteAllItems();
    ASSERT(nOk);

    //Cierra todas las imagenes y sus areas
    controlImagenes.EliminarTodas();


}

/* OBSOLETO
// Libera memoria y cierra el tab de la imagen actual
void CAnalisisDlg::OnCerrar() 
{
	int numeroFiltro, nuevoFiltro;
    int numTab = m_tabControl.GetCurSel();
    if (numTab == -1)
    {
        // No hay tab seleccionado, no hacer nada
    }
    else
    {
        numeroFiltro = filterOfTab[numTab];

        //libera la imagen
        nuevoFiltro = controlImagenes.EliminarImagen(numeroFiltro, m_control);
        if (nuevoFiltro != -1)
            // Seleccionar tab correspondiente a nuevo filtro
            m_tabControl.SetCurSel(TabOfFilter(nuevoFiltro)); //No provoca llamada a OnSelchangeTab1
        filterOfTab.erase(filterOfTab.begin() + numTab);
        m_nNumTab--;
        if (m_nNumTab == 0)
            // Deshabilitar boton de cerrar 
            m_BotonCerrar.EnableWindow(FALSE);
        HabilitarControlesFiltro(numeroFiltro,FALSE);

         //cierra el tab
        if (!m_tabControl.DeleteItem(numTab))
        {
            // error inesperado en DeleteItem 
            ASSERT(false);
        }
    }
    if (m_nNumTab < 3)
    {
        m_ButtonRGB.EnableWindow(FALSE);
    }
}
*/

//Funcion auxiliar para obtener el tab del filtro "numFiltro"
//-1 si no esta cargado ese filtro
int  CAnalisisDlg::TabOfFilter(int numFiltro)
{
    int nTab = -1;
    int nCount = 0;
    vector<int>::iterator i = filterOfTab.begin(); ;
    while (i != filterOfTab.end() && nTab==-1)
    {
        if (*i == numFiltro)
            nTab = nCount;
        i++;
        nCount++;
    }

    return nTab;
}

// a partir de un path completo, extrae como int los dos ultimos caracteres del nombre del fichero
int  CAnalisisDlg::ExtraerNumFiltro(char* nomFich)
{
    char stringNumFiltro[2];

    TCHAR drive[_MAX_DRIVE];
    TCHAR dir[_MAX_DIR];
    TCHAR fname[_MAX_FNAME];
    TCHAR ext[_MAX_EXT];                
    _tsplitpath(nomFich, drive, dir, fname, ext );     
    stringNumFiltro[1] = fname[strlen(fname)-1];
    stringNumFiltro[0] = fname[strlen(fname)-2];

    return atoi(stringNumFiltro);
}







///////////////////////////////////////////////////////////////////////////////////////////////////
//
// CONTROL MOUSE / AREAS
//
///////////////////////////////////////////////////////////////////////////////////////////////////
//AUXILIAR
// añade un area nueva en:
// - En la lista de areas interna (control)
// Calcula los valores espectrales
void CAnalisisDlg::CrearArea(CRect& rect)
{
    // Comprobamos que el area no sea nula
    ASSERT (rect.left != rect.right && rect.top != rect.bottom);

    double* pY      = new double[theApp.m_ParamIni.nBandas - 1]; //valores espectrales. pancromatico no
    double* pPerInf = new double[theApp.m_ParamIni.nBandas - 1]; //valores espectrales. pancromatico no
    double* pPerSup = new double[theApp.m_ParamIni.nBandas - 1]; //valores espectrales. pancromatico no
    double pY_histo[NUM_VALORES_HISTO]; //array se reutilizará para las lineas de histogramas

    // Añadimos area al contenedor
    CArea* pArea = m_areas.NuevaArea(rect);

    // Calculamos los valores de reflectancia para este area para todos los filtros
    long histo[NUM_VALORES_HISTO];
    double moda,sigma;
    int nPercentilInf, nPercentilSup;

    // HISTOGRAMA: No se reserva memoria para el filtro pancromatico
    for (int i=1;i<theApp.m_ParamIni.nBandas;i++) //NOS SALTAMOS "BANDA" PANCROMATICA !!!!!
    {
        // CalculaHistograma devuelve un histograma de NUM_VALORES_HISTO (independientemente de la profundidad de la imagen) y la moda y los percentiles expresados tambien en esa magnitud
        if (controlImagenes.CalculaHistograma(i+1, rect, theApp.m_nBitsProfundidad, histo, moda, sigma, nPercentilInf, nPercentilSup)) // +1 porque banda 1 es filtro 2
        {
            int nNumPixels = (rect.right - rect.left) * (rect.bottom - rect.top);
            for (int j=0; j<NUM_VALORES_HISTO;j++)
            {
                // Transformamos los valores a porcentajes
                pY_histo[j] = 100 * (double)histo[j]/nNumPixels;
            }

            //almacenar valores histograma
            pArea->CopiarHistograma(pY_histo,i-1); //-1 porque es un indice de espectros (sin pancromatico)

            //nuevo valor para la linea espectral
//double dGris8bit = pow(moda,8.0/theApp.m_nBitsProfundidad);
            pY[i-1]=moda*theApp.m_dEscalaReflectancia; // a reflectancia
            if (pY[i-1] > 100)
                pY[i-1] = 100; //a veces dEscalaReflectancia es ligeramente superior (redondeos del procesador)

            //nuevos valores para los percentiles
            pPerInf[i-1]=nPercentilInf*theApp.m_dEscalaReflectancia; // a reflectancia
            if (pPerInf[i-1] > 100)
                pPerInf[i-1] = 100; //a veces dEscalaReflectancia es ligeramente superior (redondeos del procesador)
            pPerSup[i-1]=nPercentilInf*theApp.m_dEscalaReflectancia; // a reflectancia
            if (pPerSup[i-1] > 100)
                pPerSup[i-1] = 100; //a veces dEscalaReflectancia es ligeramente superior (redondeos del procesador)

        }
        else
        {
            //indicar que este espectro no esta disponible
            pArea->CopiarHistograma(NULL,i-1); //-1 porque es un indice de espectros (sin pancromatico)
            pY[i-1]=0;
            pPerInf[i-1]=0;
            pPerSup[i-1]=0;
        }
    }

    // identificamos el mineral (con probabilidades) a partir de su espectro
        // Al estar los valores de entrenamiento (reflectancias de minerales) en niveles de gris, hay que transformar
//    double arrEspectrosNivGris[MAX_NUM_IMAGENES]; //Auxiliar
//    for (i=0;i<theApp.m_ParamIni.nBandas-1;i++)
//        arrEspectrosNivGris[i] = pY[i] / theApp.m_dEscalaReflectancia; //de reflectancias a niveles de gris
//    theApp.m_minerales.Identificar(arrEspectrosNivGris, pArea->m_listIdentificacion);
    theApp.m_minerales.Identificar(pY, pArea->m_listIdentificacion);

    //almacenar valores identificacion, espectros y percentiles en contenedor areas
    pArea->CopiarEspectro(pY);
    pArea->CopiarPercentilInf(pPerInf);
    pArea->CopiarPercentilSup(pPerSup);

    // Eliminar array dinamico de espectros (se reservara de nuevo en DefinirArea)
    delete [] pY;
    delete [] pPerInf;
    delete [] pPerSup;

    // DEFINE AREA
    DefinirArea(*pArea);

    //Habilitar boton de guardar areas (hay cambios)
    m_guardar.EnableWindow(TRUE);
}

// INTERFAZ: Llamado por CAreas para dibujar cada area tras cargar de fichero
// añade un area nueva en:
// - En la propia imagen, como un rectangulo
// - En el dialogo, en la lista de areas
// - En el grafico de espectros, como una nueva linea
void CAnalisisDlg::DibujarArea(CArea& area)
{
    // Pintar area en imagen
    controlImagenes.RedibujaRect(area.m_rect,area.m_nColorIndex);

    // DEFINE AREA
    DefinirArea(area);
}

//AUXILIAR
// añade un area nueva en:
// - En el dialogo, en la lista de areas (TreeCtrl)
// - En el grafico de espectros, como una nueva linea
void CAnalisisDlg::DefinirArea(CArea& area)
{
    // Comprobamos que el area no sea nula
    ASSERT (area.m_rect.left != area.m_rect.right && area.m_rect.top != area.m_rect.bottom);

    double* m_pY                = new double[theApp.m_ParamIni.nBandas - 1]; //valores espectrales. pancromatico no
    double* m_pY_histo = NULL; //puntero que se reutilizará para las lineas de histogramas
    int* arrIdGrafico = new int[theApp.m_ParamIni.nBandas];     //array id de lineas de histograma. pancromatico si

    // Añadimos area a TREE CONTROL
    HTREEITEM hArea = m_tree_areas.InsertItem(_T(area.ComponerNombre()), NULL);//, TVI_SORT);
    m_tree_areas.SetCheck(hArea);

    // Usaremos el identificador del tree control como identificador del area
    area.m_nId = (int)hArea;

    long r,g,b;
    m_areas.GetRGB(area,r,g,b); //color del area

    arrIdGrafico[0] = 0;
    for (int i=1;i<theApp.m_ParamIni.nBandas;i++) //NOS SALTAMOS "BANDA" PANCROMATICA !!!!!
    {
        m_pY_histo = new double[NUM_VALORES_HISTO]; //valores espectrales

        //recuperar valores histograma
        area.GetHistograma(m_pY_histo,i-1); //-1 porque es un indice de espectros (sin pancromatico)

        //Añadir linea al histograma
        if (m_pY_histo != NULL)
        {
            arrIdGrafico[i] = m_histograma.NuevaLinea(m_pY_histo,r,g,b);
            //Ocultar todas las lineas de histograma menos la del espectro actualmente seleccionado
            m_histograma.MostrarLinea(arrIdGrafico[i],false);
        }
        else
        {
            // esto ocurrirá cuando no existian valores de histograma para este espectro
            delete [] m_pY_histo;
            arrIdGrafico[i] = 0;
        }
    }

    //recuperar valores espectros
    area.GetEspectro(m_pY);

    // Añadir linea de espectros
    int nIdGrafico = m_espectros.NuevaLinea(m_pY,r,g,b);

    //Almacenar identificadores de lineas graficas en area
    area.CopiarIdHistogramas(arrIdGrafico);
    area.SetIdEspectros(nIdGrafico);

    // Si hay tab seleccionado (NO RGB)
    if(m_tabControl.GetCurSel() != -1)
        //Ocultar todas las lineas de histograma menos la del espectro actualmente seleccionado
        m_histograma.MostrarLinea(arrIdGrafico[filterOfTab[m_tabControl.GetCurSel()]-1],true);

    delete [] arrIdGrafico;
}

void CAnalisisDlg::OnLButtonDown(UINT nFlags, CPoint point) 
{
    // mouse position 'point' does not include caption, so firt transform
    // 'point' to real screen coordinates (including caption)
    POINT pointUnderCaption;
    pointUnderCaption.x= 0; 
    pointUnderCaption.y=0;
    this->ClientToScreen(&pointUnderCaption);
    point.x+=pointUnderCaption.x;
    point.y+=pointUnderCaption.y;

    // Now transform to client
    m_control.ScreenToClient(&point);

    if (m_control.ChildWindowFromPoint(point) != NULL) // inside window
    {
        CRect area;
        bool bAreaDefinida = controlImagenes.OnLButtonDown(point,area,m_areas.GetCurrentColorIndex());
        if (bAreaDefinida)
        {
            CrearArea(area);
        }
        this->Invalidate(FALSE); //actualiza m_control y m_espectros

    }

	CDialog::OnLButtonDown(nFlags, point);
}

void CAnalisisDlg::OnLButtonUp(UINT nFlags, CPoint point) 
{
    // mouse position 'point' does not include caption, so firt transform
    // 'point' to real screen coordinates (including caption)
    POINT pointUnderCaption;
    pointUnderCaption.x= 0; 
    pointUnderCaption.y=0;
    this->ClientToScreen(&pointUnderCaption);
    point.x+=pointUnderCaption.x;
    point.y+=pointUnderCaption.y;

    // Now transform to client
    m_control.ScreenToClient(&point);

    if (m_control.ChildWindowFromPoint(point) != NULL) // inside window
    {
        CRect area;
        bool bAreaDefinida = controlImagenes.OnLButtonUp(point,area,m_areas.GetCurrentColorIndex());
        if (bAreaDefinida)
        {
            CrearArea(area);
        }
        this->Invalidate(FALSE); //actualiza m_control y m_espectros

    }
    
	CDialog::OnLButtonUp(nFlags, point);
}

void CAnalisisDlg::OnMouseMove(UINT nFlags, CPoint point) 
{
    if(nFlags && MK_LBUTTON)
    {
        // mouse position 'point' does not include caption, so firt transform
        // 'point' to real screen coordinates (including caption)
        POINT pointUnderCaption;
        pointUnderCaption.x= 0; 
        pointUnderCaption.y=0;
        this->ClientToScreen(&pointUnderCaption);
        point.x+=pointUnderCaption.x;
        point.y+=pointUnderCaption.y;

        // Now transform to client
        m_control.ScreenToClient(&point);

        if (m_control.ChildWindowFromPoint(point) != NULL) // inside window
        {
            controlImagenes.OnMouseMove(point);
            m_control.Invalidate(TRUE);
        }
    }
    
	CDialog::OnMouseMove(nFlags, point);
}

void CAnalisisDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
    UPDATE_EASYSIZE;
    if (::IsWindow(m_control.m_hWnd))
    {
        double dRatio = controlImagenes.ActualizaZoom(m_control_group, m_control);
        CString csRatio;
        csRatio.Format("%.2f",dRatio);
        m_ratio.SetWindowText(csRatio);

    }
    this->Invalidate(TRUE);
}

// Para evitar que la ventana sea mayor de lo necesario (tamaño suficiente para alojar la imagen en su verdadera magnitud)
void CAnalisisDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{
    if (::IsWindow(m_control.m_hWnd))
    {
        lpMMI->ptMaxTrackSize.x = m_anchoResto + controlImagenes.GetAnchoImagen();
        lpMMI->ptMaxTrackSize.y = m_altoResto + controlImagenes.GetAltoImagen();
        lpMMI->ptMinTrackSize.x = m_anchoResto + (long)floor(controlImagenes.GetAnchoImagen() * 0.5 + 0.5);
        lpMMI->ptMinTrackSize.y = m_altoResto + (long)floor(controlImagenes.GetAltoImagen() * 0.5 + 0.5);
    }
	CDialog::OnGetMinMaxInfo(lpMMI);
}

// Si se edita el ratio de la imagen, hay que actualizarlo
void CAnalisisDlg::OnChangeRatio() 
{
    // OBSOLETO, ahora se hace en OnKillfocusRatio
}

// se actualiza el ratio de la imagen despues de editarlo
void CAnalisisDlg::OnKillfocusRatio() 
{
    // Recuperamos el ratio actual
    double ratioActual = controlImagenes.GetFactorZoom();
    CString csRatioActual;
    csRatioActual.Format("%.2f",ratioActual);

    // Leemos el ratio que se quiere aplicar
    CString csRatio;
    m_ratio.GetWindowText(csRatio);

    // No actuar si se ha modificado el ratio al cambiar el tamaño de la ventana
    // Es necesario comparar en CString para que los redondeos sean equiparables
    if (m_anchoResto != -1 && m_altoResto!= -1 && csRatioActual!=csRatio) 
    {
        // Ancho y alto correcto del display 
        double ratio = atof((LPCSTR)csRatio);
        double anchoDisplay = (long)floor(controlImagenes.GetAnchoImagen() * ratio  + 0.5);
        double altoDisplay = (long)floor(controlImagenes.GetAltoImagen() * ratio + 0.5);
        this->SetWindowPos(NULL,0,0,anchoDisplay+m_anchoResto,altoDisplay+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);
        
        // Se redimensionara el display en funcion del display (limitandolo al tamaño de la imagen 
        // y forzando a que sea cuadrado)
        controlImagenes.ActualizaZoom(m_control_group, m_control); 
    }
	
}





///////////////////////////////////////////////////////////////////////////////////////////////////
//
// AREAS - TREE CONTROL y PROPIEDADES
//
///////////////////////////////////////////////////////////////////////////////////////////////////

void CAnalisisDlg::OnGuardar() 
{
    CString csCampo;
    GetWindowText(csCampo);

	m_areas.Guardar(csCampo, m_bFormato2d, controlImagenes);

    //Deshabilitar boton de guardar areas (no hay cambios)
    m_guardar.EnableWindow(FALSE);
}

void CAnalisisDlg::OnEndlabeleditAreas(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_DISPINFO* pTVDispInfo = (TV_DISPINFO*)pNMHDR;

    if (pTVDispInfo->item.pszText != NULL)
    {
        m_areas.SetText((int)pTVDispInfo->item.hItem,/* (TCHAR*) */pTVDispInfo->item.pszText);
        m_tree_areas.SetItemText(pTVDispInfo->item.hItem,/* (TCHAR*) */pTVDispInfo->item.pszText);

        //Habilitar boton de guardar areas (hay cambios)
        m_guardar.EnableWindow(TRUE);

    }
//    m_tree_areas.SetFocus();
	
	*pResult = 0;
}

// Boton oculto que solo sirve para que se editen las areas al pulsar ENTER
void CAnalisisDlg::OnDummy() 
{
    m_tree_areas.SetFocus();	
}

// Busca el string 'csToFind' en el combobox dado
// Devuelve el indice del combobox o -1 si no se encuentra
int CAnalisisDlg::SelectStringAnywhere(CComboBox& comboBox, CString& csToFind)
{
    for(int i = 0; i < comboBox.GetCount(); i++)
    { /* search */
        CString s;
        comboBox.GetLBText(i, s);
        if(s.Find(csToFind) != -1)
        { /* found match */
            comboBox.SetCurSel(i);
            return i;
        } /* found match */
    } /* search */

    return -1;
}
//Acciones asociadas al seleccionar un area
void CAnalisisDlg::OnSelchangedAreas(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CArea* pArea;

	NM_TREEVIEW* pNMTreeView = (NM_TREEVIEW*)pNMHDR;


    if (pNMTreeView->itemOld.hItem !=NULL)
    {
        // Si habia otro rect seleccionado, hay que repintarlo en su color
        pArea = m_areas.GetArea((int)pNMTreeView->itemOld.hItem);
        controlImagenes.RedibujaRect(pArea->m_rect,pArea->m_nColorIndex);
        CWnd::KillTimer(pArea->m_nId);
    }

    pArea = m_areas.GetArea((int)pNMTreeView->itemNew.hItem);

    if (pArea !=NULL)
    {
        //Habilitar propiedades area
	    m_mineral_static.EnableWindow(TRUE);
	    m_comentario_static.EnableWindow(TRUE);
	    m_calidad_static.EnableWindow(TRUE);
	    m_combo_mineral.EnableWindow(TRUE);
	    m_combo_calidad.EnableWindow(TRUE);
	    m_comentario.EnableWindow(TRUE);
    
        RellenarComboMinerales(pArea);

        // Una vez rellenado el combo, seleccionamos el mineral seleccionado
        int nIndex = SelectStringAnywhere(m_combo_mineral, pArea->m_csMineral);
        if (nIndex == -1)
        {
            // Aun no identificado
            m_combo_mineral.SetCurSel(0); 
        }

        nIndex = m_combo_calidad.FindStringExact(-1,pArea->m_csCalidad);
        if (nIndex != LB_ERR)
        {
            m_combo_calidad.SetCurSel(nIndex);
        }
        else
        {
            ASSERT(FALSE);
            m_combo_calidad.SetCurSel(0); 
        }

        m_comentario.SetWindowText(pArea->m_csComentario);

        // Pintado de areas
        CWnd::SetTimer( pArea->m_nId, INTERVALO_TIMER_SELECCION, 0);
        controlImagenes.SeleccionaRect(pArea->m_rect);

        m_control.Invalidate(TRUE);
    }
	

	*pResult = 0;
}

//Callback para cuando se cambia el check box de "todos los minerales"
void CAnalisisDlg::OnTodosMinerales() 
{

    HTREEITEM item = m_tree_areas.GetSelectedItem();
    CArea* pArea = NULL;
    if (item != NULL)
        pArea = m_areas.GetArea((int)item);

    RellenarComboMinerales(pArea);
}

//rellenamos de nuevo el combo de minerales con las probabilidades (o los minerales sin mas) en orden
void CAnalisisDlg::RellenarComboMinerales(CArea* pArea)
{
    m_combo_mineral.ResetContent();
    m_combo_mineral.AddString(SELECCIONE_MINERAL);
    UpdateData(TRUE);
    if (m_bTodosMinerales == TRUE) // no hay que clasificar el area, si no presentar todos los minerales para que se seleccione manualmente
    {
        // Odenamos minerales por abreviatura para que funcione BuscarPorAbreviatura
        CMineral::m_enumOrden = ORDEN_ABREVIATURA; //m_enumOrden es static
        std::sort(theApp.m_minerales.m_list.begin(),theApp.m_minerales.m_list.end(),ComparadorMinerales);

        for (int i = 0; i<theApp.m_minerales.m_list.size(); i++)
            m_combo_mineral.AddString(theApp.m_minerales.m_list[i]->m_csNombre);

        //El orden del fichero de entrenamiento es en orden de identificador
        CMineral::m_enumOrden = ORDEN_IDENTIFICADOR; // m_enumOrden es static
        std::sort(theApp.m_minerales.m_list.begin(),theApp.m_minerales.m_list.end(),ComparadorMinerales);
    }
    else if (pArea != NULL) //presentar resultados de clasificacion
    {
        CString csProbabilidad;
        list<CResultadoIdentificacion>::iterator it;
        for (it = pArea->m_listIdentificacion.begin(); it != pArea->m_listIdentificacion.end();++it) //Reverse
        {
            csProbabilidad.Format("%.2f",(*it).m_dProbabilidad);
            m_combo_mineral.AddString(csProbabilidad + " - " + theApp.m_minerales.GetNombre((*it).m_nIndice));
        }
    }
}

void CAnalisisDlg::OnDropdownComboMineral() 
{
/*
    //rellenamos de nuevo el combo de minerales con las probabilidades en orden
    m_combo_mineral.ResetContent();
    m_combo_mineral.AddString(SELECCIONE_MINERAL);

    HTREEITEM item = m_tree_areas.GetSelectedItem();
    CArea* pArea;
    if (item != NULL)
        pArea = m_areas.GetArea((int)item);

    UpdateData(TRUE);
    if (m_bTodosMinerales == TRUE) // no hay que clasificar el area, si no presentar todos los minerales para que se seleccione manualmente
    {
        for (int i = 0; i<theApp.m_minerales.m_list.size(); i++)
            m_combo_mineral.AddString(theApp.m_minerales.m_list[i]->m_csNombre);
    }
    else //presentar resultados de clasificacion
    {
        CString csProbabilidad;
        list<CResultadoIdentificacion>::iterator it;
        for (it = pArea->m_listIdentificacion.begin(); it != pArea->m_listIdentificacion.end();++it) //Reverse
        {
            csProbabilidad.Format("%.2f",(*it).m_dProbabilidad);
            m_combo_mineral.AddString(csProbabilidad + " - " + theApp.m_minerales.GetNombre((*it).m_nIndice));
        }
    }
    */
}

void CAnalisisDlg::OnTimer(UINT nIDEvent) 
{
	static int nFlashState;

    CArea* pArea = m_areas.GetArea(nIDEvent);

    if (pArea != NULL) //seguridad
    {
        if (nFlashState == 1)
        {
            controlImagenes.RedibujaRect(pArea->m_rect,pArea->m_nColorIndex);
            nFlashState = 0;
        }
        else
        {
            controlImagenes.SeleccionaRect(pArea->m_rect);
            nFlashState = 1;
        }
    }
    else
        CWnd::KillTimer(nIDEvent); // evidentemente este area esta seleccionada y ya no lo estará

	CDialog::OnTimer(nIDEvent);
}

void CAnalisisDlg::DeshabilitarPropiedadesArea()
{
    //Borrar y Desahabilitar propiedades area
    m_combo_mineral.SetCurSel(0);
    m_combo_calidad.SetCurSel(0);
    m_comentario.SetWindowText("");
    m_mineral_static.EnableWindow(FALSE);
    m_comentario_static.EnableWindow(FALSE);
    m_calidad_static.EnableWindow(FALSE);
    m_combo_mineral.EnableWindow(FALSE);
    m_combo_calidad.EnableWindow(FALSE);
    m_comentario.EnableWindow(FALSE);
}

// ELIMINAR CON [SUP]
void CAnalisisDlg::OnKeydownAreas(NMHDR* pNMHDR, LRESULT* pResult) 
{
	TV_KEYDOWN* pTVKeyDown = (TV_KEYDOWN*)pNMHDR;

    if (pTVKeyDown->wVKey == 46) // DELETE
    {
        HTREEITEM item = m_tree_areas.GetSelectedItem();
        if (item != NULL)
        {
            CWnd::KillTimer((int)item); // evidentemente este area esta seleccionada y ya no lo estará
            CArea* area = m_areas.GetArea((int)item);
            controlImagenes.BorraRect(area->m_rect);
            m_espectros.EliminarLinea(area->m_nIdEspectros);
            for (int i=0;i<theApp.m_ParamIni.nBandas;i++)
            {
                if (area->m_arrIdHistograma[i] != 0)
                    m_histograma.EliminarLinea(area->m_arrIdHistograma[i]);
            }
            m_areas.Eliminar((int)item);
            m_tree_areas.DeleteItem(item);
            if (m_tree_areas.GetCount() == 0) // no hay mas areas
            {
                DeshabilitarPropiedadesArea();
            }

            //Habilitar boton de guardar areas (hay cambios)
            m_guardar.EnableWindow(TRUE);

            this->Invalidate(FALSE);
        }
    }
    
	*pResult = 0;
}

// Callback necesario para pintar las areas de colores en el TreeView
void CAnalisisDlg::OnCustomdrawAreas ( NMHDR* pNMHDR, LRESULT* pResult )
{
    NMLVCUSTOMDRAW* pLVCD = reinterpret_cast<NMLVCUSTOMDRAW*>( pNMHDR );

    // Take the default processing unless we 
    // set this to something else below.
    *pResult = CDRF_DODEFAULT;

    // First thing - check the draw stage. If it's the control's prepaint
    // stage, then tell Windows we want messages for every item.

    if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        *pResult = CDRF_NOTIFYITEMDRAW;
        }
    else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
        {
        // This is the prepaint stage for an item. Here's where we set the
        // item's text color. Our return value will tell Windows to draw the
        // item itself, but it will use the new color we set here.

            long r,g,b;

            m_areas.GetRGB((int)pLVCD->nmcd.dwItemSpec,r,g,b); //color de este item

            // Store the color back in the NMLVCUSTOMDRAW struct.
 //           pLVCD->clrTextBk = RGB(r,g,b);
//            pLVCD->clrText = RGB(255-r,255-g,255-b);
            pLVCD->clrText = RGB(r,g,b);

        // Tell Windows to paint the control itself.
        *pResult = CDRF_DODEFAULT;
        }
}

// Callback para gestionar los CHECKBOX
void CAnalisisDlg::OnClickAreas(NMHDR* pNMHDR, LRESULT* pResult) 
{
    CPoint pt; 
    GetCursorPos(&pt); 
    m_tree_areas.ScreenToClient(&pt); 
    UINT uFlags; 

    HTREEITEM ht = m_tree_areas.HitTest(pt, &uFlags); 

    if(ht != NULL)
    {
        // There is an item under the cursor.
        m_tree_areas.SelectItem(ht);

        // See what exactly was under the cursor:
        switch(uFlags)
        {
            case TVHT_ONITEMSTATEICON:
            {
                // It is the icon (checkbox)
                BOOL chkbox = m_tree_areas.GetCheck(ht);
                CArea* area = m_areas.GetArea((int)ht);
                ASSERT(area);
                if(chkbox)
                {
                    // Boton esta DESELECCIONADO
                    m_espectros.MostrarLinea(area->m_nIdEspectros, false);
                    // Si hay tab seleccionado (NO RGB)
                    if(m_tabControl.GetCurSel() != -1)
                        m_histograma.MostrarLinea(area->m_arrIdHistograma[filterOfTab[m_tabControl.GetCurSel()]-1], false);
                    this->Invalidate(FALSE);
                }
                else
                {
                    // Boton esta SELECCIONADO
                    m_espectros.MostrarLinea(area->m_nIdEspectros, true);
                    // Si hay tab seleccionado (NO RGB)
                    if(m_tabControl.GetCurSel() != -1)
                        m_histograma.MostrarLinea(area->m_arrIdHistograma[filterOfTab[m_tabControl.GetCurSel()]-1], true);
                    this->Invalidate(FALSE);
                }
                break;
            }
            case TVHT_ONITEMBUTTON:
                // It is the button
                break;
        }


    }

      *pResult = 0;
}


void CAnalisisDlg::OnSelchangeComboMineral() 
{
    HTREEITEM item = m_tree_areas.GetSelectedItem();
    if (item != NULL)
    {
        CArea* area = m_areas.GetArea((int)item);
        int nIndex = m_combo_mineral.GetCurSel();
        if (nIndex != CB_ERR)
        {
            CString csTexto;
            m_combo_mineral.GetLBText(nIndex,csTexto);
            int nPosNombre = csTexto.Find("- ");
            if (nPosNombre == -1) //no se ha encontrado, estamos en la opcion de "Todos", al no haber distancias, no hay guion tampoco
                area->m_csMineral = csTexto; 
            else
                area->m_csMineral = csTexto.Right(csTexto.GetLength()-nPosNombre-2);
            area->m_csMineralAbreviatura = theApp.m_minerales.GetAbreviatura(area->m_csMineral);

            //Actualizar nombre en TreeCtrl
            m_tree_areas.SetItemText(item,area->ComponerNombre());

            //Habilitar boton de guardar areas (hay cambios)
            m_guardar.EnableWindow(TRUE);
        }
        else
            ASSERT(FALSE); //posible cambio de ventana mientras se estaba seleccionando
    }
    else
        ASSERT(FALSE);	
}

void CAnalisisDlg::OnSelchangeComboCalidad() 
{
    HTREEITEM item = m_tree_areas.GetSelectedItem();
    if (item != NULL)
    {
        CArea* area = m_areas.GetArea((int)item);
        int nIndex = m_combo_calidad.GetCurSel();
        CString csTexto;
        m_combo_calidad.GetLBText(nIndex,csTexto);
        area->m_csCalidad = csTexto;

        //Actualizar nombre en TreeCtrl
        m_tree_areas.SetItemText(item,area->ComponerNombre());

        //Habilitar boton de guardar areas (hay cambios)
        m_guardar.EnableWindow(TRUE);
    }
    else
        ASSERT(FALSE);
}

void CAnalisisDlg::OnChangeComentario() 
{
    HTREEITEM item = m_tree_areas.GetSelectedItem();
    if (item != NULL)
    {
        CArea* area = m_areas.GetArea((int)item);
        CString csTexto;
        m_comentario.GetWindowText(csTexto);
        area->m_csComentario = csTexto;

        //Habilitar boton de guardar areas (hay cambios)
        m_guardar.EnableWindow(TRUE);

    }
    else
        ASSERT(FALSE);
}










///////////////////////////////////////////////////////////////////////////////////////////////////
//
// RGB
//
///////////////////////////////////////////////////////////////////////////////////////////////////
void CAnalisisDlg::OnRgb() 
{

    //MostrarImagenRGB tendra en cuenta los totales al promediar
//    int nTotalB = ActualizarTotalB();
//    int nTotalG = ActualizarTotalG();
//    int nTotalR = ActualizarTotalR();

    // Componemos la imagen 
    // if (m_ButtonRGB.IsWindowEnabled() == TRUE) //comentado porque no cargaba el RGB al cambiar de imagen. Esto era una proteccion para ... que? Probablemente al cambiar de button a radio button
    //{
        controlImagenes.MostrarImagenRGB(   theApp.m_configuracion.m_arrPorcentajesR,
                                                theApp.m_configuracion.m_arrPorcentajesG,
                                                theApp.m_configuracion.m_arrPorcentajesB, 
                                                m_control,100,100,100,m_bRGB_Modificado);//nTotalR,nTotalG,nTotalB))
        m_bRGB_Modificado = false; //para que la proxima vez no se calcule
        // El display tiene que ajustarse al tamaño de imagen y el zoom actual
/* --
        double dZoom = controlImagenes.GetFactorZoom();
        double anchoImagen = (long)floor(controlImagenes.GetAnchoImagen() * dZoom  + 0.5);
        double altoImagen = (long)floor(controlImagenes.GetAltoImagen() * dZoom + 0.5);
        m_control_group.SetWindowPos(NULL,0,0,anchoImagen,altoImagen, SWP_NOMOVE | SWP_NOZORDER);
        //Ajustamos la ventana principal tambien
        this->SetWindowPos(NULL,0,0,anchoImagen+m_anchoResto,altoImagen+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);
//	    UPDATE_EASYSIZE;
	    controlImagenes.ActualizaZoom(m_control_group, m_control);
*/
        // Deseleccionamos los tabs de filtros
        m_tabControl.DeselectAll(FALSE);
        m_ButtonClasificar.SetCheck(FALSE);
        m_botonDistancia.SetCheck(FALSE); // "no hundido"
        m_botonConfiabilidad.SetCheck(FALSE); // "no hundido"
        m_clasificados.EnableWindow(FALSE);            // deshabilitamos lista de clasificados
        m_no_clasificados.EnableWindow(FALSE);            // deshabilitamos lista de clasificados

        m_espectros.EliminarLineaVertical(); //ya no hay espectro seleccionado, boorar linea indicatoria
        m_histograma.MostrarTodas(false); // borra todas las lineas de todas las areas 
        this->Invalidate(FALSE); //actualiza m_control y m_espectros

        //"Hundimos" el boton RGB
        m_ButtonRGB.SetCheck(TRUE);
        m_ButtonRGB.EnableWindow(FALSE); // deshabilitado

        //Habilitamos guardar la imagen RGB
        m_ButtonGuardarRGB.EnableWindow(TRUE);
        m_ButtonClasificar.EnableWindow(TRUE); // habilitado
    //}

}

//Guarda la imagen RGB generada en un fichero
void CAnalisisDlg::OnGuardarRgb() 
{
    static char BASED_CODE szFilter[] = 
    "TIFF Image (*.tif)|*.tif|All Files (*.*)|*.*||";
  
    //Recuperar nombre muestra para presentar 
    CString csMuestra;
    GetWindowText(csMuestra);
    csMuestra += "_RGB";

    CFileDialog d( FALSE, "tif", csMuestra, OFN_HIDEREADONLY |
	     OFN_OVERWRITEPROMPT , szFilter, NULL );
    d.m_ofn.lpstrInitialDir= m_csNombreBanda;//theApp.m_configuracion.m_csDefaultPath; 

    if (d.DoModal()==IDOK) 
    {
        CString csNombreFichero = d.GetPathName(); //solo se puede seleccionar un fichero
        controlImagenes.GuardarRGB(csNombreFichero);	
    }
}

void CAnalisisDlg::DeshabilitarRGB()
{
    m_ButtonRGB.EnableWindow(FALSE);
    m_static400.EnableWindow(FALSE);
    m_static450.EnableWindow(FALSE);
    m_static500.EnableWindow(FALSE);
    m_static550.EnableWindow(FALSE);
    m_static600.EnableWindow(FALSE);
    m_static650.EnableWindow(FALSE);
    m_static700.EnableWindow(FALSE);
    m_static750.EnableWindow(FALSE);
    m_static800.EnableWindow(FALSE);
    m_static850.EnableWindow(FALSE);
    m_static900.EnableWindow(FALSE);
    m_static950.EnableWindow(FALSE);
    m_static1000.EnableWindow(FALSE);
    m_edit400b.EnableWindow(FALSE);
    m_edit400g.EnableWindow(FALSE);
    m_edit400r.EnableWindow(FALSE);
    m_edit450b.EnableWindow(FALSE);
    m_edit450g.EnableWindow(FALSE);
    m_edit450r.EnableWindow(FALSE);
    m_edit500b.EnableWindow(FALSE);
    m_edit500g.EnableWindow(FALSE);
    m_edit500r.EnableWindow(FALSE);
    m_edit550b.EnableWindow(FALSE);
    m_edit550g.EnableWindow(FALSE);
    m_edit550r.EnableWindow(FALSE);
    m_edit600b.EnableWindow(FALSE);
    m_edit600g.EnableWindow(FALSE);
    m_edit600r.EnableWindow(FALSE);
    m_edit650b.EnableWindow(FALSE);
    m_edit650g.EnableWindow(FALSE);
    m_edit650r.EnableWindow(FALSE);
    m_edit700b.EnableWindow(FALSE);
    m_edit700g.EnableWindow(FALSE);
    m_edit700r.EnableWindow(FALSE);
    m_edit750b.EnableWindow(FALSE);
    m_edit750g.EnableWindow(FALSE);
    m_edit750r.EnableWindow(FALSE);
    m_edit800b.EnableWindow(FALSE);
    m_edit800g.EnableWindow(FALSE);
    m_edit800r.EnableWindow(FALSE);
    m_edit850b.EnableWindow(FALSE);
    m_edit850g.EnableWindow(FALSE);
    m_edit850r.EnableWindow(FALSE);
    m_edit900b.EnableWindow(FALSE);
    m_edit900g.EnableWindow(FALSE);
    m_edit900r.EnableWindow(FALSE);
    m_edit950b.EnableWindow(FALSE);
    m_edit950g.EnableWindow(FALSE);
    m_edit950r.EnableWindow(FALSE);
    m_edit1000b.EnableWindow(FALSE);
    m_edit1000g.EnableWindow(FALSE);
    m_edit1000r.EnableWindow(FALSE);

    m_ButtonGuardarRGB.EnableWindow(FALSE);

    ActualizarTotalR();
    ActualizarTotalG();
    ActualizarTotalB();
}

//Habilita o deshabilita (segun bEstado) la introduccion de procentajes par el filtro numFiltro
void CAnalisisDlg::HabilitarControlesFiltro(int numFiltro,BOOL bEstado)
{
    
    switch (theApp.m_ParamIni.Rueda.espectro[numFiltro-1])
    {
        case 400:
        {
            m_static400.EnableWindow(bEstado);
            m_edit400b.EnableWindow(bEstado);
            m_edit400g.EnableWindow(bEstado);
            m_edit400r.EnableWindow(bEstado);
            break;
        }
        case 450:
        {
            m_static450.EnableWindow(bEstado);
            m_edit450b.EnableWindow(bEstado);
            m_edit450g.EnableWindow(bEstado);
            m_edit450r.EnableWindow(bEstado);
            break;
        }
        case 500:
        {
            m_static500.EnableWindow(bEstado);
            m_edit500b.EnableWindow(bEstado);
            m_edit500g.EnableWindow(bEstado);
            m_edit500r.EnableWindow(bEstado);
            break;
        }
        case 550:
        {
            m_static550.EnableWindow(bEstado);
            m_edit550b.EnableWindow(bEstado);
            m_edit550g.EnableWindow(bEstado);
            m_edit550r.EnableWindow(bEstado);
            break;
        }
        case 600:
        {
            m_static600.EnableWindow(bEstado);
            m_edit600b.EnableWindow(bEstado);
            m_edit600g.EnableWindow(bEstado);
            m_edit600r.EnableWindow(bEstado);
            break;
        }
        case 650:
        {
            m_static650.EnableWindow(bEstado);
            m_edit650b.EnableWindow(bEstado);
            m_edit650g.EnableWindow(bEstado);
            m_edit650r.EnableWindow(bEstado);
            break;
        }
        case 700:
        {
            m_static700.EnableWindow(bEstado);
            m_edit700b.EnableWindow(bEstado);
            m_edit700g.EnableWindow(bEstado);
            m_edit700r.EnableWindow(bEstado);
            break;
        }
        case 750:
        {
            m_static750.EnableWindow(bEstado);
            m_edit750b.EnableWindow(bEstado);
            m_edit750g.EnableWindow(bEstado);
            m_edit750r.EnableWindow(bEstado);
            break;
        }
        case 800:
        {
            m_static800.EnableWindow(bEstado);
            m_edit800b.EnableWindow(bEstado);
            m_edit800g.EnableWindow(bEstado);
            m_edit800r.EnableWindow(bEstado);
            break;
        }
        case 850:
        {
            m_static850.EnableWindow(bEstado);
            m_edit850b.EnableWindow(bEstado);
            m_edit850g.EnableWindow(bEstado);
            m_edit850r.EnableWindow(bEstado);
            break;
        }
        case 900:
        {
            m_static900.EnableWindow(bEstado);
            m_edit900b.EnableWindow(bEstado);
            m_edit900g.EnableWindow(bEstado);
            m_edit900r.EnableWindow(bEstado);
            break;
        }
        case 950:
        {
            m_static950.EnableWindow(bEstado);
            m_edit950b.EnableWindow(bEstado);
            m_edit950g.EnableWindow(bEstado);
            m_edit950r.EnableWindow(bEstado);
            break;
        }
        case 1000:
        {
            m_static1000.EnableWindow(bEstado);
            m_edit1000b.EnableWindow(bEstado);
            m_edit1000g.EnableWindow(bEstado);
            m_edit1000r.EnableWindow(bEstado);
            break;
        }
        default:
        {
            ASSERT(FALSE);
            break;
        }
    }

    ActualizarTotalR();
    ActualizarTotalG();
    ActualizarTotalB();
}

////////////////////////////////////
// Actualizacion Porcentajes
// Si modificamos cualquier porcentaje, se deselecciona el boton RGB
// Devuelve la suma para esta banda
////////////////////////////////////
int CAnalisisDlg::ActualizarTotalB()
{
    int nTotal = 0;
    char Dummy[5];

    nTotal = 0;
    //Solo sumamos un espectro si la ventana esta activada
    if (m_edit400b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(400)];
    if (m_edit450b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(450)];
    if (m_edit500b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(500)];
    if (m_edit550b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(550)];
    if (m_edit600b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(600)];
    if (m_edit650b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(650)];
    if (m_edit700b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(700)];
    if (m_edit750b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(750)];
    if (m_edit800b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(800)];
    if (m_edit850b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(850)];
    if (m_edit900b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(900)];
    if (m_edit950b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(950)];
    if (m_edit1000b.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(1000)];

    m_TotalB.SetWindowText(itoa(nTotal,Dummy,10));

    if (nTotal == 100)
        m_TotalB.SetTextColor(GREEN);
    else if (nTotal != 0)
        m_TotalB.SetTextColor(RED);
    else
        m_TotalB.SetTextColor(BLACK);
    return nTotal;
}
int CAnalisisDlg::ActualizarTotalG()
{
    int nTotal = 0;
    char Dummy[5];

    nTotal = 0;
    //Solo sumamos un espectro si la ventana esta activada
    if (m_edit400g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(400)];
    if (m_edit450g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(450)];
    if (m_edit500g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(500)];
    if (m_edit550g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(550)];
    if (m_edit600g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(600)];
    if (m_edit650g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(650)];
    if (m_edit700g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(700)];
    if (m_edit750g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(750)];
    if (m_edit800g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(800)];
    if (m_edit850g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(850)];
    if (m_edit900g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(900)];
    if (m_edit950g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(950)];
    if (m_edit1000g.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(1000)];

    m_TotalG.SetWindowText(itoa(nTotal,Dummy,10));
    if (nTotal == 100)
        m_TotalG.SetTextColor(GREEN);
    else if (nTotal != 0)
        m_TotalG.SetTextColor(RED);
    else
        m_TotalG.SetTextColor(BLACK);

    return nTotal;
}
int CAnalisisDlg::ActualizarTotalR()
{
    int nTotal = 0;
    char Dummy[5];

    nTotal = 0;
    //Solo sumamos un espectro si la ventana esta activada
    if (m_edit400r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(400)];
    if (m_edit450r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(450)];
    if (m_edit500r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(500)];
    if (m_edit550r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(550)];
    if (m_edit600r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(600)];
    if (m_edit650r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(650)];
    if (m_edit700r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(700)];
    if (m_edit750r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(750)];
    if (m_edit800r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(800)];
    if (m_edit850r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(850)];
    if (m_edit900r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(900)];
    if (m_edit950r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(950)];
    if (m_edit1000r.IsWindowEnabled())
        nTotal += theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(1000)];

    if (nTotal == 100)
        m_TotalR.SetTextColor(GREEN);
    else if (nTotal != 0)
        m_TotalR.SetTextColor(RED);
    else
        m_TotalR.SetTextColor(BLACK);

    m_TotalR.SetWindowText(itoa(nTotal,Dummy,10));
    return nTotal;
}


// BLUE
void CAnalisisDlg::OnChangeEdit400B() 
{
    if (m_edit400b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit400b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(400)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit450B() 
{
    if (m_edit450b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit450b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(450)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit500B() 
{
    if (m_edit500b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit500b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(500)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit550B() 
{
    if (m_edit550b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit550b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(550)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit600B() 
{
    if (m_edit600b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit600b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(600)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit650B() 
{
    if (m_edit650b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit650b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(650)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit700B() 
{
    if (m_edit700b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit700b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(700)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit750B() 
{
    if (m_edit750b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit750b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(750)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit800B() 
{
    if (m_edit800b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit800b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(800)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit850B() 
{
    if (m_edit850b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit850b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(850)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit900B() 
{
    if (m_edit900b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit900b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(900)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit950B() 
{
    if (m_edit950b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit950b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(950)] = atoi(value);
        ActualizarTotalB(); 
    }
}

void CAnalisisDlg::OnChangeEdit1000B() 
{
    if (m_edit1000b.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit1000b.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesB[GetFilterOfWavelength(1000)] = atoi(value);
        ActualizarTotalB(); 
    }
}


// BLUE
void CAnalisisDlg::OnChangeEdit400G() 
{
    if (m_edit400g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit400g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(400)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit450G() 
{
    if (m_edit450g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit450g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(450)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit500G() 
{
    if (m_edit500g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit500g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(500)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit550G() 
{
    if (m_edit550g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit550g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(550)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit600G() 
{
    if (m_edit600g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit600g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(600)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit650G() 
{
    if (m_edit650g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit650g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(650)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit700G() 
{
    if (m_edit700g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit700g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(700)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit750G() 
{
    if (m_edit750g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit750g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(750)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit800G() 
{
    if (m_edit800g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit800g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(800)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit850G() 
{
    if (m_edit850g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit850g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(850)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit900G() 
{
    if (m_edit900g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit900g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(900)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit950G() 
{
    if (m_edit950g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit950g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(950)] = atoi(value);
        ActualizarTotalG(); 
    }
}

void CAnalisisDlg::OnChangeEdit1000G() 
{
    if (m_edit1000g.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit1000g.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesG[GetFilterOfWavelength(1000)] = atoi(value);
        ActualizarTotalG(); 
    }
}


// RED
void CAnalisisDlg::OnChangeEdit400R() 
{
    if (m_edit400r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        char value[MAX_EDIT_BYTES];
        m_edit400r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(400)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit450R() 
{
    if (m_edit450r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit450r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(450)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit500R() 
{
    if (m_edit500r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit500r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(500)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit550R() 
{
    if (m_edit550r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit550r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(550)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit600R() 
{
    if (m_edit600r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit600r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(600)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit650R() 
{
    if (m_edit650r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit650r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(650)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit700R() 
{
    if (m_edit700r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit700r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(700)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit750R() 
{
    if (m_edit750r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit750r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(750)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit800R() 
{
    if (m_edit800r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit800r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(800)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit850R() 
{
    if (m_edit850r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit850r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(850)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit900R() 
{
    if (m_edit900r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit900r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(900)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit950R() 
{
    if (m_edit950r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit950r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(950)] = atoi(value);
        ActualizarTotalR(); 
    }
}

void CAnalisisDlg::OnChangeEdit1000R() 
{
    if (m_edit1000r.IsWindowEnabled() == TRUE)
    {
        m_bRGB_Modificado = true;
        m_ButtonRGB.SetCheck(FALSE);
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        ActualizarTotalR(); 
        char value[MAX_EDIT_BYTES];
        m_edit1000r.GetLine(0,value,MAX_EDIT_BYTES);
        theApp.m_configuracion.m_arrPorcentajesR[GetFilterOfWavelength(1000)] = atoi(value);
        ActualizarTotalR(); 
    }
}












///////////////////////////////////////////////////////////
//
// OTROS
//
////////////////////////////////////////////////////////////

void CAnalisisDlg::OnGuardarClasificar() 
{
    static char BASED_CODE szFilter[] = 
    "TIFF Image (*.tif)|*.tif|All Files (*.*)|*.*||";
  
    //Recuperar nombre muestra para presentar 
    CString csMuestra;
    GetWindowText(csMuestra);
    csMuestra += "_CLASIFICACION";

    CFileDialog d( FALSE, "tif", csMuestra, OFN_HIDEREADONLY |
	     OFN_OVERWRITEPROMPT , szFilter, NULL );
    d.m_ofn.lpstrInitialDir= m_csNombreBanda;//theApp.m_configuracion.m_csDefaultPath; 

    if (d.DoModal()==IDOK) 
    {
        CString csNombreFichero = d.GetPathName(); //solo se puede seleccionar un fichero
        theApp.clasificacion.GuardarClasificar(csNombreFichero);	
    }	
}

// Callback necesario para pintar los minerales clasificados de colores en el TreeView
void CAnalisisDlg::OnCustomdrawClasificados ( NMHDR* pNMHDR, LRESULT* pResult )
{
    if (m_clasificados.IsWindowEnabled())
    {
        NMTVCUSTOMDRAW* pLVCD = reinterpret_cast<NMTVCUSTOMDRAW*>( pNMHDR );

        // comprobamos que estamos en el control correcto
        if (pLVCD->nmcd.hdr.idFrom != IDC_CLASIFICADOS)
            return;

        // Take the default processing unless we 
        // set this to something else below.
        *pResult = CDRF_DODEFAULT;

        // First thing - check the draw stage. If it's the control's prepaint
        // stage, then tell Windows we want messages for every item.
        if ( CDDS_PREPAINT == pLVCD->nmcd.dwDrawStage )
        {
            *pResult = CDRF_NOTIFYITEMDRAW;
        }
        else if ( CDDS_ITEMPREPAINT == pLVCD->nmcd.dwDrawStage )
        {
            // This is the prepaint stage for an item. Here's where we set the
            // item's text color. Our return value will tell Windows to draw the
            // item itself, but it will use the new color we set here.

            list<CMineralClasificado>::iterator it = m_listClasificados.begin();
            bool bEncontrado = false;
            while (!bEncontrado && it!=m_listClasificados.end())
            {
                if ((*it).m_hTree == (HTREEITEM)pLVCD->nmcd.dwItemSpec)
                    bEncontrado = true;
                else
                    ++it;
            }
            if (bEncontrado)
            {

                // Store the color back in the NMLVCUSTOMDRAW struct.
                //           pLVCD->clrTextBk = RGB(r,g,b);
                //            pLVCD->clrText = RGB(255-r,255-g,255-b);
                BYTE r,g,b;
                if ((*it).m_nIndex != -1)
                {
                    int nIndiceColor = theApp.m_minerales.GetIndiceColor((*it).m_nIndex);
                    r = theApp.m_minerales.m_lut_clasificacion[nIndiceColor][0];
                    g = theApp.m_minerales.m_lut_clasificacion[nIndiceColor][1];
                    b = theApp.m_minerales.m_lut_clasificacion[nIndiceColor][2];
                }
                else //se trata de pixeles clasificados como MUY OSCUROS
                {
                    r = 0;
                    g = 0;
                    b = 0;
                }
                if ((r+g+b) / 3 > 126)
                    pLVCD->clrText = RGB(0,0,0); //si el mineral es de media mas bien blanco, ponemos el texto negro
                else
                    pLVCD->clrText = RGB(255,255,255); //si no, blanco

                pLVCD->clrTextBk = RGB(r,g,b); //Fondo del color del mineral
            }
            // Tell Windows to paint the control itself.
            *pResult = CDRF_DODEFAULT;
        }
    }
}


void CAnalisisDlg::OnClasificar() 
{
    if (m_ButtonClasificar.IsWindowEnabled() == TRUE)
    {
        list<CMineralClasificado>::iterator it;

        //"Seleccionamos" el boton 
        m_nUltimoFiltro = m_tabControl.GetCurSel(); //salvamos el ultimo filtro (RGB seria "-1") para el caso de que se cambie de campo
        if (m_nUltimoFiltro != -1)
        {
            // Deseleccionamos los tabs de filtros
            m_tabControl.DeselectAll(FALSE);
            m_nUltimoFiltro = filterOfTab[m_nUltimoFiltro]; //salvamos el ultimo filtro para el caso de que se cambie de campo
        }
        else
        {
            // Deseleccionamos RGB
            m_ButtonRGB.SetCheck(FALSE);
        }

        m_ButtonClasificar.SetCheck(TRUE); // "hundido"
        m_ButtonClasificar.EnableWindow(FALSE); // deshabilitado
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        //Habilitamos guardar la imagen 
        m_ButtonGuardarClasificar.EnableWindow(TRUE);
        m_clasificados.EnableWindow(TRUE);            // habilitamos lista de clasificados
        m_no_clasificados.EnableWindow(TRUE);            // habilitamos lista de clasificados
        m_botonDistancia.EnableWindow(TRUE); 
        m_botonConfiabilidad.EnableWindow(TRUE); 
        m_botonDistancia.SetCheck(FALSE); // "no hundido"
        m_botonConfiabilidad.SetCheck(FALSE); // "no hundido"
        m_checkInfDer.EnableWindow(TRUE); 
        m_checkInfIzq.EnableWindow(TRUE); 
        m_checkSupDer.EnableWindow(TRUE); 
        m_checkSupIzq.EnableWindow(TRUE); 
        m_espectros.EliminarLineaVertical(); //ya no hay espectro seleccionado, borrar linea indicatoria
        m_histograma.MostrarTodas(false); // borra todas las lineas de todas las areas 

        UpdateData();
        bool bReclasificado = controlImagenes.Clasificar(&m_control, m_listClasificados, m_nCountOscuros, NULL, atof(m_dMinRef));
        if (bReclasificado)
        {
            // Rellenar leyenda
            CString csPorcentaje;
            int nOk = m_clasificados.DeleteAllItems();
            ASSERT(nOk);
            int nTotalPixels = controlImagenes.GetAnchoImagen()*controlImagenes.GetAltoImagen() - m_nCountOscuros;
            for (it=m_listClasificados.begin();it!=m_listClasificados.end();it++)
            {
                csPorcentaje.Format("%2.2lf",100* (double)(*it).m_nCount/(double)nTotalPixels);
                CString csNombre;
                if ((*it).m_nIndex != -1) //no incluimos en la lista normal los OSCUROS
                {
                    csNombre = theApp.m_minerales.GetNombre((*it).m_nIndex);
                    HTREEITEM hMineral = m_clasificados.InsertItem(_T(csPorcentaje + " " + csNombre), NULL);//, TVI_SORT);
                    (*it).m_hTree = hMineral; //anotamos el handle de la lista para poder pintar colores en el callback OnCustomdrawClasificados
                }
            }
            //    m_clasificados.SetCheck(hArea);

            // OSCUROS en treview a parte
            nOk = m_no_clasificados.DeleteAllItems();
            ASSERT(nOk);
            csPorcentaje.Format("%2.2lf",100* (double)m_nCountOscuros/(controlImagenes.GetAnchoImagen()*controlImagenes.GetAltoImagen()));
//            m_no_clasificados.SetTextColor(RGB(0, 255, 0));
//            m_no_clasificados.SetBkColor(RGB(0, 0, 255));
            HTREEITEM hMineral = m_no_clasificados.InsertItem(_T(csPorcentaje + " Demasiado oscuros"), NULL);//, TVI_SORT);
        }
    }

    // Clasificacion condicional
//    m_bInfDer = true;
//    m_bInfIzq = true;
//    m_bSupDer = true;
//    m_bSupIzq = true;

}

////////////////////////////////////////////////////////////////////////////////
// Clasificacion selectiva (condicional)
void CAnalisisDlg::OnConfiabilidad() 
{

    if (controlImagenes.GetBufClasificacionSelectiva() != NULL)
    {
        //"Seleccionamos" el boton 
        m_nUltimoFiltro = m_tabControl.GetCurSel(); //salvamos el ultimo filtro (RGB seria "-1") para el caso de que se cambie de campo
        if (m_nUltimoFiltro != -1)
        {
            // Deseleccionamos los tabs de filtros
            m_tabControl.DeselectAll(FALSE);
            m_nUltimoFiltro = filterOfTab[m_nUltimoFiltro]; //salvamos el ultimo filtro para el caso de que se cambie de campo
        }
        else
        {
            // Deseleccionamos RGB
            m_ButtonRGB.SetCheck(FALSE);
        }
        m_ButtonClasificar.EnableWindow(TRUE); // habilitado
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        m_ButtonClasificar.SetCheck(FALSE); // "no hundido"
        m_ButtonRGB.SetCheck(FALSE); // "no hundido"
        m_botonDistancia.SetCheck(FALSE); // "no hundido"

        controlImagenes.Confiabilidad(m_control);
    }
}

void CAnalisisDlg::OnDistancia() 
{
    if (controlImagenes.GetBufClasificacionSelectiva() != NULL)
    {
        //"Seleccionamos" el boton 
        m_nUltimoFiltro = m_tabControl.GetCurSel(); //salvamos el ultimo filtro (RGB seria "-1") para el caso de que se cambie de campo
        if (m_nUltimoFiltro != -1)
        {
            // Deseleccionamos los tabs de filtros
            m_tabControl.DeselectAll(FALSE);
            m_nUltimoFiltro = filterOfTab[m_nUltimoFiltro]; //salvamos el ultimo filtro para el caso de que se cambie de campo
        }
        else
        {
            // Deseleccionamos RGB
            m_ButtonRGB.SetCheck(FALSE);
        }
        m_ButtonClasificar.EnableWindow(TRUE); // habilitado
        m_ButtonRGB.EnableWindow(TRUE); // habilitado
        m_ButtonClasificar.SetCheck(FALSE); // "no hundido"
        m_ButtonRGB.SetCheck(FALSE); // "no hundido"
        m_botonConfiabilidad.SetCheck(FALSE); // "no hundido"

        controlImagenes.Distancia(m_control);	
    }
}

void CAnalisisDlg::OnSupIzq() 
{
    if (controlImagenes.GetBufClasificacionSelectiva() != NULL)
        ClasificacionSelectiva("");
}

void CAnalisisDlg::OnSupDer() 
{
    if (controlImagenes.GetBufClasificacionSelectiva() != NULL)
        ClasificacionSelectiva("");
}

void CAnalisisDlg::OnInfIzq() 
{
    if (controlImagenes.GetBufClasificacionSelectiva() != NULL)
        ClasificacionSelectiva("");
}

void CAnalisisDlg::OnInfDer() 
{
    if (controlImagenes.GetBufClasificacionSelectiva() != NULL)
        ClasificacionSelectiva("");
}

void CAnalisisDlg::ClasificacionSelectiva(CString csFicheroLog) 
{
    double dUmbralConfiabilidad     = 1.5; //solo por defecto, se lee de GetOpcionPruebas
    double dUmbralDistancia         = 50;  //solo por defecto, se lee de GetOpcionPruebas
    bool bInfDer,bInfIzq,bSupDer,bSupIzq; // opciones del dialogo
    int nInfDer = -1;
    int nInfIzq = -1;
    int nSupDer = -1;
    int nSupIzq = -1; // valores a asignar

    //obtenemos del dialogo las opciones seleccionadas para mostrar de umbralizacion 
    GetOpcionPruebas(dUmbralConfiabilidad,dUmbralDistancia, bInfDer,bInfIzq,bSupDer,bSupIzq);
    if (bInfDer)
        nInfDer = 4;
    if (bInfIzq)
        nInfIzq = 3;
    if (bSupDer)
        nSupDer = 2;
    if (bSupIzq)
        nSupIzq = 1;

    if (!csFicheroLog.IsEmpty())
    {
        // variables de informacion relativa a la umbralizacion por cada mineral
        unsigned int* arrInfDer;
        unsigned int* arrInfIzq;
        unsigned int* arrSupDer;
        unsigned int* arrSupIzq;

        int numMin = theApp.m_minerales.GetCount();

        arrInfDer = new unsigned int[numMin];
        arrInfIzq = new unsigned int[numMin];
        arrSupDer = new unsigned int[numMin];
        arrSupIzq = new unsigned int[numMin];

        memset(arrInfDer,0,numMin*sizeof(unsigned int));
        memset(arrInfIzq,0,numMin*sizeof(unsigned int));
        memset(arrSupDer,0,numMin*sizeof(unsigned int));
        memset(arrSupIzq,0,numMin*sizeof(unsigned int));

        theApp.clasificacion.ClasificacionSelectiva(dUmbralConfiabilidad,dUmbralDistancia,
                                                nInfDer,nInfIzq,nSupDer,nSupIzq,
                                                arrInfDer, arrInfIzq,arrSupDer,arrSupIzq,
                                                controlImagenes.GetBufClasificacionSelectiva());
        theApp.clasificacion.ClasificacionSelectivaLog(  dUmbralConfiabilidad,dUmbralDistancia, 
                                                    m_listClasificados,m_nCountOscuros,
                                                    arrInfDer, arrInfIzq,arrSupDer,arrSupIzq,
                                                    csFicheroLog);

        delete [] arrInfDer;
        delete [] arrInfIzq;
        delete [] arrSupDer;
        delete [] arrSupIzq;
    }
    else
    {
        theApp.clasificacion.ClasificacionSelectiva(dUmbralConfiabilidad,dUmbralDistancia, 
                                                nInfDer,nInfIzq,nSupDer,nSupIzq,
                                                controlImagenes.GetBufClasificacionSelectiva());
    }
    controlImagenes.ClasificacionSelectivaMostrar();
}

void CAnalisisDlg::GetOpcionPruebas(double& dUmbralConfiabilidad, double& dUmbralDistancia,bool& bInfDer,bool& bInfIzq,bool& bSupDer,bool& bSupIzq)
{

    UpdateData();

    if (m_csUmbralConfiabilidad != "")
        dUmbralConfiabilidad = atof(m_csUmbralConfiabilidad);
    else 
        m_csUmbralConfiabilidad.Format("%lf",dUmbralConfiabilidad);
    if (m_csUmbralDistancia != "")
        dUmbralDistancia = atof(m_csUmbralDistancia);
    else 
        m_csUmbralDistancia.Format("%lf",dUmbralDistancia);


    UpdateData(FALSE);

    bInfDer     = m_checkInfDer.GetCheck() == 1;
    bInfIzq     = m_checkInfIzq.GetCheck() == 1;
    bSupDer     = m_checkSupDer.GetCheck() == 1;
    bSupIzq     = m_checkSupIzq.GetCheck() == 1;
}


void CAnalisisDlg::OnChangeUmbralConfiabilidad() 
{
    if(controlImagenes.GetBufClasificacionSelectiva() != NULL)
        ClasificacionSelectiva("");
}

void CAnalisisDlg::OnChangeUmbralDistancia() 
{
    if(controlImagenes.GetBufClasificacionSelectiva() != NULL)
        ClasificacionSelectiva("");
}

void CAnalisisDlg::OnConfiguracinAsociacionesminerales() 
{
	int nResponse = m_asociacionesDlg.DoModal();
	if (nResponse == IDOK)
	{
        //Guardamos opciones marcadas
        theApp.m_asociaciones.GuardarFicheroAsociacionesSeleccionadas(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS);
        //guardamos tambien en el fichero por defecto (para usar como nueva plantilla)
        CopyFile(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, theApp.m_configuracion.m_csPathEjecutable+FICHERO_ASOCIACIONES_SELECCIONADAS,FALSE);

        // Para cada mineral, comprobamos que sea compatible con las asociaciones, si no, lo desactivamos (siempre y cuando no sea un mineral seleccionado)
        theApp.m_asociaciones.ActualizarMineralesActivosPorAsociaciones();	

        // m_ButtonClasificar.EnableWindow(TRUE); //permitimos clasificar de nuevo para poder actualizar la clasificacion con los cambios de seleccion //// Habilitarlo sin mas no sirve de nada porque para poder cambiar de RGB (por ejemplo) a clasificacion sin clasificar hay un mecanismo por el que si hay memoria reservada para el buffer de clasificacion, no se reclasifica. 
    }
	else if (nResponse == IDCANCEL)
	{
	}		
}

