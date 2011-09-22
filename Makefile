
CC	= gcc

CFLAGS	= -Wall -g -D_DEBUG_ -D_DEBUG_VIR -lm -ljpeg -lpthread

OBJECT	= digitbox.c framebuffer.c config.c udisk.c resource.c maindeal.c

OBJECT_H = digitbox.h framebuffer.h config.h udisk.h resource.h maindeal.h

LDFLAGS	= 

digitbox.out: $(OBJECT) $(OBJECT_H)
	$(CC) $(CFLAGS) -o $@ $(OBJECT)

.PHONY: all clean

clean:
	rm -f *.o *~ digitbox.out