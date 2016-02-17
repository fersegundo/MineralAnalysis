/*
	comprobacionPat
*/

#ifndef COMPROBACIONPAT_H
#define COMPROBACIONPAT_H

#define N_PERFILES			6

#if !defined (__BORLANDC__)
int compruebaCalibracion(parametros *param);
#endif
void valor_bordesH(MIL_ID M_fb, int nLineas);
void valor_bordesV(MIL_ID M_fb, int nLineas);
void valor_lineaH(MIL_ID M_fb, int nLineas, bool solido);
void valor_lineaV(MIL_ID M_fb, int nLineas, bool solido);
void muestra_LineasAnalisis(MIL_ID M_fb, int nLineas, bool borrar);

#endif // COMPROBACIONPAT_H

