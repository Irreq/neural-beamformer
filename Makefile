SRC = src
OUT = lib

CFLAGS = -Ofast -flto -ffast-math -march=native -mno-vzeroupper -msse3 -msse4 -mavx2 -mavx

.PHONY: all

all: libdelay.so

delay.o:
	gcc ${CFLAGS} -lm -c -o ${OUT}/delay.o ${SRC}/delay.c

libdelay.so: delay.o
	gcc ${CFLAGS} -lm -shared -o ${OUT}/libdelay.so ${OUT}/delay.o -fPIC

.PHONY: clean

clean:
	rm ${OUT}/*.so ${OUT}/*.o