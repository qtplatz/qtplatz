/*************************************************************************
   FILENAME:	gcdllerr.h
   AUTHOR:	David Abrams    August 30, 1996
   MRU:	
   DESC:	Defines for Galactic file converter errors

   IMPORTANT!  	Changes to this header file must be reflected in the
   		assembly include file "GCDLL.INC"
 
   Copyright (C) 1996 Galactic Industries Corp.  All Rights Reserved.
**************************************************************************/

// Defines for the value returned by the GCConvert() converter DLL
// function call.

// The following two return values do not display any error messages:

#define	GCE_OK   	0		// successful return
#define	GCE_CANCEL	1		// conversion canceled by user

// The following return values display the error message pointed to by
// GCErrorMsg or the standard error string shown when GCErrorMsg is
// returned NULL

// The normal error dialog (if neither the GCE_WARN or GCE_INFO flags are set)
// has a title "File Conversion Error - <source filename>"  where the
// source filename does not include the path.  When the GCE_WARN flag is 
// set the dialog has a title "Warning" and when the GCE_INFO flag is
// set the dialog has a title "Information"

#define	GCE_UNKNOWN	2		// "Unknown converter error"
#define	GCE_DLL	 	3		// "Converter DLL error"
#define	GCE_OPEN	4		// "Error opening source file "
#define	GCE_FORMAT	5		// "Bad or unsupported format "
#define	GCE_READ	6		// "Error reading file "
#define	GCE_CREATE	7		// "Error creating destination file "
#define	GCE_WRITE	8		// "Error writing file "
#define	GCE_DISKFULL	9		// "Insufficient space writing file "
#define	GCE_MEMORY	10		// "Insufficient memory"
#define	GCE_NOEXP	11		// "File export not supported"
#define	GCE_NOUSER	12		// "User interaction required"
#define	GCE_BADOP	13		// "Operation not supported"

// Flags which can be OR'ed with above to modify error dialog behavior

#define	GCE_WARN   	0x1000		// display a warning dialog
#define	GCE_INFO   	0x2000		// display information dialog

#define GCE_SRC		0x4000		// add source file name to error
#define GCE_DEST	0x8000		// add destination file name to error

#define GCE_MAXERR	GCE_BADOP	// above this is unknown error 
#define GCE_OFST	1024		// RC file offset (used by GRAMSCNV)

