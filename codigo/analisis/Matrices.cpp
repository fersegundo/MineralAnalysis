// Matrices.cpp: utilidades generales de matrices
// Lenguaje C - copiado de proyecto antiguo granitos
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include  <math.h> //para fabs y sqrt
#include "Matrices.h"
#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif


void MatrizCovarianza(double** matDatos, Matriz matrizCovarianza, int nVariables, int nDatos)
// Calcula la matriz[p][p] de covarianzas covs a partir de la matriz[n1][p] de
// datos dat
{
    int i, j, k;
    
    // Inicializacion
    memset(matrizCovarianza,0,sizeof(double)*nVariables*nVariables);

    if (nDatos < 2)
        return;

    for (i = 0; i < nDatos; i++)
        for (j = 0; j < nVariables; j++)
            for (k = 0; k < nVariables; k++)
                matrizCovarianza[j][k] += matDatos[i][j] * matDatos[i][k];

    // Promedio
    for (i = 0; i < nVariables; i++)
        for (j = 0; j < nVariables; j++)
            matrizCovarianza[i][j] /= nDatos;
}

//---------------------------------------------------------------------------
void CopiaVector(double *v1,const double *v2, int n)
// Copia el vector[n] v2 en el v1
{
  int i;

  for (i = 0; i < n; i++)
    v1[i] = v2[i];
}
void CopiaVector(int *v1,const int *v2, int n)
// Copia el vector[n] v2 en el v1
{
  int i;

  for (i = 0; i < n; i++)
    v1[i] = v2[i];
}

//---------------------------------------------------------------------------
void CopiaMatriz(Matriz m1, Matriz m2, int filas, int cols)
// Copia la matriz[filas][cols] m1 en la m2
{
  int i, j;

  for (i = 0; i < filas; i++)
    for (j = 0; j < cols; j++)
      m1[i][j] = m2[i][j];
}
//---------------------------------------------------------------------------
// Multiplica la matriz[p][q] m1 por la matriz[q][r] m2 para obtener la
// matriz[p][r] m3
void MultMatriz(Matriz m1, Matriz m2, Matriz m3, int p, int q, int r)
{
  int i, j, k;

  for (i = 0; i < p; i++)
    for (k = 0; k < r; k++) 
    {
      m3[i][k] = 0;
      for (j = 0; j < q; j++)
        m3[i][k] += m1[i][j] * m2[j][k];
    }
}
//---------------------------------------------------------------------------
// Multiplica el vector[1][q] v1 por la matriz[q][r] m2 para obtener el
// vector[1][r] v3
void MultVectorMatriz(double* v1, Matriz m2, double* v3, int q, int r)
{
  int j, k;

    for (k = 0; k < r; k++) 
    {
      v3[k] = 0;
      for (j = 0; j < q; j++)
        v3[k] += v1[j] * m2[j][k];
    }
}
//---------------------------------------------------------------------------
// Multiplica la matriz[p][q] m1 por el vector[q][1] v2 para obtener el
// vector VERTICAL [p][1] v3
void MultMatrizVector(Matriz m1, double* v2, double* v3, int p, int q)
{
  int i, j;

  for (i = 0; i < p; i++)
      v3[i] = 0;
      for (j = 0; j < q; j++)
        v3[i] += m1[i][j] * v2[j];
}
//---------------------------------------------------------------------------
// Multiplica el vector horizontal[1][p] v1 por el vector vertical[p][1] v2 para obtener el
// escalar nRes
void MultVectorVector(double* v1, double* v2, double& dRes, int p)
{
  int i;

  dRes = 0;
  for (i = 0; i < p; i++)
    dRes += v1[i] * v2[i];
}
//---------------------------------------------------------------------------
// Multiplica el vector vertical[p][1] v1 por el vector horizontal[1][r] v2 para obtener la
// matriz m3
void MultVectorVector(double* v1, double* v2, Matriz m3, int p, int r)
{
  int i, k;

  for (i = 0; i < p; i++)
    for (k = 0; k < r; k++) 
    {
      m3[i][k] = v1[i] * v2[k];
    }
}
//---------------------------------------------------------------------------
// Intenta invertir la matriz[p][p] mp, 
// si no es posible devuelve false
bool InvierteMatriz(Matriz mp, int p)
{
  Matriz aux;
  int i, j, k, l;
  double *r1, *s1, co;
  bool inver = true;

  // Crea una matriz auxiliar y la inicializa con unos en la diagonal principal
  // y ceros en el resto

//  aux = new double *[p];
  for (i = 0; i < p; ++i) {
//    aux[i] = new double[p];
    for (j = 0; j < p; ++j)
      aux[i][j] = i == j;
  }

  // Crea dos vectores[p] auxiliares
  r1 = new double[p];
  s1 = new double[p];
  // Invierte la matriz
  for (i = 0; i < p - 1; ++i) {
    k = i + 1;
    while (fabs(mp[i][i]) < UMBRAL_INV) 
    {
      if (k == p) 
      {
        inver = false;
        goto final;
      }
      CopiaVector(r1, mp[i], p);
      CopiaVector(mp[i], mp[k], p);
      CopiaVector(mp[k], r1, p);
      CopiaVector(s1, aux[i], p);
      CopiaVector(aux[i], aux[k], p);
      CopiaVector(aux[k], s1, p);
      k++;
    }
    for (j = i + 1; j < p; ++j) {
      co = mp[j][i] / mp[i][i];
      for (l = 0; l < p; ++l) {
        mp[j][l] = mp[j][l] - co * mp[i][l];
        aux[j][l] = aux[j][l] - co * aux[i][l];
      }
    }
  }
  for (i = p - 1; i >= 1; --i)
    if (fabs(mp[i][i]) >= UMBRAL_INV)
      for (j = i - 1; j >= 0; --j) {
        co = mp[j][i] / mp[i][i];
        for (l = 0; l < p; ++l)
          aux[j][l] = aux[j][l] - co * aux[i][l];
      }
    else {
      inver = false;
      goto final;
    }
  for (i = 0; i < p; ++i)
    if (fabs(mp[i][i]) >= UMBRAL_INV)
      for (j = 0; j < p; ++j)
        aux[i][j] = aux[i][j] / mp[i][i];
    else {
      inver = false;
      goto final;
    }
  CopiaMatriz(mp, aux, p, p);
  // Libera la memoria de las variables auxiliares
final :
  delete [] s1;
  delete [] r1;
  /*
  for (i = 0; i < p; ++i)
    delete [] aux[i];
  delete [] aux;
  */
  return inver;
}
//---------------------------------------------------------------------------
void Rotacion(int p, double c, double s, int k, int l, Matriz m1)
// Rotación de Jacobi de la matriz m1
{
  double h, g;
  int j;

  h = c * c * m1[k][k] - 2 * c * s * m1[k][l] + s * s * m1[l][l];
  g = s * s * m1[k][k] + 2 * c * s * m1[k][l] + c * c * m1[l][l];
  m1[l][k] = c * s * (m1[k][k] - m1[l][l]) + (c * c - s * s) * m1[k][l];
  m1[k][l] = m1[l][k];
  m1[k][k] = h;
  m1[l][l] = g;
  for (j = 0; j < k; ++j) {
    h = c * m1[j][k] - s * m1[j][l];
    m1[j][l] = s * m1[j][k] + c * m1[j][l];
    m1[j][k] = h;
    m1[k][j] = m1[j][k];
    m1[l][j] = m1[j][l];
  }
  for (j = k + 1; j < l; ++j) {
    h = c * m1[k][j] - s * m1[j][l];
    m1[j][l] = s * m1[k][j] + c * m1[j][l];
    m1[k][j] = h;
    m1[j][k] = m1[k][j];
    m1[l][j] = m1[j][l];
  }
  for (j = l + 1; j < p; ++j) {
    h = c * m1[k][j] - s * m1[l][j];
    m1[l][j] = s * m1[k][j] + c * m1[l][j];
    m1[k][j] = h;
    m1[j][k] = m1[k][j];
    m1[j][l] = m1[l][j];
  }
}
//---------------------------------------------------------------------------
void Jacobi(Matriz m1, Matriz m2, int p)
// Descompone la matriz simétrica no invertible m1 en una diagonal (devuelta en
// m1) y los autovectores (devueltos en m2)
{
  Matriz m3, m4;
  int i, j, k, l;
  double max, theta, t, c, s;
/*
  // Crea dos matrices auxiliares m3 y m4
  m3 = new double *[p];
  m4 = new double *[p];
  for (i = 0; i < p; ++i) {
    m3[i] = new double[p];
    m4[i] = new double[p];
  }
  */
  // Inicializa las matrices cuadradas (p x p) m2 y m3 como identidad
  memset(m2,0,sizeof(double)*p*p);
  memset(m3,0,sizeof(double)*p*p);
  for (i = 0; i < p; ++i)
  {
      m2[i][i] = 1;
      m3[i][i] = 1;
  }
  do {
    max = 0;
    for (i = 0; i < p - 1; ++i)
      for (j = i + 1; j < p; ++j)
        if (fabs(m1[i][j]) > max) 
        {
          max = fabs(m1[i][j]);
          k = i;
          l = j;
        }
    if (p * max >= UMBRAL_JACOBI) 
    {
      theta = ((m1[l][l] - m1[k][k]) / m1[k][l]) / 2;
      if (fabs(theta) < UMBRAL_JACOBI)
        t = 1;
      else
        t = 1 / (fabs(theta) + sqrt(1 + theta * theta));
      if (theta < 0)
        t = -t;
      c = 1 / sqrt(1 + t * t);
      s = c * t;
      Rotacion(p, c, s, k, l, m1);

      m3[l][l] = m3[k][k] = c;
      m3[k][l] = -s;
      m3[l][k] = s;
      MultMatriz(m3, m2, m4, p, p, p);
      CopiaMatriz(m2, m4, p, p);

      //Dejamos m3 como identidad para proxima iteracion
      m3[l][l] = m3[k][k] = 1;
      m3[k][l] = 0;
      m3[l][k] = 0;
    }
  } while (p * max >= UMBRAL_JACOBI);
/*
  // Libera las dos matrices auxiliares m3 y m4
  for (i = 0; i < p; ++i) {
    delete [] m4[i];
    delete [] m3[i];
  }
  delete [] m4;
  delete [] m3;
  */
}

//---------------------------------------------------------------------------
// Invierte la matriz mat y devuelve la inversa en inv. Si mat no fuera
// invertible, usa el método de aproximación de Jacobi
void Invierte(Matriz mat, Matriz inv, int nVal)
{
  Matriz mAux, mAuxT, m3;
  int i, k;

  if (mat!=inv)
    CopiaMatriz(inv, mat, nVal, nVal);
  // Trata de invertir la matriz inv
  if (! InvierteMatriz(inv, nVal)) 
  {
      /*
    // Si la matriz de covarianzas no es invertible, crea tres matrices
    // auxiliares
    mAux = new double *[nVal];
    mAuxT = new double *[nVal];
    m3 = new double *[nVal];
    for (i = 0; i < nVal; i++) {
      mAux[i] = new double[nVal];
      mAuxT[i] = new double[nVal];
      m3[i] = new double[nVal];
    }
    */
    // Copia la matriz de covarianzas mat en inv
    CopiaMatriz(inv, mat, nVal, nVal);
    Jacobi(inv, mAux, nVal);
    for (i = 0; i < nVal; ++i)
      if (fabs(inv[i][i]) > UMBRAL_MAHAL)
        inv[i][i] = 1 / inv[i][i];
      else
        inv[i][i] = 0;
//    double *v2;
//    v2 = new double[nVal];
    for (i = 0; i < nVal; ++i)
      for (k = 0; k < nVal; ++k)
        mAuxT[i][k] = mAux[k][i];
    MultMatriz(mAuxT, inv, m3, nVal, nVal, nVal);
    MultMatriz(m3, mAux, inv, nVal, nVal, nVal);
//    delete [] v2;
    /*
    // Libera la matriz auxiliar
    for (i = 0; i < nVal; i++) {
      delete [] m3[i];
      delete [] mAuxT[i];
      delete [] mAux[i];
    }
    delete [] m3;
    delete [] mAuxT;
    delete [] mAux;
    */
  }
}
//---------------------------------------------------------------------------
