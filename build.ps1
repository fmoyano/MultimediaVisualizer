clear

pushd build
cl /std:c11 /Zi /FC /sdl /permissive- /W4 /guard:cf /I ../include ../src/*.c /Fe:main User32.lib Gdi32.lib /link /LIBPATH:"../lib" /SUBSYSTEM:WINDOWS /MACHINE:X86
popd
