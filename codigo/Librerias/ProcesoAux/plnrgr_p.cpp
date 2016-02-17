/*
  plnrgr_p.c  Fichero con funciones para el cálculo por mínimos cuadrados
              del plano de regresión más próximo a una lista de puntos
              del espacio definidos por sus coordenadas (X,Y,Z) y peso P.

              Asimismo se incluyen funciones para el cálculo por mínimos
              cuadrados de la línea de regresión más próxima a una lista
              de puntos del plano definidos por sus coordenadas (X,Y) y
              peso P.

      Autor:  David Alarcón,  AITEMIN, Noviembre 1991
 Adaptación:  Juan Carlos Catalina, AITEMIN, Abril, Julio 1996

*/
#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#include <stdio.h>
#if !defined (__BORLANDC__)
#include <stdlib.h>
#endif

#include "plnrgr_p.h"


void  dim_pointlist(int n,pointlist& lista) {
   lista.card = n;
   lista.pointn = (point*) calloc(lista.card, sizeof(point));
}

void  put_pointlist(pointlist& pl, int n, point& INp) {
   if (pl.card <= n) { printf("dimension incorrecta de pointlist"); exit(1); }
   if (n < 0)        { printf("dimension incorrecta de pointlist"); exit(1); }
   pl.pointn[n].coord[X_] = INp.coord[X_];
   pl.pointn[n].coord[Y_] = INp.coord[Y_];
   pl.pointn[n].coord[Z_] = INp.coord[Z_];
   pl.pointn[n].peso = INp.peso;
}

#if !defined (__BORLANDC__)
point  get_pointlist(pointlist& pl, int n) {
   point  OUp;

   if (pl.card <= n) { printf("dimension incorrecta de pointlist"); exit(1); }
   if (n < 0)        { printf("dimension incorrecta de pointlist"); exit(1); }
   OUp.coord[X_] = pl.pointn[n].coord[X_];
   OUp.coord[Y_] = pl.pointn[n].coord[Y_];
   OUp.coord[Z_] = pl.pointn[n].coord[Z_];
   OUp.peso = pl.pointn[n].peso;
   return  OUp;
}

void  write_pointlist(pointlist& INpL) {
   int  i;

   printf("card = %d\n", INpL.card);
   for (i=0; i < INpL.card; i++)
      printf(" ( %lf, %lf, %lf ) : %lf\n", INpL.pointn[i].coord[X_],
         INpL.pointn[i].coord[Y_], INpL.pointn[i].coord[Z_], INpL.pointn[i].peso);
}

pointlist  read_pointlist() {
   int  i;
   pointlist  OUpL;

   scanf("%d", &OUpL.card);
   OUpL.pointn = (point*) calloc(OUpL.card, sizeof(point));
   for (i=0; i < OUpL.card; i++) {
      scanf("%lf", &OUpL.pointn[i].coord[X_]);
      scanf("%lf", &OUpL.pointn[i].coord[Y_]);
      scanf("%lf", &OUpL.pointn[i].coord[Z_]);
      scanf("%lf", &OUpL.pointn[i].peso);
   }
   return OUpL;
}
#endif

void  free_pointlist(pointlist& INpL) {
   free(INpL.pointn);
}

// calcula el centro de la lista de puntos INpL teniendo en cuenta sus pesos
// no se tendrán en cuanta puntos incorrectos (pesos incorrectos)
// devuelve el numero de puntos validos usados
int get_center(pointlist& INpL, point& center) {
   int  i;
   double  peso = 1;
   int nCount = 0;

   center.coord[X_] = center.coord[Y_] = center.coord[Z_] = center.peso = 0.0;
   for (i=0; i < INpL.card; i++) 
   {
        if (INpL.pointn[i].peso > 0)
        {
            nCount ++;
            center.peso += peso = INpL.pointn[i].peso;
            center.coord[X_] += peso * INpL.pointn[i].coord[X_];
            center.coord[Y_] += peso * INpL.pointn[i].coord[Y_];
            center.coord[Z_] += peso * INpL.pointn[i].coord[Z_];
        }
   }

   if (nCount == 0 || center.peso == 0)
       return 0;

   center.coord[X_] /= center.peso;
   center.coord[Y_] /= center.peso;
   center.coord[Z_] /= center.peso;

   return nCount;
}

pointlist  trans_coord(pointlist& INpL, point& INp) {
   int  i;
   pointlist  OUpL;

   OUpL.card = INpL.card;
   OUpL.pointn = (point*) calloc(OUpL.card, sizeof(point));
   for (i=0; i < OUpL.card; i++) {
      OUpL.pointn[i].coord[X_] = INpL.pointn[i].coord[X_] - INp.coord[X_];
      OUpL.pointn[i].coord[Y_] = INpL.pointn[i].coord[Y_] - INp.coord[Y_];
      OUpL.pointn[i].coord[Z_] = INpL.pointn[i].coord[Z_] - INp.coord[Z_];
      OUpL.pointn[i].peso = INpL.pointn[i].peso;
   }
   return OUpL;
}

double  get_sumprod(pointlist& INpL, int ia, int ib) {
   int  i;
   double  od = 0;

   for (i=0; i < INpL.card; i++)
      od += INpL.pointn[i].coord[ia] * INpL.pointn[i].coord[ib]
         * INpL.pointn[i].peso;
   return od;
}


/********************************************************************

PROCESO PARA OBTENER EL PLANO DE REGRESION DADA UNA LISTA DE PUNTOS 3D
——————————————————————————————————————————————————————————————————————

1. se halla el centro de gravedad de los puntos p0=(x0,y0,z0)
2. se hace una traslación de coordenadas de modo que el centro de
   gravedad coincida con el origen de coordenadas
   p' = p - p0;
3. con el nuevo sistema de coordenadas se va a hallar el plano:
   z' = A x' + B y'
   que pasa por p'=(0,0,0) (origen y centro de gravedad)
   Teoría: . según el cambio de coordenadas el plano pasa por el
             el centro de gravedad.
           . Hay dos parámetros a determinar A y B.
           . La suma de los cuadrados de las distancias entre cada
             punto y su proyección Z al plano debe ser mínima

                 n
                sum ( A x'[n] + B y'[n] - z'[n] )^2  ->  mínimo

           . Para hallar A y B que hagan mínima esta expresión
             se necesita derivar respecto a dichas variables e
             igualar a 0. Obteniendo dos ecuaciones.

             d exp    n
             ----- = sum 2 ( A x'[n] + B y'[n] - z'[n] ) x'[n] = 0
              d A

             ====>
                    n               n                 n
             (1) A sum x'[n]^2 + B sum x'[n] y'[n] = sum x'[n] z'[n]


             d exp     n
             ----- = sum 2 ( A x'[n] + B y'[n] - z'[n] ) y'[n] = 0
              d B

             ====>
                    n                   n             n
             (2) A sum y'[n] x'[n] + B sum y'[n]^2 = sum y'[n] z'[n]

             Notación:
                 n                   n                      n
                sum x'[n]^2 = xx;   sum x'[n] y'[n] = xy;  sum x'[n] z'[n] = xz;

                 n                   n
                sum y'[n]^2 = yy;   sum y'[n] z'[n] = yz;

           . Con esto se tienen 2 ecuaciones lineales con 2 incognitas:

             (1)  A xx + B xy = xz
             (2)  A xy + B yy = yz

           . Resolviendo queda:

                  xz yy - xy yz
             A = ---------------
                   xx yy - xyý

                  xy xz - xx yz
             B = ---------------
                   xyý - xx yy

4. Volviendo al sistema de coordenadas inicial:
   z' = A x' + B y'
   siendo el origen y centro de gravedad (x0,y0,z0)
   (z - z0) = A (x - x0) + B (y - y0)
   z = A x + B y + ( z0 - A x0 - B y0)
   ====>  z = A x + B y + C
          C = z0 - A x0 - B y0

  Devuelve false si: hay menos de 3 puntos utiles o el plano es vertical
********************************************************************/

bool get_plane_rg(pointlist& INpL, plane& OUplane) {
   double  xx, xy, xz, yy, yz;
   point  center;
   pointlist  MpL;

   if (get_center(INpL, center) < 3)
        return false;

   MpL = trans_coord(INpL,center);
   xx = get_sumprod(MpL, X_, X_);
   xy = get_sumprod(MpL, X_, Y_);
   xz = get_sumprod(MpL, X_, Z_);
   yy = get_sumprod(MpL, Y_, Y_);
   yz = get_sumprod(MpL, Y_, Z_);
   free_pointlist(MpL);
   double div = ( xx * yy - xy * xy );

   if (div == 0)
   {
       OUplane.A = 0;
       OUplane.B = 0;
       return false;
   }
   else
   {
       OUplane.A = ( xz * yy - xy * yz ) / div;
       OUplane.B = ( xy * xz - xx * yz ) / -div;
   }

   OUplane.C = center.coord[Z_]
             - OUplane.A * center.coord[X_] - OUplane.B * center.coord[Y_];

   return true;
}



#if !defined (__BORLANDC__)
/****************************************************************************

—————————————————————————————————————————————————————————————————————————————
   PROCESO PARA OBTENER EL PLANO DE REGRESION DADA UNA LISTA DE PUNTOS 2D
—————————————————————————————————————————————————————————————————————————————

1. se halla el centro de gravedad de los puntos p0=(x0,y0)
2. se hace una traslación de coordenadas de modo que el centro de
   gravedad coincida con el origen de coordenadas
   p' = p - p0;
3. con el nuevo sistema de coordenadas se va a hallar la línea:
   y' = A x'
   que pasa por p'=(0,0) (origen y centro de gravedad)
   Teoría: . según el cambio de coordenadas la línea pasa por el
             el centro de gravedad.
           . Hay un parámetro a determinar: A.
           . La suma de los cuadrados de las distancias entre cada
             punto y su proyección y' a la línea debe ser mínima

                n
                ä ( A x'[n] - y'[n] )ý ——> mínimo

           . Para hallar un A que haga mínima esta expresión
             se necesita derivar respecto a dicha variable e
             igualar a 0.

             d exp   n
             ----- = ä 2 ( A x'[n] - y'[n] ) x'[n] = 0
              d A

             ÍÍÍÍ>
                   n          n
             (1) A ä x'[n]ý = ä x'[n] y'[n]

             Notación:
                n                n
                ä x'[n]ý = xx;   ä x'[n] y'[n] = xy;

           . Con esto se tiene una ecuación lineal con una incógnita:

             (1)  A xx = xy

           . Resolviendo queda:

                   xy
             A = ------
                   xx

4. Volviendo al sistema de coordenadas inicial:
   y' = A x'
   siendo el origen y centro de gravedad (x0,y0)
   (y - y0) = A (x - x0)
   ÍÍÍÍ>  y = A x + B
          B = y0 - A x0


NOTA: Si los puntos están situados a lo largo de una línea casi vertical,
      este m‚todo puede fallar debido a que A tomaría valores enormemente
      grandes, pudiendo aparecer errores de desbordamiento, etc.
      Para estos casos hemos definido la función get_line_regr_YX(),
      que invierte la definición de la línea, que pasará a corresponder
      a la ecuación:
                           x = A y + B

****************************************************************************/

line  get_line_regr_XY(pointlist& INpL) {
   double  xx, xy;
   point  center;
   pointlist  MpL;
   line  OUline;

   get_center(INpL,center);
   MpL = trans_coord(INpL, center);
   xx = get_sumprod(MpL, X_, X_);
   xy = get_sumprod(MpL, X_, Y_);
   free_pointlist(MpL);
   OUline.A = xy / xx;
   OUline.B = center.coord[Y_] - OUline.A * center.coord[X_];
   return OUline;
}


line  get_line_regr_YX(pointlist& INpL) {
   double  xy, yy;
   point  center;
   pointlist  MpL;
   line  OUline;

   get_center(INpL,center);
   MpL = trans_coord(INpL, center);
   yy = get_sumprod(MpL, Y_, Y_);
   xy = get_sumprod(MpL, X_, Y_);
   free_pointlist(MpL);
   OUline.A = xy / yy;
   OUline.B = center.coord[X_] - OUline.A * center.coord[Y_];
   return OUline;
}
#endif


line  get_line_regr_XZ(pointlist& INpL) {
   double  xx, xz;
   point  center;
   pointlist  MpL;
   line  OUline;

   get_center(INpL,center);
   MpL = trans_coord(INpL, center);
   xx = get_sumprod(MpL, X_, X_);
   xz = get_sumprod(MpL, X_, Z_);
   free_pointlist(MpL);
   OUline.A = xz / xx;
   OUline.B = center.coord[Z_] - OUline.A * center.coord[X_];
   return OUline;
}


line  get_line_regr_YZ(pointlist& INpL) {
   double  yy, yz;
   point  center;
   pointlist  MpL;
   line  OUline;

   get_center(INpL,center);
   MpL = trans_coord(INpL, center);
   yy = get_sumprod(MpL, Y_, Y_);
   yz = get_sumprod(MpL, Y_, Z_);
   free_pointlist(MpL);
   OUline.A = yz / yy;
   OUline.B = center.coord[Z_] - OUline.A * center.coord[Y_];
   return OUline;
}


/***************************************************************************/

