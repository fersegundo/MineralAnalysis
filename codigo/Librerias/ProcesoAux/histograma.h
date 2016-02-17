/*
	histograma.h	Declaraci�n de funciones de histograma.cpp

	Autores:	JCC, BLM
	Copyright:	� AITEMIN Enero 2002
*/

#ifndef HISTOGRAMA_H
#define HISTOGRAMA_H

//#include <mil.h>

/*  Par�metros de la extracci�n de histogramas. */
#define BITS_CAMARA 12
#if BITS_CAMARA == 12
#define  NUMVAL				 4096 		// N�mero de valores del histograma
#else
#define  NUMVAL				 256 		// N�mero de valores del histograma
#endif

#define  NUMELEMFIL			  256		// Radio m�ximo del filtro gaussiano.


/*  Posiciones, tama�os y colores de los gr�ficos. */

#define  POSIH					0		// M�ltiplo de 4, m�ximo seg�n destino.
#define  POSIV					0		// M�ximo seg�n destino.

#define  ANCHOH   (ANCHO_IMAGEN_R / 2)	// Anchura de la "celda" de dibujo de un histograma.
#define  ALTOH		 (ALTO_IMAGEN_R/5)	// Altura de la "celda" de dibujo de un histograma
#define  MARGENH			   20
#define  ANCH       (ANCHOH - MARGENH)	// Anchura de la zona �til de un histograma.
#define  ALTH              (ALTOH)		// Altura de la zona �til de un histograma.

#define  FILA1					0		// Coordenadas de posici�n de los histogramas.
#define  FILA2					1
#define  FILA3					2
#define  FILA4					3
#define  FILA5					4
#define  FILA6					5
#define  COLUMNA1				0
#define  COLUMNA2				1
#define  COLUMNA3				2

#define  ALTOBANDA			   40			// Altura de la banda del t�tulo.
#if !defined (__BORLANDC__)
#define  ANCE      ANCHO_IMAGEN_R			// Anchura de la banda de enfoque.
#define  ALTE				 ALTH			// Altura de la banda de enfoque.
#define  PEH					0			// Posici�n de la banda de enfoque.
#define  PEV (ALTO_IMAGEN_R - ALTH)
#endif

extern long  histograma[NUMVAL];	// Vector auxiliar para c�lculo de histogramas.

/* Declaraci�n de funciones en histograma.cpp. */

typedef long MIL_ID;

#if !defined (__BORLANDC__)
void  construye_histograma(MIL_ID M_fb, int desplazamiento, int signo,
			int coma_flotante, double kte);
#endif
void  construye_histo_base(void);
void  construye_histo_base(int nNumVal);
void  construye_histograma_estimado(void);
void  evalua_histograma(MIL_ID buffer, long *histo);
#if !defined (__BORLANDC__)
void  evalua_histograma_central(MIL_ID buffer, long *histo);
#endif
void  acumula_histograma(long *histo, int num_val, long *histoacum);
void  muestra_histograma(long *histo, MIL_ID M_fb, long fondo_escala, int paso, int fila, int columna,
			int color, int solido, int borrar, char *titulo);
void  muestra_histograma(long *histo, MIL_ID M_fb, long fondo_escala, int paso,
	int fila, int columna, int color, int solido, int borrar, char *titulo, int nNumVal);

int  graba_histograma(char *nombre, long *histo);
int	 recupera_histograma(char *nombre, long *histo);

#if !defined (__BORLANDC__)
int  procesa_histograma(long *histo, double *media, double *sigma);
int  percentil_histograma(long *histo, int num_val, int nOrden);
#endif

int	 moda_histograma(long *histo, int desde, int hasta);
int  antimoda_histograma(long *histo, int desde, int hasta);
void  asimetria_histograma(long *histo, int moda, double *sigma_inf,
					double *sigma_sup);

double  normaliza_histograma(long *histo, double *histo_norm);
void  filtra_histograma(long *histo, long *histo_fil, double rIni, double rFin);
void  calcula_coef_filtro_histograma(void);

#if !defined (__BORLANDC__)
void  evalua_histograma_zona(MIL_ID M_origen, MIL_ID M_mascara, long *histo);
#endif

void  muestra_marca(int valor, MIL_ID M_fb, int paso, int fila, int columna, int color, int borrar);

int	  copia_histograma(long *histo_o, long *histo_d, int iInf, int iSup);

#endif // HISTOGRAMA_H

