// FormTomaAuto.cpp : implementation file
//

#include "stdafx.h"
#include "FormComun.h"
#include "TomaAutoDlg.h"
#include "folder_dialog.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//extern CTomaAutoApp theApp;

/////////////////////////////////////////////////////////////////////////////
// CFormTomaUnica dialog


CFormComun::CFormComun(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
	: CDialog(nIDTemplate, pParent)
{
    m_pPadre                = NULL;
}

/////////////////////////////////////////////////////////////////////////////
// CFormTomaUnica message handlers

void CFormComun::SetPadre(CTomaAutoDlg* pPadre)
{
    m_pPadre = pPadre;
}

