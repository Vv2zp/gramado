
// globals.c

#include <kernel.h>

//
// Product support
//

// Type of product.
int g_product_type = PT_UNDEFINED;
// Size of processor word.
int g_platform_type = PFT_64BIT;
// System type.
int g_system_type = SYSTEM_TYPE_NULL;

//
// Boot support
//

// #bugbug
// We are using another structure.
// see: init.c
// see: gdef.h
struct boot_block_d  BootBlock;
// This is the write structure for this.
//see: display.h, init.c, bldisp.c
struct x_boot_block_d  xBootBlock;


// Salvando o último endereço válido de memória ram.
// usado em head.asm
// see: x64mm.h
unsigned long blSavedLastValidAddress=0;  //pa
unsigned long blSavedPhysicalMemoryInKB=0;


//
// System arch
//

// x86_64
int current_arch = CURRENT_ARCH_X86_64;


//
// System mode
//

// GRAMADO_JAIL, GRAMADO_P1 ...
int current_mode=0;


//
// System state
//

// booting, rebooting ...
int system_state = SYSTEM_NULL;

// INPUT_MODE_SETUP, INPUT_MODE_WS ...
//int current_input_mode=0;

// Organização de usuários.
int current_user=0;
int current_group=0; 

// Current runlevel. Used in init process.
int current_runlevel=0;

// Organização dos discos.
int current_disk=0;
int current_volume=0;

// Organização dos arquivos.
int current_directory=0;
int current_file=0;

// Organização dos terminais
int current_tty=0;
int current_terminal=0;
//int current_pty=0;
//int current_pts=0;

int abnt2=FALSE;

// see: smp_probe() in x64.c
unsigned int g_processor_count=0;
int g_smp_initialized = FALSE;

// see: window.h
struct gui_d  *gui; 


// Logon.
int g_logged=FALSE;


// Kernel information variables.
unsigned long KernelSize=0;
unsigned long KernelHeapSize=0;
unsigned long KernelStackSize=0;
unsigned long KernelPages=0;
//...


int g_kernel_symbols_available=FALSE;

unsigned long g_ring0area_va=0;
unsigned long g_ring3area_va=0;
unsigned long g_kernelimage_va=0;
unsigned long g_frontbuffer_va=0;
unsigned long g_backbuffer_va=0;
unsigned long g_pagedpool_va=0;  //pagedpool virtual address
unsigned long g_heappool_va=0;
unsigned long g_extraheap1_va=0;
unsigned long g_extraheap2_va=0;
unsigned long g_extraheap3_va=0;


// Endereço virtual do pool de heaps.
// os heaps nessa área serão dados para os processos.
int g_heap_count=0;
int g_heap_count_max=0;
unsigned long g_heap_size=0;


// extra heap 1
unsigned long g_extraheap1_size=0;
int g_extraheap1_initialized=0;
// extra heap 2
unsigned long g_extraheap2_size=0;
int g_extraheap2_initialized=0;
// extra heap 3
unsigned long g_extraheap3_size=0;
int g_extraheap3_initialized=0;

//#bugbug
//move this to kernel (ring 0) crt support.
//error support.
unsigned long errno=0;


