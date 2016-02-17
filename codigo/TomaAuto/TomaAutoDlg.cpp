// TomaAutoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "TomaAuto.h"
#include "TomaAutoDlg.h"
#include "..\librerias\ControlDisp\control_camara.h"
#include "..\librerias\ControlMicroscopio\control_microscopio.h" //para mspWhereP en el thread de Posicion
#include "..\librerias\ControlMicroscopio\control_microscopio_leica.h" //para constantes CUBO_REFLECTOR_SMITH y CUBO_REFLECTOR_ESPEJO
#include "..\librerias\ControlMicroscopio\control_objetivos.h" //para actualizar_configuracion_objetivos
#include "..\librerias\LibBarrido\autoenfoque.h" //autoenfoque
#include "..\librerias\LibBarrido\calibracion.h" //get_objetivo_corregido

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif


extern CTomaAutoApp theApp;
extern HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)


/////////////////////////////////////////////////////////////////////////////
// CTomaAutoDlg dialog

CTomaAutoDlg::CTomaAutoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CTomaAutoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CTomaAutoDlg)
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
    m_formActivo            = NULL;
    m_threadPos             = NULL;
    m_paramThreadPos        = NULL;
}

void CTomaAutoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CTomaAutoDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CTomaAutoDlg, CDialog)
	ON_COMMAND(ID_ACCIONES_CAMBIAROBJETIVO_1, OnAccionesCambiarobjetivo1)
	ON_COMMAND(ID_ACCIONES_CAMBIAROBJETIVO_2, OnAccionesCambiarobjetivo2)
	ON_COMMAND(ID_ACCIONES_CAMBIAROBJETIVO_3, OnAccionesCambiarobjetivo3)
	ON_COMMAND(ID_ACCIONES_CAMBIAROBJETIVO_4, OnAccionesCambiarobjetivo4)
	ON_COMMAND(ID_ACCIONES_CAMBIAROBJETIVO_5, OnAccionesCambiarobjetivo5)
	ON_COMMAND(ID_ACCIONES_CAMBIAROBJETIVO_6, OnAccionesCambiarobjetivo6)
	//{{AFX_MSG_MAP(CTomaAutoDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_ACCIONES_CAMBIARCUBOS_1, OnAccionesCambiarcubos1)
	ON_COMMAND(ID_ACCIONES_CAMBIARCUBOS_2, OnAccionesCambiarcubos2)
	ON_COMMAND(ID_ACCIONES_CAMBIARCUBOS_3, OnAccionesCambiarcubos3)
	ON_COMMAND(ID_ACCIONES_CAMBIARCUBOS_4, OnAccionesCambiarcubos4)
	ON_COMMAND(ID_ACCIONES_AUTOENFOCAR, OnAccionesAutoenfocar)
	ON_WM_CLOSE()
	ON_WM_SIZE()
	ON_WM_GETMINMAXINFO()
	ON_COMMAND(ID_MODO_TOMAUNICA, OnModoTomaunica)
	ON_COMMAND(ID_MODO_TOMAAUTOMATICA, OnModoTomaautomatica)
	ON_COMMAND(ID_MODO_CALIBRACION_ALTA, OnModoCalibracionAlta)
	ON_COMMAND(ID_MODO_CALIBRACION_BAJA, OnModoCalibracionBaja)
	ON_COMMAND(ID_MODO_CORRECCION_GEOMETRICA, OnModoCorreccionGeometrica)
	ON_COMMAND(ID_MODO_COTA, OnModoCota)
	ON_COMMAND(ID_MODO_COMPROBAR, OnModoComprobar)
	ON_COMMAND(ID_OPCIONES_CONFIGURACION, OnOpcionesConfiguracion)
	ON_COMMAND(ID_MOVER, OnMover)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CTomaAutoDlg message handlers

BOOL CTomaAutoDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    //Inicializar el menú
    m_Menu.LoadMenu(IDR_MENU);

    ::SystemParametersInfo(SPI_SETDROPSHADOW,0,0,SPIF_SENDCHANGE);
    SetMenu(&m_Menu);
    CString csNombreElemento;
    CMenu* submenu = m_Menu.GetSubMenu(0); // Menu "Acciones"

    // Cambiar Objetivos
    submenu = submenu->GetSubMenu(0);         // menu "Cambiar Objetivo"
    for (int o=0;o<MAX_OBJETIVOS;o++)
    {
        csNombreElemento.Format("%d - ",o+1); //Primero el numero del objetivo
        csNombreElemento += mspGetNombreObjetivo(o+1); //luego la descripcion
        UINT id = submenu->GetMenuItemID(o);
        if (mspSetObjetivo(-1) == false || theApp.m_ParamIni.objetivos[o].dAumento == 0) //mspSetObjetivo(-1) es solo para saber si los objetivos se pueden cambiar
            submenu->ModifyMenu(o,MF_BYPOSITION | MF_STRING | MF_GRAYED,   id, csNombreElemento);
        else
            submenu->ModifyMenu(o,MF_BYPOSITION | MF_STRING,              id, csNombreElemento);
    }
    submenu->CheckMenuItem(mspGetObjetivo()-1,MF_CHECKED | MF_BYPOSITION);

    // Cambiar Cubos
    submenu = m_Menu.GetSubMenu(0); // Menu "Acciones"
    submenu = submenu->GetSubMenu(1);         // menu "Cambiar Cubos"
    for (o=0;o<MAX_CUBOS;o++)
    {
        CString csDescripcion = mspGetNombreCubo(o+1);
        csNombreElemento.Format("%d - ",o+1); //Primero el numero del objetivo
        csNombreElemento += csDescripcion; //luego la descripcion
        UINT id = submenu->GetMenuItemID(o);
        if (mspSetCubo(-1) == false || csDescripcion == "-"/*|| theApp.m_ParamIni.objetivos[o].dAumento == 0*/) //mspSetCubo(-1) es solo para saber si los objetivos se pueden cambiar
            submenu->ModifyMenu(o,MF_BYPOSITION | MF_STRING | MF_GRAYED,   id, csNombreElemento);
        else
            submenu->ModifyMenu(o,MF_BYPOSITION | MF_STRING,              id, csNombreElemento);
    }
    submenu->CheckMenuItem(mspGetCubo()-1,MF_CHECKED | MF_BYPOSITION);

    toma2_continua(M_imagen1);	// Adquisición en modo continuo

    // Creamos todos los "forms" que dependen del dialogo
    m_formPatron.SetPadre(this);
	m_formPatron.Create(IDD_FORM_PATRON,this);

    m_formCorreccionGeometrica.SetPadre(this);
	m_formCorreccionGeometrica.Create(IDD_FORM_GEOMETRICA,this);

    m_formCota.SetPadre(this);
	m_formCota.Create(IDD_FORM_COTA,this);

    m_formComprobar.SetPadre(this);
	m_formComprobar.Create(IDD_FORM_COMPROBAR,this);

    m_formTomaUnica.SetPadre(this);
	m_formTomaUnica.Create(IDD_FORM_TOMAUNICA,this);
//    m_formTomaUnica.ShowWindow(SW_SHOW);
    // Creamos los sub-dialgos/forms
    m_formTomaAuto.SetPadre(this);
	m_formTomaAuto.Create(IDD_FORM_TOMAAUTO,this);

    // Mostrar un form u otro dependiendo de si ya esta disponible la correccion
    if (get_objetivo_corregido() == 0)
        HayCorreccion(false);
    else
    {
        HayCorreccion(true);
        OnModoTomaautomatica();
    }

    //Para mostrar el filtro actual en pantalla
    theApp.Rueda.SetDialogo(this);

    //Lanzamos un thread para actualizar continuamente la posicion en pantalla   
    // y necesitamos un mutex para poder acceder a MspWhere a la vez que el thread principal
    g_hMutexMicroscopio = CreateMutex(  NULL,// no security attributes
                                        FALSE,                      // initially not owned
                                        "MutexMicroscopio");  // name of mutex
    if (g_hMutexMicroscopio == NULL)
    {
        AfxMessageBox("Error en la creacion del Mutex!");
        return FALSE;
    }

    // Crear eventos para finalizacion del thread
    m_StopThread = CreateEvent(0, TRUE, FALSE, 0);
    m_paramThreadPos = new PARAM_THREAD_POS; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    m_paramThreadPos->pThis = this; // para que tenga acceso a el objeto dialogo

	LanzarThreadPosicion();
    //m_threadPos = AfxBeginThread(ActualizarPosicion, (void *)m_paramThreadPos);
    //m_threadPos->m_bAutoDelete = false; // para que no se haga delete automaticamente del thread (y quede invalido su handle). Necesitamos el handle para esperar por él al finalizar el thread


	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CTomaAutoDlg::OnClose() 
{

    // FINALIZAR THREAD DE POSICION
    FinalizarThreadPosicion();

    // El thread ha terminado. LIMPIAR
    delete m_paramThreadPos;
    delete m_threadPos; 
    CloseHandle(m_StopThread);

    m_formTomaAuto.DestroyWindow();
    m_formTomaUnica.DestroyWindow();

    theApp.m_configuracion.Guardar();
    theApp.m_CorreccionGeometrica.Guardar(theApp.m_ParamIni);
    
	CDialog::OnClose();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.
void CTomaAutoDlg::OnPaint() 
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


HCURSOR CTomaAutoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CTomaAutoDlg::SetSize(int nAncho, int nAlto)
{
    SetWindowPos(NULL,0,0,nAncho,nAlto, SWP_NOMOVE | SWP_NOZORDER);
}

// Para evitar que la ventana sea mayor de lo necesario (tamaño suficiente para alojar la imagen en su verdadera magnitud)
void CTomaAutoDlg::OnGetMinMaxInfo(MINMAXINFO FAR* lpMMI) 
{

    if (m_formActivo != NULL)
    {
        int nAnchoResto = m_formActivo->GetAnchoResto();
        int nAltoResto  = m_formActivo->GetAltoResto();

        lpMMI->ptMaxTrackSize.x =  nAnchoResto + theApp.m_ParamIni.Cam.anchoImagen;
        lpMMI->ptMaxTrackSize.y =  nAltoResto  + theApp.m_ParamIni.Cam.altoImagen;
        lpMMI->ptMinTrackSize.x =  nAnchoResto + (long)floor((theApp.m_ParamIni.Cam.anchoImagen * 0.5) + 0.5);
        lpMMI->ptMinTrackSize.y =  nAltoResto  + (long)floor((theApp.m_ParamIni.Cam.altoImagen * 0.5) + 0.5);
	    
    }
	CDialog::OnGetMinMaxInfo(lpMMI);
}


//////////////////////////////////////////////////////////////////////////////
// THREAD para actualizar continuamente la posicion en pantalla   
//////////////////////////////////////////////////////////////////////////////
UINT CTomaAutoDlg::ActualizarPosicion( LPVOID Param )
{
	static char mensaje[1024];
    PARAM_THREAD_POS* paramThread = (PARAM_THREAD_POS*)Param;
    HANDLE hMutex; 
    DWORD dwWaitResult; 
    point pActual;

    hMutex = OpenMutex( 
        MUTEX_ALL_ACCESS,      // request full access
        FALSE,                 // handle not inheritable
        "MutexMicroscopio");  // object name
    if (hMutex == NULL) 
    {
        AfxMessageBox("OpenMutex error"); 
        return -1;
    }

	sprintf(mensaje, "Iniciando thread de actualizar posición");
    sendlog("\nActualizarPosicion",mensaje);

    while (true) //Hasta que falle porque se ha libreado parmThread
    {
        // Check event for stop thread
        if(WaitForSingleObject(paramThread->pThis->m_StopThread, 0) == WAIT_OBJECT_0)
        {
	        sprintf(mensaje, "Fin thread de actualizar posición. Error en WaitForSingleObject\n");
            sendlog("ActualizarPosicion",mensaje);
            return 0;
        }

        //Actualizar posicion
        // Request ownership of mutex.
        dwWaitResult = WaitForSingleObject(hMutex,500L);   // half-second time-out interval
        if (dwWaitResult == WAIT_OBJECT_0)
        {
	        pActual = mspWhereP();

            // Release ownership of the mutex object.
            if (! ReleaseMutex(hMutex))
            {
                ASSERT(AfxMessageBox("ReleaseMutex error al ActualizarPosicion") );
	            sprintf(mensaje, "Fin thread de actualizar posición. Error en ReleaseMutex\n");
                sendlog("ActualizarPosicion",mensaje);
                return -1;
            }

            // Mostrar posicion manda mensajes a la ventana principal (thread principal) para que se actualizen los valores en pantalla.
            paramThread->pThis->MostrarPosicion(pActual);
        }
        else
            sendlog("ActualizarPosicion","Fallo al esperar por el MutexMicroscopio. TIMEOUT");

        Sleep(INTERVALO_ACTUALIZACION_POSICION); //para no saturar el procesador
    }

	sprintf(mensaje, "Fin thread de actualizar posición. Esto no puede ser!!!!!!!!!\n");
    sendlog("ActualizarPosicion",mensaje);
    return 0;
}

// LLamado desde el thread, redirige la llamada al form adecuado
void CTomaAutoDlg::MostrarPosicion(point p)
{
    m_formActivo->MostrarPosicion(p.coord[X_],p.coord[Y_],p.coord[Z_]);
}

// LLamado desde control_dta_revolver al cambiar un filtro
void CTomaAutoDlg::MostrarFiltro(int nFiltro)
{
    m_formActivo->MostrarFiltro(nFiltro);
}

// Acciones necesarias para lanzar el thread de posicion
void CTomaAutoDlg::LanzarThreadPosicion()
{
    //Lanzamos el thread de Actualizacion de Posicion
    ::ResetEvent(m_StopThread);
    m_threadPos = AfxBeginThread(ActualizarPosicion, (void *)m_paramThreadPos);
    m_threadPos->m_bAutoDelete = false; // para que no se haga delete automaticamente del thread (y quede invalido su handle). Necesitamos el handle para esperar por él al finalizar el thread
}

// Acciones necesarias para finalizar ordenadamente el thread de posicion
void CTomaAutoDlg::FinalizarThreadPosicion()
{   
	static char mensaje[1024];
	sprintf(mensaje, "Finalizando thread de actualizar posición...");
    sendlog("FinalizarThreadPosicion",mensaje);

    SetEvent(m_StopThread);
    // Esperar a que efectivamente este finalizado
    DWORD dExit = STILL_ACTIVE;
    while (GetExitCodeThread(m_threadPos->m_hThread, &dExit)!=0 && dExit == STILL_ACTIVE) //esperamos activamente a que el thread finalize
    {
        // Como el thread envia mensajes a la ventana principal, es decir al thread principal (este), es necesario procesar estos mensajes, 
        // si no el thread estaria bloquedado mandando mensajes y este thread principal bloqueado esperando a que termine
        MSG message;
        if (PeekMessage(&message, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&message);
            DispatchMessage(&message);
        }
    }
	sprintf(mensaje, "Finalizado thread de actualizar posición\n");
    sendlog("FinalizarThreadPosicion",mensaje);
}

// Si falla la preparacion de la correccion (por ejemplo porque no hay patrones), hay que evitar que se pueda barrer y capturar
void CTomaAutoDlg::HayCorreccion(bool bHayCorreccion, bool bCambiarBarrido)
{
    long lState;

    if (!bHayCorreccion)
    {
        OnModoCalibracionAlta();
        lState = MF_GRAYED; 
    }
    else
    {
        if (bCambiarBarrido)
            OnModoTomaautomatica();
        lState = MF_ENABLED; 
    }

    // Habilitamos cambio de objetivo (mientras se corrige no se puede cambiar objetivo porque el cambio de objetivo implica una correccion posterior)
    CMenu* submenu = m_Menu.GetSubMenu(0); // Menu "Acciones"
    submenu->EnableMenuItem(0,MF_BYPOSITION | lState); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MENU
///////////////////////////////////////////////////////////////////////////////////////////////////

// OBJETIVOS

void CTomaAutoDlg::OnAccionesCambiarobjetivo1() 
{
    CambiarObjetivo(1);	
}

void CTomaAutoDlg::OnAccionesCambiarobjetivo2() 
{
    CambiarObjetivo(2);	
}

void CTomaAutoDlg::OnAccionesCambiarobjetivo3() 
{
    CambiarObjetivo(3);	
}

void CTomaAutoDlg::OnAccionesCambiarobjetivo4() 
{
    CambiarObjetivo(4);	
}

void CTomaAutoDlg::OnAccionesCambiarobjetivo5() 
{
    CambiarObjetivo(5);	
}

void CTomaAutoDlg::OnAccionesCambiarobjetivo6() 
{
    CambiarObjetivo(6);	
}

void CTomaAutoDlg::CambiarObjetivo(int nObjetivo)
{
    CMenu* submenu = m_Menu.GetSubMenu(0); // Menu "Acciones"

    //Desmarcamos el objetivo actual
    submenu = submenu->GetSubMenu(0);         // menu "Cambiar Objetivo"
    submenu->CheckMenuItem(mspGetObjetivo()-1,MF_UNCHECKED | MF_BYPOSITION);

    theApp.m_CorreccionGeometrica.Guardar(theApp.m_ParamIni); //guardamos la correccion geometrica de este objetivo (ha podido cambiar la escala objetivo)

    //Cambiar objetivo
    if (!mspSetObjetivo(nObjetivo))
    {
        sendlog("CambiarObjetivo","No se ha podido cambiar de objetivo al seleccionar del menú");
        return;
    }

    //actualizar parametros revolver y modificar directorio de patrones
    bool bHayCambioObjetivo = false;
    bool bOk = actualizar_configuracion_objetivo(&theApp.m_ParamIni, bHayCambioObjetivo);

    //Actualizamos exposicion y base de exposicion
    int nFiltroActual = theApp.Rueda.GetFilter();
	ModificaExposicionSleep(theApp.m_ParamIni.Rueda.exposicion[nFiltroActual]);//modificar la exposicion correspondiente al filtro seleccionado
    ModificaBaseExposicion(theApp.m_ParamIni.Rueda.base_exp[nFiltroActual]);// Modificar la base de exposicion para este nuevo filtro

    if (bOk && bHayCambioObjetivo)
        //Marcamos el objetivo 
        submenu->CheckMenuItem(nObjetivo-1,MF_CHECKED | MF_BYPOSITION);

    //La escala objetivo mostrada en el dialogo ya no es correcta, hay que cambiarla con la configurada en la correccion geometrica
    //Actualizamos la configuracion geometrica para modificar la escala objetivo correspondiente al objetivo cambiado
    theApp.ActualizarCorreccionGeometrica(); // se lee la configuracion geometrica para este objetivo 
    m_formTomaAuto.ActualizarEscala(); //en dialogo usando configuraciongeometrica.m_dEscalaObjetivo

    //No se actualiza la preparacion de la correccion (porque es lento y es posible que se cambie de objetivo de nuevo, esto se hará antes de barrer)
    // OJO: si se actualizase, hay que:
        // Deshabilitamos cambio de objetivo (no se puede cambiar objetivo porque el cambio de objetivo implica una correccion posterior)
        //submenu->EnableMenuItem(0,MF_BYPOSITION | MF_ENABLED); 

}

// CUBOS
void CTomaAutoDlg::OnAccionesCambiarcubos1() 
{
    //CambiarCubo	(1);
}

void CTomaAutoDlg::OnAccionesCambiarcubos2() 
{
    CambiarCubo	(2);
}

void CTomaAutoDlg::OnAccionesCambiarcubos3() 
{
    CambiarCubo	(3);
}

void CTomaAutoDlg::OnAccionesCambiarcubos4() 
{
    //CambiarCubo	(4);
}

void CTomaAutoDlg::CambiarCubo(int nCubo)
{
    int nCuboLeica; //los cubos en leica tienen una numeracion diferente (no razonable)

    //Desmarcamos el cubo actual (desmarcando todos)
    CMenu* submenu = m_Menu.GetSubMenu(0); // Menu "Acciones"
    submenu = submenu->GetSubMenu(1);         // menu "Cambiar Cubo"
    for (unsigned int i=0; i < submenu->GetMenuItemCount();i++)
        submenu->CheckMenuItem(i,MF_UNCHECKED | MF_BYPOSITION);

    if (nCubo == 2)
        nCuboLeica = CUBO_REFLECTOR_SMITH;
    else if (nCubo == 3)
        nCuboLeica = CUBO_REFLECTOR_ESPEJO;
    else
        return;

    //Cambiar Cubo
    if (!mspSetCubo(nCubo))
    {
        sendlog("CambiarCubo","No se ha podido cambiar de Cubo al seleccionar del menú");
        return;
    }

    //Marcamos el Cubo 
    submenu->CheckMenuItem(nCubo-1,MF_CHECKED | MF_BYPOSITION);
}

// Estructura para pasar a los thread de autoenfoque y Toma
typedef struct PARAM_THREAD_AUTOENFOQUE
{
    CTomaAutoDlg*   pDialogo;
} PARAM_THREAD_AUTOENFOQUE;

void CTomaAutoDlg::OnAccionesAutoenfocar() 
{
    Autoenfocar();
}

void CTomaAutoDlg::Autoenfocar() 
{
    //Finaliza el thread de posicion 
    FinalizarThreadPosicion();

    PARAM_THREAD_AUTOENFOQUE* paramThread = new PARAM_THREAD_AUTOENFOQUE; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pDialogo = this;

    AfxBeginThread(AutoenfocarThread, (void *)paramThread);

}

UINT CTomaAutoDlg::AutoenfocarThread( LPVOID Param )
{
    PARAM_THREAD_AUTOENFOQUE* paramThread = (PARAM_THREAD_AUTOENFOQUE*)Param;

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult != WAIT_OBJECT_0)
    {
        AfxMessageBox("No es posible autoenfocar. Microscopio en uso");
        return -1;
    }

    paramThread->pDialogo->Autoenfoque();

    // Release ownership of the mutex object.
    if (! ReleaseMutex(g_hMutexMicroscopio))
        ASSERT(AfxMessageBox("ReleaseMutex error al AutoenfocarThread") );

    //Lanzamos de nuevo el thread de Actualizacion de Posicion
    paramThread->pDialogo->LanzarThreadPosicion();

    toma2_continua(M_imagen1);	// Adquisición en modo continuo

    delete paramThread;

    return 0;
}

void CTomaAutoDlg::Autoenfoque()
{
    double contraste;

    // actualizo la z absoluta del filtro predefinido antes de cambiar de filtro (la z ha podido ser cambiada por el usuario)
    theApp.Rueda.zObservacion = mspWhere(Z_); 

    // Se ha de enfocar (tanto manual como automaticamente)
    // con el filtro "de enfoque" (o "de referencia", es decir el monocromatico)
    theApp.Rueda.ChangeFilter(theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]);

    double z = mspWhere(Z_); //Z por defecto

	if (autoenfoque(z,z,&z,&contraste, theApp.m_ParamIni.objetivos[mspGetObjetivo()-1].dAumento, true, m_formActivo->GetControlImagen()) == -1)
    {
        // ERROR
        AfxMessageBox("Barrido - No es posible realizar autoenfoque");
    }
    else
    {
	    mspGoCheck(Z_, z + BACKSLASHZ);
		mspGo(Z_,z);

        // actualizo la z absoluta del filtro predefinido (calculandola a partir del filtro de enfoque)
        theApp.Rueda.zObservacion = z + theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.Rueda.posFiltro] - 
                                     theApp.m_ParamIni.Rueda.diferencia_z[theApp.m_ParamIni.filtro[theApp.m_ParamIni.Rueda.posEnfoque]]; 
    }

    // Una vez enfocado, regresamos al filtro predefinido (o de usuario)
    // Se cambiará correspondientemente la Z.
    theApp.Rueda.ChangeFilter(theApp.m_ParamIni.Rueda.posFiltro);

    //MdispControl(M_display_normal, M_OVERLAY_CLEAR, TRANSPARENTE); //MIL 8
}

void CTomaAutoDlg::OnSize(UINT nType, int cx, int cy) 
{
	CDialog::OnSize(nType, cx, cy);
	
    if (IsWindow(*m_formActivo))
        m_formActivo->SetWindowPos(NULL,0,0,cx,cy, SWP_NOMOVE | SWP_NOZORDER);
}


void CTomaAutoDlg::OnModoTomaunica() 
{
    m_formActivo->ShowWindow(SW_HIDE);
    m_formTomaUnica.ShowWindow(SW_SHOW);
    m_formActivo = &m_formTomaUnica;
    SetWindowText("Toma manual");
}

void CTomaAutoDlg::OnModoTomaautomatica() 
{
    if (m_formActivo!=NULL)
        m_formActivo->ShowWindow(SW_HIDE);
    if(m_formTomaAuto.m_hWnd != NULL)
    {
        m_formTomaAuto.ShowWindow(SW_SHOW);
        m_formActivo = &m_formTomaAuto;
        SetWindowText("Toma automática");
    }
}

void CTomaAutoDlg::OnModoCalibracionAlta() 
{
    if (m_formActivo!=NULL)
        m_formActivo->ShowWindow(SW_HIDE);
    if(m_formPatron.m_hWnd != NULL)
    {
        m_formPatron.ShowWindow(SW_SHOW);
        m_formActivo = &m_formPatron;
        m_formPatron.SetTipoPatron(true); //patron claro
        SetWindowText("Calibración patrón alta reflectancia");
    }
}

void CTomaAutoDlg::OnModoCalibracionBaja() 
{
    m_formActivo->ShowWindow(SW_HIDE);
    m_formPatron.ShowWindow(SW_SHOW);
    m_formActivo = &m_formPatron;
    m_formPatron.SetTipoPatron(false); //patron oscuro
    SetWindowText("Calibración patrón baja reflectancia");
}

void CTomaAutoDlg::OnModoCorreccionGeometrica() 
{
    m_formActivo->ShowWindow(SW_HIDE);
    m_formCorreccionGeometrica.ShowWindow(SW_SHOW);
    m_formActivo = &m_formCorreccionGeometrica;
    SetWindowText("Corrección geométrica");
}

void CTomaAutoDlg::OnModoCota() 
{
    m_formActivo->ShowWindow(SW_HIDE);
    m_formCota.ShowWindow(SW_SHOW);
    m_formActivo = &m_formCota;
    SetWindowText("Ajuste automático cota");
}

void CTomaAutoDlg::OnModoComprobar() 
{
    m_formActivo->ShowWindow(SW_HIDE);
    m_formComprobar.ShowWindow(SW_SHOW);
    m_formActivo = &m_formComprobar;
    SetWindowText("Comprobar calibración");
}

void CTomaAutoDlg::OnOpcionesConfiguracion() 
{
	int nResponse = m_OpcionesConfiguracionDlg.DoModal();
}

void CTomaAutoDlg::OnMover() 
{
	Mover();
	
}
/*
// Estructura para pasar a los thread de mover XY
typedef struct PARAM_THREAD_MOVER
{
    CTomaAutoDlg*   pDialogo;
} PARAM_THREAD_MOVER;
*/

void CTomaAutoDlg::Mover() 
{
	int nResponse = m_MoverDlg.DoModal();
    if(nResponse != IDOK)
        return;
    
    //Finaliza el thread de posicion 
    FinalizarThreadPosicion();

//    PARAM_THREAD_MOVER* paramThread = new PARAM_THREAD_MOVER; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
 //   paramThread->pDialogo = this;

//    AfxBeginThread(MoverThread, (void *)paramThread);

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult != WAIT_OBJECT_0)
    {
        AfxMessageBox("No es posible mover. Microscopio en uso");
        return;
    }

    mspGoXY(m_MoverDlg.m_dX, m_MoverDlg.m_dY);

    // Release ownership of the mutex object.
    if (! ReleaseMutex(g_hMutexMicroscopio))
        ASSERT(AfxMessageBox("ReleaseMutex error al AutoenfocarThread") );

    //Lanzamos de nuevo el thread de Actualizacion de Posicion
    //paramThread->pDialogo->LanzarThreadPosicion();
   LanzarThreadPosicion();

}
/*
UINT CTomaAutoDlg::MoverThread( LPVOID Param )
{
    PARAM_THREAD_MOVER* paramThread = (PARAM_THREAD_MOVER*)Param;

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult != WAIT_OBJECT_0)
    {
        AfxMessageBox("No es posible mover. Microscopio en uso");
        return -1;
    }

    paramThread->pDialogo->MueveXY();

    // Release ownership of the mutex object.
    if (! ReleaseMutex(g_hMutexMicroscopio))
        ASSERT(AfxMessageBox("ReleaseMutex error al AutoenfocarThread") );

    //Lanzamos de nuevo el thread de Actualizacion de Posicion
    paramThread->pDialogo->LanzarThreadPosicion();

    toma2_continua(M_imagen1);	// Adquisición en modo continuo

    delete paramThread;

    return 0;
}

*/