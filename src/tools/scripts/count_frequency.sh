#!/bin/bash

if [ $# -lt 1 ]; then
    echo $0 file1 [file2 ...]
    exit 0
fi

if [ -z $outfile ]; then
   outfile='frequency.db'
fi

if [ -z $tof_lower_limit ]; then
    tof_lower_limit=0
fi

if [ -z $tof_upper_limit ]; then
    tof_upper_limit=1.0
fi

echo "tof range = " $tof_lower_limit ", " $tof_upper_limit

sqlite3 -batch $outfile <<EOF
CREATE TABLE IF NOT EXISTS datafiles (
 id INTEGER PRIMARY KEY
, datafile TEXT
, grpid INTEGER
, expid INTEGER
, flowrate INTEGER
, UNIQUE( datafile )
);

CREATE TABLE IF NOT EXISTS frequency (
 dataid INTEGER
, protocol INTEGER
, threshold REAL
, average_peak_time
, average_peak_intensity
, average_peak_width
, counts
, FOREIGN KEY ( dataid ) REFERENCES datafile ( id )
);
EOF

for file in "$@"
do
    data=$(basename $file)
    echo $file
    sqlite3 -batch $file <<EOF
ATTACH '$outfile' as merged;
INSERT OR REPLACE INTO merged.datafiles ( datafile ) VALUES ( '$data' );

INSERT OR REPLACE INTO merged.frequency (
dataid
, protocol
, threshold
, average_peak_time
, average_peak_intensity
, average_peak_width
, counts
) SELECT 
  (SELECT id from merged.datafiles WHERE datafile like '$data')
, protocol
, ROUND(peak_intensity/5)*5 AS threshold
, avg(peak_time)
, avg(peak_intensity)
, avg(peak_width)
, COUNT(*)
 FROM 
  ( SELECT protocol
          ,peak_time
          ,peak_intensity
          ,(back_offset-front_offset) as peak_width
     FROM peak,trigger
    WHERE id=idTrigger AND peak_time > '$tof_lower_limit' AND peak_time < '$tof_upper_limit'
  ) WHERE peak_intensity < threshold GROUP by threshold,protocol;
EOF
done

sqlite3 -batch $outfile <<EOF
SELECT * FROM frequency;
EOF

