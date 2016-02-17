/** 
 ** Copyright (c) 2005 Leica Microsystems - All Rights Reserved
 ** 
 ** Tool functions for AHM properties
 **
 **/
#include "stdafx.h"
#include "proptool.h"


namespace prop_tools {



#define throwException(cls,code,sztext)

std::string  toString(iop::int32 ival){
	char szbuf[128];
	itoa(ival, szbuf, 10);
	return std::string(szbuf);
}

std::string  toString(iop::float64 fval){
	char szbuf[128];

	sprintf(szbuf, "%lf", fval);
	
	return std::string(szbuf);
}


std::string toHexString(iop::int32 ival){
	char szbuf[128];
	itoa(ival, szbuf, 16);
	std::string str("#");
	str.append(szbuf);
	return str;
}



iop::int32 toInt(iop::string szval, iop::int32 dfltval/*=0*/){

	iop::int32 result = dfltval;
	if(szval){

		if(*szval=='#'){
			szval++;
			result=dfltval;
			{
				iop::int32 ival = 0;
				while(*szval){
					char ch = toupper(*szval);
					iop::int32 nibble = 0;
					if(ch>='0' && ch<='9'){
						nibble = ch -'0';
					}
					else if(ch>='A' && ch<='F'){
						nibble = ch -'A' + 10;
					}
					else return dfltval;


					ival = (ival<<4 ) | ( nibble&0xf);
					szval++;

				}
				result = ival;//&0x7fffffff;

			}

		}
		else if(*szval){
			result = atoi(szval);
		}
	}
	return result;
}




bool getStringValue(ahm::PropertyValue *pValue, std::string &strresult){
	if(pValue){

		switch(pValue->derivedType()){
			case ahm::PropertyValue::TYPE_STRING: 
				{
					ahm::PropertyValueString* pStringValue =  (ahm::PropertyValueString*)pValue;
					ahm::StringResult * pStrResult = pStringValue->getValue();
					if(pStrResult){
						strresult = pStrResult->value();
						pStrResult->dispose();
					}
					return true;
				} break;
			case ahm::PropertyValue::TYPE_INT: 
				{
					ahm::PropertyValueInt* pIntValue =  (ahm::PropertyValueInt*)pValue;
					strresult = toString(pIntValue->getValue());
					return true;
				} break;
			case ahm::PropertyValue::TYPE_FLOAT: 
				{
					ahm::PropertyValueFloat* pFloatValue =  (ahm::PropertyValueFloat*)pValue;
					strresult = toString(pFloatValue->getValue());
					return true;
				} break;
			case ahm::PropertyValue::TYPE_BOOL: 
				{
					ahm::PropertyValueBool* pBoolValue =  (ahm::PropertyValueBool*)pValue;
					strresult = pBoolValue->getValue()? "1" : "0";
					return true;
					
				} break;
			case ahm::PropertyValue::TYPE_UNICODE_STRING: break;				// PropertyValueUnicodeString
			case ahm::PropertyValue::TYPE_INDEX: 
				{
					ahm::PropertyValueIndex* pIndexValue =  (ahm::PropertyValueIndex*)pValue;
					strresult = toString(pIndexValue->getIndex());
					return true;
				} break;
		}

	}
	return false;
}


bool setStringValue(ahm::PropertyValue *pValue, const std::string& strvalue){
	if(pValue){
		switch(pValue->derivedType()){
			case ahm::PropertyValue::TYPE_STRING: 
				{
					ahm::PropertyValueString* pStringValue =  (ahm::PropertyValueString*)pValue;
					pStringValue->setValue(strvalue.c_str());
					return true;
				} break;
			case ahm::PropertyValue::TYPE_INT: 
				{
					ahm::PropertyValueInt* pIntValue =  (ahm::PropertyValueInt*)pValue;
					pIntValue->setValue(atoi(strvalue.c_str()));
					return true;
				} break;
			case ahm::PropertyValue::TYPE_FLOAT: 
				{
					ahm::PropertyValueFloat* pFloatValue =  (ahm::PropertyValueFloat*)pValue;

					pFloatValue->setValue(atof(strvalue.c_str()));
					return true;
				} break;
			case ahm::PropertyValue::TYPE_BOOL: 
				{
					ahm::PropertyValueBool* pBoolValue =  (ahm::PropertyValueBool*)pValue;
					pBoolValue->setValue(atoi(strvalue.c_str())!=0);
					return true;
					
				} break;
			case ahm::PropertyValue::TYPE_UNICODE_STRING: break;				// PropertyValueUnicodeString
			case ahm::PropertyValue::TYPE_INDEX: 
				{
					ahm::PropertyValueIndex* pIndexValue =  (ahm::PropertyValueIndex*)pValue;
					pIndexValue->setIndex(atoi(strvalue.c_str()));
					return true;
				} break;
		}
	}
	return false;
}

bool isArray(ahm::PropertyValue* pValue){
	if(pValue){
		return pValue->derivedType() == ahm::PropertyValue::TYPE_FIXED_ARRAY;
	}
	return false;
}

bool isProperties(ahm::PropertyValue* pValue){
	if(pValue){
		return pValue->derivedType() == ahm::PropertyValue::TYPE_PROPERTIES;
	}
	return false;
}

bool isScalar(ahm::PropertyValue* pValue){
	if(pValue){
		return pValue->derivedType() >= ahm::PropertyValue::__TYPE_SCALAR_START
			&& pValue->derivedType() < ahm::PropertyValue::__TYPE_SCALAR_MAX;
	}
	return false;
}


bool isRect(ahm::PropertyValue* pValue){
	if(pValue){
		return pValue->derivedType() == ahm::PropertyValue::TYPE_RECT
			|| pValue->derivedType() == ahm::PropertyValue::TYPE_FLOAT_RECT;
	}
	return false;
}



bool isArray(ahm::Property* pProperty){
	return pProperty ? isArray(pProperty->value()) : false;
}
bool isProperties(ahm::Property* pProperty){
	return pProperty ? isProperties(pProperty->value()) : false;
}
bool isScalar(ahm::Property* pProperty){
	return pProperty ? isScalar(pProperty->value()) : false;
}
bool isRect(ahm::Property* pProperty){
	return pProperty ? isRect(pProperty->value()) : false;
}




ahm::PropertyValue * getIndexValue(ahm::PropertyValue *pValue, iop::int32 index){

	if(isArray(pValue)){
		ahm::PropertyValueArray* pArray = (ahm::PropertyValueArray*) pValue;

		if(index>= pArray->minIndex() && index<=pArray->maxIndex()){
			return pArray->getValue(index);
		}
	}
	return 0;
}
ahm::PropertyValue * getIndexValue(ahm::Property* pProperty, iop::int32 index){
	return pProperty ? getIndexValue(pProperty->value(), index) : 0;
}

ahm::Properties * properties(ahm::PropertyValue* pValue){

	if(isProperties(pValue)){
		return ((ahm::PropertyValueProperties*)pValue)->properties();
	}
	return 0;
}


ahm::Properties * properties(ahm::Property* pProperty){
	if(pProperty){
		return properties(pProperty->value());
	}
	return 0;

}


bool getStringValue(ahm::Properties* pProperties, iop::int32 id, std::string &strresult){
	if(pProperties){
		ahm::Property *pProperty = pProperties->findProperty(id);

		if(pProperty) return getStringValue(pProperty->value(), strresult);

	}
	return false;
}

bool getStringValue(ahm::PropertyValue* pValue, iop::int32 id, std::string &strresult){
	if(isProperties(pValue)){
		return getStringValue(((ahm::PropertyValueProperties*)pValue)->properties(), id, strresult);
	}
	return false;
}



bool getIntValue(ahm::PropertyValue *pValue, iop::int32 &iresult){
	if(pValue){
		if(pValue->derivedType() == ahm::PropertyValue::TYPE_INT){
			iresult = ((ahm::PropertyValueInt*)pValue)->getValue();
			return true;
		}
		else if(pValue->derivedType() == ahm::PropertyValue::TYPE_BOOL){
			iresult = ((ahm::PropertyValueBool*)pValue)->getValue()? 1:0;
			return true;
		}


	}
	return false;
}
bool getIntValue(ahm::Properties* pProperties, iop::int32 id, iop::int32 &iresult){
	if(pProperties){
		ahm::Property *pProperty = pProperties->findProperty(id);

		if(pProperty) return getIntValue(pProperty->value(), iresult);

	}
	return false;


}
bool getIntValue(ahm::PropertyValue* pValue, iop::int32 id, iop::int32 &iresult){
	if(isProperties(pValue)){
		return getIntValue(((ahm::PropertyValueProperties*)pValue)->properties(), id, iresult);
	}
	return false;
}





void copy_scalar(ahm::PropertyValue *pTarget, ahm::PropertyValue *pSource){

	if(pSource && pTarget){

		if(pSource->derivedType() == pTarget->derivedType()){

			switch(pSource->derivedType()){
				case ahm::PropertyValue::TYPE_STRING: 
				{
					ahm::PropertyValueString* pStringValueSource =  (ahm::PropertyValueString*) pSource;
					ahm::PropertyValueString* pStringValueTarget =  (ahm::PropertyValueString*) pTarget;
					ahm::StringResult * pStrResult = pStringValueSource->getValue();
					if(pStrResult){
						pStringValueTarget->setValue(pStrResult->value());
						pStrResult->dispose();
					}
				} break;
			case ahm::PropertyValue::TYPE_INT: 
				{
					ahm::PropertyValueInt* pIntValueSource =  (ahm::PropertyValueInt*)pSource;
					ahm::PropertyValueInt* pIntValueTarget =  (ahm::PropertyValueInt*)pTarget;
					pIntValueTarget->setValue(pIntValueSource->getValue());
				} break;
			case ahm::PropertyValue::TYPE_FLOAT: 
				{
					ahm::PropertyValueFloat* pFloatValueSource =  (ahm::PropertyValueFloat*)pSource;
					ahm::PropertyValueFloat* pFloatValueTarget =  (ahm::PropertyValueFloat*)pTarget;
					pFloatValueTarget->setValue(pFloatValueSource->getValue());
				} break;
			case ahm::PropertyValue::TYPE_BOOL: 
				{
					ahm::PropertyValueBool* pBoolValueSource =  (ahm::PropertyValueBool*)pSource;
					ahm::PropertyValueBool* pBoolValueTarget =  (ahm::PropertyValueBool*)pTarget;
					pBoolValueTarget->setValue(pBoolValueSource->getValue());
				} break;
			case ahm::PropertyValue::TYPE_UNICODE_STRING: break;				// PropertyValueUnicodeString
			case ahm::PropertyValue::TYPE_INDEX: 
				{
					ahm::PropertyValueIndex* pIndexValueSource =  (ahm::PropertyValueIndex*)pSource;
					ahm::PropertyValueIndex* pIndexValueTarget =  (ahm::PropertyValueIndex*)pTarget;
					pIndexValueTarget->setIndex(pIndexValueSource->getIndex());
				} break;

			}
		}

		else {
			throwException(ahm::ERROR_CLASS_GENERAL_ERROR, ahm::ERROR_CODE_PARAMETER_INVALID, 
				"copy_scalar mismatching property types");


		}
	}
}


bool getFloat(iop::float64& fltTarget, ahm::PropertyValue *pSource){
	if(isScalar(pSource)){
		switch(pSource->derivedType()){
		case ahm::PropertyValue::TYPE_INT: 
			{
				ahm::PropertyValueInt* pIntValueSource =  (ahm::PropertyValueInt*)pSource;

				fltTarget = pIntValueSource->getValue();
				return true;


			} 
		case ahm::PropertyValue::TYPE_FLOAT: 
			{
				ahm::PropertyValueFloat* pFloatValueSource =  (ahm::PropertyValueFloat*)pSource;
				fltTarget = pFloatValueSource->getValue();
				return true;
			}

		}

	}
	return false;

}


bool setFloat(ahm::PropertyValue *pTarget, iop::float64 fltval){
	if(isScalar(pTarget)){
		switch(pTarget->derivedType()){
		case ahm::PropertyValue::TYPE_INT: 
			{
				ahm::PropertyValueInt* pIntValueTarget =  (ahm::PropertyValueInt*)pTarget;

				pIntValueTarget->setValue((iop::int32) fltval);
				return true;


			} 
		case ahm::PropertyValue::TYPE_FLOAT: 
			{
				ahm::PropertyValueFloat* pFloatValueTarget =  (ahm::PropertyValueFloat*)pTarget;
				pFloatValueTarget->setValue(fltval);
				return true;
			}

		}

	}
	return false;

}






void setRectValues(ahm::PropertyValue *pTarget, ahm::PropertyValue* pSourceLeft, ahm::PropertyValue* pSourceTop, ahm::PropertyValue* pSourceRight, ahm::PropertyValue* pSourceBottom){

	if(pTarget){
		if(pTarget->derivedType() == ahm::PropertyValue::TYPE_RECT){
			ahm::PropertyValueRect *pTargetRect  = (ahm::PropertyValueRect *) pTarget;
			copy_scalar(pTargetRect->left(), pSourceLeft); 
			copy_scalar(pTargetRect->top(), pSourceTop); 
			copy_scalar(pTargetRect->right(), pSourceRight); 
			copy_scalar(pTargetRect->bottom(), pSourceBottom); 
		}
		else  if(pTarget->derivedType() == ahm::PropertyValue::TYPE_FLOAT_RECT){
			ahm::PropertyValueFloatRect *pTargetRect  = (ahm::PropertyValueFloatRect *) pTarget;
			ahm::FLOAT_RECT frect = {0,0,0,0 };
			getFloat(frect.left, pSourceLeft); 
			getFloat(frect.top, pSourceTop); 
			getFloat(frect.right, pSourceRight); 
			getFloat(frect.bottom, pSourceBottom); 

			// change
			pTargetRect->setValue(frect);
		}
	}
}




void copy_rect(ahm::PropertyValue *pTarget, ahm::PropertyValue *pSource){
	if(isRect(pTarget) && isRect(pSource)){
		// same types!
		if( pTarget->derivedType() == ahm::PropertyValue::TYPE_RECT
				&& pSource->derivedType() == ahm::PropertyValue::TYPE_RECT)
		{
			ahm::PropertyValueRect* pSourceRect =  (ahm::PropertyValueRect* ) pSource;
			ahm::PropertyValueRect* pTargetRect =  (ahm::PropertyValueRect* ) pTarget;
			copy_scalar(pTargetRect->left(), pSourceRect->left());
			copy_scalar(pTargetRect->top(), pSourceRect->top());
			copy_scalar(pTargetRect->right(), pSourceRect->right());
			copy_scalar(pTargetRect->bottom(), pSourceRect->bottom());
		}
		else if( pTarget->derivedType() == ahm::PropertyValue::TYPE_FLOAT_RECT
			&& pSource->derivedType() == ahm::PropertyValue::TYPE_FLOAT_RECT)
		{
			ahm::PropertyValueFloatRect* pSourceFloatRect =  (ahm::PropertyValueFloatRect* ) pSource;
			ahm::PropertyValueFloatRect* pTargetFloatRect =  (ahm::PropertyValueFloatRect* ) pTarget;
			ahm::FLOAT_RECT frect = {  0,0,0,0};
			pSourceFloatRect->getValue(frect);
			pTargetFloatRect->setValue(frect);
		}
		// different types
		else if( pTarget->derivedType() == ahm::PropertyValue::TYPE_RECT
			&& pSource->derivedType() == ahm::PropertyValue::TYPE_FLOAT_RECT)
		{
			ahm::PropertyValueFloatRect* pSourceFloatRect =  (ahm::PropertyValueFloatRect* ) pSource;
			ahm::PropertyValueRect* pTargetRect =  (ahm::PropertyValueRect* ) pTarget;
			ahm::FLOAT_RECT frect = {  0,0,0,0 };
			
			setFloat(pTargetRect->left(), frect.left);
			setFloat(pTargetRect->top(), frect.top);
			setFloat(pTargetRect->right(), frect.right);
			setFloat(pTargetRect->bottom(), frect.bottom);

		}

		else if( pTarget->derivedType() == ahm::PropertyValue::TYPE_FLOAT_RECT
			&& pSource->derivedType() == ahm::PropertyValue::TYPE_RECT)
		{
			ahm::PropertyValueRect* pSourceRect =  (ahm::PropertyValueRect* ) pSource;
			ahm::PropertyValueFloatRect* pTargetFloatRect =  (ahm::PropertyValueFloatRect* ) pTarget;
			ahm::FLOAT_RECT frect = {  0,0,0,0 };
			getFloat(frect.left, pSourceRect->left());
			getFloat(frect.top, pSourceRect->top());
			getFloat(frect.right, pSourceRect->right());
			getFloat(frect.bottom, pSourceRect->bottom());

			pTargetFloatRect->setValue(frect);

		}


	}
}




}// end namespace