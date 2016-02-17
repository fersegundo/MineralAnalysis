// Areas.cpp: implementation of the CAreas class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "Areas.h"
#include "analisisDlg.h"
#include "ControlImagenes.h"

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CAreas::CAreas()
{
    // Rellenar codigo de colores
    listaR[NEGRO_POS]           = NEGRO_R   ;
    listaG[NEGRO_POS]           = NEGRO_G   ;
    listaB[NEGRO_POS]           = NEGRO_B   ;
                                
    listaR[ROJO_POS]            = ROJO_R    ;
    listaG[ROJO_POS]            = ROJO_G    ;
    listaB[ROJO_POS]            = ROJO_B    ;
                                
    listaR[AZUL_POS]            = AZUL_R    ;
    listaG[AZUL_POS]            = AZUL_G    ;
    listaB[AZUL_POS]            = AZUL_B    ;
                                
    listaR[VERDE_POS]           = VERDE_R   ;
    listaG[VERDE_POS]           = VERDE_G   ;
    listaB[VERDE_POS]           = VERDE_B   ;
                                
    listaR[CYAN_POS]            = CYAN_R;
    listaG[CYAN_POS]            = CYAN_G;
    listaB[CYAN_POS]            = CYAN_B;
                                
    listaR[NARANJA_POS]         = NARANJA_R ;
    listaG[NARANJA_POS]         = NARANJA_G ;
    listaB[NARANJA_POS]         = NARANJA_B ;
                                
    listaR[MORADO_POS]          = MORADO_R  ;
    listaG[MORADO_POS]          = MORADO_G  ;
    listaB[MORADO_POS]          = MORADO_B  ;
                                
    listaR[GRIS_POS]            = GRIS_R    ;
    listaG[GRIS_POS]            = GRIS_G    ;
    listaB[GRIS_POS]            = GRIS_B    ;
                                
    listaR[ROSA_POS]            = ROSA_R   ;
    listaG[ROSA_POS]            = ROSA_G   ;
    listaB[ROSA_POS]            = ROSA_B   ;
                                
    listaR[ORO_POS]             = ORO_R     ;
    listaG[ORO_POS]             = ORO_G     ;
    listaB[ORO_POS]             = ORO_B     ;
                                
    listaR[MARRON_POS]          = MARRON_R  ;
    listaG[MARRON_POS]          = MARRON_G  ;
    listaB[MARRON_POS]          = MARRON_B  ;

    m_CurrentColorIndex = 0;
}

CAreas::~CAreas()
{

}

CArea* CAreas::NuevaArea(CRect& rect)
{
    CArea area;
    area.m_csNombre.Empty();
    area.m_rect = rect;
    area.m_nColorIndex = m_CurrentColorIndex;

    m_CurrentColorIndex++;
    if (m_CurrentColorIndex >= NUM_COLORES)
        m_CurrentColorIndex = 0;

    m_list.push_back(area);

    return &(m_list.back());
}
     
void CAreas::GetCurrentColor(long& r, long& g, long& b)
{
    r = listaR[m_CurrentColorIndex];
    g = listaG[m_CurrentColorIndex];
    b = listaB[m_CurrentColorIndex];
}
   
int CAreas::GetCurrentColorIndex()
{
    return m_CurrentColorIndex;
}
   
// Devuelve un puntero al objeto CArea del elemento dado por su id
CArea* CAreas::GetArea(int nId)
{
    list<CArea>::iterator it;

    for (it = m_list.begin(); it != m_list.end();it++)
    {
        if ((*it).m_nId == nId)
        {
            return &(*it);
        }
    }

    return NULL;
}
        
// Devuelve el color (r,g,b) del elemento dado por su puntero
void CAreas::GetRGB(CArea& area, long& r, long& g, long& b)
{
    r = listaR[area.m_nColorIndex];
    g = listaG[area.m_nColorIndex];
    b = listaB[area.m_nColorIndex];
}

// Devuelve el color (r,g,b) del elemento dado por su id
void CAreas::GetRGB(int nId, long& r, long& g, long& b)
{
    list<CArea>::iterator it;

    r = 0;
    g = 0;
    b = 0;

    for (it = m_list.begin(); it != m_list.end();it++)
    {
        if ((*it).m_nId == nId)
        {
            GetRGB((*it),r,g,b);
            break;
        }
    }

}
        
// Busca y elimima el area identificado por nId
// De paso, devuelve el CRect del elemento eliminado
CRect CAreas::Eliminar(int nId)
{
    CRect rect;
    rect.left = -1;
    rect.top = -1;
    rect.right = -1;
    rect.bottom = -1;

    list<CArea>::iterator it;

    for (it = m_list.begin(); it != m_list.end();it++)
    {
        if ((*it).m_nId == nId)
        {
            rect = (*it).m_rect;
            m_list.erase(it);
            break;
        }
    }

    return rect;
}

// Elimina todas las areas
void CAreas::EliminarTodas()
{
    m_list.clear();
    ASSERT(m_list.empty());

    // Reiniciamos el orden de los colores
    m_CurrentColorIndex = 0;
}

// Busca el elemento que tiene nId y le asigna el string 'pszText'
void CAreas::SetText(int nId,TCHAR* pszText)
{
    list<CArea>::iterator it;

    //Se busca de atras adelante por optimizar (normalmente se cambiará el ultimo elemento)
    if (!m_list.empty())
    {
        it = m_list.end();
        do
        {
            it--;
            int item_id = (*it).m_nId;
            if ((*it).m_nId == nId)
            {
                (*it).m_csNombre = pszText;
                break;
            }
        }
        while (it != m_list.begin());
    }
}





///////////////////////////////////////////////////////////////////////////////////////////////////
//
// GUARDAR
//
///////////////////////////////////////////////////////////////////////////////////////////////////
// Guardar en fichero de texto todas las areas
// Devuelve false en caso de error
// bFormato2d - true si se trata de un barrido y por tanto el nombre del campo contiene fila y columna
bool CAreas::Guardar(CString csCampo, bool bFormato2d, ControlImagenes& controlImagenes)
{
    CString csMuestra, csMuestraCampo, csFila;
    if (bFormato2d)
    {
        // Version 2 dimensiones
        csMuestra       = csCampo.Left( csCampo.GetLength()-8);
	    csMuestraCampo  = csCampo.Mid(csCampo.GetLength()-3,3);
	    csFila          = csCampo.Mid(csCampo.GetLength()-7,3);
    }
    else
    {
        // version 1d
        csMuestra       = csCampo.Left( csCampo.GetLength()-4);
	    csMuestraCampo  = csCampo.Mid(csCampo.GetLength()-3,3);
	    csFila          = "";
    }
    
    Guardar(csCampo + EXTENSION_FICHERO_CAMPO,csMuestra,csMuestraCampo,csFila,controlImagenes, AREAS_INFO);
//  OBSOLETO Guardar(csCampo + EXTENSION_FICHERO_CAMPO_CLASIFICACION,csMuestra,csMuestraCampo,csFila,controlImagenes, AREAS_CLASIFICACION);
    Guardar(csCampo + EXTENSION_FICHERO_CAMPO_PIXELS,csMuestra,csMuestraCampo,csFila,controlImagenes, AREAS_PIXELS);

    return true;
}

// Auxiliar - informacion comun a todos los tipos de ficheros de areas
bool CAreas::Guardar(CString csCampo, CString csMuestra, CString csMuestraCampo, CString csFila, ControlImagenes& controlImagenes, enum_tipo_fichero enumTipoFichero)
{
	FILE*   archivo;
	if ((archivo = fopen(csCampo, "wt")) == NULL)
		return false;

	fprintf(archivo, "# DATOS DE AREAS PARA UN CAMPO\n\n");
	fprintf(archivo, "muestra = %s\n", csMuestra);
	fprintf(archivo, "campo = %s\n", csMuestraCampo);
	fprintf(archivo, "fila = %s\n", csFila);
    // numero de areas
	fprintf(archivo, "num_areas = %ld\n", m_list.size());
    // numero de bandas
	fprintf(archivo, "num_bandas = %ld\n", theApp.m_ParamIni.nBandas-1); //pancromatico no
    if (enumTipoFichero == AREAS_INFO)
        // numero de valores histograma
	    fprintf(archivo, "num_val_histo = %ld\n", NUM_VALORES_HISTO);

    list<CArea>::iterator it;

    int nAreaCount = 1;
    for (it = m_list.begin(); it != m_list.end();it++)
    {
        // marcar sección [#area]
	    fprintf(archivo, "\n[%d]\n", nAreaCount);
        if (enumTipoFichero == AREAS_CLASIFICACION)
            GuardarClasificacionArea(*it,archivo, controlImagenes);
        else if (enumTipoFichero == AREAS_PIXELS)
            GuardarPixelsArea(*it,archivo, controlImagenes);
        else if (enumTipoFichero == AREAS_INFO)
            GuardarArea(*it,archivo);
        nAreaCount++;
    }

    fclose(archivo);

    return true;
}

/* OBSOLETO
// Guardar en fichero de texto todas las areas. Formato reducido (unicamente informativo)
// Devuelve false en caso de error
void CAreas::GuardarReducido(FILE* archivo, int nCampo)
{
    list<CArea>::iterator it;
    int nAreaCount = 1;
	fprintf(archivo, "num_datos = %d\n\n", m_list.size());
    for (it = m_list.begin(); it != m_list.end();it++)
    {
        //Campo Zona Area Mineral Calidad Comentario {Espectros} {Valor confianza}
        //Campo
		fprintf(archivo, "%03d\t\t", nCampo);
        //Zona
		fprintf(archivo, "%03d\t\t", nAreaCount);
        int nArea = abs((*it).m_rect.left - (*it).m_rect.right) * abs((*it).m_rect.top - (*it).m_rect.bottom);
        //Area total en pixeles
		fprintf(archivo, "%04d\t", nArea);
        //Abreviatura
		fprintf(archivo, "%s\t", (*it).m_csMineralAbreviatura);
        if ((*it).m_csMineralAbreviatura.GetLength() < 4)
		    fprintf(archivo, "\t");

        //Calidad
        char calidad;
        if ((*it).m_csCalidad ==  CALIDAD_A)
            calidad = 'A';
        else if ((*it).m_csCalidad ==  CALIDAD_B)
            calidad = 'B';
        else if ((*it).m_csCalidad ==  CALIDAD_C)
            calidad = 'C';
        else if ((*it).m_csCalidad ==  CALIDAD_D)
            calidad = 'D';
        else if ((*it).m_csCalidad ==  CALIDAD_E)
            calidad = 'E';
        else 
            calidad = '-';
		fprintf(archivo, "%c\t\t", calidad);
        //Comentario
        if ((*it).m_csComentario == "")
		    fprintf(archivo, "\t\t\t", (*it).m_csComentario);
        else
		    fprintf(archivo, "%s\t", (*it).m_csComentario);

        //Volcar espectros
 	    for (int i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrEspectros no contiene el pancromatico
		    fprintf(archivo, "%.2lf\t", (*it).m_arrEspectros[i]);
        fprintf(archivo,"\t"); // tabulador extra porque 1000 ocupa un espacio mas

        //Volcar percentiles
 	    for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrEspectros no contiene el pancromatico
		    fprintf(archivo, "%.2lf\t", (*it).m_arrPercentilInf[i]);
        fprintf(archivo,"\t"); // tabulador extra porque 1000 ocupa un espacio mas

        for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrEspectros no contiene el pancromatico
		    fprintf(archivo, "%.2lf\t", (*it).m_arrPercentilSup[i]);
        fprintf(archivo,"\t"); // tabulador extra porque 1000 ocupa un espacio mas

		fprintf(archivo, "\n");

        nAreaCount++;
    }
}
*/

// Guardar en fichero de texto el area especificada
void CAreas::GuardarArea(CArea& area, FILE* archivo)
{
    int i,j;

	fprintf(archivo, "# DATOS DE AREA\n");

    fprintf(archivo, "Nombre              = %s       \n",       area.m_csNombre        );
    fprintf(archivo, "Mineral             = %s       \n",       area.m_csMineral        );
    fprintf(archivo, "Calidad             = %s       \n",       area.m_csCalidad        );
    fprintf(archivo, "Comentario          = %s       \n",       area.m_csComentario        );
    //espectros
	fprintf(archivo, "# REFLECTANCIA ESPECTRAL\n");
	fprintf(archivo, "Espectros = ");
	for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrEspectros no contiene el pancromatico
		fprintf(archivo, "%.2lf ", area.m_arrEspectros[i]);
	fprintf(archivo, "\n");
    //percentiles
	fprintf(archivo, "# PERCENTILES\n");
	fprintf(archivo, "PercentilInf = ");
	for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrEspectros no contiene el pancromatico
		fprintf(archivo, "%.2lf ", area.m_arrPercentilInf[i]);
	fprintf(archivo, "\n");
	fprintf(archivo, "PercentilSup = ");
	for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrEspectros no contiene el pancromatico
		fprintf(archivo, "%.2lf ", area.m_arrPercentilSup[i]);
	fprintf(archivo, "\n");
	fprintf(archivo, "\n");

	fprintf(archivo, "# HISTOGRAMA DE CADA FILTRO\n");
	for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrHistogramas no contiene el pancromatico
    {
	    fprintf(archivo, "Histograma%02d = ",i+1);
	    for (j=0; j < NUM_VALORES_HISTO; j++)
		    fprintf(archivo, "%.2lf ", area.m_arrHistogramas[i][j]);
	    fprintf(archivo, "\n");
    }
	fprintf(archivo, "\n");

    fprintf(archivo, "rect = %ld %ld %ld %ld      \n",       area.m_rect.left,area.m_rect.top,area.m_rect.right,area.m_rect.bottom        );
//    fprintf(archivo, "nColorIndex           = %ld       \n",       area.m_nColorIndex        );

}

// Guardar en fichero de texto el area especificada
void CAreas::GuardarPixelsArea(CArea& area, FILE* archivo, ControlImagenes& controlImagenes)
{
    int i,j;

    fprintf(archivo, "Abreviatura = %s\n", area.m_csNombre.Left(area.m_csNombre.GetLength()-2)   );// Obtenemos abreviatura a partir de nombre
    fprintf(archivo, "Calidad     = %s\n", area.m_csCalidad    );
    fprintf(archivo, "Comentario  = %s\n", area.m_csComentario );
    //espectros
    int nAncho = (area.m_rect.right - area.m_rect.left);
    int nAlto = (area.m_rect.bottom - area.m_rect.top);
    int nPixels = nAncho*nAlto;
    fprintf(archivo, "Pixels      = %d\n", nPixels);
    unsigned short** arrEspectroArea = new unsigned short*[theApp.m_ParamIni.nBandas-1]; //pancromatico no
	for (j=0; j < theApp.m_ParamIni.nBandas-1; j++)
        arrEspectroArea[j] = new unsigned short[nPixels];
    controlImagenes.GetEspectroArea(area.m_rect.left, area.m_rect.top, nAncho, nAlto, arrEspectroArea);
	for (i=0; i < nPixels; i++)
    {
	    for (j=0; j < theApp.m_ParamIni.nBandas-1; j++) //pancromatico no
		    fprintf(archivo, "%.2lf ", pow(arrEspectroArea[j][i],8.0/theApp.m_nBitsProfundidad)*theApp.m_dEscalaReflectancia); // Guardamos en REFLECTANCIA
	    fprintf(archivo, "\n");
    }
	fprintf(archivo, "\n");

	for (j=0; j < theApp.m_ParamIni.nBandas-1; j++)
        delete [] arrEspectroArea[j];
    delete [] arrEspectroArea;
}

// Guardar en fichero de texto la informacion de clasificacion del area especificada
// OBSOLETO
void CAreas::GuardarClasificacionArea(CArea& area, FILE* archivo, ControlImagenes& controlImagenes)
{
	fprintf(archivo, "# DATOS DE AREA\n");

    fprintf(archivo, "Nombre              = %s       \n",       area.m_csNombre        );
    fprintf(archivo, "Mineral             = %s       \n",       area.m_csMineral        );
    fprintf(archivo, "Calidad             = %s       \n",       area.m_csCalidad        );
    fprintf(archivo, "Comentario          = %s       \n",       area.m_csComentario        );
    //espectros

//    fprintf(archivo, "rect = %ld %ld %ld %ld      \n",       area.m_rect.left,area.m_rect.top,area.m_rect.right,area.m_rect.bottom        );


    list<CMineralClasificado> listClasificados; // lista de minerales clasificados
    unsigned int nCountOscuros = -1;
    if (!controlImagenes.Clasificar(NULL,listClasificados, nCountOscuros, &area.m_rect))
    {
        fprintf(archivo, "Error al clasificar\n");
        ASSERT(FALSE);
        return;
    }
    fprintf(archivo, "num_minerales_clasificados = %d\n",listClasificados.size());
    list<CMineralClasificado>::iterator it;
    fprintf(archivo, "nombres_clasificados       = ");
    for (it=listClasificados.begin();it!=listClasificados.end();it++)
    {
        CString csNombre;
        if ((*it).m_nIndex != -1)
            csNombre = theApp.m_minerales.GetNombre((*it).m_nIndex);
	    fprintf(archivo, "%-19s ",csNombre);
    }
	fprintf(archivo, "\n");

    /*
    fprintf(archivo, "porcentaje_clasificados    = ");
    for (it=listClasificados.begin();it!=listClasificados.end();it++)
    {
        CString csPorcentaje;
        int nTotalPixels = (area.m_rect.right - area.m_rect.left) * (area.m_rect.bottom - area.m_rect.top);
        csPorcentaje.Format("%2.2lf",100* (double)(*it).m_nCount/(double)nTotalPixels);
	    fprintf(archivo, "%-19s ",csPorcentaje);
    }
    */
    
    fprintf(archivo, "pixels_clasificados        = ");
    for (it=listClasificados.begin();it!=listClasificados.end();it++)
    {
        fprintf(archivo,"%-19d ",(*it).m_nCount);
    }
    
	fprintf(archivo, "\n");
    fprintf(archivo, "distancia_clasificados     = ");
    for (it=listClasificados.begin();it!=listClasificados.end();it++)
    {
	    fprintf(archivo, "%-19lf ",(*it).m_dDistancia);
    }
	fprintf(archivo, "\n");
    fprintf(archivo, "confiabilidad_clasificados = ");
    for (it=listClasificados.begin();it!=listClasificados.end();it++)
    {
	    fprintf(archivo, "%-19lf ",(*it).m_dConfiabilidad);
    }
	fprintf(archivo, "\n");
}

// Cargar desde fichero de texto todas las areas
// Puntero a dialogo necesario para definir graficos areas por cada area
// Devuelve false si no se encuentra el fichero o en caso de error
bool CAreas::Cargar(CString csCampo, CAnalisisDlg* pDialogo)
{
    csCampo += EXTENSION_FICHERO_CAMPO;

    // lee todas las variables del fichero y las guarda en variables globales
	if ( LoadVars((LPCTSTR)csCampo) == NULL) {
		return false;
	}

    int num_areas;
    LOADINT(num_areas);

    if (num_areas < 1)
        return false;

    int num_bandas;
    LOADINT(num_bandas);

    int num_val_histo;
    LOADINT(num_val_histo);
    CArea* pArea;
    CRect dummy_rect;
    for (int i=0; i <num_areas; i++)
    {
        pArea = NuevaArea(dummy_rect); //creamos y añadimos al contenedor nueva area vacia. Color reasignado
        char strCount[3];
        itoa(i+1,strCount,10); // convertir contador a string 
        CargarArea(strCount, *pArea, num_bandas, num_val_histo); //recuperamos y actualizamos datos
        if (pDialogo != NULL)
            pDialogo->DibujarArea(*pArea);
    }

    return true;
}

// Carga el area especificada. variables previamente cargadas usando "myVars"(variables globales)
void CAreas::CargarArea(char* strCount, CArea& area, int numEspectros, int numValHisto)
{
    int i;

    //datos generales
    char* Nombre;
    Nombre = area.m_csNombre.GetBuffer(100);
	LOADSTRSECTION(Nombre,strCount);
    area.m_csNombre.ReleaseBuffer(-1);

    // Obtenemos abreviatura a partir de nombre
    area.m_csMineralAbreviatura = area.m_csNombre.Left(area.m_csNombre.GetLength()-2);

    char* Calidad;
    Calidad = area.m_csCalidad.GetBuffer(100);
	LOADSTRSECTION(Calidad,strCount);
    area.m_csCalidad.ReleaseBuffer(-1);

    char* Mineral;
    Mineral = area.m_csMineral.GetBuffer(100);
	LOADSTRSECTION(Mineral,strCount);
    area.m_csMineral.ReleaseBuffer(-1);


    char* Comentario;
    Comentario = area.m_csComentario.GetBuffer(1000);
	LOADSTRSECTION(Comentario,strCount);
    area.m_csComentario.ReleaseBuffer(-1);


    //espectros
    double* Espectros;
    Espectros = area.m_arrEspectros;
    ASSERT(numEspectros == theApp.m_ParamIni.nBandas-1); //valor guardado en fichero igual que configurado
    LOADVDBLSECTION(Espectros, min(numEspectros,theApp.m_ParamIni.nBandas-1),strCount); //min por seguridad

    // identificamos el mineral (con probabilidades) a partir de su espectro
        // Al estar los valores de entrenamiento (reflectancias de minerales) en niveles de gris, hay que transformar
//    double  arrEspectrosNivGris[MAX_NUM_IMAGENES]; //Auxiliar
//    for (i=0;i<numEspectros;i++)
//        arrEspectrosNivGris[i] = area.m_arrEspectros[i] / theApp.m_dEscalaReflectancia; //de reflectancias a niveles de gris
//    theApp.m_minerales.Identificar(arrEspectrosNivGris, area.m_listIdentificacion);
    theApp.m_minerales.Identificar(area.m_arrEspectros, area.m_listIdentificacion);

    //percentiles
    double* PercentilInf;
    PercentilInf = area.m_arrPercentilInf;
    LOADVDBLSECTION(PercentilInf, min(numEspectros,theApp.m_ParamIni.nBandas-1),strCount); //min por seguridad
    double* PercentilSup;
    PercentilSup = area.m_arrPercentilSup;
    LOADVDBLSECTION(PercentilSup, min(numEspectros,theApp.m_ParamIni.nBandas-1),strCount); //min por seguridad

    //histogramas
    ASSERT(NUM_VALORES_HISTO==numValHisto); 
	char nombre_variable[13]; 
	for (i=0; i < theApp.m_ParamIni.nBandas-1; i++) // -1 porque no m_arrHistogramas no contiene el pancromatico
    {
		sprintf(nombre_variable,"Histograma%02d",i+1);
        LOADVDBL2SECTION(area.m_arrHistogramas[i],nombre_variable,numValHisto,strCount);
    }

    //rect
    int rect[4];
    LOADVINTSECTION(rect,4, strCount);
    area.m_rect.left = rect[0];
    area.m_rect.top = rect[1];
    area.m_rect.right = rect[2];
    area.m_rect.bottom = rect[3];
}

