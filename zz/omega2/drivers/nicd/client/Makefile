
# NICCTL.BIN
# NIC Control (Client-side)


VERSION = 0
PATCHLEVEL = 0
SUBLEVEL = 0
EXTRAVERSION =
#NAME = 

CFLAGS = -m64 \
	-std=gnu89 \
	-nodefaultlibs \
	-nostdinc \
	-nostdlib \
	-static \
	-fgnu89-inline \
	-ffreestanding \
	-fno-builtin \
	-fno-pie \
	-fleading-underscore \
	-fno-stack-protector \
	-s \
	-Werror=strict-prototypes    

#rtl
LIBC      = ../../../../omega1/libs/rtl/include/
LIBCOBJ   = ../../../../omega1/libs/rtl/obj
#libgns2 (Change this name)
LIBGNS    = ../../../../omega1/libs/libgns2/include/
LIBGNSOBJ = ../../../../omega1/libs/libgns2/obj

# Objects
myObjects = crt0.o main.o globals.o rtl.o \
ctype.o stdio.o fcntl.o stdlib.o string.o \
unistd.o termios.o ioctl.o socket.o gns.o    

.PHONY: all all-c hello-link finalize clean

all: main.o all-c hello-link finalize clean
	@echo "Ok?"

main.o:
	gcc  -c  main.c  $(CFLAGS) \
    -I $(LIBC) \
    -I $(LIBGNS) \
    -I include/ \
    -o main.o
all-c:

	gcc  -c globals.c  $(CFLAGS) \
    -I $(LIBC) \
    -I $(LIBGNS) \
    -I include/ \
    -o globals.o

# Imports.

	cp $(LIBCOBJ)/crt0.o   .
	cp $(LIBCOBJ)/rtl.o  .

	cp $(LIBCOBJ)/ctype.o   .
	cp $(LIBCOBJ)/stdio.o .
	cp $(LIBCOBJ)/fcntl.o .
	cp $(LIBCOBJ)/stdlib.o .
	cp $(LIBCOBJ)/string.o .
	cp $(LIBCOBJ)/unistd.o  .

	cp $(LIBCOBJ)/termios.o  .
	cp $(LIBCOBJ)/ioctl.o  .
	cp $(LIBCOBJ)/socket.o  .

	cp $(LIBGNSOBJ)/gns.o .

hello-link:
	ld -m elf_x86_64 -T ../link.ld -o NICCTL.BIN $(myObjects)    
finalize:
	cp NICCTL.BIN  ../bin
clean:
	-rm *.o
	-rm NICCTL.BIN 
	
