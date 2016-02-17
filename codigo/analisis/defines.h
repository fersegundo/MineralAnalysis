#if !defined(DEFINES_H)
#define DEFINES_H

#define EXTENSION_FICHERO_CAMPO_CLASIFICACION   ".cla"
#define EXTENSION_FICHERO_CAMPO_PIXELS          ".pix"
#define EXTENSION_FICHERO_CAMPO                 ".dat"

   
#define MAX_CAMPOS_INEXISTENTES 4  //si falta algun fichero de datos de campo se salta al siguiente hasta un maximo

#define INTERVALO_TIMER_SELECCION   500 //milisegundos

#define NUM_VALORES_HISTO 256 //Valores del histograma. No es lo mismo que NUMVAL porque no depende de la camara

#define NUM_BANDAS 13
#define MAX_NUM_IMAGENES 16
#ifndef NUM_COLORES 
    #define NUM_COLORES 11 //maximo numero de colores para representar areas
#endif

#ifndef PI 
    #define PI 3.14159265358979323846
#endif

// Valores de los combo box
#define SELECCIONE_MINERAL          "No identificado"
#define SELECCIONE_MINERAL_ABR      "Mineral"

#define CALIDAD_SIN_VALORAR         "Sin valorar"
#define CALIDAD_SIN_VALORAR_ABR     "-"
#define CALIDAD_A                   "A - Excelente"
#define CALIDAD_B                   "B - Buena"
#define CALIDAD_C                   "C - Normal"
#define CALIDAD_D                   "D - Mala"
#define CALIDAD_E                   "E - Muy mala"

//CLASIFICACION
#define PIXEL_NO_CUMPLE_DISTANCIA_CONFIABILIDAD 1
#define PIXEL_NO_CUMPLE_DISTANCIA               2
#define PIXEL_NO_CUMPLE_CONFIABILIDAD           3
#define PIXEL_CUMPLE                            4

// COLORES

#define NEGRO_R       0x00
#define NEGRO_G       0x00
#define NEGRO_B       0x00
#define NEGRO_POS     0


#define ROJO_R        0xff
#define ROJO_G        0x00
#define ROJO_B        0x00
#define ROJO_POS      1


#define AZUL_R        0x0
#define AZUL_G        0x0
#define AZUL_B        0xFF
#define AZUL_POS      2

#define VERDE_R       0x00
#define VERDE_G       0xFF
#define VERDE_B       0x00
#define VERDE_POS     3

#define CYAN_R        0x00
#define CYAN_G        0xFF
#define CYAN_B        0xFF
#define CYAN_POS      4

#define NARANJA_R     0xFF
#define NARANJA_G     0x99
#define NARANJA_B     0x33
#define NARANJA_POS   5

#define MORADO_R      0x80
#define MORADO_G      0x00
#define MORADO_B      0x80
#define MORADO_POS    6

#define GRIS_R        0x80
#define GRIS_G        0x80
#define GRIS_B        0x80
#define GRIS_POS      7

#define ROSA_R        0xff
#define ROSA_G        0x99
#define ROSA_B        0xff
#define ROSA_POS      8

#define ORO_R         0xcc
#define ORO_G         0xcc
#define ORO_B         0x00
#define ORO_POS       9

#define MARRON_R      0x66
#define MARRON_G      0x33
#define MARRON_B      0x00
#define MARRON_POS    10


#define AMARILLO_R    0xFF
#define AMARILLO_G    0xFF
#define AMARILLO_B    0x00
#define AMARILLO_POS  0

#define VIOLETA_R     0x99
#define VIOLETA_G     0x66
#define VIOLETA_B     0xcc
#define VIOLETA_POS   0
//        #define VIOLETA_R      0x66
//        #define VIOLETA_G      0x33
//        #define VIOLETA_B      0x99

#define PLATA_R       0xc0
#define PLATA_G       0xc0
#define PLATA_B       0xc0
#define PLATA_POS     0




#endif // !defined(DEFINES_H)
