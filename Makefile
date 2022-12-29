CC = gcc

CFLAGS = `python3-config --cflags`
CFLAGS += -fPIE

# For x86_64 SIMD
CFLAGS += -Ofast -flto -ffast-math -march=native -mno-vzeroupper -msse3 -msse4 -mavx2 -mavx

PYTHON_VERSION = 3.10
PYTHON_MODULES = -I/usr/lib/python$(PYTHON_VERSION)/site-packages/numpy/core/include/

CFLAGS += -Ibuild/ -Isrc/ # Add build to path

CFLAGS += $(PYTHON_MODULES)
LFLAGS = `python3-config --embed --ldflags` # Must be --embed for >python3.8

LFLAGS += $(PYTHON_MODULES)
BIN = beamformer

BUILD = build

.PHONY: all

all: lib/libdelay.so run lib/beamformer.so

build/cy_api.c:
	cython -o build/cy_api.c src/cy_api.pyx

build/cy_api.o: build/cy_api.c
	$(CC) $(CFLAGS) -c build/cy_api.c -o build/cy_api.o

build/main.o:
	# cp src/main.c build/main.c # Force adding main.c to build
	$(CC) $(CFLAGS) -c src/main.c -o build/main.o

lib/beamformer.so: build/main.o build/cy_api.o
	$(CC) ${CFLAGS} -lm -shared -o lib/beamformer.so build/main.o build/cy_api.o -fPIC

build/delay.o: src/delay.c
	$(CC) $(CFLAGS) -lm -c -o build/delay.o src/delay.c

lib/libdelay.so: build/delay.o
	$(CC) $(CFLAGS) -lm -shared -o lib/libdelay.so build/delay.o -fPIC

run: build/cy_api.o build/main.o
	$(CC) build/main.o build/cy_api.o $(LFLAGS) -o $(BIN) 

.PHONY: clean

clean:
	echo "Removing Generated Build Files"
	#rm *.o *.so cymod.c cymod.h $(BIN)
	rm $(BUILD)/*.c $(BUILD)/*.h $(BUILD)/*.o

	echo "Removing Binaries"
	rm $(BIN)

	echo "Removing Libraries"
	rm lib/*.so