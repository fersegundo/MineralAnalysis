#if !defined(AFX_FORMCOMUN_H_INCLUDED_)
#define AFX_FORMCOMUN_H_INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000


/////////////////////////////////////////////////////////////////////////////
// Clase comun (de la que heredarán) para todos los forms del dialogo
class CTomaAutoDlg;
class CFormComun : public CDialog
{
public:
	CFormComun(UINT nIDTemplate, CWnd* pParent = NULL);   // standard constructor

protected:
    CTomaAutoDlg*       m_pPadre;

public:
    virtual void MostrarPosicion(double x, double y, double z) = 0;
    virtual void MostrarFiltro(int nFiltro) = 0;
    virtual int GetAnchoResto() = 0;
    virtual int GetAltoResto() = 0;
    virtual HWND GetControlImagen() = 0;
    void SetPadre(CTomaAutoDlg* pPadre);
};

#endif
