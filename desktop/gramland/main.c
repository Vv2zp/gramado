
// GRAMLAND.BIN
// Gramland window server.
// This is a ring0 display server and window manager.
// It has the same PID of the kernel, 0.
// #todo: 
// We need a fancy name for this project, not only 'gws'.
/*
 * File: main.c
 *    Main file for the Gramland.
 *    History:
 *        2020 - Created by Fred Nora.
 */
// See:
// Transformations
// http://math.hws.edu/graphicsbook/c2/s3.html
// See:
// https://wiki.osdev.org/Graphics_stack
// https://wayland-book.com/introduction/high-level-design.html
// ...
// See:
// x window system core protocol
// https://en.wikipedia.org/wiki/X_Window_System_core_protocol
// ...
// See:
// https://www.x.org/wiki/Releases/History/
// hostname:D.S
// [host]:<display>[.screen]
// [host]:<display>.[screen]

// host:
// host refers to a network host name, and 
// if absent means to connect to the local machine 
// via a unix domain socket.
// Each host can have multiple displays, and 
// each display can have multiple screens. 
// display: 
// A display consists (simplified) of:
// a keyboard, a mouse and a screen.
// The server serves displaying capabilities 
// to other programs that connect to it.
// screen:
// a single monitor or a group of monitors.

// display manager:
// login stuff
// The display manager must authenticate itself to the server.

// main.c
// Arquivo principal do gws.
// As funções começam com o nome do módulo
// para que em client side começem com gws_

/*
See: https://wiki.osdev.org/Graphics_stack
    > Application Layer
    > Interoperation Layers
        +Desktop Management Layer
        +Window Management Layer 
    > Presentation Layers
        +Compositing Layer
        +Widget Toolkit Layer
        +Rendering Layer 
    > Display Layers
        +Device Driver Layer
        +Hardware Layer 
*/

// This file is part of this project.
// It is NOT a library.

#include "gwsint.h"


#define VERSION  "0.1"
#define VERSION_MAJOR  0
#define VERSION_MINOR  1

// Gramado Network Protocol 

#define wsMSG_KEYDOWN     20
#define wsMSG_KEYUP       21
#define wsMSG_SYSKEYDOWN  22
#define wsMSG_SYSKEYUP    23

#define wsVK_F1    0x3B  //59    // No DOS é 0x170.  
#define wsVK_F2    0x3C  //60 
#define wsVK_F3    0x3D  //61 
#define wsVK_F4    0x3E  //62 


#define wsVK_RETURN    0x1C
#define wsVK_TAB       0x0F

#define wsCOLOR_BLACK    0x000000
#define wsCOLOR_GRAY     0x808080 


// #standard
#define GNP_WID        0
#define GNP_MESSAGE    1
#define GNP_LONG1      2
#define GNP_LONG2      3
// #extension
#define GNP_LONG3      4
#define GNP_LONG4      5
#define GNP_LONG5      6
#define GNP_LONG6      7
// #body
#define GNP_BODY_OFFSET    16
// ...

// #test
#define MSG_OFFSET_SHORTSTRING  64
#define SHORTSTRING_SIZE        64
#define MSG_OFFSET_LONGSTRING  128
#define LONGSTRING_SIZE        256
// ...

// see: globals.h
struct initialization_d  Initialization;

//see: gws.h
struct gws_d  *window_server;

static int IsAcceptingInput = FALSE;
static int IsAcceptingConnections = FALSE;
static int connection_status = 0;
// Number of accepted requests.
// How many times the accept() routine returned a valid value.
static unsigned long dispatch_counter=0;

static int IsTimeToQuit = FALSE;
static int NoReply = FALSE;
static int Notify_PongClient=FALSE;
static int ____saved_server_fd = -1;
static int ____saved_wm_magic_pid = -1;  // ?? The wm sends us its pid
static int __saved_sync_id = -1;

static int running = FALSE;

//
// == Private functions: Prototypes ========
//

// Worker
// There is a vetor with values for the next response.
// Called by dispatcher().
static int __send_response(int fd, int is_error);

static int initGUI(void);
static int on_execute(int dm);
static void dispacher(int fd);

static void __init_ws_structure(void);

static int
gwsProcedure (
    int client_fd,
    int msg,
    unsigned long long1,
    unsigned long long2 );

static int initGraphics(void);
static void initBackground(void);
static void initClientSupport(void);
static void initClientStruct( struct gws_client_d *c );


// This way the module is able to know
// if the server is accepting input or not.
int is_accepting_input(void)
{
    return (int) IsAcceptingInput;
}

// This way a module is able to deactivating the input
// temporarilly while he is very busy
// with a heavy drawing routine. Just like updating the desktop.
void set_input_status(int is_accepting)
{
    if (is_accepting != TRUE && is_accepting != FALSE)
        return;

    IsAcceptingInput = (int) is_accepting;
}

// Print a simple string in the serial port.
void gwssrv_debug_print (char *string)
{
    if ( (void*) string == NULL ){
        return;
    }
    gramado_system_call ( 
        289,
        (unsigned long) string,
        (unsigned long) string,
        (unsigned long) string );
}

// We can use the rtl or the library
unsigned long gwssrv_get_system_metrics (int index)
{

    //if (index<0){
        //gde_debug_print ("gswsrv_get_system_metrics: fail\n");
        //return 0;
    //}
    //#define	SYSTEMCALL_GETSYSTEMMETRICS  250 

   return (unsigned long) gramado_system_call ( 250, 
                               (unsigned long) index, 
                               (unsigned long) index, 
                               (unsigned long) index );
}



// enter critical section
// close the gate
void gwssrv_enter_critical_section (void)
{
    int S=0;

    // Pega o valor do spinlock principal.
    // Se deixou de ser 0 então posso entrar.
    // Se ainda for 0, continuo no while.
    // TRUE = OPEN.
    // FALSE = CLOSED.

    while (1){
        S = (int) gramado_system_call ( 226, 0, 0, 0 );

        if ( S == 1 ){ goto done; }
    };

    // Close the gate. turn FALSE.
done:
    gramado_system_call ( 227, 0, 0, 0 );
    return;
}


// exit critical section
// open the gate.
void gwssrv_exit_critical_section (void)
{
    gramado_system_call ( 228, 0, 0, 0 );
}




//
// =============================================================
//


//??
// Quando um cliente tenta se desconectar.
/*
// internal
void client_shutdown (int fd);
void client_shutdown (int fd)
{
	//remove client
	//deallocate resources.
}
*/


//?? Die after to handle a request.
// no multiple requests.
/*
void client_die(int fd);
void client_die(int fd)
{
    client_shutdown(fd);
}
*/


// ??
/*
int client_start(int fd);
int client_start(int fd)
{
    return -1;//todo
}
*/



/*
int client_send_error_response(int fd);
int client_send_error_response(int fd)
{
    return -1;//todo
}
*/


// Getting the server pid.
/*
int get_server_pid();
int get_server_pid()
{
    return (int) ... ; //#todo
}
*/

// getting the client id
/*
int get_client_id();
int get_client_id()
{
    return (int) ... ; //#todo
}
*/


// setting the client id
/*
void set_client_id ( int id );
void set_client_id ( int id )
{
	//#todo
    //____client_id = id;
}
*/


/*
int
__WriteToClient ( 
    struct gws_client_d *who, 
    char *buf, 
    int count );
int
__WriteToClient ( 
    struct gws_client_d *who, 
    char *buf, 
    int count )
{
    return -1;
}
*/


/*
int WriteToClient (int fd);
int WriteToClient (int fd)
{
    unsigned long *message_buffer = (unsigned long *) &__buffer[0];   
    int n_writes = 0;    // For responses.
 
    char *m = (char *) (&__buffer[0] + 16);
    sprintf( m, "gwssrv: This is a response from GWS!\n");

    // Primeiros longs do buffer.
    message_buffer[0] = next_response[0];         // Window ID.
    message_buffer[1] = SERVER_PACKET_TYPE_REPLY; // next_response[1] 
    message_buffer[2] = next_response[2];         // Return value (long1)
    message_buffer[3] = next_response[3];         // Return value (long2)

    if (fd<0)
        return 0;

   // #bugbug
   // How many times ?
   
__again:

    // #todo:
    // while(1){...}
    
    gwssrv_debug_print ("gwssrv: Sending response ...\n");

    //
    // Send
    //

    n_writes = write ( fd, __buffer, sizeof(__buffer) );
    //n_writes = send ( fd, __buffer, sizeof(__buffer), 0 );
    
    if (n_writes<=0){
        goto __again;
    }


    // Cleaning
    message_buffer[0] = 0;
    message_buffer[1] = 0;
    message_buffer[2] = 0;
    message_buffer[3] = 0;

    // Cleaning
    int c=0;
    for(c=0; c<32; c++)  //todo: 512
        next_response[c] = 0;

    gwssrv_debug_print ("gwssrv: Response sent\n");  
    
    return (int) n_writes;
}
*/


// Worker
// There is a vetor with values for the next response.
// Called by dispatcher().
// IN:
// fd, 1=REPLY | 2=EVENT | 3=ERROR
static int __send_response(int fd, int type)
{
// Reusing the same buffer from the request.
    unsigned long *message_buffer = (unsigned long *) &__buffer[0];
    int n_writes=0;
    int Status=0;

/*
// #bugbug
// We gotta select a standard offset for strings.
    char *m = (char *) (&__buffer[0] + 16); //#wrong
    char *m = (char *) (&__buffer[8]);  //use this one?
    sprintf( m, "~ Response from gwssrv \n");
*/

//
// Primeiros longs do buffer.
//

// 0:
// wid
    message_buffer[0] = (unsigned long) next_response[0];
    //message_buffer[0] = (unsigned long) (next_response[0] & 0xFFFFFFFF);

// 1:
// Type of reply.

    switch(type){
    // Normal reply
    case 1:
        message_buffer[1] = SERVER_PACKET_TYPE_REPLY;
        break;
    // Event
    case 2:
        message_buffer[1] = SERVER_PACKET_TYPE_EVENT;
        break;
    // Error
    case 3:
    default:
        message_buffer[1] = SERVER_PACKET_TYPE_ERROR;
        break;
    };

// 2 and 3:
// Signature in some cases.
// Can we deliver values here?
    message_buffer[2] = (unsigned long) next_response[2];  // long1
    message_buffer[3] = (unsigned long) next_response[3];  // long2

// 4,5,6,7  \./
// Data.
    message_buffer[4] = (unsigned long) next_response[4];
    message_buffer[5] = (unsigned long) next_response[5];
    message_buffer[6] = (unsigned long) next_response[6];
    message_buffer[7] = (unsigned long) next_response[7];

// 8,9,10,11  \o/
// Data.
    message_buffer[8]  = (unsigned long) next_response[8];
    message_buffer[9]  = (unsigned long) next_response[9];
    message_buffer[10] = (unsigned long) next_response[10];
    message_buffer[11] = (unsigned long) next_response[11];

// 12,13,14,15  \O/
// Data.
    message_buffer[12] = (unsigned long) next_response[12];
    message_buffer[13] = (unsigned long) next_response[13];
    message_buffer[14] = (unsigned long) next_response[14];
    message_buffer[15] = (unsigned long) next_response[15];

// more
    message_buffer[16] = (unsigned long) next_response[16];
    message_buffer[17] = (unsigned long) next_response[17];
    message_buffer[18] = (unsigned long) next_response[18];
    message_buffer[19] = (unsigned long) next_response[19];

    // ...

//__again:

//
// == Response ============================
//

    // #debug
    //gwssrv_debug_print ("__send_response: Sending response ...\n");

    // #todo:
    // while(1){...}

    /*
    // Is current client connected.
    if (currentClient->is_connected == 0)
    {
        // [FAIL] Not connected.
        // close?
    }
    */



//
// Send
//

// Limits
    if (fd<0 || fd>31)
    {
        Status = -1;
        goto exit1;
    }

// We can't write on our own socket.
    if (fd == ____saved_server_fd){
        printf("__send_response: fd == ____saved_server_fd\n");
        printf("The server can't write on your own socket\n");
        while(1){}
    }

// #test
// For now, the only valid fd is 31.
    if (fd != 31){
        printf("__send_response: fd != 31\n");
        while(1){
        };
    }

// Write

    n_writes = write ( fd, __buffer, sizeof(__buffer) );
    //n_writes = send ( fd, __buffer, sizeof(__buffer), 0 );

// Cleaning
// Limpa se a resposta der certo ou se der errado.

    // #delete:  Because message_buffer = __buffer.
    //message_buffer[0] = 0;
    //message_buffer[1] = 0;
    //message_buffer[2] = 0;
    //message_buffer[3] = 0;

    register int b=0;
    for (b=0; b<MSG_BUFFER_SIZE; ++b){
        __buffer[b] = 0;
    };

    register int c=0;
    for (c=0; c<NEXTRESPONSE_BUFFER_SIZE; ++c){
        next_response[c] = 0;
    };

// No. 
// We couldn't send a response.
// O que acontece se nao conseguirmos enviar uma resposta?
// Certamente o cliente tentara ler e tera problemas.
// Deveriamos fechar a conexao?
// Deveriamos enviar um alerta

    if (n_writes <= 0)
    {
        //#debug
        gwssrv_debug_print ("__send_response: response fail\n");
        printf             ("__send_response: Couldn't send reply\n");
        
        //close(fd);
        Status=-1;
        goto exit1;
    }

// YES, We sent the response.
    if (n_writes > 0){
        Status=0;
        goto exit0;
    }

    // Fall through.

// Fail
exit1:
    message_buffer[0] = 0;
    message_buffer[1] = 0;
    message_buffer[2] = 0;
    message_buffer[3] = 0;
    message_buffer[4] = 0;
    message_buffer[5] = 0;
exit0:
// Sync. Set response.
    rtl_set_file_sync( fd, SYNC_REQUEST_SET_ACTION, ACTION_REPLY );
    return (int) Status;
}


// Not used for now!
// Maybe in the future with smp.
void Compositor_Thread(void)
{
    // #debug
    printf ("f\n");
    wmRefreshDirtyRectangles();
}


// dispacher:
// Get client's request from socket.
// Messages sent via socket.
// obs: read and write use the buffer '__buffer'
// in the top of this file.
// #todo:
// No loop precisamos de accept() read() e write();
// Get client's request from socket.

static void dispacher(int fd)
{
    unsigned long *message_buffer = (unsigned long *) &__buffer[0];
    ssize_t n_reads = 0;
    int Status=-1;
    int SendErrorResponse=FALSE;

    dispatch_counter++;

// #todo:
// No loop precisamos de accept() read() e write();
//#atenção:
// A função accept vai retornar o descritor do 
// socket que usaremos ... por isso poderemos fecha-lo
// para assim obtermos um novo da próxima vez.

    //gwssrv_debug_print ("dispacher: \n");

    // Fail, cleaning.
    if (fd<0){
        gwssrv_debug_print ("dispacher: fd\n");
        goto exit2;
    }


//__loop:

//
// == Request ============================
//

// Requests may generate replies, events, and errors;
// Request packets are numbered sequentially by the server 
// as soon as it receives them.
// See:
// https://en.wikipedia.org/wiki/Round-trip_delay
// If the request is (XNextEvent), so the reply will be the event.

    /*
    // Is current client connected.
    if (currentClient->is_connected == 0)
    {
        // [FAIL] Not connected.
        // close?
    }
    */

// #important
// We can handle only requests.
// Drop it!

    int value = (int) rtl_get_file_sync( fd, SYNC_REQUEST_GET_ACTION );
    if (value != ACTION_REQUEST){
        goto exit2;
    }

// #todo
// Devemos escrever em nosso próprio
// socket e o kernel copia??
// o kernel copia para aquele arquivo ao qual esse estivere conectado.
// olhando em accept[0]
// Precisamos fechar o client se a leitura der errado?

//
// Recv
//

// We can't read our own socket.

    if ( fd == ____saved_server_fd ){
        printf("dispacher: fd == ____saved_server_fd\n");
        printf("The server can't read on your own socket\n");
        while(1){}
    }

// accept() always return 31 when
// getting the next client.

    if (fd != 31){
        printf("dispacher: fd != 31\n");
        while(1){}
    }

// Read
    n_reads = (ssize_t) read( fd, __buffer, sizeof(__buffer) );
    if (n_reads <= 0){
        gwssrv_debug_print ("dispacher: read fail\n");
        goto exit2;
    }

//
// == Processing the request =============================
//

// Invalid request. 
// Clean
    if (message_buffer[1] == 0){
        gwssrv_debug_print ("dispacher: Invalid request\n");
        goto exit2;
    }

// Um cliente solicitou um evento.
// Vamos sinalizar o tipo de resposta que temos que enviar,
// caso nenhum erro aconteça.
    int doSendEvent = FALSE;
    if (message_buffer[1] == GWS_GetNextEvent){
        doSendEvent = TRUE;  // The response is an EVENT, not a REPLY.
    }

// Process request.
// Do the service.
    // #debug
    // debug_print("dispacher: Process request\n");

// OUT
// <0 : error 

    Status = 
        (int) gwsProcedure (
                  (int) fd,
                  (int)           message_buffer[1],
                  (unsigned long) message_buffer[2],
                  (unsigned long) message_buffer[3] );

// Como o serviço não pode ser prestado corretamente.
// Então logo abaixo mandaremos uma resposta de erro
// e não uma resposta normal.
    if(Status < 0){
        SendErrorResponse = TRUE;
    }

//
// == Sending reply ==========
//

// Alguns requests não exigem resposta.
// Como é o caso das mensagens assíncronas.
// Entao precisamos modificar a flag de sincronizaçao.
// que ainda deve estar sinalizando um request.
// #todo
// O problema é que temos que conferir
// na biblioteca client-side se o cliente espera ou não
// por uma resposta para um dado tipo de mensagens.
// No momento todos os requests esperam por reposta?

    if (NoReply == TRUE){
        rtl_set_file_sync( 
            fd, SYNC_REQUEST_SET_ACTION, ACTION_NULL );
        goto exit0;
    }

//
// == reponse ================
//

// IN:
// fd, 1=REPLY | 2=EVENT | 3=ERROR

// ==========================================
// ERROR: (3)
// Se o serviço não pode ser prestado corretamente.
// Error message.
    if (SendErrorResponse == TRUE){
        Status = (int) __send_response(fd,3);
        goto exit2;
    }

// ==========================================
// EVENT: (2)
// Se o serviço foi prestado corretamente.
// Era uma solicitação de evento
// Event.
    if (doSendEvent == TRUE){
        Status = (int) __send_response(fd,2);
        goto exit2;
    }

// ==========================================
// REPLY: (1)
// Se o serviço foi prestado corretamente.
// Era uma solicitação de serviço normal,
// então vamos enviar um reponse normal. Um REPLY.
// Normal reply.
    if (doSendEvent != TRUE){
        Status = (int) __send_response(fd,1);
        goto exit2;
    }

// A mensagem foi enviada normalmente,
// Vamos sair normalmente.

    //if(Status >= 0)
        //goto exit0;

// Fall

exit2:
    message_buffer[0] = 0;
    message_buffer[1] = 0;
    message_buffer[2] = 0;
    message_buffer[3] = 0;
    message_buffer[4] = 0;
    message_buffer[5] = 0;
exit1:
exit0:
    return;
}

/*
 * ==============================
 * gwsProcedure:
 *     Main dialog.
 */
// Called by dispacher().
// #todo
// Dependendo do tipo de request, então construiremos
// a resposta ou prestatemos os serviço.
// Para cada tipo de request o servidor precisa construir
// uma resposta diferente.
// O request afeta os campos da mensagem.
// Esses campos estão em um buffer, mas poderiam estar
// em um arquivo json.
// Types:
// + Null: fail.
// + Identify: The server needs to identify itself.
// + Get all objects:
// + Set inspected object:
// + Set property: Probably setting a property of an object.
// + Disconnect:
// ...
// See:
// globals.h
// OUT
// <0 : error 

static int
gwsProcedure ( 
    int client_fd,
    int msg, 
    unsigned long long1, 
    unsigned long long2 )
{
    int Status=0;  //ok
    int my_pid = -1;

    // #debug
    //debug_print ("gwssrv: gwsProcedure\n");
    
    switch (msg){

    // If we received the message GWS_Quit and
    // there is no more windows, so quit the application.
    case GWS_Quit:
        if(windows_count == 0){
            //#todo: Não pode ter janelas abertas.
            IsTimeToQuit=TRUE;
        }
        break;

    // #todo
    // Isso poderia significar que existe uma estrutura
    // de request no mesmo estilo do X presente no
    // final do buffer de mensage, por exemplo
    // no offset 400
    // Mesma coisa aconteceria com as respostas.
    // A biblioteca client side pode conter um wrapper
    // com o protocolo X, colocando a estrutura de request 
    // do final do buffer da mensagem e sinalizar no header do
    // buffer que esse buffer contem uma mensagem que segue o
    // protocolo X.
    // #todo: Criar o arquivo Xproto.h ... xproto.h
    // GWS_X_Request:
         //break;

    // ===========================
    // Here starts the gws requests

    // Hello!
    // Draw text inside the root window.
    // screen_window = __root_window

    case GWS_Hello:
        gwssrv_debug_print ("gwssrv: Message number 1000\n");
        //#bugbug: Esse endereço de estrutura esta mudando para um valor
        //que nao podemos acessar em ring3.
        //if ( (void*) gui == NULL){
        //    NoReply = FALSE;
        //    break;
        //}
        if ( (void*) gui->screen_window != NULL)
        {
            //if ( gui->screen->used == 1 && gui->screen->magic == 1234 ){
                dtextDrawText ( 
                    (struct gws_window_d *) gui->screen_window,
                    long1, long2, COLOR_GREEN,
                    "gwssrv: Hello friend. This is the Gramado Window Server!");
                
                gws_show_backbuffer();
            //}
        }
        NoReply = FALSE;   //The client-side library is waiting for response.
        break;

    // Create Window
    // Usará o buffer global
    // See: window.c
    case GWS_CreateWindow:
        //#debug
        //gwssrv_debug_print ("gwssrv: [1001] serviceCreateWindow\n");
        serviceCreateWindow(client_fd);
        // #test
        // Handle incoming inputs right after a huge service routine.
        //if ( is_accepting_input() == TRUE )
        //    wmInputReader();
        NoReply = FALSE;   // We need to return the window id.
        break; 

    // backbuffer putpixel
    case GWS_BackbufferPutPixel:
        servicepixelBackBufferPutpixel(); 
        NoReply = FALSE;    //The client-side library is waiting for response.
        break;

    // backbuffer draw horizontal line
    case GWS_DrawHorizontalLine:
        servicelineBackbufferDrawHorizontalLine();
        NoReply = FALSE;  //  //The client-side library is waiting for response.
        break;

    // Draw char
    // See: char.c
    case GWS_DrawChar:
    
        // #debug
        //gwssrv_debug_print ("gwssrv: [1004] serviceDrawChar\n");
        
        serviceDrawChar();
        //NoReply = FALSE;
        NoReply = TRUE;
        break;

// Draw text
// See: dtext.c
    case GWS_DrawText:
        //#debug
        //gwssrv_debug_print ("gwssrv: [1005] serviceDrawText\n"); 
        serviceDrawText();
        //NoReply = FALSE;  // The client-side library is waiting for response.
        NoReply = TRUE; 
        break;

    // Refresh window
    case GWS_RefreshWindow:
        
        //#debug
        //gwssrv_debug_print ("gwssrv: [1006] serviceRefreshWindow\n");
        
        serviceRefreshWindow();
        // #test
        // Handle incoming inputs right after a huge service routine.
        //if ( is_accepting_input() == TRUE )
        //    wmInputReader();
        //NoReply = FALSE;
        NoReply = TRUE;
        break;

     // Redraw window
     case GWS_RedrawWindow:
         //#debug
         //gwssrv_debug_print ("gwssrv: [1007] serviceRedrawWindow\n");
         serviceRedrawWindow();
        // #test
        // Handle incoming inputs right after a huge service routine.
        //if ( is_accepting_input() == TRUE )
         //   wmInputReader();
         //NoReply = FALSE;
         NoReply = TRUE;    // syncronous
         break;

    // Resize window
    case GWS_ResizeWindow:
        
        //#debug
        //gwssrv_debug_print ("gwssrv: [1008] serviceResizeWindow\n");
        
        serviceResizeWindow();
        NoReply = FALSE;
        break;

    case GWS_ChangeWindowPosition:
        //#debug
        //gwssrv_debug_print ("gwssrv: [1009] serviceChangeWindowPosition\n");
        serviceChangeWindowPosition();
        NoReply = FALSE;
        break;

    // ...

    // backbuffer putpixel. (again)
    // IN: Color, x, y,rop
    case GWS_BackbufferPutPixel2:
        grBackBufferPutpixel ( 
            (unsigned long) COLOR_PINK, 
            (unsigned long) long1, 
            (unsigned long) long2,
            (unsigned long) 0 );
        NoReply = FALSE;
        break;

    // ...

    // Disconnect.
    // shutdown.
    // Um cliente quer se desconectar.
    case GWS_Disconnect:
        gwssrv_debug_print ("gwssrv: [2010] Disconnect\n");
        // DisconnectCurrentClient = TRUE;  // #todo
        // NoReply = TRUE;  // #todo
        break;

    case GWS_RefreshScreen:
        gws_show_backbuffer();
        // #test
        // Handle incoming inputs right after a huge service routine.
        //if ( is_accepting_input() == TRUE )
        //    wmInputReader();
        //NoReply = FALSE;      // #todo
        break;

    // Refresh rectangle ... 
    // See: wm.c
    case GWS_RefreshRectangle:
        
        //#debug
        //gwssrv_debug_print ("gwssrv: [2021] serviceRefreshRectangle\n");
        
        serviceRefreshRectangle();
        // #test
        // Handle incoming inputs right after a huge service routine.
        //if ( is_accepting_input() == TRUE )
        //    wmInputReader();
        NoReply = FALSE;
        break;

    // ?? #bugbug: The client only sends requests.
    // GWS_GetSendEvent
    case 2030:
        gwssrv_debug_print ("gwssrv: [2030] serviceClientEvent\n");
        //serviceClientEvent();
        //NoReply = FALSE;  // #todo
        break;

// The server will return an event from the its client's event queue.
    case GWS_GetNextEvent:
        //#debug
        //gwssrv_debug_print ("gwssrv: [2031] serviceNextEvent\n");
        serviceNextEvent();
        NoReply = FALSE; // Yes. We need a reply.
        break;

    // See: grprim.c
    case GWS_GrPlot0:  
        //gwssrv_debug_print ("gwssrv: [2040] serviceGrPlot0\n");
        serviceGrPlot0();  
        NoReply = FALSE;
        break;

    //#deprecated
    case GWS_GrCubeZ:  
    case GWS_GrRectangle:  
        NoReply = FALSE;
        break;

    // 2222: Async command 
    // Do not send a reply.
    case GWS_AsyncCommand:
        //#debug
        //gwssrv_debug_print ("gwssrv: GWS_AsyncCommand\n");
                  //printf ("gwssrv: GWS_AsyncCommand\n");
        serviceAsyncCommand();
        NoReply = TRUE;
        break;


    case GWS_PutClientMessage:
        
        //#debug
        //gwssrv_debug_print ("gwssrv: GWS_PutClientMessage\n");
        
        servicePutClientMessage();
        NoReply = TRUE;
        break;

    case GWS_GetClientMessage:
        //#debug
        //gwssrv_debug_print ("gwssrv: GWS_GetClientMessage\n");
        serviceGetClientMessage();
        NoReply = FALSE;
        break;

    case GWS_SetText:
        //printf ("[GWS_SetText] #todo\n");
        serviceSetText();
        NoReply = TRUE;
        break;

    case GWS_GetText:
        //printf ("[GWS_GetText] #todo\n");
        serviceGetText();
        NoReply = FALSE;  // The response is the text.
        return 0;
        break;

// Let's get one event from the client's event queue.
// Send it as a response.
    case GWS_DrainInput:
        //gwssrv_debug_print("gwssrv: gwsProcedure 8080\n");
        break;

    case GWS_GetWindowInfo:
        serviceGetWindowInfo();
        NoReply = FALSE;   // YES, send the response with the data.
        break;

// Service 9099:
// Clone and execute a process, given the image name.
    case GWS_CloneAndExecute:
        serviceCloneAndExecute();
        NoReply = TRUE;
        return 0;
        break;

    // ...

    default:
        
        //#debug
        //gwssrv_debug_print ("gwssrv: Default message number\n");
        
        //printf ("msg=%d ",msg);
        // NoReply = TRUE; //#todo
        Status = -1;  // Not ok.
        break;
    }

    return (int) Status;
}


static void initBackground(void)
{
    int WindowId = -1;
    unsigned long w = gws_get_device_width();
    unsigned long h = gws_get_device_height();

    // #debug
    // gwssrv_debug_print ("gwssrv: initBackground\n");

    if ( w==0 || h==0 ){
        gwssrv_debug_print ("initBackground: w h\n");
        printf             ("initBackground: w h\n");
        exit(1);
    }

// The background window
// #todo
// Se estivermos em JAIL, podemos arriscar algum background melhor.
// Talvez alguma imagem.

    // COLOR_BACKGROUND = that green.
    unsigned int bg_color = 
        (unsigned int) get_color(csiDesktop);

    __root_window = 
        (struct gws_window_d *) CreateWindow ( 
            WT_SIMPLE, 
            0, //style
            1, //status 
            1, //view
            "RootWindow",  
            0, 0, w, h,   
            gui->screen_window, 0, 
            bg_color, bg_color );

    // #debug
    // asm ("int $3");

    if ( (void *) __root_window == NULL ){
        gwssrv_debug_print ("initBackground: __root_window\n"); 
        printf             ("initBackground: __root_window\n");
        exit(1);
    }
    if ( __root_window->used != TRUE || __root_window->magic != 1234 ){
        gwssrv_debug_print ("initBackground: __root_window validation\n"); 
        printf             ("initBackground: __root_window validation\n");
        exit(1);
    }

// #test
    __root_window->locked = TRUE;

// Register the window.
    WindowId = RegisterWindow(__root_window);
    if (WindowId<0){
        gwssrv_debug_print ("initBackground: Couldn't register window\n");
        printf             ("initBackground: Couldn't register window\n");
        exit(1);
    }

    //#debug
    //asm ("int $3");

// Testing bmp.
// 
// See: gramado/themes

/*
    if (current_mode == GRAMADO_JAIL){
        gwssrv_display_system_icon ( 1, 8, 8 );
    }else{
        gwssrv_display_system_icon ( 2, 8, 8 );
    };
*/

// See: 
// gws.c

    if (current_mode == GRAMADO_JAIL){
        gwssrv_debug_print ("gwssrv: initBackground: Calling refresh_screen\n");
        //refresh_screen();
    }

// #debug
    //gwssrv_debug_print ("gwssrv: initBackground: done\n");
    //refresh_screen();
    //while(1){}
}

// initGraphics:
// Initialize the graphics support.
// Initialize the window server infrastructure.
// The current display and the current screen.
// Initialize the 3d support.

static int initGraphics(void)
{
    int __init_status = -1;

    //debug_print("initGraphics\n");
    //printf("initGraphics: \n");

    window_server->graphics_initialization_status = FALSE;


//#debug
    //gws_show_backbuffer();
    //while(1){}

// Initialize the graphics support.
// Now we can use 3d routines.
// See: grprim.c
    
    //gwssrv_debug_print ("initGraphics: Calling grInit() \n");
    //printf ("initGraphics: Calling grInit() \n");
    grInit();

//#debug
    //gws_show_backbuffer();
    //while(1){}

    // #debug
    //gwssrv_debug_print ("initGraphics: :)\n");
    //printf ("initGraphics: :)\n");
    //asm("int $3");
    //while(1){}

//
// Show surface.
// 

    //gws_show_backbuffer();
// invalidate the surface in ring0.
    invalidate_surface_retangle();

    comp_initialize_mouse();

    // #debug
    // asm("int $3");
    // while(1){}

    window_server->graphics_initialization_status = TRUE;
    return 0;
fail:
    window_server->graphics_initialization_status = FALSE;
    return (int) -1;
}

// Initialize the client list.
// This is an array of connections.
// See: clients.h
static void initClientSupport(void)
{
    register int i=0;

    //gwssrv_debug_print ("initClientSupport:\n");

    for (i=0; i<CLIENT_COUNT_MAX; i++)
    {
        connections[i] = 0;
    };
  
//
// The current client
//

// #todo
// We need the information about the current client
// And we need a list of the connected clientes.

    //currentClient = (struct gws_client_d *) 0;
    currentClient = NULL;

// The server client

    serverClient = 
        (struct gws_client_d *) malloc ( sizeof( struct gws_client_d ) );
    if ( (void *) serverClient == NULL ){
        gwssrv_debug_print ("initClientSupport: [FATAL] Couldn't create serverClient\n");
        printf             ("initClientSupport: [FATAL] Couldn't create serverClient\n");
        exit(1);
    }

// ID = 0.
    serverClient->id = 0;
    serverClient->is_connected = FALSE;
// The fd of the server.
// Nothing for now.
    serverClient->fd = -1;
// The pid of the server.
    serverClient->pid = (pid_t) getpid();
// group.
    serverClient->gid = getgid();
// The server is not visible ... 
    serverClient->is_visible = FALSE;
    //serverClient->window = ?;
    // ...
    serverClient->used = TRUE;
    serverClient->magic = 1234;

// The first client and the next.
    first_client = (struct gws_client_d *) serverClient;    
    first_client->next = NULL;
// Save us in the list of connections.
    connections[SERVER_CLIENT_INDEX] = (unsigned long) serverClient;
}

static void initClientStruct(struct gws_client_d *c)
{
    register int i=0;

    if ( (void *) c == NULL ){
        gwssrv_debug_print("initClientStruct: [FAIL] c\n");
        return;
    }

// ID
// #todo
    c->id = -1;  //fail

//geometry
    c->l = 0;
    c->t = 0;
    c->w = 50;
    c->h = 50;

    c->is_connected = FALSE;
    c->fd  = -1;
    c->pid = -1;
    c->gid = -1;
    
    c->is_visible = FALSE;

// No tags yet.
    for (i=0; i<4; i++){
       c->tags[i] = FALSE;
    };

    c->used = TRUE;
    c->magic = 1234;
}


/*
 //Send the message in the buffer to all the clients.
 //This is a great opportunity to shutdown the clients
 //if it is not connected.

void gwssrv_message_all_clients(void);
void gwssrv_message_all_clients(void)
{
}
*/

// When a client sent us an event
// ??? mas o cliente envia as coisas via request, ???
int serviceClientEvent(void)
{
    // The buffer is a global variable.
    //unsigned long *message_address = (unsigned long *) &__buffer[0];
    //return 0;  //ok
    return -1;
}

// serviceNextEvent:
// When a client get the next event from it's own queue.
// :: Retira em head.
// #todo: #bugbug: Por enquando só pegamos
// os eventos que estao na janela com o foco de entrada.
// Mas talvez devamos pegar os eventos em todas as janelas
// que pertencem ao cliente que chamou esse serviço.
int serviceNextEvent(void)
{
// #test
// Get the window in the queue of a given window.
// #old from Window with focus.


    unsigned long *message_address = (unsigned long *) &__buffer[0];

    struct gws_window_d *w;
    register int Head=0;

    // Get the window id
    int wid = (int) ( message_address[0] & 0xFFFFFFFF );
    if (wid<0 || wid>=WINDOW_COUNT_MAX)
        goto fail;

    // Get the pointer
    //#old focus_w = (struct gws_window_d *) get_focus();    
    w = (void *) windowList[wid];

    if ( (void*) w==NULL ){
        goto fail;
    }
    if (w->magic != 1234){
        goto fail;
    }

// Building the next response.
// It will be sent in the socket loop.

//header
    next_response[0] = 0;
    next_response[1] = SERVER_PACKET_TYPE_EVENT;  //msg type
    next_response[2] = 1234;  //signature
    next_response[3] = 5678;  //signature
// Index
    Head = (int) (w->ev_head & 0xFFFFFFFF);
// The message packet.
// wid, msg=(event type), data1, data2.
    next_response[4] = (unsigned long) w->ev_wid[Head];
    next_response[5] = (unsigned long) w->ev_msg[Head];
    next_response[6] = (unsigned long) w->ev_long1[Head];
    next_response[7] = (unsigned long) w->ev_long2[Head];
// Round
    w->ev_head++;
    if (w->ev_head >= 32){
        w->ev_head=0;
    }
// Clean
    w->ev_wid[Head] = 0;
    w->ev_msg[Head] = 0;
    w->ev_long1[Head] = 0;
    w->ev_long2[Head] = 0;
// Done
    return 0;
// Fail
fail:
    return (int) (-1);
}

// serviceNextEvent2:
// Get a message given the Index and the Restart flag.
// IN: long1=index, long2=restart index or not.
int serviceNextEvent2(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
// Window with focus.
    struct gws_window_d *focus_w;
    int Index=0;
    int Restart=0;
    register int Head=0;

    focus_w = (struct gws_window_d *) get_focus();    
    if( (void*) focus_w==NULL ){
        goto fail;
    }
    if(focus_w->magic != 1234){
        goto fail;
    }
    Index   = (int) message_address[2];  //long1
    Restart = (int) message_address[3];  //long2


// ====================================
// Building the next response.
// It will be sent in the socket loop.

//header
    next_response[0] = 0;  //
    next_response[1] = SERVER_PACKET_TYPE_EVENT;  //msg type
    next_response[2] = 1234;  //signature
    next_response[3] = 5678;  //signature

// Index
// It comes in the request.
    Head = (int) (Index & 0xFFFFFFFF);
    if (Head<0||Head>=32){
        goto fail;
    }
// The message packet.
// wid, msg=(event type), data1, data2.
    next_response[4] = (unsigned long) focus_w->ev_wid[Head];
    next_response[5] = (unsigned long) focus_w->ev_msg[Head];
    next_response[6] = (unsigned long) focus_w->ev_long1[Head];
    next_response[7] = (unsigned long) focus_w->ev_long2[Head];
// Clean
    focus_w->ev_wid[Head] = 0;
    focus_w->ev_msg[Head] = 0;
    focus_w->ev_long1[Head] = 0;
    focus_w->ev_long2[Head] = 0;
// Restart
    if (Restart==TRUE){
        focus_w->ev_head = 0;
        focus_w->ev_tail = 0;
        return 0;
    }
// Round
    focus_w->ev_head++;
    if (focus_w->ev_head >= 32){
        focus_w->ev_head=0;
    }
// Done
    return 0;
// Fail
fail:
// Restart
    if (Restart==TRUE){
        if ( (void*) focus_w != NULL){
            if(focus_w->magic==1234){
            focus_w->ev_head = 0;
            focus_w->ev_tail = 0;
            }
        }
        return 0;
    }
    return (int) (-1);
}

// Query window info.
// #todo: We can deliver more information yet.
// IN: wid ...
int serviceGetWindowInfo(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
// The request will give us the wid and the message code.
    struct gws_window_d *w;
    int wid = -1;
    int msg_code = -1;

// Get wid and message code.
    wid      = (int) message_address[0];  // window id 
    msg_code = (int) message_address[1];  // message code
    if (wid<0 || wid >= WINDOW_COUNT_MAX)
    {
        goto fail;
    }
    if (msg_code != GWS_GetWindowInfo){
        goto fail;
    }
// Window
    w = (struct gws_window_d *) windowList[wid];
    if ( (void*) w == NULL ){
        goto fail;
    }
    if (w->used != TRUE){
        goto fail;
    }
    if (w->magic != 1234){
        goto fail;
    }
// Header
// wid, msg type, signature1, signature2.
    next_response[0] = (unsigned long) (wid & 0xFFFFFFFF);
    next_response[1] = SERVER_PACKET_TYPE_REPLY;
    next_response[2] = 1234;
    next_response[3] = 5678;
// Window
// l,t,w,h
    next_response[7]  = (unsigned long) w->absolute_x;
    next_response[8]  = (unsigned long) w->absolute_y;
    next_response[9]  = (unsigned long) w->width;
    next_response[10] = (unsigned long) w->height;
// Client rectangle
// l,t,w,h
    next_response[13] = (unsigned long) w->rcClient.left;
    next_response[14] = (unsigned long) w->rcClient.top; 
    next_response[15] = (unsigned long) w->rcClient.width;
    next_response[16] = (unsigned long) w->rcClient.height;

    // ...

// #debug
    //printf("serviceGetWindowInfo: l=%d t=%d w=%d h=%d\n",
        //__root_window->left, 
        //__root_window->top, 
        //__root_window->width, 
        //__root_window->height );

//ok, send a reply response.
done:
    return 0;
fail:
    debug_print("serviceGetWindowInfo: fail\n");
    return (int) (-1);
}


// #todo
// Close all the clients and close the server.
void serviceExitGWS(void)
{
    printf ("serviceExitGWS: \n");
    // Kill all the clients.
    printf ("[TODO] Kill all the clients\n");
    // #todo
    // Deallocate resources used by the server.
    // Close the server.
    printf ("[TODO] Close all the clients\n");
    exit(0);
}

// #todo
// Now we put messages only in the window structure's message queue.
int servicePutClientMessage(void)
{
    debug_print("servicePutClientMessage: deprecated\n");
    return 0;
}


// #todo
// Now we get messages only in the window structure's message queue.
int serviceGetClientMessage(void)
{
    debug_print("serviceGetClientMessage: deprecated\n");
    return 0;
}

// serviceAsyncCommand:
// Async request.
// No response.
// #todo: Explain the arguments.
int serviceAsyncCommand(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];

    // int window_id=0;
    unsigned long message_id=0;
    unsigned long request_id=0;
    unsigned long subrequest_id=0;
    unsigned long data=0;

    unsigned long data1=0;   //same as 'data'.
    unsigned long data2=0;
    unsigned long data3=0;
    unsigned long data4=0;
    //...

// Helper
    int wid = -1;

    //#debug
    //gwssrv_debug_print ("serviceAsyncCommand:\n");
    //asm("cli");

// Parameters

    // window_id  = message_address[0];
// message code
    message_id = message_address[1];
// Service code. 
// #todo: Create a list of services. 
    request_id    = message_address[2];
    subrequest_id = message_address[3];
// data
    data = message_address[4];
    // ...

    data1 = (unsigned long) message_address[4];   //same as 'data'
    data2 = (unsigned long) message_address[5];
    data3 = (unsigned long) message_address[6];
    data4 = (unsigned long) message_address[7];

// Validate our message number.
    if (message_id != 2222){
        gwssrv_debug_print ("serviceAsyncCommand: [ERROR] message_id\n");
                    printf ("serviceAsyncCommand: [ERROR] message_id\n");
        goto fail;
        //return (int) (-1);
    }

//
// id
//
    // #debug
    // printf ("serviceAsyncCommand: [request %d] \n", request_id);
 
    switch (request_id){

    // 1 =  Exit GWS
    // #todo:
    // Close all the clients and close the server.
    case 1:
        gwssrv_debug_print ("serviceAsyncCommand: [1]  Exit GWS\n");
        //printf ("serviceAsyncCommand: [request 1] Closing server\n");
        printf("serviceAsyncCommand: Exit GWS\n");
        serviceExitGWS();
        printf("serviceAsyncCommand: [FAIL] fail when closing the GWS\n");
        exit(0);
        goto done;
        break;

    // ping
    case 2:
        gwssrv_debug_print ("serviceAsyncCommand: [2] \n");
        printf("PING\n");
        //Notify_CloseClient = TRUE;
        //Notify_PongClient = TRUE;
        //exit(0);
        //return 0;
        goto done;
        break;

    // print Hello.
    case 3:
        gwssrv_debug_print ("serviceAsyncCommand: [3] hello\n");
        printf("HELLO\n");
        //exit(0);
        //return 0;
        goto done;
        break;

    // Demos:
    // See: demos.c
    case 4:
        gwssrv_debug_print ("serviceAsyncCommand: [4] \n");
        if (current_mode == GRAMADO_JAIL){
            gwssrv_debug_print("serviceAsyncCommand: [request 4] demo\n"); 
            demos_startup_animation(subrequest_id);
            gwssrv_show_backbuffer();
            goto done;
        }
        goto done;
        break;

    // Draw black rectangle.
    // #bugbug: trava ...
    //case 5:
       //gwssrv_debug_print ("serviceAsyncCommand: [5] \n");
       //if (current_mode == GRAMADO_JAIL){
           //rectBackbufferDrawRectangle ( 
           //    0, 0, 28, 8, COLOR_GREEN, TRUE,
           //    0 );  //rop_flags
           //return 0;
       //}
       //break;

    // Setup if we will show or not the 'fps window'.
    case 6:
        gwssrv_debug_print ("serviceAsyncCommand: [6]\n");
        if( subrequest_id == TRUE ){ show_fps_window = TRUE;  goto done; }
        if( subrequest_id != TRUE ){ show_fps_window = FALSE; goto done; }
        //show_fps_window = FALSE;
        goto done;
        break;

    // Register wm pid
    // #suspended: o wm fica no ws por enquanto.
    case 7:
        //gwssrv_debug_print ("serviceAsyncCommand: [7] Register wm pid\n");
        //printf ("serviceAsyncCommand: [7] [BREAKPOINT] Register wm pid\n");
        //____saved_wm_magic_pid = (int) data;
        //exit(0);
        //return 0;
        goto done;
        break;
 
    // 8
    // Window Manager requests. Power Trio.
    // As mensages aqui interessam somente ao window manager
    // que esta dentro do window server.
    case 8:
        //gwssrv_debug_print ("serviceAsyncCommand: [8] \n");
        //printf ("serviceAsyncCommand: [8] \n");
        //if (subrequest_id = 1)
        //{
            //exit(0);
        //}
        goto done;
        break;

    // Set focus.
    case 9:
        // gwssrv_debug_print ("serviceAsyncCommand: [9] \n");
        if (data<0){ goto done; }
        set_focus_by_id( (int) data );
        goto done;
        break;


    // #test
    // drawing a rect using ring0 and ring3 routines.
    // TRUE = use kgws ; FALSE =  do not use kgws.
    case 10:
        gwssrv_debug_print ("serviceAsyncCommand: [10] \n");
                    printf ("serviceAsyncCommand: [10] \n");

        rectBackbufferDrawRectangle0(
            10, 10, 40, 40,
            COLOR_RED,
            TRUE,
            0,        // rop falgs
            FALSE );   // TRUE = use kgws. (kernel service)

        gws_refresh_rectangle(10, 10, 40, 40);
        //return 0;
        goto done;
        break;

    //see: wm.c
    case 11:
        wm_update_desktop(TRUE,TRUE);
        goto done;
        break;

    case 12:
        __switch_focus();
        goto done;
        break;

    // data=wid
    case 13:
        invalidate_window_by_id(data);
        goto done;
        break;

// Clear the window
// Repaint it using the default background color.
    case 14:
        // #todo data = wid
        //printf("14: wid={%d}\n",data);
        clear_window_by_id(data,TRUE);
        goto done;
        break;

    // Set active window by id.
    case 15:
        // gwssrv_debug_print ("serviceAsyncCommand: [9] \n");
        if (data<0){ goto done; }
        set_active_by_id( (int) data );
        goto done;
        break;

    // #todo
    // Change the title bar icon, given the cache ID.

// poweroff
// qemu only
    case 22:
        rtl_clone_and_execute("shutdown.bin");
        goto done;
        break;

// Enable ps2-mouse support.
    case 44:
        // Calling the kernel to make the full ps2 initialization.
        // #todo: Create a wrapper fot that syscall.
        // #todo: Allow only the ws pid to make this call.
        sc82 ( 22011, 0, 0, 0 );
        // Enable the use of mouse here in the server.
        gUseMouse = TRUE;
        goto done;
        break;

    // ...

    // Set flag to quit the server.
    case 88:
        printf("88: IsTimeToQuit\n");
        IsTimeToQuit = TRUE;
        goto done;
        break;

    // Reboot the system via ws.
    case 89:
        printf("89: Reboot via ws\n");
        wm_reboot();
        goto done;
        break;

    // Destroy overlapped window.
    case 90:
        wid = (int) (data & 0xFFFFFFFF);
        printf("90: Destroy overlapped window %d\n",wid);
        DestroyWindow(wid);
        wm_rebuild_list();
        wm_update_desktop(TRUE,TRUE);
        break;

    //#todo: Destroy window.
    //#todo: Create a wrapper in the lingws library,
    // the wrapper will call the async request with
    // the service '99'.
    //case 99:
        //goto done;
        //break;

    // put pixel.
    case 1000:
        //printf("1000: %d %d %d\n",data1,data2,data3);
        // IN: color, x, y, rop
        grBackBufferPutpixel(
            (unsigned int) data3,
            (data1 & 0xFFFFFFFF),
            (data2 & 0xFFFFFFFF),
            (data4 & 0xFFFFFFFF) );
        //refresh_screen();  //#bugbug
        break;

    // ...

    default:
        gwssrv_debug_print ("serviceAsyncCommand: [ERROR] bad request\n");
                 // printf ("serviceAsyncCommand: [ERROR] bad request\n");
        // return -1;
        goto fail;
        break;
    };

    goto fail;

done:
    //gwssrv_debug_print ("serviceAsyncCommand: done\n");
    //asm("sti");
    return 0;
fail:
    gwssrv_debug_print ("serviceAsyncCommand: FAIL\n");
    //asm("sti");
    return (int)(-1);
}

/*
 * serviceCreateWindow:
 *     Create a window.
 *     Service: GWS_CreateWindow.
 *     It's a wrapper.
 *     Chamaremos a função que cria a janela com base 
 * nos argumentos que estão no buffer, que é uma variável global 
 * nesse documento.
 *     Mostraremos a janela na tela ao fim dessa rotina.
 *     #todo: Mas poderíamos simplesmente marcar como 'dirty'.
 */
// Called by gwsProcedure in main.c
// #todo
// Receive the tid of the client in the request packet.
// Save it as an argument of the window structure.

int serviceCreateWindow (int client_fd)
{
// The buffer is a global variable.
    unsigned long *message_address = (unsigned long *) &__buffer[0];
// The structure for the standard request.
    gReq r;
// Iterator.
    register int i=0;

// The window been created.
    struct gws_window_d *Window;
    wid_t wid = (-1);

// The parent window.
    struct gws_window_d *Parent;
    wid_t pwid = (-1);

// Arguments
    unsigned long x=0;
    unsigned long y=0;
    unsigned long w=0;
    unsigned long h=0;
    unsigned int color=0;
    unsigned long type=0;
// Device context
    unsigned long deviceLeft = 0;
    unsigned long deviceTop = 0;
    unsigned long deviceWidth  = (__device_width  & 0xFFFF);
    unsigned long deviceHeight = (__device_height & 0xFFFF);
// tid da control thread do cliente.
    int ClientPID = -1;
    int ClientTID = -1;
    unsigned int frame_color = COLOR_WINDOW;
    unsigned int client_color = COLOR_WINDOW;
    unsigned long my_style=0;

    //#debug
    //gwssrv_debug_print ("serviceCreateWindow:\n");
    //asm("cli");
//
// Get the arguments.
//

// The header.
// 0,1,2,3
// wid, msg code, data1, data2;
    r.wid  = message_address[0];
    r.code = message_address[1];
    r.ul2  = message_address[2];
    r.ul3  = message_address[3];
// l,t,w,h
// These are the outer values.
// Including the border if it has one.
    r.ul4 = message_address[4];
    r.ul5 = message_address[5];
    r.ul6 = message_address[6];
    r.ul7 = message_address[7];
    x = (unsigned long) (r.ul4 & 0xFFFF);
    y = (unsigned long) (r.ul5 & 0xFFFF);
    w = (unsigned long) (r.ul6 & 0xFFFF);
    h = (unsigned long) (r.ul7 & 0xFFFF);
// Background color.
    r.ul8 = message_address[8];
    color        = (unsigned int) (r.ul8 & 0x00FFFFFF);
    frame_color  = (unsigned int) color;
    client_color = (unsigned int) color;
// type
    r.ul9 = message_address[9];
    type = (unsigned long) (r.ul9 & 0xFFFF);
// Parent window ID.
    r.ul10 = message_address[10]; 
    pwid = (int) (r.ul10 & 0xFFFF);
// Style
// #test
    r.ul11 = message_address[11];  
    my_style = (unsigned long) r.ul11;  // 8 bytes
// Client pid
    r.ul12 = message_address[12];  // client pid
    ClientPID = (int) (r.ul12 & 0xFFFF);
// Client caller tid
    r.ul13 = message_address[13];  // client tid
    ClientTID = (int) (r.ul13 & 0xFFFF);

// ========================================
// 14:
// This is the start of the string passed via message.
// Copy 256 bytes of given string.
// Do we ha a limit?
//++
// String support 
// Copiando para nossa estrutura local.
    register int string_off = 14;
    char *p = (char *) &message_address[string_off];
    memset(r.data, 0, 256);
    for (i=0; i<256; ++i){
        r.data[i] = *p;  //Get a char from the message buffer. 
        p++;
    };
    r.data[255] = 0;
    // #bugbug: Out of range
    //r.data[i] = 0;
    //r.data[i+1] = 0;
//--
// ========================================

    // #debug
    //printf ("serviceCreateWindow: pid=%d tid=%d *breakpoint\n", 
    //    ClientPID, ClientTID );
    //while(1){}

// Purify
    x = (x & 0xFFFF);
    y = (y & 0xFFFF);
    w = (w & 0xFFFF);
    h = (h & 0xFFFF);

// Final Limits
    //if( x >= deviceWidth )
        //return -1;
    //if( y >= deviceHeight )
        //return -1;

    //#debug
    //printf("%s\n",name_buffer);
    //while(1){}
//===================


// --------------------------------

//
// Parent window
//

// id limits.
    if (pwid<0 || pwid>=WINDOW_COUNT_MAX){
        pwid=0;
        printf("serviceCreateWindow: pwid\n");
        exit(1);
    }
// Structure pointer
    Parent = (struct gws_window_d *) windowList[pwid];
    if ( (void*) Parent == NULL ){
        printf("serviceCreateWindow: Parent\n");
        exit(1);
    }
    if (Parent->magic != 1234){
        printf("serviceCreateWindow: Parent validation\n");
        exit(1);
    }

// --------------------------------

// Draw
// See: window.c

// #todo
// What is the border size?

    Window = 
        (struct gws_window_d *) CreateWindow ( 
                                    type,      // type
                                    my_style,  // style
                                    1,         // status 
                                    1,         // view
                                    r.data,    // name
                                    x, y, w, h,  // l,t,w,h 
                                    (struct gws_window_d *) Parent,  // parent struct pointer 
                                    0,               // ?? desktop id
                                    frame_color,     // frame color.
                                    client_color );  // client area color.

    if ( (void *) Window == NULL ){
       gwssrv_debug_print ("gwssrv: CreateWindow fail\n");
       next_response[1] = 0;
       goto fail;
       //return -1;
    }

// Register window
    wid = RegisterWindow(Window);
    if (wid<0 || wid>=WINDOW_COUNT_MAX)
    {
        gwssrv_debug_print ("gwssrv: serviceCreateWindow Couldn't register window\n");
        next_response[1] = 0;  // msg code.
        goto fail;
        //return -1;
    }

//===============================================

// The client fd.
    Window->client_fd = (int) client_fd;

//
// Input queue
// 

// Initialized the input queue

/*
    // #deprecated ...
    // Enviaremos mensagens para a fila na thread
    // associada a essa janela.
    // Indicado em client_tid.

    //int i=0;
    for ( i=0; i<32; ++i )
    {
        Window->window_list[i] = 0;
        Window->msg_list[i]    = 0;
        Window->long1_list[i]  = 0;
        Window->long2_list[i]  = 0;
        Window->long3_list[i]  = 0;
        Window->long4_list[i]  = 0;
    };
    Window->head_pos = 0;
    Window->tail_pos = 0;
*/
//===============================================

// String support.
// Prepara o nome.
// #test
    register int name_len=0;
    name_len = (int) strlen(Window->name);
    if(name_len > 32){ name_len=32; }
    char w_name[64];
    sprintf(w_name,":: ");
    strncat(w_name, Window->name, name_len);
    w_name[63]=0;

// Coloca na fila
// #test: notifica na barra de tarefas

    int manage_status = -1;

    if (Window->type == WT_OVERLAPPED)
    {
        // Adiciona na lista de janelas
        wm_add_window_into_the_list(Window);
        // Associa a janela com uma estrutura de cliente.
        manage_status = wmManageWindow(Window);
        if (manage_status<0){
            printf("serviceCreateWindow: wmManageWindow fail\n");
            while(1){}
        }
        // Atualiza a barra de tarefas,
        // notificando a criaçao dessa janela.
        wm_Update_TaskBar((char *) w_name,TRUE);

        // #test
        // Activate
        //set_active_window(Window);
    }

// Save the tid of the client.
// #test
    Window->client_pid = ClientPID;
    Window->client_tid = ClientTID;

// The client's fd.
// #todo
// We need to register the client's fd.
// It is gonna be used to send replies, just like input events.
    // Window->client_fd = ?;

// Building the next response.
// It will be sent in the socket loop.
    next_response[0] = (unsigned long) (wid & 0xFFFFFFFF);  // wid
    next_response[1] = SERVER_PACKET_TYPE_REPLY;            // msg code
    next_response[2] = 0;  //#todo: Maybe we can send aditional info here.
    next_response[3] = 0;  //#todo: Maybe we can send aditional info here.

// Show
// Invalidate rectangle
// #todo: 
// We need a flag here. Came from parameters.

    // if( Show == TRUE )
    gws_show_window_rect(Window);
    
    //#debug
    //if (Window->type == WT_ICON)
    //    while(1){}
    
    //Window->dirty = TRUE;
    return 0;
fail:
    gwssrv_debug_print ("serviceCreateWindow: FAIL\n");
    return -1;
}

// Draw char.
// Service 1004.
int serviceDrawChar (void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d *window;
    int window_id = -1;
    unsigned long x=0;
    unsigned long y=0;
    unsigned long color=0;
    int __char=0;
    // #todo
    char *text_buffer; 
    unsigned char _string[4];

    // #debug
    // gwssrv_debug_print ("serviceDrawChar: \n");

// Get message parameters.
    window_id = message_address[4];
    x         = message_address[5];  // left
    y         = message_address[6];  // top
    color     = message_address[7];
    unsigned long C = (unsigned long) message_address[8];
    //text_buffer =    //#todo
   
// Let's create a fake string.
   _string[0] = (unsigned char) C;
   _string[1] = (unsigned char) 0;

// Window ID limits
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        gwssrv_debug_print ("gwssrv: serviceDrawChar window_id\n");
        return -1;
    }
// Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("gwssrv: serviceDrawChar window\n");
        return -1;
    }
    if ( window->used != TRUE || window->magic != 1234 ){
        gwssrv_debug_print ("gwssrv: serviceDrawChar window validation\n");
        return -1;
    }

    x = (unsigned long) (x & 0xFFFFFFFF);
    y = (unsigned long) (y & 0xFFFFFFFF);

// Draw
// Let's draw the char 
// using the routine for drawing a string.
// See: dtext.c
// If this window is overlapped window,
// so paint into the client area.
    if (window->type == WT_OVERLAPPED)
    {
        // #
        // A chamada indica um deslocamento 
        // dentro da área de cliente da parent,
        // não um valor absoluto.
        // >> Adiciona o deslocamento da client area.
        x = window->rcClient.left + x;
        y = window->rcClient.top  + y;
        // >> Adiciona o deslocamento da parent.
        dtextDrawText2 ( 
            (struct gws_window_d *) window,
            x, 
            y, 
            color, 
            (unsigned char *) &_string[0],
            TRUE );

    }else{
        // #
        // A chamada indica um deslocamento na parent,
        // não um valor absoluto.
        // >> Adiciona o deslocamento da parent.
        dtextDrawText2 ( 
            (struct gws_window_d *) window,
            x, 
            y, 
            color, 
            (unsigned char *) &_string[0],
            TRUE );
    };

// Draw

/*
    dtextDrawText2 ( 
        (struct gws_window_d *) window,
        x, 
        y, 
        color, 
        (unsigned char *) &_string[0],
        TRUE );
*/

    return 0;
}

int serviceChangeWindowPosition(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d *window;
    int window_id = -1;
    unsigned long x = 0;
    unsigned long y = 0;

    // #debug
    //gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition\n");

// Get
    window_id = message_address[0];  //wid
    // msg
    x         = message_address[2];  
    y         = message_address[3];  

// Window ID limits
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window_id\n");
        return -1;
    }

// Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window\n");
        return -1;
    }
    if ( window->used != TRUE || window->magic != 1234 ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition validation\n");
        return -1;
    }

// can't change root
    //if (window == __root_window)
        //return -1;

// see: wm.c
    gwssrv_change_window_position ( 
        (struct gws_window_d *) window, 
        (unsigned long) x, 
        (unsigned long) y );

    return 0;
}

int serviceResizeWindow(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d *window;
    int window_id = -1;
    unsigned long w = 0;
    unsigned long h = 0;

    // #debug
    //gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition\n");

    // #todo
    // Check all the header.

// Get
    window_id = message_address[0];  //wid
    // msg
    w         = message_address[2];
    h         = message_address[3];

// Window ID limits
    if ( window_id < 0 || 
         window_id >= WINDOW_COUNT_MAX )
    {
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window_id\n");
        return -1;
    }

// Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition window\n");
        return -1;
    }
    if ( window->used != TRUE || 
         window->magic != 1234 ){
        gwssrv_debug_print ("gwssrv: serviceChangeWindowPosition validation\n");
        return -1;
    }

// can't change root window.
    //if (window == __root_window)
        //return -1;

    gws_resize_window ( 
        (struct gws_window_d *) window, 
        (unsigned long) w, 
        (unsigned long) h );

    return 0;
}

// #bugbug
// Usaremos a função create window para desenhar botões.
// #deletar !!!
int serviceDrawButton(void)
{
    // Deprecated !!
    gwssrv_debug_print("serviceDrawButton: deprecated\n");
    printf            ("serviceDrawButton: deprecated\n");
    exit(1);
    return -1;
}

// Redraw window.
// It will invalidate the window, and it will need to be flushed
// into the frame buffer.
int serviceRedrawWindow (void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d *window;
    int window_id = -1;
    int msg_code  = 0;
    unsigned long flags = 0;

    // #debug
    // gwssrv_debug_print ("serviceRedrawWindow:\n");

// Get wid, msg code and flag.
    window_id  = message_address[0];  // window id 
    msg_code   = message_address[1];  // message code
    flags      = message_address[2];  // flags
    //??       = message_address[3];  // nothing 

    // #todo
    //  Not tested yet.
    //if ( msg_code <= 0 ){
    //    gwssrv_debug_print ("serviceRedrawWindow:\n");
    //    goto fail;
    //}

// Window ID
    window_id = (int) (window_id & 0xFFFF);
    if (window_id < 0 || window_id >= WINDOW_COUNT_MAX){
        gwssrv_debug_print ("serviceRedrawWindow: [FAIL] window_id\n");
        goto fail;
    }
// Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("serviceRedrawWindow: window\n");
        goto fail;
    }
    if (window->used != TRUE || window->magic != 1234){
        gwssrv_debug_print ("serviceRedrawWindow: window validation\n");
        goto fail;
    }

// Redraw it.
// IN: w, flags: 
// flags: 
// TRUE = show the window now. FALSE=do not show the window.
    redraw_window ( 
        (struct gws_window_d *) window, 
        (unsigned long) flags );

// Invalidate
// Now the compositor can refresh it.
// Não podemos mostra se a flag diz pra não mostrar.
    // window->dirty = TRUE;
    return 0;
fail:
    return (int) (-1);
}

// 2021
// Flush a given area into the framebuffer.
int serviceRefreshRectangle(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    unsigned long left=0;
    unsigned long top=0;
    unsigned long width=0;
    unsigned long height=0;
// #todo
// Check all the header.
    unsigned long msg_code = message_address[1];

    // #debug
    // debug_print("serviceRefreshRectangle:\n");
    // asm("cli");

    if( msg_code != GWS_RefreshRectangle ){
        goto fail;
        //return -1;
    }

// #todo
// Check if the message code is right.
// l,t,w,h
    left   = message_address[4];
    top    = message_address[5];
    width  = message_address[6];
    height = message_address[7];
    left   = (left   & 0xFFFF);
    top    = (top    & 0xFFFF);
    width  = (width  & 0xFFFF);
    height = (height & 0xFFFF);

// #todo
// Maybe we can test some limits here.
// Flush it into the framebuffer.
// See: rect.c
    gws_refresh_rectangle ( left, top, width, height );
    return 0;
fail:
    debug_print("serviceRefreshRectangle: fail\n");
    //asm("sti");
    return -1;
}

// 1006
// Flush a given window into the backbuffer.
// #todo: Here we can simple mark this window as 'dirty'
// and let the compositor do its job.
int serviceRefreshWindow(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d *window;
    int window_id = -1;
    //unsigned long x;
    //unsigned long y;
    //unsigned long color;
    //int __char;
    //char *text_buffer;    // #todo

    // #debug
    // gwssrv_debug_print ("serviceRefreshWindow:\n");

    // #todo
    // Check all the header.

// wid
    window_id = (int) message_address[0];   
// #extra
// Special case.
// Will be used in the ghost frame routines.
// ??
    if (window_id == (-4)){
        gwssrv_debug_print("serviceRefreshWindow:\n");  //debug
        gwssrv_debug_print("== R (extra) ==\n");        //debug
        refresh_device_screen();
        goto done;
        //return 0;
    }
// wid limits
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX ){
        //printf("%d\n",window_id);
        gwssrv_debug_print ("serviceRefreshWindow: window_id\n");
        goto fail;
        //return -1;
    }

// Get the window structure given the id.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void *) window == NULL ){
        gwssrv_debug_print ("serviceRefreshWindow: window\n");
        goto fail;
        //return -1;
    }
    if ( window->used != TRUE || window->magic != 1234 ){
        gwssrv_debug_print ("serviceRefreshWindow: window validation\n");
        goto fail;
        //return -1;
    }

// #todo
// simply invalidate the window and let the compositor do its job.
    //invalidate_window(window);

// Flush
    gws_show_window_rect(window);
    // wmInputReader();

done:
    return 0;
fail:
    gwssrv_debug_print ("serviceRefreshWindow: fail\n");
    return -1;
}

// #todo
// Get the name of the image for a process to lauch.
// IN: for arguments and a string.
void serviceCloneAndExecute(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
// ==================================
// Get string from message
// #todo: Talvez poderiamos receber o tamanho da string.
    unsigned char buf[256+1];
    register int i=0;
    int string_off=8;
    char *p = (char *) &message_address[string_off];
    memset(buf, 0 ,256);
    for (i=0; i<256; i++)
    {
         buf[i] = *p;
         p++;
    };
    buf[i] = 0;
// ==================================
    rtl_clone_and_execute(buf);
}

// Draw text.
// Service 1005
int serviceDrawText(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d  *window;
    int window_id = -1;      // index 4
    unsigned long x;         // index 5
    unsigned long y;         // index 6
    unsigned long color;     // index 7
// Device context
    unsigned long deviceLeft   = 0;
    unsigned long deviceTop    = 0;
    unsigned long deviceWidth  = (__device_width  & 0xFFFF );
    unsigned long deviceHeight = (__device_height & 0xFFFF );

    // #debug
    // gwssrv_debug_print ("gwssrv: serviceDrawText\n");

// Get values.
// #todo: Check the message code.
// wid, x, y, color.
    window_id = (int)           message_address[4];
    x         = (unsigned long) message_address[5];
    y         = (unsigned long) message_address[6];
    color     = (unsigned long) message_address[7];
    window_id = (window_id & 0xFFFF);
    x         = (x         & 0xFFFF);
    y         = (y         & 0xFFFF);

// Text?
    /*
    //size 256 bytes
    unsigned char *text_buffer = (unsigned char *) &message_address[MSG_OFFSET_LONGSTRING];
    int s = sizeof(text_buffer);
    if(s<0 || s>= 256){
        gwssrv_debug_print ("gwssrv: serviceDrawText [DEBUG]   SIZE \n");
        exit(1);
    }
    */

// Limits
// O texto começa fora dos limites da tela do dispositivo.
    //if( x >= deviceWidth )
        //return -1;
    //if( y >= deviceHeight )
        //return -1;

//
// == String support ========
//

// ==================================
// Get string from message
// #todo: Talvez poderiamos receber o tamanho da string.
    unsigned char buf[256+1];
    register int i=0;
    int string_off=8;
    char *p = (char *) &message_address[string_off];
    for (i=0; i<256; i++)
    {
         buf[i] = *p;  //Get a char
         p++;
    };
    buf[i] = 0;  // finalize the buffer.
// ==================================

//
// == Draw ===============================================
//

    //#todo
    //switch (alignment) {  ... }

// Window support.
// Se a janela alvo tem um índice fora dos limites

// wid
    if ( window_id < 0 ){ return -1; }
    if ( window_id >= WINDOW_COUNT_MAX ){
        return -1;
    }

// window structure.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void*) window == NULL ){ 
        return -1; 
    }
    if (window->magic != 1234){
        return -1; 
    }

// If this window is overlapped window,
// so paint into the client area.
    if (window->type == WT_OVERLAPPED){
        x += window->rcClient.left;
        y += window->rcClient.top;
    }

// Draw text
// Good window. Let's paint into it.
    dtextDrawText ( 
        (struct gws_window_d *) window,
        x, y, (unsigned int) color, buf );

// #debug
// Flush the window into the framebuffer.
// #todo: invalidate, not show.
    //gws_show_window_rect(window);
    invalidate_window(window);
    return 0;
crazy_fail:
    debug_print("serviceDrawText: [ERROR] crazy_fail\n");
    return -1;
}


// Put the text into the buffer
// if the window is an editbox window.
int serviceSetText(void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d  *window;
    int window_id = -1;      // index 4
    unsigned long x;         // index 5
    unsigned long y;         // index 6
    unsigned long color;     // index 7
// Device context
    unsigned long deviceLeft   = 0;
    unsigned long deviceTop    = 0;
    unsigned long deviceWidth  = (__device_width  & 0xFFFF );
    unsigned long deviceHeight = (__device_height & 0xFFFF );

    // #debug
    // gwssrv_debug_print ("gwssrv: serviceDrawText\n");

// Get values.
// #todo: Check the message code.
// wid, x, y, color.
    window_id = (int) message_address[4];
    x         = (unsigned long) message_address[5];
    y         = (unsigned long) message_address[6];
    color     = (unsigned long) message_address[7];
    window_id = (window_id & 0xFFFF);
    x = (x & 0xFFFF);
    y = (y & 0xFFFF);

// Text?
    /*
    //size 256 bytes
    unsigned char *text_buffer = (unsigned char *) &message_address[MSG_OFFSET_LONGSTRING];
    int s = sizeof(text_buffer);
    if(s<0 || s>= 256){
        gwssrv_debug_print ("serviceSetText: serviceDrawText [DEBUG]   SIZE \n");
        exit(1);
    }
    */

// Limits
// O texto começa fora dos limites da tela do dispositivo.
    //if( x >= deviceWidth )
        //return -1;
    //if( y >= deviceHeight )
        //return -1;

//
// == String support ========
//

// ==================================
// Get string from message
// #todo: Talvez poderiamos receber o tamanho da string.
    unsigned char buf[256+1];
    register int i=0;
    int string_off=8;
    char *p = (char *) &message_address[string_off];
    for (i=0; i<256; i++)
    {
        buf[i] = *p;  //Get a char
        p++;
    };
    buf[i] = 0;  // finalize the buffer.
// ==================================

    //#debug
    //printf("BUF: %s\n",buf);
    //while(1){}

//
// == Draw ===============================================
//

    //#todo
    //switch (alignment) {  ... }

// Window support.
// Se a janela alvo tem um índice fora dos limites

// wid

    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX )
    {
        printf("Invalid wid\n");
        goto fail;
    }

// window structure.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void*) window == NULL ){ 
        printf("Invalid window\n");
        goto fail;
    }
    if (window->magic != 1234){
        printf("window magic\n");
        goto fail;
    }

/*
// If this window is overlapped window,
// so paint into the client area.
    if (window->type == WT_OVERLAPPED){
        x += window->rcClient.left;
        y += window->rcClient.top;
    }
*/

    if ( window->type != WT_EDITBOX_SINGLE_LINE &&
         window->type != WT_EDITBOX_MULTIPLE_LINES )
    {
        printf("Invalid widnow type\n");
        goto fail;
    }

    if (window->textbuffer_size_in_bytes < 64)
    {
        printf("textbuffer_size_in_bytes\n");
        goto fail;
    }

    if ( (void*) window->window_text == NULL )
    {
        printf("window_text\n");
        goto fail;
    }
    memset( window->window_text, 0, 64 );

    window->text_size_in_bytes = 0;  // No text
    //128
    for (i=0; i<64; i++)
    {
        if ( (void*) window->window_text != NULL )
        {
            window->window_text[i] = (char) buf[i];
            window->text_size_in_bytes++;
        }
    };

    //#debug
    //printf("WINDOW: %s\n",window->window_text);
    //while(1){}

// Draw text
// Good window. Let's paint into it.
// #todo
// Maybe a flag can tell us to do that or not.

    //dtextDrawText ( 
    //    (struct gws_window_d *) window,
    //    x, y, (unsigned int) color, buf );

// #debug
// Flush the window into the framebuffer.
// #todo: invalidate, not show.
    //gws_show_window_rect(window);
    //invalidate_window(window);

// OK
    //printf("done\n");
    return 0;

crazy_fail:
    debug_print("serviceSetText: [ERROR] crazy_fail\n");
    return -1;
fail:
   printf("fail\n");
   return -1;
}


int serviceGetText(void)
{
// Get a text from a window.
// Editbox only.

    unsigned long *message_address = (unsigned long *) &__buffer[0];
    struct gws_window_d  *window;
    int window_id = -1;      // index 4
    unsigned long x;         // index 5
    unsigned long y;         // index 6
    unsigned long color;     // index 7
// Device context
    unsigned long deviceLeft  = 0;
    unsigned long deviceTop = 0;
    unsigned long deviceWidth = (__device_width  & 0xFFFF);
    unsigned long deviceHeight = (__device_height & 0xFFFF);

    // #debug
    // gwssrv_debug_print ("gwssrv: serviceDrawText\n");

// Get values.
// #todo: Check the message code.
// wid, x, y, color.
    window_id = (int) message_address[4];
    x         = (unsigned long) message_address[5];
    y         = (unsigned long) message_address[6];
    color     = (unsigned long) message_address[7];
    window_id = (window_id & 0xFFFF);
    x = (x & 0xFFFF);
    y = (y & 0xFFFF);

// Text?
    /*
    //size 256 bytes
    unsigned char *text_buffer = (unsigned char *) &message_address[MSG_OFFSET_LONGSTRING];
    int s = sizeof(text_buffer);
    if(s<0 || s>= 256){
        gwssrv_debug_print ("serviceGetText: serviceDrawText [DEBUG]   SIZE \n");
        exit(1);
    }
    */

// Limits
// O texto começa fora dos limites da tela do dispositivo.
    //if( x >= deviceWidth )
        //return -1;
    //if( y >= deviceHeight )
        //return -1;

//
// == Draw ===============================================
//

    //#todo
    //switch (alignment) {  ... }

// Window support.
// Se a janela alvo tem um índice fora dos limites

// wid
    if ( window_id < 0 || window_id >= WINDOW_COUNT_MAX )
    {
        printf("Invalid wid\n");
        return -1;
    }

// window structure.
    window = (struct gws_window_d *) windowList[window_id];
    if ( (void*) window == NULL )
    {
        printf("Invalid window\n");
        return -1;
    }
    if (window->magic != 1234)
    {
        printf("Invalid magic\n");
        return -1;
    }


/*
// If this window is overlapped window,
// so paint into the client area.
    if (window->type == WT_OVERLAPPED){
        x += window->rcClient.left;
        y += window->rcClient.top;
    }
*/

// Header
// wid, msg type, signature1, signature2.
    next_response[0] = (unsigned long) (window_id & 0xFFFFFFFF);
    next_response[1] = SERVER_PACKET_TYPE_REPLY;
    next_response[2] = 1234;
    next_response[3] = 5678;


    register int i=0;
    int string_off=8;
    char *p = (char *) &next_response[string_off];
    int gotten=0;

    if ( window->type != WT_EDITBOX &&
         window->type != WT_EDITBOX_MULTIPLE_LINES )
    {
        printf("Invalid window type\n");
        return -1;
    }

    if (window->textbuffer_size_in_bytes < 64)
    {
        printf("textbuffer_size_in_bytes\n");
        return -1;
    }

    if ( (void*) window->window_text == NULL )
    {
        printf("window_text\n");
        return -1;
    }

    //window->text_size_in_bytes = 0;  // No text

    //128
    for (i=0; i<64; i++)
    {
        // Copy the bytes from the window buffer to the
        // message buffer and finalize the message buffer.
        *p = window->window_text[i];
        p++;
        gotten++;
    };

// Finalize the string into the message buffer.
    *p = 0;

    if (gotten <= 0)
    {
        printf("Fail on getting text from window buffer\n");
        return -1;
    }

    //#debug
    //char *base = (char *) &next_response[string_off];
    //printf ("GOTTEN into the next_response buffer: %s\n", base );
    //while(1){}

// OK
    return 0;
crazy_fail:
    debug_print("serviceGetText: [ERROR] crazy_fail\n");
    return -1;
}






// O buffer é uma global nesse documento.
int 
servicelineBackbufferDrawHorizontalLine (void)
{
    unsigned long *message_address = (unsigned long *) &__buffer[0];
    // x1,y,x2,color
    unsigned long x1,y,x2,color;
    x1    = message_address[4];
    y     = message_address[5];
    x2    = message_address[6];
    color = message_address[7];
    grBackbufferDrawHorizontalLine ( x1, y, x2, (unsigned int) color );
    //gws_show_backbuffer(); // for debug   
    return 0;
}

//======================================

//#ugly
char *gwssrv_get_version(void)
{
    // ??
    return VERSION;
}


// The window server's main struture.
// See: gws.h
static void __init_ws_structure(void)
{

// The window server structure.
    window_server = 
        (struct gws_d *) malloc ( sizeof( struct gws_d) );
    if ( (void*) window_server == NULL ){
        gwssrv_debug_print("__init_ws_structure: [FAIL] window_server\n");
        printf            ("__init_ws_structure: [FAIL] window_server\n");
        exit(1);
    }
    memset( window_server, 0, sizeof(struct gws_d) );

    window_server->initialized = FALSE;
    window_server->version_major = VERSION_MAJOR;
    window_server->version_minor = VERSION_MINOR;

    // strings
    // #todo: we need to finalize these strings?

// name
    sprintf( window_server->name, "Gramado Window Server" );
    strcat(window_server->name,"\0");
// edition name
    sprintf( window_server->edition_name, "Presence" );
    strcat(window_server->edition_name,"\0");
// version string
    sprintf( window_server->version_string, "0.1" );
    strcat(window_server->version_string,"\0");
// We need to register the server in the host system.
    window_server->registration_status = FALSE;
// graphics initialization status.
    window_server->graphics_initialization_status = FALSE;
// Se devemos ou não lançarmos o primeiro client.
// #todo: Pegaremos essa informação dos parâmetros.
    window_server->launch_first_client = TRUE;
// When to quit the window server.
    window_server->quit = FALSE;
 // #todo
    window_server->status = 0;

// Not initialized yet.
// Why:
// Where gonna do this only at the end of the 
// server initialization.
}


// ========================================
// initGUI:
//     Initialize the graphics user interface.
// Init Graphics:
// Draw something.
// Init ws infrastructure.
// Initialize the '3D' graphics support.
// Let's create the standard green background.

static int initGUI(void)
{
    int graphics_status = -1;

//
// GUI
//

// gwsInit
// Initialize the window server infrastructure.
// The current display and the current screen.
// It will create the root window.
// See: gws.c

    int __init_status = (int) gwsInit();
    if (__init_status != 0){
        debug_print ("initGUI: [PANIC] Couldn't initialize the graphics\n");
        printf      ("initGUI: [PANIC] Couldn't initialize the graphics\n");
        //goto fail;
        exit(1);
    }

// Create root window.
// Create and update the taskbar window.
    initBackground();

// IN: height, flush.
// See: wm.c
    create_taskbar(METRICS_TASKBAR_DEFAULT_HEIGHT,FALSE);  
    wm_Update_TaskBar("Welcome!",TRUE);

    //#debug
    //gws_show_backbuffer();
    //while(1){}
//
// == checks ==============
//

// Check if we already have the root window.

    if ( (void*) __root_window == NULL ){
        gwssrv_debug_print ("initGraphics: [FAIL] root window doesn't exist\n");
        printf             ("initGraphics: [FAIL] root window doesn't exist\n");
        //goto fail;
        exit(1);
    }
    keyboard_owner = (void*) __root_window;
    mouse_owner    = (void*) __root_window;

// ok, no errors.
    return 0;
}

/*
 * on_execute: 
 *     + Initializes the gws infrastructure.
 *     + Create the background.
 *     + Create the taskbar.
 *     + Register window server as the current window server 
 *       for this desktop.
 *     + Create the server socket.
 *     + bind it.
 *     + Spawn a client process. (gwst.bin)
 *     + Enter in the main loop, waiting for some types of event.
 *       The possible events are: Reading messages from the kernel,
 *       or Reading our socket.
 *       In this moment we can send a response. It depends on the
 *       message found in the sockeck we readed.
 */

static int on_execute(int dm)
{
//==================
    struct sockaddr server_address;
    socklen_t addrlen;
    server_address.sa_family = AF_GRAMADO;
    server_address.sa_data[0] = 'w';
    server_address.sa_data[1] = 's';
    addrlen = sizeof(server_address);
//==================

    int flagUseClient = FALSE;  // TRUE
    int UseCompositor = TRUE;   // #debug flags
// Files
    int server_fd = -1; 
    int newconn = -1;
    int curconn = -1;
// Status
    int bind_status = -1;
    int _status = -1;
// Buffer
    char buf[32];
    int CanRead = -1;

    IsTimeToQuit = FALSE;
    IsAcceptingInput = TRUE;
    IsAcceptingConnections = TRUE;

// ==========================================
// Starting the initialization phases.
    Initialization.current_phase = 1;

// #test: Transparence.
    //gws_enable_transparence();
    gws_disable_transparence();
// Window server structure
    __init_ws_structure();
// Window manager
    wmInitializeGlobals();
    wmInitializeStructure();

/*
    FILE *input_fp;
    fflush(stdin);
    close(fileno(stdin));
    input_fp = fopen("kstdin.txt","r+");
    if( (void*) input_fp == NULL ){
        printf ("gwssrv: input_fp fail\n");
        exit(1);
    }
 */

/*
    //#test
    char display[100];
    host_name    = "gramado";
    display_number  = ":0";
    screen_number   = ".0";
    strcpy(display, host_name);
    strcat(display, display_number);
    strcat(display, screen_number);
    printf("DISPLAY={%s}\n",display);
    while(1){}
 */

// Invalidate the frame.
// Invalidate all the background.
// See: gws.c
    invalidate();
    invalidate_background();

// #todo:
// Initialize all the OS dependent stuff.
// ex: OsInit();

    // #debug
    gwssrv_debug_print ("GWSSRV.BIN: Initializing\n");

// Initialize the client list support.
    initClientSupport();
// The server is also a client.
    if ( (void*) serverClient == NULL ){
        printf ("ws: serverClient\n");
        while(1){}
        //exit(0);
    }
    if ( serverClient->used != TRUE || serverClient->magic != 1234 ){
        printf ("ws: serverClient validation\n");
        while(1){}
        //exit(0);
    } 

// Register
// #bugbug
// I don't know if we can register more than one time.
// We can fix it!
// Register.
// Register window server as the current window server 
// for this desktop.
// #bugbug: 
// Se tentarmos reiniciar o servidor, talvez
// nao consigamos registrar novamente, pois ja tem um registrado.
// Precisamos a opcao de desregistrar, para tentarmos 
// mais de um window server.
// See: connect.c

    _status = (int) register_ws();
    if (_status<0){
        gwssrv_debug_print ("gwssrv: Couldn't register the server\n");
        printf             ("gwssrv: Couldn't register the server\n");
        return -1;
        //exit(1);
    }
    window_server->registration_status = TRUE;


// Setup callback.
// Register this ring3 address as a callback.
// The kernel is gonna run this code frequently.
// Pra isso o ws precisa estar registrado.
// see: callback.c

    if (gUseCallback == TRUE){
        //callbackInitialize();
        //Initialization.setup_callback_checkpoint = TRUE;
    }

// ===============================================
// #todo
// Daqui pra frente é conexão com cliente.
// Lembrando que o servidor vai se conectar à mais de um cliente.
// ...
// #todo
// Aqui nos podemos criar vários sockets que serão usados
// pelo servidor.
// ex: CreateWellKnownSockets ();

// Creating a socket file and saving the fd.
    server_fd = (int) socket(AF_GRAMADO, SOCK_STREAM, 0);
    if (server_fd<0){
        gwssrv_debug_print ("gwssrv: Couldn't create the server socket\n");
        printf             ("gwssrv: Couldn't create the server socket\n");
        return -1;
        //exit(1);
    }
// Global variable.
    ____saved_server_fd   = (int) server_fd;
// Window server structure.
    window_server->socket = (int) server_fd;
// The server itself has its own client structure.
    serverClient->fd      = (int) server_fd;

    // #debug Breakpoint.
    //printf ("fd: %d\n", serverClient->fd);
    //while(1){}

// Associate this address with the server's socket fd.
    bind_status = 
        (int) bind (
                  server_fd, 
                  (struct sockaddr *) &server_address, 
                  addrlen );

    if (bind_status<0){
        gwssrv_debug_print ("gwssrv: [FATAL] Couldn't bind to the socket\n");
        printf             ("gwssrv: [FATAL] Couldn't bind to the socket\n");
        return -1;
    }

    // #debug Breakpoint.
    //printf ("fd: %d\n", serverClient->fd);
    //while(1){}

// Setup how many pending connections.
// SOMAXCONN is the default limit on backlog.
// see: sys/socket.h
    listen(server_fd,4);

    Initialization.setup_connection_checkpoint = TRUE;

// ==================================================
// Init GUI

    Initialization.current_phase = 2;

    int gui_status = -1;
    gui_status = (int) initGUI();
    if (gui_status<0){
        printf("on_execute: initGUI failed\n");
        return (int) -1;
    }

// ==================================================
// Init Graphics

    int graphics_status = (int) initGraphics();
    if (graphics_status<0){
        printf("on_execute: initGraphics failed\n");
        return (int) -1;
    }
    Initialization.setup_graphics_interface_checkpoint = TRUE;

// ----------------------------------------------
// Check window manager,
// Is it initialized?

    if (WindowManager.initialized != TRUE)
    {
       // Ohhhh
    }

    if( (void*) WindowManager.root == NULL ){
        gwssrv_debug_print("gwssrv: WindowManager.root fail\n");
                    printf("gwssrv: WindowManager.root fail\n");
        return -1;
        //exit(0);
    }
    if( (void*) WindowManager.taskbar == NULL ){
        gwssrv_debug_print("WindowManager.taskbar fail\n");
        printf("WindowManager.taskbar fail\n");
        return -1;
        //exit(0);
    }

// The working area.
    WindowManager.wa_left = 0;
    WindowManager.wa_top = 0;
// #danger
    WindowManager.wa_width = 
        WindowManager.root->width;
    WindowManager.wa_height =
        (WindowManager.root->height - WindowManager.taskbar->height);

// We already did that before. Its ok.
    WindowManager.initialized = TRUE;

    //#debug
    //Not working
    /*
    printf ("wa: %d %d %d %d",
        WindowManager.wa_left,
        WindowManager.wa_top,
        WindowManager.wa_width,
        WindowManager.wa_height );
    */
// ===============

    //gws_show_backbuffer();
    //while(1){}
    // #debug
    //printf ("fd: %d\n", serverClient->fd);
    //while(1){}


// Child

    /*
    if ( window_server->launch_first_client == TRUE )
    {
        // #bugbug
        // We can't use this function in ring0.
    }
    */

    // #debug
    //printf ("fd: %d\n", serverClient->fd);
    //while(1){}


//
// Client
//

/*
    if ( flagUseClient == TRUE )
    {
        //debug_print ("gwssrc: Calling client $\n");
        //rtl_clone_and_execute("terminal.bin");
    }
*/

// Launches the default Display Manager
    if(dm==TRUE){
        rtl_clone_and_execute("gdm.bin");
    }

//
// =======================================
//

//
// Loop
//

// #todo
// Isso é um teste.
// #atenção
// Provavelmente precisamos de um loop
// contendo accept, read e write.
// Não presizamos criar o socket novamente,
// mas temos que refazer a conecção toda vez
// que enviarmos uma resposta.
// Então logo após enviarmos a resposta precisamos
// fechar o arquivo? Acabaria com o socket?
// #atenção:
// A função accept vai retornar o descritor do
// socket que usaremos ... por isso poderemos fecha-lo
// para assim obtermos um novo da próxima vez.


//#todo:
// No loop precisamos de accept() read() e write();

    // Not used for now.
    connection_status = 1;

    //curconn = serverClient->fd;
    newconn = -1;

// ========================================================

//
// Loop
//

// Set foreground thread.
// Get events scanning a queue in the foreground queue.
// + Accept connection from a client.
// + Call the dispatcher to porcess the message.
// See:
// https://man7.org/linux/man-pages/man2/accept.2.html
// see: wm.c
// #todo
// Os possíveis métodos de input
// podem ser aprimorados com o tempo.
// Dispatch
// Não pode ser o fd do servidor.
// >> accept esta pegando um arquivo
// na fila e sempre devolve o fd 31.
// Chamamos o accepr soment quando
// o servidor estiver aceitando conexoes.

    gwssrv_debug_print ("gwssrv: Entering main loop.\n");
    rtl_focus_on_this_thread();

// ==========================================
// Finalize the ws structure initialization.
    window_server->status = STATUS_RUNNING;
    window_server->initialized = TRUE;
    Initialization.ws_struct_checkpoint = TRUE;

// ==========================================
// Main loop.
    IsTimeToQuit = FALSE;
    running = TRUE;
    Initialization.inloop_checkpoint = TRUE;

// The last initialization phase.
    Initialization.current_phase = 3;

    // Given 'n' seconds with no requests or input events,
    // so we can run the screen saver animation.
    
    /*
    demoCat();
    demoLine1();
    demoLines();
    demoPolygon();
    demoPolygon2();
    demoTriangle();
    demoCurve();
    */

    while (running == TRUE){

        if (IsTimeToQuit == TRUE){ break; };

        // Respecting the circular queue.
        if (IsAcceptingInput == TRUE){
            wmInputReader();
        }

        if (IsAcceptingConnections == TRUE)
        {
            newconn = accept ( 
                ____saved_server_fd,
                (struct sockaddr *) &server_address, 
                (socklen_t *) addrlen );

             // sys_accept() pega na fila e coloca em fd=31.
             if (newconn == 31){
                 dispacher(newconn);
             }

            //if (newconn <= 0){
                //gwssrv_debug_print("gwssrv: accept returned FAIL\n");
            //}

            //#debug
            if ( newconn == ____saved_server_fd ){
                printf("GWSSRV.BIN: newconn == ____saved_server_fd\n");
                while(1){}
            }
            //close(newconn);
        }

        // ##
        // We're calling this because we don't have a
        // callback anymore.
        // This was the jog of the callback.
        // #todo: We need a timer.
        compose();

        // Not accpeting
        //if (IsAcceptingConnections == FALSE){
            //close(newconn);
        //}
    };

// =======================================

    if (IsTimeToQuit != TRUE){
        debug_print ("gwssrv: [ERROR] Invalid IsTimeToQuit\n");
        return -1;
    }

// #todo
// Now we will close the window server.  
// Free all the structure, one by one in cascade.
// See: 'gws' structure in gws.h
// We will call the kernel to unregister the window server.
// We will close all the sockets.
// ...

// Close the server's fd.
    if (server_fd>0){
        close(server_fd);
    }

// Teturn to main()
    return 0; 
}

void gwssrv_quit(void)
{
    IsTimeToQuit = TRUE;
}

/*
 // We can't do this in ring 3.
static inline void __outb(uint16_t port, uint8_t val)
{
    asm volatile ( "outb %0, %1" : : "a"(val), "Nd"(port) );
}
*/

// main: Entry point.
int main (int argc, char **argv)
{
    int Status=-1;
    register int i=0;

// #test: Flags
    int f1= FALSE;
    int f2= FALSE;
    int f3= FALSE;
    int f4= FALSE;
    int fDisplayManager=FALSE;

    char shutdown_string[64];

    if (argc>0)
    {
        for (i=0; i<argc; i++)
        {
            //printf("%d: {%s}\n",i,argv[i]);

            if ( strncmp( argv[i], "-1", 2 ) == 0 )
                f1=TRUE;
            if ( strncmp( argv[i], "-2", 2 ) == 0 )
                f2=TRUE;
            if ( strncmp( argv[i], "-3", 2 ) == 0 )
                f3=TRUE;
            if ( strncmp( argv[i], "-4", 2 ) == 0 )
                f4=TRUE;

            // Launches the default Display Manager.
            if ( strncmp( argv[i], "--dm", 4 ) == 0 )
                fDisplayManager=TRUE;
        };
    }

    if (f1 == TRUE)
        printf("F1\n");
    if (f2 == TRUE)
        printf("F2\n");
    if (f3 == TRUE)
        printf("F3\n");
    if (f4 == TRUE)
        printf("F4\n");
    if (fDisplayManager == TRUE)
        printf("fDisplayManager\n");

    //exit(0);


// Callback support.
// O callback tem feito o refresh de muita coisa.
// Ficar sem ele afeta muito a interface.
// Mas isso esta aqui para testes.
    //gUseCallback = TRUE;
    gUseCallback = FALSE;

    Initialization.current_phase = 0;
    Initialization.ws_struct_checkpoint = FALSE;
    Initialization.wm_struct_checkpoint = FALSE;
    Initialization.register_ws_checkpoint = FALSE;
    Initialization.setup_callback_checkpoint = FALSE;
    Initialization.setup_connection_checkpoint = FALSE;
    Initialization.setup_graphics_interface_checkpoint = FALSE;
    Initialization.inloop_checkpoint = FALSE;

    // ##
    // Sincronizaçao provisoria.
    // Vamos precisar disso antes de tudo;
    // vamos pegar a que foi criada pelo primeiro cliente.
    // ele cria no começo da rotina.
    // Dai usaremos essa id por enquanto, pois o sistema so tem ela ainda.

/*
    while(1){
        __saved_sync_id = sc82 (10005,0,0,0);
        if( __saved_sync_id > 0 && __saved_sync_id < 1024 )
            break;
    };
 */

//0 = Time to quit.
    Status = (int) on_execute(fDisplayManager);

    // Shutdown the server.
    if (Status == 0)
    {
        gwssrv_debug_print ("GWSSRV.BIN: exit(0)\n");
        printf             ("GWSSRV.BIN: exit(0)\n");

        // Clear root window.
        if ( (void*) __root_window != NULL )
        {
            clear_window_by_id( __root_window->id, TRUE );
            memset(shutdown_string, 0 , 64);
            strcat(shutdown_string,"ws: Shutting down ...");
            strcat(shutdown_string,"\0");
            dtextDrawText ( 
                (struct gws_window_d *) __root_window,
                8, 
                8, 
                (unsigned int) COLOR_WHITE, 
                shutdown_string );
            wm_flush_window(__root_window);
            close( ____saved_server_fd );
            rtl_clone_and_execute("shutdown.bin");
            exit(0);
        }
        exit(0);
    }

// #todo
// Unexpected fail.
    gwssrv_debug_print("GWSSRV.BIN: Hang on exit\n");
    printf            ("GWSSRV.BIN: Hang on exit\n");
    while(1){
    };

//exit0:
    return 0;
}

//
// End
//

