/*
	revolver_dta.h

	Interfaz de acceso al revolver "RPF MAX 16" del fabricante DTA

	Autores:	FSG
	Copyright:	© AITEMIN  2007
*/

#ifndef REVOLVER_DTA_H
#define REVOLVER_DTA_H

#ifndef MAX_FILTROS_DTA
    #define MAX_FILTROS_DTA			16
#endif
#define MAX_BANDAS			16
#define MAXFW				8

typedef struct {

	int		com;		// Serial channel
	int		baud;		// Baud rate
	int		nfw;		// Num of filter wheel
	int		nfilt;		// Num of flters
	int		hold;		// Enable holding torque
	int		torque;		// Torque value
	int		offset;		// Filter offset
	int		step;		// Steps between filters
	int		calspd;		// Calibration speed
	int		slope;		// Slope
	int		stspd;		// Start speed
	int		enspd;		// End speed
	int		feed;		// Enable feedback
	int		dly;		// Delay after positioning
}rpf_parameters;

class Revolver
{
//CLASS ATTRIBUTES
private:
	int					m_Addr;	// Current filter wheel
	int					m_currentFilter;
	rpf_parameters		m_RPF_parameters;

// CLASS METHODS
public:
	Revolver();
	Revolver(rpf_parameters& parameters);
	~Revolver();
	bool ChangeFilter(int nFilter);
	int GetFilter();
	bool Calibrate();
	bool SetIdentity(int nId);

private:
	bool Cmd(char *str);
	unsigned char ByteHex(unsigned char val);
	unsigned char HexBin(char *str);
	void TxByte(unsigned char v);
	void TxStr(char *tx, int len);
	int RxStr(char *rx, int *len);
	void LoadCFG(rpf_parameters& param);

};

#endif // REVOLVER_DTA_H

