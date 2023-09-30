REM Build script for testbed
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files
SET cFiles=
FOR /R %%f in (*.c) DO (
    SET cFiles=!cFiles! "%%f"
)

REM echo "Files: " %cFiles%

SET assembly=testbed
SET compilerFlags=-g
REM -Wall -Werror
SET includeFlags=-Isrc -I../engine/src/
SET linkerFlags=-L../bin -lengine.lib
SET defines=-D_DEBUG -DKIMPORT

ECHO "Building %assembly%..."
clang %compilerFlags% %includeFlags% %linkerFlags% %defines% %cFiles% -o ../bin/%assembly%.exe