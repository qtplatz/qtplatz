#!/bin/bash

if [ -z $outfile ]; then
    outfile='count_peaks.db'
fi
   
rm -f $outfile
sqlite3 -batch $outfile <<EOF
CREATE TABLE IF NOT EXISTS datafiles (
  datafile TEXT
, grpid INTEGER
, id INTEGER PRIMARY KEY
, UNIQUE( datafile )
);

CREATE TABLE IF NOT EXISTS peaks ( 
 dataid INTEGER
, idTrigger INTEGER
, protocol INTEGER
, peak_time REAL
, peak_intensity REAL
, front_offset INTEGER
, front_intensity REAL
, back_offset INTEGER
, back_intensity REAL
, FOREIGN KEY ( dataid ) REFERENCES datafiles ( id )
);
EOF

#datadir=~/Documents/data/wspc/Stability-study_20-laps
#for file in $datadir/*.adfs
for file in "$@"
do
    data=$(basename $file)
    echo $file
    sqlite3 -batch $file <<EOF
ATTACH '$outfile' as merged;
INSERT INTO merged.datafiles ( datafile ) VALUES ( '$data' );
INSERT INTO merged.peaks (
 dataid
, idTrigger
, protocol
, peak_time
, peak_intensity
, front_offset
, front_intensity
, back_offset
, back_intensity
)
SELECT
  (SELECT id FROM merged.datafiles WHERE datafile like '$data')
  ,id
  ,protocol
  ,peak_time
  ,peak_intensity
  ,front_offset
  ,front_intensity
  ,back_offset
  ,back_intensity
FROM peak,trigger WHERE id=idTrigger;
EOF
done

# fix intensity < front_intensity
sqlite3 -batch $outfile <<EOF
  UPDATE peaks SET peak_intensity = front_intensity WHERE front_intensity < peak_intensity;
EOF

sqlite3 -batch $outfile <<EOF
  UPDATE datafiles SET grpid = id;
  UPDATE datafiles SET grpid = -1 WHERE cast( substr( datafile, -8, 3 ) as int ) < 3; /* outliers if runno < 3 */
EOF

# SELECT cast( substr( datafile, -8, 3 ) as int ) FROM datafiles



