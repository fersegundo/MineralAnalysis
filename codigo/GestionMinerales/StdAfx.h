// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__5CB9B1B1_E91F_4469_B5EE_432688C9732F__INCLUDED_)
#define AFX_STDAFX_H__5CB9B1B1_E91F_4469_B5EE_432688C9732F__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#pragma warning(disable: 4786) //al usar stl::list, compilar con debug information da warnings debido a nombres de indentificador muy largos

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include <math.h>
#include <algorithm> //for std::sort


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__5CB9B1B1_E91F_4469_B5EE_432688C9732F__INCLUDED_)
