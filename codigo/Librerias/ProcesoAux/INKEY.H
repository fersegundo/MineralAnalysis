/*

  inkey.h   Defines for inkey(), identical to IBM ROM BIOS definitions.
            The defines are IBM values + 256
*/

#define K_CR        13
#define K_ESC       27
#define K_TAB       9
#define K_BS        8

#define K_F1        (256 + 59 )
#define K_F2        (256 + 60 )
#define K_F3        (256 + 61 )
#define K_F4        (256 + 62 )
#define K_F5        (256 + 63 )
#define K_F6        (256 + 64 )
#define K_F7        (256 + 65 )
#define K_F8        (256 + 66 )
#define K_F9        (256 + 67 )
#define K_F10       (256 + 68 )
#define K_F11       (256 + 69 )
#define K_F12       (256 + 70 )

#define K_SF1       (256 + 84 )          /* shift F1 */
#define K_SF2       (256 + 85 )
#define K_SF3       (256 + 86 )
#define K_SF4       (256 + 87 )
#define K_SF5       (256 + 88 )
#define K_SF6       (256 + 89 )
#define K_SF7       (256 + 90 )
#define K_SF8       (256 + 91 )
#define K_SF9       (256 + 92 )
#define K_SF10      (256 + 93 )
#define K_SF11      (256 + 94 )
#define K_SF12      (256 + 95 )

#define K_CF1       (256 + 96 )          /* Ctrl F1*/
#define K_CF2       (256 + 97 )
#define K_CF3       (256 + 98 )
#define K_CF4       (256 + 99 )
#define K_CF5       (256 + 100)
#define K_CF6       (256 + 101)
#define K_CF7       (256 + 102)
#define K_CF8       (256 + 103)
#define K_CF9       (256 + 104)
#define K_CF10      (256 + 105)
#define K_CF11      (256 + 106)
#define K_CF12      (256 + 107)

#define K_AF1       (256 + 108)          /* Alt F1 */
#define K_AF2       (256 + 109)
#define K_AF3       (256 + 110)
#define K_AF4       (256 + 111)
#define K_AF5       (256 + 112)
#define K_AF6       (256 + 113)
#define K_AF7       (256 + 114)
#define K_AF8       (256 + 115)
#define K_AF9       (256 + 116)
#define K_AF10      (256 + 117)
#define K_AF11      (256 + 118)
#define K_AF12      (256 + 119)

#define K_A1        (256 + 120)          /* Alt 1 */
#define K_A2        (256 + 121)
#define K_A3        (256 + 122)
#define K_A4        (256 + 123)
#define K_A5        (256 + 124)
#define K_A6        (256 + 125)
#define K_A7        (256 + 126)
#define K_A8        (256 + 127)
#define K_A9        (256 + 128)
#define K_A0        (256 + 129)
#define K_AHPYN     (256 + 130)          /* Alt hyphen */
#define K_AEQAL     (256 + 131)          /* Alt = */

#define K_AQ        (256 + 16 )          /* Alt Q */
#define K_AW        (256 + 17 )
#define K_AE        (256 + 18 )
#define K_AR        (256 + 19 )
#define K_AT        (256 + 20 )
#define K_AY        (256 + 21 )
#define K_AU        (256 + 22 )
#define K_AI        (256 + 23 )
#define K_AO        (256 + 24 )
#define K_AP        (256 + 25 )

#define K_AA        (256 + 30 )
#define K_AS        (256 + 31 )
#define K_AD        (256 + 32 )
#define K_AF        (256 + 33 )
#define K_AG        (256 + 34 )
#define K_AH        (256 + 35 )
#define K_AJ        (256 + 36 )
#define K_AK        (256 + 37 )
#define K_AL        (256 + 38 )

#define K_AZ        (256 + 44 )
#define K_AX        (256 + 45 )
#define K_AC        (256 + 46 )
#define K_AV        (256 + 47 )
#define K_AB        (256 + 48 )
#define K_AN        (256 + 49 )
#define K_AM        (256 + 50 )

#define K_STAB      (256 + 15 )          /* Shift Tab */
#define K_HOME      (256 + 71 )
#define K_UP        (256 + 72 )
#define K_PGUP      (256 + 73 )
#define K_LEFT      (256 + 75 )
#define K_RIGHT     (256 + 77 )
#define K_END       (256 + 79 )
#define K_DOWN      (256 + 80 )
#define K_PGDN      (256 + 81 )
#define K_INS       (256 + 82 )
#define K_DEL       (256 + 83 )

#define K_CPSC      (256 + 134)          /* ctrl-PtSc */
#define K_CHOME     (256 + 141)
#define K_CUP       (256 + 142)
#define K_CPGUP     (256 + 143)
#define K_CLEFT     (256 + 145)
#define K_CRIGHT    (256 + 147)
#define K_CEND      (256 + 149)
#define K_CDN       (256 + 150)
#define K_CPGDN     (256 + 151)
#define K_CINS      (256 + 152)
#define K_CDEL      (256 + 153)

// Defines incluidos en Febrero 2002.

#define K_CR_UP		(256 + 141)
#define K_CR_DOWN	(256 + 145)
#define K_CR_LEFT	(256 + 115)
#define K_CR_RIGHT	(256 + 116)
#define K_ESPACIO		32

// Define incluidos en Septiembre 2003
#define K_ALT_UP	(256 + 152)
#define K_ALT_DOWN	(256 + 160)
#define K_ALT_LEFT	(256 + 155)
#define K_ALT_RIGHT	(256 + 157)

// Define incluidos en Septiembre 2007
#define K_PLUS		43
#define K_MINUS		45

