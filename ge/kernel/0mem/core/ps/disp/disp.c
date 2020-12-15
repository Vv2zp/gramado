/*
 * File: ps/disp/dispatch.c
 *
 * Descri��o:
 *     Arquivo principal do dispatcher do kernel.
 *     Part of the dispatcher module.
 *     Coloca tarefas pra rodar que estejam em suas 
 * respectivas filas.
 *     Alterna entre as filas de acordo com o tipo de dispatcher.
 * As op��es s�o:
 *     +System dispatcher. 
 *     +Idle dispatcher.
 *     +Periodic dispatcher.
 *     +round robin dispatcher.
 *     +realtime dispatcher.
 *
 * Observa��o:
 *    Somente um tipo de dispatcher est� ativo no momento.
 *
 * History:
 *     2015 - Created by Fred Nora.
 *     2018 - Revision.
 *     //...
 */


#include <kernel.h>


//
// Vari�veis internas.
//

//int dispatchStatus;
//int dispatchError;
//...


//
// #todo:
// Prot�tipo de fun��es internas.
//

int syscoopDispatcher (void);
int usercoopDispatcher (void);
int sysconcDispatcher (void);
int userconcDispatcher (void);
int systemDispatcher (void);
int readyDispatcher (void);
int idleDispatcher (void);
int periodicDispatcher (void);
int rrDispatcher (void);
int realtimeDispatcher (void);
int dispatch_Default (void);



/*
 *********************************************************
 * dispatcher:
 *     Despacha a thread atual que foi escolhida pelo scheduler. 
 *     Despacha, colocando ela no estado RUNNING e restaurando os valores 
 * dos registradores.
 *     Essa fun��o � chamada no final da rotina task_switch, antes dela 
 * retornar.
 *     Obs: 
 *     Despacha a tarefa de acordo dom o tipo de dispatcher.
 *     Por�m os tipos diferentes de dispacher ainda n�o est�o habilitados, 
 * s� um funciona.
 */

// #todo
// Change the return type and return with error
// if something goes wrong. So this way we can try another thing.
// int dispatcher ( int type ){

void dispatcher ( int type ){

    struct thread_d *dispatch_Pointer;


	// Obs: 
	// @todo:
	// (Fase de teste). 
	// Usando apenas um tipo de dispatcher. 
	// Deteminando o tipo de dispacher por enquanto

    if ( type != DISPATCHER_CURRENT )
    {
        //type = DISPATCHER_CURRENT;
        panic("dispatcher: type\n");
    }


    //dispatch the current_thread.

//    goto do_dispatch;
//do_dispatch:


	// Checa estrutura.

    dispatch_Pointer = (void *) threadList[current_thread];

    if ( (void *) dispatch_Pointer == NULL ){
        panic ("dispatcher: struct\n");
    }

	// Checa o 'state'.

    if ( dispatch_Pointer->state != READY ){
        panic ("dispatcher: State ERROR\n");
    }


	// #importante
	// * MOVEMENT 4 (Ready --> Running).
	// A thread passa para o estado RUNNING.
	// Reinicia a contagem.


    if ( dispatch_Pointer->state == READY )
    {
        dispatch_Pointer->state = RUNNING;
        dispatch_Pointer->runningCount = 0;

        queue_insert_data ( queue, 
            (unsigned long) dispatch_Pointer, QUEUE_RUNNING );
    }


	//
	// ## RESTORE CONTEXT ##
	//

	// #importante
	// Flag sinalizando que o contexto n�o est� mais salvo.
	// Esse flag � acionada quando o contexto � salvo no in�cio 
	// da task switch.

    dispatch_Pointer->saved = 0;

	// #importante
	// Chama a rotina que colocar� as informa��es da estrutura de thread 
	// nas vari�veis usadas pelo assembly para configurar os registradores 
	// antes do iretd.

    restore_current_context();

// #todo
// Change the return type and return with error
// if something goes wrong. So this way we can try another thing.
    //return 0;

    return;
}


// deprecated!
int readyDispatcher (void)
{
	struct thread_d *dispatch_Pointer;
	
    dispatch_Pointer = (void*) queue_get_data( queue, QUEUE_READY);
	
	if( (void*) dispatch_Pointer != NULL && 
	            dispatch_Pointer->used == 1 && 
				dispatch_Pointer->magic == 1234)
	{
        dispatch_Pointer->quantum = 0;
        goto done;
	}
	else
	{
	    dispatch_Pointer = (void*) threadList[0];
		dispatch_Pointer->quantum = 0;
        goto done; 
	};
	
	//
	// @todo: Check.
	//
	
	/*
	if( (void*) dispatch_Pointer == NULL )
	{
	    //todo
	};
    */
	
//Done.
done:
	current_thread = (int) dispatch_Pointer->tid;
	return (int) current_thread;
}



// deprecated!
int syscoopDispatcher (void)
{
	struct thread_d *dispatch_Pointer;
	
    dispatch_Pointer = (void*) queue_get_data( queue, QUEUE_SYSCOOP);
	
	if( (void*) dispatch_Pointer != NULL && 
	            dispatch_Pointer->used == 1 && 
				dispatch_Pointer->magic == 1234)
	{
        dispatch_Pointer->waitingCount++;	
		if(dispatch_Pointer->waitingCount > dispatch_Pointer->waiting_limit)
		{
		    dispatch_Pointer->waitingCount = 0;
			current_thread = (int) dispatch_Pointer->tid;
		    return (int) dispatch_Pointer->tid;
        }	
		
	}
	else
	{
	    current_thread = 0;
	    return (int) current_thread;
	};
	
fail:
    //todo: hang
    return (int) current_thread;
}



// deprecated!
int usercoopDispatcher (void)
{
	struct thread_d *dispatch_Pointer;
	
    dispatch_Pointer = (void*) queue_get_data( queue, QUEUE_USERCOOP);
	
	if( (void*) dispatch_Pointer != NULL && 
	            dispatch_Pointer->used == 1 && 
				dispatch_Pointer->magic == 1234)
	{
        dispatch_Pointer->waitingCount++;	
		if(dispatch_Pointer->waitingCount > dispatch_Pointer->waiting_limit)
		{
		    dispatch_Pointer->waitingCount = 0;
			current_thread = dispatch_Pointer->tid;
		    return (int) dispatch_Pointer->tid;
        }	
		
	}
	else
	{
	    current_thread = 0;
	    return (int) current_thread;
	};

fail:
    //todo: hang
    return (int) current_thread;
}


// deprecated!
int sysconcDispatcher (void)
{
	struct thread_d *dispatch_Pointer;
	
    dispatch_Pointer = (void*) queue_get_data( queue, QUEUE_SYSCONC);
	
	if( (void*) dispatch_Pointer != NULL && 
	            dispatch_Pointer->used == 1 && 
				dispatch_Pointer->magic == 1234)
	{
        dispatch_Pointer->waitingCount++;	
		if(dispatch_Pointer->waitingCount > dispatch_Pointer->waiting_limit)
		{
		    dispatch_Pointer->waitingCount = 0;
			current_thread = dispatch_Pointer->tid;
		    return (int) dispatch_Pointer->tid;
        }	
		
	}
	else
	{
	    current_thread = 0;
	    return (int) current_thread;
	};

fail:
    //todo: hang
    return (int) current_thread;
}


// deprecated!
int userconcDispatcher (void)
{
	struct thread_d *dispatch_Pointer;
	
    dispatch_Pointer = (void*) queue_get_data( queue, QUEUE_USERCONC);
	
	if( (void*) dispatch_Pointer != NULL && 
	            dispatch_Pointer->used == 1 && 
				dispatch_Pointer->magic == 1234)
	{
        dispatch_Pointer->waitingCount++;	
		if(dispatch_Pointer->waitingCount > dispatch_Pointer->waiting_limit)
		{
		    dispatch_Pointer->waitingCount = 0;
			current_thread = dispatch_Pointer->tid;
		    return (int) dispatch_Pointer->tid;
        }	
		
	}
	else
	{
	    current_thread = 0;
	    return (int) current_thread;
	};
	
fail:
    //todo: hang
    return (int) current_thread;
}


// deprecated!
int systemDispatcher (void)
{ 
    return 0; 
}


// deprecated!
int idleDispatcher (void)
{ 
    //current_idle_thread

    return 0; 
}


// deprecated!
int periodicDispatcher (void)
{ 
    // struct thread_d *New;
   
    return 0; 
}


// deprecated!
int rrDispatcher (void)
{ 
    return 0; 
}


// deprecated!
int realtimeDispatcher (void)
{
	
/*
 	
    struct thread_d *New;
	  
    //Real time status ~ Round robin.     
    if(realtimeStatus == 1)
    { 
        New = (void*) queue_get_data( queue, QUEUE_REALTIME);
	    if( (void*) New != NULL && 
	                New->used == 1 && 
				    New->magic == 1234 && 
				    New->type == TYPE_REALTIME &&
				    New->priority == PRIORITY_MAX )
	    {   
	        return (int) New->tid;
	    };  
    }; 
	
//
// Fail, use tID 0.
//

*/

fail:
    return 0; //idle.
}


// deprecated!
int dispatch_Default (void)
{
    struct thread_d *New;
    struct thread_d *Current;
    int qNext=0;

	Current = NULL;
	
	
	/*
	 * Fase1 - Tenta tarefa de quantum vencido.
	 */

    qNext = (int) check_quantum();
    if(qNext != 0)
	{
	    New = (void *) threadList[qNext];
	    
		// todo: check structure.
		
		New->next = (void *) Current;
		New->quantum = 0;
	    
		return (int) qNext;
    }


	
	/*
	 * Fase 2: 
	 *     Preempt, gethigher priority, no return.
	 *     + Coloca em preemp��o a tarefa atual se ela for de baixa prioridade.
	 *     + Pega uma tarefa de prioridade alta nos slots e torna ela READY,
	 *       colocando ela no fim da fila de tarefas READY. 
	 *     + N�o retorna.
	 */
	
	
	/*
	 * Checar se a tarefa atual pode entrar em preemp��o. 
	 * Se ela n�o pode entrar em premmp��o n�o faz nada. 
	 * ent�o continua no estado READY, mas vai para o fim da fila das 
	 * tarefas ready. 
	 * obs: a tarefa tem que estar no estado ready.
	 */
	
	Current = (void *) threadList[current_thread]; 
	
	if ( Current->preempted == PREEMPTABLE && 
	     Current->priority == PRIORITY_LOW )
	{
	    //preempt ..fifo
	    //S� pode entrar se o estado � ready.
        if( Current->state == READY)
		{
		    
			//manda para o fim da fila de ready.
		    //Current->event = EVENT_PREMMPTED; //Motivo.   
			
			Current->waitingCount = 0;             //Zera o tempo de espera.
			queue_insert_data(queue, (unsigned long) Current, QUEUE_SYSCOOP);
		};
	};
	

	/*
	 * Pega nos slots uma tarefa de prioridade mais alta.
	 * Executa imediatamente se ela for ready. 
	 * mas antes coloca no fim da fila uma de menor prioridade.
	 * e coloca no fim da fila de ready.
	 */
	int nextId=0; 
	nextId = find_higher_priority();
	if( nextId > 0 )
	{
	    New = (void *) threadList[nextId]; 
		if( (void*) New == NULL ){
		    panic ("dispatch_Default: findhigerpriority return null");
		}
		//inicio da fila, lifo
		if( New->state == READY)
		{
		    New->waitingCount = New->waiting_limit;    //Tempo limite de espera.
			if( New->tid != 0)
			{
			    New->quantum = 0;
				New->next = (void *) Current;
			    return (int) New->tid;
			}
		}
	}

	
	
	/*
	 * Fase 3: Pega a pr�xima se houver.
	 */
	
	//se existe uma proxima tarefa. pega ela.
	//se ela estiver no estado ready o scheduler retorna.
	//se estiver no estado initialized, da spawn.
	if( (void*) Current->next != NULL )
    {
        New = (void*) Current->next;
        Current->next = NULL; //zera o indicador.
		
		if(New->state == READY)
		{
		    New->waitingCount = 0;
			if(New->tid != 0)
			{
			    New->quantum = 0;
			    New->next = (void *) Current;
				return (int) New->tid;
            }
		}
    }

	 
	/*
	 * Fase 4: Pega a idle.
	 */ 
	 
	New = (void *) threadList[0];
	
	if ( (void *) New != NULL )
	{
	    New->quantum = 0;
		New->next = (void *) Current;
		
		return (int) New->tid;
    }
    
fail:
     //#todo: test this return.
     panic ("dispatch_Default: return not tested");
     return -1;
}



/*
 * dispatch_thread2:
 *     Restaura o contexto e retorna.
 *     #bugbug. Nem sei se essa rotina est� em uso.
 */
 
void dispatch_thread2 (void){
	
    struct thread_d *t;  
	
	//
	// *RESTORE CONTEXT. (current_thread)
	//
	
	restore_current_context();
	
	//
	// Struct.
	//
	
	t = (void *) threadList[current_thread]; 
	
	if ( (void *) t == NULL )
	{
		//deveria ser a current_idle_thread
	    current_thread = (int) 0;  //current_idle_thread
	    return;
		
	}else{
	    t->saved = 0;         //N�o est� mais salvo o contexto.
	    t->state = RUNNING;   //Movimento 2 ou 4. ??
	};
}



/*
 *****************************************************
 * dispatch_thread:
 *     #bugbug. Nem sei se essa rotina est� em uso.
 */
 
void dispatch_thread (struct thread_d *thread){

    int Status = 0;

	//
	// Structure.
	//

    if ( (void *) thread == NULL )
    {
        printf ("dispatch-dispatch_thread: thread tid={%d}", current_thread ); 
        die ();

    }else{

	    // Context.
		// #bugbug: N�o estamos mais usando esse filtro
        // que seleciona apenas threads em ring 3.		
	    //Status = contextCheckThreadRing3Context(thread->tid);
	    //if(Status == 1){
	    //    printf("pc-dispatch-dispatch_task: contextCheckThreadRing3Context\n");
		//    die();
	    //};
	    //...
	};

	
	/*
	 * State:
	 *     Dispacha de acordo com o status.
	 *     +Spawn no caso de INITIALIZED.
	 */
 
    switch (thread->state)
    {
	    //Se vai rodar pela primeira vez
		case INITIALIZED:
		    thread->saved = 0;
			KiSpawnTask(current_thread);
			//spawn_task(current_thread);
	        break;
		//Continua ?? ...	
		
        //Nothing for now.
		default:
            printf("dispatch_thread fail: State!\n");
		    break;
    };


fail:
    panic ("dispatch-dispatch_thread: fail");
}


/*
 *************************************
 * init_dispatcher:
 *     inicializa o dispacher.
 *
 * dispatcherReadyList
 *     Esta � a lista do dispatcher.
 *
 * *IMPORTANTE:
 *  As threads aqui est�o no estado READY e ordenadas 
 * por prioridade.
 *     
 *  +A �ltima thread da lista � a thread de maior prioridade.
 *  +Cada elemento dessa lista � o elemento que est� na HEAD 
 * de uma lista
 *  +Cada uma dessas lista � uma lista para uma prioridade 
 * espec�fica.
 *  +Apenas a head de cada uma das listas � colocada aqui 
 * nessa lista do dispacher.
 *
 * OBS: 
 * #bugbug
 * Nesse momento, essa lista do dispacher n�o est� em uso.  
 * O kernel est� usando a lista de threads criadas, 
 * threadList[], n�o considerando a prioridade. :) sorry.
 *
 * @todo: Mudar o nome para dispachInit();.
 */ 
 
int init_dispatcher (void){

    int i=0;

    debug_print ("init_dispatcher:\n");
    
    //Seleciona o tipo de dispatcher.
    Dispatcher.policy = DISPATCHER_CURRENT;


//dispatcher_array:
	
	//Index
    dispatcherQueueIndex = (int) 0;


	//inicializa a fila do dispacher.

    for ( i=0; i <= PRIORITY_MAX; i++ ){
        dispatcherReadyList[i] = (unsigned long) 0;
    };

	//Init
	dispatcherReadyList[0] = (unsigned long) InitThread;


	//
	// (Desliga realtime dispatcher.) ??
	//
	
	
	//
	// Para um dispatcher na forma de Linked List.
	//

//dispatcher_linked_list:

    //
    // == Conductor =============================
    //
    
    // Inicializa o indice de condutores.
    conductorIndex = 0;

    rootConductor = (void *) kmalloc( sizeof(struct thread_d) );

    if ( (void *) rootConductor == NULL ){
        panic ("init_dispatcher: rootConductor");
    }

	// #bugbug 
	// Dever�amos iniciar com a idle thread e n�o com a thread 0.
	// Inicia a lista.
	// Usado para task switch.

    //#test
    rootConductor = (void *) InitThread;

    Conductor = (void *) rootConductor;
    
    tmpConductor       = (void *) rootConductor;
    tmpConductor->next = (void *) threadList[0]; 

    // #bugbug
    // Check the threads validation.

    return 0;
}



/*
 ************************************************
 * IncrementDispatcherCount:
 *     Mensura os crit�rios de escolha.
 *     Contagem por crit�rio de sele��o.
 *     Faz uma contagem de quantas vezes o dispatcher 
 * fez uso de cada crit�rio.
 * Obs: Esse modelo apresentou bons resultados por muito 
 * tempo. Vamos preserva-lo. 
 */

	// #todo
	// Create error messages.

void IncrementDispatcherCount ( int type ){

	// Testing struct.

    if ( (void *) DispatchCountBlock == NULL )
    {
        return;
    }

	// Limits.

    if (type < 0 || type > 10)
    {
        return;
    }


	// Activating the selected type.

    switch (type)
    {
        case SELECT_IDLE_COUNT:
            DispatchCountBlock->SelectIdleCount++;
            break;

        case SELECT_INITIALIZED_COUNT:
            DispatchCountBlock->SelectInitializedCount++;
            break;

        case SELECT_NEXT_COUNT:
            DispatchCountBlock->SelectNextCount++;
            break;

        case SELECT_CURRENT_COUNT:
            DispatchCountBlock->SelectCurrentCount++;
            break;

       case SELECT_ANY_COUNT:
            DispatchCountBlock->SelectAnyCount++;
            break;

       case SELECT_IDEAL_COUNT:
            DispatchCountBlock->SelectIdealCount++;
            break;

       case SELECT_DISPATCHER_COUNT:
            DispatchCountBlock->SelectDispatcherQueueCount++;
            break;

		//...

       // Nothing.
       // Aqui poderia ter um contador de indefini��es.
       default:
           break;
    };

	// Nothing.
	//     #bugbug
	//     Obs: @todo: O la�o acima pode n�o selecionar nada.

}


/*
   Constructor.
int dispatchDispatch()
{};
*/


/*
int dispatchInit()
{};
*/


//
// End.
//

