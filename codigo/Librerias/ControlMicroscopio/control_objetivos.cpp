/*****************************************************************************
    Rutinas auxiliares para el control de los objetivos del microscopio

  Autores:	Fernando Segundo
  Copyright:	© AITEMIN 2009
*****************************************************************************/

#include "stdafx.h"
#include "control_objetivos.h"
#include "control_microscopio.h"
#include "..\ProcesoAux\parametros_adquisicion.h"
#include "..\ProcesoAux\gestion_mensajes.h" //sendlog
#include "..\LibBarrido\CorreccionGeometrica.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

static int m_nObjetivoConfiguradoActual = -1; //para no actualizar la configuracion si no se ha cambiado de objetico

// Obtiene el objetivo actual y carga la nueva configuracion para este objetivo.
// Es necesario haber leido la configuracion del microscopio
// Devuelve false si - No se ha encontrado el objetivo actual en la configuracion del microscopio
//		             - No se ha podido cargar el fichero de configuracion de revolver
bool actualizar_configuracion_objetivo(parametros* pParamIni, bool& bHayCambioObjetivo)
{
    // Antes de cargar los parametros dependientes del objetivo, hay que saber en que objetivo estamos
    //corregimos la escala objetivo segun los aumentos en los que estemos
    int nObjetivo = mspGetObjetivo(); //1..MAX_OBJETIVOS

    if (m_nObjetivoConfiguradoActual != nObjetivo)
    {
        CString csMensaje;
        csMensaje.Format("Se ha detectado un cambio de configuracion del objetivo %d al objetivo %d", m_nObjetivoConfiguradoActual, nObjetivo);
        sendlog("\nactualizar_configuracion_objetivo", csMensaje);

        m_nObjetivoConfiguradoActual = nObjetivo;

	    //	Parámetros de adquisición relativos al revolver DTA
	    char  nom_fich[512];
	    int		nResultado = 0;
	    sprintf(nom_fich, "%s%s%s", pParamIni->objetivos[m_nObjetivoConfiguradoActual-1].csDirectorio, FICH_PARAM_REVOLVER, EXT_INI);
	    if (lee_paramRevolver(nom_fich, pParamIni) == -1)
            return false;

        //Establecemos la raiz de patrones a usar
        strcpy(pParamIni->raiz_patrones, pParamIni->objetivos[m_nObjetivoConfiguradoActual-1].csDirectorio);

        //Nos aseguramos que la lampara este en la intensidad configurada
        mspSetLamp(pParamIni->Mtb.voltLamp);

        bHayCambioObjetivo = true;
    }
    else
        bHayCambioObjetivo = false;

    return true;
}

