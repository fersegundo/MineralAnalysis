/*****************************************************************************
	control_microscopio_zeiss.cpp		
    
    Rutinas para control de los dispositivos del microscopio ZEISS

	Autores:	FSG
	Copyright:	© AITEMIN 2008
*****************************************************************************/

#include "stdafx.h"

#if defined (__BORLANDC__)
#include <vcl.h>
#endif

#if !defined (__BORLANDC__)
#include <iostream>
#include <stdio.h>
#endif
#include <math.h>

#include "control_microscopio_leica.h"
#include "..\ProcesoAux\gestion_mensajes.h"
#include "proptool.h" //prubas leica

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

// defines para cambio de revolver de polarizacion
#define BOTON_POL_ARRIBA    4
#define BOTON_POL_ABAJO     5

#define VELOCIDAD_Z                80

// LAMPARA
#define MAX_LAMPARA                255
#define MAX_VOLTAJE                12

using namespace std;

////////////////////////////////////////////// CLASES LEICA
template <class clazz> clazz* find_itf(ahm::Unit *pUnit, iop::int32 iid)
{
	if(pUnit && pUnit->interfaces()){
		ahm::Interface* pInterface = pUnit->interfaces()->findInterface(iid);
		return pInterface? (clazz*) pInterface->object() : 0;
	}
	return 0;
}
iop::string safe(iop::string sz){ return sz?sz:""; }

class MyUnitBase{
public:
	virtual bool getMethodProperty(std::string &strbuf, iop::int32 index) = 0;
	virtual bool getNameProperty(std::string &strbuf, iop::int32 index) = 0;

	virtual iop::int32 minPos()
    {
		return m_pbcv ? m_pbcv->minControlValue():0;
	}
	virtual iop::int32 maxPos()
    {
		return m_pbcv ? m_pbcv->maxControlValue():0;
	}
	virtual iop::int32 current()
    {
		return m_pbcv ? m_pbcv->getControlValue():0;
	}

	virtual void moveto(iop::int32 pos)
    {
		if(m_pUnit && m_pUnit->type() && m_pUnit->type()->isA(MICROSCOPE_MOTORIZED_UNIT)){
			if(m_pbcv)m_pbcv->setControlValue(pos);
		}
		else {

			char szbuf[128];
			
			do {
				cout << "pos is now: "<<  current() << endl;

				szbuf[0] = '\0';

				if(current()!=pos){
					cout << endl << "Please move to position" <<  pos << ">";
					cin.getline(szbuf, sizeof(szbuf)-1);
					
				}
			}while(current()!=pos || szbuf[0] =='x');
			}
	}

	virtual iop::string name()
    { 
        return safe(m_pUnit ? m_pUnit->name():0); 
    }

protected:
	MyUnitBase(Unit *pUnit) : m_pUnit(pUnit)
    {
		m_pProperties = find_itf<ahm::Properties>(pUnit, IID_PROPERTIES);
		m_pbcv = find_itf<ahm::BasicControlValue>(pUnit, IID_BASIC_CONTROL_VALUE);
	}
protected:
	virtual ahm::Properties* getIndexStruct(iop::int32 index) = 0;
	ahm::Unit *m_pUnit;
	ahm::Properties *m_pProperties;
	ahm::BasicControlValue* m_pbcv;
};


class MyNosepiece : public MyUnitBase{
public:
	MyNosepiece(ahm::Unit*pUnit) : MyUnitBase(pUnit)
    {
    }

	virtual bool getMethodProperty(std::string &strbuf, iop::int32 index)
    {
		return prop_tools::getStringValue(this->getIndexStruct(index), PROP_METHODS, strbuf);
	}
	virtual bool getNameProperty(std::string &strbuf, iop::int32 index)
    {
		std::string strname, straper;
		if(prop_tools::getStringValue(this->getIndexStruct(index), PROP_MAGNIFICATION, strname)
			&& prop_tools::getStringValue(this->getIndexStruct(index), PROP_APERTURE, straper)
		){
				strbuf = strname + string("x/") + straper;

				return true;
		}

		return false;


	}

	bool isDryObjective(iop::int32 i)
    {
		std::string strtype;
		if(prop_tools::getStringValue(this->getIndexStruct(i), PROP_OBJECTIVE_TYPE, strtype)){

			if(!strtype.empty()){
				if(strtype[0] == 'D'){
					return true;
				}
				else {
					iop::int32 nFlag=0;
					if(prop_tools::getIntValue(this->getIndexStruct(i), PROP_OBJECTIVE_TYPE_COMBI_FLAG, nFlag)){
						return nFlag!=0;
					}

				}

			}



		}
		return false;
	}
protected:
	virtual ahm::Properties* getIndexStruct(iop::int32 index)
    {
		if(m_pProperties){
			ahm::Property *pProp = m_pProperties->findProperty(PROP_NOSEPIECE);

			ahm::PropertyValue *pElem = prop_tools::getIndexValue(pProp, index);
			if(prop_tools::properties(pElem)){
				pProp = prop_tools::properties(pElem)->findProperty(PROP_OBJECTIVE);
				return prop_tools::properties(pProp);
			}

		}
		return 0;
	}
};


class MyILTurret : public MyUnitBase{
public:
	MyILTurret(ahm::Unit*pUnit) : MyUnitBase(pUnit)
    {
    }

	virtual bool getMethodProperty(std::string &strbuf, iop::int32 index)
    {
		return prop_tools::getStringValue(this->getIndexStruct(index), PROP_ILTURRET_CUBE_METHODS, strbuf);
	}
	virtual bool getNameProperty(std::string &strbuf, iop::int32 index)
    {
		std::string strname;
		if(prop_tools::getStringValue(this->getIndexStruct(index), PROP_ILTURRET_CUBE_NAME, strname)
		){
				strbuf = strname ;
				return true;
		}

		return false;
	}

protected:
	virtual ahm::Properties* getIndexStruct(iop::int32 index)
    {
		if(m_pProperties){
			ahm::Property *pProp = m_pProperties->findProperty(PROP_ILTURRET);

			ahm::PropertyValue *pElem = prop_tools::getIndexValue(pProp, index);
			if(prop_tools::properties(pElem)){
				pProp = prop_tools::properties(pElem)->findProperty(PROP_ILTURRET_CUBE);
				return prop_tools::properties(pProp);
			}

		}
		return 0;
	}
};
////////////////////////////////////////////// FIN CLASES LEICA


CLeica::CLeica()
{
	m_pRootUnit = NULL;
}

CLeica::~CLeica()
{
}

void CLeica::printSpaces(int depth){
	for(int i=0;i<depth;i++)
		cout << "  ";
}
void CLeica::printUnitTree(ahm::Unit *pUnit, int depth){
	if(pUnit){
		printSpaces(depth);
		cout << "Unit: " << pUnit->name() << " ";
		printUnitType(pUnit->type());
		cout << endl;
		if(pUnit->units()){
			// print out all child units
			for(iop::int32 i = 0;i<pUnit->units()->numUnits();i++){
				printUnitTree(pUnit->units()->getUnit(i), depth+1);
			}
		}
	}
}
void CLeica::printUnitType(ahm::UnitType* pUnitType){

	if(pUnitType){
		ostream& os = cout;
		os << "Type: [";
		// iterate overa all type ids
		for(iop::int32 i=0;i<pUnitType->numTypeIds();i++){
	
			if(i>0) os << " "; // print separator
		// test if the type id is the exposed id
			bool flagExposedId = pUnitType->exposedTypeId() == pUnitType->getTypeId(i);

			// mark exposed id
			if(flagExposedId) os << "<";

			os << pUnitType->getTypeId(i);
			if(flagExposedId) os << ">";
		}
		os << "]";
	}
}

/*************************** ini_control_microscopio *************************
	Función para inicializar las funciones de control de los dispositivos
	del microscopio.
	posFiltro - Filtro predeterminado de la Rueda de filtros
	voltLamp - voltaje configurado de trabajo de la lampara del microscopio
	filtroUltimaBanda - filtro correspondiente a la ultima banda a tomar
	Devuelve -1 si no se encuentra el microscopio
*****************************************************************************/
int CLeica::ini_control_microscopio(int posFiltro,double voltLamp, limitesMtb *limMtb)
{
	char *fn = "ini_control_microscopio";

	if (theHardwareModel())
	{
		try{
			m_pRootUnit = theHardwareModel()->getUnit("DM6000-COM1");
			if (m_pRootUnit)
			{
				printUnitTree(m_pRootUnit,0); //depth 0
			}
		}catch(...){
			return -1;
		}
	}

	//	Una vez comprobados los dispositivos de los que consta el microscopio,
	//	fijamos los parámetros relativos al mismo para la toma de imágenes. 
	mspRevolverInfo(limMtb);	// Características de la rueda de filtros.


	if (voltLamp > MAX_VOLTAJE || voltLamp < 0)  {
		error_fatal(fn, "Valor inadmisible de tension de la lampara", 0);
		return 1;
	}
	mspSetLamp(voltLamp);		// Posición de la lámpara.

	if (posFiltro > limMtb->MaxPosRevolver || posFiltro < 1)  {
		error_fatal(fn, "Valor inadmisible de MTB_posFiltro", 0);
		return 1;
	}
	this->mspSetRevolver(posFiltro);// Posición de la rueda de filtros.

    /*
    //inicializamos velocidad
   double pos =  mspWhere(Z_);
   	mspGoAtThisSpeed(Z_, VELOCIDAD_Z );
   	mspGoAtThisSpeed(Z_, 0 ); //halt
   	mspGo(Z_, pos ); //dejarlo en la misma posicion
*/

    return 0;
}


/*************************** fin_control_microscopio *************************
	Función para desactivar las funciones de control de los dispositivos
	del microscopio.
	Además baja el nivel de intensidad de la lámpara a un nivel de 'reposo'.
*****************************************************************************/
int CLeica::fin_control_microscopio()
{

	if (m_pRootUnit)
	{
		m_pRootUnit->dispose();
	}

	theHardwareModel()->dispose();

	return 0;
}


/////////////////////////////////////////////////////////////////////////////////
//
//  FUNCIONES AUXILIARES COMUNES
//
/////////////////////////////////////////////////////////////////////////////////


// findUnit: tool function to find a unit with a given type id in the unit tree
Unit*   CLeica::findUnit(Unit *pUnit, ahm::TypeId typeId){
	// test unit's type for given typeId
	if(pUnit && pUnit->type()){
		if(pUnit->type()->isA(typeId)){
			return pUnit; // ok it is!
		}
		else {
			if(pUnit->units()){
				// recursively find unit in child units
				for(iop::int32 i = 0;i<pUnit->units()->numUnits();i++){
					Unit *pDeepUnit = findUnit(pUnit->units()->getUnit(i), typeId);
					if(pDeepUnit){
						return pDeepUnit; // stop recursion
					}
				}

			}

		}
	}

	return 0; // unit with type id was not found
}

// getBasicControl: tool function to find the basic control value interface of the given unit
ahm::BasicControlValue*   CLeica::getBasicControl(Unit *pUnit)
{
	ahm::BasicControlValue *pBasicControlValue = NULL;

	if(pUnit->interfaces())
	{
		// find the interface in the interface list of the unit
		Interface *pInterface = pUnit->interfaces()->findInterface(IID_BASIC_CONTROL_VALUE);
		if(pInterface)
		{ // bingo
			// safe to cast the object
			pBasicControlValue = (ahm::BasicControlValue*) pInterface->object();
		}
	}

	return pBasicControlValue; // unit with type id was not found
}

// getDeltaControl: tool function to find the delta control value interface of the given unit
ahm::DeltaControlValue*   CLeica::getDeltaControl(Unit *pUnit)
{
	ahm::DeltaControlValue *pControlValue = NULL;

	if(pUnit->interfaces())
	{
		// find the interface in the interface list of the unit
		Interface *pInterface = pUnit->interfaces()->findInterface(IID_DELTA_CONTROL_VALUE);
		if(pInterface)
		{ // bingo
			// safe to cast the object
			pControlValue = (ahm::DeltaControlValue*) pInterface->object();
		}
	}

	return pControlValue; // unit with type id was not found
}

// getPropertyValue: tool function to find the PropertyValue within the given property 
// within the Properties TREE of the given unit
// nIndex - if there is an array property within the properties tree, continue looking through branch nIndex
ahm::PropertyValue *   CLeica::findPropertyValue(Unit *pUnit, iop::int32 propertyId, int nIndex)
{
	if(pUnit->interfaces())
	{
		// find the interface in the interface list of the unit
		Interface *pInterface = pUnit->interfaces()->findInterface(IID_PROPERTIES);
		if(pInterface)
		{ // bingo
			// safe to cast the object
			ahm::Properties *pProps = (ahm::Properties*)pInterface->object();
			return findPropertyValue(pProps, propertyId,nIndex);
		}
	}

	return NULL;
}

// Funcion recursiva para encontrar el PropertyValue con id dado en el conjunto de propiedades dado
// nIndex - if there is an array property within the properties tree, continue looking through branch nIndex
//AUXILIAR a findPropertyValue
ahm::PropertyValue * CLeica::findPropertyValue(ahm::Properties *pProperties, iop::int32 propertyId, int nIndex)
{
	if (pProperties) 
	{
		// read properties
		if (pProperties->findProperty(propertyId))
			return pProperties->findProperty(propertyId)->value();
		else
		{
			// no encontrado, quizas dentro de un sub-properties o sub-array de properties
			int i;
			int numProps = pProperties->numProperties();
            ahm::PropertyValue *pFoundValue = NULL;
			for(i=0;i<numProps;i++)
			{
				ahm::Property *pProperty = pProperties->getProperty(i);
	            if(pProperty)
	            {	
				    try 
				    {
                        ahm::PropertyValue *pPropertyValue = pProperty->value();
                        pFoundValue = findPropertyValue(pPropertyValue, propertyId, nIndex);
                        if (pFoundValue != NULL)
                            return pFoundValue;
                    } // try
				    catch (...){//ahm::Exception& ex){
                        // Continuar buscando 
				    }
                } //if (pProperty)
            } // for
		}
	}

	return NULL;
}

// Funcion recursiva para encontrar el PropertyValue con id dado en la propiedad dada
// nIndex - if there is an array property within the properties tree, continue looking through branch nIndex. Not prepared for nested arrays
//AUXILIAR a findPropertyValue
ahm::PropertyValue * CLeica::findPropertyValue(ahm::PropertyValue *pPropertyValue, iop::int32 propertyId, int nIndex)
{
	if (pPropertyValue->derivedType() == ahm::PropertyValue::TYPE_PROPERTIES)
	{
		// cast to derived type
		PropertyValueProperties* pPropertiesValue = (PropertyValueProperties *) pPropertyValue;
		// recursively call function call on "child" properties
		ahm::Properties *pProperties = pPropertiesValue->properties();
		return findPropertyValue(pProperties, propertyId);
	}
    else if (pPropertyValue->derivedType() == ahm::PropertyValue::TYPE_FIXED_ARRAY)
    {
        // cast to derived type
        PropertyValueArray* pArray = (PropertyValueArray *) pPropertyValue;

        if (nIndex < pArray->minIndex() || nIndex > pArray->maxIndex())
        {
            // ERROR nIndex out of bounds
            return NULL;
        }
        // Continuamos por indice indicado
		return findPropertyValue(pArray->getValue(nIndex), propertyId); // nIndex = -1 porque solo permitimos un array
    } // TYPE_FIXED_ARRAY
    else
    {
        return NULL;
    }
}

//////////////////////////////////////////////////////////////////////////////////
//
// STAGE
//
//////////////////////////////////////////////////////////////////////////////////

/**************************  mspGo  *****************************************
	Función para ordenar un movimiento absoluto en uno de los ejes. EN MICRAS
    Bloqueante: la funcion no acaba hasta que se completa el movimiento
****************************************************************************/
void  CLeica::mspGo(int xyz, double pos)
{
	//Determinar que coordenada se mueve
	ahm::TypeId typeId;
	if (xyz == Z_)
	{
		typeId = MICROSCOPE_ZDRIVE;
	}
	else if (xyz == X_)
	{
		typeId = MICROSCOPE_X_UNIT;
	}
	else if (xyz == Y_)
	{
		typeId = MICROSCOPE_Y_UNIT;
	}
	else
	{
		//ERROR
		return;
	}

	// Encontrar unidad correspondiente
	Unit *pUnit = findUnit( m_pRootUnit, typeId );
	if ( ! pUnit) {
		error_leve("mspLampInfo", "Lámpara inexistente");
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);

		// Mover
		if(pControlValue){
			// Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
			if(pControlValue->metricsConverters()){
				MetricsConverter *pConverter = 
					pControlValue->metricsConverters()->findMetricsConverter(METRICS_MICRONS);
				if(pConverter){
					// get value
					try 
					{
						iop::int32 valorNativo = pConverter->getControlValue(pos);
						pControlValue->setControlValue(valorNativo);
					}
					catch (ahm::Exception& ex){
						iop::string err = ex.errorText();
						error_leve("mspGo", "excepcion al mover");
					}
				}
				else {
						error_leve("mspGo", "No se puede mover. Conversor de unidades no encontrado");
				}
			}
		}
	}
}


/**************************  mspWhere  **************************************
	Función para obtener la posición actual en uno de los ejes. EN MICRAS
****************************************************************************/
double CLeica::mspWhere(int xyz)
{
	//Determinar que coordenada se mueve
	ahm::TypeId typeId;
	if (xyz == Z_)
	{
		typeId = MICROSCOPE_ZDRIVE;
	}
	else if (xyz == X_)
	{
		typeId = MICROSCOPE_X_UNIT;
	}
	else if (xyz == Y_)
	{
		typeId = MICROSCOPE_Y_UNIT;
	}
	else
	{
		//ERROR
		return -1;
	}

	// Encontrar unidad correspondiente
	Unit *pUnit = findUnit( m_pRootUnit, typeId );
	if ( ! pUnit) {
		error_leve("mspLampInfo", "Lámpara inexistente");
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);

		// Operacion
		if(pControlValue){
			// Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
			if(pControlValue->metricsConverters()){
				MetricsConverter *pConverter = 
					pControlValue->metricsConverters()->findMetricsConverter(METRICS_MICRONS);
				if(pConverter){
					// get value
					iop::int32 valorNativo = pControlValue->getControlValue();
					return pConverter->getMetricsValue(valorNativo);
				}
				else {
						error_leve("mspWhere", "No se puede obtener posicion. Conversor de unidades no encontrado");
				}
			}
		}
	}

	return -1;
}


/**************************  mspGoAtThisSpeed  ******************************
	Función para moverse en uno de los ejes a una velocidad dada. Dejando la velocidad como estuviese
    La velocidad tiene signo y esta en MICRAS/SEGUNDO
****************************************************************************/
void  CLeica::mspGoAtThisSpeed(int xyz, double velo)	// Función para fijar una velocidad
{												// absoluta en uno de los ejes.
	//Determinar que coordenada se mueve
	ahm::TypeId typeId;
	if (xyz == Z_)
	{
		typeId = MICROSCOPE_ZDRIVE;
	}
	else if (xyz == X_)
	{
		typeId = MICROSCOPE_X_UNIT;
	}
	else if (xyz == Y_)
	{
		typeId = MICROSCOPE_Y_UNIT;
	}
	else
	{
		//ERROR
	}

	// Encontrar unidad correspondiente
	Unit *pUnit = findUnit( m_pRootUnit, typeId );
	if ( ! pUnit) 
    {
		error_leve("mspLampInfo", "Lámpara inexistente");
	}
	else
	{

	    if(pUnit->interfaces())
	    {
            if (velo == 0)
            {
                //HALT
		        Interface *pInterfaceHalt = pUnit->interfaces()->findInterface(IID_HALT_CONTROL_VALUE);
		        if(pInterfaceHalt )
		        { // bingo
			        // safe to cast the object
	                ahm::HaltControlValue *pControlValueHalt = (ahm::HaltControlValue*) pInterfaceHalt->object();

		            // Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
		            if(pControlValueHalt)
                    {
                            pControlValueHalt->halt();
                    } // if(pControlValue
                } //if(pInterface)
            } // if velo== 0
            else
            {
                // ESTABLECER VELOCIDAD Y MOVER
		        // find the interface in the interface list of the unit
		        ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		        Interface *pInterfaceVelocity = pUnit->interfaces()->findInterface(IID_BASIC_CONTROL_VALUE_VELOCITY);
                Interface *pInterfaceAsync = pUnit->interfaces()->findInterface(IID_BASIC_CONTROL_VALUE_ASYNC);
		        if(pInterfaceVelocity && pInterfaceAsync)
		        { // bingo
			        // safe to cast the object
	                ahm::BasicControlValueVelocity *pControlValueVelocity = (ahm::BasicControlValueVelocity*) pInterfaceVelocity->object();
                    ahm::BasicControlValueAsync *pControlValueAsync = (ahm::BasicControlValueAsync*) pInterfaceAsync->object();

		            // Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
		            if(pControlValueVelocity && pControlValueAsync && pControlValueVelocity->metricsConverters() )
                    {
			            MetricsConverter *pConverterVelocity = 
				            pControlValueVelocity->metricsConverters()->findMetricsConverter(METRICS_MICRONS_PER_SECOND);
			            if(pConverterVelocity)
                        {
                            // guardar velocidad (para no cambiarla de verdad)
				            int vel_nativa = pControlValueVelocity->getControlValue();
                            //set speed
				            pControlValueVelocity->setControlValue(pConverterVelocity->getControlValue(abs(velo)));
                            // comenzamos el movimiento
                            int min = pControlValue->minControlValue();
                            int max = pControlValue->maxControlValue();
                            AsyncResult* res = NULL;
                            if (velo > 0)
                                res = pControlValueAsync->setControlValueAsync(max);
                           else
                                res = pControlValueAsync->setControlValueAsync(min);
                           res->dispose();
                           // dejar la velocidad como estaba
 				            pControlValueVelocity->setControlValue(vel_nativa);
                       }
			           else 
					            error_leve("mspGoAtThisSpeed", "No se puede establecer velocidad. Conversor de unidades no encontrado");
                    } // if(pControlValue
                } //if(pInterface)
            } // else velo!= 0
        } //if (pUnit->interfaces())
    } // else pUnit
}



//////////////////////////////////////////////////////////////////////////////////
//
// REVOLVER
//
//////////////////////////////////////////////////////////////////////////////////

/***************************  mspSetRevolver  *******************************
	Función para fijar la posición del revolver.
    No gestiona espera.
*****************************************************************************/
int  CLeica::mspSetRevolver(int val)
{
	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_IL_FIELD_DIAPHRAGM );
	if ( ! pUnit) {
		error_leve("mspSetRevolver", "Revolver de diafragma inexistente");
        return -1;
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		// Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
		if(pControlValue && val >= pControlValue->minControlValue() && val <= pControlValue->maxControlValue())
			pControlValue->setControlValue(val);
        else
            return -1;
    } // else pUnit

	return 0; //OK
}

/***************************  mspGetRevolver  *******************************
	Función para leer la posición de actual del revolver.
*****************************************************************************/
int CLeica::mspGetRevolver()
{
	unsigned int  val = 0;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_IL_FIELD_DIAPHRAGM );
	if ( ! pUnit) {
		error_leve("mspSetRevolver", "Revolver de diafragma inexistente");
        return -1;
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		// Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
		if(pControlValue && val >= pControlValue->minControlValue() && val <= pControlValue->maxControlValue())
			return pControlValue->getControlValue();
        else
            return -1;
    } // else pUnit

	return -1;
}

/***************************  mspRevolverInfo  ******************************
	Función para obtener información relativa a la rueda de filtros:
	  - Tipo de rueda (motorizado[2], manual[1], no disponible[0]
	  - Nombre de la rueda de filtros.
	  - Número máximo de posiciones de la rueda
		(almacenada en variable global).
	Devuelve el nombre de la rueda de filtros.
*****************************************************************************/
void CLeica::mspRevolverInfo(limitesMtb *limMtb)
{
	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_IL_FIELD_DIAPHRAGM );
	if ( ! pUnit) {
		error_leve("mspSetRevolver", "Revolver de diafragma inexistente");
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		// Fijar nivel. Pero antes hay que convertir las unidades a las nativas del microscopio
		if(pControlValue)
        {
		    // retrieve the current value
		    limMtb->MaxPosRevolver = pControlValue->maxControlValue();
        }
    } // else pUnit
}



//////////////////////////////////////////////////////////////////////////////////
//
// LAMP
//
//////////////////////////////////////////////////////////////////////////////////

/***************************  mspSetLamp  ***********************************
	Función para fijar un nivel de intensidad de la luz. EN VOLTIOS
    NOTA: No se conoce la relacion entre voltios y unidades internas. Para un calculo aproximado, 
    suponemos que los voltios maximos son 12V
*****************************************************************************/
void  CLeica::mspSetLamp(double val)
{
	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_LAMP );

	if ( ! pUnit) {
		error_leve("mspLampInfo", "Lámpara inexistente");
		return;
	}

	ahm::BasicControlValue *pBasicControlValue = getBasicControl(pUnit);

	if(pBasicControlValue){
		// set value
        int valor_nativo = floor(val * MAX_LAMPARA / MAX_VOLTAJE);
		pBasicControlValue->setControlValue( valor_nativo);
	}
}

/***************************  mspGetLamp  ***********************************
	Función para leer el nivel de intensidad de la luz en voltios EN VOLTIOS
    NOTA: No se conoce la relacion entre voltios y unidades internas. Para un calculo aproximado, 
    suponemos que los voltios maximos son 12V
*****************************************************************************/
double  CLeica::mspGetLamp()
{
	double  val = -1;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_LAMP );

	if ( ! pUnit) {
		error_leve("mspLampInfo", "Lámpara inexistente");
	}
	else
	{
		ahm::BasicControlValue *pBasicControlValue = getBasicControl(pUnit);

		if(pBasicControlValue){
			// get value
			int valor_nativo = pBasicControlValue->getControlValue();
            val = valor_nativo * MAX_VOLTAJE / MAX_LAMPARA;
		}
	}

	return  val;
}


//////////////////////////////////////////////////////////////////////////////////
//
// CUBOS (IL TURRET)
//
//////////////////////////////////////////////////////////////////////////////////
/*
bool  CLeica::mspSetTurret(int val)
{
	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_IL_TURRET );
	if ( ! pUnit) {
		error_leve("mspSetObjetivo", "Rueda de objetivos inexistente");
        return false;
	}
	else
	{
        MyILTurret unit(pUnit);
        unit.moveto(val);
        return true;
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		// Fijar nivel. 
		if(pControlValue && val >= pControlValue->minControlValue() && val <= pControlValue->maxControlValue())
			pControlValue->setControlValue(val);
        else
            return false;
    } // else pUnit

	return true; //OK
}
*/
bool  CLeica::mspSetContrastingMethod(int nContrastingMethod)
{
	MicroscopeContrastingMethods* pMethods = find_itf<MicroscopeContrastingMethods>(m_pRootUnit, IID_MICROSCOPE_CONTRASTING_METHODS);
	pMethods->setContrastingMethod(pMethods->supportedMethods()->getId(nContrastingMethod));

	return true; //OK
}

bool  CLeica::mspSetTurret(int val)
{
	mspSetContrastingMethod(val);

	return true; //OK
}

int CLeica::mspGetTurret()
{
	unsigned int  val = 0;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_IL_TURRET );
	if ( ! pUnit) {
		error_leve("mspGetObjetivo", "Rueda de cubos inexistente");
        return -1;
	}
	else
	{
//	    MyILTurret ilt(pUnit);
//        return ilt.current();

		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		if(pControlValue)
			return pControlValue->getControlValue();
        else
            return -1;
    } // else pUnit

	return -1;
}

CString CLeica::mspGetNombreTurret(int nTurret)
{
	unsigned int  val = 0;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_IL_TURRET );
	if ( ! pUnit) {
		error_leve("mspGetObjetivo", "Rueda de cubos inexistente");
        return "";
	}
	else
	{
        MyILTurret unit(pUnit);
        std::string pstrValue;
        unit.getNameProperty(pstrValue,nTurret);
        return pstrValue.c_str();
/*
		ahm::PropertyValue *pPropertyValue = findPropertyValue(pUnit, PROP_OBJECTIVE, nTurret);
		if(pPropertyValue)
        {
            iop::int32 nType = pPropertyValue->derivedType();
            iop::int32 nTypeString = ahm::PropertyValue::TYPE_STRING;

            std::string pstrValue;

            if (nType == nTypeString)
            {
	            ahm::PropertyValueString* propValue = (ahm::PropertyValueString*) pPropertyValue;
	            ahm::StringResult *pStrResult = propValue->getValue(); 
				if(pStrResult)
                {
					pstrValue = pStrResult->value();
					pStrResult->dispose();
                    return pstrValue.c_str();
				}
                else
                    return "";
            }
            else
                return "";
        }
        else
            return "";
*/
    } // else pUnit

	return "";
}


//////////////////////////////////////////////////////////////////////////////////
//
// OBJETIVO
//
//////////////////////////////////////////////////////////////////////////////////

/***************************  mspSetObjetivo  *******************************
*****************************************************************************/
bool  CLeica::mspSetObjetivo(int val)
{
	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_NOSEPIECE );
	if ( ! pUnit) {
		error_leve("mspSetObjetivo", "Rueda de objetivos inexistente");
        return false;
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		// Fijar nivel. 
		if(pControlValue && val >= pControlValue->minControlValue() && val <= pControlValue->maxControlValue())
			pControlValue->setControlValue(val);
        else
            return false;
    } // else pUnit

	return true; //OK
}

/***************************  mspGetObjetivo  *******************************
	Función para leer la posición de actual del objetivo.
*****************************************************************************/
int CLeica::mspGetObjetivo()
{
	unsigned int  val = 0;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_NOSEPIECE );
	if ( ! pUnit) {
		error_leve("mspGetObjetivo", "Rueda de objetivos inexistente");
        return -1;
	}
	else
	{
		ahm::BasicControlValue *pControlValue = getBasicControl(pUnit);
		if(pControlValue)
			return pControlValue->getControlValue();
        else
            return -1;
    } // else pUnit

	return -1;
}

/***************************  mspGetNombreObjetivo  *******************************
	Devuelve el nombre del objetivo con indice nIndice
*****************************************************************************/
CString CLeica::mspGetNombreObjetivo(int nObjetivo)
{
	unsigned int  val = 0;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_NOSEPIECE );
	if ( ! pUnit) {
		error_leve("mspGetObjetivo", "Rueda de objetivos inexistente");
        return "";
	}
	else
	{
        MyNosepiece nsp(pUnit);
        std::string pstrValue;
        nsp.getNameProperty(pstrValue,nObjetivo);
        return pstrValue.c_str();
/*
		ahm::PropertyValue *pPropertyValue = findPropertyValue(pUnit, PROP_OBJECTIVE, nObjetivo);
		if(pPropertyValue)
        {
            iop::int32 nType = pPropertyValue->derivedType();
            iop::int32 nTypeString = ahm::PropertyValue::TYPE_STRING;

            std::string pstrValue;

            if (nType == nTypeString)
            {
	            ahm::PropertyValueString* propValue = (ahm::PropertyValueString*) pPropertyValue;
	            ahm::StringResult *pStrResult = propValue->getValue(); 
				if(pStrResult)
                {
					pstrValue = pStrResult->value();
					pStrResult->dispose();
                    return pstrValue.c_str();
				}
                else
                    return "";
            }
            else
                return "";
        }
        else
            return "";
*/
    } // else pUnit

	return "";
}

/***************************  mspGetNombreObjetivo  *******************************
	Devuelve el nombre del objetivo con indice nIndice
*****************************************************************************/
CString CLeica::mspGetNombreObjetivo2(int nObjetivo)
{
	unsigned int  val = 0;

	Unit *pUnit = findUnit( m_pRootUnit, MICROSCOPE_NOSEPIECE );
	if ( ! pUnit) {
		error_leve("mspGetObjetivo", "Rueda de objetivos inexistente");
        return "";
	}
	else
	{
		ahm::Properties *pProperties;
        if(pUnit && pUnit->interfaces())
        {
		    ahm::Interface* pInterface = pUnit->interfaces()->findInterface(IID_PROPERTIES);
            if (pInterface)
                pProperties = (ahm::Properties*) pInterface->object();
        }
        //getIndexStruct
        ahm::Properties* pIndexStruct;
		if(pProperties)
        {
			ahm::Property *pProp = pProperties->findProperty(PROP_NOSEPIECE);

			ahm::PropertyValue *pElem;
            //prop_tools::getIndexValue
	        if(pProp->value() && pProp->value()->derivedType() == ahm::PropertyValue::TYPE_FIXED_ARRAY)
            {
                ahm::PropertyValueArray* pArray = (ahm::PropertyValueArray*) pProp->value();

		        if(nObjetivo>= pArray->minIndex() && nObjetivo<=pArray->maxIndex()){
			        pElem = pArray->getValue(nObjetivo);
		        }
	        }
			if(((ahm::PropertyValueProperties*)pElem)->properties()){
				pProp = ((ahm::PropertyValueProperties*)pElem)->properties()->findProperty(PROP_OBJECTIVE);
				pIndexStruct = ((ahm::PropertyValueProperties*)pProp)->properties();
			}

		}
        //getNameProperty
		std::string strname, straper, strbuf;
	    if(pIndexStruct)
        {
            //prop_tools::getStringValue MAGNIFICATION
		    ahm::Property *pPropertyMagnification = pProperties->findProperty(PROP_MAGNIFICATION);

		    if(pPropertyMagnification)
            {
		        if (pPropertyMagnification->value()->derivedType() == ahm::PropertyValue::TYPE_STRING)
		        {
			        ahm::PropertyValueString* pStringValue =  (ahm::PropertyValueString*)pPropertyMagnification->value();
			        ahm::StringResult * pStrResult = pStringValue->getValue();
			        if(pStrResult){
				        strname = pStrResult->value();
				        pStrResult->dispose();
			        }
		        }
            }

            //prop_tools::getStringValue PROP_APERTURE
		    ahm::Property *pPropertyAperture = pProperties->findProperty(PROP_APERTURE);

		    if(pPropertyAperture)
            {
		        if (pPropertyAperture->value()->derivedType() == ahm::PropertyValue::TYPE_STRING)
		        {
			        ahm::PropertyValueString* pStringValue =  (ahm::PropertyValueString*)pPropertyAperture->value();
			        ahm::StringResult * pStrResult = pStringValue->getValue();
			        if(pStrResult){
				        strname = pStrResult->value();
				        pStrResult->dispose();
			        }
		        }
            }

			strname.append("x ");
			strname.append(straper);
            return strname.c_str();

	    }


    } // else pUnit

	return "";
}





