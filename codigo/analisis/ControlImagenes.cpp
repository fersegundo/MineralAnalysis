#include "stdafx.h"
#include <math.h> //para floor
#include <sys/stat.h> //para stat
#include "Analisis.h"
#include "AnalisisDlg.h"
#include "ControlImagenes.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern CAnalisisApp theApp;

ControlImagenes::ControlImagenes()
{
    //Inicilizacion
//    m_Milsistema        = M_NULL;
    m_Milaplicacion     = M_NULL;
    m_Mildisplay        = M_NULL;
    m_Miloverlay        = M_NULL;
    m_lut_overlay       = M_NULL;
    m_numImagenes       = 0;
    m_nAnchoImagen      = -1;
    m_nAltoImagen       = -1;
    for (int i=0;i<MAX_NUM_IMAGENES;i++)
        m_Milimagen[i]  = M_NULL;        
    m_MilRGB            = M_NULL;
    M_Clasificacion     = M_NULL;
    m_bufClasificacionSelectiva = NULL;
    m_bDefiniendoArea   = false;
    m_bPintandoRect     = false;


	MappAlloc(M_DEFAULT, &m_Milaplicacion);	// Selecciono la aplicación MIL.
	if ( m_Milaplicacion == M_NULL ) {
        AfxMessageBox("Error: No se pudo inicilizar MIL", MB_ICONERROR | MB_OK);
	}

	//	Pregunto al sistema la versión de la librería de control de proceso de imagen
	//MappInquire(M_VERSION, &m_Milversion);

    // pruebas MIL 8
//	MsysAlloc(M_SYSTEM_METEOR_II_1394, M_DEV0, M_DEFAULT, &M_sistema);	// Selecciono un sistema hardware

//	if ( M_sistema == M_NULL )  {
//        AfxMessageBox("Error: No se pudo inicilizar MIL", MB_ICONERROR | MB_OK);
//	}

    //	Display normal para las imágenes adquiridas y procesadas. 
	MdispAlloc(M_DEFAULT_HOST, M_DEFAULT, "M_DEFAULT", M_WINDOWED + M_GDI_OVERLAY, &m_Mildisplay);
	if ( m_Mildisplay == M_NULL )  {
        AfxMessageBox("Error: No se pudo inicilizar MIL", MB_ICONERROR | MB_OK);
	}

	// El texto tiene color de fondo transparente
    MgraControl(M_DEFAULT, M_BACKGROUND_MODE, M_TRANSPARENT);

    // OVERLAY (diferente para MIL 8 y anteriores
    if (M_MIL_CURRENT_VERSION >= 8.0)
    {
        // Inicializar colores para pintar areas
        m_listaColor[NEGRO_POS] =     M_RGB888(NEGRO_R,NEGRO_G,NEGRO_B);
        m_listaColor[MARRON_POS] =     M_RGB888(MARRON_R,MARRON_G,MARRON_B);
        m_listaColor[ROJO_POS] =     M_RGB888(ROJO_R,ROJO_G,ROJO_B);
        m_listaColor[CYAN_POS] =     M_RGB888(CYAN_R,CYAN_G,CYAN_B);
        m_listaColor[NARANJA_POS] =     M_RGB888(NARANJA_R,NARANJA_G,NARANJA_B);
        m_listaColor[VERDE_POS] =     M_RGB888(VERDE_R,VERDE_G,VERDE_B);
        m_listaColor[AZUL_POS] =     M_RGB888(AZUL_R,AZUL_G,AZUL_B);
        m_listaColor[MORADO_POS] =     M_RGB888(MORADO_R,MORADO_G,MORADO_B);
        m_listaColor[GRIS_POS] =     M_RGB888(GRIS_R,GRIS_G,GRIS_B);
        m_listaColor[ORO_POS] =     M_RGB888(ORO_R,ORO_G,ORO_B);
        m_listaColor[ROSA_POS] =    M_RGB888(ROSA_R,ROSA_G,ROSA_B);

        MdispControl(m_Mildisplay, M_OVERLAY, M_ENABLE);

        // Establezco TRANSPARENTE como color transparente
        MdispControl(m_Mildisplay, M_TRANSPARENT_COLOR, TRANSPARENTE);

        //Tipo de interpolacion para el redimensionamiento de la imagen en el display
        //Si se activa en MIL 8, quedan lineas de color transparernte en el overlay cuando el zoom no es 1
        //MdispControl(m_Mildisplay, M_INTERPOLATION_MODE, M_NEAREST_NEIGHBOR);
    }
    else
    {
        // Inicializar colores para pintar areas
        m_listaColor[0] =     NEGRO   ;
        m_listaColor[1] =     ROJO_OSC  ;
        m_listaColor[2] =     ROJO    ;
        m_listaColor[3] =     CYAN ;
        m_listaColor[4] =     AMARILLO;
        m_listaColor[5] =     VERDE   ;
        m_listaColor[6] =     AZUL    ;
        m_listaColor[7] =     MAGENTA_OSC  ;
        m_listaColor[8] =     GRIS    ;
        m_listaColor[9] =     AMARILLO_OSC     ;
        m_listaColor[10] =    GRIS_CLA   ;

        // LUT
	    MbufAllocColor(M_DEFAULT_HOST, 3L, 256L, 1L, 8L+M_UNSIGNED, M_LUT, &m_lut_overlay);
	    if ( m_lut_overlay == M_NULL)  {
            AfxMessageBox("Error: No se pudo inicilizar MIL", MB_ICONERROR | MB_OK);
	    }
    
	    crea_LUT_overlay(m_lut_overlay, 0);	// Genera LUT color y corrección de gamma
	    configura_overlay(m_Mildisplay, M_NULL, &m_Miloverlay, 1); // repartido entre aqui y CargarImagen // M_NULL, antes ponia M_imagen1 que ya no esta disponible en Analisis, seguramente falle

        //	Asocio la LUT de pseudo-color al display de overlay
	    MdispControl(m_Mildisplay, M_OVERLAY_LUT, m_lut_overlay);


	    //	Modifico la configuración del display: m_Mildisplay_normal.
	    //		- Deshabilito el menú de tareas.
	    //		- Deshabilito la barra del título.
	    //	    - Fijo la posición de la ventana en 0,0. 
	    MdispControl(m_Mildisplay, M_WINDOW_MENU_BAR, M_DISABLE);
	    MdispControl(m_Mildisplay, M_WINDOW_TITLE_BAR, M_DISABLE);
	    MdispControl(m_Mildisplay, M_WINDOW_INITIAL_POSITION_X, 0 );
	    MdispControl(m_Mildisplay, M_WINDOW_INITIAL_POSITION_Y, 0 );

        // Establezco TRANSPARENTE como color transparente
		MdispOverlayKey(m_Mildisplay, M_KEY_ON_COLOR, M_EQUAL, 0xff, TRANSPARENTE);
    }

    // Bitmap de fondo para poner siempre que no haya mas imagenes
    BYTE bBits[] = {0xd8,0xe9,0xec};
    m_backgound_bitmap.CreateBitmap(1,1,1,32,bBits);        
}

ControlImagenes::~ControlImagenes()
{
    for (int i=0;i<MAX_NUM_IMAGENES;i++)
    {
        if (m_Milimagen[i] != M_NULL)
            MbufFree(m_Milimagen[i]);
    }
    if (m_MilRGB != M_NULL)
        MbufFree(m_MilRGB);

    // CLASIFICACION
    if(M_Clasificacion!=M_NULL)
        MbufFree(M_Clasificacion);
    if (m_bufClasificacionSelectiva != NULL)
        delete [] m_bufClasificacionSelectiva;

    MdispFree(m_Mildisplay);
    //MsysFree(m_Milsistema);
    MappFree(m_Milaplicacion);
}


int ControlImagenes::GetAnchoImagen()
{
    return m_nAnchoImagen;
}

int ControlImagenes::GetAltoImagen()
{
    return m_nAltoImagen;
}

void ControlImagenes::SetAnchoImagen(int nAncho)
{
    m_nAnchoImagen = nAncho;
}

void ControlImagenes::SetAltoImagen(int nAlto)
{
    m_nAltoImagen = nAlto;
}


///////////////////////////////////////////////////////////////////////////////////////////////////
//
// ZOOM
//
///////////////////////////////////////////////////////////////////////////////////////////////////

double ControlImagenes::GetFactorZoom()
{
    return m_factorZoom;
}

// Actualiza y aplica al display el factor de zoom segun el tamaño del display 
// pero con limite el tamaño de la imagen
// Devuelve el zoom o ratio aplicado
double ControlImagenes::ActualizaZoom(CButton& m_control_group, CStatic&	m_control)
{
    // Ancho y alto maximo posible
    RECT Rect;
    m_control_group.GetClientRect(&Rect);
    long anchoDisplay = abs(Rect.right);
    long altoDisplay = abs(Rect.bottom);

    // calcula el zoom optimo (maximo) dentro de lo posible
    // el resultado esta en m_factorZoom
    CalculaZoom(anchoDisplay,altoDisplay,
        m_nAnchoImagen,m_nAltoImagen);

    //Ajustar la ventana de display al zoom calculado
    long alto,ancho;
    if (m_factorZoom < 0)
    {
        ancho = (long)floor(m_nAnchoImagen * -1.0/m_factorZoom + 0.5);
        alto = (long)floor(m_nAltoImagen * -1.0/m_factorZoom + 0.5);
    }
    else 
    {
        ancho = (long)floor(m_nAnchoImagen * m_factorZoom + 0.5);
        alto = (long)floor(m_nAltoImagen * m_factorZoom + 0.5);
    }

    m_control.SetWindowPos(NULL,0,0,ancho,alto,SWP_NOMOVE | SWP_NOZORDER);

    MdispZoom(m_Mildisplay, m_factorZoom, m_factorZoom);

    return m_factorZoom;
}

//AUXILIAR
//Calcula los factores de zoom entre el tamaño del display y la imagen para poder
//mostrarla posteriormente correctamente redimensionada en el display
void ControlImagenes::CalculaZoom(long anchoDisplay,long altoDisplay,
                                  long anchoIm,long altoIm)
{
    double factorZoomX = (double)anchoDisplay / anchoIm;
    double factorZoomY = (double)altoDisplay / altoIm;

    m_factorZoom  = __min(factorZoomX,factorZoomY);
    // SI VERSION MIL ES 8.0, factorZoom debe dejarse en coma flotante
    // SI VERSION MIL ES MENOR, factorZoom debe ser [-16 a -1] o [1 a 16]
    if (M_MIL_CURRENT_VERSION < 8.0)
    {
        if ( m_factorZoom > 16)
            m_factorZoom = 16;
        if ( m_factorZoom > 1)
            m_factorZoom = floor(m_factorZoom);
        else if (m_factorZoom < ((double)1/16))
            m_factorZoom = -16;
        else
            m_factorZoom = -ceil(1 / m_factorZoom);
    }
    else
    {
        if (m_factorZoom>1)
            m_factorZoom = 1; // No aumentamos el tamaño de la imagen
        else if (m_factorZoom < 0.5)
            m_factorZoom = 0.5; // Tamaño minimo del display, la mitad de la imagen
    }
}

// Calculamos el punto adecuado segun el factor de zoom
void ControlImagenes::AplicaZoom(CPoint& point)
{
    if (m_factorZoom < 0)
    {
        point.x = (long)floor(point.x * -m_factorZoom + 0.5);
        point.y = (long)floor(point.y * -m_factorZoom + 0.5);
    }
    else 
    {
        point.x = (long)floor(point.x * 1.0/m_factorZoom + 0.5);
        point.y = (long)floor(point.y * 1.0/m_factorZoom + 0.5);
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////
//
// MOUSE / AREAS
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Acciones de control y dibujo cuando se presiona el boton del mouse
// Se devolverá 'true' en el caso de que implique la formacion de un area, 
// devolviendose el area en 'area'
bool ControlImagenes::OnLButtonDown(CPoint& point,CRect& area, int nColorIndex)
{
    bool bAreaDefinida = false;

    if (m_numImagenes > 0)
    {
        // Primero calculamos el punto adecuado segun el factor de zoom
        AplicaZoom(point);

        if (m_bDefiniendoArea)
        {
            //nos aseguramos que no se haya seleccionado de nuevo el mismo punto
            //Si se selecciona el mismo punto no se hace nada (aun definiendo area)
            if (m_punto_inicial != point) 
            {
                //Este es el segundo punto (que define un area)
                m_bDefiniendoArea = false;
                m_bPintandoRect = false;
                muestra_punto(m_punto_inicial.x,m_punto_inicial.y, 3, TRANSPARENTE,0); // borramos el punto inicial 
                //Nos aseguramos que el area no sea nula
                //Si es nula, no se define el area
                if (m_punto_inicial.x != point.x && m_punto_inicial.y != point.y) 
                {
                    bAreaDefinida = true;
                    DibujaRect(m_punto_inicial, point,m_listaColor[nColorIndex]);
                    area.left   = __min(m_punto_inicial.x,point.x);
                    area.top    = __min(m_punto_inicial.y,point.y);
                    area.right  = __max(m_punto_inicial.x,point.x);
                    area.bottom = __max(m_punto_inicial.y,point.y);
                }
            }
        }
        else
        {
            m_punto_inicial = point;
            m_bDefiniendoArea = true;
            DibujaPunto(point, M_COLOR_RED);
        }
    }

    return bAreaDefinida;

}

// Acciones de control y dibujo cuando se suelta el boton del mouse
// Se devolverá 'true' en el caso de que implique la formacion de un area, 
// devolviendose el area en 'area'
bool ControlImagenes::OnLButtonUp(CPoint& point,CRect& area, int nColorIndex)
{
    bool bAreaDefinida = false;

    if (m_numImagenes > 0)
    {
        if (m_bDefiniendoArea) //Nos aseguramos, podria ser que se hubiese formado el area al pulsar
        {
            // Primero calculamos el punto adecuado segun el factor de zoom
            AplicaZoom(point);
            //nos aseguramos que no estemos aun en el mismo punto
            if (m_punto_inicial != point) 
            {
                //Este es el segundo punto (que define un area)
                m_bDefiniendoArea = false;
                m_bPintandoRect = false;
                // nos aseguramos que el area no sea nula
                if (m_punto_inicial.x != point.x && m_punto_inicial.y != point.y) 
                {
                    //Este es el segundo punto (que define un area)
                    bAreaDefinida = true;
                    DibujaRect(m_punto_inicial, point, m_listaColor[nColorIndex]); 

                    // el area que devolvemos esta "normalizada" (sin zoom)
                    area.left   = __min(m_punto_inicial.x,point.x);
                    area.top    = __min(m_punto_inicial.y,point.y);
                    area.right  = __max(m_punto_inicial.x,point.x);
                    area.bottom = __max(m_punto_inicial.y,point.y);
                }
                else
                {
                    // Si el area es nula, no se define area, borrando 
                    DibujaRect(m_punto_inicial, point, TRANSPARENTE); 

                }

            }
        }
    }

    return bAreaDefinida;
}

// Acciones de dibujo cuando se mueve el mouse
void ControlImagenes::OnMouseMove(CPoint& point)
{
    static CPoint punto_temp;

    if (m_numImagenes > 0)
    {
        if (m_bDefiniendoArea) //Nos aseguramos, podria ser que se hubiese formado el area al pulsar
        {
            // Primero calculamos el punto adecuado segun el factor de zoom
            AplicaZoom(point);

            if (m_bPintandoRect)
            {
                DibujaRect(m_punto_inicial, punto_temp, TRANSPARENTE);
            }

            punto_temp = point;
            m_bPintandoRect = true;
            DibujaRect(m_punto_inicial, point, COLOR_SELECCIONADO);
        }
    }
}

// Borra el rectangulo 'rect'
void ControlImagenes::BorraRect(CRect& rect)
{
    CPoint p1,p2;

    p1.x = rect.left;
    p1.y = rect.top;
    p2.x = rect.right;
    p2.y = rect.bottom;

    DibujaRect(p1, p2, TRANSPARENTE);
}

void ControlImagenes::SeleccionaRect(CRect& rect)
{
    CPoint p1,p2;

    p1.x = rect.left;
    p1.y = rect.top;
    p2.x = rect.right;
    p2.y = rect.bottom;

    DibujaRect(p1, p2, COLOR_SELECCIONADO);
}

void ControlImagenes::RedibujaRect(CRect& rect, int nColorIndex)
{
    CPoint p1,p2;

    p1.x = rect.left;
    p1.y = rect.top;
    p2.x = rect.right;
    p2.y = rect.bottom;

    DibujaRect(p1, p2, m_listaColor[nColorIndex]);
}

// Muestra un punto en el overlay teniendo en cuenta el factor de zoom
void ControlImagenes::DibujaPunto(CPoint point, double color)
{
    muestra_punto(point.x,point.y, 3, color,0);
//    unsigned __int8 TransparentColor;
//    MdispInquire(m_Mildisplay, M_TRANSPARENT_COLOR, &TransparentColor);
///*pruebas MIL 8*/ MdispControl(m_Mildisplay, M_TRANSPARENT_COLOR, TRANSPARENTE);
//    muestra_punto(point.x,point.y, 3, TransparentColor,0);
}

// Muestra un rectangulo en el overlay teniendo en cuenta el factor de zoom
void ControlImagenes::DibujaRect(CPoint& point_inicial, CPoint& point_final, double color)
{
//		MdispOverlayKey(m_Mildisplay, M_KEY_ON_COLOR, M_EQUAL, 0xff, TRANSPARENTE);
//MdispControl(m_Mildisplay, M_KEY_MODE, M_KEY_ON_COLOR);
///*pruebas MIL 8*/ MdispControl(m_Mildisplay, M_TRANSPARENT_COLOR, TRANSPARENTE);
    muestra_punto(point_inicial.x,point_inicial.y, 3, TRANSPARENTE,0); // borramos el punto inicial 

//    MbufGet2d(M_overlay_normal,pos_x,pos_y,1,1, valor_pixel);
 //   muestra_punto(point_final.x,point_final.y, 3, color,0);
	MgraColor(M_DEFAULT, color);
//	MgraControl(M_DEFAULT, M_COLOR, M_RGB888(255,0,0) & M_RED);
//	MgraControl(M_DEFAULT, M_COLOR, M_RGB888(0,255,0) & M_GREEN);
//	MgraControl(M_DEFAULT, M_COLOR, M_RGB888(0,0,255) & M_BLUE);
    //Por duplicado para que se vea cuando el factor de zoom sea negativo (display mas pequeño)
    MgraRect(M_DEFAULT, M_overlay_normal, point_inicial.x, point_inicial.y, point_final.x, point_final.y);
    MgraRect(M_DEFAULT, M_overlay_normal, point_inicial.x+1, point_inicial.y+1, point_final.x+1, point_final.y+1);
/*
    MgraRect(M_DEFAULT, M_overlay_normal, point_inicial.x-1, point_inicial.y-1, point_final.x-1, point_final.y-1);
    MgraRect(M_DEFAULT, M_overlay_normal, point_inicial.x+2, point_inicial.y+2, point_final.x+2, point_final.y+2);
    MgraRect(M_DEFAULT, M_overlay_normal, point_inicial.x-2, point_inicial.y-2, point_final.x-2, point_final.y-2);
 */   
}





///////////////////////////////////////////////////////////////////////////////////////////////////
//
// IMAGENES
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Se reserva memoria, se carga y se presenta la imagen "nomFich"
// Se presupone que no hay otra imagen cargada con el mismo numero de filtro ("numFiltro")
//  m_control es el control al dialogo
//  bMostrar indica si debe de mostrarse la imagen en pantalla o solo cargar la imagen en memoria
// Se inicializa M_overlay_normal
// Se deja el zoom a 0.5
bool ControlImagenes::CargarImagen(char* nomFich, CStatic&	m_control, int numFiltro, bool bMostrar)
{

	// Comprobaciones
	struct stat stFileInfo;
	if (stat(nomFich,&stFileInfo) != 0)
		return false;
    if (m_Milimagen[numFiltro-1] != NULL)
        return false;

    long lSizeX, lSizeY;
    MbufDiskInquire(nomFich,M_SIZE_X, &lSizeX);
    MbufDiskInquire(nomFich,M_SIZE_Y, &lSizeY);
    MbufAlloc2d(M_DEFAULT_HOST, lSizeX,lSizeY, 16 + M_UNSIGNED, M_IMAGE + M_DISP + M_PROC, &m_Milimagen[numFiltro-1]);
    if (MbufImport(nomFich,M_TIFF,M_LOAD,M_NULL,&m_Milimagen[numFiltro-1]) == M_NULL)
//    if (MbufImport(nomFich,M_TIFF,M_RESTORE,M_DEFAULT_HOST,&m_Milimagen[numFiltro-1]) == M_NULL)
        return false;
    if (m_Milimagen[numFiltro-1] == NULL)
        return false;

    m_numImagenes++;

	if (bMostrar)
	{
		MdispSelectWindow(m_Mildisplay,m_Milimagen[numFiltro-1],m_control);
		// Inicializar Overlay la primera vez que se carga una imagen
		// Esto se hace aqui porque no funciona si no hay un buffer asociado con el display
		// ha de usarse la variable global M_overlay_normal para que funcionen las funciones graficas
		if (M_overlay_normal == M_NULL)
		{
			MdispInquire(m_Mildisplay, M_OVERLAY_ID, &M_overlay_normal);

			// Relleno el buffer overlay con el color TRANSPARENTE
//			MbufClear(M_overlay_normal, TRANSPARENTE );
		}
        // TRUCO para evitar que los rectangulos se pinten con un reborde negro. Pero la carga de imagenes es mas lenta
        // Se deja el zoom a 0.5
        // Los rectangulos se pintan con un reborde negro cuando el zoom esta entre 0.5 y 1 en MIL 8
        // por razones desconocidas. Este truco funciona, pero tampoco se sabe porque
//        MdispZoom(m_Mildisplay, 0.5, 0.5);
        //En la funcion llamante, se debe actualizar correctamente el zoom
	}
    // Solo una vez cada vez que se cargan todas las imagenes
    if (numFiltro == 2)
    {
        // Recuperamos las dimensiones de la imagen
        MbufInquire(m_Milimagen[1],M_SIZE_X,&m_nAnchoImagen);
        MbufInquire(m_Milimagen[1],M_SIZE_Y,&m_nAltoImagen);
	}

    return true;
}

void ControlImagenes::EliminarTodas()
{
    if (M_overlay_normal != M_NULL)
    {
        MbufClear(M_overlay_normal, TRANSPARENTE );// Relleno el buffer overlay con el color TRANSPARENTE
        M_overlay_normal = M_NULL;
    }


    MIL_ID selected;
    MdispInquire(m_Mildisplay,M_SELECTED,&selected);
    if (selected!=M_NULL)
        MdispDeselect(m_Mildisplay,selected);

    for (int i=0;i<MAX_NUM_IMAGENES;i++)
    {
        if (m_Milimagen[i] != M_NULL)
        {
            MbufFree(m_Milimagen[i]);
            m_Milimagen[i] = M_NULL;
        }
    }
    m_numImagenes = 0;

    //Liberamos RGB
    if (m_MilRGB != M_NULL)
    {
        MbufFree(m_MilRGB);
        m_MilRGB = M_NULL;
    }

    //Liberamos imagen identificacion
    if (M_Clasificacion != M_NULL)
    {
        MbufFree(M_Clasificacion);
        M_Clasificacion = M_NULL;
    }
    if (m_bufClasificacionSelectiva != NULL)
    {
        delete [] m_bufClasificacionSelectiva;
        m_bufClasificacionSelectiva = NULL;
    }
    theApp.clasificacion.Liberar();
}

// Se elimina la imagen correspondiente al filtro "numFiltro", liberando la memoria del buffer
// Se muestra la imagen correspondiente al primer filtro anterior o posterior disponible
// Devuelve el nuevo filtro que se muestra o -1 si no hay mas
int ControlImagenes::EliminarImagen(int numFiltro, CStatic&	m_control)
{
    int filtroAnterior = EncontrarFiltroAnterior(numFiltro);

    if (filtroAnterior == -1)
    {
        // No hay ningun filtro
        MdispDeselect(m_Mildisplay,m_Milimagen[numFiltro-1]);
        m_control.SetBitmap((HBITMAP)m_backgound_bitmap); 

        //al quitar el fondo, se queda lo ultimo (es decir m_backgound_bitmap)
        //y ademas se veran las imagenes MIL cuando haya
        m_control.SetBitmap(NULL); 
    }
    else
    {
        MdispSelectWindow(m_Mildisplay,m_Milimagen[filtroAnterior-1],m_control);
    }
    MbufFree(m_Milimagen[numFiltro-1]);
    m_Milimagen[numFiltro-1] = M_NULL;
    m_numImagenes--; 

    return filtroAnterior;
}

// Se presenta la imagen (previamente cargada) correspondiente al numero de Filtro "numFIltro"
bool ControlImagenes::MostrarImagen(int numFiltro, CStatic&	m_control)
{

    MIL_ID selected;
    MdispInquire(m_Mildisplay,M_SELECTED,&selected);
    if (selected!=M_NULL)
        MdispDeselect(m_Mildisplay,selected);

    DisplayBits(theApp.m_nBitsProfundidad); //para que el display muestre las imagenes de la profundidad de la imagen (puede que no estuviese si se estaba presentando la imagen de clasificacion)

    MdispSelectWindow(m_Mildisplay,m_Milimagen[numFiltro-1],m_control);
    return true;

}

// Obtiene el filtro anterior al dado que tiene una imagen valida (es decir, que esta cargado)
// Si no lo encuentra anterior, lo busca posterior
// Devuelve -1 si no hay otro filtro valido
int ControlImagenes::EncontrarFiltroAnterior(int numFiltro)
{
    int filtro = -1;
    int i = numFiltro - 1;

    // buscar anterior
    while (i>0 && filtro == -1)
    {
        if (m_Milimagen[i-1] != M_NULL)
            filtro = i;
        i--;
    }
    i = numFiltro + 1;
    // buscar posterior si no se encontro anterior
    while (i <= MAX_NUM_IMAGENES && filtro == -1)
    {
        if (m_Milimagen[i-1] != M_NULL)
            filtro = i;
        i++;        
    }

    return filtro;
}

// Calcula el histograma de la region 'area' del filtro 'numFiltro', devolviendo los valores en 'histo', 'media' y 'sigma'
// El histograma se devuelve con NUM_VALORES_HISTO valores independientemente de la profundidad de la imagen 
// nBitsImagen - profundidad en bits de las imagenes. Hay que tener esto en cuenta para transformar histograma a NUM_VALORES_HISTO
// Devuelve false si la imagen correspondiente no esta disponible
bool ControlImagenes::CalculaHistograma(int numFiltro, const CRect& area, int nBitsImagen, long* histo, double& moda, double& sigma, int &nPercentilInf, int &nPercentilSup)
{
    MIL_ID  M_AreaAux;
    double media;

    int nValorMaxImagen = 1<<nBitsImagen;
    long * histoImagen = new long [nValorMaxImagen]; //histograma auxiliar con TODOS los valores de gris de la imagen

    memset(histo,0,NUM_VALORES_HISTO*sizeof(long)); //inicializamos

    if (m_Milimagen[numFiltro-1] != M_NULL)
    {
	    MbufChild2d(m_Milimagen[numFiltro-1], area.left, area.top,area.right - area.left, area.bottom - area.top, &M_AreaAux);
        if (EvaluaHistograma(M_AreaAux,histoImagen, nValorMaxImagen))
        {
            MbufFree(M_AreaAux);
            // calcular histograma con NUM_VALORES_HISTO
            if (NUM_VALORES_HISTO < nValorMaxImagen)
            {
                int nValoresAgrupar = nValorMaxImagen / NUM_VALORES_HISTO; //numero de valores a sumar para agrupar 
                for (int i = 0; i<NUM_VALORES_HISTO; i++)
                {
                    for (int j=0;j<nValoresAgrupar;j++)
                        histo[i] += histoImagen[i*nValoresAgrupar + j];
                }
            }
            else
            {
                memcpy(histo,histoImagen,nValorMaxImagen*sizeof(long));
            }
            delete [] histoImagen;

            procesa_histograma(histo,&media,&sigma);
            moda = moda_histograma(histo, 0, NUM_VALORES_HISTO-1);
            nPercentilInf = percentil_histograma(histo,NUM_VALORES_HISTO,5);
            nPercentilSup = percentil_histograma(histo,NUM_VALORES_HISTO,95);
            return true;
        }
        else
        {
            MbufFree(M_AreaAux);
            delete [] histoImagen;
            return false;
        }
    }
    else
    {
        delete [] histoImagen;
        return false;
    }
}

bool  ControlImagenes::EvaluaHistograma(MIL_ID buffer, long *histo, int nValoresHisto)
{
    MIL_ID M_histograma;

	if ( buffer == M_NULL) 
		return false;

	MimAllocResult(M_DEFAULT_HOST , nValoresHisto, M_HIST_LIST, &M_histograma);

	//	Calculamos el histograma de toda la imagen
	MimHistogram(buffer, M_histograma);

	// Obtengo los resultados, almacenándolos en el array: histograma
	MimGetResult(M_histograma, M_VALUE, histo);

    // Libero el buffer de resultados del histograma
	MimFree(M_histograma);

    return true;
}

// Obtiene los valores en niveles de gris de la zona de la imagen especificada
void  ControlImagenes::GetEspectroArea(int nOffsetX,int nOffsetY, int nSizeX, int nSizeY, unsigned short** arrEspectroArea)
{
    for (int i=1;i<theApp.m_ParamIni.nBandas;i++) //pancromatico no
    {
        MbufGet2d(m_Milimagen[i],nOffsetX,nOffsetY,nSizeX, nSizeY, arrEspectroArea[i-1]);
    }
}

// Configura el display MIL de tal manera que se muestren las imagenes con la profundidad dada
void ControlImagenes::DisplayBits(int nBitsProfundidad)
{
    if (nBitsProfundidad == 8)
    {
        MdispControl(m_Mildisplay,M_VIEW_MODE,M_TRANSPARENT);
    }
    else if (nBitsProfundidad == 12)
    {
        MdispControl(m_Mildisplay,M_VIEW_MODE,M_BIT_SHIFT);
        MdispControl(m_Mildisplay,M_VIEW_BIT_SHIFT,4); //despreciamos los 4 bits menos significativos
//	    MdispLut(m_Mildisplay, M_PSEUDO); //esto es necesario para que la intensidad no sea anormalmente elevada
    }
    else if (nBitsProfundidad == 16)
    {
        MdispControl(m_Mildisplay,M_VIEW_MODE,M_BIT_SHIFT);
        MdispControl(m_Mildisplay,M_VIEW_BIT_SHIFT,8); //despreciamos los 4 bits menos significativos
//	    MdispLut(m_Mildisplay, M_PSEUDO); //esto es necesario para que la intensidad no sea anormalmente elevada
    }
}




///////////////////////////////////////////////////////////////////////////////////////////////////
//
// RGB
//
///////////////////////////////////////////////////////////////////////////////////////////////////

// Calcula las imagenes monocromas para R, G y B segun los porcentajes dados, 
// reservando memoria y presentando la imagen RGB
// Devuelve true si se ha realizado el calculo, false si no (porque ya se realizó antes)
bool ControlImagenes::MostrarImagenRGB(int arrFiltroPorcentajesR[MAX_NUM_IMAGENES], 
                                       int arrFiltroPorcentajesG[MAX_NUM_IMAGENES], 
                                       int arrFiltroPorcentajesB[MAX_NUM_IMAGENES],
                                       CStatic&	m_control,
                                       int nTotalR,int nTotalG,int nTotalB,
                                       bool bModificado)
{

    MIL_ID selected;
    MdispInquire(m_Mildisplay,M_SELECTED,&selected);
    if (selected!=M_NULL)
        MdispDeselect(m_Mildisplay,selected);

    DisplayBits(8); //para que el display muestre las imagenes en profundidad 8 bit

    // Si es la primera vez, reservar la memoria de la imagen RGB
    if (m_MilRGB == M_NULL)
        MbufAllocColor(M_DEFAULT_HOST, 3, m_nAnchoImagen, m_nAltoImagen, 8+M_UNSIGNED, M_IMAGE+M_DISP, &m_MilRGB); //16 por si las imagenes estan en 16bit
    else if (!bModificado)
    {
        MdispSelectWindow(m_Mildisplay,m_MilRGB,m_control);
        return false;
    }


    unsigned short* pBuffImagen;    //auxiliar para volcar cada imagen de cada filtro, 16 bit
    unsigned char* pRed;           // auxiliar para calcular la banda de color (8 bit)
    unsigned char* pGreen;         // auxiliar para calcular la banda de color (8 bit)
    unsigned char* pBlue;          // auxiliar para calcular la banda de color (8 bit)

    pBuffImagen = (unsigned short*)calloc(m_nAnchoImagen*m_nAltoImagen, sizeof(unsigned short));
    pRed = (unsigned char*)calloc(m_nAnchoImagen*m_nAltoImagen, sizeof(unsigned char));
    pGreen = (unsigned char*)calloc(m_nAnchoImagen*m_nAltoImagen, sizeof(unsigned char));
    pBlue = (unsigned char*)calloc(m_nAnchoImagen*m_nAltoImagen, sizeof(unsigned char));

    for (int i= 0; i<MAX_NUM_IMAGENES;i++)
    {
        // Cargar buffers de filtros solo si se necesita en alguna de las bandas
        if (arrFiltroPorcentajesR[i] != 0 || arrFiltroPorcentajesG[i] != 0 || arrFiltroPorcentajesB[i] != 0)
        {
            MbufGet(m_Milimagen[i],pBuffImagen);

            //Acumular el pocentaje adecuado de este filtro en cada color
            if(arrFiltroPorcentajesR[i] != 0)
            {
                Acumular(pRed, pBuffImagen, ((double)arrFiltroPorcentajesR[i])/nTotalR, m_nAnchoImagen*m_nAltoImagen);
            }
            if(arrFiltroPorcentajesG[i] != 0)
            {
                Acumular(pGreen, pBuffImagen, ((double)arrFiltroPorcentajesG[i])/nTotalG, m_nAnchoImagen*m_nAltoImagen);
            }
            if(arrFiltroPorcentajesB[i] != 0)
            {
                Acumular(pBlue, pBuffImagen, ((double)arrFiltroPorcentajesB[i])/nTotalB, m_nAnchoImagen*m_nAltoImagen);
            }

        }
    }


    // Volcar el array auxiliar calculado al color correspondiente
    MbufPutColor(m_MilRGB,M_SINGLE_BAND,M_BLUE/*2*/,pBlue);
    MbufPutColor(m_MilRGB,M_SINGLE_BAND,M_GREEN/*1*/,pGreen);
    MbufPutColor(m_MilRGB,M_SINGLE_BAND,M_RED/*0*/,pRed);

    free(pBuffImagen);
    free(pBlue);
    free(pGreen);
    free(pRed);

    // Mostar la imagen en color
    MdispSelectWindow(m_Mildisplay,m_MilRGB,m_control);
	// Inicializar Overlay la primera vez que se carga una imagen
	// Esto se hace aqui porque no funciona si no hay un buffer asociado con el display
	// ha de usarse la variable global M_overlay_normal para que funcionen las funciones graficas
    // y porque puede ser que no se haya hecho al cargar las imagenes pancromaticas (en el caso
    // de que la primera imagen mostrada sea la pancromatica, ver CAnalisisDlg::AbrirTodas y CargarImagen)
	if (M_overlay_normal==NULL)
	{
		MdispInquire(m_Mildisplay, M_OVERLAY_ID, &M_overlay_normal);

		// Relleno el buffer overlay con el color TRANSPARENTE
		MbufClear(M_overlay_normal, TRANSPARENTE );
	}

    return true;
}

//Acumulamos en el buffer de cada banda el porcentaje de esta banda dado
void ControlImagenes::Acumular(unsigned char* pAcumulado, unsigned short* pTotal, double dPorcentaje, int nTamBuffers)
{
    // Las imagenes pueden estar en 8, 12 o 16 bit, pero la imagen RGB resultante estará siempre en 8 bit, por 
    // tanto hay que convertir cada acumulacion a 8 bit 
    // De esta manera el resultado total de cada banda estará en 8 bit
    double dMultiplicador = dPorcentaje / (1<<(theApp.m_nBitsProfundidad-8));

    for (int i = 0;i<nTamBuffers;i++)
        pAcumulado[i] = pAcumulado[i] + (unsigned short) floor(pTotal[i]*dMultiplicador);
}

// guarda la imagen rgb en el fichero dado
void ControlImagenes::GuardarRGB(CString csNombreFichero)
{
    MbufExport((char*)(LPCTSTR)csNombreFichero,M_TIFF,m_MilRGB);
}




///////////////////////////////////////////////////////////////////////////////////////////////////
//
//CLASIFICACION
//
///////////////////////////////////////////////////////////////////////////////////////////////////

BYTE* ControlImagenes::GetBufClasificacionSelectiva()
{
    return m_bufClasificacionSelectiva;
}

// interfaz con Clasificacion para clasificar la imagen m_Milimagen (ya cargada y perteneciente a ControlImagen)
// devuelve false, si ha habido un error o si ya se habia clasificado y unicamente se ha mostrado la imagen ya clasificada
bool ControlImagenes::Clasificar(CStatic*	pDisplay, list<CMineralClasificado> &listClasificados, unsigned int& nCountOscuros, CRect* pRect, double dRefMin)
{
    if (M_Clasificacion == M_NULL) // hace falta clasificar de nuevo, no se hizo con anterioridad
        if (!theApp.clasificacion.CargarImagenes(&m_Milimagen[1],m_numImagenes,pRect)) // nos saltamos la banda pancromatica (no usada)
            return false;

    if (pDisplay!=NULL)
    {
        MIL_ID selected;
        MdispInquire(m_Mildisplay,M_SELECTED,&selected);
        if (selected!=M_NULL)
            MdispDeselect(m_Mildisplay,selected);

        DisplayBits(8); //para que el display muestre las imagenes de la profundidad leida
        if (!theApp.clasificacion.InicializaDisplay(pDisplay,m_Mildisplay,M_Clasificacion,M_Confiabilidad,M_Distancia))
        {
           ASSERT(m_bufClasificacionSelectiva != NULL);
           return false;
        }

        if (m_bufClasificacionSelectiva == NULL)
           m_bufClasificacionSelectiva = new BYTE[m_nAnchoImagen*m_nAltoImagen];//buffer para la imagen de salida (clasificada selectivamente) 
    }

    if (!theApp.clasificacion.Clasificar(listClasificados,nCountOscuros,theApp.m_dEscalaReflectancia, theApp.m_nBitsProfundidad, dRefMin))
        return false;

    return true;
}

void ControlImagenes::Confiabilidad(CStatic&	m_control)
{
    MIL_ID selected;
    MdispInquire(m_Mildisplay,M_SELECTED,&selected);
    if (selected!=M_NULL)
        MdispDeselect(m_Mildisplay,selected);

    DisplayBits(8); //para que el display muestre las imagenes en profundidad 8 bit
    MdispSelectWindow(m_Mildisplay, M_Confiabilidad, m_control);
}
void ControlImagenes::Distancia(CStatic&	m_control)
{
    MIL_ID selected;
    MdispInquire(m_Mildisplay,M_SELECTED,&selected);
    if (selected!=M_NULL)
        MdispDeselect(m_Mildisplay,selected);

    MdispSelectWindow(m_Mildisplay, M_Distancia, m_control);
}

// Mostramos la imagen clasificada selectivamente
void ControlImagenes::ClasificacionSelectivaMostrar()
{
    MbufPut2d(M_Clasificacion, 0, 0, m_nAnchoImagen, m_nAltoImagen, m_bufClasificacionSelectiva); 
}


