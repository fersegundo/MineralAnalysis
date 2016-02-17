#if !defined(DEFINES_H)
#define DEFINES_H


#ifndef MAX_FILTROS_DTA
    #define MAX_FILTROS_DTA			16
#endif

#define WM_UPDATEFIELDS WM_APP + 1 //Necesario para actualizar un CWnd desde un Thread (UpdateData no es posible)

//#define CS_DROPSHADOW 0x00020000
#define SPI_SETDROPSHADOW 0x1025

#define INTERVALO_ACTUALIZACION_POSICION 200
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
