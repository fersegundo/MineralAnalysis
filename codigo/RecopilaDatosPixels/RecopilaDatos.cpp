// RecopilaDatos.cpp : Aplicación de consola para buscar en un directorio ficheros .dat con informacion
// espectral de varias muestras/campos/areas y compilarlos en un archivo unico
//

#include "stdafx.h"

//#include <direct.h> // Para mkdir
#include "FileFinder.h"
#include "..\librerias\ProcesoAux\myUtilIO.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Recupera del fichero y escribe el area especificada. 
//nValidAreaCount indica el area total actual. La primera area es 1
void ProcesarArea(FILE* fichero, char* strNumArea, FILE* salida,int nValidAreaCount,CString csMuestra,int campo,int fila)
{
    // Escribir contador de area a fichero de salida
    fprintf(salida, "[%d]\n", nValidAreaCount);
    fprintf(salida, "Procedencia = %s campo:%d fila:%d", csMuestra, campo, fila);


    // Copiamos todo el texto hasta la siguiente area
    ULONG ulNumLineasDummy;
    ULONG ulNumLineasFin;
    int pos_inicio = BuscaString(fichero,"]",ulNumLineasDummy); //nos posicionamos al principio de este area
    int pos_fin = BuscaString(fichero,"[",ulNumLineasFin);    //nos posicionamos al final de este area
    ASSERT(pos_inicio>0);
    if (pos_inicio<=0) //no se ha encontrado el area esperada, probablemente el fichero este corrupto
    {
        printf("Error al procesar area %s del campo %d fila %d de la muestra %s\n",strNumArea, campo, fila, csMuestra);
        return;
    }
    if (pos_fin == -1)
    {
        // no se ha encontrado el inicio del siguiente area (probablemente sea el ultimo area)
        // establecemos el final de este area como el final del fichero
        fseek(fichero,0,SEEK_END);
        pos_fin = ftell(fichero) +1; //+1 porque luego se restara uno
                                        // probablemente sea una posicion fisica incorrecta mas alla del fin de linea, pero no importa
    }
    int bytes_to_read = pos_fin - pos_inicio - ulNumLineasFin -1;//-1 para no incluir "["
    fseek(fichero,pos_inicio,SEEK_SET); 
    char* buffer = new char[bytes_to_read];
    memset(buffer,bytes_to_read,0);
    int bytes_to_write = fread(buffer,1,bytes_to_read,fichero);
    //bytes_to_write puede ser menor que bytes_to_read en el caso del ultimo area 
    //porque fread reemplaza los pares retorno de carro, nueva linea con unicamente una nueva linea
    fwrite(buffer,1,bytes_to_write, salida);
    delete [] buffer;

}


//Devuelve el numero de areas validas en este fichero
//nValidAreaCount indica el area total actual. Se debe incrementar. La primera area es 0
int ProcesarFichero(CString csFichero, FILE* salida,int& nValidAreaCount, int& num_bandas_param)
{
    int eof;
    char* pString;
    ULONG pos;
    ULONG ulNumLineasDummy;

    int found = csFichero.Find("Cv",0);
    if (found != -1)
        found = 0;


	FILE*   fichero;
	if ((fichero = fopen(csFichero, "rt")) == NULL)
    {
        AfxMessageBox("No se puede abrir " + csFichero + strerror(errno));
		return -1;
    }

    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(fichero,"muestra =",ulNumLineasDummy);
    CString csMuestra;
    if (pos!=-1)
        csMuestra = LTrim(RTrim(ReadLine(fichero,eof)));

    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(fichero,"campo =",ulNumLineasDummy);
    int campo;
    if (pos!=-1)
    {
        pString = ReadLine(fichero,eof);
        campo = atoi(pString);
    }

    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(fichero,"fila =",ulNumLineasDummy);
    int fila;
    if (pos!=-1)
    {
        pString = ReadLine(fichero,eof);
        fila = atoi(pString);
    }

    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(fichero,"num_areas =",ulNumLineasDummy);
    int num_areas;
    if (pos!=-1)
    {
        pString = ReadLine(fichero,eof);
        num_areas = atoi(pString);
    }
    //buscamos (secuencialmente) y nos posicionamos depues de ...
    pos = BuscaString(fichero,"num_bandas =",ulNumLineasDummy);
    if (pos!=-1)
    {
        pString = ReadLine(fichero,eof);
        num_bandas_param = atoi(pString);
    }
    for (int i=0; i <num_areas; i++)
    {
        char strCount[3];
        itoa(i+1,strCount,10); // convertir contador a string 
        nValidAreaCount++; // primera area es 1
        ProcesarArea(fichero, strCount, salida, nValidAreaCount, csMuestra, campo, fila); //recuperamos y actualizamos datos
    }
    fclose(fichero);
    return num_areas;
}


/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es un fichero con un listado de directorios donde buscar (recursivamente) los ficheros de entrada
	 - Segundo es el nombre (y el camino) del fichero de salida
*****************************************************************************/
bool procesa_argumentos(int argc, TCHAR* argv[], CString& csFicheroDirectorios, CString& csDirectorioSalida)
{
	char	mensaje[1024];
	char	*fn = "procesa_argumentos";

	if (argc <= 2)  {
		sprintf(mensaje,
		"\a RecopilaDatos busca en un directorio ficheros con información espectral.\n"
		"\a y los compila en un archivo único de salida .\n"
		"\t Uso:  RecopilaDados.exe  fichero_entrada_directorios fichero_salida\n");
		
		CharToOemBuff(mensaje, mensaje, sizeof(mensaje));
		fprintf(stderr, "\n%s", mensaje);
		Sleep(3000);		// Espera para leer el mensaje que aparece en consola
		return false;
	}

	/*	El argumento de la linea de comandos es el directorio de patrones. */
	
	csFicheroDirectorios = argv[1];
	csDirectorioSalida = argv[2];

    return true;
}

void ProcesarDirectorio(char* pDirectorio, FILE* archivo, int& nValidFileCount, int& nValidAreaCount,int& num_bandas)
{
    CFileFinder	_finder;
    // Buscar todos los archivos de datos
	CFileFinder::CFindOpts	opts;
	// Set CFindOpts object
	opts.sBaseFolder = pDirectorio;
	opts.sFileMask.Format("*.pix");
	opts.bSubfolders = true;
	opts.FindNormalFiles();
	_finder.RemoveAll();
	_finder.Find(opts);
    // Fin de la busqueda

    // Para cada archivo, leer la informacion y volcarla al fichero general
    CString csFichero;
    int nAreasInFile = -1;
    for (int i=0 ; i<_finder.GetFileCount();i++)
    {
        csFichero = _finder.GetFilePath(i);
        nAreasInFile = ProcesarFichero(csFichero, archivo, nValidAreaCount, num_bandas); //nValidAreaCount se incrementa
        if (nAreasInFile > 0)
        {
            nValidFileCount++;
        }
    }
}

/**********************************************************************
// MAIN
***********************************************************************/
int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    CString csFicheroDirectorios;
    CString csDirectorioSalida;
    char* pString = NULL;
    int eof;
    int nValidFileCount = 0;
    int nValidAreaCount = 0;
    int num_bandas = 0;

	if ( procesa_argumentos(argc, argv,csFicheroDirectorios,csDirectorioSalida) == false)		
		return 1;

    //Abrir fichero de salida
	FILE*   archivo;
	if ((archivo = fopen(csDirectorioSalida, "wt")) == NULL)
		return 1;

    // Dejar espacio para resumen posterior
    fseek(archivo,170 + 7*2,SEEK_SET);


    // Leer fichero con directorios de entrada
	FILE*   fDirectorios;
	if ((fDirectorios = fopen(csFicheroDirectorios, "rt")) == NULL)
		return 1;

    pString = LTrim(RTrim(ReadLine(fDirectorios, eof)));
   
    while(eof == 0 && pString != NULL)
    {
        if (strcmp(pString,"")==0)
        {
            pString = LTrim(RTrim(ReadLine(fDirectorios, eof)));
            continue; //saltamos linea en blanco
        }
        ProcesarDirectorio(pString, archivo,nValidFileCount,nValidAreaCount, num_bandas);
        pString = LTrim(RTrim(ReadLine(fDirectorios, eof)));
    }

    fclose(fDirectorios);

    // Añadir resumen a archivo de salida
    fseek(archivo,0,SEEK_SET);
    fprintf(archivo, "# Información de reflectancia de todos los PIXELES de todos minerales\n");
    // Añadir fecha
    SYSTEMTIME st;
    GetSystemTime(&st);
    fprintf(archivo,"# %2d del %2d de %4d a las %2d:%2d\n" ,st.wDay,st.wMonth,st.wYear,st.wHour,st.wMinute);
    fprintf(archivo, "# Numero total de ficheros usados: %4d\n",nValidFileCount);
    fprintf(archivo, "\n");
    fprintf(archivo, "num_areas = %4d\n",nValidAreaCount);
    fprintf(archivo, "num_bandas = %2d\n",num_bandas);
    fprintf(archivo, "\n");

    fclose(archivo);

	return 0;
}


