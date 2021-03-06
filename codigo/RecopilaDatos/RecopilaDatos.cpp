// RecopilaDatos.cpp : Aplicaci�n de consola para buscar en un directorio ficheros .dat con informacion
// espectral de varias muestras/campos/areas y compilarlos en un archivo unico
//

#include "stdafx.h"

//#include <direct.h> // Para mkdir
#include "FileFinder.h"
#include "..\librerias\ProcesoAux\MyVars.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Recupera del fichero y escribe el area especificada. variables previamente cargadas usando "myVars"(variables globales)
//nValidAreaCount indica el area total actual. La primera area es 1
void ProcesarArea(char* strNumArea, int numEspectros, FILE* salida,int nValidAreaCount,CString csMuestra,int campo,int fila)
{
    //datos generales
    char* Nombre;
    CString csNombre;
    Nombre = csNombre.GetBuffer(100);
	LOADSTRSECTION(Nombre,strNumArea);
    csNombre.ReleaseBuffer(-1);

    // Obtenemos abreviatura a partir de nombre
    CString csAbrebiatura = csNombre.Left(csNombre.GetLength()-2);

    char* Calidad;
    CString csCalidad;
    Calidad = csCalidad.GetBuffer(100);
	LOADSTRSECTION(Calidad,strNumArea);
    csCalidad.ReleaseBuffer(-1);

    char* Mineral;
    CString csMineral;
    Mineral = csMineral.GetBuffer(100);
	LOADSTRSECTION(Mineral,strNumArea);
    csMineral.ReleaseBuffer(-1);


    char* Comentario;
    CString csComentario;
    Comentario = csComentario.GetBuffer(1000);
	LOADSTRSECTION(Comentario,strNumArea);
    csComentario.ReleaseBuffer(-1);


    //espectros
    char* Espectros;
    CString csEspectros;
    Espectros = csEspectros.GetBuffer(6*numEspectros);
	LOADSTRSECTION(Espectros,strNumArea);
    csEspectros.ReleaseBuffer(-1);

    //percentiles
    char* PercentilInf;
    CString csPercentilInf;
    PercentilInf = csPercentilInf.GetBuffer(6*numEspectros);
	LOADSTRSECTION(PercentilInf,strNumArea);
    csPercentilInf.ReleaseBuffer(-1);
    char* PercentilSup;
    CString csPercentilSup;
    PercentilSup = csPercentilSup.GetBuffer(6*numEspectros);
	LOADSTRSECTION(PercentilSup,strNumArea);
    csPercentilSup.ReleaseBuffer(-1);

    // Escribir datos a fichero de salida
    fprintf(salida, "[%d]\n", nValidAreaCount);
    fprintf(salida, "Abreviatura = %s\n", csAbrebiatura);
    fprintf(salida, "Mineral = %s\n", csMineral);
    fprintf(salida, "Calidad = %s\n", csCalidad);
    fprintf(salida, "Comentario = %s\n", csComentario);
    fprintf(salida, "Procedencia = %s campo:%d fila:%d\n", csMuestra, campo, fila);
    fprintf(salida, "Espectros   = %s\n", csEspectros);
    fprintf(salida, "Percentil05 = %s\n", csPercentilInf);
    fprintf(salida, "Percentil95 = %s\n", csPercentilSup);
    fprintf(salida, "\n");

}


//Devuelve el numero de areas validas en este fichero
//nValidAreaCount indica el area total actual. Se debe incrementar. La primera area es 0
int ProcesarFichero(CString csFichero, FILE* salida,int& nValidAreaCount, int& num_bandas_param)
{
    // lee todas las variables del fichero y las guarda en variables globales
	if ( LoadVars((LPCTSTR)csFichero) == NULL) {
		return 0;
	}

    CString csMuestra;
    char* muestra;
    muestra = csMuestra.GetBuffer(100);
    LOADSTR(muestra);
    csMuestra.ReleaseBuffer(-1);

    int num_areas;
    LOADINT(num_areas);

    int num_bandas = -1;
    LOADINT(num_bandas); //devuelve -1 si no encontrado

// SOPORTE A DATS ANTIGUOS
if (num_bandas == -1)
{
    int num_espectros = -1;
    LOADINT(num_espectros); //devuelve -1 si no encontrado
    num_bandas = num_espectros;
   
}
    num_bandas_param = num_bandas;
        
    int campo;
    LOADINT(campo);
    int fila;
    LOADINT(fila);

    for (int i=0; i <num_areas; i++)
    {
        char strCount[3];
        itoa(i+1,strCount,10); // convertir contador a string 
        nValidAreaCount++; // primera area es 1
        ProcesarArea(strCount, num_bandas, salida, nValidAreaCount, csMuestra, campo, fila); //recuperamos y actualizamos datos
    }

    return num_areas;
    
}


/**************************  procesa_argumentos ******************************
	Funci�n para procesar los argumentos de la l�nea de comandos.
	 - Primero es el directorio donde buscar (recursivamente) los ficheros de entrada
	 - Segundo es el nombre (y el camino) del fichero de salida
*****************************************************************************/
bool procesa_argumentos(int argc, TCHAR* argv[], CString& csDirectorioEntrada, CString& csFicheroSalida)
{
	char	mensaje[1024];
	char	*fn = "procesa_argumentos";

	if (argc <= 2)  {
		sprintf(mensaje,
		"\a RecopilaDatos busca en un directorio ficheros con informaci�n espectral.\n"
		"\a y los compila en un archivo �nico de salida .\n"
		"\t Uso:  RecopilaDados.exe  directorio_busqueda fichero_salida\n");
		
		CharToOemBuff(mensaje, mensaje, sizeof(mensaje));
		fprintf(stderr, "\n%s", mensaje);
		Sleep(3000);		// Espera para leer el mensaje que aparece en consola
		return false;
	}

	/*	El argumento de la linea de comandos es el directorio de patrones. */
	
	csDirectorioEntrada = argv[1];
	csFicheroSalida = argv[2];

    return true;
}

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    CFileFinder	_finder;
    CString csDirectorioEntrada;
    CString csFicheroSalida;

	if ( procesa_argumentos(argc, argv,csDirectorioEntrada,csFicheroSalida) == false)		
		return 1;

    //Abrir fichero de salida
	FILE*   archivo;
	if ((archivo = fopen(csFicheroSalida, "wt")) == NULL)
		return 1;

    // Dejar espacio para resumen posterior
    fseek(archivo,153 + 7*2,SEEK_SET);


    // Buscar todos los archivos de datos
	CFileFinder::CFindOpts	opts;
	// Set CFindOpts object
	opts.sBaseFolder = csDirectorioEntrada;
	opts.sFileMask.Format("*.dat");
	opts.bSubfolders = true;
	opts.FindNormalFiles();
	_finder.RemoveAll();
	_finder.Find(opts);
    // Fin de la busqueda

    // Para cada archivo, leer la informacion y volcarla al fichero general
    CString csFichero;
    int nValidFileCount = 0;
    int nValidAreaCount = 0;
    int nAreasInFile = -1;
    int num_bandas = 0;
    for (int i=0 ; i<_finder.GetFileCount();i++)
    {
        csFichero = _finder.GetFilePath(i);
        nAreasInFile = ProcesarFichero(csFichero, archivo, nValidAreaCount, num_bandas); //nValidAreaCount se incrementa
        if (nAreasInFile > 0)
        {
            nValidFileCount++;
        }
    }

    // A�adir resumen a archivo de salida
    fseek(archivo,0,SEEK_SET);
    fprintf(archivo, "# Informaci�n de reflectancia de todos los minerales\n");
    // A�adir fecha
    SYSTEMTIME st;
    GetSystemTime(&st);
    fprintf(archivo,"# %2d del %2d de %4d a las %2d:%2d\n" ,st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute);
    fprintf(archivo, "# Numero total de ficheros usados: %4d\n",nValidFileCount);
    fprintf(archivo, "\n");
    fprintf(archivo, "num_datos = %4d\n",nValidAreaCount);
    fprintf(archivo, "num_bandas = %2d\n",num_bandas);
    fprintf(archivo, "\n");

	return 0;
}

