/*****************************************************************************

    Interfaz para control de los dispositivos del microscopio LEICA

*****************************************************************************/

#ifndef CONTROL_MICROSCOPIO_LEICA_H
#define CONTROL_MICROSCOPIO_LEICA_H

// include declarations of dataclasses for different devices
#include "..\ProcesoAux\estructuras.h"

#include ".\Leica_SDK_2.3\C++\include\ahm.h"		// definitions of Leica Alternative hardware model
#include ".\Leica_SDK_2.3\C++\include\ahwbasic.h"	// include basic control interfaces (BasicControlValue)
#include ".\Leica_SDK_2.3\C++\include\ahwmic.h"	// include basic control interfaces (BasicControlValue)
#include ".\Leica_SDK_2.3\C++\include\ahwmicpropid.h" // include property specific definitions
#include ".\Leica_SDK_2.3\C++\include\ahwprop2.h" // include property interfaces

using namespace ahm;

#define CUBO_REFLECTOR_SMITH		1
#define CUBO_REFLECTOR_ESPEJO		3	

class BasicControlValue;
class CLeica
{
private:
	ahm::Unit*	m_pRootUnit;
public:

	CLeica();
	~CLeica();


	int		ini_control_microscopio(int posFiltro,double voltLamp, limitesMtb *limMtb);
	int		fin_control_microscopio();

	void	mspGo(int xyz, double pos);
	double	mspWhere(int xyz);
	void	mspGoAtThisSpeed(int xyz, double velo);


	void	mspSetLamp(double val);
	double	mspGetLamp();

    bool  CLeica::mspSetTurret(int val);
    int CLeica::mspGetTurret();
    CString CLeica::mspGetNombreTurret(int nTurret);

	bool	mspSetObjetivo(int val);
	int	    mspGetObjetivo();
    CString CLeica::mspGetNombreObjetivo(int nObjetivo);
    CString CLeica::mspGetNombreObjetivo2(int nObjetivo);

	int		mspSetRevolver(int val);
	int		mspGetRevolver();


private:
	void						printSpaces(int depth);
	void						printUnitTree(ahm::Unit *pUnit, int depth);
	void						printUnitType(ahm::UnitType* pUnitType);
    Unit*						findUnit(Unit *pUnit, ahm::TypeId typeId);
	ahm::BasicControlValue*		getBasicControl(Unit *pUnit);
	ahm::DeltaControlValue*     getDeltaControl(Unit *pUnit);
	ahm::PropertyValue *		findPropertyValue(Unit *pUnit,					                                   iop::int32 propertyId, int nIndex = -1);
	ahm::PropertyValue *		findPropertyValue(ahm::Properties *pProperties,                     iop::int32 propertyId, int nIndex = -1);
	ahm::PropertyValue *		findPropertyValue(ahm::PropertyValue *pPropertyValue,       iop::int32 propertyId, int nIndex = -1);
	void						mspRevolverInfo(limitesMtb *limMtb);
	void						mspLampInfo(limitesMtb *limMtb);
    bool  CLeica::mspSetContrastingMethod(int nContrastingMethod);

/*
// AUXILIARES
int		coinciden(int xyz, double v1, double v2);
int		coincidenP(point p1, point p2);
char	nombre_eje(int xyz);
int		mspEsperaRevolverInicio();
#if !defined (__BORLANDC__)
int		mspFijaPos(int xyz, double pos);
#endif
void	mspGetStageSpeed(double *vx, double *vy);
void	mspSetStageSpeed(double vx, double vy);
void	mspGetFocSpeed(double *vz);
void	mspStop(int xyz);
*/
};

#endif 

