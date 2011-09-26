
CC	= gcc

CFLAGS	= -Wall -g -D_DEBUG_ -D_DEBUG_VIR -lm -ljpeg -lpthread \
	-lfreetype -lz

OBJECT	= digitbox.c framebuffer.c config.c udisk.c resource.c \
	maindeal.c screen.c image.c pixel.c jpeg.c line.c \
	plane.c font.c text.c weather.c effects.c

OBJECT_H = digitbox.h framebuffer.h config.h udisk.h resource.h \
	maindeal.h screen.h image.h pixel.h jpeg.h line.h \
	plane.h font.h text.h weather.h effects.h

LDFLAGS	= 

digitbox.out: $(OBJECT) $(OBJECT_H)
	$(CC) $(CFLAGS) -o $@ $(OBJECT)

.PHONY: all clean

clean:
	rm -f *.o *~ digitbox.out