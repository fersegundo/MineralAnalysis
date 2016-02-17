/*
	dateTime
*/
#include "stdafx.h"

#include "dateTime.h"


char *date_eu()
// Return date in the format dd-mm-yy
{
  static char r[20];

  char date_USA[10];

  _strdate(date_USA);             // Format: mm/dd/yy
  r[0] = date_USA[3]; // día
  r[1] = date_USA[4];
  r[2] = '-';
  r[3] = date_USA[0]; // mes
  r[4] = date_USA[1];
  r[5] = '-';
  r[6] = date_USA[6]; // año
  r[7] = date_USA[7];
  r[8] = 0;
  return r;                       // Format: dd-mm-yy
}

