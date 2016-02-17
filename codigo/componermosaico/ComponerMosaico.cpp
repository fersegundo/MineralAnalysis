// ComponerMosaico.cpp : Aplicación de consola para la componer un mosaico a partir de un barrido
//

#include "stdafx.h"
#include "myVars.h"
#include "FileFinder.h"
#include "..\librerias\LibTiff\InterfazLibTiff.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Configuaracion composicion mosaico
CString g_csDirectorioBarrido;
int  g_nFilaInicio      = -1;
int  g_nColumnaInicio   = -1;
int  g_nFilaFin         = -1;
int  g_nColumnaFin      = -1;

// Variables del barrido
CString g_csMuestra; //nombre de la muestra
int g_nCampos_x = -1;
int g_nCampos_y = -1;
int g_nBandas   = -1;
int g_arrFiltros[16];



/****************************************************************************
Escibe en consola el menu para la TomaUnica 
Espera una tecla y la devuelve. Tambien devuelve la posicion cuando la tecla fue pulsada
AUXILIAR
int menuTomaUnica(point& punto)
{
	char mensaje[LONGITUD_TEXTO*2];

	sprintf(mensaje,"\n+-  TOMA UNICA   ------------------------------------------+");
	strcat(mensaje, "\n| Posiciónese en la zona donde quiera tomar las imagenes   |");
	strcat(mensaje, "\n| Presione [A] para realizar enfoque automatico            |");
	strcat(mensaje, "\n| Presione [ENTER] proceder a la toma sin autoenfoque      |");
	strcat(mensaje, "\n| [Esc] - Para salir de la aplicacion                      |");
	strcat(mensaje, "\n+----------------------------------------------------------+\n");
	CharToOem(mensaje,mensaje);
	printf(mensaje);

	int tecla;
    bool bDefiniendoArea = false;
    float x = 0;
    float y = 0;
	do{
		tecla = pideP(&punto);
        if (tecla == ' ')
        {
            // Establecimiento de areas/areas de interes mediante raton
            POINT cursorPos;
            GetCursorPos(&cursorPos);
            if (bDefiniendoArea)
            {
                muestra_pixel(x,y,NULL,1); //borramos el punto inicial
	            MgraColor(M_DEFAULT, M_COLOR_YELLOW);
               	MgraRect(M_DEFAULT, M_overlay_normal, x, y, cursorPos.x, cursorPos.y);

            }
            else
            {
                x = cursorPos.x; 
                y = cursorPos.y;
                muestra_pixel(x,y,ROJO,0);
            }
            bDefiniendoArea = !bDefiniendoArea;

        }
        else if (bDefiniendoArea == true)
        {
            muestra_pixel(x,y,NULL,1); //borramos el punto inicial
        }

	} while (tecla != K_ESC && tecla != 'a' && tecla != 'A' && tecla != K_CR);

	return tecla;
}
*****************************************************************************/


/****************************************************************************
Escibe en consola el menu para pedir el nombre de la toma 
Devuelve lo leido (o añadido) en nombreToma
AUXILIAR
bool PideNombreToma(char* nombreToma)
{
	char mensaje[LONGITUD_TEXTO*2];

	sprintf(mensaje,"\n+-  TOMA UNICA   ------------------------------------------+");
	strcat(mensaje, "\n| Introduzca el nobre de la toma                           |");
	strcat(mensaje, "\n| Presione [ENTER] para aceptar el nombre actual           |");
	strcat(mensaje, "\n| [Esc] - Para salir de la aplicacion                      |");
	strcat(mensaje, "\n+----------------------------------------------------------+\n");
	strcat(mensaje, nombreToma);
	CharToOem(mensaje,mensaje);
	printf(mensaje);

    // Traer consola al frente para que no este tapada por la imagen
    HWND console = FindWindow("ConsoleWindowClass",NULL);
    BringWindowToTop(console);

    int pos = strlen(nombreToma);
	int tecla;
	do{
		tecla = getKey();
		if (tecla == K_ESC)	
        {
            return false;
        }
        else if (tecla == K_DEL || tecla == K_BS)
        {
            if (pos!=0)
            {
                printf("\b \b");
                pos--;
                nombreToma[pos] = '\0';
            }
        }
        else if (tecla == 0)
        {}
        else if (tecla != K_CR && pos < MAX_NOMBRE_TOMA)
        {
            printf("%c",tecla);
            nombreToma[pos] = tecla;
            pos++;
        }
		Sleep(200);
	} while (tecla != K_CR);
    
    //Apartar la consola para que no tape la imagen
    RECT rect;
    GetWindowRect(console,&rect);
    SetWindowPos(console,HWND_TOP,paramIni.Cam.anchoImagen,rect.top,0,0,SWP_NOSIZE | SWP_NOZORDER);

    return true;
}
*****************************************************************************/

/*
/////////////////////////////////////////////////////////////////////////////////////////////////
//	Función para leer los parámetros de adquisición 
//	Devuelve 0 si todo ha ido bien, -1 en otro caso
int lee_param()
{
	// Hay que declarar una variable por cada valor a leer porque la funcion auxiliar
	// que usamos (LOADINT, etc) usa el nombre de la variable tambien como string a buscar 
//	int apaga_lamp;
	char  nom_fich[512];

	char	*fn = "lee_Param";

	int		nResultado = 0;


	//	Indico el nombre del archivo donde se encuentran los parámetros
	//	de toma de imágenes, así como del microscopio y la cámara.
	sprintf(nom_fich, "%s%s", "parametros_aplicacion", ".ini");

	if ( LoadVars(nom_fich) == NULL) {
		printf("El fichero de configuracion %s no se encuentra", nom_fich);
        getch();
		return -1;
	}

	//	Leemos el directorio
    char * directorio;
    directorio = g_strDirectorioBarrido;
	LOADSTR(directorio);

    // Leemos el inicio y el fin
    int  fila_inicio;
    int  columna_inicio;
    int  fila_fin;
    int  columna_fin;

	LOADINT(fila_inicio);
    g_nFilaInicio      = fila_inicio;
	LOADINT(columna_inicio);
    g_nColumnaInicio   = columna_inicio;
	LOADINT(fila_fin);
    g_nFilaFin         = fila_fin;
	LOADINT(columna_fin);
    g_nColumnaFin      = columna_fin;
   
    return nResultado;
}
*/

//Leemos el fichero de informacion de barrido del directorio elegido y mostramos la informacion de barrido
bool LeerInfoBarrido(CString csDirectorio)
{
    //Primero, debemos buscar el fichero de informacion (no sabemos como se llama porque no sabemos cual
    CFileFinder	_finder;
    CString csExtInfoBarrido = "_INFO_BARRIDO.txt";

	CFileFinder::CFindOpts	opts;
	opts.sBaseFolder = csDirectorio;
	opts.sFileMask.Format("*" + csExtInfoBarrido);
	opts.bSubfolders = false;
	opts.FindNormalFiles();
	_finder.RemoveAll();
	_finder.Find(opts); //Busqueda

    CString csFichero;
    CString csError;

    if (_finder.GetFileCount() <1) return false; //no hay informacion de barrido

    csFichero = _finder.GetFilePath(0);
    if (_finder.GetFileCount() > 1)
        printf("ATENCION: se han encontrado varios archivos de información de barrido, se usará %s", csFichero);

    //Lee el fichero
    FILE* f_info_barrido = fopen(csFichero, "rt");

    char strDummy[500]; //para saltarse la cabecera
//    fscanf(f_info_barrido,"%[^\n]",strDummy); //cabecera
    fgets(strDummy,500,f_info_barrido);
    fscanf(f_info_barrido,"%dx%d",&g_nCampos_x,&g_nCampos_y);
    fscanf(f_info_barrido,"%d",&g_nBandas);
    for (int b=0;b<g_nBandas;b++)
        fscanf(f_info_barrido,"%d\t",&g_arrFiltros[b]); //se leen los filtros (1..N) correspondientes a cada banda

    fclose(f_info_barrido);

    g_csMuestra = csFichero.Mid(csDirectorio.GetLength(), csFichero.GetLength() - csDirectorio.GetLength() - csExtInfoBarrido.GetLength());

    return true;   
}

/**************************  graba_imagen_campo  ****************************
   Función para efectuar la grabación en disco de una imagen del campo.
   "nombre" es el nombre del archivo a almacenar.
   "buffer" es el buffer que quiero copiar a disco.
****************************************************************************/
int graba_imagen_campo(char *nombre, MIL_ID buffer)
{
	char  nombre_ext[1024], *ext, mensaje[1024], *fn = "graba_imagen_campo";

	strcpy(nombre_ext, nombre);		// Se copia en nombre_ext el nombre de la imagen a cargar
	ext = strrchr(nombre_ext, '.');	// Se busca la extensión del fichero que se desea guardar
	strcpy(nombre_ext, ext+1);

	// Copiamos el buffer seleccionado en el archivo seleccionado (nombre_ext).
	if ( strcmp(nombre_ext,"jpg")==0 )  MbufExport(nombre, M_JPEG_LOSSLESS, buffer);
	if ( strcmp(nombre_ext,"tif")==0 )
    {
        MbufExport(nombre, M_TIFF, buffer);
    }
	if ( strcmp(nombre_ext,"mim")==0 )  MbufExport(nombre, M_MIL, buffer);

	/* Compruebo que no ha habido ningún error durante el proceso de grabación a disco. */

	if ( MbufDiskInquire(nombre, M_SIZE_X, M_NULL) == 0)  {
		sprintf(mensaje, "No puedo crear la imagen %s", nombre_ext);
		printf(mensaje);
		return 1;
	}

	return  0;
}

/**************************  carga_imagen_campo  ****************************
	Función para efectuar la carga desde disco de una imagen del campo.
	Se le pasan a esta función los siguientes argumentos:
		nombre: nombre del fichero que quiero cargar.
		buffer: le paso el buffer donde quiero almacenar la imagen a cargar.
				Esto define el tamaño del mismo.
	Esta función únicamente carga la imagen en el buffer seleccionado, no la
	muestra en el display.
****************************************************************************/
int carga_imagen_campo(char *nombre, MIL_ID buffer)
{
	FILE *fich;
#if !defined (__BORLANDC__)
	char  mensaje[1024];
    char *fn = "carga_imagen_campo";
#endif

	/*	Compruebo que la imagen solicitada (el archivo) existe. */

	fich = fopen(nombre, "rb");
	if (fich == NULL)  {
#if !defined (__BORLANDC__)
		sprintf(mensaje, "No puedo leer la imagen %s", nombre);
		printf(fn, mensaje);
#endif
		return -1;
	}
	fclose(fich);

	//	Si el archivo existe, lo importo, almacenándolo en el buffer solicitado.
	MbufImport(nombre, M_DEFAULT, M_LOAD, M_NULL, &buffer);

	return  0;
}

/**************************  carga_imagen_campo  ****************************
	Función para efectuar la carga desde disco de una imagen del campo.
	Se le pasan a esta función los siguientes argumentos:
		nombre: nombre del fichero que quiero cargar.
		buffer: le paso el buffer donde quiero almacenar la imagen a cargar.
				Esto define el tamaño del mismo.
        nProfundidad: profundidad (bits) del buffer (8,12,16)
	Esta función únicamente carga la imagen en el buffer seleccionado, no la
	muestra en el display.
    LEE LA PROFUNDIDAD GUARDADA, EL BUFFER ESTA EN nProfundidad
****************************************************************************/
int carga_imagen_campo_bits(char *nombre, MIL_ID buffer, int nProfundidad)
{
    // Leer informacion TIF (escala reflectancia y profundidad)
    int nProfundidadFichero = -1;
    double dEscalaFichero;
    try
    {
        CString csNombreImagen;
        INTERFAZ_LIBTIF::LeerTags(nombre,nProfundidadFichero,dEscalaFichero); //Leemos de la imagen la profundidad en bits y la escala reflectancia-gris
        if (nProfundidadFichero < 0 )
            nProfundidadFichero = 8; // por defecto las imagenes estan en 8 bit
    }
    catch(...)
    {
        nProfundidadFichero = 8;
    }

    if (nProfundidad != nProfundidadFichero) 
    {
        // Buffer auxiliar de 16 es necesario para poder operar (M_PROC)
        MIL_ID M_buf16;
        long altoImagen,anchoImagen;
        MbufInquire(buffer, M_SIZE_X, &anchoImagen);
        MbufInquire(buffer, M_SIZE_Y, &altoImagen);
	    MbufAlloc2d(M_DEFAULT_HOST, anchoImagen, altoImagen, 16+M_UNSIGNED,
		    M_IMAGE+M_DISP+M_PROC, &M_buf16 );
        if (nProfundidad >= 12 && nProfundidadFichero >= 12) 
        {
            if (carga_imagen_campo(nombre,M_buf16))
                return 1;//ERROR
            MimShift(M_buf16,buffer,nProfundidad - nProfundidadFichero);//conversion de 12 a 16 bit 
        }
        else if (nProfundidadFichero == 8) //nProfundidad>8
        {
            MIL_ID M_buf8;
	        MbufAlloc2d(M_DEFAULT_HOST, anchoImagen, altoImagen, 8+M_UNSIGNED,
		        M_IMAGE+M_DISP+M_PROC, &M_buf8 );

            if (carga_imagen_campo(nombre,M_buf8))
                //ERROR
                return 1;

	        MbufCopy(M_buf8, M_buf16); 
            MbufFree(M_buf8);
            MimShift(M_buf16,buffer,nProfundidad - nProfundidadFichero);//conversion de 8 a 12 bit 
        }
        else //(nProfundidad == 8 && nProfundidadFichero > 8
        {
            if (carga_imagen_campo(nombre,M_buf16))
                //ERROR
                return 1;

            MimShift(M_buf16,buffer,nProfundidad - nProfundidadFichero);//conversion de 8 a 12 bit 
        }
        MbufFree(M_buf16);
    }
    else
    {
        if (carga_imagen_campo(nombre,buffer))
            //ERROR
            return 1;
    }

	return  0;
}

// A partir de las imagenes guardadas en disco, genera la imagen mosaico
void GenerarMosaicos()
{
	char nombre_fich[512];
    MIL_ID M_aux;
    MIL_ID M_mosaico;
    long lSizeX, lSizeY; //dimensiones imagen


    // Leer tamaño de primera imagen
    sprintf(nombre_fich, "%s%s_%03d_%03d_%02d%s", g_csDirectorioBarrido, g_csMuestra, g_nFilaInicio, g_nColumnaInicio, g_arrFiltros[0]+1, ".tif");
    MbufDiskInquire(nombre_fich,M_SIZE_X, &lSizeX);
    MbufDiskInquire(nombre_fich,M_SIZE_Y, &lSizeY);

	MbufAlloc2d(M_DEFAULT_HOST, lSizeX, lSizeY, 8/*theApp.m_ParamIni.Cam.profundidad*/+M_UNSIGNED,
		M_IMAGE+M_PROC, &M_aux);
	MbufAlloc2d(M_DEFAULT_HOST, lSizeX * (g_nColumnaFin - g_nColumnaInicio + 1), lSizeY * (g_nFilaFin - g_nFilaInicio + 1), 8/*theApp.m_ParamIni.Cam.profundidad*/+M_UNSIGNED,
		M_IMAGE+M_PROC, &M_mosaico);
	for (int f=0; f < g_nBandas; f++)  
    {
        MbufClear(M_mosaico,0);
        for (int i=g_nFilaInicio;i<=g_nFilaFin;i++)
        {
            printf("\rComponiendo mosaico filtro %d  Fila %d ...    ", g_arrFiltros[f], i);
            for (int j=g_nColumnaInicio;j<=g_nColumnaFin;j++)
            {
		        sprintf(nombre_fich, "%s%s_%03d_%03d_%02d%s", g_csDirectorioBarrido, g_csMuestra, i, j, g_arrFiltros[f], ".tif");
		        //carga_imagen_campo8bit(nombre_fich, M_aux, theApp.m_ParamIni.Cam.profundidad); 
		        carga_imagen_campo_bits(nombre_fich, M_aux,8); 
                MbufCopyClip(M_aux,M_mosaico,(j-g_nColumnaInicio)*lSizeX,(i-g_nFilaInicio)*lSizeY);
            }
        }
        printf("\rSalvando mosaico filtro %d ...                      ", g_arrFiltros[f]);
        sprintf(nombre_fich, "%s%s_%02d%s", g_csDirectorioBarrido, g_csMuestra, g_arrFiltros[f], ".tif");
		graba_imagen_campo(nombre_fich, M_mosaico); 

    }
    printf("\r                                                         \n");

	MbufFree(M_aux);
	MbufFree(M_mosaico);
}

/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es el nombre (y el camino) del directorio de patrones.
	 - Segundo es el nombre (y el camino) del directorio donde guardar las imagenes.
*****************************************************************************/
bool procesa_argumentos(int argc, TCHAR* argv[])
{
	char	mensaje[1024];
	char	*fn = "procesa_argumentos";

	if (argc != 2 && argc != 6 )  {
		sprintf(mensaje,
		"\a ERROR. Numero incorrecto de argumentos\n"
		"\t Uso:              ComponerMosaico.exe  directorio_barrido - se intentará componer el mosaico con el barrido completo\n"
		"\t Uso alternativo:  ComponerMosaico.exe  directorio_barrido fila_inicio columna_inicio fila_final columna final\n");
		
		CharToOemBuff(mensaje, mensaje, sizeof(mensaje));
		fprintf(stderr, "\n%s", mensaje);
		getch();
		return false;
	}

    //El segundo argumento es el directorio de trabajo
    g_csDirectorioBarrido = argv[1];

    if (argc == 6)
    {
        g_nFilaInicio       = atoi(argv[2]);
        g_nColumnaInicio    = atoi(argv[3]);
        g_nFilaFin          = atoi(argv[4]);
        g_nColumnaFin       = atoi(argv[5]);

        // Comprobaciones
        if ( g_nFilaInicio > g_nFilaFin || g_nFilaInicio < 1 ||
            g_nColumnaInicio > g_nColumnaFin || g_nColumnaInicio < 1 )
        {
            printf("Error: Inicio de mosaico fuera de limites \n");
		    return false;
        }
    }
    else
    {
        g_nFilaInicio       = -1;
        g_nColumnaInicio    = -1;
        g_nFilaFin          = -1;
        g_nColumnaFin       = -1;
    }

	//	El argumento de la linea de comandos es el directorio de patrones.
//	return argv[1];

    return true;
}



int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
//	char  *raiz_paramIni;	// raiz del directorio del fichero inicial
	int nRetCode = 0;


    /*
	// Compongo el nombre de log de la aplicación
	sprintf(fichLog, "TomaUnica%s",EXT_LOG);
	if ( (fich = fopen(fichLog, "wt")) == NULL )  {
		sprintf(mensaje, "NO se puede crear el fichero: %s", fichLog);
		error_fatal("TomaUnica.exe", mensaje, 0);
		goto final;
	}
	fclose(fich);
*/
    //Inicia MIL
    MIL_ID Milaplicacion;
	MappAlloc(M_DEFAULT, &Milaplicacion);	// Selecciono la aplicación MIL.
	if ( Milaplicacion == M_NULL ) {
        AfxMessageBox("Error: No se pudo inicilizar MIL", MB_ICONERROR | MB_OK);
	}

	// Lectura de las variables de inicialización
    // Tambien los parametros de correccion geometrica
//	if (lee_param() < 0)
//		goto final;
    
    //  Procesamos los argumentos de la línea de comandos
 	if (!procesa_argumentos(argc, argv))
    {
        MappFree(Milaplicacion);
        exit(0);
    }

    // Nos aseguramos de que el ultimo caracter sea '\'
    CString csUltimoCaracter = g_csDirectorioBarrido.Right(1);
    if (csUltimoCaracter != "\\" && csUltimoCaracter != "/")
        g_csDirectorioBarrido += _T("\\");

    if (!LeerInfoBarrido(g_csDirectorioBarrido))
    {
        printf("Error: no se ha encontrado el fichero de informacion de barrido\n");
        MappFree(Milaplicacion);
        exit(0);
    }

  	printf("\n+-  COMPONER MOSAICO---------------------------------------");
	printf("\n| Directorio: %s",g_csDirectorioBarrido);
    if (g_nFilaInicio > 0)
    {
	    printf("\n| Inicio: %d,%d     Fin: %d,%d",g_nFilaInicio,g_nColumnaInicio,g_nFilaFin,g_nColumnaFin);

        // Comprobaciones
        if (g_nFilaFin > g_nCampos_y || g_nColumnaFin > g_nCampos_x)
        {
            printf("Error: Fin de mosaico fuera de limites (%d,%d)\n",g_nCampos_y, g_nCampos_x );
            MappFree(Milaplicacion);
            exit(0);
        }
    }
    else
    {
	    printf("\n| Completo (%d x %d)",g_nCampos_x, g_nCampos_y);
        g_nFilaInicio      = 1;
        g_nColumnaInicio   = 1;
        g_nFilaFin         = g_nCampos_y;
        g_nColumnaFin      = g_nCampos_x;
    }
	printf("\n+----------------------------------------------------------\n");


    GenerarMosaicos();

    /*
	int tecla;
    if (PideNombreToma(nombreToma))
    {

    } //PideNombre == true (no se ha pulsado ESC)
    */


    MappFree(Milaplicacion);

    return nRetCode;
}


