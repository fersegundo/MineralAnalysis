
#if !defined(INTERFAZ_LIBTIFF_H_)
#define INTERFAZ_LIBTIFF_H_

#include ".\include\tiffio.h"
#include <stdio.h>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#define TIFFTAG_ESCALA 65000
#define TIFFTAG_BITS   65001

// Espacio que engloba las funcionalidades accesibles globalmente basadas en la libreria LIBTIFF
namespace INTERFAZ_LIBTIF 
{
    static bool bPrimeraVez = true;

    static void TagExtender(TIFF *tiff)
    {
        static const TIFFFieldInfo xtiffFieldInfo[] = {
            { TIFFTAG_ESCALA, 1, 1, TIFF_DOUBLE,  FIELD_CUSTOM, 0, 1,
                "EscalaTag" },
            { TIFFTAG_BITS, 1, 1, TIFF_SHORT,  FIELD_CUSTOM, 0, 1,
                "BitsTag" },
        };

        int nf = sizeof(xtiffFieldInfo);
        int nf0 = sizeof(xtiffFieldInfo[0]);

        double ds = sizeof(xtiffFieldInfo) / sizeof(xtiffFieldInfo[0]);
        int ns = sizeof(xtiffFieldInfo) / sizeof(xtiffFieldInfo[0]);
        TIFFMergeFieldInfo( tiff, xtiffFieldInfo,
		            sizeof(xtiffFieldInfo) / sizeof(xtiffFieldInfo[0]) );
    }

    static void Init()
    {
        TIFFSetTagExtender(TagExtender);
//        TIFFSetWarningHandler(NULL); //suppress warnings
    }

    static void EscribirTags(CString csFichero, int nBits, double dEscala)
    {
        TIFF    *tiff;
        
        if (bPrimeraVez)
        {
            Init();
            bPrimeraVez = false;
        }

        tiff = TIFFOpen(csFichero, "r+");

        TIFFSetField(tiff, TIFFTAG_BITS, 1, &nBits);
        TIFFSetField(tiff, TIFFTAG_ESCALA, 1, &dEscala);

        TIFFRewriteDirectory(tiff);

        TIFFClose(tiff);
    }

    static void LeerTags(CString csFichero, int& nBits, double& dEscala)
    {
        TIFF    *tiff;
        double  *padfDouble;
        short   *padfShort;
        unsigned short iCount;

        if (bPrimeraVez)
        {
            Init();
            bPrimeraVez = false;
        }

        tiff = TIFFOpen(csFichero, "r");

        TIFFGetField(tiff, TIFFTAG_ESCALA, &iCount, &padfDouble);
        if (iCount != 1)
        {
            return;
            TIFFClose(tiff);
        }
        dEscala = *padfDouble;

        TIFFGetField(tiff, TIFFTAG_BITS, &iCount, &padfShort);
        if (iCount != 1)
        {
            return;
            TIFFClose(tiff);
        }
        nBits = *padfShort;


        TIFFClose(tiff);
    }
};

#endif 
