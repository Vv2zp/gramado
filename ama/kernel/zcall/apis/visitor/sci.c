
// sci.c
// (System Call Interface).
// System Service Dispatcher.
//
// Handlers for the system interrupts.
// #todo: 
// There is another point:
// The 'event delivery' and 'return' support.

#include <kernel.h>

static unsigned long __default_syscall_counter=0;

//globals
//see:sw.asm
unsigned long sci0_cpl=0;
unsigned long sci1_cpl=0;
unsigned long sci2_cpl=0;

//
// == private functions: prototypes =============
//


static void __service897(void);
static void __servicePutChar(int c);
static void __invalidate_surface_rectangle(void);

static void __setup_surface_rectangle(
    unsigned long left,
    unsigned long top,
    unsigned long width,
    unsigned long height );

// =============================

/*
 * __servicePutChar:
 * Movendo para terminal/output.c 
 * Coloca um char usando o 'terminal mode' de stdio selecionado em _outbyte.
 * stdio_terminalmode_flag = nao transparente.
 */
static void __servicePutChar(int c)
{
// Put char into the fg console.
// Local worker.

    if (fg_console < 0){
        return;
    }
    console_putchar ( (int) c, fg_console );
}

// #todo: Return 'int'.
void newos_reboot(unsigned long reboot_flags)
{
    sys_reboot(reboot_flags);
}

unsigned long newos_get_system_metrics(int index)
{
    if (index<0){
        return 0;
    }
    return (unsigned long) doGetSystemMetrics ( (int) index );
}

pid_t newos_getpid(void)
{
    return (pid_t) sys_getpid();
}

// REAL (coloca a thread em standby para executar pela primeira vez.)
// MOVEMENT 1 (Initialized --> Standby).
int newos_start_thread(struct thread_d *thread)
{

// Validation
    if ((void*) thread == NULL)
        return (-1);
    if (thread->used != TRUE)
        return (-1);
    if (thread->magic != 1234)
        return (-1);

    SelectForExecution( (struct thread_d *) thread );
    return 0;
}

int newos_get_current_runlevel(void)
{
    return (int) current_runlevel;
}

unsigned long newos_get_memory_size_mb(void)
{
    unsigned long __mm_size_mb = 
        (unsigned long) (memorysizeTotal/0x400);

    return (unsigned long) __mm_size_mb;
}

// Usado pelo malloc em ring3.
void *newos_alloc_shared_ring3_pages(pid_t pid, int number_of_bytes)
{
    int number_of_pages=0;

// #todo
// pid premission

// #todo
// Check max limit

    if ( number_of_bytes < 0 )
        number_of_bytes = 4096;

    if ( number_of_bytes <= 4096 ){
        return (void *) allocPages(1);
    }

// Alinhando para cima.
    number_of_pages = (int) ((number_of_bytes/4096) + 1);

    return (void *) allocPages(number_of_pages);
}


// 34 - Setup cursor for the current virtual console.
// See: core/system.c
// IN: x,y
// #todo: Essa rotina dever pertencer ao user/
void newos_set_cursor( unsigned long x, unsigned long y )
{

// #todo
// Maybe check some limits.

    set_up_cursor ( 
        (unsigned long) x, 
        (unsigned long) y );
}




static void __service897(void)
{
    struct thread_d *myThread; 
    struct rect_d r;
    unsigned int _Color=0;


// Current thread
// This routine only can be called by the 
// init process. Its tid is INIT_TID.
    if ( current_thread < 0 || current_thread >= THREAD_COUNT_MAX ){
        return;
    }
    if (current_thread != INIT_TID){
        return;
    }

    _Color = (unsigned int) (COLOR_GREEN + 0);

// Setup rectangle
    r.left = 0;
    r.top = 0;
    r.width = 24;
    r.height = 24;
    r.dirty = FALSE; 
    r.used = TRUE;
    r.magic = 1234;

// Paint
// Invalidate means that the rectangle need to be flushed
// into the framebuffer.
// If the rectangle is dirty, so it needs to be flushed into 
// the framebuffer.
// When we draw a window it needs to be invalidated.

    backbuffer_draw_rectangle( 
        r.left, r.top, r.width, r.height, 
        _Color, 
        0 );      // #todo: rop flags.

    r.dirty = TRUE;

// Setup surface rectangle.

//  Thread
    myThread = (struct thread_d *) threadList[current_thread];
// Valid thread
    if ( (void*) myThread != NULL )
    {
        if ( myThread->used == TRUE && myThread->magic == 1234 ){
            myThread->surface_rect = (struct rect_d *) &r;
            return;
        }
    }
}

static void __setup_surface_rectangle(
    unsigned long left,
    unsigned long top,
    unsigned long width,
    unsigned long height )
{
    struct thread_d *t;
    struct rect_d *r;

/*
// dc: Clippint
    unsigned long deviceWidth  = (unsigned long) screenGetWidth();
    unsigned long deviceHeight = (unsigned long) screenGetHeight();
    if ( deviceWidth == 0 || deviceHeight == 0 ){
        debug_print ("__setup_surface_rectangle: [PANIC] w h\n");
        panic       ("__setup_surface_rectangle: [PANIC] w h\n");
    }
*/

    if ( current_thread<0 || current_thread >= THREAD_COUNT_MAX ){
        return;
    }

// Thread
    t = (struct thread_d *) threadList[current_thread];
    if ((void*) t == NULL){
        return;
    }
    if (t->magic != 1234){
        return;
    }

// Rect
    r = t->surface_rect;
    if ((void*) r == NULL){
        return;
    }
    if (r->magic != 1234){
        return;
    }

// Rect data
    r->left   = (left   & 0xFFFF);
    r->top    = (top    & 0xFFFF);
    r->width  = (width  & 0xFFFF);
    r->height = (height & 0xFFFF);
    r->dirty = FALSE;
}

static void __invalidate_surface_rectangle(void)
{
    struct thread_d *t;
    struct rect_d *r;

    if ( current_thread<0 || current_thread >= THREAD_COUNT_MAX ){
        return;
    }

// Thread
    t = (struct thread_d *) threadList[current_thread];
    if ((void*) t == NULL){
        return;
    }
    if (t->magic != 1234){
        return;
    }

// Rect
    r = t->surface_rect;
    if ((void*) r == NULL){
        return;
    }
    if (r->magic != 1234){
        return;
    }

// Rect data
    r->dirty = TRUE;
}

// =====================================================

// Handler for the interrupt 0x80.
void *sci0 ( 
    unsigned long number, 
    unsigned long arg2, 
    unsigned long arg3, 
    unsigned long arg4 )
{
// E25 - The queen.
// Getting requests from ring3 applications via systemcalls.
// #test
// It's ok when init.bin calls this systemcall.

    struct process_d  *p;
    //struct thread_d  *t;

    unsigned long *message_address = (unsigned long *) arg2;

    struct zing_hook_d *__zh;

    unsigned long *a2 = (unsigned long*) arg2;
    unsigned long *a3 = (unsigned long*) arg3;
    unsigned long *a4 = (unsigned long*) arg4;

    // Not in use!
    //char *aa2 = (char *) arg2;
    //char *aa3 = (char *) arg3;
    //char *aa4 = (char *) arg4;

    pid_t current_process = (pid_t) get_current_pid();
    int layer = LAYER_UNDEFINED;

//cpl
    unsigned long *cpl_buffer = (unsigned long *) &sci0_cpl;
    int cpl=-1;
    unsigned long tmp_cpl = (unsigned long) cpl_buffer[0];
    cpl = (int) (tmp_cpl & 3);

    if ( cpl != 0 && cpl != 1 && cpl != 2 && cpl != 3 )
    {
        panic("sci0: cpl");
    }

    if (cpl == 0){
        printf("number=%d\n",number);
        printf("pid=%d\n",current_process);
        //if(current_process != 0){
            panic("sci0: cpl 0\n");
        //}
    }
    if (cpl == 1){
        panic("sci0: cpl 1\n");
    }
    if (cpl == 2){
        panic("sci0: cpl 2\n");
    }
    if (cpl == 3){
        // ok
    }

    // #debug
    //debug_print("sc0:\n");
    //printf("sc0:\n");

    //?? #deprecated?
    g_profiler_ints_gde_services++;

// Profiling in the process structure.

// Permission

    if (current_process<0 || current_process >= PROCESS_COUNT_MAX){
        panic("sci0: current_process\n");
    }
    p = (struct process_d *) processList[current_process];
    if ((void*) p == NULL){
        debug_print("sci0: p\n");
        panic("sci0: p\n");
    }
    if ( p->used != TRUE || p->magic != 1234 ){
        debug_print("sci0: p validation\n");
        panic("sci0: p validation\n");
    }

    layer = p->_layer;

// Feito pra rodar no Gramado OS.
    if (p->personality != PERSONALITY_GRAMADO_AMAZING)
    {
        debug_print("sci0: Personality\n");
        panic      ("sci0: Personality\n");
    }

// Counting ...
    p->syscalls_counter++;


// What is the number for SYS_EXIT?
// #todo: Put this in the right spot.
// exit
// Marcamos no processo nossa intenção de fechar.
// Marcamos na thread de controle nossa intenção de fechar.
// #todo: as outras threads do processo.
    if (number == SYS_EXIT)
    {
        debug_print("sci0: SYS_EXIT\n");
        p->exit_in_progress = TRUE;
        // Quando o scheduler passar por ela,
        // vai pular ela e marca-la como zombie.
        if ((void*) p->control != NULL)
        {
            if (p->control->magic == 1234){
                p->control->exit_in_progress = TRUE;
            }
        }
        return NULL;
    }

// ==== 0 mark ====================================================


    // SYS_NULL
    if (number == 0)
        return NULL;


// Business Logic:
// 1 (i/o) Essa rotina pode ser usada por 
// um driver em user mode.
// #todo: This operation needs permition.
// #todo: Return value.
// IN: buffer address and lba address.
    if (number == SYS_READ_LBA){
        storage_read_sector( (unsigned long) arg2, (unsigned long) arg3 ); 
        return NULL;
    }

// Business Logic:
// 2 (i/o) Essa rotina pode ser usada por 
// um driver em user mode.
// #todo: This operation needs permition.
// #todo: Return value.
// IN: buffer address and lba address.
    if (number == SYS_WRITE_LBA){ 
        storage_write_sector( (unsigned long) arg2, (unsigned long) arg3 ); 
        return NULL;
    }

// Business Logic:
// 3 
// Carregar um arquivo do disco para a memória.
// See: fs/fs.c
// IN: name, flags, mode
    if (number == SYS_READ_FILE)
    {
        //if ( (void*) a2 == NULL )
           //return NULL;
        return (void *) sys_read_file_from_disk ( 
                            (char *) a2, 
                            (int)    arg3, 
                            (mode_t) arg4 ); 
    }

// Business Logic:
// 4 
// Save file.
// See: fs/fs.c
// IN: name, size in sectors, size in bytes, adress, flag.
    if (number == SYS_WRITE_FILE)
    {
        sys_write_file_to_disk ( 
            (char *)        message_address[0],
            (unsigned long) message_address[1],
            (unsigned long) message_address[2],
            (char *)        message_address[3],
            (char)          message_address[4] );
        return NULL;
    }

// Business Logic:
// 5
// See: sys.c 
    if (number == SYS_VSYNC)
    {
        sys_vsync();
        return NULL;
    }

// Business Logic:
// 6 - Put pixel. 
// Coloca um pixel no backbuffer.
// Isso pode ser usado por um servidor. 
// cor, x, y, rop_flags.
// todo: chamar kgws_backbuffer_putpixel
    if (number == SYS_BUFFER_PUTPIXEL)
    {
        backbuffer_putpixel ( 
            (unsigned long) a2,  // color
            (unsigned long) a3,  // x
            (unsigned long) a4,  // y
            0 );                 // rop_flags
        return NULL;
    }

// 7

// Business Logic:
// 8 
// #todo: #bugbug: 
// Aqui precisamos de mais parâmetros.
    if (number == SYS_BUFFER_DRAWLINE)
    {
        backbuffer_draw_horizontal_line ( 
            (unsigned long) a2, 
            (unsigned long) a3, 
            (unsigned long) a4, 
            COLOR_WHITE,
            0 );   // rop_flags
        return NULL;
    }

// Business Logic:
// 9 - Draw a rectangle into the backbuffer.
// SYS_BUFFER_DRAWRECT
// see: rect.c
    if (number == 9)
    {
        //debug_print("sci0: [9]\n");
        backbuffer_draw_rectangle ( 
            (unsigned long) message_address[0],    //x 
            (unsigned long) message_address[1],    //y
            (unsigned long) message_address[2],    //width
            (unsigned long) message_address[3],    //height
            (unsigned int)  message_address[4],    //color
            (unsigned long) message_address[5] );  //rop_flags
        return NULL;
    }

// Business Logic:
// 10 - Refresh rectangle.
// Region?
    if (number == 10)
    {
        //debug_print("sci0: [10]\n");
        refresh_rectangle ( 
            (unsigned long) message_address[0],    //x 
            (unsigned long) message_address[1],    //y
            (unsigned long) message_address[2],    //width
            (unsigned long) message_address[3] );  //height 
        return NULL;
    }


// Business Logic:
// 11:
// Flush the backbuffer into the lfb.
    if (number == SYS_REFRESHSCREEN)
    { 
        invalidate_screen();
        return NULL;
    }

// Reserved.
// Netword: 12,13,14,15

// Business Logic: 
// 16 - open() implementation.
// In ring3, see: fcntl.c
// In ring0, see: fs.c
// IN: pathname, flags, mode
// OUT: fd
    if (number == SYS_OPEN)
    {
        debug_print ("sci0: SYS_OPEN\n");
        //if ( (void*) arg2 == NULL )
            //return NULL;
        return (void *) sys_open ( 
                                (const char *) arg2, 
                                (int)          arg3, 
                                (mode_t)       arg4 ); 
    }

// Business Logic:
// 17 - close() implementation.
// See: sys.c
// IN: fd
    if (number == SYS_CLOSE)
    {
        debug_print ("sci0: SYS_CLOSE\n");
        //if (arg2 < 0)
            //return NULL;
        return (void *) sys_close( (int) arg2 );
    }

// Business Logic:
// 18 - read() implementation.
// IN: ?
// See: sys.c
    if (number == SYS_READ)
    {
        return (void *) sys_read ( 
                            (unsigned int) arg2, 
                            (char *)       arg3, 
                            (int)          arg4 ); 
    }

// Business Logic:
// 19 - write() implementation
// IN: ?
// See: sys.c
    if (number == SYS_WRITE)
    {
        return (void *) sys_write ( 
                            (unsigned int) arg2, 
                            (char *)       arg3, 
                            (int)          arg4 );  
    }


// == + ==================================================


// Reserved:
// 20~23 - Buffers support 
// 24~28 - Surface support 
// 29 - String stuff
// 30,31,32 - Putpixel?
// 33 - free number.

// Business Logic:
// 34
// Setup cursor position for the current virtual console.
    if (number == SYS_VIDEO_SETCURSOR)
    { 
        newos_set_cursor((unsigned long) arg2, (unsigned long) arg3);
        return NULL;
    }

// 35 - free number

// 36 - #deprecated
// 37 - #deprecated

// Business Logic:
// 38 - get host name
    if (number == SYS_GETHOSTNAME){
        return (void *) __gethostname((char *) arg2);
    }

// Business Logic:
// 39 - set host name 
// #todo: This operation needs permition?
    if (number == SYS_SETHOSTNAME){
        return (void *) __sethostname((const char *) arg2);
    }

// Business Logic:
// 40 - get user name 
    if (number == SYS_GETUSERNAME){
        return (void *) __getusername((char *) arg2);
    }

// Business Logic:
// 41 - Set user name 
// #todo: This operation needs permition?
    if (number == SYS_SETUSERNAME){
        return (void *) __setusername((const char *) arg2);
    }

// 42 - #deprecated

// Business Logic:
// 43 - Create an empty file.
// Called by libgws.
// See: fs.c
    if (number == 43){
        return (void *) sys_create_empty_file( (char *) arg2 );
    }

// Business Logic:
// 44 - Create an empty directory.
// Called by libgws.
// See: fs.c
    if (number == 44){
        return (void *) sys_create_empty_directory( (char *) arg2 );
    }

// 45 - livre
// #todo: Usar para manipulação de arquivo ou diretório.
// 46 - livre
// 47 - livre
        // 48 - livre

// 49
// Show system info
// See: sys.c
    if (number == 49){
        sys_show_system_info((int) arg2);
        return NULL;
    }

// ...

// Business Logic:
// 65
// Put a char in the current virtual console.
// see: console.c
// IN: ch, console id.
    if (number == SYS_KGWS_PUTCHAR)
    {
        //if (arg3<0)
            //return NULL;
        console_putchar ( (int) arg2, (int) arg3 ); 
        return NULL;
    }

//
// ## EXIT ##
//

// 70 - Exit.
// Atende a funcao exit() da libc. 
// Criaremos um 'request' que sera atendido somente quando 
// houver uma interrupcao de timer. 
// Enquanto isso a thread deve esperar em um loop.
// #bugbug: Pode haver sobreposicao de requests?
// Assincrono.
// IN: ??
// #todo: 
// Criar um wrapper em sys.c ou exit.c
// See: request.c
// Request number 12. (Exit thread)

    // #todo: See at the beginning of this routine.
    if (number == SYS_EXIT){
        panic("sci.c: SYS_EXIT\n");
        return NULL;
    }

// 71 - fork?

// Business Logic:
// 72
// See: sys.c
// Cria uma thread que fica no estado INITIALIZED.
// Outra syscall tem que colocar ela em STANDBY.
    if (number == SYS_CREATETHREAD)
    {
        debug_print("sci0: [FIXME] SYS_CREATETHREAD\n");
        return (void *) sys_create_thread (
                            NULL,
                            arg2,             // init eip
                            arg3,             // init stack
                            current_process,  // pid
                            (char *) a4 );    // name
    }

// Business Logic:
// 73
// See: sys.c
// Cria um processo e coloca a thread primária pra rodar.
// #bugbug: 
// Na inicializacao do kernel, nos criamos um processo
// usando create_process. Mas nesse momento estavamos usando
// o diretorio de paginas do kernel e os registradores de segmento
// pertenciam ao kernel.
// Nessa tentativa de criarmos um processo usando create_process
// as coisas estao um pouco diferentes ... provavelmente
// estamos usando o diretorio de paginas do processo e os
// registradores de segmento podem estar em ring3.
// ?? Talvez poderiamos criar um request, da mesma maneira 
// que fazemos com a criaçao de threads e o spawn.
// #todo
// Aqui no kernel, precisamos criar mais rotinas de suporte
// a criacao de processos.
// Temos poucas opçoes e tudo esta meio fora de ordem ainda.
// syscall: 
// arg2 = name
// arg3 = process priority
// arg4 = nothing

    if (number == SYS_CREATEPROCESS)
    {
        debug_print("sci0: [FIXME] SYS_CREATEPROCESS\n");
        return (void *) sys_create_process ( 
                            NULL,             // zing hook
                            0,                // Reserved
                            arg3,             // priority
                            current_process,  // ppid
                            (char *) a2,      // name
                            RING3 );          // iopl 
    }

// 74,75,76,77,78,79.

// 80 - Show current process info.
// #todo: Mostrar em uma janela própria.
// #todo: Devemos chamar uma função que 
// mostre informações apenas do processo atual. 
    if (number == SYS_CURRENTPROCESSINFO)
    {
        show_currentprocess_info();
        return NULL;
    }

// Business Logic:
// 81: Get parent process id.
// See: sys.c
    if (number == SYS_GETPPID){ 
        return (void *) sys_getppid();
    }

// 82 - Mostra informações sobre todos os processos.
    if (number == 82){
        show_process_information();
        return NULL;
    }

// 83
// Suporte a chamada da libc waitpid(...).
// schedi.c
// #todo.
// TID, PID 
// TID eh a thread atual.
// PID veio via argumento.
// IN: pid, status, option
// #todo: Change the name to sys_xxxx
    if (number == SYS_WAIT4PID)
    { 
        debug_print("sci0: [FIXME] SYS_WAIT4PID\n");
        return (void *) do_waitpid( 
                            (pid_t) arg2, 
                            (int *) arg3, 
                            (int)   arg4 );
                
       //block_for_a_reason ( (int) current_thread, (int) arg2 ); //suspenso
    }

// 84 - livre.

// Business Logic:
// 85
    if (number == SYS_GETPID){
        return (void *) get_current_pid();
   }

// 86,87 - livre.

// Testa se o processo é válido
// se for valido retorna 1234
// testando ...
    if (number == SYS_88){
        return (void *) processTesting(arg2);
    }

// 89 - livre

// ------------------
// 90~99 Reservado para thread support

// Business Logic:
// 94
    if (number == SYS_STARTTHREAD){
        debug_print("sci0: SYS_STARTTHREAD\n");
        return (void *) newos_start_thread((struct thread_d *) arg2);
    }

// ------------------

// 100~109: free

// Business Logic:
// 110
// IN: flags.
// see: ke/sys.c
    int reb_ret=-1;
    if (number == SYS_REBOOT){
        debug_print("sci0: SYS_REBOOT\n");
        reb_ret = (int) sys_reboot(0);
        return (void *) (reb_ret & 0xFFFFFFFF);
    }

// Business Logic:
// 111
// See: tlib.c
// Get the next system message.
// IN: buffer for message elements.
    if (number == 111){
        //debug_print("sci0: 111\n");
        return (void *) sys_get_message( (unsigned long) &message_address[0] );
    }

// Business Logic:
// tlib.c
// Post message to tid.
// Asynchronous.
// IN: tid, message buffer address.
    if (number == 112){
        //if (arg2<0)
            //return NULL;
        return (void *) sys_post_message_to_tid( 
                            (int) arg2, 
                            (unsigned long) arg3 );
    }

// 113~117: 
// 118~119: 

// Pop data from network queue.
// IN: user buffer, buffer lenght.
    if (number == 118){
        return (void*) network_pop_packet( 
                           (unsigned long) &message_address[0], 
                           (int) arg3 );
    }

// Push data into the network queue?
    //case 119:
        //break;

// 120
// Get a message given the index.
// With restart support.
    if (number == 120)
    {
        //debug_print("sci0: \n");
        return (void *) sys_get_message2( 
                (unsigned long) &message_address[0], arg3, arg4 );
    }

// 121,122,123

// 124 (teste)
// Defered system procedure call.
// #todo: 
// Precisamos armazenasr os argumentos em algum lugar.
// #bugbug: Precisamos criar um request.
    if (number == 124){
        kernel_request = KR_DEFERED_SYSTEMPROCEDURE;
        return NULL;
    }

// 125

// Business Logic:
// 126
// Permitindo que drivers e servidores em usermode acessem as portas.
// #todo: This operation needs permition?
// #bugbug
// #todo: 
// Tem que resolver as questoes de privilegios.
// IN: bits, port
    if (number == SYS_USERMODE_PORT_IN)
    {
        return (void *) portsx86_IN ( 
                            (int) (arg2 & 0xFFFFFFFF), 
                            (unsigned short) (arg3 & 0xFFFF) );
    }

// Business Logic:
// 127
// Permitindo que drivers e servidores em usermode acessem as portas.
// #todo: This operation needs permition?
// #bugbug
// #todo: 
// Tem que resolver as questoes de privilegios.
// IN: bits, port, value
    if (number == SYS_USERMODE_PORT_OUT)
    {
        portsx86_OUT ( 
            (int) arg2, 
            (unsigned short) (arg3 & 0xFFFF), 
            (unsigned int)   (arg4 & 0xFFFFFFFF) );
        return NULL;
    }

// 128~129: free
// --------
// 130, 131

// 132 - d_draw_char
// Desenha um caractere e pinta o pano de fundo.
// #todo: We do not have an api routine yet.
// IN: x, y, c, fg color, bg color
    if (number == 132)
    { 
        d_draw_char(
            (unsigned long)  message_address[0],    // x
            (unsigned long)  message_address[1],    // y 
            (unsigned long)  message_address[2],    // c
            (unsigned long)  message_address[3],    // fg
            (unsigned long)  message_address[4] );  // bg
        return NULL;
    }

// 133 - d_drawchar_transparent
// Desenha um caractere sem alterar o pano de fundo.
// IN: x, y, color, c
    if (number == 133)
    {
        d_drawchar_transparent(
            (unsigned long)  message_address[0],   // x
            (unsigned long)  message_address[1],   // y 
            (unsigned long)  message_address[2],   // color
            (unsigned long)  message_address[3] ); // c
        return NULL;
    }

// 134~136: free

// 137 - #deprecated?
    if (number == SYS_GETCH)
    {
        panic("SYS_GETCH: #deprecated?\n"); 
        return NULL;
    }

// 138 - Get key state.
// IN: vk.
    if (number == 138){
        return (void *) keyboardGetKeyState((unsigned char) arg2);
    }

// 139

// -----------------
// 140~149
// -----------------

// 150~156 User and group support.

// 152 - get uid
    if (number == SYS_GETCURRENTUSERID){
        return (void *) current_user; 
    }

// 154 - get gid
    if (number == SYS_GETCURRENTGROUPID){
        return (void *) current_group; 
    }

// 157~159: Security

// 157 - get user session id
    if (number == SYS_GETCURRENTUSERSESSION){
        return (void *) current_usersession; 
    }

// 158 - free

// 159 - get current zing hook id
    if (number == SYS_GETCURRENTDESKTOP){
        return (void *) current_zh; 
    }

// ----------------
// 160~169: Reserved to network support.

// 161
// get socket IP
// Gramado API socket support. (not libc)
    if (number == 161){
        return (void *) getSocketIPV4( (struct socket_d *) arg2 );
    }

// 162
// get socket port
// Gramado API socket support. (not libc)
    if (number == 162){
        return (void *) getSocketPort( (struct socket_d *) arg2 );
    }

// 163
// update socket  
// retorno 0=ok 1=fail
// Gramado API socket support. (not libc)
    if (number == 163){
        return (void *) update_socket ( 
                            (struct socket_d *) arg2, 
                            (unsigned int)      (arg3 & 0xFFFFFFFF), 
                            (unsigned short)    (arg4 & 0xFFFF) );
    }

// ----------------

// 170 - command 'pwd'.
// Cada processo tem seu proprio pwd.
// Essa rotina mostra o pathname usado pelo processo.
// See: fs.c
// #test
// Isso é um teste. Essa chamada não precisa disso.
    if (number == SYS_PWD)
    {
        if (is_superuser() == TRUE)
        {
            debug_print("sci0: [SYS_PWD] Yes, I'm the super user\n");
            printf     ("sci0: [SYS_PWD] Yes, I'm the super user\n");
        }
        sys_pwd();
        return NULL;
    }

// 171 - retorna o id do volume atual.
    if (number == SYS_GETCURRENTVOLUMEID){
        return (void *) current_volume;
    }

//172 - configura o id do volume atual.
//#bugbug: Estamos modificando, sem aplicar nenhum filtro.
    if (number == SYS_SETCURRENTVOLUMEID){
        current_volume = (int) arg2;
        return NULL;
    }

// 173
// Lista arquivos de um diretório, dado o número do disco,
// o numero do volume e o número do diretório,
// args in: disk id, volume id, directory id
// See: fs.c
    if (number == SYS_LISTFILES){
        fsListFiles( arg2, arg3, arg4 );
        return NULL;
    }

// 174
    if (number == SYS_SEARCHFILE){
        debug_print ("sci0: SYS_SEARCHFILE\n");
        return (void *) search_in_dir ( 
                            (const char *) arg2, 
                            (unsigned long)   arg3 );
    }


// 175 - 'cd' command support.
// +Atualiza o pathname na estrutura do processo atual.
// +Atualiza o pathname na string global.
// +Carrega o arquivo referente ao diretório atual.
// See: fs.c
    if (number == 175){
        debug_print("sci0: 175\n");
        sys_cd_command((char *) arg2);
        return NULL;
    }

// 176
// Remove n nomes de diretório do pathname do processo 
// indicado no argumento.
// Copia o nome para a string global.
    if (number == 176)
    {
        debug_print("sci0: 176\n");
        fs_pathname_backup( current_process, (int) arg3 );
        return NULL;
    }

// 177 -  'dir' command.
// Comando dir no shell.
// Listando arquivos em um diretório dado seu nome.
// #bugbug: Talvez tenhamos que usr a sci2.
// See: fs.c
    if (number == 177)
    {
        debug_print("sci0: [177]\n");
        fsList((const char *) arg2);
        return NULL;
    }

// 178
// See: sys.c
    if (number == 178){
        return (void *) sys_get_file_size((unsigned char *) arg2);
    }

// 179

//----------
// 180~189: memory support.
// (Privilegies)

// 184
// Pega o endereço do heap do processo dado seu id.
// See: process.c
    if (number == SYS_GETPROCESSHEAPPOINTER)
    {
        debug_print("sci0: [184]\n");
        return (void *) GetProcessHeapStart((int) arg2);
    }

//----------
// 190~199: free
// 200~209: free

//----------
// 210~219: terminal/virtual console support.

//211
    if (number == SYS_GETCURRENTTERMINAL){
        return (void *) current_terminal;
    }

//212
    if (number == SYS_SETCURRENTTERMINAL)
    {
        // #todo: Permissions.
        current_terminal = (int) arg2;
        return NULL;
    }

//----------

// 223 - Get sys time info.
// informaçoes variadas sobre o sys time.
    if (number == 223){
        return (void *) get_systime_info((int) arg2);
    }

// Business Logic:
// 224
    if (number == SYS_GETTIME){
        return (void *) get_time();
    }

// Business Logic:
// 225
    if (number == SYS_GETDATE){
        return (void *) get_date();
    }

// 226 - get
// Obs: 
// #todo: 
// Poderia ser uma chamada para configurar o posicionamento 
// e outra para configurar as dimensões.
// #todo: Atomic stuff.
    if (number == SYS_GET_KERNELSEMAPHORE){
        return (void *) __spinlock_ipc;
    }

// 227 - close gate
// Entering critical section.
// See: process.c
    if (number == SYS_CLOSE_KERNELSEMAPHORE){
        process_close_gate(current_process);
        return NULL;
    }

// 228 - open gate
// Exiting critical section.
// #todo: Quando um processo fechar e estiver
// em sua sessão crítica, então devemos liberar
// essa flag. Isso fica mais fácil de lembrar se
// existir uma flag na estrutura de processo.
// See: process.c
    if (number == SYS_OPEN_KERNELSEMAPHORE){
        process_open_gate(current_process);
        return NULL;
    }

// 229

//---------------------
// 230~239: Reserved for tty support.

// 236 - get tty id
    if (number == 236){
        return (void *) current_tty;
    }

//---------------------

// 240~249: Reserved for text editing support.

// 240, 241
    if (number == SYS_GETCURSORX){
        return (void *) get_cursor_x();
    }
    if (number == SYS_GETCURSORY){
        return (void *) get_cursor_y();
    }

// =====================================
// (250 ~ 255) - Info support.

// Business Logic: Get system metrics.
// 250
// IN: index
    if (number == SYS_GETSYSTEMMETRICS)
    {
        //if (arg2<0)
            //return NULL;
       return (void *) sys_get_system_metrics((int) arg2);
    }

// == - ==================================================


// ==== 260 mark ===================================================

// Outro numero fará esse trabalhao.
    if (number == 260)
    {
        return (void *) sys_read ( 
                            (unsigned int) arg2, 
                            (char *)       arg3, 
                            (int)          arg4 );
    }

// Outro numero fará esse trabalho.
    if (number == 261)
    {
        return (void *) sys_write ( 
                            (unsigned int) arg2, 
                            (char *)       arg3, 
                            (int)          arg4 );
    }


// read on virtual console!
// range: 0 ~ 3
// chamado por read_VC em ring3.
// IN: fd, buf, count
    if (number == 262){
        return (void *) console_read((int) arg2, 
                            (const void *) arg3, (size_t) arg4 );
    }

// write on virtual console!
// range: 0 ~ 3
// chamado por write_VC em ring3.
// IN: fd, buf, count
    if ( number == 263 ){
        return (void *) console_write ((int) arg2, 
                            (const void *) arg3, (size_t) arg4 );
    }



// 266
// Pega o número da tty de um processo, dado o pid.
// process.c
// IN: PID.
// OUT: tty id.
    if (number == 266){
        return (void *) process_get_tty((int) arg2);
    }

// Ligar duas tty, dados os pids dos processos que possuem as tty.
// tty/pty.c
// IN: master pid, slave pid.
    //if (number == 267){
    //    return (void *) pty_link_by_pid ( (int) arg2, (int) arg3 );
    //}


// Channel is a file descriptor in the file list 
// of the current process.
// IN: fd, buf, count.
    if (number == 272)
    {
           return (void *) tty_read ( 
                               (unsigned int) arg2,    // channel 
                               (char *)       arg3,    // buf
                               (int)          arg4 );  // nr
    }

// Channel is a file descriptor in the file list 
// of the current process.
// IN: fd, buf, count.
    if (number == 273)
    {
        return (void *) tty_write ( 
                            (unsigned int) arg2,    // channel 
                            (char *)       arg3,    // buf
                            (int)          arg4 );  // nr
    }



// Get current virtual console.
    if (number == 277){
        return (void *) console_get_current_virtual_console();
    }

// Set current cirtual console.
// #todo: precisa de privilégio. 
    if (number == 278){
        console_set_current_virtual_console((int) arg2);
        return NULL;
    }
    

// Returns the current runlevel.
    if (number == 288){
        return (void *) newos_get_current_runlevel();
    }

// Serial debug print string.
// See: sys.c
    if ( number == 289 ){
        return (void *) sys_serial_debug_printk( (char *) arg2 );
    }

    if ( number == 292 ){
        return (void *) newos_get_memory_size_mb();
    }

// #bugbug: cuidado.
// get boot info.
// See: info.c
// IN: index to select the info.
    if ( number == 293 ){
        return (void *) info_get_boot_info ( (int) arg2 );
    }

// Inicializar ou reinicializar componentes do sistema
// depois da inicialização completa do kernel.
// Isso poderá ser chamado pelo init.bin, pelo shell
// ou qualquer outro.
// see: 
    if (number == 350){
        printf("350:\n"); 
        return (void *) sys_initialize_component((int) arg2);
    }


// 377 
// Get info to fill the utsname structure.
// See: sys.c
    if (number == 377)
    {
        debug_print("__extra_services: [377]\n");
        if ( (void*) arg2 == NULL )
            return NULL;
        sys_uname ( (struct utsname *) arg2 );
        return NULL;
    }

// #bugbug
// It crashes the system.
// Clear the screen.
    if (number==390)
    {
        debug_print("__extra_services: [390] :)\n");
        //displayInitializeBackground(,COLOR_BLUE,TRUE);
        return NULL;
    }

// #bugbug
// Falha se tentamos pintar a tela toda.
    if (number==391)
    {
        //debug_print("__extra_services: [391]\n");
        backbuffer_draw_rectangle ( 
            (unsigned long) message_address[0],    //x 
            (unsigned long) message_address[1],    //y
            (unsigned long) message_address[2],    //width
            (unsigned long) message_address[3],    //height
            (unsigned int)  message_address[4],    //color
            (unsigned long) message_address[5] );  //rop_flags
        return NULL;
    }

// 512 - Get display server PID for a given zing hook.
// IN: zing hook structure pointer.
// OUT: pid
    if (number == SYS_GET_WS_PID)
    {
        debug_print("__extra_services: SYS_GET_WS_PID\n");
        __zh = (struct zing_hook_d *) arg2;
        if ( (void *) __zh != NULL )
        {
            if ( __zh->used  == TRUE && 
                 __zh->magic == 1234 )
            {
                return (void *) __zh->__display_server_pid; 
            }
        }
        // It means pid=0.
        return NULL;
    }

// 513
// Register the ring3 display server.
// Set display PID for a given zing hook structure.
// Register a display server.
// gramado_ports[11] = ws_pid
// Called by the ring 3 display server.
// arg2 = zing hook structure pointer.
// arg3 = The display erver PID.
    int display_server_ok=FALSE;
    if (number == SYS_SET_WS_PID)
    {
        debug_print("__extra_services: SYS_SET_WS_PID\n");
        
        // IN: zing hook, caller pid.
        // see: network.c
        display_server_ok = 
            (int) network_register_ring3_display_server(
                (struct zing_hook_d *) arg2, (pid_t) arg3);
        
        if (display_server_ok == TRUE){
            return (void*) TRUE;
        }
        return NULL;
    }   


// #deprecated
// 514 - get wm PID for a given zing hook
    if ( number == SYS_GET_WM_PID ){
        panic("__extra_services: SYS_GET_WM_PID\n");
        return NULL;
    }
// #deprecated
// 515 - set wm PID for a given zing hook
    if (number == SYS_SET_WM_PID){
        panic("__extra_services: SYS_SET_WM_PID\n");
        return NULL;
    }

// #bugbug
// This is a ring0 pointer.
// A ring3 process can't handle this thing.
// Get current zing hook
    if (number == 519){
        return (void *) CurrentZingHook;
    }

// 521 - set ns PID for a given zing hook.
// network server
// Register a network server.
    if (number == 521)
    {
        __zh = (struct zing_hook_d *) arg2;
        if ( (void *) __zh != NULL )
        {
            if ( __zh->used == TRUE && 
                 __zh->magic == 1234 )
            {
                __zh->__network_server_pid = (pid_t) arg3;

                socket_set_gramado_port(
                    GRAMADO_PORT_NS,
                    (pid_t) current_process );

                return (void *) TRUE;  //ok 
            }
        }
        return NULL; //fail
    }    



// 600 - dup
    if (number == 600){
        return (void *) sys_dup( (int) arg2 );  
    }
// 601 - dup2
    if (number == 601){
        return (void *) sys_dup2( (int) arg2, (int) arg3 );
    }
// 602 - dup3
    if (number == 602){
        return (void *) sys_dup3( (int) arg2, (int) arg3, (int) arg4 );
    }

// 603 - lseek support.
// See: kunistd.c
// IN: fd, offset, whence.
    if (number == 603){
        return (void *) sys_lseek ( 
                            (int)   arg2, 
                            (off_t) arg3, 
                            (int)   arg4 );
    }

// 640
// Lock the taskswtiching.
// Only the init thread can call this service.
    if (number == 640)
    {
        if (current_thread == INIT_TID){
            taskswitch_lock();
        }
        return NULL;
    }

// 641
// Unlock taskswitching.
// Only the init thread can call this service.
    if (number == 641)
    {
        if (current_thread == INIT_TID){
            taskswitch_unlock();
        }
        return NULL;
    }

// 642
// Lock the scheduler.
// Only the init thread can call this service.
    if (number == 642)
    {
        if (current_thread == INIT_TID){
            scheduler_lock();
        }
        return NULL;
    }

// 643
// Unlock scheduler.
// Only the init thread can call this service.
    if (number == 643)
    {
        if (current_thread == INIT_TID){
            scheduler_unlock();
        }
        return NULL;
    }



// 770 - Show device list.
    if (number == 770)
    {
        // #bugbug
        // Showing only one type of object,
        devmgr_show_device_list(ObjectTypeTTY);
        return NULL;
    }

// 777 - cpu usage for idle thread.
    if (number == 777){
        return (void *) profiler_percentage_idle_thread;
    }

// 801 - get host name
    if ( number == 801 ){
        return (void *) __gethostname((char *) arg2);
    }
// 802 - set host name
    if ( number == 802 ){
        return (void *) __sethostname((const char *) arg2); 
    }
// 803 - Get user name.
    if (number == 803){
        return (void *) __getusername((char *) arg2);
    }
// 804 - Set user name.
    if (number == 804){
        return (void *) __setusername((const char *) arg2); 
    }

// #todo
// supporting ptsname libc function
// get_ptsname
// #todo: Change the name to sys_ptsname()
// IN: fd do master, buffer em ring3 para o nome, buflen.
    if (number == 808){
        return (void *) __ptsname ((int) arg2, 
                            (char *) arg3, (size_t) arg4  ); 
    }

// (#bugbug: The same as above?)
//#todo
//supporting ptsname_r libc function
// #todo: Change the name to sys_ptsname()
//IN: fd do master, buffer e buflen.
    if ( number == 809 ){
        return (void *) __ptsname ( (int) arg2, 
                            (char *) arg3, (size_t) arg4  ); 
    } 

// 880 - Get process stats given pid
// IN: pid, index
    if ( number == 880 ){
       return (void *) get_process_stats ( (pid_t) arg2, (int) arg3 );
    }

// 881 - Get thread stats given tid
// IN: tid, number
    if ( number == 881 ){
        return (void *) GetThreadStats ( (int) arg2, (int) arg3 );
    }

// 882 - Get process name
// IN: PID, ubuffer.
    if ( number == 882 ){
        return (void *) getprocessname( (pid_t) arg2, (char *) arg3 );
    }

// 883 - Get thread name
    if ( number == 883 ){
        return (void *) getthreadname ( (int) arg2, (char *) arg3 );
    }

// 884 - alarm()
// See: sys.c
    if ( number == 884 ){
        return (unsigned long) sys_alarm( (unsigned long) arg2 );
    }

// 891 - Allocate shared ring3 pages.
    if ( number == 891 ){
        debug_print("__extra_services: 891, Allocate shared ring3 pages\n");
        return (void *) newos_alloc_shared_ring3_pages( (pid_t) current_process, (int) arg2 );
    }

// 892 - Setup the thread's surface rectangle.
    if (number == 892)
    {
        __setup_surface_rectangle( 
            (unsigned long) message_address[0],  //l 
            (unsigned long) message_address[1],  //t
            (unsigned long) message_address[2],  //w
            (unsigned long) message_address[3]); //h
        return NULL;
    }



// 893 - Invalidate the thread's surface rectangle.
    if (number == 893)
    {
        __invalidate_surface_rectangle();
        return NULL;
    }
// 896 - Invalidate the whole screen
    if ( number == 896 )
    {
        invalidate_screen();
        //screen_is_dirty = TRUE;
        return NULL;
    }
// 897 - Create a rectangle.
    if (number == 897)
    {
        __service897();
        return NULL;
    }
// 898 - Enable prompt
    if ( number == 898 )
    {
        printf ("Prompt ON: Type something\n");
        printf("\n");
        ShellFlag = TRUE;
        return NULL;
    }
// 899 - Desable prompt
    if ( number == 899 )
    {
        printf ("Prompt OFF: Bye\n");
        printf("\n");
        ShellFlag = FALSE;
        return NULL;
    }
// is the socket full?
// See: sys.c
// IN: fd
// OUT: -1= error; FALSE= nao pode ler; TRUE= pode ler.
    if ( number == 913 ){
        return (void *) sys_sleep_if_socket_is_empty(arg2);
    }


// get screen window.
// #todo. checar validade
    //if ( number == 955 ){  return (void *) gui->screen;  } 
    
    //if ( number == 956 ){  return (void *) gui->background; } 

// get main window.
// #todo. checar validade
    //if ( number == 957 ){ return (void *) gui->main; }  

// 970 - Create request.
// ?? #bugbug
// A interrupção não conseguirá retornar para a mesma thread.
// Chamará o scheduler por conta própria.
// IN: reason, reason
    if ( number == 970 )
    {
        // #suspended
        /*
        create_request ( 
            (unsigned long) 15,      // number 
            (int) 1,                 // status 
            (int) 0,                 // timeout. 0=imediatamente.
            (pid_t) current_process,   // target_pid
            (tid_t) current_thread,    // target_tid
            (int) 0,                 // msg  
            (unsigned long) arg2,    // long1  
            (unsigned long) arg3 );  // long2
        */
        return NULL;
    }


// api - load file (string ???)
// #todo: Tem que retornar algum identificador para a api.
// poderia ser um indice na tabela de arquivos abertos pelo processo.
// #todo: rever.
// See: kstdio.c
    //if ( number == 4002 ){
    //    return (void *) k_fopen ( (const char *) arg2, "r+" );
    //}

// 4444
// Show root files system info.
// Print into the raw kernel console.
    if (number == 4444){
        fs_show_root_fs_info();
        return NULL;
    }



    //7000 ~ 7020 for network sockets

// 7000 - socket() 
// See: socket.c
// family, type, protocol
    if (number == 7000){
        return (void *) sys_socket( (int) arg2, (int) arg3, (int) arg4 );
    }

// 7001 - connect()
// fd, sockaddr struct pointer, addr len.
    if (number == 7001){
        return (void *) sys_connect ( 
                            (int) arg2, 
                            (const struct sockaddr *) arg3,
                            (socklen_t) arg4 );
    }

// 7002 - accept()
// This is the unix standard method.
// Our major goal is to return the fd for the client socket file.
// #bugbug: Work in progress.
// fd, sockaddr struct pointer, addr len pointer.
    if (number == 7002){
        return (void *) sys_accept ( 
                            (int) arg2, 
                            (struct sockaddr *) arg3, 
                            (socklen_t *) arg4 ); 
    }

// 7003 - bind()
// fd, sockaddr struct pointer, addr len.
    if ( number == 7003 ){
        return (void *) sys_bind ( 
                            (int) arg2, 
                            (const struct sockaddr *) arg3,
                            (socklen_t) arg4 );
    }

// 7004 - listen() support.
// IN: fd, backlog
// see: 
    if (number == 7004){
        return (void *) sys_listen((int) arg2, (int) arg3);  
    }

// 7005

// 7006
// Salvar um pid em uma das portas.
// IN: gramado port, PID
    if (number == 7006){
        return (void *) socket_set_gramado_port( (int) arg2, (int) arg3 );
    }

// 7007 - sys_getsockname()
// fd, sockaddr struct pointer, addr len.
    if ( number == 7007 ){
        return (void *) sys_getsockname ( 
                            (int) arg2, 
                            (struct sockaddr *) arg3,
                            (socklen_t *) arg4 );
     }

// 7008 - show socket info for a process.
// IN: pid
    if ( number == 7008 ){
        show_socket_for_a_process((int) arg2);
        return NULL;
    }

// 7009 - libc: shutdown() IN: fd, how
    if ( number == 7009 ){
        sys_socket_shutdown( (int) arg2, (int) arg3 );
        return NULL;
    }


// 8000 - ioctl() implementation.
// See: fs.c
// IN: fd, request, arg
    if (number == 8000){
        return (void *) sys_ioctl ( 
                            (int) arg2, 
                            (unsigned long) arg3, 
                            (unsigned long) arg4 );
    }

// 8001 - fcntl()
// See: sys.c    
    if ( number == 8001 ){
        return (void *) sys_fcntl ( 
                            (int) arg2, 
                            (int) arg3, 
                            (unsigned long) arg4 );
    }

// ?? #bugbug
// Setup stdin pointer
// See: kstdio
// IN: fd
    if (number == 8002){
        return (void *) sys_setup_stdin((int) arg2);
    }

// Pegando o endereço de um buffer de icone.
// queremos saber se ele eh compartilhado.
// shared_buffer_terminal_icon
// #bugbug: Static size for the icons. Static buffer size.
// See: wm.c
    if (number == 9100){
        if (arg2<0)
            return NULL;
        return (void *) ui_get_system_icon((int) arg2);
    }

// ========================================================
//done:
    __default_syscall_counter++;
    return NULL;
}


// Handler for the interrupt 0x81.
void *sci1 ( 
    unsigned long number, 
    unsigned long arg2, 
    unsigned long arg3, 
    unsigned long arg4 )
{
// A15 - The vomit.
// Getting requests from ring3 applications via systemcalls.
// #test
// It's ok when gramland calls this systemcall.

    debug_print ("sci1: [TODO]\n");

    pid_t current_process = (pid_t) get_current_process();
    int layer = LAYER_UNDEFINED;

//cpl
    unsigned long *cpl_buffer = (unsigned long *) &sci1_cpl;
    int cpl=-1;
    unsigned long tmp_cpl = (unsigned long) cpl_buffer[0];
    cpl = (int) (tmp_cpl & 3);

    if( cpl != 0 && cpl != 1 && cpl != 2 && cpl != 3 )
    {
        panic("sci1: cpl");
    }

    if(cpl == 0){
        panic("sci1: cpl 0\n");
    }
    if(cpl == 1){
        panic("sci1: cpl 1\n");
    }
    if(cpl == 2){
        panic("sci1: cpl 2\n");
    }
    if(cpl == 3){
        // ok
    }

// permission
    if (current_process<0 || current_process >= PROCESS_COUNT_MAX){
        panic("sci1: current_process\n");
    }

/*
//++
//-------------------------------------
// #test
// #todo: This is a work in progress.
// Maybe this interrupt can be used 
// to call the services provided by the first module, mod0.bin.
// see: mod.c and mod.h.
    unsigned long return_value=0;

    if ((void*) kernel_mod0 == NULL)
        return NULL;
    if (kernel_mod0->magic != 1234)
        return NULL;
    if (kernel_mod0->initialized != TRUE)
        return NULL;

// #test
// Calling the virtual function, and
// getting the return value.
    if ( (void*) kernel_mod0->entry_point != NULL )
    {
        return_value = 
            (unsigned long) kernel_mod0->entry_point(
                number,
                arg2,
                arg3,
                arg4 );
        //printf ("RETURN: %d\n",return_value);
        return (void*) return_value;
    }
//-------------------------------------
//--
*/


    switch (number){

    case 1:
        return NULL;
        break;

    // 110 - Reboot
    case SYS_REBOOT:
        keReboot();
        break;

    // ...

    default:
        __default_syscall_counter++;
        return NULL;
        break;
    };

// #todo
// Maybe kill the caller.

    panic("sci1: [FIXME] default syscall\n");
    return NULL;
}

// Handler for the interrupt 0x82.
void *sci2 ( 
    unsigned long number, 
    unsigned long arg2, 
    unsigned long arg3, 
    unsigned long arg4 )
{
// D8 - The k.
// Getting requests from ring3 applications via systemcalls.
// #test
// It's ok when the network server calls this systemcall.

    struct process_d  *p;
    struct thread_d  *t;

    // Array de longs.
    // Not in use!
    //unsigned long *a2 = (unsigned long*) arg2;
    //unsigned long *a3 = (unsigned long*) arg3;
    //unsigned long *a4 = (unsigned long*) arg4;

    pid_t current_process = (pid_t) get_current_process();
    int layer = LAYER_UNDEFINED;

//cpl
    unsigned long *cpl_buffer = (unsigned long *) &sci2_cpl;
    int cpl=-1;
    unsigned long tmp_cpl = (unsigned long) cpl_buffer[0];
    cpl = (int) (tmp_cpl & 3);

    if ( cpl != 0 && cpl != 1 && cpl != 2 && cpl != 3 )
    {
        panic("sci2: cpl");
    }

    if (cpl == 0){
        panic("sci2: cpl 0\n");
    }
    if (cpl == 1){
        panic("sci2: cpl 1\n");
    }
    if (cpl == 2){
        panic("sci2: cpl 2\n");
    }
    if (cpl == 3){
        // ok
    }

    // debug_print("sci2: [TODO]\n");

// Profiling in the process structure.

// Permission

    if ( current_process < 0 || current_process >= PROCESS_COUNT_MAX ){
        panic("sci2: current_process\n");
    }

    p = (struct process_d *) processList[current_process];
    if ((void*) p == NULL){
        debug_print("sci2: p\n");
        panic("sci2: p\n");
    }
    if ( p->used != TRUE || p->magic != 1234 ){
        debug_print("sci2: p validation\n");
        panic("sci2: p validation\n");
    }

// Personality
// Feito pra rodar no Gramado OS.
    if (p->personality != PERSONALITY_GRAMADO_AMAZING)
    {
        debug_print("sci2: Personality\n");
        panic      ("sci2: Personality\n");
    }

// The layer.
    layer = p->_layer;

// Counting syscalls ...
    p->syscalls_counter++;

//
// switch
//

// 1 - Set magic (in kernel console)
// #todo: This operation needs permition?
    if (number == 1){
        //CONSOLE_TTYS[fg_console].magic = arg2;
        return NULL;
    }
// 2 - Get magic (in kernel console)
    if (number == 2){
        return (void*) CONSOLE_TTYS[fg_console].magic;
    }

// 3 - Get system metrics
    if (number == 3){
        return (void*) sys_get_system_metrics(arg2);
    }

// 4 - ioctl() handler.
// See: fs.c
// IN: fd, request, arg
    if (number == 4){
        debug_print("sci2: [4] ioctl\n");
        // #todo
        //return (void*) sys_ioctl ( (int) arg2, (unsigned long) arg3, (unsigned long) arg4 );
        return NULL;
    }

// 5 - Business Logic: fcntl() implementation.
// See: ?
    if (number == 5){
        debug_print("sci2: [5] fcntl\n");
        return (void*) sys_fcntl( (int) arg2, (int) arg3, (unsigned long) arg4 );
    }

// 18 - Business Logic: read() implementation.
// See: fs.c
    if (number == 18){
        //debug_print("sci2: [18] read\n");
        return (void *) sys_read( 
                            (unsigned int) arg2, 
                            (char *)       arg3, 
                            (int)          arg4 );
    }

// 19 - Business Logic: write() implementation.
// See: fs.c
    if (number == 19){
        //debug_print("sci2: [19] write\n");
        return (void *) sys_write ( 
                            (unsigned int) arg2, 
                            (char *)       arg3, 
                            (int)          arg4 );
    }

    // ...

// 265 - yield
// Business Logic: 
//  + Yield 
//  + Set a flag that this thread will be preempted.
// See: sched/schedi.c
    if (number == 265)
    {
        debug_print("sci2: [265] Yield\n");
        yield(current_thread); 
        return NULL; 
    }

// 266 - sleep
// Business Logic: 
// Sleep until.
// #todo: Explaint it better here.
// #bugbug
// We cant drastically change the state of a thread,
// we need to schedule that operation.
// given to the ts the opportunity to do that 
// in the right moment. As well as we do for the yield operation.
// Agendando a operação de sleep.
// O ts vai fazer isso quando for seguro.
// IN: tid, ms
    if (number == 266)
    {
        // #bugbug: ?
        printf("sci2: [266] Sleep until\n");
        sleep( (tid_t) current_thread, (unsigned long) arg2 );
        return NULL;
    }

// 777 - kinda nice() 
    if (number == 777){
        sys_broken_vessels(current_thread);
        return NULL;   
    }

// ---------------------------
// 900 - copy process
// Business Logic: 
// Clona e executa o filho dado o nome do filho.
// O filho inicia sua execução do início da imagem.
// #bugbug: Isso às vezes falha na máquina real.
// #todo: Use more arguments.
// See: clone.c
// IN: 
//   + file name, 
//   + parent pid, 
//   + clone flags.
// OUT: Child's PID.

    unsigned long clone_flags = (unsigned long) arg3;
    //unsigned long extra = (unsigned long) arg4;
    if (number == 900)
    {
        debug_print("sci2: [900] clone and execute\n");
        // #debug
        //printf("sci2: copy_process called by pid{%d}\n",current_process);
        return (void *) copy_process( 
                            (const char *) arg2, 
                            (pid_t) current_process, 
                            (unsigned long) clone_flags );
    }

// This process is telling us that he wants
// to be treated as a terminal,
// giving him the permission to create Connectors[i].
    if (number == 901)
    {
        if (p->_is_terminal == TRUE)
            return NULL;
        if (arg2 != 1234)
            return NULL;
        p->Connectors[0] = -1;
        p->Connectors[1] = -1;
        p->_is_terminal = TRUE;
        return NULL;
    }

// Get the file descriptor for a given connector
// #todo: Create a worker for this.
// return: 
// NULL = fail.
    int conn_fd = 0;
    if (number == 902)
    {
        if (p->_is_terminal == TRUE || 
            p->_is_child_of_terminal == TRUE )
        {
            // The first connector
            if (arg2 == 0)
            {
                conn_fd = (int) p->Connectors[0];
                if (conn_fd < 0)
                    return NULL;
                if (conn_fd < 3)
                    return NULL;
                // 31 is the socket.
                if (conn_fd >= 30)
                    return NULL;
                return (void*) (conn_fd & 0xFF);
            }
            // The second connector
            if (arg2 == 1)
            {
                conn_fd = (int) p->Connectors[1];
                if (conn_fd < 0)
                    return NULL;
                if (conn_fd < 3)
                    return NULL;
                // 31 is the socket.
                if (conn_fd >= 30)
                    return NULL;
                return (void*) (conn_fd & 0xFF);
            }
            // Fail
            // Invalid connector number.
            return NULL;
        }
        // Fail
        return (void*) NULL;
    }

// 8000 - Business Logic: ioctl() implementation.
// See: fs.c
// IN: fd, request, arg
    if (number == 8000)
    {
        debug_print("sci2: [8000] ioctl\n");
        //printf("sci2: [8000] ioctl\n");
        return (void *) sys_ioctl ( 
                            (int) arg2, 
                            (unsigned long) arg3, 
                            (unsigned long) arg4 );
    }


// 8001 - Business Logic: fcntl() implementation. 
// (second time) see: number 5.
// See: sys.c
    if (number == 8001){
        debug_print("sci2: [8001] fcntl\n");
        return (void *) sys_fcntl (
                            (int) arg2, 
                            (int) arg3, 
                            (unsigned long) arg4 );
    }

// 8003
// Business Logic: 
// Clear the fg console background with a given color.
// Do not change the colors.
    unsigned int bg_color = COLOR_BLACK;
    unsigned int fg_color = COLOR_WHITE;
    if (number == 8003)
    {
        if (fg_console<0 || fg_console > 3){
            return NULL;
        }
        bg_color = (unsigned int) CONSOLE_TTYS[fg_console].bg_color;
        fg_color = (unsigned int) CONSOLE_TTYS[fg_console].fg_color;
        // IN: bg color, fg color, console number.
        clear_console( bg_color, fg_color, fg_console );
        return NULL;
    }

// 8004
// Business Logic: 
// Change the foreground color of the current console.
    if (number == 8004)
    {
        if (fg_console<0 || fg_console > 3){
            return NULL;
        }

        // #bugbug
        // #deprecated
        // Cant change the kernel consoles.
        // Only the other ttys.
        // CONSOLE_TTYS[fg_console].fg_color = (unsigned int) arg2;

        return NULL;
    }

// 10000 - sys_set_file_sync
// Business Logic: 
// Configurando sincronização de leitura e escrita em arquivo.
// principalmente socket.
// A estrutura de arquivo contém uma estrutura de sincronização de leitura e escrita.
// #ok: podemos usar ioctl
// See: sys.c
    if (number == 10000)
    {
        debug_print("sci2: [10000] sys_set_file_sync\n");
        // IN: fd, request, data
        sys_set_file_sync( (int) arg2, (int) arg3, (int) arg4 );
        return NULL;
    }

// 10001 - sys_get_file_sync
// Business Logic: 
// Pegando informação sobre sincronização de leitura e escrita de arquivos.
// principalmente para socket.
// A estrutura de arquivo contém uma estrutura de sincronização de leitura e escrita.
// #ok: podemos usar ioctl
// See: sys.c
// IN: fd, request
    if (number == 10001){
        //debug_print("sci2: [10000] sys_get_file_sync\n");
        return (void*) sys_get_file_sync( (int) arg2, (int) arg3 );
    }

//
// Global sync
//

// Global sync - Not used anymore.

// ============
// See: sys.c
// Business Logic: Set action.
    if (number == 10002){
        sys_set_global_sync( (int) arg2, (int) arg3, (int) arg4 );
        return NULL;
    }
    // Business Logic: Get action.
    if (number == 10003){
        return (void*) sys_get_global_sync( (int) arg2, (int) arg3 );
    }
    // Business Logic: Create sync.
    // OUT: sync id.
    if (number == 10004){
        return (void*) sys_create_new_sync();
    }
    // Business Logic: Get sync id.
    // Provisorio para teste
    if(number == 10005){
        return (void*) get_saved_sync();
    }
//=====

//
// Sync in file.
//

// ===============
// Business Logic: 
// Set file sync action
// IN: fd, request, data
// see: fs.c
    if (number == 10006){
        sys_set_file_sync( (int) arg2, (int) arg3, (int) arg4 );
        return NULL;
    }
// Get file sync action
// IN: fd, request
// see: fs.c
    if (number == 10007){
        return (void*) sys_get_file_sync( (int) arg2, (int) arg3 );
    }
// ===============

// 10008
// Business Logic:
// Save FAT cache into the disk.
// FAT cache.
// This is the FAT cache for the system disk.
// The boot partition.
    int savefat_Status = -1;
    if (number == 10008)
    {
        savefat_Status = (int) fs_save_fat16_cache();
        if ( savefat_Status < 0 || 
             fat_cache_saved != FAT_CACHE_SAVED )
        {
            panic("sci2: [10008] Couldn't save FAT\n");
        }
        return NULL;
    }

// 10010 - Business Logic: Get the tid of the current thread.
    if (number == 10010){
        //debug_print("sci2: [10010] GetCurrentTID\n");
        return (void*) GetCurrentTID();
    }

// -----------------------------
// 10011
// Business Logic: 
// Set the foreground thread given it's tid.
// #todo: We need a method for that.
// IN: arg2=tid.
    if (number == 10011)
    {
        debug_print("sci2: [10011] set foreground thread tid\n");
        //Change the priority of the old foreground thread?
        //set_thread_priority( threadList[foreground_thread], PRIORITY_NORMAL);
        if (arg2<0 || arg2>=THREAD_COUNT_MAX){ return NULL; }  //fail
        t = (struct thread_d *) threadList[arg2];
        if ((void*) t == NULL){ return NULL; }; //fail
        if (t->used != TRUE)  { return NULL; }; //fail
        if (t->magic != 1234) { return NULL; }; //fail
        //Giving more credits. But the scheduler will balance
        //it at the and of the round.
        //t->quantum = QUANTUM_FIRST_PLANE;
        t->quantum = (QUANTUM_MAX + 88);
        t->priority = PRIORITY_MAX;
        
        //#test
        //t->signal |= 1<<(SIGALRM-1);
        //t->signal |= 1<<(SIGKILL-1);
        
        foreground_thread = (int) arg2;
        
        // #deprecated
        // it will select the next input reponder.
        // set_input_responder_tid(foreground_thread);
        return NULL;
    }

    // Business Logic: Get Init PID
    if (number == 10020){ 
        return (void*) GRAMADO_PID_INIT;
    }
    // Business Logic: Get Init TID
    if (number == 10021){
        return (void*) INIT_TID;
    }

//
// Network
//

// #todo:
// We can put all the network services in a single dialog function.
// Just like a procedure. networkProcesure(....);

// 22001 - Set network status.
// Enable or disable.
    if (number == 22001){
        networkSetStatus((int) arg2);
        return NULL;
    }
// 22002 - Get network status.
    if (number == 22002){
        return (void*) networkGetStatus();
    }

// 22003 - Test some net component.
    if (number == 22003)
    {
        switch (arg2){
            case 1:
                network_send_arp_request();
                break;
            case 2:
                network_test_udp();
                break;
            case 3:
                network_initialize_dhcp();
                break;
            case 4:
                break;
        };
        return NULL;
    }

// 22011
// Business Logic:
// PS2 full initialization.
    if (number == 22011)
    {
        if (HVInfo.initialized == TRUE)
        {
            // qemu.
            if (HVInfo.type == HV_TYPE_QEMU)
            {
                printf("sci2: [22011] PS2 full initialization on qemu\n");
                PS2_initialization();
                return NULL;
            }
            //if (HVInfo.type == ?){...}
        }
        return NULL;
    }

// #deprecated.
// shared memory 2mb surface.
// ring 3.
    if (number == 22777)
    {

        // #bugbug
        // Is it true?
        // I guess we are using all the extraheaps for slab allocations.
        // This syscall was called to provide shared memory for surfaces 
        // for ring3.

        //if (g_extraheap3_initialized==TRUE)
        //    return (void*) g_extraheap3_va;

        panic ("[22777] Deprecated syscall\n");
        return NULL;
    }


// 44000
// Business Logic:
// #important: We're avoiding the callback support.
// Callback support.
// see: ts.c
// see: pit.c
// arg2 = address
// arg3 = pid
// arg4 = signature
    pid_t ws_pid = -1;
    unsigned long r3_handler=0;
    if (number == 44000)
    {
        // Somente o window server pode chamar esse serviço.
        ws_pid = (pid_t) socket_get_gramado_port(GRAMADO_PORT_WS);
        // Checando se caller foi o window server.
        if (current_process != ws_pid){
            panic("sci2: [44000] current_process!=ws_pid\n");
        }
        // PID
        if (arg3 != ws_pid){
            panic("sci2: [44000] Invalid PID\n");
        }
        // nao foi inicializado pela inicialização do kenrel.
        if ( ws_callback_info.initialized != TRUE ){
            panic("sci2: [44000] callback support Not initialized\n");
        }
        // Se ele ja esta pronto para efetuarmos o iretq
        // é porque tem alguma coisa errada.
        if ( ws_callback_info.ready == TRUE ){
            panic("sci2: [44000] called again\n");
        }
        // Enable for the first time.
        // Configuramos o callback em ts.c.
        r3_handler = (unsigned long) arg2;
 
        //arg4: desired ms
        if (arg4<1 || arg4>1000){
            printf("sci2: [44000] Invalid ms\n");
            panic ("sci2: [44000] Invalid ms\n");
        }
         //setup_callback( (unsigned long) r3_handler, 16 );
        setup_callback( (unsigned long) r3_handler, arg4 );
        return NULL;
    }

// Counter
    __default_syscall_counter++;

// #todo
// Maybe kill the caller. 
// Maybe return.

    panic("sci2: [FIXME] default syscall\n");
    return NULL;
}

//
// End
//


