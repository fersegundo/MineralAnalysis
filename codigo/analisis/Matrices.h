// Matrices.h: interface for the CMatriz
//
//////////////////////////////////////////////////////////////////////

#if !defined(MATRICES_H_)
#define MATRICES_H_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


#include "defines.h"

// Umbral para el cálculo de la distancia de Mahalanobis
#define UMBRAL_MAHAL    1E-7
#define UMBRAL_INV      1E-9      // Umbral para invertir una matriz
#define UMBRAL_JACOBI   1E-8      // Umbral para el método de Jacobi

//using namespace std;

typedef  double  Matriz[NUM_BANDAS][NUM_BANDAS];

void CopiaVector(double *v1,const double *v2, int n);
void CopiaVector(int *v1,const int *v2, int n);

void MatrizCovarianza(double** matDatos, Matriz matrizCovarianza, int nVariables, int nDatos);
void Invierte(Matriz mat, Matriz inv, int nVal);
bool InvierteMatriz(Matriz mp, int p);

void MultMatriz(Matriz m1, Matriz m2, Matriz m3, int p, int q, int r);
void MultVectorMatriz(double* v1, Matriz m2, double* v3, int q, int r);
void MultMatrizVector(Matriz m1, double* v2, double* v3, int p, int q);
void MultVectorVector(double* v1, double* v2, double& nRes, int p);
void MultVectorVector(double* v1, double* v2, Matriz m3, int p, int r);


#endif
