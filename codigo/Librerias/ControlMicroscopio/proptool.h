/** 
 ** Copyright (c) 2005 Leica Microsystems - All Rights Reserved
 ** 
 ** Tool functions for AHM properties
 **
 **/

#ifndef AHM_PROPERTIES_TOOLBOX_H
#define AHM_PROPERTIES_TOOLBOX_H


#include "Leica_SDK_2.3\C++\include\ahwprop2.h"

#include <vector>
#include <string>

namespace prop_tools {



std::string toString(iop::int32 ival);
std::string toString(iop::float64 fval);
std::string toHexString(iop::int32 ival);
iop::int32 toInt(iop::string szval, iop::int32 dfltval=0);




bool setStringValue(ahm::PropertyValue *pValue, const std::string& strval);
bool getStringValue(ahm::PropertyValue *pValue, std::string &strresult);

bool getStringValue(ahm::Properties* pProperties, iop::int32 id, std::string &strresult);
bool getStringValue(ahm::PropertyValue* pValue, iop::int32 id, std::string &strresult);

bool getIntValue(ahm::PropertyValue *pValue, iop::int32 &iresult);
bool getIntValue(ahm::Properties* pProperties, iop::int32 id, iop::int32 &iresult);
bool getIntValue(ahm::PropertyValue* pValue, iop::int32 id, iop::int32 &iresult);


bool isArray(ahm::PropertyValue* pValue);
bool isProperties(ahm::PropertyValue* pValue);
bool isScalar(ahm::PropertyValue* pValue);
bool isRect(ahm::PropertyValue* pValue);


bool isArray(ahm::Property* pProperty);
bool isProperties(ahm::Property* pProperty);
bool isScalar(ahm::Property* pProperty);
bool isRect(ahm::Property* pProperty);




ahm::PropertyValue * getIndexValue(ahm::PropertyValue *pValue, iop::int32 index);
ahm::PropertyValue * getIndexValue(ahm::Property* pProperty, iop::int32 index);

ahm::Properties * properties(ahm::PropertyValue* pValue);
ahm::Properties * properties(ahm::Property* pProperty);


void copy_scalar(ahm::PropertyValue *pTarget, ahm::PropertyValue *pSource);
void copy_rect(ahm::PropertyValue *pTarget, ahm::PropertyValue *pSource);





}






#endif