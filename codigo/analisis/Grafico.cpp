// Grafico.cpp: implementation of the CGrafico class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "analisis.h"
#include "Grafico.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CGrafico::CGrafico()
{
    m_parentWindow      = NULL;
    m_pGraph            = NULL;
    m_nPoints           = 0;
    m_pY_Dummy          = NULL;
    m_id_Dummy          = 0;     
    m_id_LineaVertical = 0;

    m_pGraph = new CPGLGraph;
    ASSERT_VALID(m_pGraph);
}

CGrafico::~CGrafico()
{
    if (m_id_Dummy != 0)
        EliminarLinea(m_id_Dummy);
    if (m_id_LineaVertical != 0)
    {
        EliminarLineaVertical();
        m_id_LineaVertical = 0;
    }
    delete m_pGraph;
}

void CGrafico::Inicializar(CWnd* parentWindow, int nPoints, int nY_Max, int nX_Min, double dX_Step, char* csTitle, char* csLabelX, char* csLabelY )
{
    m_parentWindow = parentWindow;
    m_nPoints           = nPoints;
    m_nX_Min            = nX_Min;
    m_dX_Step           = dX_Step;

    // Get a pointer the axis object
    CPGLAxe2D* pAxis = m_pGraph->GetAxe();

    // Change the title text and color
    pAxis->GetTitle()->Show(TRUE);
    pAxis->SetTitle(csTitle);
    pAxis->GetTitle()->SetColor(0 /* red */,
                                1.0f /* green */,
                                0 /* blue*/ /* alpha optional */);

    pAxis->SetShowGrid(1,FALSE); //Turn off horizontal grid, (vertical -> 0, horizontal -> 1) 
    if (nPoints <= NUM_BANDAS) // grafico de espectros
    {
        pAxis->SetGridStep(0,m_dX_Step); //(vertical -> 0, horizontal -> 1) 
        pAxis->SetAutoSteps(0,FALSE); //(vertical -> 0, horizontal -> 1) 
        pAxis->SetBottomSecondTicksNb(0);
    }

    // Show and change labels
    pAxis->GetRightLabel()->Show(FALSE);
    pAxis->GetLeftLabel()->Show(TRUE);
    pAxis->GetLeftLabel()->SetString(csLabelY);

    pAxis->GetTopLabel()->Show(FALSE);
    pAxis->GetBottomLabel()->Show(TRUE);
    pAxis->GetBottomLabel()->SetString(csLabelX);

    pAxis->SetTopSecondTicksNb(0);

    // Añadimos una linea transparente para que se dimensione el grafico correctamente
    m_pY_Dummy                = new double[m_nPoints];
    for (UINT i=0;i< m_nPoints-1;i++)
    {
	    m_pY_Dummy[i]=0;
    }
    m_pY_Dummy[m_nPoints-1] = nY_Max;
    m_id_Dummy = NuevaLinea(m_pY_Dummy,0,0,0);
    //hacer transparente linea dummy
    CPGLLine2D* pLine = (CPGLLine2D*)m_pGraph->FindObject(m_id_Dummy);
    ASSERT(pLine->CheckID(m_id_Dummy));
    pLine->SetAlpha(0.0); 
}

// Dibuja el grafico (m_pGraph) dentro de la ventana padre m_parentWindow
void CGrafico::MuestraGrafico()
{
    CPaintDC* ppaintDC;
    ppaintDC = new CPaintDC(m_parentWindow);


    Gdiplus::Graphics graphics(ppaintDC->m_hDC);

	// setting BoundingBox 
    CRect rect;
    m_parentWindow->GetClientRect(&rect);

    m_GDIpInterface.SetMainBoundingBox(rect);
    ASSERT(m_pGraph); 
	m_pGraph->SetViewport(rect.left,rect.top,rect.Width(),rect.Height());
	m_pGraph->ZoomAll(TRUE);


    Gdiplus::Bitmap MemBitmap(m_GDIpInterface.GetMainBBWidth(), m_GDIpInterface.GetMainBBHeight());

    Gdiplus::Graphics* pMemGraphics;
    pMemGraphics = Gdiplus::Graphics::FromImage(&MemBitmap); 

    m_GDIpInterface.SetGraphics(pMemGraphics);

    m_GDIpInterface.BeginPaint();

    pMemGraphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
    //pMemGraphics->SetTextRenderingHint(Gdiplus::TextRenderingHintSystemDefault);
    //pMemGraphics->SetInterpolationMode(Gdiplus::InterpolationModeDefault);

    // Here you pass the context to the PGL library
    m_pGraph->PlotGfx(m_GDIpInterface);

    // We're done
    m_GDIpInterface.EndPaint();

    // Draw the image onto the screen (eqv. to BitBlt)
    graphics.DrawImage(&MemBitmap, 0,0);//100,100,500,500);

    // Clean up 
    delete pMemGraphics;
    delete ppaintDC;
}

// arrValores debe estar ya reservado en memoria y debe existir mientras exista la linea
UINT CGrafico::NuevaLinea(double arrValores[],int r, int g, int b)
{
    CPGLLine2D* pLine = new CPGLLine2D;

    // Se necesita crear un array del eje X tambien porque al borrar la linea, se borran ambos arrays
    double* m_pX                = new double[m_nPoints];
    for (UINT i=0;i< m_nPoints;i++)
    {
	    m_pX[i]=m_nX_Min+i*m_dX_Step;
    }
    pLine->SetDatas(    m_nPoints ,  // number of points
                        m_pX      ,  // x(i) 
                        arrValores        ); // y(i) 
    pLine->SetLineWidth(LINE_WIDTH);
    pLine->SetColor((double)r/256,(double)g/256,(double)b/256);
    m_pGraph->AddObject(pLine);

    return pLine->GetID();

}

bool CGrafico::EliminarLinea(UINT nId)
{
    CPGLLine2D* pLine = (CPGLLine2D*)m_pGraph->FindObject(nId);
    m_pGraph->RemoveObject(nId); // no borra las lineas asociadas
    ASSERT(pLine->CheckID(nId));
    if (pLine!=NULL)
    {
        delete pLine; //borra tambien el array asociado
    }

    return (pLine != NULL);
}

// dX debe estar ya reservado en memoria y debe existir mientras exista la linea
void CGrafico::NuevaLineaVertical(double dX,int r, int g, int b)
{
    if (dX >= m_nX_Min && dX <= m_nX_Min + m_dX_Step*m_nPoints)
    {
        // si ya habia una linea vertical, borrarla
        if (m_id_LineaVertical != 0)
        {
            EliminarLineaVertical();
            m_id_LineaVertical = 0;
        }

        // crear otra
        CPGLLineVer* pLine = new CPGLLineVer;

        pLine->SetX(dX);
        pLine->SetLineWidth(LINE_WIDTH_VER);
        pLine->SetColor((double)r/256,(double)g/256,(double)b/256);
        m_pGraph->AddObject(pLine);

        m_id_LineaVertical = pLine->GetID();
    }
}

void CGrafico::EliminarLineaVertical()
{
    CPGLLineVer* pLine = (CPGLLineVer*)m_pGraph->FindObject(m_id_LineaVertical);
    if (pLine!=NULL)
    {
        m_pGraph->RemoveObject(m_id_LineaVertical); // no borra las lineas asociadas
        ASSERT(pLine->CheckID(m_id_LineaVertical));
        delete pLine; //borra tambien el array asociado
    }
}

// Borra y libera todas las lineas
void CGrafico::EliminarLineas()
{
    m_pGraph->DeleteAllExcept(m_id_Dummy); // libera las lineas asociadas
}

void CGrafico::MostrarLinea(UINT nId, bool bMostrar)
{
    CPGLLine2D* pLine = (CPGLLine2D*)m_pGraph->FindObject(nId);
    ASSERT(pLine->CheckID(nId));
    if (bMostrar == true)
        pLine->Show(TRUE);
    else
        pLine->Show(FALSE);
}

void CGrafico::MostrarTodas(bool bMostrar)
{
    if (bMostrar)
        m_pGraph->UnhideAll();
    else
        m_pGraph->HideAll();
}

