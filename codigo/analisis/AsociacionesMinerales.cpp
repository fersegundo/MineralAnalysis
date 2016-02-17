#include "stdafx.h"
#include "AsociacionesMinerales.h"
#include "Minerales.h"
#include "..\librerias\ProcesoAux\myUtilIO.h" //BuscaString
#include <algorithm> //for std::sort
#include "..\librerias\ProcesoAux\myVars.h" //LoadVars

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// PRUEBAS BORRAR
#include "..\librerias\procesoaux\gestion_mensajes.h"
#include "ClasificaBarrido.h"

extern bool ComparadorMinerales(CMineral* mineral1,CMineral* mineral2); //definido en Minerales.cpp, necesario para ordenar la lista de minerales

CAsociacionesMinerales::CAsociacionesMinerales()
{
    m_nAsociaciones                 = -1;
    m_arrNombresAsociaciones        = NULL;
    m_pMinerales                    = NULL;
}

CAsociacionesMinerales::~CAsociacionesMinerales()
{
    if (m_arrNombresAsociaciones != NULL)
        delete [] m_arrNombresAsociaciones;
    if (m_arrDescripcionAsociaciones != NULL)
        delete [] m_arrDescripcionAsociaciones;
    if (m_arrCompatibilidadAsociaciones != NULL)
        delete [] m_arrCompatibilidadAsociaciones;
}

//retiene el puntero a la clase Minerales
void CAsociacionesMinerales::Init(CMinerales* pMinerales)
{
    m_pMinerales = pMinerales;
}

//llamar cuando no se usen mas las asociaciones. Libera las asociaciones minerales y desasocia los punteros de la clase Minerales
void CAsociacionesMinerales::Liberar() 
{
    vector<CMineral*>::iterator it;

    for (it = m_pMinerales->m_list.begin(); it != m_pMinerales->m_list.end();++it)
    {
        if ((*it)->m_pAsociaciones)
        {
            delete (*it)->m_pAsociaciones;
            (*it)->m_pAsociaciones = NULL;
        }
    }
}

// Leemos el fichero con las asociaciones minerales
bool CAsociacionesMinerales::CargarFichero(CString csFichero)
{
    int i;
    FILE* archivo;
    char* pString;
    int eof;
    CString csError;
    ULONG ulNumLineasDummy = 0;

    if (m_pMinerales == NULL)
    {
        csError.Format("No se ha inicializado la clase de Asociaciones Minerales");
        AfxMessageBox(csError);
        ASSERT(FALSE);
        return false;
    }

    if (csFichero == "")
        csFichero = FICHERO_DEFECTO_ASOCIACIONES;
	if ((archivo = fopen(csFichero, "rt")) == NULL)
    {
        csError.Format("Error al abrir el fichero de asociaciones: %s",csFichero);
        AfxMessageBox(csError);
        ASSERT(FALSE);
        return false;
    }

    // CABECERA DE ASOCIACIONES

    // Buscamos la posicion del comienzo de las descripciones
    ReadLine(archivo,eof); // terminar de leer la linea (ambitos)
    fseek(archivo,2,SEEK_CUR); //saltamos el ;;
    unsigned long posDescripciones = ftell(archivo);

    // Buscamos la posicion del comienzo de los nombres identificativos
    unsigned long posNombres = BuscaString(archivo,"Minerales;Abrev.;",ulNumLineasDummy); //buscamos (secuencialmente) y nos posicionamos depues de ...
    if (posNombres==-1)
    {
        AfxMessageBox("Error al buscar Minerales;Abrev.;");
        ASSERT(FALSE);
        fclose(archivo);
        return false;
    }

    // Primero determinamos unicamente cuantas asociaciones hay (para dimensionar el array)
    pString = ReadLine(archivo,eof);
    if (pString == NULL || eof==1)
    {
        fclose(archivo);
        return false;
    }
    
    // Eliminamos posibles ';' al final del string
    int len = strlen(pString);
    for ( int p=len-1; p>=0; p-- ) 
    {
        if ( pString[p] == ';' || pString[p] == ' ' || pString[p] == '\t' ) 
            pString[p] = (char)0; 
        else 
            break;
    }
    m_nAsociaciones = Count(';',pString) + 1; // Calculamos numero de asociaciones

    // Una vez tenemos el numero de asociaciones, reservamos memoria
    m_arrNombresAsociaciones        = new CString [m_nAsociaciones];
    m_arrDescripcionAsociaciones    = new CString [m_nAsociaciones];
    m_arrCompatibilidadAsociaciones = new int     [m_nAsociaciones];

    memset(m_arrCompatibilidadAsociaciones,0,m_nAsociaciones*sizeof(int));

    // Leemos las descripciones de asociaciones
    fseek(archivo, posDescripciones, SEEK_SET); // volvemos al principio de la lista de nombres
    for (i=0; i<m_nAsociaciones; i++)
    {
        m_arrDescripcionAsociaciones[i] = ReadUntil(archivo, ";",eof);
        if (eof == 1)
            ASSERT(false); //tienen que estar todos los que leimos antes
        fseek(archivo,1,SEEK_CUR); //saltamos el ;
    }
    ReadLine(archivo,eof); // terminar de leer la linea
    if (eof == 1)
        ASSERT(false);

    // Leemos los nombres de asociaciones
    fseek(archivo, posNombres, SEEK_SET); // volvemos al principio de la lista de nombres
    for (i=0; i<m_nAsociaciones; i++)
    {
        m_arrNombresAsociaciones[i] = ReadUntil(archivo, ";",eof);
        if (eof == 1)
            ASSERT(false); //tienen que estar todos los que leimos antes
        fseek(archivo,1,SEEK_CUR); //saltamos el ;
    }
    ReadLine(archivo,eof); // terminar de leer la linea
    if (eof == 1)
        ASSERT(false);

    // RESTO DE TABLA
    // Odenamos minerales por abreviatura para que funcione BuscarPorAbreviatura
    CMineral::m_enumOrden = ORDEN_ABREVIATURA; //m_enumOrden es static
    std::sort(m_pMinerales->m_list.begin(),m_pMinerales->m_list.end(),ComparadorMinerales);

    CMineral* pMineral = NULL;
    while(eof != 1)
    {
        ReadUntil(archivo, ";",eof); // saltamos el nombre
        fseek(archivo,1,SEEK_CUR); //saltamos el ;        
        CString csAbreviatura = ReadUntil(archivo, ";",eof); // abreviatura
        fseek(archivo,1,SEEK_CUR); //saltamos el ;        
//        if (strcmp(pString,"") != 0)
//        {
            csAbreviatura.MakeLower();
            pMineral = m_pMinerales->BuscarPorAbreviatura(csAbreviatura);
            if (pMineral)
            {
                pMineral->m_pAsociaciones = new CListaAsociaciones;
                pMineral->m_pAsociaciones->m_lista = new unsigned __int8[m_nAsociaciones];
                for (i=0; i<m_nAsociaciones; i++)
                {
                    pMineral->m_pAsociaciones->m_lista[i] = (unsigned __int8) atoi(ReadUntil(archivo, ";\n",eof));
                    if (eof == 1)
                        ASSERT(false); //tienen que estar todos los que leimos antes
                    fseek(archivo,1,SEEK_CUR); //saltamos el ;
                }
            }
//        }
        ReadLine(archivo,eof); // terminar de leer la linea
    }

    // Ordenamos por reflectancia media (de todas las bandas) para usar arrays de busqueda (para ahorrar tiempo en la clasificacion)
    CMineral::m_enumOrden = ORDEN_REFLECTANCIA; //m_enumOrden es static
    std::sort(m_pMinerales->m_list.begin(),m_pMinerales->m_list.end(),ComparadorMinerales);

    fclose(archivo);
    return true;
}

// Carga el fichero de asociaciones seleccionadas por el usuario
bool CAsociacionesMinerales::CargarFicheroAsociacionesSeleccionadas(CString csFichero)
{
    bool bAlgunaAsociacion = false;
	if ( LoadVars(csFichero) == NULL) {
		return false;
	}

    char aux[1];
    for (int i= 0; i<m_nAsociaciones;i++)
    {
        LOADSTR2(aux,(char *)(LPCSTR)m_arrNombresAsociaciones[i]);
        if (aux != NULL && aux[0] == '1')
        {
            m_arrCompatibilidadAsociaciones[i] = 1;
            bAlgunaAsociacion = true;
        }
        else
            m_arrCompatibilidadAsociaciones[i] = 0;
    }

    int asociacion_usuario;
    LOADINT(asociacion_usuario);
    if (asociacion_usuario == 1)
    {
        m_bAsociacionUsuario = true;
        bAlgunaAsociacion = true;
    }

    for (i= 0; i<m_pMinerales->GetCount();i++)
    {
        LOADSTR2(aux,(char *)(LPCSTR)m_pMinerales->GetAbreviatura(i));
        if (aux != NULL && aux[0] == '1')
            m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario = true;
        else
            m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario = false;
    }

    return bAlgunaAsociacion;
}

// Guardamos el fichero de asociaciones seleccionadas por el usuario
bool CAsociacionesMinerales::GuardarFicheroAsociacionesSeleccionadas(CString csFichero)
{
    FILE* file = NULL;
    if ((file = fopen(csFichero, "wt")) == NULL)
        return false;

    for (int i= 0; i<m_nAsociaciones;i++)
    {
        fprintf(file,"%s = %d\n",m_arrNombresAsociaciones[i],m_arrCompatibilidadAsociaciones[i]);
    }
    fprintf(file,"\n");
    fprintf(file,"asociacion_usuario = %d\n",(BOOL)m_bAsociacionUsuario);
    for (i= 0; i<m_pMinerales->GetCount();i++)
    {
        fprintf(file,"%s = %d\n",m_pMinerales->GetAbreviatura(i), (BOOL)m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario);
    }

    fclose(file);

    return true;

}

// Para cada mineral, comprobamos que sea compatible con las asociaciones, si no, lo desactivamos (siempre y cuando no sea un mineral seleccionado)
void CAsociacionesMinerales::ActualizarMineralesActivosPorAsociaciones() 
{
#ifdef _DEBUG
FILE* min_activos;
min_activos = fopen("min_activos.txt","wt");
#endif

    CListaAsociaciones* pAsociaciones; //Auxiliar, para cada mineral identificado
    int nMinerales = m_pMinerales->GetCount();
    m_pMinerales->m_bAlgunMineralClasificable = false;
    for(int i = 0; i < nMinerales; i++)
    {
        // Primero comprobar si es compatible con la asociacion de usuario (en caso de que este seleccionada)
        if (m_bAsociacionUsuario &&
            m_pMinerales->m_list[i]->m_bPerteneceAsociacionUsuario)
        {
#ifdef _DEBUG
fprintf(min_activos,"%s USUARIO\n", m_pMinerales->m_list[i]->m_csNombre);
#endif
            m_pMinerales->m_list[i]->m_bClasificable = true;
            m_pMinerales->m_bAlgunMineralClasificable = true;
            continue; //ya no hace falta buscar este mineral en el resto de asociaciones
        }
        else
        {
            m_pMinerales->m_list[i]->m_bClasificable = false;
        }

        pAsociaciones = m_pMinerales->m_list[i]->m_pAsociaciones;
        if (pAsociaciones)
        {
            for(int a=0;a<m_nAsociaciones;a++)
            {
                if (m_arrCompatibilidadAsociaciones[a] > 0 && pAsociaciones->m_lista[a] != AS_MIN_AUSENTE)
                {
#ifdef _DEBUG
fprintf(min_activos,"%s %s: %d\n", m_pMinerales->m_list[i]->m_csNombre,m_arrNombresAsociaciones[a],pAsociaciones->m_lista[a]);
#endif
                    m_pMinerales->m_list[i]->m_bClasificable = true; // El mineral es compatible, lo activamos
                    m_pMinerales->m_bAlgunMineralClasificable = true;
                    break;
                }

            } // for asociaciones
        }
    } //for minerales

#ifdef _DEBUG
fclose (min_activos);
#endif
}

// Filtramos las asociaciaciones compatibles segun los minerales presentes y la tabla de asociaciones
// arrOriginal          - minerales presentes
// arrAsocCompatibles   - resultado que se devuelve: Asociaciones compatibles
bool CAsociacionesMinerales::FiltrarAsociaciones(unsigned int* arrOriginal, int* arrAsocCompatibles)
{
    int                 i,a;
    int                 nTotal = 0;
    int                 nMinerales = m_pMinerales->GetCount();
    CListaAsociaciones* pAsociaciones; //Auxiliar, para cada mineral identificado

    unsigned int*                arrOrdenado        = new unsigned int[nMinerales]; // lista que contiene el indice de los minerales ordenados por cantidad de pixeles
    unsigned __int8*    arrPosibilidades   = new unsigned __int8[nMinerales]; // posibilidad de aparicion (0..5) de minerales originales

//    int* arrResultado  = new int[numMin];
//    memset(arrResultado,0,numMin*sizeof(int));

    memset(arrPosibilidades,0,nMinerales*sizeof(unsigned __int8));
    memset(arrAsocCompatibles,0,m_nAsociaciones*sizeof(int));

    if (m_pMinerales == NULL || m_arrNombresAsociaciones == NULL)
    {
        ASSERT(false);
        return false;
    }

    // Primero, calculamos los pixeles totales e inicializamos la lista ordenada
    int nPrincipio = 0; //indice para ayudar a inicializar lista ordenada desde el principio 
    int nFinal = nMinerales - 1; //indice para ayudar a inicializar lista ordenada desde el final (con ceros)
    for(i = 0; i< nMinerales; i++)
    {
        nTotal += arrOriginal[i];
        // Aprovechamos para ordenar los minerales con 0 pixeles al final de la lista para que luego se ordene mas rapido
        if (arrOriginal[i] == 0)
        {
            arrOrdenado[nFinal] = i; // la lista ordenada de indices estará inicialmente como esté la lista original
            nFinal--;
        }
        else
        {
            arrOrdenado[nPrincipio] = i; // la lista ordenada de indices estará inicialmente como esté la lista original
            nPrincipio++;
        }
        ASSERT(nFinal+1 >= nPrincipio);
    }
    ASSERT(nPrincipio == nFinal+1);
    if (nFinal == -1) //no hay ningun mineral con pixeles
    {
        AfxMessageBox("Ningún mineral cumplió los umbrales");
        delete [] arrPosibilidades;
        delete [] arrOrdenado;
        return false;
    }

    // para usar luego en Quicksort, queremos que nFinal apunte al ultimo valor no-cero
    if (arrOriginal[arrOrdenado[nFinal]] == 0)
        nFinal--;
    ASSERT(nFinal>=0);


    // Calculamos  posibilidad de aparicion (da igual hacerlo antes de ordenar o despues)
    // De paso, inicializamos el array resultado
    double dPorcentaje;
    for(i = 0; i< nMinerales; i++)
    {
        if (arrOriginal[i] == 0)
            arrPosibilidades[i] = AS_MIN_AUSENTE;
        else
        {
            dPorcentaje = (double)arrOriginal[i] / nTotal;
            if (dPorcentaje > AS_MIN_ABUNDANTE_PORCENTAJE_MIN)
            {
                arrPosibilidades[i] = AS_MIN_ABUNDANTE;
//                arrResultado[i] = arrPosibilidades[i]; // inicializamos el array resultado con los valores de entrada
            }
            else if (dPorcentaje > AS_MIN_PRINCIPAL_PORCENTAJE_MIN)
            {
                arrPosibilidades[i] = AS_MIN_PRINCIPAL;
//                arrResultado[i] = arrPosibilidades[i]; // inicializamos el array resultado con los valores de entrada
            }
            else if (dPorcentaje > AS_MIN_ACCESORIO_PORCENTAJE_MIN)
            {
                arrPosibilidades[i] = AS_MIN_ACCESORIO;
                arrOriginal[i] = 0; // al no tenerlo en cuenta, es como si no se hubiese encontrado
            }
            else if (dPorcentaje > AS_MIN_ESCASO_PORCENTAJE_MIN)
            {
                arrPosibilidades[i] = AS_MIN_ESCASO;
                arrOriginal[i] = 0; // al no tenerlo en cuenta, es como si no se hubiese encontrado
            }
            else 
            {
                arrPosibilidades[i] = AS_MIN_ESPORADICO;
                arrOriginal[i] = 0; // al no tenerlo en cuenta, es como si no se hubiese encontrado
            }
        }
    }

    // Ordenamos la lista por cantidad de pixels pero usando un array auxiliar para no tocar el orden
    // original de la lista (para poder indexarlo igual que esta la lista general de minerales)
    quicksort(arrOriginal, arrOrdenado, 0, nFinal); // a partir de nFinal son ceros

    // Calculamos las asociaciones compatibles
    
    // acciones y pesos aplicados ( 0 indica que no se tendrá en cuenta, negativo indica incompatibilidad de la asociacion)
    // al calcular asociaciones compatibles         0    1    2    3    4    5 <- posibilidad encontrada primera pasada (arrPosibilidades)
    //                                              --------------------------------------------------------
    int tablaPesos[AS_MIN_NUM][AS_MIN_NUM] = {      0  ,-900,-900, 0  , 0 , 0 , //0  ,-900,-900,-200,-100, 0  , // 0
                                                    0  , 1  , 1  , 0  , 0 , 0 , //0  , 200, 50 , 1  , 1  , 0  , // 1
                                                    0  , 1  , 1  , 0  , 0 , 0 , //0  , 50 , 100, 10 , 10 , 0  , // 2
                                                    0  , 1  , 1  , 0  , 0 , 0 , //0  , 0  , 10 , 50 , 10 , 0  , // 3   <- posibilidad que dice la asociacion (pAsociaciones->m_lista)
                                                    0  , 0  , 1  , 0  , 0 , 0 , //0  ,-10 , 1  , 10 , 50 , 0  , // 4
                                                    0  , 0  , 0  , 0  , 0 , 0   //0  ,-10 ,-10 , 10 , 10 , 0    // 5
                                             };
    i = 0;
    // ##### OPCION: Restringimos busqueda a posibilidades 1,2 ####
    // OJO: si se añaden o quitan posibilidades de busqueda, hay que añadirlas/ponerlas al inicializar arrResultado (ver bucle anterior)
    while(arrPosibilidades[arrOrdenado[i]] != AS_MIN_AUSENTE && arrPosibilidades[arrOrdenado[i]] != AS_MIN_ESPORADICO
        && arrPosibilidades[arrOrdenado[i]] != AS_MIN_ESCASO && arrPosibilidades[arrOrdenado[i]] != AS_MIN_ACCESORIO 
        && i<nMinerales)
    {
CString nom = m_pMinerales->m_list[arrOrdenado[i]]->m_csNombre;
        pAsociaciones = m_pMinerales->m_list[arrOrdenado[i]]->m_pAsociaciones;
        ASSERT(pAsociaciones);
        for(a=0;a<m_nAsociaciones;a++)
        {
/*
            if (pAsociaciones->m_lista[a] == AS_MIN_AUSENTE && arrAsocCompatibles[a] > 0)  
                arrAsocCompatibles[a] = -1; // ATENCION!! al menos un mineral previo (mas abundante) era compatible con esta asociacion
            else
            */
//                if (!(pAsociaciones->m_lista[a] != AS_MIN_AUSENTE && arrAsocCompatibles[a] < 0) )
                    arrAsocCompatibles[a] += tablaPesos[pAsociaciones->m_lista[a]][arrPosibilidades[arrOrdenado[i]]]; //La asociacion no es compatible con este mineral
                //else // ATENCION!! al menos un mineral previo (mas abundante) no era compatible con esta asociacion 
                    //no sumamos peso, que se quede incompatible

        } // for
        i++;
    } //while
    
    //marcar estos pixels como no clasificados para que se clasificaran de nuevo en segunda pasada

/*
    // Calculamos la lista resultado
    for(i = 0; i< nMinerales; i++)
    {
        pAsociaciones = m_pMinerales->m_list[i]->m_pAsociaciones;
        if (pAsociaciones)
        {
            for(a=0;a<m_nAsociaciones;a++)
            {
                if (arrAsocCompatibles[a] > 0)
                {
                    if (pAsociaciones->m_lista[a] != AS_MIN_AUSENTE)
                    {
                        //if (arrOriginal[i] == 0) // ##### OPCION SIN MODIFICAR POSIBILIDADES ORIGINALES (para OPCION MODIFICANDO, comentar) #####
                            if (arrResultado[i] == 0)
                                arrResultado[i] = pAsociaciones->m_lista[a]; //primera vez que se añade este mineral
                            else
                                arrResultado[i] = min(arrResultado[i], pAsociaciones->m_lista[a]); // #### OPCION MIN ####
                        //    #### OPCION MAYOR PESO ####, ESTABLECER MAYOR PESO DESDE BUCLE ANTERIOR (CALCULO ASOC. COMPATIBLES) Y AHORRARSE BUCLE DE ASOCIACIONES AQUI
                    }

                }

            } // for asociaciones
        }
        // Si el mineral no es compatible, lo desactivamos para que no se tenga en cuenta al clasificar en segunda pasada
        if (arrResultado[i] == 0)
            m_pMinerales->m_list[i]->m_bActivo = false; // DESACTIVAMOS EL MINERAL <------------------------
    } //for minerales
*/

/*
// LOG 
char mensaje[1024];
sendlog_timestamp(theApp.m_log, "\nFiltrarAsociacionesMinerales: Minerales");
sendlog_timestamp(theApp.m_log, " Mineral\tpixel\tp\tor\tresultado");
for(i = 0; i< nMinerales; i++)
{
//sprintf(mensaje, " %7s\t%5d\t%d\t%2d\t%d ", m_pMinerales->m_list[i]->m_csAbreviatura, arrOriginal[i], arrPosibilidades[i], arrOrdenado[i], arrResultado[i]);
sprintf(mensaje, " %7s\t%5d\t%d\t%2d ", m_pMinerales->m_list[i]->m_csAbreviatura, arrOriginal[i], arrPosibilidades[i], arrOrdenado[i]);
sendlog_timestamp(theApp.m_log, mensaje);
} //for minerales

sendlog_timestamp(theApp.m_log, "\nFiltrarAsociacionesMinerales:Asociaciones");
for(a=0;a<m_nAsociaciones;a++)
{
sprintf(mensaje, "%9s\t%d", m_arrNombresAsociaciones[a], arrAsocCompatibles[a]);
sendlog_timestamp(theApp.m_log, mensaje);
}

*/
    delete [] arrPosibilidades;
    delete [] arrOrdenado;

    return true;
}

//funcion para dividir el array y hacer los intercambios  
unsigned __int8 CAsociacionesMinerales::dividir (unsigned int *arrOriginal, unsigned int* arrOrdenado, int inicio, int fin)
{  
  int izq;  
  int der;  
  int pibote;  
  int temp;  
   
  pibote = arrOriginal[arrOrdenado[inicio]];  
  izq = inicio;  
  der = fin;  
   
  //Mientras no se cruzen los índices  
  while (izq < der){  
    while (arrOriginal[arrOrdenado[der]] < pibote){  
      der--;  
    }  
   
    while ((izq < der) && (arrOriginal[arrOrdenado[izq]] >= pibote)){  
      izq++;  
    }  
   
    // Si todavia no se cruzan los indices seguimos intercambiando  
    if(izq < der)
    {  
      temp= arrOrdenado[izq];  
      arrOrdenado[izq] = arrOrdenado[der];  
      arrOrdenado[der] = temp;  
    }  
  }  
   
  //Los indices ya se han cruzado, ponemos el pivote en el lugar que le corresponde  
  temp = arrOrdenado[der];  
  arrOrdenado[der] = arrOrdenado[inicio];  
  arrOrdenado[inicio] = temp;  
   
  //La nueva posición del pivote  
  return der;  
}  
   
//funcion recursiva para hacer el ordenamiento  
void CAsociacionesMinerales::quicksort( unsigned int *arrOriginal, unsigned int* arrOrdenado, int inicio, int fin)  
{  
  int pivote;  

  if(inicio < fin)
  {  
    pivote = dividir(arrOriginal, arrOrdenado, inicio, fin );  
    quicksort( arrOriginal, arrOrdenado, inicio, pivote - 1 );//ordeno la lista de los menores  
    quicksort( arrOriginal, arrOrdenado, pivote + 1, fin );//ordeno la lista de los mayores  
  }  
}  
