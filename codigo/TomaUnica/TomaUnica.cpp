// TomaUnica.cpp : Aplicación de consola para la adquisición de una imagen de cada banda en una posicion dada
//

#include "stdafx.h"

#include <conio.h>
#include <math.h>
#include <direct.h> // Para mkdir

#include "..\librerias\include\procesoAux.h"
#include "..\librerias\include\controlDisp.h"

//#include "TomaUnica.h"
#include "..\librerias\LibBarrido\calibracion.h" //correccion de imagenes
#include "..\librerias\LibBarrido\CorreccionGeometrica.h" //correccion geometrica de imagenes
#include "..\librerias\LibBarrido\control_barrido.h"
//#include "adquisicion_imagenPat.h"
//#include "comprobacionPat.h"
//#include "perfiles.h"
#include "..\librerias\ProcesoAux\plnrgr_p.h" //para estructura point
#include "..\librerias\ControlRevolver\control_revolver_dta.h"
#include "..\librerias\ControlMicroscopio\control_objetivos.h" //actualizar_configuracion_objetivo

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MICROSCOPIO
#define MAX_NOMBRE_TOMA 50 //Longitud maxima del nombre de la toma que se pide al usuario

/*	Declaramos las estructuras que contienen la información esencial
	para el barrido de la preparación. */

limitesMtb	limMtb;				// Máximos y mínimos de los parámetros ++
limitesCam	limCam;				// parametros camara. Se rellena en control_proceso_imagenes.ini_control_digitalizador 
								// y se usa en ControlDisp.control_parametros.cpp
parametros	paramIni;				// Parámetros iniciales

DtaRevolver Rueda;           // Objeto revolver para el control de la rueda de filtros

extern MIL_ID	M_display_resul;		// Display de resultados.

/*	Declaramos los histogramas de trabajo. */

long histo_estimado[NUMVAL];				// Histograma estimado de la muestra
long histo_acumulado[NUMVAL];				// Histograma acumulado de la muestra
long histo_acum_granos[MAX_FILTROS_DTA+2][NUMVAL]; // Matriz para histogramas acumulados de los granos (_max, _min, _3, _4, _5)
long histo_grano[MAX_GRANOS][NUMVAL];		// Matriz para histogramas de cada grano de un campo
long histo_fil[NUMVAL];						// Histograma filtrado

char fichLog[LONGITUD_TEXTO];				// Fichero '*.log' de la aplicación

// Variables y funciones privadas a este modulo:
static int m_nAnchoAOI;

int lee_param(parametros *paramAdq);


/////////////////////////////////////////////////////////////////////////////
// The one and only application object

//CWinApp theApp;

//using namespace std;

char* procesa_argumentos(int argc, TCHAR* argv[], char* directorioToma);

/**************************  pideP  *********************************
Espera una tecla y la devuelve. Tambien devuelve la posicion cuando la tecla fue pulsada
AUXILIAR
SOLO VISUAL
*****************************************************************************/
int pideP(point *p)
{
	int t;
	char mensaje[LONGITUD_TEXTO];

	do {
		*p = mspWhereP();
		p->peso=1;
		sprintf(mensaje, "X: %8.2lf Y: %8.2lf Z: %8.2lf",
			p->coord[X_], p->coord[Y_], p->coord[Z_]);
		sendcont(mensaje);
		t = getKey();
		if (t == K_ESC)			break;
		Sleep(200);
	}while (t == 0);		// Espera hasta que no pulse una tecla

	return t;
}

/****************************************************************************
Escibe en consola el menu para la TomaUnica 
Espera una tecla y la devuelve. Tambien devuelve la posicion cuando la tecla fue pulsada
AUXILIAR
*****************************************************************************/
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
                /*
                // Control de posicionamiento fuera de ventana
                // Funciona pero no esta en uso
                long x_offset= -1,y_offset = -1, size_x = -1, size_y = -1;
                MdispInquire(M_display_resul, M_WINDOW_OFFSET_X, &x_offset);
                MdispInquire(M_display_resul, M_WINDOW_OFFSET_Y, &y_offset);
                MdispInquire(M_display_resul, M_SIZE_X, &size_x);
                MdispInquire(M_display_resul, M_SIZE_Y, &size_y);
                if (x<x_offset || x>x_offset+size_x ||
                    y<y_offset || y>y_offset+size_y   )
                {
                    //mouse fuera de la ventana
	                printf("Atencion: mouse fuera de la ventana\r");
                }
                else
                {
            
                    muestra_pixel(x-x_offset,y-y_offset,ROJO,0);
                }*/
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

void muestra_aoi(parametros& param)
{
	MgraColor(M_DEFAULT, M_COLOR_YELLOW);
    MgraRect(M_DEFAULT, M_overlay_normal, param.Cam.anchoImagen/2 - m_nAnchoAOI/2, param.Cam.altoImagen/2 - m_nAnchoAOI/2, 
                                          param.Cam.anchoImagen/2 + m_nAnchoAOI/2, param.Cam.altoImagen/2 + m_nAnchoAOI/2);
}

void calcula_y_guarda_AOI(FILE* fich_AOI, parametros& param, MIL_ID Mbanda)
{
    MIL_ID  M_Centro;
    long histo[NUMVAL];
    double media,sigma;
    static bool bPrimeraVez = true;

    if (bPrimeraVez) //primera vez de esta "sesion"
    {
        // Escribir cabecera en el fichero
        bPrimeraVez = false;

        // Get local time
        time_t rawtime;
        struct tm * timeinfo;
        time ( &rawtime );
        timeinfo = localtime ( &rawtime );

        fprintf(fich_AOI, "\n%s - Reflectancias medias y sigmas de regiones de interes\n",asctime (timeinfo));        
    }

	MbufChild2d(Mbanda, param.Cam.anchoImagen/2 - m_nAnchoAOI/2, param.Cam.altoImagen/2 - m_nAnchoAOI/2,
		m_nAnchoAOI, m_nAnchoAOI, &M_Centro);
    evalua_histograma(M_Centro,histo);
    MbufFree(M_Centro);
    procesa_histograma(histo,&media,&sigma);
    if (BITS_CAMARA == 12) 
        fprintf(fich_AOI, "%.2lf\t%.2lf\t",media*param.escala/16,sigma*param.escala/16);        
    else 
        fprintf(fich_AOI, "%.2lf\t%.2lf\t",media*param.escala,sigma*param.escala);        

}

/****************************************************************************
Escibe en consola el menu para pedir el nombre de la toma 
Devuelve lo leido (o añadido) en nombreToma
AUXILIAR
*****************************************************************************/
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

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
//	char  *raiz_paramIni;	// raiz del directorio del fichero inicial
	int nRetCode = 0;

	int i, c;
	int aplicaLutColor = 0;
	char mensaje[512], nombre_ext[512];
	FILE *fich;
    int nCampo = 0;
    char nombreToma[MAX_NOMBRE_TOMA] = "Toma"; //Para pedir el nombre de la toma y guardar las imagenes
    char directorioToma[512] = ""; //Directorio que crearemos para las imagenes de las tomas
    CCorreccionGeometrica correccionGeometrica;

    //  Procesamos los argumentos de la línea de comandos
 	procesa_argumentos(argc, argv,directorioToma);
   /*
	raiz_paramIni = procesa_argumentos(argc, argv,directorioToma);
	if ( raiz_paramIni==NULL || directorioToma == "")		
	{
		return (nRetCode = 1);
	}
	
    strcpy(paramIni.raiz_patrones,raiz_paramIni);
*/
	// Compongo el nombre de log de la aplicación
	sprintf(fichLog, "TomaUnica%s",EXT_LOG);
	if ( (fich = fopen(fichLog, "wt")) == NULL )  {
		sprintf(mensaje, "NO se puede crear el fichero: %s", fichLog);
		error_fatal("TomaUnica.exe", mensaje, 0);
		goto final;
	}
	fclose(fich);

	//	Inicialización del control del microscopio (MTB)
#ifdef MICROSCOPIO
	CoInitialize(NULL);					// Inicialización del servidor OLE
	//	Parámetros de adquisición relativos al microscopio
	sprintf(nombre_ext, "%s%s", FICH_PARAM_MICROSCOPIO, EXT_INI);
	if (lee_paramMicroscopio(nombre_ext, &paramIni) == -1)
    {
		AfxMessageBox("fallo al leer los parametros del microscopio");
        return false;
    }
	if ( ini_control_microscopio(paramIni.Mtb.rueda_filtros.posFiltro, paramIni.Mtb.voltLamp, &limMtb, 
		paramIni.Mtb.rueda_filtros.filtros[paramIni.Mtb.rueda_filtros.numFiltros-1]) )  
    {
		error_fatal("TomaUnica.exe", "fallo en la inicialización del MTB", 0);
		nRetCode = 1;
		goto final;
	}
	sprintf(mensaje, "Voltaje de la lámpara: %.3lf Posición de la rueda de filtros: %d\n",
		paramIni.Mtb.voltLamp, paramIni.Mtb.rueda_filtros.posFiltro);
	sendop(mensaje);
#endif

	// Lectura de las variables de inicialización
    // Tambien los parametros de correccion geometrica
	if (lee_param(&paramIni) < 0)
		goto final;
    correccionGeometrica.Leer(paramIni);
	/*	Nos aseguramos que los filtros sean positivos */
	for (i=0; i < paramIni.nBandas; i++)
		paramIni.filtro[i] = abs(paramIni.filtro[i]);
    paramIni.Rueda.posFiltro = abs(paramIni.Rueda.posFiltro);
	if ( paramIni.nBandas == 0 )  {
		error_fatal("TomaUnica.exe", "debe haber al menos una banda definida", 0);
		nRetCode = 1;
		goto final;
	}
    
	//	Inicialización de la librería de proceso de imágen (MIL)
	if ( ini_control_proceso_imagenes(&paramIni, SISTEMA_METEOR_II_1394, 0) )  {
		error_fatal("TomaUnica.exe", "fallo en la inicialización del proceso de imágenes", 0);
		nRetCode = 1;
		goto final;
	}
	
	// Deselecciono la ventana de resultados
	MdispDeselect(M_display_resul, M_resultados);

	//	Inicialización del control y configuración del digitalizador (cámara)
	if ( ini_control_digitalizador(&paramIni) )  {
		error_fatal("TomaUnica.exe", "fallo en la inicialización del proceso de digitalizador", 0);
		nRetCode = 1;
		goto final;
	}
    //HACER: EN TOMAPATRON ESTO ESTA ELIMINADO PORQUE EN REALIDAD SOLO SIRVE
    //PARA PROBAR QUE FUNCIONA Y FALLARÁ SI FINALMENTE SE INCLUYE DENTRO DE configura_digitalizador
    // LO DE DESELECCIONAR Y fin_toma
	if ( configura_digitalizador(FORMATO_NORMAL) )  {
		error_fatal("configura_digitalizador", "Formato incorrecto", 0);
		nRetCode = 1;
		goto final;
	}

	//	Inicio de la adquisición.
	if (aplicaLutColor)			// LUT falso color
		MdispLut(M_display_normal, M_PSEUDO);

	// prepara_correccion calcula las imagenes auxiliares (numerador y denominador) para cada banda
	// a partir de las imagenes de los patrones
	if (prepara_correccion(&paramIni) != 0)
	{
		error_fatal("TomaUnica.exe", "Fallo en prepara_correccion", 0);
		nRetCode = 1;
		goto final;
	}
    
	point punto;
	double zFoco, contraste;
	int tecla;
    if (PideNombreToma(nombreToma))
    {
	    //Inicializacion Revolver
	    Rueda.Init(&paramIni);

        //Construir directorio tomas
        //Aseguramos que el ultimo caracter sea '\'
        char* pdest;
        pdest = strrchr(directorioToma , '\\' );
        if ((int)pdest != (int)directorioToma + strlen(directorioToma) -1)
        {
            // el string no termina con '\', añadir '\'
            strcat(directorioToma,"\\");
        }
        //Creamos el directorio para esta serie de tomas
        strcat(directorioToma,nombreToma);
        strcat(directorioToma,"\\");

        if (mkdir(directorioToma) != 0 && errno != EEXIST && errno != 0)
        {
		    sprintf(mensaje, "NO se puede crear el directorio: %s", directorioToma);
		    error_fatal("TomaUnica.exe", mensaje, 0);
		    nRetCode = 1;
		    goto final;
        }
   
	    do  {
            nCampo ++;

            toma2_continua(M_imagen1);	// Adquisición en modo continuo
            MdispSelect(M_display_normal, M_imagen1); 
            configura_display_normal();

            if (paramIni.Cam.profundidad == 16)
            {
		        MdispLut(M_display_normal, M_PSEUDO); //esto es necesario para que la intensidad no sea anormalmente elevada
                MdispControl(M_display_normal,M_VIEW_BIT_SHIFT,4); //despreciamos los 4 bits menos significativos
            }
            muestra_aoi(paramIni);
            // Se ha de enfocar (tanto manual como automaticamente)
            // con el filtro "de enfoque" (o "de referencia", es decir el monocromatico)
            Rueda.ChangeFilter(paramIni.filtro[param.Rueda.posEnfoque]);
		    tecla = menuTomaUnica(punto);
		    if (tecla == 'a' || tecla == 'A')
		    {
			    if (autoenfoque(punto.coord[Z_],punto.coord[Z_],&zFoco,&contraste) == -1)
                    goto final;
			    mspGo(Z_,zFoco);
	            toma2_continua(M_imagen1);	// Adquisición en modo continuo. Porque autoenfoque lo deja en fija
				borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);
                muestra_aoi(paramIni);
		    }
		    else if (tecla == K_ESC)
		    {
                // actualizo la z absoluta del filtro predefinido (calculandola a partir del filtro de enfoque)
                Rueda.zPredefinido = mspWhere(Z_) + paramIni.Rueda.diferencia_z[paramIni.Rueda.posFiltro] - 
                                             paramIni.Rueda.diferencia_z[paramIni.filtro[param.Rueda.posEnfoque]]; 
                // Regresamos al filtro predefinido (o de usuario)
                Rueda.ChangeFilter(paramIni.Rueda.posFiltro);
			    goto final;
		    }
            else
            {
                zFoco = mspWhere(Z_);
            }
            // actualizo la z absoluta del filtro predefinido (calculandola a partir del filtro de enfoque)
            Rueda.zPredefinido = zFoco + paramIni.Rueda.diferencia_z[paramIni.Rueda.posFiltro] - 
                                         paramIni.Rueda.diferencia_z[paramIni.filtro[param.Rueda.posEnfoque]]; 

            // Una vez enfocado, regresamos al filtro predefinido (o de usuario)
            // Se cambiará correspondientemente la Z.
            Rueda.ChangeFilter(paramIni.Rueda.posFiltro);

            // CAPTURA. Resultado en M_banda[banda] en 8 o 16bit
		    captura_bandas(paramIni, paramIni.nImagenAcum,Rueda);

            // Correcion de las diferencias de posicion, escala, etc entre bandas
            correccionGeometrica.CorregirImagenes(M_banda, paramIni,-1, false);


		    //	Limpio el buffer de overlay
		    borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0, NULL, NULL);


		    fin_toma(); //camara

		    //	Una vez obtenida la imagen promedio se guarda esta en disco.
	        char nombre_AOI[512];
            FILE* fich_AOI = NULL;
	        sprintf(nombre_AOI, "%s%s%s", directorioToma, "ReflectanciasROIs", ".txt");
	        if ( (fich_AOI = fopen(nombre_AOI, "a+t")) == NULL )  {
		        sprintf(mensaje, "NO se puede crear el fichero: %s", nombre_AOI);
		        error_fatal("TomaUnica.exe", mensaje, 0);
	        }
		    for (i=0; i < paramIni.nBandas; i++)  {
			    sprintf(nombre_ext, "%s%s_%03d_%02d%s", directorioToma, nombreToma,nCampo,paramIni.filtro[i]+1, EXT_IMAG);
			    graba_imagen_campo_8bit(paramIni, nombre_ext, M_banda[i]); 
                if (paramIni.Cam.profundidad == 16)
                    //conversion a 12 bit. Para que luego los histogramas funcionen correctamente
                    MimShift(M_banda[i],M_banda[i],-4);

                if (fich_AOI != NULL)
                    calcula_y_guarda_AOI(fich_AOI,paramIni,M_banda[i]);
		    }
			sprintf(mensaje, "Imagenes de la toma %s promediadas y grabadas en disco", nombreToma);
		    sendop(mensaje);
		    printf("\n");
            if (fich_AOI != NULL)
                fprintf(fich_AOI, "\n");        
                fclose(fich_AOI);

		    //	Analizamos los valores (en nivel de gris) que contiene una linea horizontal
			//    o vertical de la imagen promediada del patrón. 
		    i = 0;
		    do  {
			    sprintf(mensaje, "Imagen BANDA: %d \t\t", i);
			    sendcont(mensaje);
			    MbufCopy(M_banda[i], M_imagen1);
			    c = getKey();
			    switch (c)  {
				    case K_UP:
					    i++;
					    if (i >= paramIni.nBandas)  {
						    putch('\a');
						    i = paramIni.nBandas-1;
					    }
					    sprintf(mensaje, "Imagen BANDA: %d \t\t", i);
					    sendcont(mensaje);
					    MbufCopy(M_banda[i], M_imagen1);
					    break;
				    case K_DOWN:
					    i--;
					    if (i < 0) {
						    putch('\a');
						    i = 0;
					    }
					    sprintf(mensaje, "Imagen BANDA: %d \t\t", i);
					    sendcont(mensaje);
					    MbufCopy(M_banda[i], M_imagen1);
					    break;
				    case K_CR:
					    //perfiles(M_imagen1, &paramIni);
					    break;
				    case ' ':
                        // M_imagen1 esta en 12 o 8 bits reales
					    construye_histo_base(NUMVAL);
					    sendcont("Pulse una tecla");
					    getch();
					    borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0,
						    paramIni.Cam.anchoImagen, paramIni.Cam.altoImagen);
					    break;
				    case 0:
					    Sleep(200);
					    break;
				    case K_ESC:
                        // actualizo la z absoluta del filtro predefinido (calculandola a partir del filtro de enfoque)
                        Rueda.zPredefinido = mspWhere(Z_); 
					    break;
				    default:
					    putch('\a');
			    }
		    
		    } while (c != K_ESC);
	    } while (true); //hasta que se presione ESC
    } //PideNombre == true (no se ha pulsado ESC)
final:
	fin_toma(); //camara
	fin_control_digitalizador();
	fin_control_proceso_imagenes();		// Finalizo el proceso de imágenes.
#ifdef MICROSCOPIO
	fin_control_microscopio();	// Finalizo el MTB
	CoUninitialize();							// Finalizo el servidor OLE
#endif
	return nRetCode;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
//	Función para leer los parámetros de adquisición 
//	Devuelve 0 si todo ha ido bien, -1 en otro caso
int lee_param(parametros *paramAdq)
{
	// Hay que declarar una variable por cada valor a leer porque la funcion auxiliar
	// que usamos (LOADINT, etc) usa el nombre de la variable tambien como string a buscar 
//	int apaga_lamp;
	char  nom_fich[512];
	int	 num_bandas, *filtros_bandas;

	char	*fn = "lee_Param";

	int		nResultado = 0;

	paramAdq->nBandas = 0;
    memset(paramAdq->filtro, sizeof(paramAdq->filtro), 0);

	//	Indico el nombre del archivo donde se encuentran los parámetros
	//	de toma de imágenes, así como del microscopio y la cámara.
	sprintf(nom_fich, "%s%s", FICH_PARAM_APLICACION, EXT_INI);

	if ( LoadVars(nom_fich) == NULL) {
		error_fatal(nom_fich, "El fichero indicado no se encuentra", 0);
		return -1;
	}

	//	Leemos el número de bandas que procesamos en el barrido.
	//	Si el número de bandas es NULL se procesa por defecto una
	//	sola banda. 
	LOADINT(num_bandas);
	if (num_bandas > MAX_FILTROS_DTA || num_bandas <= 0) {
		error_fatal(fn, "Valor de 'num_bandas' inaceptable", 1);
		return -1;
	}
	paramAdq->nBandas = num_bandas;

	filtros_bandas = paramAdq->filtro;
	LOADVINT(filtros_bandas, MAX_FILTROS_DTA);

	// En el fichero, los filtros van de 1 a 16, en memoria trabajamos de 0 a 15
	for (int i=0; i<num_bandas;i++)
	{
		if (paramAdq->filtro[i]<1 || paramAdq->filtro[i]>MAX_FILTROS_DTA)
		{
			error_leve(fn,"Se ha encontrado un filtro con valor fuera del rango 1..MAX_FILTROS_DTA en el fichero de configuracion");
			paramAdq->filtro[i] = 0;
		}
		else
		{
			paramAdq->filtro[i]--;
		}
	}

    double escala;
	if ( LOADDBL(escala) == 0.0 )
		escala = ESCALA_BASE;
	paramAdq->escala = escala;

    // Parametros barrido patrones
    int nImagenAcum = -1;
	LOADINT(nImagenAcum);
    paramAdq->nImagenAcum = nImagenAcum;

    // Ancho y alto del area de interes de la muestra
    int ancho_AOI = -1;
	LOADINT(ancho_AOI);
    m_nAnchoAOI = ancho_AOI;

	//Leemeos otros ficheros de inicializacion

		//	Parámetros de adquisición relativos a la camara
	sprintf(nom_fich, "%s%s", FICH_PARAM_CAMARA, EXT_INI);
	nResultado = nResultado + lee_paramCamara(nom_fich, paramAdq);

    // Antes de cargar los parametros relativos al objetivo, hay que saber en que objetivo estamos
    //corregimos la escala objetivo segun los aumentos en los que estemos
    bool bHayCambioObjetivo;
    if (!actualizar_configuracion_objetivo(paramAdq, bHayCambioObjetivo)) 
    {
		error_fatal(fn, "Error: No se ha encontrado el objetivo actual en la configuracion del microscopio o no se ha podido cargar el fichero de configuracion de revolver", 1);
        return -1;
    }

    return nResultado;
}


/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es el nombre (y el camino) del directorio de patrones.
	 - Segundo es el nombre (y el camino) del directorio donde guardar las imagenes.
*****************************************************************************/
char* procesa_argumentos(int argc, TCHAR* argv[], char* directorioToma)
{
	char	mensaje[1024];
	char	*fn = "procesa_argumentos";

	if (argc != 2)  {
		sprintf(mensaje,
		"\aToma una unica imagen de una muestra.\n"
		"\t Uso:  TomaUnica.exe  directorio_trabajo\n"
		"Los argumentos admitidos son:\n"
		"\t directorio_trabajo: ruta completa al directorio donde se guardan las imagenes\n");
		
		CharToOemBuff(mensaje, mensaje, sizeof(mensaje));
		fprintf(stderr, "\n%s", mensaje);
		Sleep(3000);		// Espera para leer el mensaje que aparece en consola
		return NULL;
	}

    //El segundo argumento es el directorio de trabajo
    strcpy(directorioToma, argv[1]);

	//	El argumento de la linea de comandos es el directorio de patrones.
//	return argv[1];
    return NULL;
}


