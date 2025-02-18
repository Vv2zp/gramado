
# ke/
# Compile all the files and create ar_ke.o

AS	= as
LD	= ld
CC	= gcc
AR	= ar
MAKE	= make
NASM	= nasm
OBJCOPY	= objcopy
OBJDUMP	= objdump
#LEX	= flex
#YACC	= bison
PERL	= perl
PYTHON	= python
PYTHON2	= python2
PYTHON3	= python3
RUBY	= ruby


# test: gramado 64bit
# See: https://gcc.gnu.org/onlinedocs/gcc/Code-Gen-Options.html
CFLAGS := \
	-Wall \
	-Wundef \
	-Wmissing-prototypes \
	-Wno-uninitialized \
	-Wno-format-security \
	-Wno-trigraphs \
	-Werror=strict-prototypes \
	-Werror=implicit-function-declaration \
	-Werror=implicit-int \
	-Werror=return-type \
	-std=gnu89 \
	-m64 \
	-s \
	-static \
	-fcommon \
	-fgnu89-inline \
	-finline-functions \
	-fshort-wchar \
	-ffreestanding \
	-fleading-underscore \
	-fstrict-overflow \
	-nostdlib \
	-nodefaultlibs \
	-nostdinc \
	-fno-builtin \
	-fno-stack-protector \
	-fno-strict-aliasing \
	-fno-PIE \
	-fno-pie \
	-fno-omit-frame-pointer \
	-fno-optimize-sibling-calls    


# Exported files. Kernel api;
# Used by the kernel modules and user space applications.
USER_INCLUDE   := -I ../admin/include/uapi/

# Internal files
# Used to build the kernel.
KERNEL_INCLUDE := -I ../admin/include/ $(USER_INCLUDE)  

# See: https://linux.die.net/man/1/ld
# -s --strip-all:   Omit all symbol information from the output file.
# -x --discard-all: Delete all local symbols.
# -M --print-map:   Print a link map to the standard output.
# LDFLAGS := -m elf_x86_64 -static
LDFLAGS := -m elf_x86_64 -s -x -static  

# Everything is a file.
OBJS = process.o plib.o thread.o tlib.o \
msg.o \
create.o clone.o \
detect.o \
ke.o \
hal.o \
apic.o \
pit.o ports64.o rtc.o \
x64.o x64info.o x64init.o x64mi.o x64nmi.o x64smp.o \
queue.o \
ts.o dispatch.o spawn.o x64cont.o \
sched.o schedi.o schedq.o \
pixel.o line.o rect.o bitblt.o bg.o font.o char.o text.o gre.o \
input.o dispsrv.o callback.o    

all:

#--------
# ps/
	$(CC) -c ps/msg/msg.c          $(KERNEL_INCLUDE) $(CFLAGS) -o msg.o
	$(CC) -c ps/process/process.c  $(KERNEL_INCLUDE) $(CFLAGS) -o process.o
	$(CC) -c ps/process/plib.c     $(KERNEL_INCLUDE) $(CFLAGS) -o plib.o
	$(CC) -c ps/thread/thread.c    $(KERNEL_INCLUDE) $(CFLAGS) -o thread.o
	$(CC) -c ps/thread/tlib.c      $(KERNEL_INCLUDE) $(CFLAGS) -o tlib.o
	$(CC) -c ps/create.c           $(KERNEL_INCLUDE) $(CFLAGS) -o create.o
	$(CC) -c ps/clone.c            $(KERNEL_INCLUDE) $(CFLAGS) -o clone.o
# Entrance 
	$(CC) -c ps/intake/disp/ts.c              $(KERNEL_INCLUDE)  $(CFLAGS) -o ts.o  
	$(CC) -c ps/intake/disp/dispatch.c        $(KERNEL_INCLUDE)  $(CFLAGS) -o dispatch.o  
	$(CC) -c ps/intake/disp/spawn.c           $(KERNEL_INCLUDE)  $(CFLAGS) -o spawn.o  
	$(CC) -c ps/intake/disp/x86_64/x64cont.c  $(KERNEL_INCLUDE)  $(CFLAGS) -o x64cont.o  
	$(CC) -c ps/intake/sched/sched.c          $(KERNEL_INCLUDE)  $(CFLAGS) -o sched.o  
	$(CC) -c ps/intake/sched/schedi.c         $(KERNEL_INCLUDE)  $(CFLAGS) -o schedi.o  
	$(CC) -c ps/intake/sched/schedq.c         $(KERNEL_INCLUDE)  $(CFLAGS) -o schedq.o  

#--------
# x86_64/

	$(CC) -c x86_64/detect.c  $(KERNEL_INCLUDE) $(CFLAGS) -o detect.o

	#Interrupts
	$(CC) -c x86_64/inter/apic.c  $(KERNEL_INCLUDE) $(CFLAGS) -o apic.o
	#$(CC) -c x86_64/inter/ioapic.c  $(KERNEL_INCLUDE) $(CFLAGS) -o ioapic.o
	#$(CC) -c x86_64/inter/pic.c  $(KERNEL_INCLUDE) $(CFLAGS) -o pic.o

	$(CC) -c x86_64/pit.c      $(KERNEL_INCLUDE) $(CFLAGS) -o pit.o
	$(CC) -c x86_64/ports64.c  $(KERNEL_INCLUDE) $(CFLAGS) -o ports64.o
	$(CC) -c x86_64/rtc.c      $(KERNEL_INCLUDE) $(CFLAGS) -o rtc.o

	$(CC) -c x86_64/x64.c      $(KERNEL_INCLUDE) $(CFLAGS) -o x64.o
	$(CC) -c x86_64/x64info.c  $(KERNEL_INCLUDE) $(CFLAGS) -o x64info.o
	$(CC) -c x86_64/x64init.c  $(KERNEL_INCLUDE) $(CFLAGS) -o x64init.o
	$(CC) -c x86_64/x64mi.c    $(KERNEL_INCLUDE) $(CFLAGS) -o x64mi.o
	$(CC) -c x86_64/x64nmi.c   $(KERNEL_INCLUDE) $(CFLAGS) -o x64nmi.o
	$(CC) -c x86_64/x64smp.c   $(KERNEL_INCLUDE) $(CFLAGS) -o x64smp.o

#--------

	$(CC) -c queue.c     $(KERNEL_INCLUDE) $(CFLAGS) -o queue.o
	$(CC) -c ke.c        $(KERNEL_INCLUDE) $(CFLAGS) -o ke.o
	$(CC) -c hal.c       $(KERNEL_INCLUDE) $(CFLAGS) -o hal.o

# gre - Graphics Engine
	$(CC) -c gre/pixel.c   $(KERNEL_INCLUDE) $(CFLAGS) -o pixel.o
	$(CC) -c gre/line.c    $(KERNEL_INCLUDE) $(CFLAGS) -o line.o
	$(CC) -c gre/rect.c    $(KERNEL_INCLUDE) $(CFLAGS) -o rect.o
	$(CC) -c gre/bitblt.c  $(KERNEL_INCLUDE) $(CFLAGS) -o bitblt.o
	$(CC) -c gre/bg.c      $(KERNEL_INCLUDE) $(CFLAGS) -o bg.o
	$(CC) -c gre/font.c    $(KERNEL_INCLUDE) $(CFLAGS) -o font.o
	$(CC) -c gre/char.c    $(KERNEL_INCLUDE) $(CFLAGS) -o char.o
	$(CC) -c gre/text.c    $(KERNEL_INCLUDE) $(CFLAGS) -o text.o
	$(CC) -c gre/gre.c     $(KERNEL_INCLUDE) $(CFLAGS) -o gre.o

#
	$(CC) -c caliuser/input.c     $(KERNEL_INCLUDE) $(CFLAGS) -o input.o
	$(CC) -c caliuser/dispsrv.c   $(KERNEL_INCLUDE) $(CFLAGS) -o dispsrv.o
	$(CC) -c caliuser/callback.c  $(KERNEL_INCLUDE) $(CFLAGS) -o callback.o

# link
	$(LD) -r -o ar_ke.o $(OBJS)
	cp ar_ke.o ../
	rm *.o
	
clean:
	rm *.o

