// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#if !defined(AFX_STDAFX_H__CB89FCFD_F4CF_4D90_AB9F_7EE90F3DFD46__INCLUDED_)
#define AFX_STDAFX_H__CB89FCFD_F4CF_4D90_AB9F_7EE90F3DFD46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

//#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers
//#define _WIN32_WINNT 0x0400 //fue necesario para CoInitializeEx

/* To get CS_DROPSHADOW with the MSVC headers */
#define _WIN32_WINNT 0x0501
#define WINVER 0x0501

//#include <assert.h>
//#include <stdlib.h>
//#include <stdarg.h>
//#include <stdio.h>

//#include "windef.h"
//#include "winbase.h"
//#include "winreg.h"
//#include "wingdi.h"

#include <afxwin.h>         // MFC core and standard components
#include <afxext.h>         // MFC extensions
#include <afxdtctl.h>		// MFC support for Internet Explorer 4 Common Controls
#include <math.h>
#ifndef _AFX_NO_AFXCMN_SUPPORT
#include <afxcmn.h>			// MFC support for Windows Common Controls
#endif // _AFX_NO_AFXCMN_SUPPORT
//#include "winuser.h"


//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_STDAFX_H__CB89FCFD_F4CF_4D90_AB9F_7EE90F3DFD46__INCLUDED_)
