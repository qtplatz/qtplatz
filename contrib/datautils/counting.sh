#!/bin/bash

if [ $# -lt 1 ]; then
    echo $0 file1 [file2 ...]
    exit 0
fi

if [ -z $outfile ]; then
   outfile='counting.db'
fi

if [ -z $TOF ]; then
    TOF=0
fi

if [ -z $WIDTH ]; then
    WIDTH=1.0
fi

for file in "$@"; do
	echo $file
done

sqlite3 -batch $outfile <<EOF
CREATE TABLE IF NOT EXISTS datafiles (
 id INTEGER PRIMARY KEY
, datafile TEXT
, description TEXT
, UNIQUE( datafile )
);

CREATE TABLE IF NOT EXISTS counts (
  id INTEGER PRIMARY KEY
, dataid INTEGER
, timeSinceEpoch
, protocol INTEGER
, tR REAL
, intensity REAL
, counts INTEGER
, FOREIGN KEY ( dataid ) REFERENCES datafiles ( id )
);
EOF

for file in "$@"
do
    data=$(basename $file)
    echo $file
    sqlite3 -batch $file <<EOF
ATTACH '$outfile' as merged;
INSERT OR IGNORE INTO merged.datafiles ( datafile ) VALUES ( '$data' );

INSERT OR IGNORE INTO merged.counts (
dataid
,timeSinceEpoch
,protocol
,tR
,intensity
,counts ) SELECT
  (SELECT id from merged.datafiles WHERE datafile like '$data')
 ,timeSinceEpoch
 ,protocol
 ,ROUND((timeSinceEpoch-(SELECT timeSinceEpoch from trigger WHERE events = 285212672))*1e-9) as tR
 ,AVG(peak_intensity)
 ,COUNT(*)
  FROM trigger,peak WHERE id=idTrigger AND protocol='$protocol' AND peak_time > '$TOF' - '$WIDTH' AND peak_time < '$TOF' + '$WIDTH' GROUP BY tR;

EOF
done

exit
