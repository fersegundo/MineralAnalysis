/*
  plnrgr_p.h  Fichero con declaraciones de tipos y funciones para el manejo
              de puntos del espacio definidos por sus coordenadas (X,Y,Z)
              y su peso P.

      Autor:  David Alarcón,  AITEMIN, Noviembre 1991
 Adaptación:  Juan Carlos Catalina, AITEMIN, Abril, Julio 1996

*/

#ifndef  _PLNRGR_P_
#define  _PLNRGR_P_   1

#define  X_ 0
#define  Y_ 1
#define  Z_ 2

typedef struct { double coord[3]; double peso; } point, *ppoint;
typedef struct { int card; ppoint pointn; } pointlist;
typedef struct { double A; double B; double C; } plane;
typedef struct { double A; double B; } line;

#if !defined (__BORLANDC__)
pointlist  read_pointlist();
#endif
void  dim_pointlist(int n,pointlist& lista);
void  put_pointlist(pointlist& pl, int n, point& INp);
#if !defined (__BORLANDC__)
point  get_pointlist(pointlist& pl, int n);
void  write_pointlist(pointlist& INpL);
#endif
void  free_pointlist(pointlist& INpL);
int get_center(pointlist& INpL,point& center);
pointlist  trans_coord(pointlist& INpL, point& INp);
double  get_sumprod(pointlist& INpL, int ia, int ib);
bool  get_plane_rg(pointlist& INpL, plane& OUplane);
#if !defined (__BORLANDC__)
line  get_line_regr_XY(pointlist& INpL);
line  get_line_regr_YX(pointlist& INpL);
#endif
line  get_line_regr_XZ(pointlist& INpL);
line  get_line_regr_YZ(pointlist& INpL);

#endif

