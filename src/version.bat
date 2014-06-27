git describe > version.tmp
set /p VERSION=< version.tmp
echo #define VERSION "%VERSION%" > version.h
