/*
 * File: init.c 
 *
 * Descri��o:
 *     Arquivo principal do m�dulo INIT kernel.
 *     Faz inicializa��es.
 *     Esse m�dulo est� em kernel base.
 *
 * In this file:
 *    +init
 *    +init_test_disk
 *    +init_video
 *    +init_globals
 *
 * Hist�rico:
 *     Vers�o 1.0, 2015 - Esse arquivo foi criado por Fred Nora.
 *     Vers�o 1.0, 2016 - Aprimoramento geral das rotinas b�sicas.
 *     //...
 */ 

 
#include <kernel.h>


//Onde ficam os c�digos e arquivos de configura��o usados na inicializa��o.
//A ideia � que se a inicializa��o precisar de algum arquivo, deve procur�-lo
//nos diret�rios previamente combinados.
//melhor que sejam poucos e que sempre sejam os mesmos.

#define INIT_ROOT_PATH  "/"
#define INIT_BIN_PATH   "/BIN"
//...


//externs
//@todo:
//isso dever� ser definido em /gramado.
//extern char *GramadoSystemRoot;
//extern char *GramadoMajorVersion;
//extern char *GramadoMinorVersion;
//extern char *GramadoVersionString;
//...


//Vari�veis internas.
//int initStatus;
//...


/*
void init_set_current_runlevel ( int number )
void init_set_current_runlevel ( int number )
{
	//#todo: limits.
    current_runlevel = number;
}
*/

/*
int init_get_current_runlevel (void)
int init_get_current_runlevel (void)
{
    return (int) current_runlevel;
}
*/


// Deprecated
void save_kernel_args (void)
{
    panic ("save_kernel_args: DEPRECATED");
}



/*
 ***********************************
 * init_architecture_dependent:
 *     Rotina de inicializa��o dependente da arquitetura atual da m�quina.
 *     A fase 1 identificou o tipo de processador. Aqui chamaremos a rotina 
 * de inicializa��o de acordo com o tipo identificado.
 *     A fase 1 inicializou a estrutura processor. Aqui checamos se ela � 
 * v�lida.
 *  GDT, IDT, ... TR ...
 *
 * Obs: Essa � a fase 2 de inicializa��o.
 *
 * Obs: Dependente significa dependente da marca do procesador.
 */

int init_architecture_dependent (void){

    int Status = 0;
    unsigned char Type=0;


    debug_print ("init_architecture_dependent:\n");

    //
    // Fase. 
    // (Verificar se essa rotina foi chamada na fase certa 
    // de inicializa��o.)
    //

    if ( KeInitPhase != 1 ){
        panic ("init_architecture_dependent: KeInitPhase\n");
    }


	// #### IMPORTANTE ####
	//
	// VAMOS ANTECIPAR ESSA INICIALIZA��O NA TENTATIVA DE
	// ANTECIPARMOS O USO DE MENSAGENS.
    // >>> mas essa rotina precisa do kmalloc ,,,
	//ent�o tem que ser depois da inicializa��o do stdio.
	
	
	// Os par�metros de tela dependem das propriedades de hardware
	// como monitor e placa de v�deo.
	
	//screenInit();

    //printf("init_architecture_dependent: #Debug");
    //refresh_screen();
    //while(1){};		
	

	//
	// A estrutura para informa��es sobre o processador. 
	//

	// Check structure.
    if ( (void *) processor == NULL ){
        panic("init_architecture_dependent: processor\n");
    }


    // Sonda pra ver qual � a marca do processador.
    // #todo: 
    // � a segunda vez que fazemos a sondagem ?!

    Type = (int) hal_probe_processor_type();

    if(Type==0)
        panic("init_architecture_dependent: Type\n");

    processor->Type = (int) Type;

    //
    // Inicializa de acordo com o tipo de processador.
    // Essas rotinas pegarao informacoes sobre o processador
    // e salvarao em estruturas
    //

    switch (Type)
    {

        case Processor_INTEL: 
            init_intel(); 
            break;

        case Processor_AMD: 
            init_amd(); 
            break;

        // ...


        default:
            panic ("init_architecture_dependent: default Type\n");
            break;
    };


	//
	// #todo: ?? maybe 
	// GDT, IDT (usar extern).
	//


	// #obs: O contexto � depedente.
	// Inicializando o Process manager.

    init_process_manager();


    //
    // Continua ...
    //

    // Done.

    // 0 = ok.
    return (int) Status;
}



/*
 ********************************************************
 * init_architecture_independent:
 *
 *    Rotina de inicializa��o da parte do sistema que � 
 * independente da arquitetura presente. 
 *    Independente da tipo de processador. 
 *    Obs: Essa � a fase 1 de inicializa��o.
 */

int init_architecture_independent (void){

    int Status=0;


    debug_print ("init_architecture_independent\n");


    // #important
    // We need to be in the phase 0.
    
    if (KeInitPhase != 0){
        panic ("init_architecture_independent: KeInitPhase\n");
    }


	//
	// == hal ===================================
	//


	// #bugbug
	// Se � HAL � dependente da arquitetura.
	// Isso deveria ficar na outra rotina.
	// N�o mudaremos por enquanto.

//#todo: Mudar o nome EXECVE_VERBOSE
#ifdef EXECVE_VERBOSE
    printf ("init_architecture_independent: Initializing HAL..\n");
#endif

    Status = init_hal();

    if (Status != 0){
        panic ("init_architecture_independent: init_hal fail\n");
    }

    //
    //  == mk =================================================
    //

    // mm, ipc, ps ...


	// Microkernel:
#ifdef EXECVE_VERBOSE
	// Obs: O Microkernel lida com informa��es dependentes da arquitetura,
	// por�m inicializa a gerencia de processos e threads e de comunica��o
	//entre processos.
	//#bugbug @todo: Se � microkernel � processo � registrador ... acho que leva em considera��o a arquitetura.
	printf ("init_architecture_independent: Initializing Microkernel..\n");
#endif


    Status = init_microkernel();

    if (Status != 0){
        panic ("init_architecture_independent: init_microkernel fail\n");
    }


	//
	// == executive ===============================================
	//


    // Executive:
#ifdef EXECVE_VERBOSE
	// Obs: O executive n�o � t�o dependente da arquitetura, ele �
	//uma camada mais alta, por�m ser� inicializado aqui para
	//efeito de ordem, j� que estamos inicializando os tres m�dulos
	//b�sicos do kernel base nesse momento.
    printf ("init_architecture_independent: Initializing Executive..\n");
#endif


    Status = init_executive();

    if (Status != 0){
        panic ("init_architecture_independent: init_executive\n"); 
    }



	// Gramado:
#ifdef EXECVE_VERBOSE
    printf ("init_architecture_independent: Initializing Gramado..\n");
#endif

   
    // #bugbug
    // Deprecated?

    Status = init_gramado();

    if (Status != 0){
        panic ("init_architecture_independent: init_gramado fail\n"); 
    }

    //
    // == window manager ================================
    //

    // #important:
    // This is the window manager embedded in the base kernel.

    // Text mode not supported.
    
    if (g_useGUI != 1){
        panic ("init_architecture_independent: [PANIC] No ring0 GUI!\n");
    }


#ifdef EXECVE_VERBOSE
    printf ("init_architecture_independent: init_window_manager\n");
#endif

    init_window_manager();


    // More? ...


//done:

#ifdef EXECVE_VERBOSE
    printf ("init_architecture_independent: Done\n");
    //refresh_screen();
    //while(1){}
#endif

    // 0=ok.
    return 0;
}


/*
 **************************************************
 * init_globals:
 *     Init globals. 
 *     Inicia vari�veis globais do Kernel Base.
 *     Obs: Inicializar por categorias.
 */

void init_globals (void){


    int i=0;


#ifdef EXECVE_VERBOSE
    debug_print("init_globals:\n");
    printf     ("init_globals:\n");
#endif


    //Outros.
    errno = 0;

    // Inicializa as estruturas do fluxo padr�o.
    // Isso vai usar a file table.
    // #todo: Mudar para kstdioInitialize.
    
    stdioInitialize();


    //
    // == Now we can print strings in the screen ==================
    //

    screenInit();

    //
    // == First message ====================================
    //

    // #todo
    // Maybe we can print the banner.


    debug_print("init_globals: WE HAVE MESSAGES NOW!\n");
    printf     ("init_globals: WE HAVE MESSAGES NOW!\n");


	//
	// Runlevel
	//
	
	// 5 	Start the system normally with appropriate 
	// display manager (with GUI) 	
	// Same as runlevel 3 + display manager.
	// Full multi-user graphical mode. 

    current_runlevel = 5;

    //===================	
	//vamos atrasar configura��o de janela em favor de configura��o de mensagem

	//Atalho para a pr�xima mensagem de teclado..(test) debug
    gNextKeyboardMessage = (int) 0;
	
	//Essa flag bloqueia e impede que uma janela obtenha o foco.
    gFocusBlocked = (int) 0;

	
	//
	// ==== Profiler ====
	//
	
	// See: include/kernel/profiler/pints.h
	
	// Interrup��o para servi�os do sistema.
	g_profiler_ints_gde_services = 0;

    // Legacy hardware interrupts (irqs) (legacy pic)
    g_profiler_ints_irq0 = 0;  
    g_profiler_ints_irq1 = 0;  
    g_profiler_ints_irq2 = 0;  
    g_profiler_ints_irq3 = 0;  
    g_profiler_ints_irq4 = 0;  
    g_profiler_ints_irq5 = 0;  
    g_profiler_ints_irq6 = 0;  
    g_profiler_ints_irq7 = 0;  
    g_profiler_ints_irq8 = 0;   
    g_profiler_ints_irq9 = 0;   
    g_profiler_ints_irq10 = 0;  
    g_profiler_ints_irq11 = 0;  
    g_profiler_ints_irq12 = 0;  
    g_profiler_ints_irq13 = 0;  
    g_profiler_ints_irq14 = 0;  
    g_profiler_ints_irq15 = 0;  
    // ...


	//
	// As globais relativas � usu�rio s�o independentes do 
	// ambiente gr�fico.
	//

	//User and group.

    current_user  = 0;
    current_group = 0;

    // Security layers.
    // user session, room(window station), desktop.
    current_usersession = (int) 0;
    current_room        = (int) 0;
    current_desktop     = (int) 0;

    //Process, Thread.
    current_process = (int) 0;
    current_thread  = (int) 0;


    kernel_request = 0;


    if ( g_useGUI != 1 ){
        panic("init_globals: No GUI");
    }

    // Window support.
    current_window    = (int) 0;  // Current Window.
    windows_count     = (int) 0;  // Window count.
    window_with_focus = (int) 0;  // Window with focus.
    current_menu      = (int) 0;  // Current Menu.
    // Continua ...

    // windows
    init_windows();
        
    // #todo: Delete?
    init_menus();

    //Continua...


    // Messages.
    g_new_message = 0;


    // FS type.
    // type 1, fat16.
    
    g_currentvolume_filesystem_type = FS_TYPE_FAT16;    
    g_currentvolume_fatbits = (int) 16;



	//===================
	
	// #importante:
	// provavelmente isso altere o comportamento do cursor,
	// levando o cursor par ao in�cio da tela. Ent�o precisamos
	// repintar o background para recome�armos.
	
	//printf("#breakpoint glob");
    //refresh_screen(); 
    //while(1){}	
	
	//#bugbug isso esta dando problemas.
#ifdef EXECVE_VERBOSE
    backgroundDraw ( (unsigned long) COLOR_BLUE ); 
#endif


    //printf("#breakpoint glob");
    //refresh_screen(); 
    //while(1){}

	//
	// Keyboard support.
	//
	
	//ldisc_init_modifier_keys();
	//ldisc_init_lock_keys();	


    //Continua ...

}


/*
 ******************************************
 * init:
 *     Base initializations.
 *     (Four phases).
 *     Isso foi chamado por systemStartUp() em core/system.c
 *   
 *    #todo: 
 *    Descrever aqui as fazer dessa rotina.
 */ 

// It was called by systemStartUp() in core/system.c

int init (void){

    int Status = 0;


    debug_print ("==== core-init:\n");

    // Check kernel phase.

    if ( KeInitPhase != 0 )
    {
        debug_print ("core-init: KeInitPhase fail\n");
        panic       ("core-init: KeInitPhase fail\n");
    }


	//
	// #IMPORTANT
	//

    // Globals.
    debug_print ("core-init: Globals\n");
    init_globals ();


	// #importante
	// � partir daqui podemos exibir strings na tela.


	
	
	
#ifdef EXECVE_VERBOSE
    printf ("core-init: init_globals ok\n");     
#endif  

	
	//#bugbug:
	//Depois de iniciar as globais, provavelmente o cursor mude 
	//para o in�cio da tela.
	//ent�o ap�s iniciarmos as globais temos que repintar o background e 
	// fazer um refresh da tela se a flag de verbose estiver ligada.
	
    //Object manager.	
#ifdef EXECVE_VERBOSE	
	printf ("core-init: init_object_manager\n");
#endif


    debug_print ("core-init: Object manager\n");
    init_object_manager ();


    //i/o Manager.
#ifdef EXECVE_VERBOSE	
	printf("core-init: ioInit\n");	
#endif	


    debug_print ("core-init: io manager\n");
    ioInit ();

    // Inicializa o gerenciamento de dispositivos.
    // Inicializa a lista de dispositivos.
    debug_print ("core-init: device manager\n");
    
    init_device_manager ();

    //
    // =================== ## STORAGE ## ===========================
    //

    // #ordem:
    // +storage
    // +disk
    // +volume 
    // +file system

	//#importante 
	//A estrutura 'storage' vai ser o n�vel mais baixo.
	//� nela que as outras partes devem se basear.
    debug_print ("core-init: storage structure\n");
    
    storage = (void *) kmalloc ( sizeof(struct storage_d) );

    if ( (void *) storage == NULL ){
       panic ("core-init: storage");
    }


#ifdef EXECVE_VERBOSE
    printf ("core-init: disk_init\n");
#endif  

    debug_print ("core-init: disk\n");
    disk_init ();


#ifdef EXECVE_VERBOSE
    printf("core-init: volume_init\n");
#endif

    debug_print ("core-init: volume\n");
    volume_init ();


	
#ifdef EXECVE_VERBOSE	
	printf ("core-init: VFS..\n");
#endif

    debug_print ("core-init: vfs\n");
    vfsInit ();


//deletar
#ifdef EXECVE_VERBOSE	
    printf ("core-init: fsInit\n");
#endif   

    debug_print ("core-init: fs\n");
    fsInit ();


    // #todo
    // Mount root fs.
    // See: mountedList[] in sci/fs/fs.c
    // #test: Vamos tentar montar o volume root em mountedList[0].
    debug_print ("core-init: [FIXME] Initialize mounted list in fs.c\n");

    // Inicializa a lista.
    fs_initialize_mounted_list ();
        

#ifdef EXECVE_VERBOSE
    printf("core-init: initialize_system_message_queue\n");
#endif

    initialize_system_message_queue (); 
    
    
	//
	// Network
	//

    debug_print ("core-init: network\n");
    networkInit ();


    //
    // Initialize Platform structure.
    //

#ifdef EXECVE_VERBOSE
    printf ("core-init: Platform\n");
#endif

    // #important
    // This is the Root struct. :)
    debug_print ("core-init: Platform struct\n");
    
    Platform = (void *) kmalloc ( sizeof(struct platform_d) );

    if ( (void *) Platform ==  NULL ){
        panic ("core-init: Platform\n");

    }else{

        // UP or MP.
        Platform->system_type = SYSTEM_TYPE_NULL;

        // Hardware
        Hardware = (void *) kmalloc ( sizeof(struct hardware_d) );

        if ( (void *) Hardware ==  NULL ){
            panic ("core-init: Hardware\n");

        }else{
            Platform->Hardware = (void *) Hardware;
            //printf(".");			
        };

		//Firmware
        Firmware = (void *) kmalloc ( sizeof(struct firmware_d) );

        if ((void *) Firmware ==  NULL ){
            panic ("core-init: Firmware\n");

        }else{
            Platform->Firmware = (void *) Firmware;
            //printf(".");  			
        };

		
		//System (software)

		//
		// #IMPORTATE: 
		// Aqui estamos inicializando a estrutura do systema.
		//

        System = (void *) kmalloc ( sizeof(struct system_d) );

        if ( (void *) System ==  NULL ){
            panic ("core-init: System\n");

        }else{

            System->used = 1;    //Sinaliza que a estrutura esta em uso.
            System->magic = 1234; //sinaliza que a estrutura n�o esta corrompida.
            Platform->System = (void *) System;
            //printf(".");
        };

		//printf(" Done!\n");	
		//...
    };


    Status = (int) init_architecture_independent ();
    if (Status != 0){
        panic ("core-init: init_architecture_independent fail\n"); 
    }

    //
    // == phase 1 ? ================================================
    //

    printf("=========================\n");
    printf("core-init: end of phase 0\n");
    refresh_screen();
    //while(1){}

    KeInitPhase = 1;


	//Fase 2: Inicia a parte de arquitetura especifica da m�quina atual.
	//        Ou seja, considera a marca do processador.
    Status = (int) init_architecture_dependent ();
    if (Status != 0){
        panic ("core-init: init_architecture_dependent fail\n"); 
    }



    // #importante
    // S� podemos carregar o diret�rio raiz depois que 
    // inicializamos o controlador de IDE e as estruturas de 
    // sistema de arquivos.
    // #todo: Precisamos inicializar a estrutura de diretorio
    // usada pelo diretorio raiz, para colocarmos o ponteiro
    // na estrutura de processos, juntamente com o ponteiro
    // para o cwd. 
    // Talvez essa seja a hora de criarmos a estrutura para
    // o diretorio raiz, mas ainda nao temos estrutura de processo.

    debug_print ("core-init: load root dir.\n");
    fs_load_rootdir ();


	// Disable interrupts, lock task switch and scheduler.
	
	asm ("cli");
	set_task_status(LOCKED); 
	scheduler_lock();

    // #todo
    // Talvez devamos antecipar isso, pois faz parte do teclado.
    
    ldisc_init_modifier_keys ();
    ldisc_init_lock_keys ();


    //
    // == phase 2 ? ================================================
    //

    printf("=========================\n");
    printf("core-init: end of phase 1\n");
    refresh_screen();
    //while(1){}

    KeInitPhase = 2;


    //#debug
    // printf ("init: *breakpoint\n"); 
    // refresh_screen();
    // while(1){}	

	//
	//  ==============  #### LOGON #### ===============
	//

    debug_print ("core-init: logon\n");

	
	//printf("#breakpoint before logon");
	//refresh_screen();
	//while(1){}	
	
	//
	// Logon.
	//
	
	//Fase3: Logon. 
//Logon:

    //
    // Logon. 
    // Cria Background, main window, navigation bar.
    // de acordo com predefini��o.
    //

    // See:
    // windows/logon.c


    if ( g_useGUI != 1 )
        panic("core-init: NO GUI");


    printf ("core-init: calling init_logon_manager ...\n");
    init_logon_manager();

    //
    // == phase 3 ? ================================================
    //

    printf("=========================\n");
    printf("core-init: end of phase 2\n");
    refresh_screen();
    //while(1){}

    KeInitPhase = 3; 


    //#debug
    //printf ("init: *breakpoint :) \n"); 
    //refresh_screen();
    //while(1){}


	// Continua ...
	

// * Fase 3:
// *     classe system.device.unblocked.
// *	   @todo: Inicializar dispositivos LPC/super io.
// *            Keyboard, mouse, TPM, parallel port, serial port, FDC. 
// *
 //* Fase 4:
 //*     classe system.device.unblocked.
 //*     @todo: Dispositivos PCI, ACPI ...
 //*
	
	
// Done.
//done:
    //printf("Done!\n");	
	//refresh_screen();
	//@todo: Deve retornar a vari�vel Status.


#ifdef BREAKPOINT_TARGET_AFTER_INIT
    //#debug 
	//a primeira mensagem s� aparece ap�s a inicializa��o da runtime.
	//por isso n�o deu pra limpar a tela antes.
    printf("core-init: debug breakpoint after init");
    refresh_screen(); 
    while (1){ 
        asm ("cli");
        asm ("hlt"); 
    }
#endif

    debug_print ("==== init: done\n");
    printf      ("==== init: done\n");



    // #debug
    //printf("*breakpoint\n");
    //refresh_screen();
    //while(1){}

    return 0;  
}


//
// End.
//
