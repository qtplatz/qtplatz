#!/bin/bash

if [ -z $outfile ]; then
   outfile='frequency.db'
fi

sqlite3 -separator $'\t' -batch $outfile <<EOF
SELECT
 protocol
,threshold
,sum(counts)
,avg(average_peak_time)
,avg(average_peak_intensity)
,avg(average_peak_width) 
FROM frequency 
WHERE threshold < -5.0
GROUP BY threshold,protocol;
EOF

