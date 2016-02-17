#include "stdafx.h"

#if !defined (__BORLANDC__)
#include <stdio.h>
#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "Wsc.h"
#endif

#include "revolver_dta.h"
#include "..\ProcesoAux\gestion_mensajes.h"

#define MAX_INTENTOS_RESPUESTA_COM 2000000

#if !defined (__BORLANDC__)
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif
#endif

Revolver::Revolver()
{
	m_Addr = 0;
	m_currentFilter = -1;
	char	str[80];

	// Inicializamos
	m_RPF_parameters.nfw		= 1;
	m_RPF_parameters.nfilt		= MAX_FILTROS_DTA;
	// filter steps
	m_RPF_parameters.step		= 800 / m_RPF_parameters.nfilt;

	m_RPF_parameters.com		= COM5;
	m_RPF_parameters.baud		= Baud19200;
	m_RPF_parameters.hold		= 1;
	m_RPF_parameters.torque		= 15984;
	m_RPF_parameters.offset		= 0;
	m_RPF_parameters.calspd		= 20000;
	m_RPF_parameters.slope		= 255;
	m_RPF_parameters.stspd		= 65535;
	m_RPF_parameters.enspd		= 6144;
	m_RPF_parameters.feed		= 1;
	m_RPF_parameters.dly		= 125;

	//Cargamos
	LoadCFG(m_RPF_parameters);

	int er = SioReset(m_RPF_parameters.com, 64, 64);
	if (er != 0)
	{
		if (er == IE_BADID)
			error_fatal("Init Revolver", "Imposible abrir COM: Puerto inexistente", 0);
		if (er == IE_OPEN)
			error_fatal("Init Revolver", "Imposible abrir COM: Puerto ya abierto", 0);
		if (er == IE_MEMORY)
			error_fatal("Init Revolver", "Imposible abrir COM: No hay memoria suficiente", 0);
		else
			error_fatal("Init Revolver", "Imposible abrir COM", 0);
		return;
	}
	er = SioBaud(m_RPF_parameters.com, m_RPF_parameters.baud);
	if (er == IE_BAUDRATE )
	{
		error_fatal("Init Revolver", "Unsupported boud rate", 0);
		return;
	}
 	for(int n = 0; n < m_RPF_parameters.nfw; n++)
	{	
		m_Addr = n;
		// Hold
		sprintf(str, "9%d", m_RPF_parameters.hold);
		if (Cmd(str) == false)
        {
			AfxMessageBox("***  Init Revolver: No hay respuesta por el puerto COM, asegurese que el dispositivo esta conectado");
            exit(-1);
        }

		// Torque value
		sprintf(str, "3%04X", m_RPF_parameters.torque);
		Cmd(str);
		// Offset value
		sprintf(str, "4%02X", m_RPF_parameters.offset + 127);
		Cmd(str);
		// Num. of filters
		sprintf(str, "5%02X", m_RPF_parameters.nfilt);
		Cmd(str);
		// Num. of filters
		sprintf(str, "6%03X", m_RPF_parameters.step);
		Cmd(str);
		// Calibration speed
		sprintf(str, "8%04X", m_RPF_parameters.calspd);
		Cmd(str);
		// Slope
		sprintf(str, "A%02X", m_RPF_parameters.slope);
		Cmd(str);
		// Start speed
		sprintf(str, "B%04X", m_RPF_parameters.stspd);
		Cmd(str);
		// End speed
		sprintf(str, "C%04X", m_RPF_parameters.enspd);
		Cmd(str);
		// Enable feedback
		sprintf(str, "L%d", m_RPF_parameters.feed);
		Cmd(str);
		// Delay after pos.
		sprintf(str, "K%04X", m_RPF_parameters.dly);
		Cmd(str);
		// Calibrate
		Calibrate();
	}
}

// Optional change of configuration (if different from default)
Revolver::Revolver(rpf_parameters& parameters)
{
	m_Addr = 0;
	m_currentFilter = -1;
	char	str[80];

	m_RPF_parameters.nfw		= parameters.nfw	;
	m_RPF_parameters.nfilt		= parameters.nfilt	;
	// filter steps
	m_RPF_parameters.step		= parameters.step	;

	m_RPF_parameters.com		= parameters.com	;
	m_RPF_parameters.baud		= parameters.baud	;
	m_RPF_parameters.hold		= parameters.hold	;
	m_RPF_parameters.torque		= parameters.torque	;
	m_RPF_parameters.offset		= parameters.offset	;
	m_RPF_parameters.calspd		= parameters.calspd	;
	m_RPF_parameters.slope		= parameters.slope	;
	m_RPF_parameters.stspd		= parameters.stspd	;
	m_RPF_parameters.enspd		= parameters.enspd	;
	m_RPF_parameters.feed		= parameters.feed	;
	m_RPF_parameters.dly		= parameters.dly	;

	int er = SioReset(m_RPF_parameters.com, 64, 64);
	if (er != 0)
	{
		if (er == IE_BADID)
			error_fatal("Init Revolver", "Imposible abrir COM: Puerto inexistente", 0);
		if (er == IE_OPEN)
			error_fatal("Init Revolver", "Imposible abrir COM: Puerto ya abierto", 0);
		if (er == IE_MEMORY)
			error_fatal("Init Revolver", "Imposible abrir COM: No hay memoria suficiente", 0);
		else
			error_fatal("Init Revolver", "Imposible abrir COM", 0);
		return;
	}
	er = SioBaud(m_RPF_parameters.com, m_RPF_parameters.baud);
	if (er == IE_BAUDRATE )
	{
		error_fatal("Init Revolver", "Unsupported boud rate", 0);
		return;
	}
	for(int n = 0; n < m_RPF_parameters.nfw; n++)
	{	
		m_Addr = n;
		// Hold
		sprintf(str, "9%d", m_RPF_parameters.hold);
		Cmd(str);
		// Torque value
		sprintf(str, "3%04X", m_RPF_parameters.torque);
		Cmd(str);
		// Offset value
		sprintf(str, "4%02X", m_RPF_parameters.offset + 127);
		Cmd(str);
		// Num. of filters
		sprintf(str, "5%02X", m_RPF_parameters.nfilt);
		Cmd(str);
		// Num. of filters
		sprintf(str, "6%03X", m_RPF_parameters.step);
		Cmd(str);
		// Calibration speed
		sprintf(str, "8%04X", m_RPF_parameters.calspd);
		Cmd(str);
		// Slope
		sprintf(str, "A%02X", m_RPF_parameters.slope);
		Cmd(str);
		// Start speed
		sprintf(str, "B%04X", m_RPF_parameters.stspd);
		Cmd(str);
		// End speed
		sprintf(str, "C%04X", m_RPF_parameters.enspd);
		Cmd(str);
		// Enable feedback
		sprintf(str, "L%d", m_RPF_parameters.feed);
		Cmd(str);
		// Delay after pos.
		sprintf(str, "K%04X", m_RPF_parameters.dly);
		Cmd(str);
		// Calibrate
		Calibrate();
	}
}

Revolver::~Revolver()
{
	SioDone(m_RPF_parameters.com);
}

// Places the wheel in filter number 'nFilter' [0..MAX_FILTROS_DTA]
// Once the placement has been donde, true is returned
// If nFilter is out of range, there is a hardware failure or the recalibration has not taken place
// false is returned
bool Revolver::ChangeFilter(int nFilter)
{
	bool	bOk = false;
	char	buf[80];

	if (nFilter >= 0 && nFilter < m_RPF_parameters.nfilt)
	{
		sprintf(buf, "2%2X", nFilter);
		bOk = Cmd(buf);

        // HACER: quitar, esto retrasa innecesariamente
        // If torque is enabled, position again (trick to solve the sound problem)
        if(bOk && this->m_RPF_parameters.hold == 1)
		    bOk = Cmd(buf);
	}

	if (bOk)
		m_currentFilter = nFilter;

	return bOk;
}

// Returns the current filter
int Revolver::GetFilter()
{
	return m_currentFilter;
}

// Performs the adjusting by placing on filter number 0
// On success, true is returned
bool Revolver::Calibrate()
{
	bool	er = false;

	er = Cmd("1");
	
	if (er == true)
		m_currentFilter = 0;
		
	return er;

}

// In serial mode, RPF is identified by an address, so that up to 8 devices can be linked
// Each device connected must have an input address (even if only one is connected)
// This functions sets the address, 0 by default
bool Revolver::SetIdentity(int nId)
{
	char	buf[80];

	sprintf(buf, "D3F%04X", nId);
	return Cmd(buf);
}

// Sends the command string 'str'
// Returns 'true' if the command was successful (ACK00 was received back)
bool Revolver::Cmd(char *str)
{
	int		n;
	char	check[256];

	TxStr(str, strlen(str));
	//delay(10);
	if (RxStr(check, &n) == 1)
        return false;

	if(strcmp(check, "ACK00") != 0)
		return false;
	return true;
}

unsigned char Revolver::ByteHex(unsigned char val)
{
	unsigned char ch = val;
	if(ch <= 9) ch += '0';
	else ch += '0' + 7;
	return ch;
}

unsigned char Revolver::HexBin(char *str)
{
	unsigned char	ch;
	if(str[0] <= '9')
		ch = (str[0] - '0') << 4;
	else
		ch = (str[0] - '0' - 7) << 4;
	if(str[1] <= '9')
		ch += str[1] - '0';
	else
		ch += str[1] - '0' - 7;
	return ch;
}

void Revolver::TxByte(unsigned char v)
{
	SioPutc(m_RPF_parameters.com, v);
}

void Revolver::TxStr(char *tx, int len)
{
	unsigned char cksm = 0, ch;
	int		c;
	TxByte('$');

	ch = ByteHex((m_Addr >> 4) & 0x0F);
	TxByte(ch);
	cksm += ch;
	ch = ByteHex(m_Addr & 0x0F);
	TxByte(ch);
	cksm += ch;
	for(c = 0; c < len; c++)
	{
		TxByte(tx[c]);
		cksm += tx[c];
	}
	TxByte('#');

	ch = ByteHex((cksm >> 4) & 0x0F);
	TxByte(ch);
	ch = ByteHex(cksm & 0x0F);
	TxByte(ch);
	TxByte(13);
}

// Devuelve 0 si todo bien, 1 si error
int Revolver::RxStr(char *rx, int *len)
{
	char	bufrx[256];
	unsigned char cksm = 0, rcksm, add;
	int		c;
	int		nIntentos=0; //Para dar error en caso de que no haya respuesta por el COM

	bufrx[0] = bufrx[1] = 0;

	do
    {
		c = SioRxQue(m_RPF_parameters.com);
        ++nIntentos;
    }
	while(c < 12 && nIntentos<MAX_INTENTOS_RESPUESTA_COM);
    if (nIntentos>=MAX_INTENTOS_RESPUESTA_COM)
        return 1;

	SioGets(m_RPF_parameters.com, bufrx, 80);
	if(bufrx[0] != '$')
		return 1;
	add = HexBin(&bufrx[1]);
	if(add != m_Addr)
	return 2;
	cksm += bufrx[1];
	cksm += bufrx[2];
	for(c = 3; c < 80; c++)
	{
		if(bufrx[c] != '#')
		{
			cksm += bufrx[c];
			rx[c - 3] = bufrx[c];
		}
		else
		{
			c++;
			rcksm = HexBin(&bufrx[c]);
			rx[c-4]=0;
			break;
		}
	}
	if(rcksm != cksm)
		return 1;
	*len = c - 3;
	return 0;
}

void Revolver::LoadCFG(rpf_parameters& param)
{
	FILE	*fs;
	char	buf[256];

	fs = fopen("rpf.cfg", "r");
	if(fs == NULL) return;

	// Num of fw
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.nfw);

	// Num of filters
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.nfilt);

	// Serial port
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.com);

	// baud rate
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.baud);

	// enable torque
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.hold);

	// torque value
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.torque);

	// offset value
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.offset);

	// filter steps
	param.step = 800 / param.nfilt;

	// calibration speed
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.calspd);

	// slope
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.slope);

	// start speed
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.stspd);

	// end speed
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.enspd);

	// enable feedback
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.feed);

	// delay after pos.
	fgets(buf, 256, fs); // Comment
	fgets(buf, 256, fs);
	sscanf(buf, "%d", &param.dly);

	fclose(fs);
}

/*
#include "mgui.h"
#include "rpfdef.h"

rpfpar	RPF;

fname	FN[MAXFW];

MOBJECT	sb;

void delay(unsigned long d)
{
	DWORD	s, e;
	s = GetTickCount();
	do
		e = GetTickCount();
	while(labs(e - s) < d);
}

MOBJECT PopMessage(char *msg)
{
	MOBJECT sh, lbl;

	sh = MCreateShell("", SF_POPUP);
	MObjectSetBackgroundImageFile(sh, ".\\images\\back.gif", BI_TILED);

	lbl = MCreateLabel(sh, msg, FIXED_SMALL);
	MObjectSetShadow(lbl, WS_NONE, 0, 0);
	MObjectSetForegroundRGB(lbl, 224, 224, 176);

	sb = MCreateStatusBar(sh, "Init", 120, FIXED_SMALL);

	MShellRealize(sh);

	return sh;
}

void SaveName(void)
{
	FILE	*fd;
	fd = fopen("FILTER.CFG", "w");

	fwrite(&FN[0].filt[0][0], sizeof(FN), 1, fd);

	fclose(fd);
}

void LoadName(void)
{
	FILE	*fd;
	fd = fopen("FILTER.CFG", "r");

	fread(&FN[0].filt[0][0], sizeof(FN), 1, fd);

	fclose(fd);
}


void MDQuitCB(MOBJECT shell, void *od, void *ad)
{
	MShellUnrealize(shell);
	MShellDestroy(shell);
	MGUITerm();
	SioDone(RPF.com);
	SaveName();
	exit(0);
}void FiltNameCB(MOBJECT edit, char *buff, void *a)
{
	int		w = (int) a, ad, f;

	ad = w / 100;
	f = w - ad * 100;
	strcpy(FN[ad].filt[f], buff);
}

MOBJECT MDCreateMain(void)
{
	MOBJECT sh, tab, a, b, pb;
	int		n, f;
	char	buf[80];

	sh = MCreateShell("RPF Max HANDLER Rev. 1.3", SF_NO_RESIZE|SF_NO_MAXIMIZE|SF_NO_MINIMIZE);
	MObjectSetBackgroundImageFile(sh, "images\\back.gif", BI_TILED);

	tab = MCreateTabbedRowForm(sh, RPF.nfw, FIXED_SMALL);
	MObjectSetTransparency(tab, 255);
	MObjectSetResize(tab, True, False);

	LoadName();

	for(n = 0; n < RPF.nfw; n++)
	{
		MPagedFormSetActivePage(tab, n);

		sprintf(buf, "FW%d", n);
		MObjectSetText(tab, buf);
		a = MCreateRowForm(tab);
		MFormSetSpacing(a, 0);
		MObjectSetTransparency(a, 255);
		MObjectSetShadow(a, WS_NONE, 0, 0);

		for(f = 0; f < RPF.nfilt; f++)
		{
			b = MCreateColForm(a);
			MObjectSetTransparency(b, 255);
			MObjectSetShadow(b, WS_NONE, 0, 0);
			MObjectSetResize(b, True, True);

			pb = MCreateImagePButtonExt(b, "images\\flt.bmp", "images\\flton.bmp", "", "", "", "");
			MButtonSetImageMaskByPixel(pb, 0, 0);
			MObjectSetTransparency(pb, 255);
			MPButtonSetCallback(pb, CamCB, (void *) (100 * n + f));

			if(f == 0)
				RPF.pix[n][f] = MCreatePixmapFromFile(b, "images\\ledon.bmp");
			else
				RPF.pix[n][f] = MCreatePixmapFromFile(b, "images\\ledoff.bmp");

			//sprintf(buf, "Filter %d", f);
			RPF.fed[n][f] = MCreateEdit(b, FN[n].filt[f], 20, FIXED_SMALL);
			MEditSetActCallback(RPF.fed[n][f], FiltNameCB, (void *) (100 * n + f));
			MObjectSetForegroundRGB(RPF.fed[n][f], 224, 224, 176);
			MObjectSetBackgroundRGB(RPF.fed[n][f], 116, 114, 158);
			MObjectSetPopupHelp(RPF.fed[n][f], "To modify this field\nplease write the name\nand then press ENTER", 500, 500)			;
		}
		RPF.prv[n] = 0;

		pb = MCreateImagePButtonExt(tab, "images\\cal.bmp", "images\\calon.bmp", "", "images\\calarm.bmp", "", "");
		MObjectSetShadow(pb, WS_NONE, 0, 0);
		MButtonSetImageMaskByPixel(pb, 0, 0);
		MObjectSetTransparency(pb, 255);
		MPButtonSetCallback(pb, CalibCB, (void *) (100 * n));
	}
	MShellRealize(sh);
	MShellSetWMCloseCallback(sh, MDQuitCB, 0L);

	return sh;
}
void MGUIMain(int argc, char **argv)
{
	int		er, n, DIAGNOSE = 0, VIEW = 0;
	char	str[80];
	MOBJECT	sh;
	
	RPF.nfw = MAXFW;
	RPF.nfilt = MAX_FILTROS_DTA;

	LoadCFG();

	SioReset(RPF.com, 64, 64);
	SioBaud(RPF.com, RPF.baud);

	// Set the RPF identity
	if(argc > 2 && strcmp(argv[1], "/s") == 0)
	{
		sprintf(str, "D3F%04X", atoi(argv[2]));
		Cmd(str);
	}

	// Execute a diagnose
	if(argc > 1 && strcmp(argv[1], "/d") == 0)
		DIAGNOSE = 1;

	if(argc > 1 && strcmp(argv[1], "/v") == 0)
		VIEW = 1;

	//SioParms(RPF.com, NoParity, OneStopBit, WordLength8);

	sh = PopMessage("RPF Max\nControl Program\nRev.1.3\nX Me/2000/XP\n(C) 2007 DTA\nAll Rights Reserved\n\nPlease wait !\nI'm calbrating filter wheel(s)");

	// Calibrate all connected FW
	if(VIEW == 0)
		for(n = 0; n < RPF.nfw; n++)
		{	
			Addr = n;
			// Hold
			MStatusBarSetPos(sb, 10);
			sprintf(str, "9%d", RPF.hold);
			er = Cmd(str);
			// Torque value
			MStatusBarSetPos(sb, 20);
			sprintf(str, "3%04X", RPF.torque);
			er = Cmd(str);
			// Offset value
			MStatusBarSetPos(sb, 30);
			sprintf(str, "4%02X", RPF.offset + 127);
			er = Cmd(str);
			// Num. of filters
			MStatusBarSetPos(sb, 40);
			sprintf(str, "5%02X", RPF.nfilt);
			er = Cmd(str);
			// Num. of filters
			MStatusBarSetPos(sb, 50);
			sprintf(str, "6%03X", RPF.step);
			er = Cmd(str);
			// Calibration speed
			MStatusBarSetPos(sb, 60);
			sprintf(str, "8%04X", RPF.calspd);
			er = Cmd(str);
			// Slope
			MStatusBarSetPos(sb, 70);
			sprintf(str, "A%02X", RPF.slope);
			er = Cmd(str);
			// Start speed
			MStatusBarSetPos(sb, 80);
			sprintf(str, "B%04X", RPF.stspd);
			er = Cmd(str);
			// End speed
			MStatusBarSetPos(sb, 90);
			sprintf(str, "C%04X", RPF.enspd);
			er = Cmd(str);
			// Enable feedback
			MStatusBarSetPos(sb, 100);
			sprintf(str, "L%d", RPF.feed);
			er = Cmd(str);
			// Delay after pos.
			MStatusBarSetPos(sb, 110);
			sprintf(str, "K%04X", RPF.dly);
			er = Cmd(str);
			// Calibrate
			MStatusBarSetPos(sb, 120);
			er = Cmd("1");
		}

	MShellUnrealize(sh);
	MShellDestroy(sh);

	if(DIAGNOSE)
	{
		int		f, pos[6][8] = {{ 0, 4, 0, 4, 0, 4, 0, 4 }, { 0, 1, 2, 3, 4, 5, 6, 7 },
								{ 4, 0, 4, 0, 4, 0, 4, 0 }, { 7, 6, 5, 4, 3, 2, 1, 0 },
								{ 1, 5, 1, 5, 1, 5, 1, 5 }, { 3, 5, 4, 7, 1, 3, 2, 6 }};
		int		   p16[6][16]= {{ 0, 4, 0, 4, 0, 4, 0, 4, 8,12, 8,12, 8,12, 8,12 }, 
								{ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,10,11,12,13,14,15 },
								{ 4, 0, 4, 0, 4, 0, 4, 0, 8,12, 8,12, 8,12, 8,12 }, 
								{ 15,14,13,12,11,10,9, 8, 7, 6, 5, 4, 3, 2, 1, 0 },
								{ 1, 5, 1, 5, 1, 5, 1, 5, 9,13, 9,13, 9,13, 9,13 }, 
								{ 3, 5, 4, 7, 1, 3, 2, 6,11,13,12,15, 9,11,10,14 }};

		int		loop = 0;
		char	buf[80];
		sh = MDCreateMain();
		while(1)
		for(n = 0; n < RPF.nfw; n++)
		{
			Addr = n;
			loop++;
			for(f = 0; f < RPF.nfilt; f++)
			{
				if(RPF.nfilt > 8)
					CamCB(0, 0, (void *) (100 * n + p16[loop % 6][f]));
				else
					CamCB(0, 0, (void *) (100 * n + pos[loop % 6][f]));
				sprintf(buf, "Loop = %d, RPF = %d, Filter = %d", loop, n, f);
				MShellSetTitle(sh, buf);
				MLoopWhileEvents(0);
				delay(100);
			}
		}
	}

	MDCreateMain();

	MMainLoop();
}
*/