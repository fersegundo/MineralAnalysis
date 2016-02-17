// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__4873BDAE_27F1_460E_B43D_3A616DCF5F24__INCLUDED_)
#define AFX_STDAFX_H__4873BDAE_27F1_460E_B43D_3A616DCF5F24__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

// disable warning C4786: symbol greater than 255 character,
// okay to ignore
#pragma warning(disable: 4786)

#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
#define _WIN32_WINNT 0x0400 //fue necesario para CoInitializeEx

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT

#include "myTime.h" // para TIMEBENCH
#include <math.h>
#include <algorithm> //for std::sort
//#include < iostream>
#include <fstream>

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__4873BDAE_27F1_460E_B43D_3A616DCF5F24__INCLUDED_)
