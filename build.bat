@ windres.exe -i main.rc --input-format=rc -o main.res -O coff

@ g++ -O2 -s -c main.cpp -o main.o

@ g++ -mwindows main.res main.o -o main.exe -static-libgcc -l gdi32

@ del *.o *.res
