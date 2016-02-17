/*
	Declaración de funciones contenidas en el módulo
	funciones_contraste.cpp

	Autores:	JCC, BLM
	Copyright:	© AITEMIN Enero 2002.
*/

#ifndef FUNCIONES_CONTRASTE_H
#define FUNCIONES_CONTRASTE_H

//  Parámetros de determinación del contraste. 
#define  KTE_TRUNC           0.03		// Constante para "generar" la lut de cuadrados.
#define  KTE_CUADR          10.00		// Constante para "generar" la lut de cuadrados.
#define  KTE_ENFOQUE      1000.00		// Constante para "generar" la lut de cuadrados.

//  Definiciones de constantes empleadas en autoenfoque 
#define ALTAVELZ_LEICA_20x	  12			// Velocidad alta del eje Z
#define ALTAVELZ_ZEISS_20x	  6 // 8			// Velocidad alta del eje Z
#define BAJAVELZ_LEICA_20x	  5			// Velocidad lenta del eje Z
#define BAJAVELZ_ZEISS_20x	  2			// Velocidad lenta del eje Z
#define TIEMPO_FICTICIO_AUTOENFOQUE_MANUAL		0.5			// En segundos. En autoenfoque manual (sin movimiento), hay que desplazarse en funcion de la "velocidad", este es el tiempo que suponemos entre paso y paso
#define MEJOR_AJUSTE_LEICA_20x  0.7  // 10x = 1.0
#define MEJOR_AJUSTE_ZEISS_20x  0.42 // 10x = 0.8, 5x = 1.75

#define MAXPASOS		150			// Número máximo de pasos de enfoque

// rangos de z para el autonfoque para un objetivo de 20x
#define BANDAFOCOINI_20x	  7.0//15.0	// Semialtura banda de enfoque inicial
#define BANDAFOCOBAJO_20x	 25.0//30	// Semialtura banda de enfoque final
#define BANDAFOCOALTO_20x	 35.0		// Semialtura banda de enfoque final
#define BANDAFOCOMAX_20x	 40.0		// Semialtura banda de enfoque máxima
#define BANDAFOCOFINAL_20x	  4.0		// Semialtura banda de enfoque fino

#define FACTOR_CONTRASTEMAX 0.98 //0.8

#define CONTRASTEMIN	 20.0//30.0//50.0//40.0//50.0		// Mínimo valor de contraste útil.
#define UMBRALCONTRASTE	 20.0//10//20.0
#define FRACCIONFOCOSUBE 15.0		// Fracción significativa del valor
#define FRACCIONFOCOBAJA 15.0 //10.0		// Fracción significativa del valor

//	Definiciones de constantes utilizadas en el análisis de la evolución del
//	contraste para interpolar la posición del punto de máximo enfoque. 
#define RADIOFOCO           2
#define	DESPLAZAFOCOLENTO	0.80	// Corrección para las cotas de enfoque a velocidad lenta
#define DESPLAZAFOCORAPIDO	2.20	// Corrección para las cotas de enfoque a velocidad rapida

//	Definición de los estados para determinar el máximo contraste. 
#define NADA			0x00
#define SUBIDA			0x01
#define BAJADA			0x02
#define MAXIMO			0x03
#define CONTRASTE_BAJO	0x04
#define MUCHA_RESINA	0x08
#define RECORRIDOFIN	0x10		// Si se ha realizado el recorrido largo

/////////////////////////////////////////////////////////////////////////////

//	Declaración de las funciones. 
int  ini_contraste(HWND	pImagen = NULL);
int  fin_contraste(HWND	pImagen = NULL);
double determina_contraste(MIL_ID M_fb);
int  autoenfoque(double zBase, double zPlano, double *zFoco, double *contraste, double dAumentos = -1, bool bMoviendose = true,
                 HWND	pImagen = NULL);

#endif // FUNCIONES_CONTRASTE_H

