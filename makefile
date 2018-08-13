LTC68042.exe: main.o LTC6804_2.o 
		gcc main.o LTC6804_2.o -o LTC68042.exe

main.o: main.c 
		gcc -c main.c -lwiringPi -o main.o

LTC6804_2.o: LTC6804_2.c LTC6804_2.h LTC68042_PI.h
		gcc -c LTC6804_2.c -lwiringPi -o LTC6804_2.o

clean:
		rm -rf main.o LTC6804_2.o LTC68042.exe
