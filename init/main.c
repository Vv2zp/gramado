/*
 * File: main.c 
 *
 *     The INIT.BIN process.
 * 
 * Environment:
 *     ring 3.
 * 
 * Purpose:
 *     + To call interrupt 129 to enable maskable interrupts.
 *     + Hang forever. Some new process will reuse this process.
 * 
 *
 *     Esse programa dever� ser chamado sempre que o sistema estiver ocioso,
 * ou com falta de op��es vi�veis. Ent�o esse programa deve ficar respons�vel 
 * por alguma rotina de manuten��o do equil�brio de sitema, ou por ger�ncia de 
 * energia, com o objetivo de poupar energia nesse momento de ociosidade.
 *
 * O processo idle pode solicitar que processos de gerencia de energia entrem em
 * atua��o. Pois a chamada do processo idle em si j� � um indicativo de ociosidade
 * do sistema. Pode-se tamb�m organizar bancos de dados, registros, mem�ria, buffer,
 * cache etc.
 *
 *     O sistema pode configurar o que esse processo faz quando a m�quina 
 * est� em idle mode. Quando n�o h� nenhum processo pra rodar ou a cpu se 
 * encontra ociosa, pode-se usar alguma rotina otimizada presente neste 
 * programa. Parece que a intel oferece sujest�es pra esse caso, n�o sei.
 * 
 * Obs: O entry point est� em head.s
 *      Agora idle pode fazer systemcalls. 
 *
 * #todo: 
 *     Criar argumento de entrada.
 *
 * History:
 *     2015 - Created by Fred Nora.
 *     2016 - Lil changes.
 *     2019 - The int 129 support.
 *     ...
 */
 
 
//
// Includes.
// 
 
#include "init.h"

/*
 Example:
ID 	Name 	Description
0 	Halt 	Shuts down the system.
1 	Single-user mode 	Mode for administrative tasks.[2][b]
2 	Multi-user mode 	Does not configure network interfaces and does not export networks services.[c]
3 	Multi-user mode with networking 	Starts the system normally.[1]
4 	Not used/user-definable 	For special purposes.
5 	Start the system normally with appropriate display manager (with GUI) 	Same as runlevel 3 + display manager.
6 	Reboot 	Reboots the system.  
 */

int __current_runlevel;

//
// Vari�veis internas.
//

//Idle application support.
int idleStatus;
int idleError;
//...

//
//  ## Status do servidor INIT ##
//

int ServerStatus;
//...


/*
struct idle
{
	struct thread_d *current_idle_thread;
	//int
}
*/

//
// Prot�tipos.
//





//...
void enable_maskable_interrupts();


//
// ==========
//


static inline void pause2 (void){
	
    asm volatile("pause" ::: "memory"); 
}; 


/* REP NOP (PAUSE) is a good thing to insert into busy-wait loops. */
static inline void rep_nop (void){
	
    asm volatile ("rep;nop": : :"memory");
};


#define cpu_relax()  rep_nop()


 


// interna
// Uma interrup��o para habilitar as interrup��es mascar�veis.
// S� depois disso a interrup��o de timer vai funcionar.

void enable_maskable_interrupts()
{
    asm ("int $129 \n");
}




/*
 ********************** 
 * main:
 * 
 */

// See: sw.asm in the kernel. 
// O objetivo � chamar a interrup��o 129.
// Uma interrup��o para habilitar as interrup��es mascar�veis.
// S� depois disso a interrup��o de timer vai funcionar.

int main ( int argc, char *argv[] ){

    char runlevel_string[128];
    char *_string = "init.bin: Init is alive! Calling int 129";


    // Initialize with error value.
    __current_runlevel = (int) -1; 
 

    //
    // Main loop
    //

    //++
    while (1){

    debug_print ("---------------------------\n");    
    debug_print ("init.bin: Initializing ...\n");

    // Using api.
    gde_draw_text ( NULL, 
        0, 0, COLOR_YELLOW, _string );

    gde_show_backbuffer ();

    // #debug
    // while(1){}


    //
    // Habilita as interrup��es mascaraveis.
    //
   
    
    // #DEBUG
    // Olhando eflags.
    // asm ("int $3 \n");
    
    enable_maskable_interrupts ();
    //asm ("int $129 \n");
    

    // #DEBUG
    // Olhando eflags.
    //asm ("int $3 \n");



    //
    // Runlevel
    // 
 
		// 0 	Halt 	
		// Shuts down the system. 
		// 1 	Single-user mode 	
		// Mode for administrative tasks.
		// 2 	Multi-user mode 	
		// Does not configure network interfaces and 
		// does not export networks services.
		// 3 	Multi-user mode with networking 	
		// Starts the system normally.
        // Full multi-user text mode.
		// 4 	Not used/user-definable 	
		// For special purposes.
		// 5 	Start the system normally with appropriate 
		// display manager (with GUI) 	
		// Same as runlevel 3 + display manager. 
		// Full multi-user graphical mode.
		// 6 	Reboot 	Reboots the system. 

    // Get the current runlevel.
    __current_runlevel = (int) gramado_system_call ( 288, 0, 0, 0 );  

    itoa (__current_runlevel, runlevel_string);

    printf ("The current runlevel is %s \n",runlevel_string);


    //
    // Initializing in the selected runlevel.
    //
    
    switch (__current_runlevel){


       //case ?:
           //gramado_system_call ( 900, (unsigned long) "gws.bin", 0, 0 ); 
           //break;


       //case ?:
           //break;

        //...
        
        // Usar esse por enquanto.
        default:
            gramado_system_call ( 900, (unsigned long) "gdeshell.bin", 0, 0 ); 
            break;
    };

    //
    // Hang (#debug)
    //

    // Yield This thread.
    while (1){
        asm ("pause");
        gramado_system_call (265,0,0,0); 
    };
    
    
    };
    // Main loop end.
    //--
}


//
// End.
//






//
// End.
//

