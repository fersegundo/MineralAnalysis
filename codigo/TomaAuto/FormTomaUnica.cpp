// FormTomaAuto.cpp : implementation file
//

#include "stdafx.h"
#include "TomaAuto.h"
#include "defines.h"
#include "FormTomaUnica.h"
#include "TomaAutoDlg.h"
#include "folder_dialog.h"
#include "..\librerias\ControlDisp\control_camara.h"
#include "..\librerias\ControlMicroscopio\control_microscopio.h" //para mspWhereP en el thread de Posicion
#include "..\librerias\LibBarrido\control_barrido.h" //para captura_bandas
#include <direct.h> //mkdir
#include <errno.h> //mkdir

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CTomaAutoApp theApp;
extern HANDLE  g_hMutexMicroscopio; // SINCRONIZACION para el uso exclusivo del microscopio (para que Threads no hagan MspWhere a la vez)
extern limitesCam	limCam; // declarado en la aplicacion llamante y usado aqui

/////////////////////////////////////////////////////////////////////////////
// CFormTomaUnica dialog


CFormTomaUnica::CFormTomaUnica(CWnd* pParent /*=NULL*/)
	: CFormComun(CFormTomaUnica::IDD, pParent)
{
    m_bCambiadoManualmente  = false;
//    m_enumEsquinaActiva     = ESQUINA_1;
    m_nCampo                = 1;
    m_pPadre                = NULL;

    //{{AFX_DATA_INIT(CFormTomaUnica)
	m_static_dimension_x = _T("");
	m_static_dimension_y = _T("");
	//}}AFX_DATA_INIT
}

void CFormTomaUnica::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CFormTomaUnica)
	DDX_Control(pDX, IDC_ESCALA_VENTANA, m_ratio);
	DDX_Control(pDX, IDC_DIMENSION_MAX_Y, m_dimension_max_y);
	DDX_Control(pDX, IDC_DIMENSION_MAX_X, m_dimension_max_x);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_X, m_dimension_x);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_Y, m_dimension_y);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_MICRAS_Y, m_dimension_micras_y);
	DDX_Control(pDX, IDC_EDIT_DIMENSION_MICRAS_X, m_dimension_micras_x);
	DDX_Control(pDX, IDC_EDIT_ESCALA, m_escala);
	DDX_Control(pDX, IDC_STATIC_POS_X, m_pos_x);
	DDX_Control(pDX, IDC_STATIC_POS_Z, m_pos_z);
	DDX_Control(pDX, IDC_STATIC_POS_Y, m_pos_y);
	DDX_Control(pDX, IDC_EDIT_NOMBRE, m_nombre);
	DDX_Control(pDX, IDC_IMAGE, m_imagen);
	DDX_Control(pDX, IDC_EDIT_DIRECTORIO, m_Directorio);
	//}}AFX_DATA_MAP
	DDX_Control(pDX, IDC_STATIC_NM, m_nanometros);
	DDX_Control(pDX, IDC_STATIC_LONGITUD_ONDA, m_longitud_onda);
	DDX_Control(pDX, IDC_FILTRO_ACTUAL, m_filtro_actual);
}


BEGIN_MESSAGE_MAP(CFormTomaUnica, CDialog)
	//{{AFX_MSG_MAP(CFormTomaUnica)
	ON_BN_CLICKED(IDC_DIRECTORIO, OnExaminar)
	ON_EN_CHANGE(IDC_EDIT_DIRECTORIO, OnChangeEditDirectorio)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_X, OnChangeEditDimensionX)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_Y, OnChangeEditDimensionY)
	ON_EN_CHANGE(IDC_EDIT_ESCALA, OnChangeEditEscala)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_MICRAS_X, OnChangeEditDimensionMicrasX)
	ON_EN_CHANGE(IDC_EDIT_DIMENSION_MICRAS_Y, OnChangeEditDimensionMicrasY)
	ON_WM_SIZE()
	ON_EN_KILLFOCUS(IDC_ESCALA_VENTANA, OnKillfocusEscalaVentana)
	ON_WM_DESTROY()
	ON_WM_SHOWWINDOW()
	ON_BN_CLICKED(IDC_TOMA, OnToma)
	ON_BN_CLICKED(IDC_AUTOENFOCAR, OnAutoenfocar)
	ON_WM_GETMINMAXINFO()
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CFormTomaUnica message handlers

BOOL CFormTomaUnica::OnInitDialog() 
{
	CFormComun::OnInitDialog();
	
    UpdateData(FALSE);

    // Inicializacion variables
    m_Directorio.SetWindowText(theApp.m_configuracion.m_csDirectorioToma);
    CString csDimension;
    if (theApp.m_configuracion.m_nDimension_toma_x > 0)
        csDimension.Format("%d",theApp.m_configuracion.m_nDimension_toma_x);
    else
        csDimension.Empty();
    m_dimension_x.SetWindowText(csDimension);
    if (theApp.m_configuracion.m_nDimension_toma_y > 0)
        csDimension.Format("%d",theApp.m_configuracion.m_nDimension_toma_y);
    else
        csDimension.Empty();
    m_dimension_y.SetWindowText(csDimension);

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

void CFormTomaUnica::OnDestroy() 
{
	CDialog::OnDestroy();
	
    fin_toma();
}

void CFormTomaUnica::OnShowWindow(BOOL bShow, UINT nStatus) 
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
            ActualizarEscala(); //en dialogo usando theApp.m_CorreccionGeometrica.m_dEscalaObjetivo

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
BOOL CFormTomaUnica::PreTranslateMessage(MSG* pMsg)
{
    if (pMsg->message == WM_KEYDOWN)
    {
        if ((pMsg->wParam == VK_RETURN) || (pMsg->wParam == VK_ESCAPE))
            pMsg->wParam = VK_TAB;
    }
    return CDialog::PreTranslateMessage(pMsg);
}

// Mostrar explicitamente la posicion dada en el dialogo
void CFormTomaUnica::MostrarPosicion(double x, double y, double z)
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
void CFormTomaUnica::MostrarFiltro(int nFiltro)
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



////////////////////////////////////////////////////////////////////////////////////
//
//      CONFIGURACION DEL BARRIDO
//
////////////////////////////////////////////////////////////////////////////////////

// Cambia el directorio de trabajo
void CFormTomaUnica::OnExaminar() 
{
	CFolderDialog dlg(&theApp.m_configuracion.m_csDirectorioToma);
	if (dlg.DoModal() == IDOK)
	{
		// pathSelected contain the selected folder.
		//theApp.m_configuracion.m_csDirectorioToma = pathSelected;
        m_Directorio.SetWindowText(theApp.m_configuracion.m_csDirectorioToma);
	}  
}

//Para actualizar el directorio de trabajo si se modifica manualmente
void CFormTomaUnica::OnChangeEditDirectorio() 
{
    LPTSTR temp = theApp.m_configuracion.m_csDirectorioToma.GetBuffer( 1024 );
	m_Directorio.GetLine(0,temp);	
    theApp.m_configuracion.m_csDirectorioToma.ReleaseBuffer();  // Surplus memory released, temp is now invalid.

    // Nos aseguramos de que el ultimo caracter sea '\'
    CString csUltimoCaracter = theApp.m_configuracion.m_csDirectorioToma.Right(1);
    if (csUltimoCaracter != "\\" && csUltimoCaracter != "/")
        theApp.m_configuracion.m_csDirectorioToma += _T("\\");
}

void CFormTomaUnica::OnChangeEditDimensionX() 
{
    //Obtenemos la dimension del dialogo
    CString csAux;
    m_dimension_x.GetWindowText(csAux);
    if (!m_bCambiadoManualmente)
    {
        theApp.m_configuracion.m_nDimension_toma_x = atoi((LPCSTR)csAux);
        if (theApp.m_configuracion.m_nDimension_barrido_x > limCam.anchoDig)
        {
            theApp.m_configuracion.m_nDimension_barrido_x = limCam.anchoDig;
            csAux.Format("%d",theApp.m_configuracion.m_nDimension_barrido_x);
            m_bCambiadoManualmente = true;
            m_dimension_x.SetWindowText((LPCTSTR)csAux);
        }

        if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_toma_x > 0)
        {
            //Actualizar la dimension en micras (dejamos la escala constante)
            csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_toma_x * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
            m_bCambiadoManualmente = true;
            m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
        }
        else if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0) //theApp.m_configuracion.m_nDimension == 0
        {
            // Borrar dimension en micras
            m_bCambiadoManualmente = true;
            m_dimension_micras_x.SetWindowText("");
        }
        else if (theApp.m_configuracion.m_nDimension_toma_x > 0) // theApp.m_CorreccionGeometrica.m_dEscalaObjetivo == 0
        {
            m_dimension_micras_x.GetWindowText(csAux);
            if (csAux.IsEmpty == FALSE)
            {
                //Actualizar Escala
                theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_toma_x;
                csAux.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                m_bCambiadoManualmente = true;
                m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);
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

void CFormTomaUnica::OnChangeEditDimensionY() 
{
    //Obtenemos la dimension del dialogo
    CString csAux;
    m_dimension_y.GetWindowText(csAux);
    if (!m_bCambiadoManualmente)
    {
        theApp.m_configuracion.m_nDimension_toma_y = atoi((LPCSTR)csAux);
        if (theApp.m_configuracion.m_nDimension_barrido_y > limCam.altoDig)
        {
            theApp.m_configuracion.m_nDimension_barrido_y = limCam.altoDig;
            csAux.Format("%d",theApp.m_configuracion.m_nDimension_barrido_y);
            m_bCambiadoManualmente = true;
            m_dimension_y.SetWindowText((LPCTSTR)csAux);
        }


        if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_toma_y > 0)
        {
            //Actualizar la dimension en micras (dejamos la escala constante)
            csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_toma_y * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
            m_bCambiadoManualmente = true;
            m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
        }
        else if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0) //theApp.m_configuracion.m_nDimension == 0
        {
            // Borrar dimension en micras
            m_bCambiadoManualmente = true;
            m_dimension_micras_y.SetWindowText("");
        }
        else if (theApp.m_configuracion.m_nDimension_toma_y > 0) // theApp.m_CorreccionGeometrica.m_dEscalaObjetivo == 0
        {
            m_dimension_micras_y.GetWindowText(csAux);
            if (csAux.IsEmpty == FALSE)
            {
                //Actualizar Escala
                theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_toma_y;
                csAux.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                m_bCambiadoManualmente = true;
                m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);
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

void CFormTomaUnica::OnChangeEditDimensionMicrasX() 
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
            if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_toma_x > 0)
            {
                // Borrar dimension en pixels
                m_bCambiadoManualmente = true;
                theApp.m_configuracion.m_nDimension_toma_x = 0;
                m_dimension_x.SetWindowText("");

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
                    theApp.m_configuracion.m_nDimension_toma_x = (int)(atof((LPCSTR)csAux) / theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    csAux.Format("%d",theApp.m_configuracion.m_nDimension_toma_x);
                    m_bCambiadoManualmente = true;
                    m_dimension_x.SetWindowText((LPCTSTR)csAux);
                }
                else
                {
                    if (theApp.m_configuracion.m_nDimension_toma_x > 0)
                    {
                        //Actualizar Escala
                        theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_toma_x;
                        csAux.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                        m_bCambiadoManualmente = true;
                        m_escala.SetWindowText((LPCTSTR)csAux);
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

void CFormTomaUnica::OnChangeEditDimensionMicrasY() 
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
            if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > 0 && theApp.m_configuracion.m_nDimension_toma_y > 0)
            {
                // Borrar dimension en pixels
                m_bCambiadoManualmente = true;
                theApp.m_configuracion.m_nDimension_toma_y = 0;
                m_dimension_y.SetWindowText("");

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
                    theApp.m_configuracion.m_nDimension_toma_y = (int)(atof((LPCSTR)csAux) / theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    csAux.Format("%d",theApp.m_configuracion.m_nDimension_toma_y);
                    m_bCambiadoManualmente = true;
                    m_dimension_y.SetWindowText((LPCTSTR)csAux);
                }
                else
                {
                    if (theApp.m_configuracion.m_nDimension_toma_y > 0)
                    {
                        //Actualizar Escala
                        theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = atof(csAux) / theApp.m_configuracion.m_nDimension_toma_y;
                        csAux.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                        m_bCambiadoManualmente = true;
                        m_escala.SetWindowText((LPCTSTR)csAux);
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

void CFormTomaUnica::OnChangeEditEscala() 
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
            if (theApp.m_configuracion.m_nDimension_toma_x > 0)
            {
                // Si no hay informacion de escala, no puede haberla de dimension en micras
                m_bCambiadoManualmente = true;
                m_dimension_micras_x.SetWindowText("");

            }
            // Y
            if (theApp.m_configuracion.m_nDimension_toma_y > 0)
            {
                // Si no hay informacion de escala, no puede haberla de dimension en micras
                m_bCambiadoManualmente = true;
                m_dimension_micras_y.SetWindowText("");
            }
            //Borramos las dimensiones maximas
            m_dimension_max_x.SetWindowText("");
            m_dimension_max_y.SetWindowText("");
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
                    csAux.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_escala.SetWindowText((LPCTSTR)csAux);
                }
                if (theApp.m_CorreccionGeometrica.m_dEscalaObjetivo > MAX_ESCALA_REFLECTANCIA)
                {
                    theApp.m_CorreccionGeometrica.m_dEscalaObjetivo = MAX_ESCALA_REFLECTANCIA;
                    csAux.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_escala.SetWindowText((LPCTSTR)csAux);
                }
                // X
                if (theApp.m_configuracion.m_nDimension_toma_x > 0)
                {
                    //Actualizar la dimension en micras (dejamos la dimension en pixels constante)
                    csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_toma_x * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_dimension_micras_x.SetWindowText((LPCTSTR)csAux);

                }
                // Y
                if (theApp.m_configuracion.m_nDimension_toma_y > 0)
                {
                    //Actualizar la dimension en micras (dejamos la dimension en pixels constante)
                    csAux.Format("%.2f",theApp.m_configuracion.m_nDimension_toma_y * theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
                    m_bCambiadoManualmente = true;
                    m_dimension_micras_y.SetWindowText((LPCTSTR)csAux);

                }
                //Actualizamos las dimensiones maximas (que unicamente dependen de la escala)
                csAux.Format("%d",theApp.CalcularAnchoMax(false));
                m_dimension_max_x.SetWindowText((LPCTSTR)csAux);
                csAux.Format("%d",theApp.CalcularAltoMax(false));
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
void CFormTomaUnica::ActualizarEscala()
{
    CString csEscala;
    csEscala.Format("%.3f",theApp.m_CorreccionGeometrica.m_dEscalaObjetivo);
    m_escala.SetWindowText(csEscala);
}






/////////////////////////////////////////////////////////////////////////////////////
//
//  ZOOM - ESCALA
//
/////////////////////////////////////////////////////////////////////////////////////

//AUXILIAR
//Calcula los factores de zoom entre el tamaño del display y la imagen para poder
//mostrarla posteriormente correctamente redimensionada en el display
void CFormTomaUnica::CalculaZoom(long anchoDisplay,long altoDisplay)
{
    double factorZoomX = (double)anchoDisplay / theApp.m_ParamIni.Cam.anchoImagen; //theApp.m_configuracion.m_nDimension_toma_x;
    double factorZoomY = (double)altoDisplay / theApp.m_ParamIni.Cam.altoImagen; //theApp.m_configuracion.m_nDimension_toma_y;

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
double CFormTomaUnica::ActualizaZoom()
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
        ancho = (long)floor(theApp.m_ParamIni.Cam.anchoImagen /*theApp.m_configuracion.m_nDimension_toma_x*/ * -1.0/theApp.m_configuracion.m_ratio_inicial + 0.5);
        alto = (long)floor(theApp.m_ParamIni.Cam.altoImagen/*theApp.m_configuracion.m_nDimension_toma_y*/ * -1.0/theApp.m_configuracion.m_ratio_inicial + 0.5);
    }
    else 
    {
        ancho = (long)floor(theApp.m_ParamIni.Cam.anchoImagen/*theApp.m_configuracion.m_nDimension_toma_x*/ * theApp.m_configuracion.m_ratio_inicial + 0.5);
        alto = (long)floor(theApp.m_ParamIni.Cam.altoImagen/*theApp.m_configuracion.m_nDimension_toma_y*/ * theApp.m_configuracion.m_ratio_inicial + 0.5);
    }

    m_imagen.SetWindowPos(NULL,0,0,ancho,alto,SWP_NOMOVE | SWP_NOZORDER);

    MdispZoom(M_display_normal, theApp.m_configuracion.m_ratio_inicial, theApp.m_configuracion.m_ratio_inicial);

    return theApp.m_configuracion.m_ratio_inicial;
}

void CFormTomaUnica::OnSize(UINT nType, int cx, int cy) 
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
void CFormTomaUnica::OnKillfocusEscalaVentana() 
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
    //        long anchoDisplay = (long)floor(theApp.m_configuracion.m_nDimension_toma_x * ratio  + 0.5);
    //        long altoDisplay = (long)floor(theApp.m_configuracion.m_nDimension_toma_y * ratio + 0.5);
            this->SetWindowPos(NULL,0,0,anchoDisplay+m_anchoResto,altoDisplay+m_altoResto, SWP_NOMOVE | SWP_NOZORDER);
            if (m_pPadre)
                m_pPadre->SetSize(anchoDisplay+m_anchoResto+m_anchoTitulo,altoDisplay+m_altoResto+m_altoTitulo);

            // Se redimensionara el display en funcion del display (limitandolo al tamaño de la imagen 
            // y forzando a que sea cuadrado)
            ActualizaZoom();
        }
    }
}

int CFormTomaUnica::GetAnchoResto()
{
    return m_anchoResto + m_anchoTitulo;
}

int CFormTomaUnica::GetAltoResto()
{
    return m_altoResto + m_altoTitulo;
}

HWND CFormTomaUnica::GetControlImagen()
{
    return (HWND) m_imagen;
}

// Controles para redimensionar los elementos del dialogo
BEGIN_EASYSIZE_MAP(CFormTomaUnica)
    EASYSIZE(IDC_IMAGE                        ,ES_BORDER,ES_BORDER,ES_BORDER,ES_BORDER,0)
// IDC_AREA_DERECHA
    EASYSIZE( IDC_STATIC_ESCALA_VENTANA       ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_ESCALA_VENTANA              ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_FILTRO_ACTUAL               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_FILTRO_ACTUAL        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_LONGITUD_ONDA        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_NM                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_POSICION             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSX                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSY                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POSZ                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_POS_X                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Y                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_POS_Z                ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    

    EASYSIZE( IDC_STATIC_DIMENSIONES          ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_STATIC_ESCALA               ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_EDIT_ESCALA                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_TAMPIXELS            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_EDIT_DIMENSION_X            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_x5                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_DIMENSION_Y            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_TAMMICRAS            ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_EDIT_DIMENSION_MICRAS_X     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_x6                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_EDIT_DIMENSION_MICRAS_Y     ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_MAX                  ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_DIMENSION_MAX_X             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)    
    EASYSIZE( IDC_STATIC_x1                   ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_DIMENSION_MAX_Y             ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)   
    
    EASYSIZE( IDC_STATIC_TOMA                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_TOMA                        ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
    EASYSIZE( IDC_AUTOENFOCAR                 ,ES_KEEPSIZE,ES_BORDER,ES_BORDER,ES_KEEPSIZE,0)
END_EASYSIZE_MAP



// Estructura para pasar a los thread de Toma
typedef struct PARAM_THREAD_TOMA
{
    CFormTomaUnica*   pDialogo;
    CString           csNombreToma;
} PARAM_THREAD_TOMA;

void CFormTomaUnica::OnToma() 
{    
    if (theApp.CalcularAnchoMax(false) < theApp.m_configuracion.m_nDimension_toma_x)
    {
        AfxMessageBox("Imposible efectuar barrido: Ancho de imagen mayor que maximo - Revise los valores de Tamaño");
        return;
    }
    else if (theApp.CalcularAltoMax(false) < theApp.m_configuracion.m_nDimension_toma_y) 
    {
        AfxMessageBox("Imposible efectuar barrido: Alto de imagen mayor que maximo - Revise los valores de Tamaño");
        return;
    }

    //Finaliza el thread de posicion 
    m_pPadre->FinalizarThreadPosicion();

    // Aunque el thread se haya finalizado, para mayor seguridad, nos aseguramos que tengamos acceso exclusivo al microscopio
    DWORD dwWaitResult = WaitForSingleObject(g_hMutexMicroscopio,500L);   // half-second time-out interval
    if (dwWaitResult == WAIT_OBJECT_0)
    {
        theApp.Rueda.zObservacion = mspWhere(Z_);

        // Release ownership of the mutex object.
        if (! ReleaseMutex(g_hMutexMicroscopio))
            ASSERT(AfxMessageBox("ReleaseMutex error al OnBarrer") );
    }

    //Crear directorio si no esta ya creado
    if (mkdir(theApp.m_configuracion.m_csDirectorioToma) != 0)
	{
        if (GetLastError() != ERROR_ALREADY_EXISTS)
        {
		    AfxMessageBox("NO se puede crear el directorio");
            return;
        }
	}

    PARAM_THREAD_TOMA* paramThread = new PARAM_THREAD_TOMA; //debe estar en el heap para que sea accesible desde el thread. Se borra al final del propio thread
    paramThread->pDialogo = this;
 
    UpdateData(); //para que se actualice el nombre de la toma
    UpdateData(FALSE); //para que se actualice el nombre de la toma
    LPTSTR temp = paramThread->csNombreToma.GetBuffer( 1024 );
	int nRet = m_nombre.GetLine(0,temp, 1024);	
    paramThread->csNombreToma.ReleaseBuffer();  // Surplus memory released, temp is now invalid.

    AfxBeginThread(TomaThread, (void *)paramThread);
}

UINT CFormTomaUnica::TomaThread( LPVOID Param )
{
    PARAM_THREAD_TOMA* paramThread = (PARAM_THREAD_TOMA*)Param;

    //Tomar Imagenes (en M_banda[])
    captura_bandas( theApp.m_ParamIni, theApp.m_configuracion.m_nImagenAcum,theApp.Rueda, theApp.m_configuracion.m_dEscalaReflectancia);

    //Lanzamos el thread de Actualizacion de Posicion
    paramThread->pDialogo->m_pPadre->LanzarThreadPosicion();

    // Correcion de las diferencias de posicion, escala, etc entre bandas
    theApp.CorregirImagenes(false); //false=sin corregir rotacion

	//	Una vez obtenida la imagen promedio se guarda esta en disco 
	char nombre_fich[512];

    long nInicioX   = 0;
    int nTamX       = theApp.m_ParamIni.Cam.anchoImagen;        
    if (theApp.m_configuracion.m_nDimension_toma_x > 0)
    {
        nInicioX    = (long)(theApp.m_ParamIni.Cam.anchoImagen - theApp.m_configuracion.m_nDimension_toma_x)/2;
        nTamX       = theApp.m_configuracion.m_nDimension_toma_x;
    }

    long nInicioY   = 0;
    int nTamY       = theApp.m_ParamIni.Cam.altoImagen;        
    if (theApp.m_configuracion.m_nDimension_toma_y > 0)
    {
        nInicioY    = (long)(theApp.m_ParamIni.Cam.altoImagen - theApp.m_configuracion.m_nDimension_toma_y)/2;
        nTamY       = theApp.m_configuracion.m_nDimension_toma_y;
    }

	for (int f=0; f < theApp.m_ParamIni.nBandas; f++)  
    {
        // Guardamos en el tamaño de salida definido por el usuario
        MIL_ID Mchild;
        MbufChild2d(M_banda[f],nInicioX, nInicioY, nTamX,nTamY,&Mchild);
		sprintf(nombre_fich, "%s%s_%03d_%02d%s", theApp.m_configuracion.m_csDirectorioToma, paramThread->csNombreToma, paramThread->pDialogo->m_nCampo, theApp.m_ParamIni.filtro[f]+1, EXT_IMAG);
		graba_imagen_campo_bits(theApp.m_ParamIni, nombre_fich, Mchild, theApp.m_configuracion.m_nBits, theApp.m_configuracion.m_dEscalaReflectancia); 
        MbufFree(Mchild);
    }

    paramThread->pDialogo->m_nCampo++;

    delete paramThread;


    return 0;
}

void CFormTomaUnica::OnAutoenfocar() 
{
    m_pPadre->Autoenfocar();
}
