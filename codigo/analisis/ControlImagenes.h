#if !defined(CONTROL_IMAGENES_H)
#define CONTROL_IMAGENES_H

#include "defines.h"
#include "mil.h"

#define COLOR_SELECCIONADO  M_COLOR_WHITE

class CAnalisisDlg;
class ControlImagenes
{
//    MIL_ID m_Milsistema;
    MIL_ID m_Milaplicacion;
//    MIL_ID M_sistema; //pruebas MIL 8
//    MIL_ID M_graphics; //pruebas MIL 8
    MIL_ID m_Mildisplay;
    MIL_ID m_Miloverlay;
    MIL_ID m_lut_overlay;
    MIL_ID m_Milimagen[MAX_NUM_IMAGENES];
    MIL_ID m_MilRGB;
    int    m_numImagenes;
    double m_factorZoom;        //factor zoom necesario para que la imagen quepa en la ventana (manteniendo aspecto)
    int     m_nAnchoImagen; 
    int     m_nAltoImagen;

    CBitmap m_backgound_bitmap; //para poner siempre que no haya mas imagenes
    
    CPoint m_punto_inicial; //punto inicial del area que se esta seleccionando
    bool m_bDefiniendoArea; //indica si se esta en proceso de seleccionar un area
    bool m_bPintandoRect;   // para saber si borrar o no el rectangulo previamente pintado (ver OnMove)
    int  m_listaColor[NUM_COLORES]; //colores especiales MIL para representar areas
    // CLASIFICACION
    MIL_ID M_Clasificacion;
BYTE* m_bufClasificacionSelectiva; //buffer para la imagen clasificada selectivamente
MIL_ID M_Confiabilidad;
MIL_ID M_Distancia;

public:
    ControlImagenes();
    virtual ~ControlImagenes();

    double ActualizaZoom(CButton& m_control_group, CStatic&	m_control);

    bool CargarImagen(char* nomFich, CStatic&	m_control, int numFiltro, bool bMostrar);
    bool MostrarImagen(int numFiltro, CStatic&	m_control);
    int  EliminarImagen(int numFiltro, CStatic&	m_control);
    void EliminarTodas();
    bool CalculaHistograma(int numFiltro, const CRect& area, int nBitsImagen,  long* histo, double& media, double& sigma, int &nPercentilInf, int &nPercentilSup);
    void GetEspectroArea(int nOffsetX,int nOffsetY, int nSizeX, int nSizeY, unsigned short** arrEspectroArea);
    void ControlImagenes::DisplayBits(int nBitsProfundidad);

    bool OnLButtonDown(CPoint& point,CRect& area, int nColorIndex);
    bool OnLButtonUp(CPoint& point,CRect& area, int nColorIndex);
    void OnMouseMove(CPoint& point);
    void BorraRect(CRect& rect);
    void SeleccionaRect(CRect& rect);
    void RedibujaRect(CRect& rect, int nColorIndex);

    bool MostrarImagenRGB(int arrFiltroPorcentajesR[MAX_NUM_IMAGENES], 
                                       int arrFiltroPorcentajesG[MAX_NUM_IMAGENES], 
                                       int arrFiltroPorcentajesB[MAX_NUM_IMAGENES],
                                       CStatic&	m_control,
                                       int nTotalR,int nTotalG,int nTotalB,
                                       bool bModificado);
    void GuardarRGB(CString csNombreFichero);

    int GetAnchoImagen();
    int GetAltoImagen();
    void SetAnchoImagen(int nAncho);
    void SetAltoImagen(int nAlto);
    double GetFactorZoom();
    bool Clasificar(CStatic*	pDisplay, list<CMineralClasificado> &listClasificados, unsigned int& nCountOscuros, CRect* pRect, double dRefMin = -1);
BYTE* GetBufClasificacionSelectiva();
void Confiabilidad(CStatic&	m_control);
void Distancia(CStatic&	m_control);
void ClasificacionSelectivaMostrar();

private:
    int EncontrarFiltroAnterior(int numFiltro);
    void Acumular(unsigned char* pAcumulado, unsigned short* pTotal, double dPorcentaje, int nTamBuffers);
    void CalculaZoom(long anchoDisplay,long altoDisplay,
        long anchoIm,long altoIm);
    void AplicaZoom(CPoint& point);
    void DibujaPunto(CPoint point, double color);
    void DibujaRect(CPoint& point_inicial, CPoint& point_final, double color);
    bool EvaluaHistograma(MIL_ID buffer, long *histo, int nValoresHisto);

};

#endif // !defined(CONTROL_IMAGENES_H)
