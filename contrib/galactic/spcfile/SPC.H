/**************************************************************************
*   FILENAME:	spc.h
*   AUTHOR:	Steven Simonoff 11-25-90 (from spc.inc)
*   MRU:	5-6-97 (New X,Y types, fexper types, float ssftime, 4D data)
*
*	Contains the Spectrum Information Header structure definitions.
*	Based on zspchdr.inc from Lab Calc (tm).
*	Must #include <windows.h> before this file.
*
*   Copyright (C) 1986-1997 by Galactic Industries Corp.
*   All Rights Reserved.
***************************************************************************

* The following defines a trace header and file format.
* All floating values in SPC files are in the IEEE standard formats.

* There are two basic formats, new and old.  FVERSN flags the format and
* also serves as a check byte.	The new format has header values in the file
* exactly as they appear below.  The old format has slightly different file
* header formating which is translated as it is read into memory. New features
* like XY data and audit log information are not supported for the old format.

* The new format allows X,Y pairs data to be stored when the TXVALS flag is set.
* The main header is immediately followed by any array of fnpts 32-bit floating
* numbers giving the X values for all points in the file or subfiles.  Note
* that for multi files, there is normally a single X values array which applies
* to all subfiles.  The X values are followed by a subfile header and fixed
* point Y values array or, for multi files, by the subfiles which each consist
* of a subfile header followed by a fixed-point Y values array.  Note that
* the software may be somewhat slower when using X-values type files.

* Another X,Y mode allows for separate X arrays and differing numbers of
* points for each subfile.  This mode is normally used for Mass Spec Data.
* If the TXYXYS flag is set along with TXVALS, then each subfile has a
* separate X array which follows the subfile header and preceeds the Y array.
* An additional subnpts subfile header entry gives the number of X,Y values
* for the subfile (rather than the fnpts entry in the main header).  Under
* this mode, there may be a directory subfile pointers whose offset is
* stored in the fnpts main header entry.  This directory consists of an
* array of ssfstc structures, one for each of the subfiles.  Each ssfstc
* gives the byte file offset of the begining of the subfile (that is, of
* its subfile header) and also gives the Z value (subtime) for the subfile
* and is byte size.  This directory is normally saved at the end of the
* file after the last subfile.	If the fnpts entry is zero, then no directory
* is present and GRAMS/32 automatically creates one (by scanning through the
* subfiles) when the file is opened.  Otherwise, fnpts should be the byte
* offset into the file to the first ssfstc for the first subfile.  Note
* that when the directory is present, the subfiles may not be sequentially
* stored in the file.  This allows GRAMS/32 to add points to subfiles by
* moving them to the end of the file.

* Y values are represented as fixed-point signed fractions (which are similar
* to integers except that the binary point is above the most significant bit
* rather than below the least significant) scaled by a single exponent value.
* For example, 0x40000000 represents 0.25 and 0xC0000000 represents -0.25 and
* if the exponent is 2 then they represent 1 and -1 respectively.  Note that
* in the old 0x4D format, the two words in a 4-byte DP Y value are reversed.
* To convert the fixed Y values to floating point:
*	FloatY = (2^Exponent)*FractionY
* or:	FloatY = (2^Exponent)*IntegerY/(2^32)		  -if 32-bit values
* or:	FloatY = (2^Exponent)*IntegerY/(2^16)		  -if 16-bit values

* Optionally, the Y values on the disk may be 32-bit IEEE floating numbers.
* In this case the fexp value (or subexp value for multifile subfiles)
* must be set to 0x80 (-128 decimal).  Floating Y values are automatically
* converted to the fixed format when read into memory and are somewhat slower.
* GRAMS/32 never saves traces with floating Y values but can read them.

* Thus an SPC trace file normally has these components in the following order:
*	SPCHDR		Main header (512 bytes in new format, 224 or 256 in old)
*      [X Values]	Optional FNPTS 32-bit floating X values if TXVALS flag
*	SUBHDR		Subfile Header for 1st subfile (32 bytes)
*	Y Values	FNPTS 32 or 16 bit fixed Y fractions scaled by exponent
*      [SUBHDR	]	Optional Subfile Header for 2nd subfile if TMULTI flag
*      [Y Values]	Optional FNPTS Y values for 2nd subfile if TMULTI flag
*	...		Additional subfiles if TMULTI flag (up to FNSUB total)
*      [Log Info]	Optional LOGSTC and log data if flogoff is non-zero

* However, files with the TXYXYS ftflgs flag set have these components:
*	SPCHDR		Main header (512 bytes in new format)
*	SUBHDR		Subfile Header for 1st subfile (32 bytes)
*	X Values	FNPTS 32-bit floating X values
*	Y Values	FNPTS 32 or 16 bit fixed Y fractions scaled by exponent
*      [SUBHDR	]	Subfile Header for 2nd subfile
*      [X Values]	FNPTS 32-bit floating X values for 2nd subfile
*      [Y Values]	FNPTS Y values for 2nd subfile
*	...		Additional subfiles (up to FNSUB total)
*      [Directory]	Optional FNSUB SSFSTC entries pointed to by FNPTS
*      [Log Info]	Optional LOGSTC and log data if flogoff is non-zero

* Note that the fxtype, fytype, and fztype default axis labels can be
* overridden with null-terminated strings at the end of fcmnt.	If the
* TALABS bit is set in ftflgs (or Z=ZTEXTL in old format), then the labels
* come from the fcatxt offset of the header.  The X, Y, and Z labels
* must each be zero-byte terminated and must occure in the stated (X,Y,Z)
* order.  If a label is only the terminating zero byte then the fxtype,
* fytype, or fztype (or Arbitrary Z) type label is used instead.  The
* labels may not exceed 20 characters each and all three must fit in 30 bytes.

* The fpost, fprocs, flevel, fsampin, ffactor, and fmethod offsets specify
* the desired post collect processing for the data.  Zero values are used
* for unspecified values causing default settings to be used.  See GRAMSDDE.INC
* Normally fpeakpt is zero to allow the centerburst to be automatically located.

* If flogoff is non-zero, then it is the byte offset in the SPC file to a
* block of memory reserved for logging changes and comments.  The beginning
* of this block holds a logstc structure which gives the size of the
* block and the offset to the log text.  The log text must be at the block's
* end.	The log text consists of lines, each ending with a carriage return
* and line feed.  After the final line's CR and LF must come a zero character
* (which must be the first in the text).  Log text requires V1.10 or later.
* The log is normally after the last subfile (or after the TXYXYS directory).

* The fwplanes allows a series of subfiles to be interpreted as a volume of
* data with ordinate Y values along three dimensions (X,Z,W).  Volume data is
* also known as 4D data since plots can have X, Z, W, and Y axes.  When
* fwplanes is non-zero, then groups of subfiles are interpreted as planes
* along a W axis.  The fwplanes value gives the number of planes (groups of
* subfiles) and must divide evenly into the total number of subfiles (fnsub).
* If the fwinc is non-zero, then the W axis values are evenly spaced beginning
* with subwlevel for the first subfile and incremented by fwinc after each
* group of fwplanes subfiles.  If fwinc is zero, then the planes may have
* non-evenly-spaced W axis values as given by the subwlevel for the first
* subfile in the plane's group.  However, the W axis values must be ordered so
* that the plane values always increase or decrease.  Also all subfiles in the
* plane should have the same subwlevel.  Equally-spaced W planes are recommended
* and some software may not handle fwinc=0.  The fwtype gives the W axis type.
***************************************************************************/

typedef struct
   {
   BYTE   ftflgs;	/* Flag bits defined below */
   BYTE   fversn;	/* 0x4B=> new LSB 1st, 0x4C=> new MSB 1st, 0x4D=> old format */
   BYTE   fexper;	/* Instrument technique code (see below) */
   char   fexp; 	/* Fraction scaling exponent integer (80h=>float) */
   DWORD  fnpts;	/* Integer number of points (or TXYXYS directory position) */
   double ffirst;	/* Floating X coordinate of first point */
   double flast;	/* Floating X coordinate of last point */
   DWORD  fnsub;	/* Integer number of subfiles (1 if not TMULTI) */
   BYTE   fxtype;	/* Type of X axis units (see definitions below) */
   BYTE   fytype;	/* Type of Y axis units (see definitions below) */
   BYTE   fztype;	/* Type of Z axis units (see definitions below) */
   BYTE   fpost;	/* Posting disposition (see GRAMSDDE.H) */
   DWORD  fdate;	/* Date/Time LSB: min=6b,hour=5b,day=5b,month=4b,year=12b */
   char   fres[9];	/* Resolution description text (null terminated) */
   char   fsource[9];	/* Source instrument description text (null terminated) */
   WORD   fpeakpt;	/* Peak point number for interferograms (0=not known) */
   float  fspare[8];	/* Used for Array Basic storage */
   char   fcmnt[130];	/* Null terminated comment ASCII text string */
   char   fcatxt[30];	/* X,Y,Z axis label strings if ftflgs=TALABS */
   DWORD  flogoff;	/* File offset to log block or 0 (see above) */
   DWORD  fmods;	/* File Modification Flags (see below: 1=A,2=B,4=C,8=D..) */
   BYTE   fprocs;	/* Processing code (see GRAMSDDE.H) */
   BYTE   flevel;	/* Calibration level plus one (1 = not calibration data) */
   WORD   fsampin;	/* Sub-method sample injection number (1 = first or only ) */
   float  ffactor;	/* Floating data multiplier concentration factor (IEEE-32) */
   char   fmethod[48];	/* Method/program/data filename w/extensions comma list */
   float  fzinc;	/* Z subfile increment (0 = use 1st subnext-subfirst) */
   DWORD  fwplanes;	/* Number of planes for 4D with W dimension (0=normal) */
   float  fwinc;	/* W plane increment (only if fwplanes is not 0) */
   BYTE   fwtype;	/* Type of W axis units (see definitions below) */
   char   freserv[187]; /* Reserved (must be set to zero) */
   } SPCHDR;

#define SPCHSZ sizeof(SPCHDR)	/* Size of spectrum header for disk file. */

/**************************************************************************
* In the old 0x4D format, fnpts is floating point rather than a DP integer,
* ffirst and flast are 32-bit floating point rather than 64-bit, and fnsub
* fmethod, and fextra do not exist.  (Note that in the new formats, the
* fcmnt text may extend into the fcatxt and fextra areas if the TALABS flag
* is not set.  However, any text beyond the first 130 bytes may be
* ignored in future versions if fextra is used for other purposes.)
* Also, in the old format, the date and time are stored differently.
* Note that the new format header has 512 bytes while old format headers
* have 256 bytes and in memory all headers use 288 bytes.  Also, the
* new header does not include the first subfile header but the old does.
* The following constants define the offsets in the old format header:

* Finally, the old format 32-bit Y values have the two words reversed from
* the Intel least-significant-word-first order.  Within each word, the
* least significant byte comes first, but the most significant word is first.
***************************************************************************/

typedef struct
   {
   BYTE  oftflgs;
   BYTE  oversn;	/* 0x4D rather than 0x4C or 0x4B */
   short oexp;		/* Word rather than byte */
   float onpts; 	/* Floating number of points */
   float ofirst;	/* Floating X coordinate of first pnt (SP rather than DP) */
   float olast; 	/* Floating X coordinate of last point (SP rather than DP) */
   BYTE  oxtype;	/* Type of X units */
   BYTE  oytype;	/* Type of Y units */
   WORD  oyear; 	/* Year collected (0=no date/time) - MSB 4 bits are Z type */
   BYTE  omonth;	/* Month collected (1=Jan) */
   BYTE  oday;		/* Day of month (1=1st) */
   BYTE  ohour; 	/* Hour of day (13=1PM) */
   BYTE  ominute;	/* Minute of hour */
   char  ores[8];	/* Resolution text (null terminated unless 8 bytes used) */
   WORD  opeakpt;
   WORD  onscans;
   float ospare[7];
   char  ocmnt[130];
   char  ocatxt[30];
   char  osubh1[32];	/* Header for first (or main) subfile included in main header */
   } OSPCHDR;

/**************************************************************************
* This structure defines the subfile headers that preceed each trace in a
* multi-type file.  Note that for evenly-spaced files, subtime and subnext are
* optional (and ignored) for all but the first subfile.  The (subnext-subtime)
* for the first subfile determines the Z spacing for all evenly-spaced subfiles.
* For ordered and random multi files, subnext is normally set to match subtime.
* However, for all types, the subindx must be correct for all subfiles.
* This header must must always be present even if there is only one subfile.
* However, if TMULTI is not set, then the subexp is ignored in favor of fexp.
* Normally, subflgs and subnois are set to zero and are used internally.
***************************************************************************/

#define SUBCHGD 1	/* Subflgs bit if subfile changed */
#define SUBNOPT 8	/* Subflgs bit if peak table file should not be used */
#define SUBMODF 128	/* Subflgs bit if subfile modified by arithmetic */

typedef struct
   {
   BYTE  subflgs;	/* Flags as defined above */
   char  subexp;	/* Exponent for sub-file's Y values (80h=>float) */
   WORD  subindx;	/* Integer index number of trace subfile (0=first) */
   float subtime;	/* Floating time for trace (Z axis corrdinate) */
   float subnext;	/* Floating time for next trace (May be same as beg) */
   float subnois;	/* Floating peak pick noise level if high byte nonzero */
   DWORD subnpts;	/* Integer number of subfile points for TXYXYS type */
   DWORD subscan;	/* Integer number of co-added scans or 0 (for collect) */
   float subwlevel;	/* Floating W axis value (if fwplanes non-zero) */
   char  subresv[4];	/* Reserved area (must be set to zero) */
   } SUBHDR;

#define FSNOIS fsubh1+subnois+3 /* Byte which is non-zero if subnois valid */

/* This structure defines the entries in the XY subfile directory. */
/* Its size is guaranteed to be 12 bytes long. */

typedef struct
   {
   DWORD ssfposn;	/* disk file position of beginning of subfile (subhdr)*/
   DWORD ssfsize;	/* byte size of subfile (subhdr+X+Y) */
   float ssftime;	/* floating Z time of subfile (subtime) */
   } SSFSTC;

/* This structure defines the header at the beginning of a flogoff block. */
/* The logsizd should be large enough to hold the text and its ending zero. */
/* The logsizm is normally set to be a multiple of 4096 and must be */
/* greater than logsizd.  It is normally set to the next larger multiple. */
/* The logdsks section is a binary block which is not read into memory. */

typedef struct		/* log block header format */
   {
   DWORD logsizd;	/* byte size of disk block */
   DWORD logsizm;	/* byte size of memory block */
   DWORD logtxto;	/* byte offset to text */
   DWORD logbins;	/* byte size of binary area (immediately after logstc) */
   DWORD logdsks;	/* byte size of disk area (immediately after logbins) */
   char logspar[44];	/* reserved (must be zero) */
   } LOGSTC;

/* Possible settings for fxtype, fztype, fwtype. */
/* XEV and XDIODE - XMETERS are new and not supported by all software. */

#define XARB	0	/* Arbitrary */
#define XWAVEN	1	/* Wavenumber (cm-1) */
#define XUMETR	2	/* Micrometers (um) */
#define XNMETR	3	/* Nanometers (nm) */
#define XSECS	4	/* Seconds */
#define XMINUTS 5	/* Minutes */
#define XHERTZ	6	/* Hertz (Hz) */
#define XKHERTZ 7	/* Kilohertz (KHz) */
#define XMHERTZ 8	/* Megahertz (MHz) */
#define XMUNITS 9	/* Mass (M/z) */
#define XPPM	10	/* Parts per million (PPM) */
#define XDAYS	11	/* Days */
#define XYEARS	12	/* Years */
#define XRAMANS 13	/* Raman Shift (cm-1) */

#define XEV	14	/* eV */
#define ZTEXTL	15	/* XYZ text labels in fcatxt (old 0x4D version only) */
#define XDIODE	16	/* Diode Number */
#define XCHANL	17	/* Channel */
#define XDEGRS	18	/* Degrees */
#define XDEGRF	19	/* Temperature (F) */
#define XDEGRC	20	/* Temperature (C) */
#define XDEGRK	21	/* Temperature (K) */
#define XPOINT	22	/* Data Points */
#define XMSEC	23	/* Milliseconds (mSec) */
#define XUSEC	24	/* Microseconds (uSec) */
#define XNSEC	25	/* Nanoseconds (nSec) */
#define XGHERTZ 26	/* Gigahertz (GHz) */
#define XCM	27	/* Centimeters (cm) */
#define XMETERS 28	/* Meters (m) */
#define XMMETR	29	/* Millimeters (mm) */
#define XHOURS	30	/* Hours */

#define XDBLIGM 255	/* Double interferogram (no display labels) */

/* Possible settings for fytype.  (The first 127 have positive peaks.) */
/* YINTENS - YDEGRK and YEMISN are new and not supported by all software. */

#define YARB	0	/* Arbitrary Intensity */
#define YIGRAM	1	/* Interferogram */
#define YABSRB	2	/* Absorbance */
#define YKMONK	3	/* Kubelka-Monk */
#define YCOUNT	4	/* Counts */
#define YVOLTS	5	/* Volts */
#define YDEGRS	6	/* Degrees */
#define YAMPS	7	/* Milliamps */
#define YMETERS 8	/* Millimeters */
#define YMVOLTS 9	/* Millivolts */
#define YLOGDR	10	/* Log(1/R) */
#define YPERCNT 11	/* Percent */

#define YINTENS 12	/* Intensity */
#define YRELINT 13	/* Relative Intensity */
#define YENERGY 14	/* Energy */
#define YDECBL	16	/* Decibel */
#define YDEGRF	19	/* Temperature (F) */
#define YDEGRC	20	/* Temperature (C) */
#define YDEGRK	21	/* Temperature (K) */
#define YINDRF	22	/* Index of Refraction [N] */
#define YEXTCF	23	/* Extinction Coeff. [K] */
#define YREAL	24	/* Real */
#define YIMAG	25	/* Imaginary */
#define YCMPLX	26	/* Complex */

#define YTRANS	128	/* Transmission (ALL HIGHER MUST HAVE VALLEYS!) */
#define YREFLEC 129	/* Reflectance */
#define YVALLEY 130	/* Arbitrary or Single Beam with Valley Peaks */
#define YEMISN	131	/* Emission */

/* Possible bit FTFLGS flag byte settings. */
/* Note that TRANDM and TORDRD are mutually exclusive. */
/* Code depends on TXVALS being the sign bit.  TXYXYS must be 0 if TXVALS=0. */
/* In old software without the fexper code, TCGRAM specifies a chromatogram. */

#define TSPREC	1	/* Single precision (16 bit) Y data if set. */
#define TCGRAM	2	/* Enables fexper in older software (CGM if fexper=0) */
#define TMULTI	4	/* Multiple traces format (set if more than one subfile) */
#define TRANDM	8	/* If TMULTI and TRANDM=1 then arbitrary time (Z) values */
#define TORDRD	16	/* If TMULTI abd TORDRD=1 then ordered but uneven subtimes */
#define TALABS	32	/* Set if should use fcatxt axis labels, not fxtype etc.  */
#define TXYXYS	64	/* If TXVALS and multifile, then each subfile has own X's */
#define TXVALS	128	/* Floating X value array preceeds Y's  (New format only) */

/* FMODS spectral modifications flag setting conventions: */
/*  "A" (2^01) = Averaging (from multiple source traces) */
/*  "B" (2^02) = Baseline correction or offset functions */
/*  "C" (2^03) = Interferogram to spectrum Computation */
/*  "D" (2^04) = Derivative (or integrate) functions */
/*  "E" (2^06) = Resolution Enhancement functions (such as deconvolution) */
/*  "I" (2^09) = Interpolation functions */
/*  "N" (2^14) = Noise reduction smoothing */
/*  "O" (2^15) = Other functions (add, subtract, noise, etc.) */
/*  "S" (2^19) = Spectral Subtraction */
/*  "T" (2^20) = Truncation (only a portion of original X axis remains) */
/*  "W" (2^23) = When collected (date and time information) has been modified */
/*  "X" (2^24) = X units conversions or X shifting */
/*  "Y" (2^25) = Y units conversions (transmission->absorbance, etc.) */
/*  "Z" (2^26) = Zap functions (features removed or modified) */

/* Instrument Technique fexper settings */
/* In older software, the TCGRAM in ftflgs must be set if fexper is non-zero. */
/* A general chromatogram is specified by a zero fexper when TCGRAM is set. */

#define SPCGEN	0	/* General SPC (could be anything) */
#define SPCGC	1	/* Gas Chromatogram */
#define SPCCGM	2	/* General Chromatogram (same as SPCGEN with TCGRAM) */
#define SPCHPLC 3	/* HPLC Chromatogram */
#define SPCFTIR 4	/* FT-IR, FT-NIR, FT-Raman Spectrum or Igram (Can also be used for scanning IR.) */
#define SPCNIR	5	/* NIR Spectrum (Usually multi-spectral data sets for calibration.) */
#define SPCUV	7	/* UV-VIS Spectrum (Can be used for single scanning UV-VIS-NIR.) */
#define SPCXRY	8	/* X-ray Diffraction Spectrum */
#define SPCMS	9	/* Mass Spectrum  (Can be single, GC-MS, Continuum, Centroid or TOF.) */
#define SPCNMR	10	/* NMR Spectrum or FID */
#define SPCRMN	11	/* Raman Spectrum (Usually Diode Array, CCD, etc. use SPCFTIR for FT-Raman.) */
#define SPCFLR	12	/* Fluorescence Spectrum */
#define SPCATM	13	/* Atomic Spectrum */
#define SPCDAD	14	/* Chromatography Diode Array Spectra */

