set ( QTPLATZ_CONFIG_Batch     OFF )
set ( QTPLATZ_CONFIG_Sequence  OFF )
set ( QTPLATZ_CONFIG_Cluster   OFF )

#-------->
set ( QTPLATZ_CONFIG_Servant    ON )
set ( QTPLATZ_CONFIG_Acquire    ON ) # was OFF
set ( QTPLATZ_CONFIG_Dataproc   ON )
set ( QTPLATZ_CONFIG_Quan       ON )
set ( QTPLATZ_CONFIG_figshare   ON )

set ( QTPLATZ_CONFIG_Chemistry  ${rdkit_FOUND} )
set ( QTPLATZ_CONFIG_Peptide    ON  )
set ( QTPLATZ_CONFIG_Query      ON  )
set ( QTPLATZ_CONFIG_Lipidid    ON  )
set ( QTPLATZ_CONFIG_SDFViewer  OFF ) # not be able to compile after QT_VERSION 8 or higher
set ( QTPLATZ_CONFIG_Video      OFF ) # ${OpenCV_FOUND} )
set ( QTPLATZ_CONFIG_Example    OFF )
set ( QTPLATZ_CONFIG_HelloWorld OFF )
set ( QTPLATZ_CONFIG_AccutofAcquire ON )
