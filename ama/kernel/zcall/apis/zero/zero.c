
// zero.c
// Wrappers for kernel initialization routines.

#include <kernel.h>

// As estruturas de console sao estruturas de tty,
// mas sao um array de estruturas, nao precisa de malloc,
// por isso podem ser chamadas nesse momento.
// #test
// We need to test it better.
// see:
// dev/chardev/tty/console.c
// crt/kstdio.c
void zero_initialize_virtual_consoles(void)
{
    VirtualConsole_initialize();
}

// See: 
// dev/chardev/display/bldisp/bldisp.c
void zero_initialize_video(void)
{
    //#breakpoint: BLACK ON WHITE.
    //ok, funcionou na maq real no modo jail, provavelmente 320x200.
    //for (i=0; i< 320*25; i++){ fb[i] = 0; };
    //while(1){asm("hlt");};

    Video_initialize();
    PROGRESS("zero_initialize_video: Initialized\n");
}

// see:
// dev/chardev/display/bg.c
void zero_initialize_background(void)
{
    displayInitializeBackground(COLOR_KERNEL_BACKGROUND,TRUE);
    PROGRESS("zero_initialize_background: ok\n");
}

// Setup Default kernel font.
// ROM BIOS 8x8 font.
// see: font.c
void zero_initialize_default_kernel_font(void)
{
    gwsInitializeDefaultKernelFont();
    PROGRESS("zero_initialize_default_kernel_font: ok\n");
}


// =================================
// Console:
// We have a virtual console and we can use the printf.
// This is the first message in the screen.
// see: tty/console.c
void zero_show_banner(void)
{
    char product_string[256];
    char build_string[256];
    size_t size=0;

    memset(product_string,0,256);
    memset(build_string,0,256);

// product string
    sprintf(product_string,PRODUCT_NAME);
    size = sizeof(PRODUCT_NAME);
    if (size >= 256)
        return;
    product_string[size+1]=0;

// build string
    sprintf(build_string,BUILD_STRING);
    size = sizeof(BUILD_STRING);
    if (size >= 256)
        return;
    build_string[size+1]=0;
    
// Crear screen and print version string.
    console_banner( product_string, build_string, 0 );
}



