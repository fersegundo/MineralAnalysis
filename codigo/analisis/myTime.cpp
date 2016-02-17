///////////////////////////////////////////////////////////////////////
#include "myTime.h"
///////////////////////////////////////////////////////////////////////
#include <stdio.h>
#include <windows.h>
///////////////////////////////////////////////////////////////////////
class Crono {
 public:
  Crono();
  void Ini();
  double getSec();
 private:
  LARGE_INTEGER frec;
  double dfrec;
  LARGE_INTEGER t0;
  double t0_h, t0_l;
};
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
double myTime() {
 static Crono crono;
 return crono.getSec();
}
///////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////
Crono::Crono() {
 Ini();
}
///////////////////////////////////////////////////////////////////////
void Crono::Ini() {
 bool error = false;
 if ( QueryPerformanceFrequency(&frec) == 0 ) error = true;
 dfrec = (double)frec.QuadPart;
 if ( dfrec == 0 ) error = true;
 if ( QueryPerformanceCounter(&t0) == 0 ) error = true;
 if ( error ) {
   printf("Hight resulution time is not supported\n");
   exit(1);
 }
 t0_h = (double)t0.HighPart;
 t0_l = (double)t0.LowPart;
}
///////////////////////////////////////////////////////////////////////
double Crono::getSec() {
 static double m = 4294967296.0; // = 2^32
 LARGE_INTEGER t1;
 QueryPerformanceCounter(&t1);
 double t1_h = (double)t1.HighPart;
 double t1_l = (double)t1.LowPart;
 double d_h = t1_h - t0_h;
 double d_l = t1_l - t0_l;
 double ds_l = d_l / dfrec;
 if ( d_h == 0 ) return ds_l;
 double ds_h = d_h / dfrec;
 double r = ds_h * m + ds_l;
 return r;
}
///////////////////////////////////////////////////////////////////////
