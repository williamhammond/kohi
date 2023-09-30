REM Build script for engine
@ECHO OFF
SetLocal EnableDelayedExpansion

REM Get a list of all the .c files
SET cFiles=
FOR /R %%f in (*.c) DO (
    SET cFiles=!cFiles! "%%f"
)

REM echo "Files: " %cFiles%

SET assembly=engine
SET compilerFlags=-g -shared -Wvarargs -Wall -Werror
REM -Wall -Werror
SET includeFlags=-Isrc -I%VULKAN_SDK%\Include
SET linkerFlags=-luser32 -lvulkan-1 -L%VULKAN_SDK%\Lib
SET defines=-D_DEBUG -DKEXPORT -D_CRT_SECURE_NO_WARNINGS

ECHO "Building %assembly%..."
clang %compilerFlags% %includeFlags% %linkerFlags% %defines% %cFiles% -o ../bin/%assembly%.dll