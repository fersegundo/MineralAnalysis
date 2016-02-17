// TomaPatron.cpp : Aplicación de consola, para la adquisición de los patrones
//

#include "stdafx.h"

#include <conio.h>
#include <math.h>

#include "..\librerias\LibBarrido\correccionGeometrica.h"
#include "..\librerias\LibBarrido\calibracion.h"
#include "..\librerias\include\procesoAux.h"
#include "..\librerias\include\controlDisp.h"
#include "..\Librerias\ControlRevolver\control_revolver_dta.h"
#include "..\Librerias\ControlMicroscopio\control_objetivos.h"

#include "adquisicion_imagenPat.h"
#include "comprobacionPat.h"
#include "perfiles.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

#define MICROSCOPIO

/*	Declaramos las estructuras que contienen la información esencial
	para el barrido de la preparación y/o el análisis petrográfico. */

limitesMtb	limMtb;				// Máximos y mínimos de los parámetros ++
limitesCam	limCam;				// parametros camara. Se rellena en control_proceso_imagenes.ini_control_digitalizador 
								// y se usa en ControlDisp.control_parametros.cpp
parametros	paramIni;				// Parámetros iniciales
DtaRevolver Rueda;           // Objeto revolver para el control de la rueda de filtros

/*	Declaramos los histogramas de trabajo. */

long histo_estimado[NUMVAL];				// Histograma estimado de la muestra
long histo_acumulado[NUMVAL];				// Histograma acumulado de la muestra
long histo_acum_granos[MAX_FILTROS_DTA+2][NUMVAL]; // Matriz para histogramas acumulados de los granos (_max, _min, _3, _4, _5)
long histo_grano[MAX_GRANOS][NUMVAL];		// Matriz para histogramas de cada grano de un campo
long histo_fil[NUMVAL];						// Histograma filtrado

char fichLog[LONGITUD_TEXTO];				// Fichero '*.log' de la aplicación

TomaAutomatica_T TomaAutomatica;

extern MIL_ID	M_display_resul;		// Display de resultados.

// Funciones privadas:
int procesa_argumentos(int argc, TCHAR* argv[], char* raiz_paramIni, char** tipo_patron);
bool CompruebaValidezParam(parametros* param);
int menu();
bool menu_correccion_imagen(parametros* pParamIni);
int lee_param(parametros *paramAdq);

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
    char nombre_aplicacion[20] = "TomaPatron.exe";
	char  *tipo_patron = NULL;		// Tipo de patron: claro(CL); oscuro(OS) que se desea adquirir.
	int nRetCode = 0;
    // variable que indica la necesidad de ajustar automaticamente las exposiciones justo antes de comenzar el barrido del patron (en caso de que no se haya hecho manualmente antes)
    bool bExposicionAjustada = false;

	int i, c;
	char mensaje[512], nombre_ext[512];
	FILE *fich;

	//  Procesamos los argumentos de la línea de comandos
//	if (procesa_argumentos(argc, argv, paramIni.raiz_patrones, &tipo_patron)==0 )		return (nRetCode = 1);
	
	// Compongo el nombre de log de la aplicación
	sprintf(fichLog, "TomaPatron%s", EXT_LOG);
	if ( (fich = fopen(fichLog, "wt")) == NULL )  {
		sprintf(mensaje, "NO se puede crear el fichero: %s", fichLog);
		error_fatal(nombre_aplicacion, mensaje, 0);
		goto final;
	}
	fclose(fich);

	//	Inicialización del control del microscopio (MTB)
#ifdef MICROSCOPIO
	CoInitialize(NULL);					// Inicialización del servidor COM
	//	Parámetros de adquisición relativos al microscopio
	sprintf(nombre_ext, "%s%s", FICH_PARAM_MICROSCOPIO, EXT_INI);
	if (lee_paramMicroscopio(nombre_ext, &paramIni) == -1)
    {
		AfxMessageBox("fallo al leer los parametros del microscopio");
        return false;
    }
	if (paramIni.Mtb.rueda_filtros.numFiltros == 0)
	{
		if ( ini_control_microscopio(paramIni.Mtb.rueda_filtros.posFiltro, paramIni.Mtb.voltLamp, &limMtb, 
			paramIni.Mtb.rueda_filtros.posFiltro) )  {
			error_fatal(nombre_aplicacion, "fallo en la inicialización del MTB", 0);
			nRetCode = 1;
	        CoUninitialize();							// Finalizo el servidor OLE
	        return nRetCode;
		}
	}
	else
	{
		if ( ini_control_microscopio(paramIni.Mtb.rueda_filtros.posFiltro, paramIni.Mtb.voltLamp, &limMtb, 
			paramIni.Mtb.rueda_filtros.filtros[paramIni.Mtb.rueda_filtros.numFiltros-1]) )  {
			error_fatal(nombre_aplicacion, "fallo en la inicialización del MTB", 0);
			nRetCode = 1;
	        CoUninitialize();							// Finalizo el servidor OLE
	        return nRetCode;
		}
	}
	sprintf(mensaje, "Voltaje de la lámpara: %.3lf Posición de la rueda de filtros: %d\n",
		paramIni.Mtb.voltLamp, paramIni.Mtb.rueda_filtros.posFiltro);
	sendop(mensaje);
#endif

	// Lectura de las variables de inicialización
	if (lee_param(&paramIni) > 0) // < !!!!
		goto final;
	if ( paramIni.nBandas == 0 )  {
		error_fatal(nombre_aplicacion, "debe haber al menos una banda definida", 0);
		nRetCode = 1;
	    CoUninitialize();							// Finalizo el servidor OLE
	    return nRetCode;
	}

	//Inicializacion Revolver
	Rueda.Init(&paramIni);

	//	Inicialización de la librería de proceso de imágen (MIL)
	if ( ini_control_proceso_imagenes(&paramIni, SISTEMA_METEOR_II_1394, 0) )  {
		error_fatal(nombre_aplicacion, "fallo en la inicialización del proceso de imágenes", 0);
		nRetCode = 1;
		goto final;
	}
	
	// Deselecciono la ventana de resultados
	MdispDeselect(M_display_resul, M_resultados);


	//	Inicialización del control y configuración del digitalizador (cámara)

        // Tomaremos los patrones en formato maximo independientemente de lo configurado
	    //	Configuro el digitalizador en formato normal. 
	    if (M_digitalizador != M_NULL)
		    libera_digitalizador();
	    if ( MdigAlloc (M_sistema, M_DEV0, paramIni.Cam.formatoDig, M_DEFAULT, &M_digitalizador)
		    == M_NULL )  {
		    error_fatal(nombre_aplicacion, "formato digitalizador incorrecto", 0);
		    nRetCode = 1;
		    goto final;
	    }
	    MdigInquire(M_digitalizador, M_SIZE_X, &limCam.anchoDig);
	    MdigInquire(M_digitalizador, M_SIZE_Y, &limCam.altoDig);
        paramIni.Cam.anchoImagen = limCam.anchoDig; 
        paramIni.Cam.altoImagen = limCam.altoDig; 
	if ( ini_control_digitalizador(&paramIni) )  {
		error_fatal(nombre_aplicacion, "fallo en la inicialización del proceso de digitalizador", 0);
		nRetCode = 1;
		goto final;
	}

    // Comprueba alguno valores de configuracion especificos para TomaPatron
    if (CompruebaValidezParam(&paramIni) == false) {
		error_fatal(nombre_aplicacion, "error en los parametros de configuracion", 0);
		nRetCode = 1;
		goto final;
	}

    // No necesario. Solo para comprobar que funciona
	if ( configura_digitalizador(FORMATO_NORMAL) )  {
		error_fatal("configura_digitalizador", "Formato incorrecto", 0);
		nRetCode = 1;
		goto final;
	}

	//	Inicio de la adquisición de los patrones. 
	toma2_continua(M_imagen1);	// Adquisición en modo continuo
    MdispSelect(M_display_normal, M_imagen1); 
    configura_display_normal();

    if (TomaAutomatica.bAutomatica == false)
    {
        // Traer consola al frente para que no este tapada por la imagen
        HWND console = FindWindow("ConsoleWindowClass",NULL);
        BringWindowToTop(console);
        //Apartar la consola para que no tape la imagen
        RECT rect;
        GetWindowRect(console,&rect);
        SetWindowPos(console,HWND_TOP,paramIni.Cam.anchoImagen,rect.top,0,0,SWP_NOSIZE | SWP_NOZORDER);

        // Menu

        int tecla = -1;
            
        do
        {
            tecla = menu();

            if (tecla == '1')
            {
                // Comprobación de la necesidad de recalibrar el sistema
                if (compruebaCalibracion(&paramIni) == 'q')
                {
		            fin_toma();
                    goto final; //QUIT
                }
            }
            else if (tecla == '2' || tecla == '3')
            {
                if (tecla == '2')
                    tipo_patron = "PAT_CL";
                else
                    tipo_patron = "PAT_OS";

                // Función que permite modificar y guardar los distintos parámetros iniciales
                if (control_parametros(M_digitalizador, &paramIni, tipo_patron, bExposicionAjustada) == 'q')
                {
		            fin_toma();
                    goto final; //QUIT
                }
            }
            else if (tecla == '4')
            {
                // Correccion de la imagen mediante el POLKA-DOT
                if (!menu_correccion_imagen(&paramIni))
                {
		            fin_toma();
                    goto final; //QUIT
                }
            }
            else
            {
		        fin_toma();
                goto final; //QUIT
            }
        } while (tecla != '2' && tecla != '3'); //El menu se repite hasta que hay que tomar uno de los patrones oscuro o claro
    }

    ////////////////////////////////////////////////////////////////////
    // adquisicion 
    ////////////////////////////////////////////////////////////////////
	if ( adquierePat(&paramIni, tipo_patron, bExposicionAjustada) < 0)  {
		fin_toma();
		goto final;
	}
	fin_toma();

	//	Una vez obtenida la imagen promedio se guarda esta en disco con
	//	la extensión y nombre del patrón seleccionado inicialmente. 
	for (i=0; i < paramIni.nBandas; i++)  {
		sprintf(nombre_ext, "%s%s_%02d%s", paramIni.raiz_patrones, tipo_patron, paramIni.filtro[i]+1, EXT_PAT);
		graba_imagen_campo(nombre_ext, M_banda[i]);
	}
	sprintf(mensaje, "Imagenes del patron %s promediadas y grabadas en disco", tipo_patron);
	sendop(mensaje);
	printf("\n");

	//	Analizamos los valores (en nivel de gris) que contiene una linea horizontal
	//	o vertical de la imagen promediada del patrón. 
    if (TomaAutomatica.bAutomatica == false)
    {
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
				    perfiles(M_imagen1, &paramIni);
                    break;
			    case ' ':
				    construye_histo_base();
				    sendcont("Pulse una tecla");
				    getch();
				    borra_buffer(M_overlay_normal, TRANSPARENTE, 0, 0,
					    paramIni.Cam.anchoImagen, paramIni.Cam.altoImagen);
				    break;
			    case 0:
				    Sleep(200);
				    break;
			    case K_ESC:
				    break;
			    default:
				    putch('\a');
		    }
	    
	    } while (c != K_ESC);
    }

final:
    // Dejamos el filtro en la posicion predeterminada (importante para mantener consistencia de enfoque entre ejecuciones)
    Rueda.ChangeFilter(paramIni.Rueda.posFiltro);
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
			error_leve(fn,"Se ha encontrado un filtro con valor fuera del rango 1..MAX_FILTROS_DTA en el fichero de configruacion");
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
    paramAdq->BarridoPatrones.nImagenAcum = nImagenAcum;

    int camposEjeX = -1;
	LOADINT(camposEjeX);
    paramAdq->BarridoPatrones.camposEjeX = camposEjeX;

    int camposEjeY = -1;
	LOADINT(camposEjeY);
    paramAdq->BarridoPatrones.camposEjeY = camposEjeY;

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


// Muestra el menu principal
int menu()
{
	int tecla;
	char mensaje[LONGITUD_TEXTO*2];

    sprintf(mensaje,"\n+-  MENU PRINCIPAL  ---------------------------------------------------------+");
	strcat(mensaje, "\n| Seleccione entre las opciones:                                             |");
	strcat(mensaje, "\n|  - [1]: Comprobar la calibracion del sistema                               |");
	strcat(mensaje, "\n|  - [2]: Calibrar la adquisicion con el PATRON DE ALTA REFLECTACIA (CLARO)  |");
	strcat(mensaje, "\n|  - [3]: Calibrar la adquisicion con el PATRON DE BAJA REFLECTACIA (OSCURO) |");
	strcat(mensaje, "\n|  - [4]: Calibrar la correccion de las imagenes con el POLKA-DOT            |");
	strcat(mensaje, "\n|  - [ESC]  : Salir de la aplicacion                                         |");
	strcat(mensaje, "\n|  - [q]    : Salir de la aplicacion                                         |");
	strcat(mensaje, "\n+----------------------------------------------------------------------------+");
	strcat(mensaje, "\n");
	CharToOem(mensaje, mensaje);
	printf(mensaje);

	do 
    {
		tecla = getKey();
//		if (tecla != 0) printf("  %d", tecla);
		switch(tecla)  
        {
			case '1':
				break;
			case '2':
				break;
			case '3':
				break;
			case '4':
				break;
			case K_ESC:
			case 'q':
			case 'Q':
                tecla = 'q';
				break;
			case 0:
				Sleep(200);
				break;
			default:
				putch('\a');
		}
	} while (tecla != K_ESC && tecla != 'q' && tecla != '1' && tecla != '2' && tecla != '3' && tecla != '4');

	return tecla;
}

// Menu Correccion de la imagen mediante el POLKA-DOT
bool menu_correccion_imagen(parametros* pParamIni)
{
	int tecla;
	char mensaje[LONGITUD_TEXTO*2];
    CCorreccionGeometrica correccionGeometrica;

    sprintf(mensaje,"\n+-  CORRECCIÓN IMÁGEN  -------------------------------------------------------+");
	strcat(mensaje, "\n| Coloque el patrón POLKA-DOT de tal manera que se vean 4 cuadrados completos |");
	strcat(mensaje, "\n| A continuación enfoque y presione [INTRO]                                   |");
	strcat(mensaje, "\n|  - [ESC]  : Cancelar                                                        |");
	strcat(mensaje, "\n|  - [q]    : Salir de la aplicación                                          |");
	strcat(mensaje, "\n+-----------------------------------------------------------------------------+\n");
	CharToOem(mensaje, mensaje);
	printf(mensaje);

	do 
    {
		tecla = getKey();
		if (tecla == K_CR)
        {
            //actualizo el valor absoluto de z del filtro predefinido, porque el usuario ha podido enfocar manualmente
            double zPancromatico = mspWhere(Z_) + pParamIni->Rueda.diferencia_z[pParamIni->Rueda.posFiltro] - pParamIni->Rueda.diferencia_z[Rueda.GetFilter()] ;
            Rueda.zPredefinido = zPancromatico; 
            correccionGeometrica.ProcesarPolka(pParamIni,Rueda);
            sendop("\nParametros de correccion guardados en disco\n\n");
            return true;
        }
        else if(tecla == 'q' || tecla == 'Q')
        {
                tecla = 'q';
                return false;
        }
        else if (tecla == 0)
        {
				Sleep(200);
        }
        else if (tecla == K_ESC)
        {
            // Dejar salir del bucle
        }
        else
        {
				putch('\a');
        }
	} while (tecla != K_ESC && tecla != 'q');

    return true;
}


/**************************  procesa_argumentos ******************************
	Función para procesar los argumentos de la línea de comandos.
	 - Primero es el nombre (y el camino) del archivo de parámetros iniciales.
*****************************************************************************/
int procesa_argumentos(int argc, TCHAR* argv[], char* raiz_paramIni, char** tipo_patron)
{
	char	mensaje[1024];
	char	*fn = "procesa_argumentos";

	if (argc <= 1)  {
		sprintf(mensaje,
		"\aToma una muestra de los patrones.\n"
		"\t Uso:  TomaPatron.exe  raiz_parametros\n"
		"                *:\\*\\*\\     -a     -b\n"
		"Los argumentos admitidos son:\n"
		"\t raiz_patrones: nombre genérico del directorio de patrones\n");
		
		CharToOemBuff(mensaje, mensaje, sizeof(mensaje));
		fprintf(stderr, "\n%s", mensaje);
		Sleep(3000);		// Espera para leer el mensaje que aparece en consola
		return 0;
	}

	//	El argumento de la linea de comandos es el directorio de patrones. 
	
	strcpy(raiz_paramIni,argv[1]);

    
    //Toma automatica
    if (argc == 3 + 3 * PUNTOS_PLANO_REGRESION)
    {
        TomaAutomatica.bAutomatica = true;

	    //	El segundo argumento es el tipo de patron que se desea adquirir. 
	    *tipo_patron = argv[2];

	    if ( strcmp(*tipo_patron, "PAT_OS") != NULL && strcmp(*tipo_patron, "PAT_CL") != NULL)  {
		    sprintf(mensaje, "\nArgumentos admitidos para el tipo de patron:"
							    "\n\t Patron oscuro --> PAT_OS"
							    "\n\t Patron claro  --> PAT_CL");
		    sendop(mensaje);
		    Sleep(3000);		// Espera para leer el mensaje que aparece en consola
		    return 0;
	    }

        // A partir del tercer argumento son los puntos del plano de regresion
        for (int j=0;j<PUNTOS_PLANO_REGRESION;j++) //puntos plano regresion
        {
            for (int k=0;k<3;k++) //coordenadas
            {
                TomaAutomatica.Puntos[j].coord[k] = atof(argv[3+j*k]);
            }
        }
    }
    else
    {
        TomaAutomatica.bAutomatica = false;
    }

	return 1;
}

/**************************  CompruebaValidezParam ******************************
	Comprueba algunos valores de configuracion
*****************************************************************************/
bool CompruebaValidezParam(parametros* param)
{
    // Comprobar que no se supera la profundidad del buffer de acumulacion
    if (pow(2,BITS_CAMARA) * 
        param->BarridoPatrones.nImagenAcum *
        param->BarridoPatrones.camposEjeX  *
        param->BarridoPatrones.camposEjeY
        >
        pow(2,PROFUNDIDAD_ACUMULACION) )
    {
        CString mensaje;
        mensaje.Format("Se sobrepasa la profundidad maxima (%d bits). Bits camara:%d Imagenes a acumular:%d Barrido:%d x %d",
            PROFUNDIDAD_ACUMULACION, 
            BITS_CAMARA,
            param->BarridoPatrones.nImagenAcum, 
            param->BarridoPatrones.camposEjeX, 
            param->BarridoPatrones.camposEjeY);
        AfxMessageBox(mensaje);
        return false;
    }

    return true;
}

