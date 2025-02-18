/*
 * File: main.c
 *    Browser application. UI.
 * 2020 - Created by Fred Nora.
 */

// rtl 
#include <types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>
//#include <arpa/inet.h>
#include <sys/socket.h>
#include <packet.h>
// The client-side library.
#include <gws.h>

// network ports.
#define PORTS_WS  4040
#define PORTS_NS  4041
#define PORTS_FS  4042
// ...

#define IP(a, b, c, d) \
    (a << 24 | b << 16 | c << 8 | d)


//char *hello = "Hello there!\n";
/*
#define IP(a, b, c, d) (a << 24 | b << 16 | c << 8 | d)
struct sockaddr_in addr = {
    .sin_family = AF_INET,
    .sin_port   = 7548, 
    .sin_addr   = IP(192, 168, 1, 79),
};
*/

static int __main_window = -1;
static int __addressbar_window = -1;
static int __button_window = -1;
static int __client_window = -1;


struct child_window_d
{
    unsigned long l;
    unsigned long t;
    unsigned long w;
    unsigned long h;
};
struct child_window_d cwAddressBar;
struct child_window_d cwButton;
struct child_window_d cwClientWindow;
// ...

//
// == Private functions: prototypes ================
//

static int 
browserProcedure(
    int fd, 
    int event_window, 
    int event_type, 
    unsigned long long1, 
    unsigned long long2 );

static int do_event_loop(int fd);
static void update_clients(int fd);

// ====================================

static void update_clients(int fd)
{
    // Local
    struct gws_window_info_d lWi;

    if (fd<0){
        return;
    }
// Get info about the main window.
//IN: fd, wid, window info structure.
    gws_get_window_info(
        fd, 
        __main_window,   // The app window.
        (struct gws_window_info_d *) &lWi );

// ---------------------------------------------
// address bar
    cwAddressBar.l = (( lWi.cr_width/8 )*2);
    cwAddressBar.t = 4;
    cwAddressBar.w = (( lWi.cr_width/8 )*3);
    cwAddressBar.h = 24;
    gws_change_window_position( 
        fd,
        __addressbar_window,
        cwAddressBar.l,
        cwAddressBar.t );
    gws_resize_window(
        fd,
        __addressbar_window,
        cwAddressBar.w,
        cwAddressBar.h );

//---------------------------------------------
// Button
    cwButton.l = (( lWi.cr_width/8 )*7);
    cwButton.t = 4;
    cwButton.w = (( lWi.cr_width/8 )*1);
    cwButton.h = 24;
    gws_change_window_position( 
        fd,
        __button_window,
        cwButton.l,
        cwButton.t );
    gws_resize_window(
        fd,
        __button_window,
        cwButton.w,
        cwButton.h );

//-----------------------
// the client window
// Save
// Save
    cwClientWindow.l = 0;
    cwClientWindow.t = (cwAddressBar.t + cwAddressBar.h + 2);
    cwClientWindow.w = lWi.cr_width;
    cwClientWindow.h = (lWi.cr_height - cwClientWindow.t);

    gws_change_window_position( 
        fd,
        __client_window,
        cwClientWindow.l,
        cwClientWindow.t );
    gws_change_window_position( 
        fd,
        __client_window,
        cwClientWindow.l,
        cwClientWindow.t );
    gws_resize_window(
        fd,
        __client_window,
        cwClientWindow.w,
        cwClientWindow.h );

// ...

    gws_set_focus(fd,__addressbar_window);

// At the end ...
    gws_redraw_window(fd, __addressbar_window, TRUE);
    gws_redraw_window(fd, __button_window, TRUE);
    gws_redraw_window(fd, __client_window, TRUE);
}


static int 
browserProcedure(
    int fd, 
    int event_window, 
    int event_type, 
    unsigned long long1, 
    unsigned long long2 )
{

    if(fd<0){
        return -1;
    }
    if(event_window<0){
        return -1;
    }
    if(event_type<0){
        return -1;
    }

    switch (event_type){

    // Evento de teste.
    case 1000:
        // If the event window is the main window, so
        // redraw client window
        if( event_window == __main_window ){
            gws_redraw_window(fd,__client_window,TRUE);
            return 0;
        }
        break;

    //36
    case MSG_MOUSERELEASED:
        
         // #test
         // We are in the browser.
        if ( event_window == __client_window )
        {
            // Refresh?
            gws_draw_char (
                (int) fd,              // fd
                (int) event_window,    // wid
                (unsigned long) long1, // left
                (unsigned long) long2, // top
                (unsigned long) COLOR_BLACK,
                (unsigned long) '.' );
            return 0;
        }
        
        return 0;
        break;

    case MSG_PAINT:
        if (event_window == __main_window){
            update_clients(fd);
            return 0;
        }
        break;

    case MSG_CLOSE:
        printf ("browser.bin: MSG_CLOSE\n");
        gws_destroy_window(fd,__button_window);
        gws_destroy_window(fd,__main_window);
        exit(0);
        break;

    //...
    
    default:
        return -1;
        break;
    };

    return -1;
}


static int do_event_loop(int fd)
{
    if(fd<0)
        return -1;

// #test
// pegando um evento com o ws.
// See: libgws/

    struct gws_event_d lEvent;
    lEvent.used = FALSE;
    lEvent.magic = 0;
    lEvent.type = 0;
    lEvent.long1 = 0;
    lEvent.long2 = 0;

    struct gws_event_d *e;

// loop
// Call the local window procedure 
// if a valid event was found.

    while (1)
    {
        //if (isTimeToQuit == TRUE)
            //break;

        e = (struct gws_event_d *) gws_get_next_event(
                fd, 
                __main_window,
                (struct gws_event_d *) &lEvent );

        if ((void *) e != NULL)
        {
            //if( e->used == TRUE && e->magic == 1234 )
            if (e->magic == 1234){
                browserProcedure( 
                    fd, e->window, e->type, e->long1, e->long2 );
            }
        }
    };

// Exit application withou error.
    return 0;
}

int main( int argc, char *argv[] )
{
    int client_fd = -1;

    struct sockaddr_in addr_in;
    addr_in.sin_family = AF_INET;
    addr_in.sin_port = PORTS_WS;   
    addr_in.sin_addr.s_addr = IP(127,0,0,1);

    debug_print ("browser: Initializing ...\n");

// Device info.
    unsigned long w = gws_get_system_metrics(1);
    unsigned long h = gws_get_system_metrics(2);
    if ( w == 0 || h == 0 ){
        printf ("browser: w h \n");
        exit(1);
    }

// socket
    client_fd = socket( AF_INET, SOCK_STREAM, 0 );
    if (client_fd<0){
       printf ("browser: Couldn't create socket\n");
       exit(1);
    }

// connect
// Nessa hora colocamos no accept um fd.
// Então o servidor escreverá em nosso arquivo.

    while (TRUE){
        if (connect(client_fd, (void *) &addr_in, sizeof(addr_in)) < 0){ 
            debug_print("browser: Connection Failed \n"); 
            printf     ("browser: Connection Failed \n"); 
        }else{ break; }; 
    };

// ==============================================

// #todo: 
// Salvar em global.
// Por enquanto aqui.

    int main_window=0;
    int addressbar_window=0;    
    int client_window=0;
    int button=0;

// A janela é a metade da tela.
    unsigned long w_width  = (w/2);
    // #hack
    if (w == 800)
        w_width = 640;
    // #hack
    if (w == 640)
        w_width = 480;
    unsigned long w_height = (h/2);

    //unsigned long viewwindowx = ( ( w - w_width ) >> 1 );
    //unsigned long viewwindowy = ( ( h - w_height) >> 1 );
    unsigned long viewwindowx = 10;
    unsigned long viewwindowy = 10;
    

// #test
// #bugbug
// Se tentarmos pintar fora dos limites da tela,
// então a janela sofrerá clipping e não teremos problemas.
// O proble não é a pintura, é sim na hora do refresh.
// Pois o refresh vai escrever em área não mapeada de memória.

    if (w == 320)
    {
        // dimensoes
        w_width  = w;
        w_height = h;
        // posicionamento
        viewwindowx = 0;
        viewwindowy = 0;
    }

// ===================
// main window
// style: 
// 0x0001=maximized | 0x0002=minimized | 0x0004=fullscreen | 0x0008 statusbar

    main_window = 
        (int) gws_create_window ( 
                  client_fd,
                  WT_OVERLAPPED, 
                  WINDOW_STATUS_ACTIVE,  // status
                  VIEW_NULL,             // view
                  "BROWSER",
                  viewwindowx, viewwindowy, w_width, w_height,
                  0,   // No parent.
                  0x0000,  
                  COLOR_GRAY, COLOR_GRAY );

    if (main_window < 0){
        debug_print("browser: main_window fail\n"); 
        exit(1);
    }
// Save globally
    if (main_window > 0){
        __main_window = main_window;
    }

// ===================
// address bar
// #todo: set focus.
// Se a janela mae for overlapped,
// entao seremos relativos à sua áre de cliente.

    //unsigned long ab_width = (w_width -4 -4 -24 -4);
    unsigned long ab_width = (w_width -50);

    addressbar_window = 
        (int) gws_create_window (
                  client_fd,
                  WT_EDITBOX, 1, 1, "AddressBar",
                  4, 
                  4, 
                  (w_width -4 -4 -24 -4), 
                  24,//32,
                  main_window,  // janela mãe é overlapped. pinta na client area.
                  WS_CHILD, 
                  COLOR_WHITE, 
                  COLOR_WHITE );

    if (addressbar_window < 0){
        debug_print("browser: addressbar_window fail\n"); 
    }

// IN: 
// fd, window id, left, top, color, string
    if( addressbar_window > 0 ){
        __addressbar_window = addressbar_window;
        gws_draw_text (
            (int) client_fd,
            (int) addressbar_window,
             8, 8, (unsigned long) COLOR_BLACK,
            "https://github.com/frednora");
    }

// ===================
// button WT_BUTTON

    button = 
        (int) gws_create_window (
                  client_fd,
                  WT_BUTTON, 
                  BS_DEFAULT, 
                  1, 
                  ">",
                  (w_width -24 -4),  // l 
                  4,                 // t
                  24, 
                  24,
                  main_window,// janela mãe é overlapped. pinta na client area. 
                  0, COLOR_GRAY, COLOR_GRAY );


/*
// #testing WT_ICON
    button = 
        (int) gws_create_window (
                  client_fd,
                  WT_ICON, 1, 1, ">",
                  (w_width -24 -4),  // l 
                  4,                 // t
                  24, 
                  24,
                  main_window,// janela mãe é overlapped. pinta na client area. 
                  0, COLOR_RED, COLOR_RED );
*/

    if (button < 0)
        debug_print("browser: button fail\n"); 

    if(button>0)
        __button_window=button;

// ===================
// client window (White)

    unsigned long cw_left = 4;
    unsigned long cw_top = 4 +24 +4;
    unsigned long cw_width  = (w_width>>1);
    unsigned long cw_height = (w_height>>1);

    client_window = 
        (int) gws_create_window (
                  client_fd,
                  WT_SIMPLE, 1, 1, "Client",
                  cw_left, cw_top, cw_width, cw_height,
                  main_window,// janela mãe é overlapped. pinta na client area. 
                  0, COLOR_WHITE, COLOR_WHITE );

    if (client_window < 0)
        debug_print("browser: client_window fail\n"); 

    if (client_window > 0){
        // Save globally.
        __client_window = client_window;
        gws_draw_text (
            (int) client_fd,      // fd
            (int) client_window,  // window id
            (unsigned long) 8,    // left
            (unsigned long) 8,    // top
            (unsigned long) COLOR_BLACK,
            "GRAMADO OS");
    }

// Refresh
    gws_refresh_window( client_fd, main_window );

// ============================================
// focus
// Set focus on addressbar window.
//    gws_async_command(
//        client_fd, 9, addressbar_window, addressbar_window );
// Set focus on client window.
    //gws_async_command(
    //    client_fd, 9, client_window, client_window );
// =======================================================

// set active window.
    //gws_async_command(
    //     client_fd,
    //     15, 
    //     main_window,
    //     main_window );

    gws_set_focus( client_fd, client_window );
    gws_set_active( client_fd, main_window );

// Call the event loop.
    return (int) do_event_loop(client_fd);
}

//
// End
//

