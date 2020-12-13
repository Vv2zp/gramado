/*
 * File: i8042/i8042.c
 *     i8042 controller initialization.
 *     initialize ps2 controller.
 * env:
 *     Ring 0. Kernel base persistent code.
 * 2018 - Created by Fred Nora.
 */


 
// When the keyboard and mouse are USB devices, the BIOS uses SMM code 
// to emulate PS/2 devices. 
// I see mentioned that the USB devices should halt ps/2 emulation 
// once they've been initialized (or at least their host hub?)


#include <kernel.h>







// =======================
// prepare ..

void prepare_for_input (void)
{
    kbdc_wait(0);
}

void prepare_for_output (void)
{
    kbdc_wait (1);
}


// =======================
// wait then ...

unsigned char wait_then_read (int port)
{
    prepare_for_input();
    return (unsigned char) in8 (port);
}

void wait_then_write ( int port, int data )
{
    prepare_for_output();
    out8 ( port, data );
}



/*
 ***************
 * ps2:
 *     Inicializa o controlador ps2.
 * 
 *     Essa rotina de inicializa��o do controlador 
 * poder� ter seu pr�prio m�dulo.
 * 
 *     Inicializa a porta do teclado no controlador.
 *     Inicializa a porta do mouse no controlador.
 *     Obs: *importante: A ordem precisa ser respeitada.
 *     As vezes os dois n�o funcionam ao mesmo tempo se a 
 *     inicializa��o n�o for feita desse jeito. 
 */


// Essa eh uma inicializa�ao completa.

void ps2 (void)
{
	// #debug
    printf ("ps2: Initializing..\n");
    refresh_screen();

    // The main structure fisrt of all.
    // #todo: create ps_initialize_main_structure();
    PS2.used = 1;
    PS2.magic = 1234;
    PS2.pooling = FALSE;
    PS2.keyboard_initialized = FALSE;
    PS2.mouse_initialized = FALSE;

    // Zerando estruturas que precisam serem reinicializadas.
    
    //keyboard
    PS2KeyboardDeviceObject = NULL;
    PS2KeyboardDevice = NULL;
    PS2KeyboardDeviceTTYDriver = NULL;
    PS2KeyboardDeviceTTY = NULL;
    
    //mouse
    PS2MouseDeviceObject = NULL;
    PS2MouseDevice = NULL;
    PS2MouseDeviceTTYDriver = NULL;
    PS2MouseDeviceTTY = NULL;



    // ======================================
    // Deactivate ports!
    wait_then_write (0x64,0xAD);  // Disable keyboard port.
    wait_then_write (0x64,0xA7);  // Disable mouse port.

    // Keyboard.
    printf ("ps2: Initializing keyboard ..\n");
    refresh_screen();
    ps2kbd_initialize_device ();
    PS2.keyboard_initialized = TRUE;
        
    // Mouse.
    printf ("ps2: Initializing mouse ..\n");
    refresh_screen();
    ps2mouse_initialize_device ();
    PS2.mouse_initialized = TRUE;

    // ======================================
    // Reactivate ports!
    wait_then_write (0x64,0xAE);  // Reenavle keyboard port.
    wait_then_write (0x64,0xA8);  // Renable mouse port.

    // Wait for nothing!
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);

    // Done.

    //#debug
    printf ("ps2: done\n");
    refresh_screen();
}



/*
 ************************************************************
 * early_ps2_init:
 * 
 * 
 */

// Inicializa��o preliminar. Sem mouse.
// Ela existe porque a emula��o de ps2 na m�quina real 
// apresenta falhas.
// No gdeshell.bin incluiremos os comando "ps2-init"
// para obtermos a inicializa��o completa.

// #importante
// Nao chamamos a rotina de inicializa�ao do mouse.
// mas poderiamos. O importante eh deixar a porta desabilitada
// ao final da rotina.
// Ao fim dessa rotina, reabilitamos apenas a porta de teclado.
// A porta de mouse permane�e fechada.

void early_ps2_init (void)
{
    // mas simples...
    // apenas teclado.

	// #debug
	printf ("early_ps2_init: Initializing..\n");
	refresh_screen();

    // The main structure fisrt of all.
    // #todo: create ps_initialize_main_structure();
    PS2.used = 1;
    PS2.magic = 1234;
    PS2.pooling = FALSE;
    PS2.keyboard_initialized = FALSE;
    PS2.mouse_initialized = FALSE;


    // Zerando estruturas que precisam serem reinicializadas.
    
    //keyboard
    PS2KeyboardDeviceObject = NULL;
    PS2KeyboardDevice = NULL;
    PS2KeyboardDeviceTTYDriver = NULL;
    PS2KeyboardDeviceTTY = NULL;
    
    //mouse
    PS2MouseDeviceObject = NULL;
    PS2MouseDevice = NULL;
    PS2MouseDeviceTTYDriver = NULL;
    PS2MouseDeviceTTY = NULL;


    // ======================================
    // Deactivate ports!
    wait_then_write (0x64,0xAD);  //Disable keyboard port.
    wait_then_write (0x64,0xA7);  //Disable mouse port.


    // Keyboard.
    printf ("early_ps2_init: Initializing keyboard ..\n");
    refresh_screen();
    ps2kbd_initialize_device ();
    PS2.keyboard_initialized = TRUE;

    // Mouse.
    //printf ("ps2: Initializing mouse ..\n");
    //refresh_screen();
    //ps2mouse_initialize_device ();
    PS2.mouse_initialized = FALSE;


    // ======================================
    // Reactivate ports!
    wait_then_write (0x64,0xAE);    // Reenable keyboard port.
    //wait_then_write (0x64,0xA8);  // do NOT reenable the mouse port.


    // We can call this one.
    // wait_then_write (0x64,0xA7);  //Disable mouse port.


    // Wait for nothing!
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);
    kbdc_wait (1);

    // Done.

    //#debug
    printf ("early_ps2_init: done\n");
    refresh_screen();
}



// This is called by gdeshell.
int PS2_initialize(void)
{
	ps2();
	return 0;
}

// this is called during the kernel initialization.
int PS2_early_initialization(void)
{
	early_ps2_init();
	return 0;
}



// See:
// ps2.h
int ps2_ioctl ( int fd, unsigned long request, unsigned long arg )
{
    debug_print("ps2_ioctl: [TODO]\n");
    return -1;
}


//
// End.
//

