#!/bin/bash
grep "round trip" qtplatz.log | sed -r 's/^.*at ([0-9\.]+)[ \t]*round trip[ \t]*([0-9\.]+).*$/\1,\t\2/g' | sed 'N;N;s/\n/,\t/g'
