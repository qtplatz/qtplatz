#!/bin/sh
# extract the creation date-string and sample injection time(steady_clock epoch_time) and store them to timestamp.db

sqlite3 -batch 'timestamp.db' <<EOF
CREATE TABLE IF NOT EXISTS results (
      file TEXT
      , ctime TEXT
      , epoch_time INTEGER
      , UNIQUE( file )
);
EOF

for file in "$@"
do
    echo "Processing file: " $file 

    sqlite3 -batch $file <<EOF
ATTACH 'timestamp.db' as merged; 
INSERT OR IGNORE INTO merged.results( file, epoch_time, ctime ) SELECT DISTINCT '$file',epoch_time,(SELECT ctime FROM superblock) FROM AcquiredData WHERE events = 285212672;
EOF

done

