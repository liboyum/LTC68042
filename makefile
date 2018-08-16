LTC68042.exe: main.o LTC6804_2.o 
		gcc -o main.o LTC6804_2.o -lwiringPi

main.o: main.c 
		gcc -c main.c -lwiringPi

LTC6804_2.o: LTC6804_2.c LTC6804_2.h LTC68042_PI.h
		gcc -c LTC6804_2.c -lwiringPi 

clean:
		rm -rf main.o LTC6804_2.o LTC68042.exe
