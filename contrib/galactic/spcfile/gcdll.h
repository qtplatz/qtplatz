/*************************************************************************
   FILENAME:	gcdll.h
   AUTHOR:	David Abrams    August 30, 1996

   MRU:		10-20-96 dea add GCI_SHWLST & GCC_SHWLST flags

   DESC:	Defines for Galactic file converters written in 'C'

		Compile with FLAT32=1 for 32-bit version
		This file should be included after Windows typedefs in
		windows.h and windef.h as it uses standard Windows data types.

   IMPORTANT!  	Changes to this header file must be reflected in the
   		assembly include file "GCDLL16.INC"

   OVERVIEW:  	Galactic DLL File Converters are standard 16-bit Windows
		DLL's renamed with a *.CN4 extension (32-bit converters
		are renamed to a *.CN5 extension).  Curently only
		16-bit converters are supported by GRAMSCNV.EXE but all
		converters should be written so that they can be compiled
		to a 32-bit DLL for future Galactic products.  16-bit 
		converters should be given an 8.3 DOS filename where the
		base filename (8 characters) describes the instrument
		manufacturer and the model (as in "NIC_OMNC"), the
		extension will always be ".CN4"  32-bit converters should
		use Win32 long filenames of up to 32 characters containing
		the same information.  The base filename is used as the
		converter name in the converter selection listbox.

		Each file converter must support the following EXPORT'ed
		calls:

		BOOL EXPORT GCGetInfo (lpGCGETINFOS);		// ordinal @2
		BOOL EXPORT GCCheckData (lpGCCHECKDATAS);	// ordinal @3
		int  EXPORT GCConvert (lpGCCONVERTS);		// ordinal @4

		All API calls take a single pointer to a structure
		(defined below) to pass information between the caller
		and the converter.  The first call (GCGetInfo) returns
		information about the converter including its version,
		extension(s) for source files, prompt stings and functions
		supported to the caller.  The GCCheckData() call allows
		the caller to see if the converter can handle a particular
		piece of data.  This is useful when the caller does not know
		the format of the data and needs to find a converter which
		understands it.  The final call imports or exports foreign
		data to or from the Galactic data format.  Conversions are
		done from disk file to disk file or the Galactic data file
		may be supplied (on export) or returned (on import) in a
		block of global memory.  A converter may also optionally
		support importing a foreign file from a memory file and/or
		exporting a foreign file to a memory file.  The converter is
		responsible for all file I/O including opening/creating the
		source/destination files and cleaning up if an error occurs.
		However, in general a file converter should not have to do
		any user I/O, all user input is normally handled by the caller.  
		The format of the Galactic data file is defined in SPC.H
		(only the new Galactic format is supported).

		Note that a caller to the converter does not have to call
		these API's in a particular order so a converter should
		not, for example, require GCGetInfo() to be called before
		GCConvert() in order for the converter to function.

		Converters can expect to run on an Intel 80386 or better
		with a math coprocessor available.

		All strings are NULL terminated byte character text strings
		unless otherwise noted.

		Note: 16-bit DLL converters written to this specification
		(version 1) do not have to support long filenames.  However
		32-bit converter DLL's must support long filenames.

   Copyright (C) 1996 Galactic Industries Corp.  All Rights Reserved.
**************************************************************************/


#ifndef FLAT32			// 16-bit version

#define EXPORT	   	__export
#define DllExport
#define SPECLEVEL	1	// level 1 of the DLLL converter specification

#else				// 32-bit version

#define EXPORT	   	
#define DllExport	__declspec( dllexport )
#define SPECLEVEL	1001	// +1000 flags 32 bit converter

#endif

#define MAXSTRING	80	// max string length for strings in GCGETINFOS
#define MAXVERTXT	16	// max string length for version text

// ***********************************************************************
// Structure passed by GCGetInfo() call to converter DLL.  The first
// item (size) is filled in by caller, all others filled in by converter.

// This function provides information to the caller about the converter
// type and version.  The return value is normally TRUE.  It should
// return FALSE it there is an error initializing the converter and it
// is not available for use (insufficent memory for example).

// The caller may not modify any of returned strings (so they may be declared
// LPCSTR in the converter).  All returned strings are currently 8-bit 
// character type (UNICODE is not supported).

//	GCInfoFlags defines

#define	GCI_EXPORTOK	 0x0001L 	// set if export supported by converter
#define	GCI_IFILESEL 	 0x0002L 	// set if converter handles import fsel
#define	GCI_MEMIMPOK	 0x0004L 	// memory source import supported
#define	GCI_MEMEXPOK	 0x0008L 	// memory dest export supported
#define	GCI_SRCDIR  	 0x0010L 	// foreign filename is a directory
#define	GCI_SHWLST  	 0x0020L 	// converter supports GCC_SHWLST flag

typedef struct 	{
		DWORD	GCInfoSSize;	// length, in bytes, of the structure
		DWORD 	GCSpecLevel;	// must return 1 (16bit) or 1001 (32bit)
		DWORD 	GCInfoFlags;	// returns converter function flags
		LPSTR 	GCVersion;	// retrns ptr to cnv version string
		LPSTR	GCDescription; 	// retrns ptr to cnv description string
		LPSTR	GCImpExt;	// retrns ptr to extension(s) for import
		LPSTR	GCImpPrompt;	// retrns ptr to title for imp file sel
		LPSTR	GCExpPrompt;	// retrns ptr to title for exp file sel
		LPSTR	GCRegFormat;	// retrns ptr to clipboard/DDE fmt strng
		} GCGETINFOS, FAR *lpGCGETINFOS;

// All LPSTR variables point to strings no longer than MAXSTRING (including
// terminating NULL) except GCVersion which is no longer than MAXVERTXT in
// length.

// GCInfoSSize is the number of bytes in the structure.  It is filled in
// by the caller before calling this function and may be used by the converter
// to check the specification level supported by the caller.

// GCSpecLevel is the DLL converter specification supported.  Currently, only
// level 1 of the specification exists (this spec) and this value should 
// always return 1 (this spec) for 16-bit converters or 1 + 1000 for
// 32-bit converters.

// GCInfoFlags returns a set of OR'ed flags indicating the options
// supported by the converter.  Currently only the GCI_EXPORTOK,
// GCI_IFILESEL, GCI_IMPMEMOK, GCI_EXPMEMOK and GCI_SRCDIR flags
// are defined (all other bits must be set to 0).
// The GCI_EXPORTOK flag should be set by the converter to indicate
// that it supports export from Galactic format to the native format.  If
// this flag is not set GRAMSCNV will disable its "Export" button.
// If the GCI_IFILESEL flag is set, the calling program will not display
// a file selector when the user tries to import a file.   Instead, the
// calling program will call the converter exported function GCFileSel()
// defined below.  The converter is then responsible for selecting the
// foreign file to import.  MOST CONVERTERS SHOULD NOT SET THIS FLAG!
// The only converters which may need to set this flag are those in which
// the foreign file is encapsulated in a larger database and there is no
// way for a file based selector to choose a file.  Bypassing the
// caller's import file selector may make a converter inoperable with
// some Galactic programs and may cause the converter to fail when
// called directly via the command line or DDE.  In addition, the normal
// GRAMSCNV file selector supports multiple file import while the 
// GCFileSel() call only supports a single file at a time.
// All converters must support import to a Galactic data file in memory. If
// the converter also supports import from a foreign file in memory, it
// should return the GCE_MEMIMPOK flag set.  All converters should
// try to support import from memory if the source format can be
// represented with a single file.  The sample converter includes routines
// which make the import file form transparent to the caller.
// All converters must support export from a Galactic data file in memory. If
// the converter also supports export to a foreign file in memory, it should
// return the GCE_MEMEXPOK flag set.  All converters should try to support 
// export to a memory file if the destination format can be represented 
// with a single file.  The sample converter includes routines which
// make the export file form transparent to the caller.  If a converter
// that normally creates additional files on import (e.g. *.CFL, *.CGM) is
// called to convert a file with the destination Galactic file in memory
// none of the additional files are created.  Similarly, if a converter
// normally exports files in addition to the main foreign data file,
// none of the additonal files are created if the destination file is
// a memory file.  If the converter returns the GCI_SRCDIR flag set it
// indicates that the converter expects a directory name as the source
// filename on import and as the destination filename on export.  Note that
// the destination directory may not exist on file export, the converter is
// responsible for creating the directory if it does not already exist.
// Multiple file conversion when the GCI_SRCDIR is not supported by
// the current GRAMSCNV.EXE
//
// The GCI_SHWLST flag is set by converters with file formats which have
// multiple data files in a single disk file but normally do not require an
// internal file selector (i.e. do not have the GCI_IFILESEL flag set).
// These formats convert a default internal data file, but if the GCC_SHWLST
// flag is set when GCConvert is called on import or export the converter
// presents a list of data files available in the input file (or destination
// file) and allows the user to choose which file to convert.


// GCVersion points to a text string containing the converter version
// This string must end in a "D4" to indicate a 16-bit DLL converter
// (i.e. *.CN4) or end in "D5" for a 32-bit converter (i.e. *.CN5)
// For example "1.03D4\0".  The maximum version string length is MAXVERTXT.
// The converter version must be incremented any time the converter source
// code is modified resulting in a new executable (*.CN4 or *.CN5) file.
// In addition the version information in the converter's version resource
// should also track this version number.  The sample converters place the 
// version information at the top of the <cvnname>.H file to set the converter
// version in both places making it easier to update the file.  We recommend
// incrementing the version number before editing any source files in a 
// converter to be modified.

// GCDescription points to a text string which describes the converter, i.e. 
// "ASCII File with Header"  This string is displayed next to the 
// base file converter name in the initial GRAMSCNV dialog.  The string
// normally should not exceed 60 characters (less if mostly uppercase letters)
// in order to fit the dialog box and is limited to MAXSTRING in all cases.

// GCImpExt points to a NULL terminated text string used by the file import
// file selector for the source file extension(s).  Multiple extensions may be
// specified.  This string has the '|' characters (ASCII 0x7c) replaced with
// NULL characters and the result is passed directly to the Windows Common
// Dialog file open dialog routine.  This string contains one or more pairs of
// '|' separated strings specifying filters. The first string in each pair
// describes a filter (for example, "ASCII Files (*.asp)"); the second
// specifies the filter pattern (for example, "*.asp"). Multiple filters can
// be specified for a single item; in this case, the semicolon (;) is used to
// separate filter pattern strings - for example, "*.txt;*.doc;*.bak". The last
// string in the buffer must be terminated by a null character.  GRAMSCNV adds
// "All Files (*.*)|*.*" to the string returned by GCGetInfo().  If the last
// character in the GCImpExt[] string is a ('|'), then the "All Files
// (*.*)" entry will not be added to the file type dropdown combobox. 
// A converter should not supress the "All Files" addition without a very good
// reason.  The filter strings must be in the proper order, the system does
// not change the order.  Extensions should be in lowercase.  For example:
//
//	"Data Files (*.dat)|*.dat"
//	"Text Files|*.txt;*.prn;*.cap"
//	"Data Files (*.dat)|*.dat|Capture Files (*.cap)|*.cap"
//	"Data Files Only|*.dat|"
//
// The GCImpExt[] string is limited to a maximum of MAXSTRING characters.  The
// first extension is always used as the default on import if no extension is
// typed and is the default for export if no extension is typed.  Note that
// the '|' character is NOT allowed in the string except as the separator. 
// Using the '|' in place of NULL separated strings allows the extension
// specifying string to be placed as a single entry in the converter's
// stringtable resource.  If the GCI_SRCDIR flag is set, the extension string
// is ignored and the return pointer should be set to NULL.

// GCImpPrompt points to a text string used as the prompt for the source file
// selector on import.  This string normally should not exceed 60 characters
// (less if mostly uppercase letters) in order to fit the dialog box and is
// limited to MAXSTRING in all cases.  It should of the form "Convert from
// ASCII File with Header to Galactic File"

// GCExpPrompt points to a text string used as the prompt for the source file
// selector on export.  It should normally be no longer than 60 characters 
// (and is always limited to MAXSTRING characters) and usually
// is of the form "Convert from Galactic File to ASCII File with Header"

// GCRegFormat points to a string specifying the registered clipboard format(s)
// supported by the converter.  Multiple clipboard formats may be specified
// separated by semicolons ';'.  For example:  "CF_OLDINST;CF_NEWINST"
// If no clipboard or DDE registered formats are supported this pointer 
// should be set to NULL.

// **********************************************************************
// Structure passed by GCCheckData() call to converter DLL.  All items in
// this structure are filled in by the caller.

// This function queries the converter to see if it can import a particular
// data file.  The converter returns TRUE if it handles this format or
// FALSE if it does not.  The handle of the open file, the name of the
// open file and a pointer to the first 256 bytes in the file are normally
// passed to this function.  If the caller has the source file only in memory,
// then the file handle (GChFile) is NULL and GCChkBytes is set to the total
// bytes in the memory file pointed to by GCFileData (which may be much 
// larger than 256 bytes - converters should always do a DWORD compare
// against the number of pointed to by GCFileData).  If the data is
// passed solely in memory the filename pointer (GCFileName) will normally
// be NULL.  This function should return as quickly as possible if it
// does not handle the data.  If there are less than 256 bytes in the
// data file - unused bytes should be set to 0.

typedef struct 	{
		DWORD	GCChkDSize;	// length, in bytes, of the structure
		DWORD	GCChkBytes;	// actual number of bytes in GCFileData
		LPSTR   GCFileName;	// ptr to name of open file GChFile
		LPVOID	GCFileData;	// ptr to first GCChkBytes bytes in file
#ifndef FLAT32
		HFILE	GChFile; 	// handle of file to check or NULL
#else
		HANDLE	GChFile; 	// handle of file to check or NULL
#endif
		} GCCHECKDATAS, FAR *lpGCCHECKDATAS;

// GCChkDSize is the number of bytes in the structure.  It is filled in
// by the caller before calling this function.

// GCChkBytes is the actual number of bytes in the buffer pointed to by
// GCFileData.  This a number between 0 and 256 if GChFile is not NULL,
// otherwise it is the total number of bytes in the memory file.  Some
// converters may use this value to quickly qualify or reject a file.

// GCFileName points to the name of the file being tested.  This may be NULL.
// The file name is provided for those cases where the converter needs to
// open or check for the presence of an auxiliary file to test the format.

// GCFileData points to a buffer holding the first GCChkBytes bytes in
// the data file.  If GChFile is not NULL this will not exceed 256 bytes.

// GChFile is the handle of an open data file (HFILE type opened with the
// OpenFile() function call if 16-bit or HANDLE type opened with CreateFile()
// function call if 32-bit) or NULL if the entire data file is pointed to by
// GCFileData.

// The converter MUST NOT CLOSE the data file GChFile, but it does not have
// to reset the file pointer to the beginning of the file if it reads from
// the file or moves the file pointer.  The file pointer is always reset to
// the beginning of the file before this function is called.

// *****************************************************************
// Structure passed by GCConvert() call to converter DLL.
// All items are filled in by caller (GCErrorMsg and GCDestMemFile are
// set to NULL by the caller and GCDestMemFSize is set to 0L.  
// GCSrcMemFile and GCSrcMemFSize are set to NULL and 0L respectively
// by the caller if not used).  The return value is 0 if no error occurs
// or an error code (see GCDLLERR.H) if the conversion fails.

//	GCCvtFlags defines

#define	GCC_EXPORT	 0x0001L 	// set if export operation
#define	GCC_NOUSER	 0x0002L 	// set to supress user interaction
#define	GCC_SHWLST	 0x0004L 	// show multiple file choice if any
/
// If the GCC_EXPORT flag is set on entry, the file pointed to GCSrcFileNm
// (or in the memory file GCSrcMemFile) is a Galactic data file which
// should be exported to the converter's native format.  Note that the
// converter should handle this case  (with an error message) even if the
// converter did not return the GCI_EXPORTOK flag on the call to GCGetInfo()
// since some callers may call this API without first calling GCGetInfo().

// If the GCC_NOUSER flag is set by the caller then the converter should
// convert the file with no user interaction (however it should still return
// and error code and message if applicable).  This flag is normally ignored
// since most file conversions do not require any interaction in order to
// convert a file.  However some converters give the user conversion options
// or warning messages and these should not be displayed if the GCC_NOUSER
// flag is set.  If the converter cannot continue without user interaction it
// should return the GCE_NOUSER error code.  Note that a converter can still
// call GCCallback with messages even if this flag is set as it is up to the
// calling program whether to display them or not.

// The GCC_SHWLST flag is used only by file formats which have multiple
// data files in a single disk file but normally do not require an internal
// file selector (i.e. do not have the GCI_IFILESEL flag set).  These formats
// convert a default internal data file, but if the GCC_SHWLST flag is set
// on import or export the converter presents a list of data files available 
// in the input file (or destination file) and allows the user to choose
// which file to convert.  This flag is ignored if the file format does not
// contain more than one data file or if the GCE_NOUSER flag is set.

typedef struct 	{
		DWORD	GCCvnSSize;	// length, in bytes, of the structure
		DWORD	GCCvtFlags;	// GCC_EXPORT and/or GCC_NOUSER	
		LPSTR	GCSrcFileNm; 	// ptr to source filename
		HGLOBAL	GCSrcMemFile;	// handle for source memory file
		DWORD	GCSrcMemFSize;	// bytes in source memory file
		LPSTR	GCDestFileNm;	// ptr to destination filename
		HGLOBAL	GCDestMemFile;	// handle for destination memory file
		DWORD	GCDestMemFSize;	// bytes in destination memory file
		FARPROC GCCallback;	// caller timesharing callback or NULL
		LPSTR	GCErrorMsg;	// returns optional ptr to err msg text
		HWND	GChParent;	// handle of parent window
		} GCCONVERTS, FAR *lpGCCONVERTS;

// GCCnvSSize is the number of bytes in the structure.  It is filled in
// by the caller before calling this function.

// GCCvtFlags contains the action flags defined above, all other bits
// will be set to 0.

// GCSrcFileNm is a pointer to the name of the file to open (or NULL if
// the source is a memory file).  It is a maximum of MAX_PATH bytes long.
// The converter cannot modify this buffer, it must copy the filename to
// a local buffer if it needs to modify the string.  The file is not open
// when the call to GCImport() or GCExport() is made.  The converter is
// responsible for opening (in read only mode) and closing the file.  
// Normally, when called by GRAMSCNV.EXE this string will contain a filename
// and extension but no path.  GRAMSCNV.EXE changes the current directory
// to the directory containing the source file before calling the GCConvert()
// function.  However, other users of the converter may choose to pass a full
// pathname instead.  The converter must handle both cases.  The converter
// does not have to restore the original directory if it changes the current
// directory.  If this pointer is NULL then the source file is avaiable in
// the block of global memory whose handle is passed in GCSrcMemFile.

// GCSrcMemFile is the handle of a block of global memory containing
// the source data file if GCSrcFileNm is NULL.  The converter must not
// free this memory block but it must unlock it before returning to the
// caller.  If the converter does not support import from a memory file
// it should return GCE_BADOP.

// If a source memory file is supplied, GCSrcMemFSize is set to the total
// number of bytes in the memory file (not the amount of memory allocated
// which may be greater).  This allows the user of the memory file to ensure
// that they do not try to access memory beyond the end of the file if the
// file format contains errors. 

// GCDestFileNm is a pointer to the name of the file to create (or NULL if
// the destination is a memory file).  It is a maximum of MAX_PATH bytes
// long.  The converter cannot modify this buffer, it must copy the filename
// to a local buffer if it needs to modify the string.  The file is not open
// when the call to GCImport() or GCExport() is made.  The converter is
// responsible for creating this file.  The converter does NOT need to check
// to see if the file already exists, the caller is responsible for this. 
// The converter should delete any partial destination file(s) if an error
// occurs.  This string may contain a path or may contain just a filename and
// extension.  The converter must handle both cases.  In particular, make sure
// the converter creates the destination file before changing the current
// directory if the destination file does not include a path.  If GCDestFileNm
// is NULL the converter should create the destination data file as a memory
// file.

// If GCDestFileNm is NULL the converter should return the destination
// file in a block of global memory allocated with the GHND flag.  On return,
// the memory handle should be stored in GCDestMemFile and GCE_OK returned.  
// The memory handle returned must be unlocked by the converter before
// returning.  The caller is responsible for FREE'ing the handle returned.
// If an error occurs (and the converter returns any value other than GCE_OK)
// the converter is responsible for freeing any memory allocated and should
// return a NULL value in GCDestMemFile.  If insufficient memory exists to
// return the data file in memory, the converter should return GCE_MEMORY.
// The caller can then choose to call the function again with a temporary
// disk file as the destination name.  If the converter does not support
// export to a memory file it should return GCE_BADOP.

// If a destination memory file is returned, GCDestMemFSize is set to the 
// total number of bytes in the memory file (not the amount of memory
// allocated which may be greater).  This allows the user of the memory
// file to ensure that they do not try to access memory beyond the end of
// the file if the file format contains errors. 

// If a converter that normally creates additional files on import
// (e.g. *.CFL, *.CGM) is called to import a file to a Galactic memory
// file none of the additional files are created.  Similarly, if a converter
// normally exports suplemental files in addition to the main foreign data
// file, none of the additonal files are created if the destination file is
// a memory file.  The destination file must be a named disk file 

// GCCallback is a pointer to a function which allows Windows non-
// preemptive timesharing in while the converter is in a tight conversion
// loop and allows the converter to set status messages in the caller's
// dialog window.  It also checks for abort by the user.  The converter
// must call this function often (no less than twice per second) while
// converting to allow Windows multitasking to occur and to check for
// user abort.  The return value is a BOOL.  TRUE to abort the conversion, 
// FALSE to continue.  If this function returns TRUE, the converter should
// end its conversion and return (GCE_CANCEL) to the caller as soon as possible.
// Note that as long as the converter calls this function often, it does not
// have to call PeekMessage() to allow Windows multitasking.
//
// The callback function is declared as follows:
//
//	BOOL CALLBACK GRCheckAbort (LPSTR szStatus)
//
// The LPSTR parameter should be set to NULL if no status message needs
// to be displayed.  IMPORTANT!  If the caller does not support the
// callback function it will be set to NULL.  The converter should not
// call a NULL callback function!

// GCErrorMsg is set to NULL by the caller.  If GCConvert() returns GCE_OK
// or GCE_CANCEL, this value is ignored and the caller does not put up any
// error dialog.  If any other error code is returned, the caller displays 
// the error in an error dialog.  If GCErrorMsg is returned NULL, then the
// caller uses its own message string (see GRDLLERR.H for standard error
// strings)  based on the error code returned.  If GCErrorMsg is not NULL,
// the string it points to is used for the error message.  Note that you
// can OR GCE_WARN or GCE_INFO to the error code to display a Warning (!)
// or Information (I) dialog respectively rather than an Error (Stop Sign)
// dialog.  Also, you can add the source or destination filename to the end
// of the message string (either the msg returned by the converter or the
// internal error messsage) by OR'ing the GCE_SRC or GCE_DEST flags to the
// returned error code, however the default error dialog already displays
// the source file name in the dialog title.  If the GCE_WARN or the GCE_INFO
// flags are not set, the error dialog is preceeded by a Galactic error beep.
// If a converter returns a custom error message in GCErrorMsg for a condition
// which is not defined in GCDLLERR.H, the function should return GCE_UNKNOWN.

// GChParent is the handle of the parent window.  This window may be hidden
// if the conversion is taking place in the background or it may be iconic.
// The converter should not popup a dialog or window if the parent window
// is not the active application.  It should wait until the parent window
// becomes active to put up an error.  Normally, converters do not need
// to do any user interaction and may ignore this value

// *****************************************************************
//  		The following call is optional and should
//		not be supported by most file converters
// *****************************************************************

// Structure passed by optional GCFileSel() call to converter DLL.
// All items except GCFSelBuffer are filled in by caller.  The return
// value is count of filenames in buffer, 0 if import should be canceled,
// or -1 if an error occurs.

//	GCFileSel defines

#define	GCF_LONGFN	 0x0001L 	// set if long filenames allowed
#define	GCF_MULTIOK	 0x0002L 	// set if multiple null sep names ok

typedef struct 	{
		DWORD	GCFSelSSize;	// length, in bytes, of the structure
		DWORD	GCFSelFlags;	// GCF_LONGFN and/or GCF_MULTIOK
		LPSTR	GCFSelBuffer; 	// ptr to returned filename(s)
		HWND	GCFhParent;	// handle of parent window
		} GCFILESELS, FAR *lpGCFSELS;

// GCFSelSSize is the number of bytes in the structure.  It is filled in
// by the caller before calling this function.

// GCFSelFlags are set by the caller to indicate what options are
// supported by the converter.  If the GCF_LONGFN flag is not set, the
// filename returned in GCFSelBuffer must conform to the DOS 8.3 filenaming
// conventions.   The filenames may include a full path or a filename only.
// If no path is returned, the current directory is assumed.  The GCFileSel()
// call may change the current directory.   

// If the GCF_MULTIOK flag is set, the converter may return multiple NULL
// separated filenames in the GCFSelBuffer.  The value returned by the 
// function is the actual number of filenames in the buffer.  The maximum
// number of filenames returned must be less than 32768 (in the 16-bit
// version of the converter) since the return value must be positive.
// Currently, the 2.00 version of GRAMSCNV does not set either flag (so the
// return value must be -1, 0 or 1).  All undefined bits in GCFSelFlags
// are set to 0 by the caller.

// The converter must point GCFSelBuffer to a buffer containing the file(s)
// selected separated by NULL characters.  Each filename returned is copied
// to the GCSrcFileNm buffer in the GCCONVERTS structure before GCConvert()
// is called.  The caller does no parsing or checking of the filename(s)
// returned.  GRAMSCNV does NOT check for the validity of the filename
// returned but merely passes it back to GCConvert().  The maximum length of
// a single filename is MAX_PATH.  Note that if the converter allocates
// memory for the filename buffer, it is responsible for freeing it when it
// unloads.

// GCFhParent is the window handle of the parent window which may be needed 
// if the converter needs to put up a dialog window or file selector.

// ******************************************
// Required functions exported by converter
// ******************************************

DllExport BOOL WINAPI EXPORT GCGetInfo (lpGCGETINFOS);		// ordinal @2
DllExport BOOL WINAPI EXPORT GCCheckData (lpGCCHECKDATAS);	// ordinal @3
DllExport UINT WINAPI EXPORT GCConvert (lpGCCONVERTS);		// ordinal @4

// ******************************************
// The following function is only reguired if
// the GCI_IFILESEL flag is set on the call to
// GCGetInfo
// ******************************************

DllExport int  WINAPI EXPORT GCFileSel (lpGCFSELS);		// ordinal @5

