// ClasificaBarridoDlg.cpp : implementation file
//

#include "stdafx.h"
#include "ClasificaBarrido.h"
#include "ClasificaBarridoDlg.h"
#include "AsociacionesMinerales.h" // para Liberar las asociaciones
#include "folder_dialog.h"
#include "filefinder.h" // para buscar imagenes en directorio CFileFinder

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CClasificaBarridoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoDlg dialog

CClasificaBarridoDlg::CClasificaBarridoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CClasificaBarridoDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CClasificaBarridoDlg)
	m_csMuestra = _T("");
	m_csPorcentajeAceptados = _T("");
	m_csCampoActual = _T("");
	m_csCamposTotal = _T("");
	m_csCamposX = _T("");
	m_csCamposY = _T("");
	//}}AFX_DATA_INIT
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClasificaBarridoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CClasificaBarridoDlg)
	DDX_Control(pDX, IDC_SELECCIONAR_DIRECTORIO, m_buttonExaminar);
	DDX_Control(pDX, IDC_PROGRESO_NO_ACUM, m_progreso_no_acum);
	DDX_Control(pDX, IDC_PROGRESO_NO, m_progreso_no);
	DDX_Control(pDX, IDC_PROGRESO_ACUM, m_progreso_acum);
	DDX_Control(pDX, IDC_STATIC_x, m_static_x);
	DDX_Control(pDX, IDC_SEGUNDA_PASADA, m_buttonSegundaPasada);
	DDX_Control(pDX, IDC_PROGRESO, m_progreso);
	DDX_Control(pDX, IDC_DIRECORIO, m_Directorio);
	DDX_Text(pDX, IDC_MUESTRA, m_csMuestra);
	DDX_Text(pDX, IDC_PORCENTAJE_ACEPTADOS, m_csPorcentajeAceptados);
	DDX_Text(pDX, IDC_STATIC_CAMPO, m_csCampoActual);
	DDX_Text(pDX, IDC_TOTAL, m_csCamposTotal);
	DDX_Text(pDX, IDC_CAMPOS_X, m_csCamposX);
	DDX_Text(pDX, IDC_CAMPOS_Y, m_csCamposY);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CClasificaBarridoDlg, CDialog)
	//{{AFX_MSG_MAP(CClasificaBarridoDlg)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_MESSAGE(WM_UPDATEFIELDS, UpdateDisplay) //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
	ON_BN_CLICKED(IDC_SELECCIONAR_DIRECTORIO, OnSeleccionarDirectorio)
	ON_EN_KILLFOCUS(IDC_DIRECORIO, OnKillfocusDirecorio)
	ON_BN_CLICKED(IDC_SEGUNDA_PASADA, OnSegundaPasada)
	ON_WM_CLOSE()
	ON_COMMAND(ID_CONFIGURACION_OPCIONES, OnConfiguracionOpciones)
	ON_COMMAND(ID_CONFIGURACION_ASOCIACIONESMINERALES, OnConfiguracionAsociacionesminerales)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CClasificaBarridoDlg message handlers

BOOL CClasificaBarridoDlg::OnInitDialog()
{
    bool bHayInfoBarrido = false;

	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

    //Inicializar el menú
    m_Menu.LoadMenu(IDR_MENU);
    SetMenu(&m_Menu);

    if (!theApp.m_configuracion.m_csDefaultPath.IsEmpty())
    {
        m_Directorio.SetWindowText(theApp.m_configuracion.m_csDefaultPath);
        //bHayInfoBarrido = MostrarInfoBarrido();
        TratarDirectorio(); //busca ficheros de configuracion necesarios
    }
    UpdateData(FALSE);

    // Cambiamos el font a courier para que se alinen correctamente las columnas
    CFont f1;
    LOGFONT lf1;
    ::ZeroMemory(&lf1, sizeof(lf1));
//    lf1.lfHeight = 12;
//    lf1.lfCharSet = ANSI_CHARSET;
//    lf1.lfWeight = FW_NORMAL;
    ::lstrcpy(lf1.lfFaceName, "Courier");
    f1.CreateFontIndirect(&lf1);
    m_progreso.SetFont(&f1);
    m_progreso_acum.SetFont(&f1);
    m_progreso_no.SetFont(&f1);
    m_progreso_no_acum.SetFont(&f1);
    f1.Detach();

    bool bInitOk = m_asociacionesDlg.Init(&theApp.m_asociaciones, &theApp.m_minerales); 
    /* ya se hace en MostrarInfoBarrido
    if(bHayInfoBarrido && bInitOk) //hay alguna asociacion seleccionada
        // habilitamos segunda pasada 
        m_buttonSegundaPasada.EnableWindow(TRUE);
    */

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CClasificaBarridoDlg::OnClose() 
{
    // Hay que liberar la memoria de asociaciones minerales (no se puede hacer en su destructor porque tenemos que asegurarnos de que se haga antes de liberar Minerales)
    theApp.m_asociaciones.Liberar();
    m_asociacionesDlg.Liberar();
    
	CDialog::OnClose();
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CClasificaBarridoDlg::OnPaint() 
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

HCURSOR CClasificaBarridoDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

// Para evitar que el dialogo se cierre al presionar ENTER o ESC
BOOL CClasificaBarridoDlg::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// El usuario selecciona un directorio
void CClasificaBarridoDlg::OnSeleccionarDirectorio() 
{
	CFolderDialog dlg(&theApp.m_configuracion.m_csDefaultPath);
	if (dlg.DoModal() == IDOK)
	{
		// pathSelected contain the selected folder.
		//theApp.csPathToma = pathSelected;
        // Nos aseguramos de que el ultimo caracter sea '\'
        CString csUltimoCaracter = theApp.m_configuracion.m_csDefaultPath.Right(1);
        if (csUltimoCaracter != "\\" && csUltimoCaracter != "/")
            theApp.m_configuracion.m_csDefaultPath += _T("\\");
        m_Directorio.SetWindowText(theApp.m_configuracion.m_csDefaultPath);
        TratarDirectorio(); //busca ficheros de configuracion necesarios
    }
}

// Tratamos en directorio theApp.m_configuracion.m_csDefaultPath para encontrar ficheros necesarios de barrido
void CClasificaBarridoDlg::TratarDirectorio()
{
    bool bMostrarOk = MostrarInfoBarrido();

    if (bMostrarOk)
    {
        // Habilitar menu asociaciones minerales
        m_Menu.EnableMenuItem(0,MF_BYPOSITION | MF_ENABLED); 
        DrawMenuBar(); 

        m_buttonSegundaPasada.EnableWindow(TRUE); //permitimos Clasificar
     
        // Cargar opciones asociaciones
	    if (fopen(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, "rt") == NULL)
            // No hay fichero asociaciones especifico. copiar y usar el general
            CopyFile(theApp.m_configuracion.m_csPathEjecutable+FICHERO_ASOCIACIONES_SELECCIONADAS,theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, FALSE);
        else
            // Hay fichero especifico, sobreescribir general para que siempre sea igual al especifico actual
            CopyFile(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, theApp.m_configuracion.m_csPathEjecutable+FICHERO_ASOCIACIONES_SELECCIONADAS,FALSE);
        theApp.m_asociaciones.CargarFicheroAsociacionesSeleccionadas(theApp.m_configuracion.m_csDefaultPath + FICHERO_ASOCIACIONES_SELECCIONADAS);
    }
    else
    {
        // Deshabilitar menu asociaciones minerales
        m_Menu.EnableMenuItem(0,MF_BYPOSITION | MF_GRAYED); 
        DrawMenuBar(); 

        m_buttonSegundaPasada.EnableWindow(FALSE); //sin informacion de barrido no permitimos Clasificar
    }
}

// El usuario ha terminado de escribir un nuevo directorio (en vez de OnChageDirectorio para que no se haga cada vez que se escriba un caracter, ni al principio)
void CClasificaBarridoDlg::OnKillfocusDirecorio() 
{
    LPTSTR temp = theApp.m_configuracion.m_csDefaultPath.GetBuffer( 1024 );
	m_Directorio.GetLine(0,temp);	
    theApp.m_configuracion.m_csDefaultPath.ReleaseBuffer();  // Surplus memory released, temp is now invalid.

    // Nos aseguramos de que el ultimo caracter sea '\'
    CString csUltimoCaracter = theApp.m_configuracion.m_csDefaultPath.Right(1);
    if (csUltimoCaracter != "\\" && csUltimoCaracter != "/")
        theApp.m_configuracion.m_csDefaultPath += _T("\\");
    m_Directorio.SetWindowText(theApp.m_configuracion.m_csDefaultPath);
    TratarDirectorio(); //busca ficheros de configuracion necesarios
    /* YA SE HACE EN MonstrarInfoBarrido
    //habilitamos filtrado de asociaciones
    if (bMostrarOk && theApp.m_configuracion.m_dConfiabilidadPrimera > 0 && theApp.m_configuracion.m_dDistanciaPrimera > 0)
        m_asociacionesDlg.HabilitaFiltrar(TRUE, m_csMuestra);
    else
        m_asociacionesDlg.HabilitaFiltrar(FALSE);
        */
}

// Leemos el fichero de informacion de barrido del directorio elegido y mostramos la informacion de barrido
// Devuelve true si se ha encontrado el fichero o si estamos en toma manual
bool CClasificaBarridoDlg::MostrarInfoBarrido()
{
    //Primero, debemos buscar el fichero de informacion (no sabemos como se llama porque no sabemos cual
    CFileFinder	_finder;
    CString csExtInfoBarrido = "_INFO_BARRIDO.txt";

	CFileFinder::CFindOpts	opts;
	opts.sBaseFolder = theApp.m_configuracion.m_csDefaultPath;
	opts.sFileMask.Format("*" + csExtInfoBarrido);
	opts.bSubfolders = false;
	opts.FindNormalFiles();
	_finder.RemoveAll();
	_finder.Find(opts); //Busqueda

    CString csFichero;
    CString csError;
    if (_finder.GetFileCount() <1)
    {
        // NO HAY FICHERO INFO_BARRIDO, ESTAMOS ANTE UNA TOMA MANUAL
        //Buscamos fichero tif
        CString csExt = ".tif";
	    opts.sFileMask.Format("*" + csExt);
	    _finder.RemoveAll();
	    _finder.Find(opts); //Busqueda
        if (_finder.GetFileCount() <1)
        {
            csError.Format("No se ha encontrado fichero de informacion de barrido ni ninguna imagen con extension %s", csExt);
            AfxMessageBox(csError);
            theApp.m_nCampos_x = - 1;
            theApp.m_nCampos_y = - 1;
            m_csMuestra.Empty();
            m_csCamposX.Empty();
            m_csCamposY.Empty();
            m_csCamposTotal.Empty();
            return false;
        }
        csFichero = _finder.GetFilePath(0); // Cualquiera de las imagenes
        csFichero = csFichero.Left(csFichero.GetLength() - 7 - csExt.GetLength()); //quitamos numero de campo, banda y extension

        // buscamos ultimo filtro (para saber numero de filtros)
        int nFiltro = 1; //incluimos pancromatico
        CString csImagen;
        while (true)
        {
            csImagen.Format("%s_001_%02d%s",csFichero,nFiltro,csExt);
            if (GetFileAttributes(csImagen) == 0xFFFFFFFF)
                // no existe
                break;
            theApp.m_arrFiltros[nFiltro-1] = nFiltro;
            nFiltro++;
        }
        if (nFiltro == 2)
        {
            csError.Format("No se ha encontrado la imagen %s", csImagen);
            AfxMessageBox(csError);
            theApp.m_nCampos_x = - 1;
            theApp.m_nCampos_y = - 1;
            m_csMuestra.Empty();
            m_csCamposX.Empty();
            m_csCamposY.Empty();
            m_csCamposTotal.Empty();
            return false;
        }
        theApp.m_nBandas = nFiltro - 1;

        // buscamos ultimo campo (para saber numero de campos)
        int nCampo = 1;
        while (true)
        {
            csImagen.Format("%s_%03d_02%s",csFichero,nCampo,csExt);
            if (GetFileAttributes(csImagen) == 0xFFFFFFFF)
                // no existe
                break;
            nCampo++;
        }
        if (nCampo == 1)
        {
            csError.Format("No se ha encontrado la imagen %s", csImagen);
            AfxMessageBox(csError);
            theApp.m_nCampos_x = - 1;
            theApp.m_nCampos_y = - 1;
            m_csMuestra.Empty();
            m_csCamposX.Empty();
            m_csCamposY.Empty();
            m_csCamposTotal.Empty();
            return false;
        }

        theApp.m_nCampos_x = nCampo - 1;
        theApp.m_nCampos_y = 1;
        theApp.m_bBarrido2d = false;

        m_csMuestra = csFichero.Right(csFichero.GetLength() - theApp.m_configuracion.m_csDefaultPath.GetLength());

        //Mostramos info barrido en dialogo
        m_csCamposY.Empty();
        m_static_x.ShowWindow(SW_HIDE);
    }
    else
    {
        csFichero = _finder.GetFilePath(0);
        if (_finder.GetFileCount() > 1)
        {
            csError.Format("ATENCION: se han encontrado varios archivos de información de barrido, se usará %s", csFichero);
            AfxMessageBox(csError);
        }

        //Lee el fichero
        FILE* f_info_barrido = fopen(csFichero, "rt");

        char strDummy[500]; //para saltarse la cabecera
    //    fscanf(f_info_barrido,"%[^\n]",strDummy); //cabecera
        fgets(strDummy,500,f_info_barrido);
        fscanf(f_info_barrido,"%dx%d",&theApp.m_nCampos_x,&theApp.m_nCampos_y);
        fscanf(f_info_barrido,"%d",&theApp.m_nBandas);
        for (int b=0;b<theApp.m_nBandas;b++)
            fscanf(f_info_barrido,"%d\t",&theApp.m_arrFiltros[b]); //se leen los filtros (1..N) correspondientes a cada banda

        fclose(f_info_barrido);

        theApp.m_bBarrido2d = true;

        m_csMuestra = csFichero.Mid(theApp.m_configuracion.m_csDefaultPath.GetLength(), csFichero.GetLength() - theApp.m_configuracion.m_csDefaultPath.GetLength() - csExtInfoBarrido.GetLength());

        //Mostramos info barrido en dialogo
        m_static_x.ShowWindow(SW_SHOW);
        m_csCamposY.Format("%d",theApp.m_nCampos_y);
   }
    
    //Mostramos info barrido en dialogo
    m_csCamposX.Format("%d",theApp.m_nCampos_x);
    m_csCamposTotal.Format("%d",theApp.m_nCampos_x*theApp.m_nCampos_y);
    UpdateData(FALSE);

    //habilitamos filtrado de asociaciones si los umbrales son validos y hay barrido definido
    if (theApp.m_configuracion.m_dDistanciaPrimera > 0 && theApp.m_configuracion.m_dConfiabilidadPrimera > 0 //HACER, ya que no depende de info barrido, poner esto al cambiar configuracion?
        && !m_csCamposTotal.IsEmpty()) //HACER: innecesaro esta ultimo porque si no hay info barrido, no se podra ni siquiera abrir el dialogo de asosciaciones
        m_asociacionesDlg.HabilitaFiltrar(TRUE, m_csMuestra);
    else
        m_asociacionesDlg.HabilitaFiltrar(FALSE);

    return true;   
}

//Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)
LRESULT CClasificaBarridoDlg::UpdateDisplay(WPARAM wParam, LPARAM lParam)
{
    UpdateData((BOOL)wParam);
    return 0;
}

/*
void CClasificaBarridoDlg::OnPrimeraPasada() 
{
    //Recuperamos umbrales de dialogo
    UpdateData();

    ASSERT(m_progreso.DeleteAllItems());
    m_csCampoActual.Format("%d",0);
    UpdateData(FALSE);

    double dUmbralConfiabilidad;
    double dUmbralDistancia;
    if (theApp.m_configuracion.m_csUmbralConfiabilidad == "" || theApp.m_configuracion.m_csUmbralDistancia == "")
    {
        AfxMessageBox("Es necesario dar un valor de distancia y confiabilidad");
        return;
    }
    dUmbralConfiabilidad = atof(theApp.m_configuracion.m_csUmbralConfiabilidad);
    dUmbralDistancia = atof(theApp.m_configuracion.m_csUmbralDistancia);

    theApp.m_clasificacionBarrido.PrimeraPasada(dUmbralDistancia, dUmbralConfiabilidad,
                                            m_csMuestra, this);
}
*/

// PRUEBAS BORRAR
#include "..\librerias\procesoaux\gestion_mensajes.h"
//char fichLog[LONGITUD_TEXTO];

void CClasificaBarridoDlg::ActualizaProgresionMinerales(unsigned int* arrParcial, unsigned int* arrAcum,                                                                 
                                                        unsigned int nCountOscuros, unsigned int nCountNoClasificados, unsigned int nCountOscurosAcum, unsigned int nCountNoClasificadosAcum,
                                                        int nCamposTerminados, bool bUltimoCampo,
                                                        FILE* ficheroResultadoBarrido)
{
    CString     csAux;
    CString     csName;
    unsigned int         nTotalParcial = 0;
    unsigned int         nTotalAcum    = 0;

    int nMinerales = theApp.m_minerales.GetCount();
    int nAnchoxAltoImagen = theApp.m_nAnchoImagen * theApp.m_nAltoImagen;

    m_progreso.DeleteAllItems();
    m_progreso_acum.DeleteAllItems();
    m_progreso_no.DeleteAllItems();
    m_progreso_no_acum.DeleteAllItems();


    for (int nIndice=0; nIndice < nMinerales; nIndice++)
    {
        nTotalParcial += arrParcial[nIndice];
        nTotalAcum    += arrAcum[nIndice];
    }

    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)TRUE, (LPARAM)0); //Update display
    m_csCampoActual.Format("%d",nCamposTerminados);
    m_csPorcentajeAceptados.Format("%.2f", 100.0 * (nTotalAcum/(nAnchoxAltoImagen * (double) nCamposTerminados)));
CString mensaje;
mensaje.Format("Progreso. campo: %d porcentaje:%s", nCamposTerminados, m_csPorcentajeAceptados);
sendlog_timestamp(theApp.m_log,mensaje);

    // listas auxiliares necesarias para ordenar por numero de pixels
    list<CResultadoClasificacionBarrido> listClasificacionParcial;
    list<CResultadoClasificacionBarrido> listClasificacionAcum;
    list<CResultadoClasificacionBarrido>::iterator it_clasificacion;
    CResultadoClasificacionBarrido  resClasificacion;
    for (nIndice=0; nIndice < nMinerales; nIndice++)
    {
        if (arrAcum[nIndice] != 0)
        {
            resClasificacion.m_csName = theApp.m_minerales.GetNombre(nIndice);
            if (resClasificacion.m_csName.GetLength() > 22)
                resClasificacion.m_csName = theApp.m_minerales.GetAbreviatura(nIndice);

            if (arrParcial[nIndice]>0)
            {
                resClasificacion.m_dPorcentaje = 100.0 * (double)arrParcial[nIndice] / nTotalParcial;
                resClasificacion.m_nPixels = arrParcial[nIndice];
                listClasificacionParcial.push_back(resClasificacion);
            }

            if (arrAcum[nIndice]>0)
            {
                resClasificacion.m_dPorcentaje = 100.0 * (double)arrAcum[nIndice] / nTotalAcum;
                resClasificacion.m_nPixels = arrAcum[nIndice];
                listClasificacionAcum.push_back(resClasificacion);
            }

        }
    }
    listClasificacionParcial.sort();
    listClasificacionAcum.sort();

    if (ficheroResultadoBarrido != NULL)
    {
        fprintf(ficheroResultadoBarrido,"\n\n---------------  Campo %d  --------------\n", nCamposTerminados);
        fprintf(ficheroResultadoBarrido,"Numero_total_de_pixeles_descartados_por_oscuros:             %d\n",nCountOscuros);
        fprintf(ficheroResultadoBarrido,"Numero_total_de_pixeles_no_clasificados_(no_pasan_umbrales): %d\n\n",nCountNoClasificados);
        fprintf(ficheroResultadoBarrido,"%-22s %-9s %-6s\n","Mineral","Pixeles","Porcentaje");
    }
    for (it_clasificacion=listClasificacionParcial.begin();it_clasificacion!=listClasificacionParcial.end();it_clasificacion++)
    {
        csAux.Format("%-22s %9d %5.2f %%", (*it_clasificacion).m_csName, (*it_clasificacion).m_nPixels, (*it_clasificacion).m_dPorcentaje);
        m_progreso.InsertItem(_T(csAux), 0, 0, TVI_ROOT, NULL);
        // Guardar en fichero
        if (ficheroResultadoBarrido != NULL)
            fprintf(ficheroResultadoBarrido,"%s\n", csAux);
    }
    if (ficheroResultadoBarrido != NULL && bUltimoCampo) // solo si ultimo campo
    {
        fprintf(ficheroResultadoBarrido,"\n\n---------------  TOTAL  --------------\n",nCountOscuros);
        fprintf(ficheroResultadoBarrido,"Numero_total_de_pixeles_descartados_por_oscuros:             %d\n",nCountOscurosAcum);
        fprintf(ficheroResultadoBarrido,"Numero_total_de_pixeles_no_clasificados_(no_pasan_umbrales): %d\n\n",nCountNoClasificadosAcum);
        fprintf(ficheroResultadoBarrido,"Mineral             \tPixeles\tPorcentaje\n");
        PermitirNuevaClasificacion(); // prepara la interfaz para una nueva clasificacion
    }
    for (it_clasificacion=listClasificacionAcum.begin();it_clasificacion!=listClasificacionAcum.end();it_clasificacion++)
    {
        csAux.Format("%-22s %9d %5.2f %%", (*it_clasificacion).m_csName, (*it_clasificacion).m_nPixels, (*it_clasificacion).m_dPorcentaje);
        m_progreso_acum.InsertItem(_T(csAux), 0, 0, TVI_ROOT, NULL);
        if (ficheroResultadoBarrido != NULL && bUltimoCampo) // solo si ultimo campo
            fprintf(ficheroResultadoBarrido,"%s\n", csAux);
    }

    csAux.Format("%-22s %9d %5.2f %%", "Demasiado oscuros:" , nCountOscuros, (double)nCountOscuros * 100 / nAnchoxAltoImagen);
    m_progreso_no.InsertItem(_T(csAux), 0, 0, TVI_ROOT, NULL);
    csAux.Format("%-22s %9d %5.2f %%", "No pasan umbrales:" , nCountNoClasificados, (double)nCountNoClasificados * 100 / nAnchoxAltoImagen);
    m_progreso_no.InsertItem(_T(csAux), 0, 0, TVI_ROOT, NULL);

    csAux.Format("%-22s %9d %5.2f %%", "Demasiado oscuros:" , nCountOscurosAcum, 100.0 * (nCountOscurosAcum / (nAnchoxAltoImagen * (double) nCamposTerminados)));
    m_progreso_no_acum.InsertItem(_T(csAux), 0, 0, TVI_ROOT, NULL);
    csAux.Format("%-22s %9d %5.2f %%", "No pasan umbrales:" , nCountNoClasificadosAcum, 100.0 * (nCountNoClasificadosAcum / (nAnchoxAltoImagen * (double) nCamposTerminados)));
    m_progreso_no_acum.InsertItem(_T(csAux), 0, 0, TVI_ROOT, NULL);

    //UpdateData(FALSE);
    ::SendMessage(this->m_hWnd, WM_UPDATEFIELDS, (WPARAM)FALSE, (LPARAM)0); //Update display



}

void CClasificaBarridoDlg::OnSegundaPasada() 
{
    //Recuperamos umbrales de dialogo
    UpdateData();

    ASSERT(m_progreso.DeleteAllItems());
    ASSERT(m_progreso_acum.DeleteAllItems());
    ASSERT(m_progreso_no.DeleteAllItems());
    ASSERT(m_progreso_no_acum.DeleteAllItems());
    m_csCampoActual.Format("%d",0);
    m_csPorcentajeAceptados.Empty();
    UpdateData(FALSE);

    if (theApp.m_configuracion.m_dConfiabilidadSegunda <= 0 || theApp.m_configuracion.m_dDistanciaSegunda <= 0)
    {
        AfxMessageBox("Es necesario dar un valor de distancia y confiabilidad");
        return;
    }

    // No permitimos modificar configuracion durante la clasificacion
    m_Menu.EnableMenuItem(0,MF_BYPOSITION | MF_GRAYED); 
    DrawMenuBar(); 
//  CMenu* submenu = m_Menu.GetSubMenu(0); // Menu "Configuracion"
//  submenu->EnableMenuItem(0,MF_BYPOSITION | MF_GRAYED); 
//  submenu->EnableMenuItem(1,MF_BYPOSITION | MF_GRAYED); 

    m_buttonSegundaPasada.EnableWindow(FALSE); // Deshabilitamos boton de clasificacion
    m_buttonExaminar.EnableWindow(FALSE); // Deshabilitamos boton de examinar
    m_Directorio.EnableWindow(FALSE); //deshabilitar edit directorio

    theApp.m_clasificacionBarrido.SegundaPasada(theApp.m_configuracion.m_dDistanciaSegunda, theApp.m_configuracion.m_dConfiabilidadSegunda, theApp.m_configuracion.m_dRefMin,
                                            m_csMuestra, this);
}

// prepara la interfaz para una nueva clasificacion
void CClasificaBarridoDlg::PermitirNuevaClasificacion()
{
    m_Menu.EnableMenuItem(0,MF_BYPOSITION | MF_ENABLED); 
    DrawMenuBar(); 

    m_buttonSegundaPasada.EnableWindow(TRUE);
    m_buttonExaminar.EnableWindow(TRUE);
    m_Directorio.EnableWindow(TRUE); // edit directorio

}

////////////////////////////////////////////////////////////////////////////
/// MENU
////////////////////////////////////////////////////////////////////////////

void CClasificaBarridoDlg::OnConfiguracionOpciones() 
{
	int nResponse = theApp.m_configuracion.DoModal();
	if (nResponse == IDOK)
	{
        //habilitamos filtrado de asociaciones si los umbrales son validos y hay barrido definido
        if (theApp.m_configuracion.m_dDistanciaPrimera > 0 && theApp.m_configuracion.m_dConfiabilidadPrimera > 0
            && !m_csCamposTotal.IsEmpty())
            m_asociacionesDlg.HabilitaFiltrar(TRUE, m_csMuestra);
        else
            m_asociacionesDlg.HabilitaFiltrar(FALSE);
	}
	else if (nResponse == IDCANCEL)
	{
	}	
}

void CClasificaBarridoDlg::OnConfiguracionAsociacionesminerales() 
{
	int nResponse = m_asociacionesDlg.DoModal();
	if (nResponse == IDOK)
	{
        //Guardamos opciones marcadas
        theApp.m_asociaciones.GuardarFicheroAsociacionesSeleccionadas(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS);
        //guardamos tambien en el fichero por defecto (para usar como nueva plantilla)
        CopyFile(theApp.m_configuracion.m_csDefaultPath+FICHERO_ASOCIACIONES_SELECCIONADAS, theApp.m_configuracion.m_csPathEjecutable+FICHERO_ASOCIACIONES_SELECCIONADAS,FALSE);

        bool bAlgunaAsociacion = false;
        if (theApp.m_asociaciones.m_bAsociacionUsuario)
            bAlgunaAsociacion = true;
        else
        {
            for (int i= 0; i<theApp.m_asociaciones.m_nAsociaciones;i++)
            {
                if (theApp.m_asociaciones.m_arrCompatibilidadAsociaciones[i])
                {
                    // habilitamos segunda pasada 
                    bAlgunaAsociacion = true;
                    break;
                }
            }
        }
        if (bAlgunaAsociacion)
            // habilitamos segunda pasada 
            m_buttonSegundaPasada.EnableWindow(TRUE);
        else
            // habilitamos segunda pasada 
            m_buttonSegundaPasada.EnableWindow(FALSE);
	}
	else if (nResponse == IDCANCEL)
	{
	}	
}
