/*
	estructuras.h	Declaración de las estructuras necesarias para la realización
					del ensayo.

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Julio 2002
*/

#ifndef ESTRUCTURAS_H
#define ESTRUCTURAS_H

#include "plnrgr_p.h"
#include "histograma.h"

#if defined (__BORLANDC__)
#define max(a, b)  (((a) > (b)) ? (a) : (b))
#define min(a, b)  (((a) < (b)) ? (a) : (b))
#endif

#define PI 3.14159265358979323846

/*  Definición de constantes para dimensionamiento de estructuras. */

#define  MAX_FIL			  60 //maximo numero de filas de campos
#define  MAX_COL			  60
#define  MAX_IMAGENES		2250
#define  MAX_FILTROS_MTB	  8
#define  MAX_GRANOS			  30
#define  MAX_CARBONES		   7	// Número máximo de componentes identificables en una mezcla
#define  MAX_BLOBS			 255	// Número máximo de áreas analizables.
#define  TAM_INFO_FILTROS	 255
#ifndef MAX_FILTROS_DTA
    #define MAX_FILTROS_DTA			16
#endif
#define MAX_OBJETIVOS   6
#define MAX_CUBOS           4

#if !defined (__BORLANDC__)
typedef long histoGrano[MAX_GRANOS][NUMVAL];
#endif

// Para poder hacer tomas de patrones automaticamente mediante consola
#define PUNTOS_PLANO_REGRESION  4
typedef struct 
{
    point          Puntos[PUNTOS_PLANO_REGRESION]; // para ejecucion en modo "batch" 
    bool           bAutomatica;
} TomaAutomatica_T;

/*  Tipos de estructuras definidas para leer y almacenar la información
	generada durante la realización del ensayo. */
typedef struct {
	long		MaxBrillo, MinBrillo;			// Límites de brillo de la cámara
	long		MaxExposicion, MinExposicion;	// Límites de exposición de la cámara
	long		MaxBaseExposicion, MinBaseExposicion;	// Límites de exposición de la cámara
	long		MaxGanancia, MinGanancia;		// Límites de ganancia de la cámara
	long		anchoDig, altoDig;			// Ancho y alto del digitalizador en formato normal
	long		anchoDig_e, altoDig_e;		// Ancho y alto del digitalizador en formato enfoque
} limitesCam;

typedef struct {
	int			brillo;
	int			ganancia;
	int			anchoImagen;		// Tamaño de la imagen que se desea adquirir
	int			altoImagen;
	char		formatoDig[64];		// Formato de configuración del digitalizador
	int			anchoImagen_e;
	int			altoImagen_e;
	char		formatoDig_e[64];
    int         profundidad;        // Profundidad bruta de la camara.
} paramCam;

typedef struct {
	double		MaxLamp, MinLamp;			// Máximo y mínimo de la lámpara.
	int			MaxPosRevolver;				// Máxima posición de la rueda de filtros.
#if !defined (__BORLANDC__)
	CString		nomFiltro[8];				// Vector para almacenar los nombres de los filtros
#else
	AnsiString	nomFiltro[8];				// Vector para almacenar los nombres de los filtros
#endif
} limitesMtb;

typedef struct {
	int			posFiltro;			// Posición prefijada del filtro del microscopio [1 .. MAX_FILTROS_MTB]
	int			numFiltros;			// Numero de filtros a usar
	int			filtros[MAX_FILTROS_MTB]; //Numero de filtro [1 .. MAX_FILTROS_MTB] aplicable a cada filtro 
	int			porcentaje_exposicion[MAX_FILTROS_MTB]; //Porcentaje de reduccion de la luminosidad para cada filtro
} paramMtbFiltros;


typedef struct {
	double		voltLamp;			// Voltaje prefijada de la lámpara
	paramMtbFiltros rueda_filtros;
} paramMtb;

typedef struct {
    int     nImagenAcum ; //imagenes a acumular en la adquisicion de patrones
	int		camposEjeX;
	int     camposEjeY;
	int		rango_util[MAX_FILTROS_DTA]; //adquisicion de patrones: niveles de gris (12bit) a tener en cuenta desde la moda
} paramBarridoPatrones;

typedef struct{
	int			posFiltro;			// Posición prefijada del filtro (para observación). Rango [0 .. MAX_FILTROS_DTA-1]
	int			posEnfoque;			// Posición prefijada del filtro (para enfoque). Rango [0 .. MAX_FILTROS_DTA-1]
	int			exposicion[MAX_FILTROS_DTA]; //tiempo de exposicion necesario para cada FILTRO.
	int			base_exp[MAX_FILTROS_DTA];   // base de exposicion (la exposicion final es el resultado de multiplicar la exposicion y la base de exposicion)
	double		diferencia_z[MAX_FILTROS_DTA]; //diferencia (con respecto al filtro prefijado) de la z para cada FILTRO. Rango [0 .. MAX_FILTROS_DTA - 1]
	double		reflectancia_nominal_pat_os[MAX_FILTROS_DTA]; //reflectancia del patron oscuro con cada FILTRO. Rango [0 .. MAX_FILTROS_DTA - 1]
	double		reflectancia_nominal_pat_cl[MAX_FILTROS_DTA]; //reflectancia del patron claro con cada FILTRO. Rango [0 .. MAX_FILTROS_DTA - 1]
	int			aplicaTing[MAX_FILTROS_DTA];
	int			espectro[MAX_FILTROS_DTA]; //longitud de onda media para este filtro
	char		comentario[MAX_FILTROS_DTA][TAM_INFO_FILTROS]; // string asociado a cada filtro
} paramRueda;

typedef struct{
    CString     csDirectorio;
    double      dAumento;
    double      dMaxDistPlano;
}paramObjetivo;

typedef struct {
	char		raiz_patrones[512];	// "Path" al origen de los patrones
	int			nBandas;			// Número de bandas que hay que adquirir de cada campo
	int			filtro[MAX_FILTROS_DTA]; // Número de filtro [0.. MAX_FILTROS_DTA-1] aplicable a cada BANDA [0 .. MAX_FILTROS_DTA - 1]
	paramMtb	Mtb;
	paramRueda	Rueda;
	paramCam	Cam;
	double		escala;
    paramBarridoPatrones BarridoPatrones;
    paramObjetivo    objetivos[MAX_OBJETIVOS];
    int         nImagenAcum; //imagenes a acumular en la adquisicion normal
} parametros;

#endif	// ESTRUCTURAS_H

