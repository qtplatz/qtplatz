INSERT OR REPLACE INTO Spectrometer ( id, scanType, description, fLength )
       VALUES ( 'd8472724-40dd-4859-a1de-064b4a5e8320', 0, 'MULTUM Chamber for MALPIX EXP.', 0.5 )
       ;

INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer )
       VALUES ( '76d1f823-2680-5da7-89f2-4d2d956149bd'
       	      	, '1.ap240.ms-cheminfo.com'
	      	, 4000.0
		, 0.0
		, 'd8472724-40dd-4859-a1de-064b4a5e8320'
		, 'e45d27e0-8478-414c-b33d-246f76cf62ad' )
		;

INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer )
       VALUES ( '62ede8f7-dfa3-54c3-a034-e012173e2d10'
       	      	, '1.image.malpix.ms-cheminfo.com'
	      	, 4000.0
		, 0.0
		, 'd8472724-40dd-4859-a1de-064b4a5e8320'
		, 'e45d27e0-8478-414c-b33d-246f76cf62ad' )
		;

INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer )
       VALUES ( 'ebec355c-3277-5b15-9430-b83031e7555c'
       	      	, 'histogram.1.malpix.ms-cheminfo.com'
	      	, 4000.0
		, 0.0
		, 'd8472724-40dd-4859-a1de-064b4a5e8320'
		, 'e45d27e0-8478-414c-b33d-246f76cf62ad' )
		;

INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer )
       VALUES ( '4f431f91-b08c-54ba-94f0-e1d13eba29d7'
       	      	, 'timecount.1.ap240.ms-cheminfo.com'
	      	, 4000.0
		, 0.0
		, 'd8472724-40dd-4859-a1de-064b4a5e8320'
		, 'e45d27e0-8478-414c-b33d-246f76cf62ad' )
		;

INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer )
       VALUES ( '89a396e5-2f58-571a-8f0c-9da68dd31ae4'
       	      	, 'histogram.timecount.1.ap240.ms-cheminfo.com'
	      	, 4000.0
		, 0.0
		, 'd8472724-40dd-4859-a1de-064b4a5e8320'
		, 'e45d27e0-8478-414c-b33d-246f76cf62ad' )
		;

INSERT OR REPLACE INTO ScanLaw ( objuuid, objtext, acclVoltage, tDelay, spectrometer, clsidSpectrometer )
       VALUES ( 'eb9d5589-a3a4-582c-94c6-f7affbe8348a'
       	      	, 'tdcdoc.waveform.1.ap240.ms-cheminfo.com'
	      	, 4000.0
		, 0.0
		, 'd8472724-40dd-4859-a1de-064b4a5e8320'
		, 'e45d27e0-8478-414c-b33d-246f76cf62ad' )
		;

