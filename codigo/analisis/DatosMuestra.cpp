// 

#include "stdafx.h"
#include "Areas.h"
#include "DatosMuestra.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//extern CAnalisisApp theApp;

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////

CDatosMuestra::CDatosMuestra()
{

}

CDatosMuestra::~CDatosMuestra()
{

}

/* OBSOLETO
// Genera el fichero de datos de la muestra (todos los campos) a partir de los ficheros de 
// datos de cada campo
// - csMuestra: nombre de la muestra (sin numero de campo), pero con "_"
// - csCampo: numero del campo actual
bool CDatosMuestra::Generar(CString csMuestra, int nCampoActual)
{
    CString csDatosCampo;
    int i;
    int nCampo = 1;
    int nCamposInexistentes = 0;
    bool bCargado = false;

    //Abrir fichero de salida
	FILE*   archivo;
	if ((archivo = fopen(csMuestra + "Zonas"+"txt", "wt")) == NULL)
		return false;
   
    // Cabecera
    //Campo Zona Area Mineral Calidad Comentario {Espectros} {Valor confianza}
	fprintf(archivo, "%s\t%s\t%s\t%s\t%s\t%s\t", "Campo", "Zona", "Area", "Mineral", "Calidad", "Comentario");
 	for (i=1; i < theApp.m_ParamIni.nBandas; i++) // i = 1 porque no queremos el pancromatico
		fprintf(archivo, "Ref_%d\t", theApp.m_ParamIni.Rueda.espectro[i]);
 	for (i=1; i < theApp.m_ParamIni.nBandas; i++) 
		fprintf(archivo, "P05_%d\t", theApp.m_ParamIni.Rueda.espectro[i]);
 	for (i=1; i < theApp.m_ParamIni.nBandas; i++) 
		fprintf(archivo, "P95_%d\t", theApp.m_ParamIni.Rueda.espectro[i]);
    fprintf(archivo, "\n");

    while(nCamposInexistentes < MAX_CAMPOS_INEXISTENTES || nCampo <= nCampoActual)
    {
        csDatosCampo.Format("%s%03d",csMuestra, nCampo);

        CAreas areas_temp;
        //Leer y cargar datos de este campo
        if (areas_temp.Cargar(csDatosCampo, NULL))
        {
            nCamposInexistentes = 0;
            //Cargar todas las areas de este campo
            areas_temp.GuardarReducido(archivo, nCampo);
        }
        else
            nCamposInexistentes++;
        nCampo++;
    } //while

    fclose(archivo);
    
    return true;
}
*/