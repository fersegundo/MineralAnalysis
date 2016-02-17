
#include "stdafx.h"
#include "TomaAuto.h"
#include "Progreso.h"
#include "..\librerias\include\procesoAux.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

extern CTomaAutoApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CProgreso::CProgreso()
{
    M_Progreso_child = NULL;
    // Creamos el buffer maximo (padre) que se irá construyendo y se mostrará en la pantalla de progreso
	MbufAlloc2d(M_sistema, DIMENSION_PROGRESO, DIMENSION_PROGRESO, 8+M_UNSIGNED,
		M_IMAGE+M_PROC+M_DISP, &M_Progreso);
    //Creamos el display
	MdispAlloc(M_sistema, M_DEFAULT, "M_DEFAULT", M_WINDOWED, &M_Display_progreso);//M_DISPLAY_SETUP
}

CProgreso::~CProgreso()
{
    MbufFree(M_Progreso);
}

void CProgreso::Borrar()
{
    MbufClear(M_Progreso,64);
}

void CProgreso::Inicializar(int nCamposX, int nCamposY,CStatic& m_progreso)
{
    MbufClear(M_Progreso,64);

    // calculamos las dimensiones que tendran cada campo de progreso teniendo en cuenta
    if (nCamposX*theApp.m_configuracion.m_nDimension_barrido_x >= nCamposY*theApp.m_configuracion.m_nDimension_barrido_y)
    {
        m_nDimensionCampoX = (int)floor(DIMENSION_PROGRESO/nCamposX);
        m_nDimensionCampoY = (int)(m_nDimensionCampoX * theApp.m_configuracion.m_nDimension_barrido_y/theApp.m_configuracion.m_nDimension_barrido_x + 0.5);
    }
    else
    {
        m_nDimensionCampoY = (int)floor(DIMENSION_PROGRESO/nCamposY);
        m_nDimensionCampoX = (int)(m_nDimensionCampoY * theApp.m_configuracion.m_nDimension_barrido_x/theApp.m_configuracion.m_nDimension_barrido_y + 0.5);
    }
    //Creamos un child buffer para asociar con el display con las dimensiones reales del mosaico 
    //(no tiene porque cubrir todo el buffer padre si el barrido o la adquisicion no son cuadradradas)
    if (M_Progreso_child!=NULL) // Codigo defensivo
        MbufFree(M_Progreso_child);
    MbufChild2d(M_Progreso,0,0,m_nDimensionCampoX*nCamposX, m_nDimensionCampoY*nCamposY,&M_Progreso_child);

    MdispControl(M_Display_progreso, M_FILL_DISPLAY, M_ENABLE);
//    MdispSelectWindow(M_Display_progreso,M_Progreso_child,m_progreso); //HACER: o poner lo de abajo (y se quedaria el fondo negro o redimensionar el display
    MdispSelectWindow(M_Display_progreso,M_Progreso,m_progreso);
}

void CProgreso::Fin()
{
//    MbufClear(M_Progreso,0);
//    MdispSelect(M_display_progreso,M_NULL);
    if (M_Progreso_child!=NULL)
    {
        MbufFree(M_Progreso_child);
        M_Progreso_child = NULL;
    }
}

//Añade una imagen reducida RGB en la ventana de progreso
//a partir de las imagenes espectrales en M_banda[]
// nCampoX, nCampoY - numero de campo en X e Y
void CProgreso::NuevoCampo(int nCampoX, int nCampoY)
{
   
    MIL_ID M_rgb;
    MbufAllocColor(M_sistema, 3, theApp.m_configuracion.m_nDimension_barrido_x, theApp.m_configuracion.m_nDimension_barrido_y, 8+M_UNSIGNED, M_IMAGE+M_PROC, &M_rgb); 

    //Componemos la imagen RGB (en tamaño normal)
    CalcularRGB(M_rgb);
    //Reducimos la imagen a tamaño de progreso
    MIL_ID M_RGB_mini;
    MbufAllocColor(M_sistema, 3, m_nDimensionCampoX, m_nDimensionCampoY, 8+M_UNSIGNED, M_IMAGE+M_PROC+M_DISP, &M_RGB_mini); 
    MimResize(M_rgb,M_RGB_mini,M_FILL_DESTINATION,M_FILL_DESTINATION,M_DEFAULT);

    MbufFree(M_rgb);

    // Añadimos la mini imagen al buffer de progreso
    MbufCopyClip(M_RGB_mini,M_Progreso_child,nCampoX*m_nDimensionCampoX,nCampoY*m_nDimensionCampoY);

    MbufFree(M_RGB_mini);
}



void CProgreso::CalcularRGB(MIL_ID M_rgb)
{
    unsigned __int16* pBuffImagen; //auxiliar para volcar cada imagen de cada filtro
    unsigned char* pRed;
    unsigned char* pGreen;
    unsigned char* pBlue;

    // Buffers auxiliares para componer luego la imagen RGB
    pBuffImagen = (unsigned __int16*)calloc(theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y, sizeof(unsigned __int16)); // 16 BIT!!
    pRed = (unsigned char*)calloc(theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y, sizeof(unsigned char));
    pGreen = (unsigned char*)calloc(theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y, sizeof(unsigned char));
    pBlue = (unsigned char*)calloc(theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y, sizeof(unsigned char));
    
    MIL_ID Mchild;
    for (int i= 1; i<7;i++)
    {
        //Recortamos las imagenes
        MbufChild2d(M_banda[i], 
            (long)(theApp.m_ParamIni.Cam.anchoImagen - theApp.m_configuracion.m_nDimension_barrido_x)/2, 
            (long)(theApp.m_ParamIni.Cam.altoImagen - theApp.m_configuracion.m_nDimension_barrido_y)/2, 
            theApp.m_configuracion.m_nDimension_barrido_x, 
            theApp.m_configuracion.m_nDimension_barrido_y, 
            &Mchild);

        MbufGet(Mchild,pBuffImagen); //16 bit
        MbufFree(Mchild);
        //Acumular el pocentaje adecuado de este filtro en cada color
        if(i == 1 || i==2)
        {
            Acumular(pBlue, pBuffImagen, 0.5, theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y);
        }
        else if(i == 3 || i==4)
        {
            Acumular(pGreen, pBuffImagen, 0.5, theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y);
        }
        else if(i == 5 || i==6)
        {
            Acumular(pRed, pBuffImagen, 0.5, theApp.m_configuracion.m_nDimension_barrido_x*theApp.m_configuracion.m_nDimension_barrido_y);
        }
    }

    // Volcar el array auxiliar calculado al color correspondiente
    MbufPutColor(M_rgb,M_SINGLE_BAND,M_BLUE/*2*/,pBlue);
    MbufPutColor(M_rgb,M_SINGLE_BAND,M_GREEN/*1*/,pGreen);
    MbufPutColor(M_rgb,M_SINGLE_BAND,M_RED/*0*/,pRed);

    free(pBuffImagen);
    free(pBlue);
    free(pGreen);
    free(pRed);
}

//AUXILIAR
void CProgreso::Acumular(unsigned char* pAcumulado, unsigned __int16* pTotal, double dPorcentaje, int nTamBuffers)
{
    for (int i = 0;i<nTamBuffers;i++)
    {

        pAcumulado[i] = pAcumulado[i] + (unsigned char) floor(dPorcentaje*pTotal[i]/256); //Conversion de 16 a 8 bit
    }
}

