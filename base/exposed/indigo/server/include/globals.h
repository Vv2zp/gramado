
// globals.h

#ifndef __GLOBALS_H
#define __GLOBALS_H    1

// Buffer for the messages.
// See: globals.c
#define GNS_BUFFER_SIZE  512
extern char __buffer[GNS_BUFFER_SIZE];

// Buffer
// This is the buffer ussed for the next response.
// Marsheling ...
// Esses valores serão enviados como
// resposta ao serviço atual.
// Eles são configurados pelo dialogo na hora da
// prestação do serviço.
// No início desse array fica o header.
// see: globals.c
extern unsigned long next_response[32];


#endif    

