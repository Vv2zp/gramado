
// config.h
// Config for the Gramado kernel only. Not the whole system.

#ifndef ____KERNEL_CONFIG_H
#define ____KERNEL_CONFIG_H    1


// Flags:
#define USE_SERIALDEBUG    0
#define USE_SMP            1
#define USE_E1000          1
// ...


// ----------------------------------------------------------------
// Disk configuration:
// ## IMPORTANTE ##
// TEM QUE CONFIGURAR O BL TAMBÉM
// Usaremos essa configuraçao provisoriamente
// ate que tenhamos a condiçao de selecionarmos corretamente
// o canal e o dispositivo.
// Inicializaremos essas variaveis ao inicializarmos
// o controlador de ata. ide. em ata.c ata_initialize.
// USE PRIMARY MASTER!!
// Portas bases encontradas nas BARs.
// BAR0 = base port 1F0  (channel 0 primary master) (channel 0 primary slave) 
// BAR1 = base port 3F6  channel ??
// BAR2 = base port 170  (channel 1 secondary master) (channel 1 secondary slave)
// BAR3 = base port 376  channel ??
// #importante
// master e slave é coisa do PATA
// então 3f6 pode ser canal 2 e 376 pode ser canal 3.

#define __BAR0  0
#define __BAR1  1
#define __BAR2  2
#define __BAR3  3
#define __CHANNEL0  __BAR0 
#define __CHANNEL1  __BAR2
#define __CHANNEL2  __BAR1
#define __CHANNEL3  __BAR3
// See: 
// ata_initialize in ata.c
#define __IDE_PORT    __CHANNEL0

// ----------------------------------------------------------------
// virtualbox Info:
// PIIX3 ATA: LUN#0: disk, PCHS=963/4/17, total number of sectors 65536
#define FRED_VHD_32MB
#if defined(FRED_VHD_32MB)
#define VHD_32MB_CHS { 963, 4, 17,  0 }  
//#define VHD_32MB_CHS { 862, 4, 19,  0 }  
//#define VHD_32MB_CHS { 0x35E, 0x4, 0x13, 0 }  
#else 
#error "must define vhd"
#endif
// ----------------------------------------------------------------

// #bugbug
// Ouve uma falha..
// Só temos o registro das portas 0 e 2.
// A porta 0 equivale ao canal primary.
// A porta 2 equivale ao canal secondary.
// Eram para as portas 0 e 1 representarem o canal primary.
// Eram para as portas 2 e 3 represerntarem o canal secondary.

// #importante:
// Na verdade só funcionam as portas 0 e 2 porque são 
// selecionadores das BARs 0 e 2, onde estão as portas de HD.
// #todo: rever o código nessa parte de configuração das BARs.

// A questão é que existem canais extras.
// Vamos presizar ler mais BARs?

/*
Current disk controller chips almost always support two ATA buses per chip. 
There is a standardized set of IO ports to control the disks on the buses. 
The first two buses are called the Primary and Secondary ATA bus, and are almost 
always controlled by IO ports 0x1F0 through 0x1F7, and 0x170 through 0x177, 
respectively (unless you change it). The associated 
Device Control Registers/Alternate Status ports are IO ports 0x3F6, and 0x376,
respectively. The standard IRQ for the Primary bus is IRQ14, and IRQ15 for the Secondary bus.

If the next two buses exist, they are normally controlled by IO ports 0x1E8 through 0x1EF, 
and 0x168 through 0x16F, respectively. The associated
Device Control Registers/Alternate Status ports are IO ports 0x3E6, and 0x366. 
*/

/*
This is the multiple IDE interface driver, as evolved from hd.c.
It supports up to 9 IDE interfaces per default, on one or 
more IRQs (usually 14 & 15). 
There can be up to two drives per interface, as per the ATA-6 spec.
Primary:    ide0, port 0x1f0; major=3;  hda is minor=0; hdb is minor=64
Secondary:  ide1, port 0x170; major=22; hdc is minor=0; hdd is minor=64
Tertiary:   ide2, port 0x1e8; major=33; hde is minor=0; hdf is minor=64
Quaternary: ide3, port 0x168; major=34; hdg is minor=0; hdh is minor=64
fifth..     ide4, usually PCI, probed
sixth..     ide5, usually PCI, probed
*/

// ----------------------------------------------------------------
// PIT configuration:

#define DEFAULT_JIFFY_FREQ    1000

// ----------------------------------------------------------------
// Runlevels
// Where are the types defines?
// See:
// core/init.c
// 5 Start the system normally with appropriate 
// display manager (with GUI) 
// Same as runlevel 3 + display manager.
// Full multi-user graphical mode. 
// #define DEFAULT_RUNLEVEL  0
#define DEFAULT_RUNLEVEL  5

/* configuração de compilação */

// Algumas adaptações precisam ser feitas 
// para que as ferramentas de compilação 
// sejam usadas corretamente.

// gcc version:
// gcc 8 on manjaro

// #importante:
// O kernel tem três possíveis processos usados na inicialização.
// #obs: cada um deles tem sua própria estrutura de processo e suas 
// próprias threads, mas até o momento todos os três estão usando 
// o diretório de páginas do processo kernel.  
// Com essas flags podemos habilitar qual dos três processos 
// irão rodar na inicialização.
// Se a configuração não indicar nenhum processo então o kernel deve 
// falhar na inicialização, por enquanto. 

//=====================================================
//    ######## Creation flags ########
//

//Cria o processo shell e sua thread de controle.
#define ENTRY_CREATE_SHELL   
//Cria o processo taskman e sua thread de controle.
#define ENTRY_CREATE_TASKMAN 
//Cria uma thread para o processo kernel, que irá rodar em ring0.
#define ENTRY_CREATE_KERNELTHREAD_RING0

//==========================================
// modes

//Se usaremos os processos do x server ou apenas o init.
#define ENTRY_GRAMADO_CORE  

//====================================================
//    ######## more ... ########
//

//Essa flag aciona a rotina que vai checar o máximo de 
//configurações e inicializações válidas.

#define ENTRY_DEBUG_CHECK_VALIDATIONS  1

//
// Set up what kind of debug message we wanna see.
//

// #bugbug
// Essas flags perderam o sentido
// precisamos renomear ou usarmos apenas a principal.

// Esse é usado para exibir somente os marcadores principais,
// que são mesnagens escritas em maiúculas.
// funciona como uma visão panorâmica, 
// uma espinha dorsal, um esqueleto.
#define KERNEL_VERBOSE  1

// Esse é usado para trabalhar a rotina de 
// entrypoint da arquitetura alvo.
#define ENTRY_VERBOSE  1

// Esses quatro são as camadas principais.
#define HAL_VERBOSE     1
#define PS_VERBOSE      1
#define EXECVE_VERBOSE  1
#define GWS_VERBOSE     1

// Permite que várias mensagens secundárias 
// sejam enviadas para a porta serial.
#define SERIAL_DEBUG_VERBOSE  1

//
// ## breack points ##
// Set up what what is the breakpoint.
//

// Seriam inicializações parciais programadas. 

//#todo
//Criar um breakpoint apo's a sondagem de dispositivos pci.

//#define BREAKPOINT_TARGET_AFTER_VIDEO 1
//#define BREAKPOINT_TARGET_AFTER_SYSTEM 1
//#define BREAKPOINT_TARGET_AFTER_RUNTIME 1
//#define BREAKPOINT_TARGET_AFTER_INIT 1
//#define BREAKPOINT_TARGET_AFTER_LOGON 1
//#define BREAKPOINT_TARGET_AFTER_LOGOFF 1
//#define BREAKPOINT_TARGET_AFTER_HAL 1
//#define BREAKPOINT_TARGET_AFTER_MK 1
//#define BREAKPOINT_TARGET_AFTER_ENTRY 1

//
// ## targets ##
//

// Também pretendo fazer a inicialização mudar de 
// direção dependendo do alvo programado.

// Isso inicializa os três aplicativos do gramado core.
//#define TARGET_GRAMADOCORE_APPS 1

// Isso inicializa apenas o app init do gramado core.
//#define TARGET_GRAMADOCORE_INIT 1

// See: fonts.h
// #bugbug: FONT8X8 is define bellow this definition.
//#define DEFAULT_FONT_SIZE  FONT8X8
//#define DEFAULT_FONT_SIZE FONT8X16
//...

// See: ps/quantum.h
#define CONFIG_QUANTUM_MULTIPLIER    1
//#define CONFIG_QUANTUM_MULTIPLIER    2
//#define CONFIG_QUANTUM_MULTIPLIER    3

#endif 


