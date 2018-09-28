CFLAGS ?= -Wall -s -Os -flto
# xcb headers
CFLAGS != pkgconf --cflags xcb
CFLAGS += -L../sulfur/ -lsulfur
CFLAGS += -I../sulfur/include/
OUT := cinnabar

LIBS != pkgconf --libs xcb

all: $(OUT)

$(OUT): src/*.c
	$(CC) -o $(OUT) src/*.c $(LIBS) $(CFLAGS)

clean:
	rm $(OUT)
