main: main.o ltc68042_bcm.o 
		gcc -o main main.o ltc68042_bcm.o -lbcm2835

main.o: main.c 
		gcc -c main.c -lbcm2835

ltc68042_bcm.o: ltc68042_bcm.c ltc68042_bcm.h 
		gcc -c ltc68042_bcm.c -lbcm2835 

clean:
		rm -rf main.o ltc68042_bcm.o main