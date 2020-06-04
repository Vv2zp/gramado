/*
 * File: io.h
 *
 * Descri��o:
 *     Arquivo principal do I/O Manager.
 *     Todo gerenciamento de dispositivos,
 *     toda concess�o de acesso passar� por 
 *     esse gerenciador, que faz parte da
 *     camada de abstra��o de hardware oferecidad pelo
 *     kernel base.
 *
 * History:
 *     2016 - Created by Fred Nora.
 */


#ifndef ____IO_H
#define ____IO_H


// Estrutura para gerenciamento de buffer de i/o. 

struct ioBuffer_d
{
    object_type_t objectType;
    object_class_t objectClass;

    struct process_d *OwnerProcess;    // processo dono do buffer.
    unsigned long address;             // Address.
    
    int size;                          // size in number of bytes.
}; 

// #test
struct ioBuffer_d *CurrentIOBuffer;

//struct ioBuffer_d *lineBuffer;    //Buffer para uma linha de caracteres.
//struct ioBuffer_d *fileBuffer;    //buffer para um arquivo de texto.
//struct ioBuffer_d *CurrentBuffer;
//struct ioBuffer_d *ipcBuffer;    //IPC buffer. (�rea de transfer�ncia.)
// ... 

 
//
// Prototypes.
// 
 

// Handler for services.
void *ioServices ( 
    unsigned long number, 
    unsigned long arg2, 
    unsigned long arg3, 
    unsigned long arg4 );


int ioInit (void);


#endif   


//
// End.
//

