
// fs.c
// File system support.

#include <kernel.h>


// Canonical:
// The list of the main directories in the system.
// These are the first directories searched when
// we're looking for a file.
struct system_directory_d  sdROOT;      // '/'
//struct system_directory_d  sdEFI;       // 'EFI
//struct system_directory_d  sdGRAMADO;   // 'GRAMADO'
struct system_directory_d  sdPROGRAMS;  // 'PROGRAMS'
//struct system_directory_d  sdUSERS;     // 'USERS'


// Buffers for 8 directories. Each one with 512 entries.
// 32*512 = 16384
// 16KB
// 16384/4096 = 4 pages.
// ------------------
// with 8 pages we have 8 directories for 1024 entries each dir.

//unsigned long fs_buffers[8];
unsigned long fs_buffers[FS_N_BUFFERS];

//see: fs.h
struct filesystem_d  *root;

// gcc -Wall Defined but not used!
static char *____root_name = "/";

struct cwd_d  CWD;

//char search_path[?];
unsigned long search_path_dir_address=0;
unsigned long search_path_dir_entries=0;

// ========================================

//
// == Private functions: Prototypes ===========
//

//=============================

// File read.
// It's called by sys_read.
// Copia à partir do início do arquivo.
// // OUT: number of read bytes.
int file_read_buffer( file *f, char *buffer, int len )
{
    char *p;
    int Count=0;

    p = buffer;
// #test
    Count = (int) (len & 0xFFFF);

// Check file
    if ((void *) f == NULL){
        printf ("file_read_buffer: f\n");
        goto fail;
    }
    if ( f->used != TRUE || f->magic != 1234 ){
        printf ("file_read_buffer: f validation\n");
        goto fail;
    }
// Check buffer
    if ((void *) p == NULL){
        printf ("file_read_buffer: p\n");
        goto fail;
    }
// nada para ler.
    if (Count <= 0){
         printf ("file_read_buffer: Count <= 0\n");
         goto fail;
    }
// Chech len
// #bugbug: Isso é provisório
// A quantidade que desejamos ler é menor que o tamanho do buffer.
// Estamos lendo do início do arquivo?
    if (Count > f->_lbfsize){
        printf ("file_read_buffer: Count > f->_lbfsize\n");
        goto fail;
    }

    //if ( Count > f->_fsize ){
        //printf ("file_read_buffer: Count > f->_fsize\n");
    //    Count = f->_fsize;
    //    goto fail;
    //}

/*
// stdin
    if( f->_file == 0 )
    {
        if( f->_lbfsize != PROMPT_SIZE)
        {
           printf ("file_read_buffer: [FAIL] Wrong size for stdin _lbfsize\n");
           goto fail;
        }
    }
 */

//=======================================
// Copy!
// #todo
// nao podemos ler mais que o limite do arquivo.
// A próxima leitura precisa ser depois dessa.

// =================================
// Socket:
// Se o arquivo é um socket, 
// então não concatenaremos escrita ou leitura.
// You also can write now.
// But i can still read.
    if (f->____object == ObjectTypeSocket){
        memcpy ( (void *) buffer, (const void *) f->_base, Count );
        f->_flags |= __SWR;
        return (int) Count;
    }

// =================================
// Pipe:
// Não concatenaremos
// You also can write now.
// But i can still read.
    if (f->____object == ObjectTypePipe){
        memcpy ( (void *) buffer, (const void *) f->_base, Count );
        f->_flags |= __SWR;
        return (int) Count;
    }

// =================================
// Regular file, tty, iobuffer.
// Nesse caso a leitura tem que respeitar os offsets e limites.
    if ( f->____object == ObjectTypeFile ||
         f->____object == ObjectTypeTTY  ||
         f->____object == ObjectTypeIoBuffer)
    {
        // Se o buffer tem tamanho 0.
        if (f->_lbfsize <= 0){
            printf ("file_read_buffer: _lbfsize is 0\n");
            goto fail;
        }

        // Se o tamanho do buffer for maior que o padrão.
        // #todo: O buffer pdoerá ser maior que isso no futuro.
        //if ( f->_lbfsize > BUFSIZ ){
        //    printf ("file_read_buffer: _lbfsize\n");
        //    goto fail;
        //}

        // #test: Limite provisorio
        if (f->_lbfsize > (8*1024)){
            printf ("file_read_buffer: _lbfsize bigger than 8KB\n");
            goto fail;
        }

        // Não podemos ler antes do início do arquivo.
        if ( f->_r < 0 ){
            f->_r = 0;
            printf ("file_read_buffer: f->_r = 0\n");
            goto fail;
        }

        // Nao leremos depois do fim do arquivo.
        if ( f->_r > f->_lbfsize )
        {
            //#debug: provisorio
            printf ("file_read_buffer: f->_r > f->_lbfsize\n");
            goto fail;
            //debug_print("file_read_buffer: f->_r > f->_lbfsize\n");
            //f->_r = f->_lbfsize;
            //f->_w = f->_lbfsize;
            //f->_p = (f->_base + f->_lbfsize);
            //f->_cnt = 0;
            //return EOF;
        }

        // Se o offset de leitura for maior que
        // o offset de escrita, então temos que esperar.
        // #bugbug: mas talvez isso não seja assim para pipe.
        if ( f->_r > f->_w )
        {
            // EOF
            printf ("file_read_buffer: f->_r > f->_w\n");
            //goto fail;
            //debug_print("file_read_buffer: f->_r > f->_w\n");
            f->_r = f->_w;
            //faremos o ajuste logo abaixo.
            //f->_p = (f->_base + f->_r);
            // You also can write now.
            // But i can still read.
            //f->_flags = __SWR;
            return 0;
        }

        // Se a quantidade que desejamos ler
        // é maior que o espaço que temos.
        // # Isso ja foi feito logo acima.
        if (Count > f->_lbfsize)
        {

            //printf ("file_read_buffer: Count > f->_lbfsize\n");
            //goto fail;

            //printf ("file_read_buffer: [FAIL] local_len limits\n");
            //goto fail;
        
            //#test #bugbug
            // leia tudo então. hahaha
            Count = (f->_lbfsize - 1);
        }

        // Se o tanto que queremos ler é maior
        // que o que nos resta da buffer,
        // então vamos ler apenas o resto do buffer.

        // #bugbug: Isso esta errado. #delete

        // So podemos ler ate limite de bytes disponíveis 
        // no buffer.
        //if (Count > f->_cnt)
        //{
            //printf ("file_read_buffer: local_len > f->_cnt\n");
            //goto fail;
            //Count = f->_cnt;
        //}
 
        // 
        int delta = (f->_w - f->_r);

        // nada para ler.
        // pois o ponteiro de escrita e o de leitura sao iguais,
        if (delta == 0){
            // 0 bytes lidos
            //printf ("delta=0\n");
            return 0;
        }
 
        // #delta
        // Se o tanto que queremos ler
        // é maior que o tanto que foi efetivamente escrito,
        // então leremos somente o que foi escrito.
        // se a diferença entra o ponteiro de escrita e o ponteiro
        // de leitura for menor que a quantidade que queremos ler.
        // Se queremos ler mais do que foi escrito.
        // entao vamos ler apenas o que foi escrito.
        if (Count > delta){
            Count = delta;
        }

        // Atualizando o ponteiro de trabalho.
        // Vamos ler daqui.
        // A partir do offset de leitura.
        f->_p = (f->_base + f->_r);

        // read

        //#debug
        if (Count <= 0){
            printf ("file_read_buffer: Count <= 0 SECOND\n");
            goto fail;
            //printf("local_len\n");
            //refresh_screen();
            //return -1;
        }

        //---
        memcpy( (void *) buffer, (const void *) f->_p, Count ); 

        // Atualizamos o ponteiro de trabalho
        f->_p = (f->_p + Count);

        // Atualizamos o offset de leitura.
        // Ele é usado em relação à base.
        // #bugbug: Talvez essa leitura e escrita devesse ser em relação
        // ao ponteiro de trabalho.
        f->_r = (f->_r + Count);

        if ( f->_r > f->_w ){
            f->_r = f->_w;
        }

        // You also can write now.
        // But i can still read.
        f->_flags |= __SWR;
        f->sync.can_write = TRUE;

        return (int) Count;
    }

fail:
    //refresh_screen();
    return EOF;
}

/*
 * file_write_buffer: 
 * Escreve no arquivo uma certa quantidade 
 * de caracteres de uma dada string.
 */
// Escreve no arquivo uma certa quantidade de caracteres 
// de uma dada string.
// It's called by sys_write.
// OUT: number of written bytes.
int 
file_write_buffer ( 
    file *f, 
    char *string, 
    int len )
{

    char *p;
    p = string;

    // debug_print ("file_write_buffer:\n");

// File validation.
    if ((void *) f == NULL ){
        printf ("file_write_buffer: f\n");
        goto fail;
    }
    // #todo: Check used and magic.
    //if (f->magic != 1234)
        //return -1;

    if ((void *) p == NULL){
        printf ("file_write_buffer: p\n");
        goto fail;
    }

// Tentando escrever mais do que cabe no arquivo.
    if (len >= BUFSIZ){
        printf ("file_write_buffer: len > BUFSIZ\n");
        goto fail;
    }

//
// Copy
//

// Socket file.
// Se o arquivo é um socket, então não concatenaremos 
// escrita ou leitura.
    if (f->____object == ObjectTypeSocket){
        memcpy( (void *) f->_base, (const void *) string, len );
        return (int) len;
    }

// Pipe file.
// não concatenaremos
    if (f->____object == ObjectTypePipe){
        memcpy( (void *) f->_base, (const void *) string, len );
        return (int) len;
    }

// Regular file, TTY file, IO Buffer file.
// Tem que atualizar o ponteiro para que o próximo
// write seja depois desse write.
// Para isso o ponteiro precisa estar no base quando
// o write for usado pela primeira vez.
// Mas se o write for usado num arquivo aberto com 
// open(), então o ponteiro deve estar no fim do arquivo.
// Isso não significa que está no fim do buffer.
// Nesse caso o aplicative deveria usar rewind() antes
// para ter um ponteiro de escrita adequado.

    if ( f->____object == ObjectTypeFile ||     // regular file
         f->____object == ObjectTypeTTY  ||     // tty
         f->____object == ObjectTypeIoBuffer )  // ??
    {
        // #bugbug
        // Temos que ter um limite aqui ... !!!
        // #todo

        // se o tamanho do buffer for maior que o padrao.
        if (f->_lbfsize > BUFSIZ){
            printf ("file_write_buffer: _lbfsize\n");
            goto fail;
        }
    
        if (f->_w < 0)
        {
            f->_p = f->_base;
            f->_w = 0;
            f->_r = 0;
            f->_cnt = f->_lbfsize;
            return EOF;
        }

        // Se o offset de escrita ultrapassa os limites.
        if (f->_w >= BUFSIZ)
        {
            //#bugbug
            debug_print("file_write_buffer: f->_w >= BUFSIZ\n");
            printf     ("file_write_buffer: f->_w >= BUFSIZ\n");
            f->_w = BUFSIZ;
            f->_cnt = 0;
            return EOF;
        }

        //if ( f->_w 

        // recalculando quanto espaço temos.
        //f->_cnt = (f->_lbfsize - f->_w);

        // Se a quantidade que temos ultrapassa os limites.

        // fim do arquivo.
        if (f->_cnt < 0)
        {
            f->_cnt = 0;
            f->_w = f->_lbfsize;
            f->_r = f->_lbfsize;
            f->_p = (f->_base + f->_w);
            return EOF;
        }

        // Inicio do arquivo
        if (f->_cnt > f->_lbfsize)
        {
            printf ("file_write_buffer: _cnt\n");
            f->_cnt = f->_lbfsize;
            f->_p = f->_base;
            f->_w = 0;
            f->_r = 0;
        }

        if (len < 0){
            return -1;
        }

        // Se o que desejamos escrever é maior que o espaço que temos.
        if (len > f->_cnt)
        {
            // Estamos no fim do arquivo
            if ( f->_cnt <= 0 )
            {
                f->_w = f->_lbfsize;
                f->_r = f->_lbfsize;
                f->_p = f->_base + f->_lbfsize;
                f->_cnt = 0;
                return -1;
            }

            // Só podemos escrever esse tanto.
            if ( f->_cnt > 0 ){
                len = f->_cnt; 
            }
        }

        // write.
        // Se o offset de escrita é menor que o offset de leitura.
        // Então adiantaremos o offset de escrita.
        // pois nao devemos tocar no offset de leitura.
        if (f->_w < f->_r)
        {
            f->_w = f->_r;
            f->_cnt = ( f->_lbfsize - f->_w );
        }

        // Vamos escrever aqui.
        f->_p = (f->_base + f->_w);

        // Escrevemos usando o ponteiro de escrita.
        memcpy( (void *) f->_p, (const void *) string, len ); 
    
        // Atualizamos o ponteiro de escrita
        f->_p = (f->_p + len);

        // Atualizamos o offset de escrita.
        f->_w = (f->_w + len);
    
        // Atualizamos o quanto nos falta.
        f->_cnt = (f->_cnt - len);
        
        // You can read now.
        f->_flags = __SRD;
        f->sync.can_read = TRUE;

        //debug_print ("file_write_buffer: ok, done\n");

        // Retornamos a quantidade escrita no buffer.
        return (int) len;
    }

// Unknown type.
fail:
    //printf ("file_write_buffer: fail\n");
    //refresh_screen();
    return EOF;
}

/*
 * sys_read:
 *     ring0 read() implementation.
 *     Called via syscall.
 */
// Service 18.
// #todo: There's a lot to do with synchronization.
// #todo
// We need to call one subfunctions for
// each different kind of file intead of implementing
// an internal sub-routine.
// #todo
// Isso deveria ser apenas um wrapper,
// chamando as rotinas apropriadas para cada tipo 
// de arquivo.
// OUT:
// 0 = Couldn't read.
// -1 = Error.
// return 'ssize_t'?

ssize_t 
sys_read (
    int fd,
    char *ubuf,       //#todo: use 'void *'
    size_t count )    //#todo: use 'size_t'.
{

// File pointer.
    file *fp;

    ssize_t nbytes=0;
    struct socket_d *s;
    int ubuf_len=0;

// #bugbug
// O argumento é 'unsigned int'.
// Deveria ser int?

// fd
    if ( fd < 0 || fd >= OPEN_MAX ){
        return (ssize_t) (-EBADF);
    }
// ubuf
// todo: Checar a validade da região de memória.
    if ((void *) ubuf == NULL){
        return (ssize_t) (-EINVAL);
    }
// count
    if (count < 0){ 
        return (ssize_t) (-EINVAL);
    }
// Nothing to do.
    if (count == 0){ 
        return 0; 
    }

// Size of the buffer.
    ubuf_len = strlen( (const char *) ubuf );

//Se a quantidade desejada é maior que o buffer disponível,
//temos um problema.
    if (count > ubuf_len){
        //debug_print ("sys_read: [FIXME] count too large\n");
    }

// Se o buffer disponível é maior que a quantidade desejada
// então não temos problemas, nem precisamos disso. 
    if (ubuf_len > count){
        ubuf_len = count;
    }

// #fixme: 
// The buffer is too small.

    if (ubuf_len > 512){
        debug_print ("sys_read: [FIXME] limiting ubuf_len\n");
        //ubuf_len = 512;
    }

// File
// Get the object pointer.
    fp = (file *) get_file_from_fd(fd);
    if ((void *) fp == NULL)
    {
        debug_print("sys_read: fp not open\n");
        printf     ("sys_read: fp not open\n");
        goto fail; 
    }

    if (fp->sync.can_read != TRUE){
        debug_print("sys_read: [PERMISSION] Can NOT read the file\n");
        printf     ("sys_read: [PERMISSION] Can NOT read the file\n");
        goto fail; 
    }

/*
    // #todo
    // ainda nao inicializamos esse elemento.
    if( fp->is_readable == FALSE ){
        debug_print ("sys_read: Not readable\n");
        return -1;
    }
*/

    // #todo: Create thie element in the structure.
    // if( fp->is_directory == TRUE ){}

//==========================================================
// ::0
// stdin
    if (fp->_file == STDIN_FILENO){
        if (fp->____object == ObjectTypeFile){
            goto RegularFile;
        }
    }

//==========================================================
// ::1
// stdout
    if (fp->_file == STDOUT_FILENO){
        if (fp->____object == ObjectTypeFile){
            goto RegularFile;
        }
    }

//==========================================================
// ::2
// stderr
    if (fp->_file == STDERR_FILENO){
        if (fp->____object == ObjectTypeFile){
            goto RegularFile;
        }
    }

//
// == Socket file ===========================================
//

// ==== Socket =======================
// Se o arquivo for um socket.
// #todo: 
// Now we can call socket_read(...)

    if (fp->____object == ObjectTypeSocket)
    {
        // debug_print("sys_read: [DEBUG] Trying to read a socket object\n");
        do_credits_by_tid(current_thread);

        // not reading yet
        if ((fp->_flags & __SRD) == 0) 
        {
            //debug_print("sys_read: [FAIL] flag __SRD \n");
            //yield (current_thread);
            goto fail;
        }

        nbytes = 0;

        // vazio? 
        // nao podemos ler.
        // acorda escritores e dorme.
        if (fp->socket_buffer_full == FALSE)
        { 
            debug_print("sys_read: [FAIL] can't read an empty buffer\n");
            //goto fail;
            
            // #debug
            // debug_print("sys_read: WAKEUP WRITER\n");
            
            fp->_flags = 0;
            fp->_flags |= __SWR;                  // pode escrever
            //do_thread_ready( fp->tid_waiting );   // acorda escritores. 
            fp->tid_waiting = -1;

            // #bugbug
            // Isso pode ser ruim pela natureza da chamada sys_read()
            // que vem de uma syscall que nao salvou o contexto.

            if (fp->sync.block_on_read_empty == TRUE )
            {
                debug_print("sys_read: SLEEP READER\n");
                panic("sys_read: [DEBUG] Couldn't read socket. Buffer not full\n");

                fp->tid_waiting = current_thread;
                //do_thread_waiting (current_thread);
                yield (current_thread);
                goto fail;
            }
            yield (current_thread);
            goto fail;
        }

        // cheio? 
        // le e acorda escritores.
        if (fp->socket_buffer_full == TRUE)
        {
            if (fp->_flags & __SRD)
            {
                //debug_print ("sys_read: >>>> READ\n");
            
                // read!
                nbytes = 
                    (ssize_t) file_read_buffer ( 
                                  (file *) fp, 
                                  (char *) ubuf, 
                                  (int) count );

                if (nbytes <= 0){
                    debug_print("sys_read: [FAIL] file_read_buffer fail when reading a socket \n");
                    //yield (current_thread);
                    goto fail;
                }

                // ok
                if (nbytes > 0)
                {
                    //debug_print("sys_read: [DEBUG] lemos mais que 0 bytes em um socket.\n");
                    fp->socket_buffer_full = FALSE;     // buffer vazio
                    fp->_flags &= ~__SRD;  // nao posso mais LER.            
                    fp->_flags |= __SWR;   // pode escrever também
                    //debug_print("sys_read: WAKEUP WRITER\n");
                    //do_thread_ready( fp->tid_waiting );  // acorda escritores.
                    fp->tid_waiting = -1;
                    //debug_print("sys_read:done\n");
                    return (ssize_t) nbytes;    // bytes escritos.
                }
            }
        } 

        panic ("sys_read: Unexpected error when reading socket\n");
    }

//
// == Regular file ============================================
//

// ==== Regular file =======================
// Read a regular file.
// See: unistd.c
// #todo Tem que retornar a quantidade de bytes lido.
// #bugbug: Nem todos arquivos criados possuem essa flag configurada
// apropriadamente ...estamos fazendo isso.
// #hackhack
// We will open all the other kind of object
// with this IF right here.
// We need to fix it in the open() routines
// to give us the correct type.

RegularFile:

    if (fp->____object == ObjectTypeFile)
    {

         //#debug
         //if( fp->_file == 5)
         //{
         //    if ( (fp->_flags & __SRD) == 0 )
         //        printf("cant read\n");
         //    if ( fp->_flags & __SRD )
         //        printf("can read\n");
         //    printf("sys_read-OUTPUT 5: %s \n",fp->_base);
         //    refresh_screen();
         //}

        //debug_print("sys_read: [DEBUG] Trying to read a regular file object\n");
        
        //Se não pode ler.
        if ( (fp->_flags & __SRD) == 0 )
        {
            //debug_print("sys_read: [FLAGS] Can't read!\n");
        
            //Não conseguimos ler.
            //nada de errado, apenas espera.
            //do_thread_waiting (current_thread);
            //fp->tid_waiting = current_thread;
            //fp->_flags |= __SWR;  //pode escrever.
            //scheduler();
            
            //printf("sys_read: fp->_flags & __SRD \n");
            //refresh_screen();
            
            goto fail;
        }
        
        nbytes = 0;

        // Se puder ler:
        // + Call a function to read a regular file.
        // + Sinalize that another process can write.
        // #todo: wake the one that was waiting to write.

        if (fp->_flags & __SRD)
        {
            nbytes = 
                (ssize_t) file_read_buffer ( 
                              (file *) fp, 
                              (char *) ubuf, 
                              (int) count );
 
            if (nbytes<=0){
               //yield (current_thread);
               goto fail;
            }

            // Se conseguimos ler.
            if (nbytes>0)
            {
                //fp->_flags &= ~__SRD;  // nao posso mais LER.            
                //fp->_flags |= __SWR;   // pode escrever também
                // ok to write.
                fp->_flags |= __SWR;
                fp->sync.can_write = TRUE;
                // #test
                // Acordar quem esperava por esse evento
                //do_thread_ready( fp->tid_waiting );
                return (ssize_t) nbytes; 
            }

            //Não conseguimos ler.
            //nada de errado, apenas espera.
            //do_thread_waiting (current_thread);
            //fp->tid_waiting = current_thread;
            //fp->_flags |= __SWR;  //pode escrever.
            //scheduler();
            return 0;
        }

        // #todo
        // Do we need something here ?
        //return -1;
    }

// ========================================
// Pseudo terminal multiplexer.

    if (fp->____object == ObjectTypePTMX){
        printk ("sys_read: [TODO] trying to read a PTMX device file\n");
        //refresh_screen();
        return 0;
    }

// ========================================
// pty () pseudo terminal.

    if (fp->____object == ObjectTypePTY){
        printk ("sys_read: [TODO] trying to read a PTY device file\n");
        //refresh_screen();
        return 0;
    }

// ========================================
// file system

    if (fp->____object == ObjectTypeFileSystem){
        printk ("sys_read: [TODO] trying to read a file system\n");
        //refresh_screen();
        return 0;
    }

// ========================================
// See: pipe.c

    if (fp->____object == ObjectTypePipe){
        return (ssize_t) sys_read_pipe ( (int) fd, (char *) ubuf, (int) count ); 
    }

// ======================================================
    // ...
    debug_print ("sys_read: [FAIL] Unknown object type\n");
// ==========================================

//
// Fail
//
    
// #bugbug
// See manual for the error:
// https://www.gnu.org/software/libc/manual/html_node/I_002fO-Primitives.html
// http://man7.org/linux/man-pages/man2/read.2.html

// A intenção é que um erro seja representado por -1 e não por 0.
// Zero representará fim de arquivo. 

fail:

    // #debug
    // debug_print ("sys_read: [FAIL] something is wrong!\n");

// #bugbug: 
// Isso pode degradar o desempenho.

    // printf ("sys_read: [FAIL] something is wrong!\n");
    //refresh_screen();  
    //bloqueando, autorizando a escrita e reescalonando.
    //do_thread_waiting (current_thread);
    //fp->tid_waiting = current_thread;
    //fp->_flags |= __SWR;  //pode escrever      
    //scheduler();  //#bugbug: Isso é um teste  

// Something is wrong!
    return (ssize_t) (-1);
}

/*
 * sys_write:
 *     Implemantation of write() for libc.
 */
// service 19.
// Copiar um buffer para um arquivo dado o descritor.
// Aqui devemos selecionar o dispositivo à escrever.
// See:
// https://github.com/zavg/linux-0.01/blob/master/fs/read_write.c
// https://linux.die.net/man/2/write
// ...
// #todo
// We need to call one subfunction for each different kind of file 
// intead of implementing an internal sub-routine.

// #todo
// Isso deveria ser apenas um wrapper,
// chamando as rotinas apropriadas para cada tipo 
// de arquivo.

// #todo
// Copiar deve ser uma opcao e nao uma regra!
// Precisamos de uma flag para copyonwrite.
// atençao
// Podemos escrever num socket que nao eh o privado!!!
// existe a possibilidade de o processo atual estar 
// escrevendo em um socket que pertence a outro processo.
// mas que por causa da conexao possui o fd dele registrado
// em sua estrutura.
// Entao o fd passado por argumento nao precisa ser o mesmo
// do socket privado do processo. Pois poderemos estar
// escrevendo em outro socket que nao o privado.

// OUT:
// 0 = Couldn't read.
// -1 = Error.

ssize_t sys_write (int fd, char *ubuf, size_t count)
{

// File pointer.
    file *fp;

    ssize_t nbytes=0;
    struct socket_d *s1;
    struct socket_d *s2;
    int ubuf_len=0;
    size_t ncopy=0;

    //debug_print("sys_write: :)\n");

// fd
    if ( fd < 0 || fd >= OPEN_MAX ){
        return (ssize_t) (-EBADF);
    }
// ubuf
    if ((void *) ubuf == NULL){
        return (ssize_t) (-EINVAL);
    }
// count
    if (count < 0){ 
        return (ssize_t) (-EINVAL);
    }
// Nothing to do.
    if (count == 0){ 
        return 0; 
    }

//
// Size of the buffer.
//

// len
    ubuf_len = strlen( (const char *) ubuf );

// Se a quantidade desejada é maior que o buffer disponível.
    if(count > ubuf_len){
        //debug_print("sys_write: [FIXME] count too long!\n");
    }

// se o buffer é maior que a quantidade desejada, 
// então não temos problemas
    if (ubuf_len > count ){
        ubuf_len = count;
        //debug_print ("sys_write: [FIXME] Ajusting ubuf_len\n");
    }

// #debug: 
// limits
// Um socket tem o tamanho de BUFSIZ.

    if (ubuf_len > 512){
        ubuf_len = 512;
        debug_print ("sys_write: [FIXME] Ajusting ubuf_len to 512\n");
    }

// File
// Get the object pointer from the list
// in the process structure.

    fp = (file *) get_file_from_fd(fd);
    if ((void *) fp == NULL)
    {
        debug_print("sys_write: fp not open\n");
        printf     ("sys_write: fp not open #hang\n");
        //printf      ("fd{%d} pid{%d}\n",fd,current_process);
        //printf("entry0: %x\n", __P->Objects[0]);
        //printf("entry1: %x\n", __P->Objects[1]);
        //printf("entry2: %x\n", __P->Objects[2]);
        //printf("entry3: %x\n", __P->Objects[3]);
        //printf("entry4: %x\n", __P->Objects[4]);
        //refresh_screen();
        //while(1){}
        goto fail;
    }

    if (fp->sync.can_write != TRUE){
        debug_print("sys_write: [PERMISSION] Can NOT write the file\n");
        printf     ("sys_write: [PERMISSION] Can NOT write the file\n");
        goto fail; 
    }

/*
// #todo
// ainda nao inicializamos esse elemento.
    if( fp->is_writable == FALSE ){
        debug_print ("sys_write: Not writable\n");
        return -1;
    }
*/

// #todo: 
// Create thie element in the structure.
    //if( fp->is_directory == TRUE ){}

// =======================================================
// ::0
// stdin
// + Write on regular file.
    if (fp->_file == STDIN_FILENO)
    {
        if (fp->____object == ObjectTypeFile){
            goto RegularFile;
        }
        goto fail;
    }

// =======================================================
// ::1
// stdout
// + Write on console.
// + Write on regular file.
    if (fp->_file == STDOUT_FILENO)
    {
        // If the file is a console.
        if (fp->____object == ObjectTypeVirtualConsole)
        {
            return (int) console_write ( 
                             (int) fg_console, 
                             (const void *) ubuf, 
                             (size_t) count );
        }
        // If the file is a regular file.
        if (fp->____object == ObjectTypeFile){
            goto RegularFile;
        }
        goto fail;
    }

// =======================================================
// ::2 
// stderr
// + Write on regular file.
    if (fp->_file == STDERR_FILENO)
    {
        if (fp->____object == ObjectTypeFile){
            goto RegularFile;
        }
        goto fail;
    }

//
// == NIC device ===============================================
//

    if ( fp->____object == ObjectTypeNIC ){
       printf("sys_write: Writing on NIC device ... #todo\n");
       //network_on_sending(ubuf,count);
       return 0;
   }

//
// == Sockets ===============================================
//
    ncopy = count;

// == Socket ===============================
// Descobrindo o soquete que devemos copiar.
// Se o arquivo é do tipo socket, então devemos
// saber onde está o buffer.
// #todo: Talvez podemos chamar a função socket_write().
// #todo: Lidar com a questao do copiar, pois
// accept deve retornar o fd do cliente, para que o servidor
// construa uma lista de clientes.
// Entao copiar sera uma opçao, gerenciada por uma flag.

// #bugbug
// Nao podemos fazer a copia se os dois sockets 
// estiverem com a conexao pendente.
    if (fp->____object == ObjectTypeSocket)
    {
        do_credits_by_tid(current_thread);
        
        // Can't write.
        if ((fp->_flags & __SWR) == 0){
            debug_print("sys_write: [FAIL] flag __SWR \n");
            //yield (current_thread);
            goto fail;
        }

        // cheio? 
        // Não podemos escrever porque o buffer está cheio.
        // Acorde os leitores para esvaziar.
        // Mas e se não tiver mais escritores.
        // #todo: Indicação da quantidade de escritores.
        // Sendo assim, vamos dizer que também podem ler
        // caso existam leitores. Mas não pode mais escrever.
        if (fp->socket_buffer_full == TRUE)
        {
            
            debug_print("sys_write: [FAIL] can't write on a full buffer\n");
            
            //#debug
            //debug_print("sys_write: WAKEUP READER\n");
            
            fp->_flags = 0; // não pode mais escrever.
            fp->_flags |= __SRD;                 // pode ler.
            do_thread_ready( fp->tid_waiting );  // acorda leitores
            fp->tid_waiting = -1;
            if (fp->sync.block_on_write_full == TRUE)
            {
                 //#debug
                 //debug_print("sys_write: SLEEP WRITER\n");
                 fp->tid_waiting = current_thread;
                 //do_thread_waiting(current_thread);
                 yield (current_thread);
                 goto fail;
            }
            yield (current_thread); 
            goto fail;
        }

        // Se o buffer não estiver cheio,
        // Então escreva e acorde os leitores.
        if (fp->socket_buffer_full == FALSE)
        {
            // Se podemos escrever.
            // #todo: Ja fizemos isso logo acima.
            if ( fp->_flags & __SWR )
            {
                //debug_print ("sys_write: >>>> WRITE\n");
                fp->_flags = 0;

                // Write in the socket buffer.
                nbytes = 
                    (ssize_t) file_write_buffer ( 
                                  (file *) fp, 
                                  (char *) ubuf, 
                                  (int) count );

                // fail
                if (nbytes <= 0){
                    debug_print("sys_write: [FAIL] file_write_buffer couldn't write on socket \n");
                    //#todo: Isso pode afetar o desempenho.
                    yield (current_thread);
                    goto fail;
                }

                // ok, write funcionou.
                if (nbytes>0)
                {
                    // #debug
                    // debug_print("sys_write: WAKEUP READER\n");
                    
                    fp->socket_buffer_full = TRUE;     // buffer cheio
                    fp->_flags &= ~__SWR;              // nao posso mais ESCREVER.            
                    fp->_flags |= __SRD;               // pode ler 
                    //do_thread_ready(fp->tid_waiting);  // acorda leitores
                    fp->tid_waiting = -1;
                
                    // #bugbug
                    // Isso pode ser ruim pela natureza da chamada sys_write()
                    // que vem de uma syscall que nao salvou o contexto.
                    if (fp->sync.block_on_write == TRUE)
                    {
                        // #debug
                        //debug_print("sys_write: SLEEP WRITER\n");
                        
                        fp->tid_waiting = current_thread;
                        //do_thread_waiting(current_thread);
                    }
                    // #bugbug: test ...
                    //  impedir que eu mesmo me leia.
                    //yield (current_thread);
                    return (ssize_t) nbytes;
                }
            }
        }

        panic("sys_write: Unexpected error when writing on socket\n");
    }

//
// Arquivos normais (regular)
//

RegularFile:

// == Regular file =========================================
// Tem que retonar o tanto de bytes escritos.
// Escreve em uma stream uma certa quantidade de chars.

    if (fp->____object == ObjectTypeFile)
    {
        // Can't write.
        if ( (fp->_flags & __SWR) == 0){
             debug_print("sys_write: [FLAGS] Can't write!\n");
             return 0;
        }

        // Can write.
        if (fp->_flags & __SWR)
        {
            // Regular file.
            nbytes = 
                (ssize_t) file_write_buffer ( 
                              (file *) fp, 
                              (char *) ubuf, 
                              (int) count );

            // Avisa que o arquivo não está mais no modo escrita,
            // que agora pode ler.

            // Adiciona o bit que permite a leitura.
            // Assim o servidor pode ler o request.
            // #todo: wait on write.
            // #bugbug:
            // A questão é que se o cliente está esperando por resposta,
            // então ele lerá também.
            // Atualizamos a flag e acordamos quem esperava pelo evento.

            if (nbytes>0){
                fp->_flags = __SRD;
                fp->sync.can_read = TRUE;
                do_thread_ready(fp->tid_waiting);
                return (ssize_t) nbytes;
            }

            // suspenso.
            // Não conseguimos escrever ... 
            // nada de errado, apenas esperaremos.
            //do_thread_waiting (current_thread);
            //fp->tid_waiting = current_thread;
            //fp->_flags |= __SWR;  //pode escrever.
            //scheduler();
            debug_print ("sys_write: [FAIL] file_write_buffer fail!\n");
            return 0;
        }
 
        debug_print ("sys_write: [FAIL] Something is wrong!\n");
        return (ssize_t) (-1);
    } //regular file.

// ======================================================
// Pseudo terminal multiplexer.
    if (fp->____object == ObjectTypePTMX){
        printk ("sys_write: [TODO] trying to write a PTMX device file\n");
        //refresh_screen();
        return 0;
    }

// ======================================================
// pty () pseudo terminal.
    if (fp->____object == ObjectTypePTY){
        printk ("sys_write: [TODO] trying to write a PTY device file\n");
        //refresh_screen();
        return 0;
    }

// ======================================================
// File system
    if (fp->____object == ObjectTypeFileSystem){
        printk ("sys_write: [TODO] trying to write a file system\n");
        //refresh_screen();
        return 0;
    }

// ======================================================
// pipe:
// See: pipe.c
    if (fp->____object == ObjectTypePipe){
        return (ssize_t) sys_write_pipe ( (int) fd, (char *) ubuf, (int) count ); 
    }

    // ...

//==============================================
    debug_print ("sys_write: [FAIL] Unknown object type!\n");
//==============================================

fail:
    // #debug
    // debug_print ("sys_write: [FAIL] Something is wrong!\n");
    // printf      ("sys_write: [FAIL] something is wrong!\n");
fail2:

    //invalidate_screen();

// #bugbug: 
// Isso pode degradar o desempenho.
    //refresh_screen();   

    // Não conseguimos escrever ... 
    // Estamos com problemas 
    //do_thread_waiting (current_thread);
    //fp->tid_waiting = current_thread;
    //fp->_flags |= __SWR;  //pode escrever.
    //scheduler();

    return (ssize_t) (-1);
}

// sys_open:
// open() function implementation in ring 0.
// Syscall 16.
// #bugbug
// Precisamos de um endereço em ring 3
// para que a libc possa manipular o buffer ?
// ou open deve ser usado somente com dispositivos ??
// #obs:
// vamos retornar o indice da tabela de arquivos abertos 
// do processo atual.
// #? isso não pertence à fcntl.c?
// SVr4,  4.3BSD,  POSIX.1-2001. 
// OUT: fd.

int 
sys_open (
    const char *pathname, 
    int flags, 
    mode_t mode )
{
// Open a file, or create if is doesn't exist.
// creat chama open.
// open tenta ler num arquivo que nao existe?

    int value = -1;
    file *fp;

    //debug_print ("sys_open: $\n");

// #todo:
// check arguments.

    if ((void*) pathname == NULL){
        return (int) (-EINVAL);
    }
    if (*pathname == 0){
        return (int) (-EINVAL);
    }

//
// Local copy
//

    char pathname_local_copy[256];
    memset(pathname_local_copy,0,256);
    // Coping more than we need, 
    // this way we're coping the 0x00 byte at the and of string
    // and some extra bytes.
    strncpy(pathname_local_copy,pathname,256);

//----------------------------

// Searth for a device associated with this path
// in the deviceList[]
// See: devmgr.c

    fp = (file *) devmgr_search_in_dev_list(pathname_local_copy);
    // Yes, we have a valid pointer 
    // found in the table.
    if ((void*) fp != NULL)
    {
        if (fp->isDevice == TRUE)
        {
            // #todo 
            // Put it into the list inside the
            // current process structure
            // and return the fd.
            printf("sys_open: #todo isDevice!\n");
            //refresh_screen();
            return -1;
        }
    }

// Vamos carregar o arquivo que esta no disco.
// See: fs.c
// OUT: fd

    value = 
        (int) do_read_file_from_disk ( 
                  (char *) pathname_local_copy, 
                  flags, 
                  mode );

    if (value<0){
        return (int) -1;
    }

// The limit is 32.
// Too many open files.

    if (value>31){
        return (int) (-EMFILE);
    }

// fd
    // #debug
    // refresh_screen();
    return (int) value;
}


/*
 *  sys_close:
 */
// Fechar um dos objetos abertos do processo atual.
// O descritor é um índice na sua tabela de objetos abertos.
// #todo: Se fecharmos um socket, tem que antes destruir a estrutura
// de socket associada ao arquivo.
// See:
// https://man7.org/linux/man-pages/man2/close.2.html
// https://pubs.opengroup.org/onlinepubs/009695399/functions/close.html
/*
A successful close does not guarantee that the data has been
successfully saved to disk, as the kernel uses the buffer cache
to defer writes.  Typically, filesystems do not flush buffers
when a file is closed.  If you need to be sure that the data is
physically stored on the underlying disk, use fsync(2).  (It will
depend on the disk hardware at this point.)
*/

int sys_close(int fd)
{
    file *object;
    struct process_d *p;
    int Done=FALSE;

    pid_t current_process = (pid_t) get_current_process();

// ??
// Can we close this devices?

    /*
    if (fd == 0 || fd == 1 || fd == 2 )
    {
        debug_print("sys_close: [FIXME] We can't close the standard stream\n");
        // WHY NOT ???!!
        return (int) (-1);
    }
    */

// Invalid fd
    if ( fd < 0 || fd >= OPEN_MAX )
    {
        debug_print("sys_close: bad fd\n");
        return (int) (-EBADF);
    }

// Process
// #todo: There is a helper for that small routine.
    if (current_process < 0 || current_process >= PROCESS_COUNT_MAX){
        debug_print("sys_close: current_process\n");
        goto fail;
    }
    p = (void *) processList[current_process];
    if ((void *) p == NULL){
        debug_print("sys_close: p\n");
        goto fail;
    }
    if ( p->used != TRUE || p->magic != 1234 ){
        debug_print("sys_close: p validation\n");
        goto fail;
    }

// object
// The object is a file structure.

    object = (file *) p->Objects[fd];
    if ((void *) object == NULL){
        debug_print("sys_close: object\n");
        goto fail;
    }
    if ( object->used != TRUE || object->magic != 1234 ){
        debug_print("sys_close: object validation\n");
        goto fail;
    }

// What type of object?
// socket, pipe, virtual console, tty, regular file ??

    Done = FALSE;

// ===============================================
// socket
// Do NOT save it into the disk.
    if (object->____object == ObjectTypeSocket)
    {
        debug_print("sys_close: Trying to close a socket object\n");
        object->socket->used = FALSE; //invalidando a estrutura de socket
        object->socket->magic = 0;//invalidando a estrutura de socket
        Done = TRUE;
    }

// ==============================================
// pipe
    if (object->____object == ObjectTypePipe)
    {
        debug_print("sys_close: Trying to close a pipe object\n");
        Done = TRUE;
    }

// ====================================================
// virtual console.
// #todo
    if (object->____object == ObjectTypeVirtualConsole)
    {
        debug_print("sys_close: Trying to close a virtual console object\n");
        return 0;
    }

// =====================================================
// tty
    if (object->____object == ObjectTypeTTY)
    {
        debug_print("sys_close: Trying to close a tty object\n");
        Done = TRUE;
    }

// #bugbug
// Poderemos ter problemas aqui com os diversos tipos
// de arquivos.

// #bugbug
// ugly test

// ===========================================
// regular file
// Save the file if it is a regular file.
    if (object->____object == ObjectTypeFile)
    {
        debug_print("sys_close: [FIXME] trying to close a regular file\n");
        debug_print("sys_close: [FIXME] fsSaveFile\n");

        // #fixme: buffer limit
        if (object->_lbfsize < 512)
        {  
            debug_print("sys_close: [FIXME] Ajusting file size\n");
            object->_lbfsize = 512; 
        }

        // Save file in the root dir.
        // #bugbug: Where to save?

        fsSaveFile ( 
            VOLUME1_FAT_ADDRESS, 
            VOLUME1_ROOTDIR_ADDRESS, 
            FAT16_ROOT_ENTRIES,
            (char *)         object->_tmpfname, 
            (unsigned long) (object->_lbfsize/512),  // file_size, in sectors       
            (unsigned long)  object->_lbfsize,       // size_in_bytes,  
            (char *)         object->_base,          // buffer address          
            (char)           0x20 );                 // flag ??

        Done = TRUE;
    }

// Object type not supported.
    if (Done != TRUE){
        debug_print("sys_close:[FAIL] Object type not supported yet \n");
        goto fail;
    }

// Everything was done.
// Let's destroy the object pointer, empty the spot in the list
// and return 0.
    if (Done == TRUE)
    {
        object = NULL;
        p->Objects[fd] = (unsigned long) 0;
        return 0;
    }

fail:
    debug_print("sys_close: [FAIL]\n");
    return (int) (-1);
}

// #todo: 
// Rever esses argumentos.
// SVr4, 4.3BSD, POSIX.1-2001. and more.
// See: 
// https://man7.org/linux/man-pages/man2/fcntl.2.html

int sys_fcntl ( int fd, int cmd, unsigned long arg )
{
    debug_print ("sys_fcntl: #todo\n");

    if ( fd < 0 || fd >= OPEN_MAX ){
        return (int) (-EBADF);
    }
    if (cmd < 0){
        return (int) (-EINVAL);
    }

// POSIX Table 6-1.
// See: fcntl.h

    switch (cmd){

    //duplicate file descriptor
    case F_DUPFD:
        debug_print ("sys_fcntl: [TODO] F_DUPFD\n");
        return -1;
        break;

    //get file descriptor flags
    case F_GETFD:
        debug_print ("sys_fcntl: [TODO] F_GETFD\n");
        return -1;
        break;

    //set file descriptor flags
    case F_SETFD:
         debug_print ("sys_fcntl: [TODO] F_SETFD\n");
         return -1;
         break;

    //get file status flags
    case F_GETFL:
        debug_print ("sys_fcntl: [TODO] F_GETFL\n");
        return -1;
        break;

    //set file status flags
    case F_SETFL:
        debug_print ("sys_fcntl: [TODO] F_SETFL\n");
        return -1;
        break;

    //get record locking information
    case F_GETLK:
        debug_print ("sys_fcntl: [TODO] F_GETLK\n");
        return -1;
        break;

    // set record locking information
    case F_SETLK:
        debug_print ("sys_fcntl: [TODO] F_SETLK\n");
        return -1;
        break;

    //set record locking info; wait if blocked
    case F_SETLKW:
        debug_print ("sys_fcntl: [TODO] F_SETLKW\n");
        return -1;
        break;

    //free a section of a regular file
    case F_FREESP:
        debug_print ("sys_fcntl: [TODO] F_FREESP\n");
        return -1;
        break;

    // ...

    default:
        debug_print ("sys_fcntl: default command\n");
        break;
    };

    debug_print ("sys_fcntl: FAIL\n");

    return (int) -1; //#todo
}


// Called by sc82 in sci.c
// Enquanto sys_ioctl eh chamada pelos applicativos,
// io_ioctl eh chamada pelas rotinas dentro do kernel.
// See: io.c
int sys_ioctl( int fd, unsigned long request, unsigned long arg )
{
// ioctl() implementation.

    int status=-1;

    debug_print ("sys_ioctl: [FIXME] \n");
    //printf("sys_ioctl: [FIXME] \n");

    if ( fd < 0 || fd >= OPEN_MAX ){
        return (int) (-EBADF);
    }

    status = (int) io_ioctl(fd,request,arg);
    if (status<0)
    {
        //?
    }

    // #debug
    // We need to see the error messages.
    //refresh_screen();

    return (int) status;
}


// helper
// dado o fd, pegamos o ponteiro 
// para estrutura de arquivo na lista de
// objetos abertos no processo atual.
// ## Talvez essa rotina ja foi implementada
// em algum outro lugar.
// Prototype in rtl/sci/sys.h
file *get_file_from_fd(int fd)
{
// File pointer
    file *fp;
// Current process
    struct process_d *p;

    pid_t current_pid = (pid_t) get_current_process();
    if ( current_pid < 0 || current_pid >= PROCESS_COUNT_MAX ){
        return NULL;
    }
    p = (struct process_d *) processList[current_pid];
    if ((void*) p == NULL){
        debug_print ("get_file_from_fd: p\n");
        panic       ("get_file_from_fd: p\n");
        //return NULL;
    }
    if (p->used != TRUE){
        return NULL;
    }
    if (p->magic != 1234){
        return NULL;
    }

    if (fd < 0 || fd >= 32){
        goto fail;
    }
    fp = (file *) p->Objects[fd];
    if ( (void*) fp == NULL ){
        //#debug
        //printf("fd{%d} pid{%d}\n",fd,current_pid);
        //printf("entry0: %x\n", p->Objects[0]);
        //printf("entry1: %x\n", p->Objects[1]);
        //printf("entry2: %x\n", p->Objects[2]);
        //printf("entry3: %x\n", p->Objects[3]);
        //printf("entry4: %x\n", p->Objects[4]);
        goto fail;
    }

    return (file *) fp;
fail:
    return NULL;
}

// 10003
// Get action.
// See: kstdio.h
int sys_get_global_sync (int sync_id, int request)
{ 
    struct kstdio_sync_d *s;

    if ( sync_id < 0 || sync_id >= SYNC_COUNT_MAX )
    {
       //message? panic?
       return -1;
    }

    s = (struct kstdio_sync_d *) syncList[sync_id];

    if (request == SYNC_REQUEST_GET_ACTION)
    {
        if ((void*) s == NULL){
            goto fail;
        }
        return (int) s->action;
    }

fail:
    return -1; 
}

// 10002
// Set action.
// See: kstdio.h
void sys_set_global_sync(int sync_id, int request, int data)
{
    struct kstdio_sync_d *s;

    if ( sync_id < 0 || sync_id >= SYNC_COUNT_MAX )
    {
       //message? panic?
       return;
    }

    s = (struct kstdio_sync_d *) syncList[sync_id];

    if (request == SYNC_REQUEST_SET_ACTION)
    {
        if ((void*) s != NULL){
            s->action = data;
        }
    }
}

// 10004
// Create sync structure and
// put the pointer into the list.
// See: kstdio.h
int sync_count=0;
static int __saved_sync_id=0;

int sys_create_new_sync(void)
{
    struct kstdio_sync_d *s;

    s = (struct kstdio_sync_d *) kmalloc( sizeof(struct kstdio_sync_d) );
    if ( (void*) s == NULL ){
        return (int) -1;
    }
    s->used=TRUE;
    s->magic=1234;
    s->action = ACTION_NULL;
    sync_count++;
    if ( sync_count > 0 && sync_count < SYNC_COUNT_MAX )
    {
        syncList[sync_count] = (unsigned long) s;
        __saved_sync_id = (int) sync_count;
        return (int) sync_count;
    }
    return (int) -1;
}

// Get the id.
// provisorio, para testes
// See: kstdio.h
int get_saved_sync(void)
{
    return (int) __saved_sync_id;
}

// --------------------------------------------------
// service 10006
// #todo
// Comment the purpose of this routine.
// It is used on socket communication.
void sys_set_file_sync(int fd, int request, int data)
{
    struct process_d  *p;
    file *object;

    pid_t current_process = (pid_t) get_current_process();
    
    //#bugbug
    // Pensaremos nessa possibilidade.

/*
    if (fd == 0 || fd == 1 || fd == 2 )
    {
        debug_print("sys_close: [FIXME] We can't close the standard stream\n");
        // WHY NOT ???!!
        return (int) (-1);
    }
 */

    if ( fd < 0 || fd >= OPEN_MAX ){
        debug_print("sys_set_file_sync: fd\n");
        return;
    }

// == Process ================

    if ( current_process < 0 || current_process >= PROCESS_COUNT_MAX ){
        debug_print("sys_set_file_sync: current_process\n");
        return;
    }

    p = (void *) processList[current_process];
    if ((void *) p == NULL){
        debug_print("sys_set_file_sync: p\n");
        return;
    }
    if (p->magic != 1234){
        debug_print("sys_set_file_sync: p validation\n");
        return;
    }

// object
// Everything is a file.

    object = (file *) p->Objects[fd];
    if ((void*) object == NULL){
        debug_print("sys_set_file_sync: object\n");
        return;
    }
    if ( object->used != TRUE || object->magic != 1234 ){
        debug_print("sys_set_file_sync: validation\n");
        return;
    }

// request.

    switch (request){

    // set last action
    case SYNC_REQUEST_SET_ACTION:
        object->sync.action = data;
        break;

    // #test
    // reset
    // Now we can write
    // SYNC_REQUEST_RESET_WR
    case 216:

        //#debug
        //printf("216:\n"); 
        //refresh_screen();

        object->sync.action = 0;
        //object->_flags = (__SWR | __SRD); 
        object->_flags = __SWR;
        k_rewind(object);
        object->_r = 0;
        object->_w = 0;
        object->socket_buffer_full = FALSE; //empty buffer
        return;
        break;

    // #test
    // Now we can read
    // SYNC_REQUEST_RESET_RD
    case 217:

        //#debug
        //printf("217:\n"); 
        //refresh_screen();

        object->sync.action = 0;
        //object->_flags = (__SWR | __SRD); 
        object->_flags = __SRD;
        return;
        break;

    // ...

    default:
        debug_print("sys_set_file_sync: [FAIL] Default request\n");
        return;
        break;
    };

    // ...
}

// ---------------------------------
// service 10007
// #todo
// Comment the purpose of this routine.
// It is used on socket communication.
// #todo: Explain the output values.
int sys_get_file_sync(int fd, int request)
{
    struct process_d  *p;
    file *object;

    pid_t current_process = (pid_t) get_current_process();
    
    //#bugbug
    // Pensaremos nessa possibilidade.

/*
    if (fd == 0 || fd == 1 || fd == 2 )
    {
        debug_print("sys_close: [FIXME] We can't close the standard stream\n");
        // WHY NOT ???!!
        return (int) (-1);
    }
 */

    if ( fd < 0 || fd >= OPEN_MAX ){
        return (int) (-EBADF);
    }

// == Process ================

    if ( current_process < 0 || current_process >= PROCESS_COUNT_MAX ){
        debug_print("sys_get_file_sync: current_process\n");
        return (int) (-1);
    }

    p = (void *) processList[current_process];
    if ((void *) p == NULL){
        debug_print("sys_get_file_sync: p\n");
        return (int) (-1);
    }
    if (p->magic != 1234){
        debug_print("sys_get_file_sync: p validation\n");
        return (int) (-1);
    }

// Object

    object = (file *) p->Objects[fd];
    if ((void*) object == NULL){
        debug_print("sys_get_file_sync: [FAIL] object\n");
        return (int) (-1);
    }
    if ( object->used != TRUE || object->magic != 1234 ){
        debug_print("sys_get_file_sync: validation\n");
        return (int) (-1);
    }

    switch (request){
    // get last action
    case SYNC_REQUEST_GET_ACTION:
        return (int) object->sync.action;
        break;
    //case ?:
        //break;
    // ...
    default:
        debug_print("sys_get_file_sync: [FAIL] Default request\n");
        return (int) (-1);
        break;
    };

// ...

// ?? Why '0'?
    return 0;
}

// Get the device number in the dev_dir[] list
// given the pathname.
// ex: "/DEV/DEV1"
int sys_get_device_number_by_path(char *path)
{
    return -1;
}

// #test
// #todo
// Precisamos uma syscall para esse serviço.
// Open device by number.
// Index in the dev_dir[] list.
// Incluir o ponteiro para arquivo na lista 
// de arquivos abertos do processo atual.
// See: sys.h

int sys_open_device_by_number(int device_index)
{
    struct process_d *p;
    file *fp;
    register int __slot=0;
    int i = device_index;

    if (i<0 || i>=32){
        return (int) (-EINVAL);
    }
    if (dev_dir[i].magic != 1234){
        return (int) -1;
    }
    if (dev_dir[i].initialized != TRUE){
        return (int) -1;
    }

// Get file pointer.
    fp = (file*) dev_dir[i].fp;
    if ((void*) fp == NULL){
        return (int) -1;
    }
    if (fp->magic != 1234){
        return (int) -1;
    }

// Put the pointer into the list.

// Process
    pid_t current_process = (pid_t) get_current_process();
    if (current_process < 0 || current_process >= PROCESS_COUNT_MAX){
        return -1;
    }
    p = (struct process_d *) processList[current_process];
    if ((void *) p == NULL){
        debug_print ("sys_open_device_by_number: p\n");
        return -1;
    }
    if ( p->used != TRUE || p->magic != 1234 ){
        debug_print ("sys_open_device_by_number: validation\n");
        return -1;
    }

// Probing for a free slot.
    for (__slot=0; __slot<32; __slot++)
    {
         if ( p->Objects[__slot] == 0 ){
             goto __OK;
         }
    };

// fail
    panic ("sys_open_device_by_number: No slots!\n");

// Slot found.
__OK:

    if ( __slot < 0 || __slot >= 32 ){
        printf ("sys_open_device_by_number: __slot fail\n");
        //refresh_screen();
        return (int) (-1);
    }

// save
    p->Objects[__slot] = (unsigned long) fp;

// Return fd.
    return (int) __slot;
}

// 178
// Only root dir.
//#todo: 
// unsigned long sys_get_file_size ( char *path, unsigned long dir_address )

unsigned long sys_get_file_size(char *path)
{
    unsigned long FileSize=0;

    if ((void*) path == NULL){
        debug_print("sys_get_file_size: path\n");
        return 0;
    }
    if ( *path == 0 ){
        debug_print("sys_get_file_size: *path\n");
        return 0;
    }

    FileSize = 
    (unsigned long) fsGetFileSize ( 
                        (unsigned char *) path, 
                        (unsigned long) VOLUME1_ROOTDIR_ADDRESS );

    return (unsigned long) FileSize; 
}

// IN: fd
// OUT: 
// -1= error; 
// FALSE= nao pode ler; 
// TRUE= pode ler.

int sys_sleep_if_socket_is_empty(int fd)
{
    struct process_d *p;
    file *object;

    pid_t current_process = (pid_t) get_current_process();
    if ( current_process < 0 || current_process >= PROCESS_COUNT_MAX ){
        debug_print("sys_sleep_if_socket_is_empty: current_process\n");
        return (int) (-1);
    }

    if ( fd < 0 || fd >= OPEN_MAX ){
        return (int) (-EBADF);
    }

// process
    p = (void *) processList[current_process];
    if ( (void *) p == NULL ){
        debug_print("sys_sleep_if_socket_is_empty: p\n");
        return (int) (-1);
    }
    if (p->magic != 1234){
        return (int) (-1);
    }

// Object

    object = (file *) p->Objects[fd];
    if ( (void *) object == NULL ){
        debug_print("sys_sleep_if_socket_is_empty: object\n");
        return (int) (-1);
    }
    if (object->used != TRUE || object->magic != 1234 ){
        debug_print("sys_sleep_if_socket_is_empty: object validation\n");
        return -1;
    }

// Is it a socket object?
    if ( object->____object != ObjectTypeSocket ){
        debug_print ("sys_sleep_if_socket_is_empty: [ERROR] only for sockets\n");
        return -1;
    }

    // TRUE or FALSE
    //return (int) object->socket_buffer_full;   

// Sim esta vazio, 
// nao pode ler;  
// retorna FALSE
    if ( object->socket_buffer_full == FALSE )
    {
        debug_print("sys_sleep_if_socket_is_empty: Buffer is empty. we can not read. sleeping\n");
        object->_flags |= __SWR;                  // pode escrever
        //todo: falg que nege a leitura.
        object->tid_waiting = current_thread;     // thread atual dorme   
        //do_thread_waiting (current_thread);
        return FALSE;  // nao pode ler
    }

// O buffer esta cheio, pode ler. retorna TRUE
    if ( object->socket_buffer_full == TRUE ){
        debug_print("sys_sleep_if_socket_is_empty: Buffer is empty. we can read\n");
        object->_flags |= __SRD;
        return TRUE;
    }

//#bugbug
//Fail
    debug_print ("sys_sleep_if_socket_is_empty: [FAIL] Unexpected error \n");
    return -1;
}

// Get free slots in the file_table[].
int get_free_slots_in_the_file_table(void)
{
// Called by kstdio.c when creating streams.

    file *tmp;
    register int i=0;

    if (FileTableInfo.initialized != TRUE){
        panic("get_free_slots_in_the_file_table: FileTableInfo not initialized\n");
    }

    for (i=0; i<NUMBER_OF_FILES; i++)
    {
        tmp = (void*) file_table[i];
        // Nenhum file descritor está usando essa estrutura.
        if (tmp->used == TRUE && 
            tmp->magic == 1234 && 
            tmp->fd_counter == 0)
        { 
            return (int) i;
        }
    };

    return (int) -1;
}

// Get free slots in the inode_table[].
int get_free_slots_in_the_inode_table(void)
{
    struct inode_d *tmp;
    register int i=0;

    for (i=0; i<32; i++)
    {
        tmp = (void*) inode_table[i];
        // Se nenhum descritor de estrutura de arquivo 
        // está usando essa estrutura inode.
        if (tmp->used == TRUE && 
            tmp->magic == 1234 && 
            tmp->filestruct_counter == 0)
        { 
            return (int) i; 
        }
    }

    return (int) -1;
}

/*
 * get_filesystem_type:
 * Pega o tipo de sistema de arquivos.
 * ?? #bugbug: De qual volume ??  
 */
int get_filesystem_type (void)
{
    return (int) g_currentvolume_filesystem_type;
}

/*
 * set_filesystem_type:
 *     Configura o tipo de sistema de arquivo.
 *     ?? #bugbug: De qual volume ?? 
 */

void set_filesystem_type (int type)
{
    g_currentvolume_filesystem_type = (int) type;
}

// Counting path levels.
// Credits: Sirius OS.
// return 'int'?
unsigned long fs_count_path_levels (unsigned char *path)
{
    unsigned long Counter=0;
    register int i=0;
    int MaxChars = 2000;  //(80*25), 25 lines.

    if ( (void*) path == NULL ){ return 0; }
    if ( *path == 0 )          { return 0; }

    for ( i=0; i < MaxChars; ++i )
    {
        if (path[i] == '/') { Counter++; }
        if (path[i] == '\0'){ break; }
    };

    return (unsigned long) Counter;
}

/*
 * get_file:
 *     Get the pointer given the index in file_table[].
 */
// na lista de arquivos do kernel.
void *get_file (int Index)
{
//Limits.
//#todo: max.

    if (Index < 0)
    {
        // ?? todo: message
        return NULL;
    }

    return (void *) file_table[Index];
}

// set_file:
// Put the pointer in the list, given the index.
// Na lista de arquivos do kernel.
void set_file ( void *file, int Index )
{
    if (Index < 0){
        // ?? todo: message
        return;
    }

// #todo:
// Limite máximo da lista.

// Structure.

    if ( (void *) file == NULL )
    {
        // ?? todo: message
        return;
    }

// Include pointer in the list.

    file_table[Index] = (unsigned long) file;
}

int fs_initialize_dev_dir(void)
{
    register int i=0;

// Clear the list
    for (i=0; i<32; i++){
        dev_dir[i].used = FALSE;
        dev_dir[i].magic = FALSE;
        dev_dir[i].initialized = FALSE;
        dev_dir[i].path[0] = 0;
        dev_dir[i].fp = NULL;
    };

    return 0;
}


void initialize_FAT_and_main_directories(void)
{
// >> So podemos chamar isso depois de termos
// inicialiado o hardware de disco ...
// see: I_init in x64init.c
// Load FAT for the first time
// Load root dir for the first time.
// Load the other main directories for the first time.

   size_t name_size=0;

// ------------------
// FAT
    fs_load_fat( VOLUME1_FAT_ADDRESS, VOLUME1_FAT_LBA, 246 );
    sfMainFAT.initialized = TRUE;

// ------------------
// ROOT
    char *root_name = "/";

    fs_load_rootdir ( 
        VOLUME1_ROOTDIR_ADDRESS, 
        VOLUME1_ROOTDIR_LBA, 
        32 );

    sdROOT.address = (unsigned long) VOLUME1_ROOTDIR_ADDRESS;
    sdROOT.number_of_entries = 512;
// name
    name_size = strlen(root_name);
    if (name_size <= 0 || name_size >= 64)
        x_panic("root_name");
    sdROOT.name_size = (size_t) name_size; 
    sprintf( sdROOT.name, root_name );
    sdROOT.initialized = TRUE;

// ------------------
// PROGRAMS
// Load the directory that belongs to the root dir.
    char *programs_name = "PROGRAMS";

    unsigned long programs_buffer=0;
    unsigned long programs_buffer_size=0;   //in bytes

    // #bugbug
    // se carregarmos mais arquivos lá do que isso,
    // então teremos problemas para ler.
    // # isso é so um teste por enquanto.
    // Podemos melhorar essa rotina no futuro.
    // 32 setores, igual o root.
    // 512 * 32 = 16384 bytes
    // 16384/4096 = 4 páginas.
    programs_buffer = 
        (unsigned long) allocPages(8);  //#todo: A lot bigger when possible.

    if ( (void*) programs_buffer == NULL ){
        x_panic("programs_buffer");
    }

    // size in bytes
    // 32 setores.
    programs_buffer_size = 16384; //(32*512)

    unsigned long Ret = 1;  // 1 fail | 0 ok.
    Ret = fsLoadFile ( 
              (unsigned long) VOLUME1_FAT_ADDRESS,      // fat address
              (unsigned long) VOLUME1_ROOTDIR_ADDRESS,  // dir address. onde procurar.
              (int) 512,                                // #bugbug: Number of entries.  
              (const char *) programs_name,  // nome do diretorio 
              (unsigned long) programs_buffer,  // onde carregar. 
              (unsigned long) programs_buffer_size ); // tamanho do buffer onde carregar.                            

    if (Ret!=0){
        x_panic("Couldn't load PROGRAMS directory");
    }

    sdPROGRAMS.address = (unsigned long) programs_buffer;
    sdPROGRAMS.number_of_entries = 512;
// name
    name_size = strlen(programs_name);
    if (name_size <= 0 || name_size >= 64)
        x_panic("programs_name");
    sdPROGRAMS.name_size = (size_t) name_size; 
    sprintf( sdPROGRAMS.name, programs_name );
    sdPROGRAMS.initialized = TRUE;

// ----------------------            
   //...

}

// ------------------------------
// fsInit:
// Called by I_init() in x64init.c
int fsInit (void)
{
    register int i=0;
    int slot = -1;

    debug_print ("fsInit: [TODO]\n");

// Buffers
// Buffers for loading the directories while walking on a pathname
// when loading a file.

    int max = FS_N_BUFFERS;  // Number of levels.
    unsigned long addr=0;

    for (i=0; i<max; i++)
    {
        //addr = (unsigned long) allocPages(1);
        addr = (unsigned long) allocPages(4);  //32*512=16KB
        if ((void*) addr == NULL){
            panic("fsInit: addr\n");
        }
        fs_buffers[i] = (unsigned long) addr;
    }

// Undefined fs!
    set_filesystem_type(FS_TYPE_NULL);

// Initialize fat16 support for the system's volume.
// #todo: 
// Devemos checar o tipo da partiçao de boot. 
// Se nao aqui, depois!
    fat16Init();

// Init dev/ dir.
    fs_initialize_dev_dir();

//
// == fileList =========================
//

// Initializing the file pointers
// volume1_rootdir_fp and volume2_rootdir_fp.
// See: kstdio.h

//
// == volume1_rootdir ================================
//

    // pega slot em file_table[] para
    slot = get_free_slots_in_the_file_table();
    if (slot<0 || slot >= NUMBER_OF_FILES){
        panic("fsInit: slot");
    }
    volume1_rootdir_fp = (file *) file_table[slot];
    volume1_rootdir_fp->filetable_index = slot;

    if ((void *) volume1_rootdir_fp == NULL){
        panic ("fsInit: volume1_rootdir_fp\n");
    } else {
        volume1_rootdir_fp->used  = TRUE;
        volume1_rootdir_fp->magic = 1234;
        volume1_rootdir_fp->____object = ObjectTypeVolume;

        volume1_rootdir_fp->_base = (unsigned char *) VOLUME1_ROOTDIR_ADDRESS;
        volume1_rootdir_fp->_p    = (unsigned char *) VOLUME1_ROOTDIR_ADDRESS;
        volume1_rootdir_fp->_cnt = (32 * 512);
        volume1_rootdir_fp->_file = 0;
        volume1_rootdir_fp->_tmpfname = "VOLUME1 VOL";
        volume1_rootdir_fp->fd_counter = 1;

        // #bugbug: 
        // Validade da estrutura.
        
        // ...
        // inode support.
        // pega slot em inode_table[] 
        slot = get_free_slots_in_the_inode_table();
        if (slot<0 || slot >=32){
            panic("fsInit: volume1_rootdir_fp inode slot");
        }
        volume1_rootdir_fp->inode = (struct inode_d *) inode_table[slot];
        volume1_rootdir_fp->inodetable_index = slot;
        if ( (void*) volume1_rootdir_fp->inode == NULL ){
            panic("fsInit: volume1_rootdir_fp inode struct");
        }
        volume1_rootdir_fp->inode->filestruct_counter = 1;  //inicialize
        memcpy ( 
            (void*)       volume1_rootdir_fp->inode->path, 
            (const void*) volume1_rootdir_fp->_tmpfname, 
                  sizeof( volume1_rootdir_fp->inode->path ) );
        // ... 

        // File that represents a volume
        //#todo: Change to 'storage->__bootvolume_fp'.
        storage->__file = volume1_rootdir_fp; 
    };


//
// == volume2_rootdir =========================================== 
//
    // pega slot em file_table[] para
    slot = get_free_slots_in_the_file_table();
    if (slot<0 || slot >= NUMBER_OF_FILES){
        panic("fsInit: slot");
    }
    volume2_rootdir_fp = (file *) file_table[slot];
    volume2_rootdir_fp->filetable_index = slot;

    if ((void *) volume2_rootdir_fp == NULL){
        panic ("fsInit: volume2_rootdir_fp\n");
    }else{
        volume2_rootdir_fp->used  = TRUE;
        volume2_rootdir_fp->magic = 1234;
        volume2_rootdir_fp->____object = ObjectTypeVolume;
 
        volume2_rootdir_fp->_base = (unsigned char *) VOLUME2_ROOTDIR_ADDRESS;
        volume2_rootdir_fp->_p    = (unsigned char *) VOLUME2_ROOTDIR_ADDRESS;
        volume2_rootdir_fp->_cnt = (32 * 512);  // #bugbug: Check this size.
        volume2_rootdir_fp->_file = 0;          // ?
        volume2_rootdir_fp->_tmpfname = "VOLUME2 VOL";
        volume2_rootdir_fp->fd_counter = 1;

        // ...
        // inode support.
        // pega slot em inode_table[] 
        slot = get_free_slots_in_the_inode_table();
        if (slot<0 || slot >= 32){
            panic("fsInit: volume2_rootdir inode slot");
        }
        volume2_rootdir_fp->inode = (struct inode_d *) inode_table[slot];
        volume2_rootdir_fp->inodetable_index = slot;
        if ( (void*) volume2_rootdir_fp->inode == NULL ){
            panic("fsInit: volume2_rootdir_fp inode struct");
        }
        volume2_rootdir_fp->inode->filestruct_counter = 1;  //inicialize
        memcpy ( 
            (void*)       volume2_rootdir_fp->inode->path, 
            (const void*) volume2_rootdir_fp->_tmpfname, 
                  sizeof( volume2_rootdir_fp->inode->path ) );
        // ... 
    };


//
// == pipe_gramadocore_init_execve ================================ 
//

// ## Inicializando os pipes usados em execve ## 
// gramado core init execve 
// #todo: mudar para pipe_gramadocore_init_execve_fp
// aloca memoria para a estrutura.
    
    pipe_gramadocore_init_execve = 
        (file *) kmalloc( sizeof(file) );

    if ((void *) pipe_gramadocore_init_execve == NULL){
        panic ("fsInit: pipe_gramadocore_init_execve\n");
    }else{

        // Aloca memória para o buffer.
        // #todo: Define this variable in the top of the body.
        // #bugbug: Chech this size.
        unsigned long pipe0base = (unsigned long) kmalloc(512);
        if ((void *) pipe0base == NULL){
            panic ("fsInit: pipe0base\n");
        }

        pipe_gramadocore_init_execve->used = TRUE;
        pipe_gramadocore_init_execve->magic = 1234;

        pipe_gramadocore_init_execve->_base = (unsigned char *) pipe0base;
        pipe_gramadocore_init_execve->_p    = (unsigned char *) pipe0base;
        pipe_gramadocore_init_execve->_cnt  = 512;  // #bugbug: Check this size.
        pipe_gramadocore_init_execve->_file = 0;    // ??
        pipe_gramadocore_init_execve->_tmpfname = "pipe0";

        // #todo
        //fileList[ ? ] = (unsigned long) pipe_gramadocore_init_execve;

        // 0
        pipeList[0] = (unsigned long) pipe_gramadocore_init_execve;
    };


// Initialize directory facility structures.
    init_directory_facilities();

// CWD Structure.
// Inicializa o pwd support.
    fsInitializeWorkingDiretoryString();

// Target dir struct
// Inicializa a estrutura de suporte ao target dir.
    fsInitTargetDir(VOLUME1_ROOTDIR_ADDRESS,"/");

    return 0;
}

int init_directory_facilities(void)
{

// '/'
    directory_facility_RootDir.dir_address = VOLUME1_ROOTDIR_ADDRESS;
    directory_facility_RootDir.dir_name[0] = '/';
    directory_facility_RootDir.dir_name[1] = 0;
    directory_facility_RootDir.name_size = 1;
    directory_facility_RootDir.initialized = TRUE;
// EFI/
    directory_facility_EFIDir.dir_address = 0;
    directory_facility_EFIDir.dir_name[0] = 0;
    directory_facility_EFIDir.name_size = 0;
    directory_facility_EFIDir.initialized = FALSE;
// GRAMADO/
    directory_facility_GramadoDir.dir_address = 0;
    directory_facility_GramadoDir.dir_name[0] = 0;
    directory_facility_GramadoDir.name_size = 0;
    directory_facility_GramadoDir.initialized = FALSE;
// PROGRAMS/
    directory_facility_ProgramsDir.dir_address = 0;
    directory_facility_ProgramsDir.dir_name[0] = 0;
    directory_facility_ProgramsDir.name_size = 0;
    directory_facility_ProgramsDir.initialized = FALSE;
// UBASE/
    directory_facility_ubaseDir.dir_address = 0;
    directory_facility_ubaseDir.dir_name[0] = 0;
    directory_facility_ubaseDir.name_size = 0;
    directory_facility_ubaseDir.initialized = FALSE;
// USERS/
    directory_facility_usersDir.dir_address = 0;
    directory_facility_usersDir.dir_name[0] = 0;
    directory_facility_usersDir.name_size = 0;
    directory_facility_usersDir.initialized = FALSE;

    return 0;
}


// #todo
// Change the return type to 'int' and
// remove all the messages. Maybe.
// Create the 'root' filesystem structure.
void fs_init_structures (void)
{
    int Type=0;

    debug_print ("fs_init_structures: [TODO]\n");

//
// root
//

// struct
// Create the 'root' filesystem structure.
// Initialize the 'root' filesystem structure.
// "/"

    debug_print ("fs_init_structures: root\n");

    root = (void *) kmalloc( sizeof(struct filesystem_d) );
    if ( (void *) root == NULL ){
        panic("fs_init_structures: Couldn't create the root structure.\n");
    }
    root->objectType  = ObjectTypeFileSystem;
    root->objectClass = ObjectClassKernelObjects;
    root->used = TRUE;
    root->magic = 1234;
    root->name = (char *) ____root_name;

// #todo 
// #bugbug   volume_vfs  ??

// Se o volume do vfs ainda não foi criado 
// então não podemos prosseguir.

        //if ( (void *) volume_vfs == NULL )
        //{
        //    debug_print("fs_init_structures: [FAIL] volume_vfs not initialized");
        //    panic      ("fs_init_structures: [FAIL] volume_vfs not initialized");
        //}
        //volume_vfs->fs = root;
 

// Save is in the 'storage' low level structure.

    if ((void*) storage == NULL){
         panic("fs_init_structures: storage");
    }

    storage->fs = (void*) root;
    // ...

// Type
// #bugbug: 
// Em qual disco e volume pegamos o tipo de sistema de arquivos?

    debug_print ("fs_init_structures: Type\n");

// #bugbug
// Why do we have this information?
    Type = (int) get_filesystem_type();
    Type = (Type & 0xFFFF);
    if (Type <= 0){
        panic ("fs_init_structures: [PANIC] Type\n");
    }else{
        root->type = (int) Type;
    };

    switch (Type){

    case FS_TYPE_FAT16:

        // Disk stuff.
        // spc - Sectors per cluster.
        root->spc = (int) VOLUME1_SPC;
        //root->spc = (int) get_spc(); 

        // Rootdir, Fat and data area.
        // #bugbug: Specific for fat16.
        
        // root dir
        root->rootdir_address   = VOLUME1_ROOTDIR_ADDRESS;
        root->rootdir_first_lba = VOLUME1_ROOTDIR_LBA;
        //root->rootdir_last_lba = 0;  //#todo 
        root->rootdir_size_in_sectors = 0;  //#todo
        
        // fat1
        root->fat_address     = VOLUME1_FAT_ADDRESS;
        root->fat_first_lba   = VOLUME1_FAT_LBA;
        //root->fat_last_lba = 0;  //#todo
        root->fat_size_in_sectors = 0;  //#todo
        
        // fat2
        // ...
        
        // dataarea
        //filesystem->dataarea_address = ??;
        root->dataarea_first_lba    = VOLUME1_DATAAREA_LBA; 
        //root->dataarea_last_lba = 0;  //#todo
        //root->dataarea_size_in_sectors = 0;  //#todo
 
        //root->fs_first_lba = 0;  //#todo
        
        //#todo (dataarea + dataarea size)?
        //root->fs_last_lba = 0;   

        //(mbrsize + reserved + rootdirsize + fat1size + fat2size +dataareasize)
        //root->fs_size_in_sectors = 0;

        // Root dir.
            
        // Number of entries in the root dir.
        // #bugbug: Specific for fat16.
        root->dir_entries = FAT16_ROOT_ENTRIES;

        // Size of the entry in bytes.
        // #bugbug: Specific for fat16.
        root->entry_size = FAT16_ENTRY_SIZE;

        // ...
        break;

    case FS_TYPE_EXT2:
    default:
        panic ("fs_init_structures: [todo] default Type\n");
        break;
    };
}

// #todo: use int return.
void file_close (file *_file)
{
    debug_print("file_close: todo\n");

    if ( (void*) _file == NULL ){
        return;
    }
}


size_t file_get_len(file *_file)
{
    debug_print("file_get_len: todo\n");

    if ( (void*) _file == NULL ){
        return -1;
    }

    return (size_t) -1;
}


//OUT: inode structure.
struct inode_d *file_inode (file *f)
{
    if ( (void *)f==NULL ){
        return (struct inode_d *) 0;
        //return NULL;
    }

    return (struct inode_d *) f->inode;
}

//??
int file_truncate (file *_file, size_t len)
{
    debug_print("file_truncate: todo\n");

    if ( (void*) _file == NULL ){
        return -1;
    }
    if (len<0){
        return -1;
    }

    return -1;
}

// Check the signature in a elf header.
// OUT:
// 0 = OK. #todo: Isso poderia ser '1'.
// < 0 Means error.
// #todo
// We can do the same for some other types.
// Use TRUE or FALSE.

// #todo
// is address virtual or physical?
// Change this name to pa or va.

int fsCheckELFFile(unsigned long address)
{
    if (address == 0){
        return 0;
    }
    return (int) elfCheckSignature(address);
}

/*
 * fsInitializeWorkingDiretoryString:
 *     Atualiza a string do diretorio de trabalho.
 * Essa eh a string que sera mostrada antes do prompt.
 * 'pwd'> 
 */

void fsInitializeWorkingDiretoryString (void)
{

    // #bugbug
    // We have some issues with string sizes.
    // Buffer overflow!

    // vamos contar o tamanho da string que estamos construindo.
    int string_count = 0;
    int string_size = 0;
    struct volume_d  *v;
    char volume_string[8];   

    debug_print ("fsInitializeWorkingDiretoryString:\n");

// See: 
// fs.h

    CWD.initialized = FALSE;

// volume string 

    volume_string[0] = 'v';
    volume_string[1] = 'o';
    volume_string[2] = 'l';
    volume_string[3] = 'u';
    volume_string[4] = 'm';
    volume_string[5] = 'e';
    volume_string[6] = (char)( '1' + (char) current_volume - (char) 1 );
    volume_string[7] = '\0';

	//'root:'
	//  ## volume list ##
	//primeiro colocamos a string que indica a lista de volumes. 

    sprintf ( CWD.path, FS_ROOT_STRING ); 

	//'/'
	// ## separador ##
    strcat ( CWD.path, FS_PATHNAME_SEPARATOR );

//
// volume root dir 
//

// #todo
// Check overflow.

    if ( current_volume < 0 ){
        panic ("fsInitializeWorkingDiretoryString: current_volume\n");
    }

    v = (struct volume_d *) volumeList[current_volume];

    if ( (void *) v == NULL ){
        panic ("fsInitializeWorkingDiretoryString: v\n");
    }else{
        if ( v->used != TRUE || v->magic != 1234 ){
            panic ("fsInitializeWorkingDiretoryString: validation\n");
        }

        switch (v->id)
        {
			//case 0:
		        //global usada para string do nome do volume.
			//    current_volume_string = (char *) FS_VOLUME0_STRING;
			//    break;

			//case 1:
		        //global usada para string do nome do volume.
			//   current_volume_string = (char *) FS_VOLUME1_STRING;
			//   break;
			
            default:
		        //fail.
		        //printf("fsInitializeWorkingDiretoryString: default volume #todo\n");
		        //die();
                current_volume_string = (char *) volume_string; 
                break;
        };

        // #bugbug
        // We need to finalize the string.
        // limit 32.
        // See: volume.h
        
        // path string na estrutura do volume.
        
        string_size = sizeof(current_volume_string);

        if (string_size >= 32){
            debug_print ("fsInitializeWorkingDiretoryString: [FIXME] string size\n"); 
            return;
        }
        
        // copy the string. limit 32 bytes.
        sprintf ( v->path_string, current_volume_string );
        
        // finalize.
        v->path_string[31] = 0;

        // #bugbug
        // What is the limit for this string ? 32 bytes.
        // See: rtl/fs/path.h and globals.h

        strcat ( CWD.path, v->path_string );
	    //strcat ( CWD.path, current_volume_string );
	    
        CWD.path[31] = 0;
    };

// #bugbug
// What is the limit for this string ? 32 bytes.
// See: rtl/fs/path.h and globals.h
// Separador

    strcat ( 
        CWD.path, 
        FS_PATHNAME_SEPARATOR );

//
// Size
//

// #test

    CWD.path[31] = 0;

    int size=0;
    size = strlen(CWD.path);

    if (size > 31){
        size = 31;
    }

    CWD.size = size;

// More ?...

    //debug_print ("fsInitializeWorkingDiretoryString: done\n");

// See: 
// fs.h

    CWD.initialized = TRUE;
}

/*
 * fsInitTargetDir:
 *     Para inicializarmos o sistema já com um alvo.
 */
// #todo
// is dir_address virtual or physical?
// Change this name to dir_pa or dir_va.

// using the structure 'current_target_dir'.
// IN:
// directory address, directory name.

int fsInitTargetDir (unsigned long dir_address, char *dir_name)
{
    register int i=0;

    current_target_dir.used = TRUE;
    current_target_dir.magic = 1234;
    for ( i=0; i<11; i++ ){
        current_target_dir.name[i] = '\0';
    };

// Dir address
    if (dir_address == 0){
        panic("fsInitTargetDir: dir_address\n");
    }
    //current_target_dir.current_dir_address = VOLUME1_ROOTDIR_ADDRESS;
    current_target_dir.current_dir_address = 
        (unsigned long) dir_address;

// Dir name
    
    if ( (void*) dir_name == NULL ){
        panic("fsInitTargetDir: dir_name\n");
    }
    if (*dir_name == 0){
        panic("fsInitTargetDir: *dir_name\n");
    }
    //current_target_dir.name[0] = '/';
    //current_target_dir.name[1] = 0;

// Limits: 
// Copy 8 bytes only
    for ( i=0; i<8; i++ ){
        current_target_dir.name[i] = dir_name[i];
    };
// done:
    current_target_dir.initialized = TRUE;

    return 0;
}

// fsList
// Ring 0 routine to list files.
// #todo
// Use 'pathname'.
// #shortcut: '[' 

int fsList(const char *dir_name)
{
    int Absolute = FALSE;
    register int i=0;

    debug_print ("fsList:\n");

// Directory name

    if ( (void *) dir_name == NULL ){
        debug_print ("fsList: [FAIL] dir_name\n");
        goto fail;
    }
    if ( *dir_name == 0 ){
        debug_print ("fsList: [FAIL] *dir_name\n");
        goto fail;
    }

// Copy name
// Limits: 
// Copy 8 bytes only

    for ( i=0; i<8; i++ ){
        current_target_dir.name[i] = dir_name[i];
    };
    current_target_dir.name[i] = '\0';

// The root directory.
// #shortcut: '[' 
// #todo: Explain it.
// Open the dirname '/'.

// #test: Default in root.
    current_target_dir.current_dir_address = 
        VOLUME1_ROOTDIR_ADDRESS;
    current_target_dir.name[0] = '/';
    current_target_dir.name[1] = '\0'; 

    if ( dir_name[0] == '[' && 
         dir_name[1] == 0 )
    {
        //debug_print ("fsList: root\n");
        Absolute = TRUE;

        current_target_dir.current_dir_address = 
            VOLUME1_ROOTDIR_ADDRESS;
        
        // Clear the whole buffer.
        for ( i=0; i<11; i++ ){
            current_target_dir.name[i] = '\0';
        };
        // Create the name.
        current_target_dir.name[0] = '/';
        current_target_dir.name[1] = '\0'; 
    }


// #bugbug
// We are using the current directory address,
// not the directory provide by the user.
// #todo
// Set up the current dir address, based on the
// name provided by the user.

//
// == current_target_dir ====================
//

// directory address.
    if (current_target_dir.current_dir_address == 0){
        debug_print ("fsList: current_target_dir.current_dir_address\n");
        goto fail;
    }
    
    // #bugbug
    // Missing string finalization.
    //printk ("fsList: current_target_dir.name = {%s}\n", current_target_dir.name);

// Listing ...
// IN:
// name, dir address, number of entries;
// No return value.

    // Well, the root dir has 512 entries.
    int n=256;
    if ( current_target_dir.current_dir_address == 
         VOLUME1_ROOTDIR_ADDRESS )
    {
        n=512;
    }

    fsFAT16ListFiles ( 
        (const char *)     current_target_dir.name,
        (unsigned short *) current_target_dir.current_dir_address, 
        (int) n );

    //debug_print ("fsList: done\n");
    return 0;

fail:
    debug_print ("fsList: fail\n");
    //refresh_screen();
    return -1;
}

/*
 * fsListFiles:
 *     Lista os arquivos em um diretório, 
 * dados os índices de disco, 
 * volume e diretório.
 */
// #bugbug
// Do not list this in ring0.

void 
fsListFiles ( 
    int disk_id, 
    int volume_id, 
    int directory_id )
{
// #todo: Incomplete routine.

    // #todo: 
    // Checar mais limites.
    // Use this: if ( disk_id < 0 || volume_id < 0 || directory_id < 0 )

    if ( disk_id == -1 || volume_id == -1 || directory_id == -1 )
    {
        debug_print ("fsListFiles: [FAIL] parameters\n");
        goto fail;
    }

    printf ("fsListFiles: disk={%d} vol={%d} dir={%d}\n", 
        disk_id, volume_id, directory_id );

// Show!
// Se o diret�rio selecionado � o diret�rio raiz do VFS.

    if ( current_disk == 0 && current_volume == 0 && current_directory == 0 )
    {
        debug_print ("fsListFiles: [FIXME] current\n");
        //vfsListFiles ();
        goto done;
    }

// #todo: 
// Suportar outros diretórios.
// ...

    goto done;

fail:
    printf ("fail\n");
done:
    //refresh_screen ();
    return;
}

/*
 * fsUpdateWorkingDiretoryString:
 *     +Atualiza o pathname na estrutura do processo atual.
 *     +Atualiza o pathname na string global. 
 */ 
// Used by the service 175, cd command.

void fsUpdateWorkingDiretoryString ( char *string )
{
    struct process_d  *p;
    char *tmp;
    register int i=0; 
    int string_size = 0;

    debug_print ("fsUpdateWorkingDiretoryString:\n"); 

    tmp = string;
    string_size = sizeof(string);

    pid_t current_process = (pid_t) get_current_process();

// Initialized?
    if (CWD.initialized != TRUE)
    {
        debug_print ("fsUpdateWorkingDiretoryString: [FAIL] CWD not initialized\n"); 
        // #todo
        // Call the initialization routine.
        return;
    }

// string
    if ( (void *) string == NULL ){
        debug_print ("fsUpdateWorkingDiretoryString: string\n"); 
        return;  
    }
    if (*string == 0){
        debug_print ("fsUpdateWorkingDiretoryString: *string\n"); 
        return;  
    }
    if (string_size <= 0){
        debug_print ("fsUpdateWorkingDiretoryString: [FAIL] string_size\n"); 
        return;  
    }

// Current process.

    if (current_process < 0 || current_process >= PROCESS_COUNT_MAX){
        panic ("fsUpdateWorkingDiretoryString: current_process\n");
    }

    p = (struct process_d *) processList[current_process];

    if ( (void *) p == NULL ){
        panic ("fsUpdateWorkingDiretoryString: p\n");
    }else{
        if ( p->used != 1 || p->magic != 1234 ){
            panic ("fsUpdateWorkingDiretoryString: p validation\n");
        }

        // Atualiza a string do processo atual. 
        // Concatenando.
        
        if ( (void *) string != NULL )
        {
            // #bugbug
            // We need to handle the string size limit.
            
            // Concatena string.
            strcat ( p->cwd_string, string );

            // Concatena separador.
            strcat ( p->cwd_string, FS_PATHNAME_SEPARATOR );

            // Atualiza a string global usando a string do 
            // processo atual.
            // #bugbug: nao precisamos disso ...
            // so precismos de cwd na estrutura de processo.
            
            // #importante
            // Respeitar o limite.
            
            for ( i=0; i<32; i++ )
            {
                CWD.path[i] = p->cwd_string[i];
            };
            CWD.path[31] = 0; //finaliza

            // #bugbug: rever isso.
            // Nome do diretório alvo atual.
            
            // nao precismos disso ...
            // ou usamos o cwd do processo ou
            // o diretorio raiz para paths absolutos.
            
            for ( i=0; i< 11; i++ )
            {
                current_target_dir.name[i] = *tmp;
                tmp++;
            };
            current_target_dir.name[11] = 0; //finaliza
        }
    };

    //debug_print ("fsUpdateWorkingDiretoryString: done\n"); 
}

/*
 * fs_fntos:  
 *     (Filename to string).   
 *     'file name to string'.
 *     rotina interna de support.
 *     isso deve ir para bibliotecas depois.
 *     não tem protótipo ainda.
 * Created by: Luiz Felipe.
 * 2020 - Adapted by Fred Nora.
 */
 // #bugbug
 // Isso modifica a string lá em ring3.
 // prejudicando uma segunda chamada com a mesma string
 // pois já virá formatada.
// #bugbug
// const char * tornaria esse endereço em apenas leitura.
// Allowed in fat16: $ % ' - _ @ ~ ` ! ( ) { } ^ # &
void fs_fntos(char *name)
{
    int i=0;
    int ns = 0;
    char ext[4];

    // #todo
    int fAddNewExt = FALSE;

// No extension
// 
    ext[0] = 0;
    ext[1] = 0;
    ext[2] = 0;
    ext[3] = 0;

// Invalid parameter
    if ((void*) name == NULL){
        return;
    }
    if (*name == 0){
        return;
    }

// Transforma em maiúscula enquanto não achar um ponto.
// #bugbug: 
// E se a string já vier maiúscula teremos problemas.

// Transforma somente as letras e 
// somente ate encontrarmos o ponto.
    //int max = (8+1+3); 
    //int max = (8); 
    while ( *name && *name != '.' )
    {
        // Se a string eh muito grande.
        //if (ns >= max)
            //break;
        
        if ( *name >= 'a' && *name <= 'z' )
        {
            *name -= 0x20;
        }
        name++;
        ns++;
    };

// #bugbug
// Esse negócio de acrescentar a extensão
// não é bom para todos os casos.
// >> name[0] significa que na ultima posiçao do ponteiro *p,
//    encontramos o valor 0x00.
// >> Mas ainda somos menor ou igual a 8, que eh o campo
//    para nome de 8 bytes.

    if ( name[0] == '\0' && ns <= 8 )
    {
        // The flag says that 
        // we're gonna have a new extension.
        fAddNewExt = TRUE;
        // Complete the DOS name with spaces. Until 8 bytes.
        goto CompleteName8WithSpaces;
    }

    //if ( name[0] == '.' && ns < 8 )

// Aqui name[0] é o ponto. 
// Mas o nome tem 8 bytes.
// Então constrói a extensão colocando as letras na extensão.

    for ( i=0; 
          i<3 && name[i+1]; 
          i++ )
    {
        // #ps: name[0] = '.'

        //Transforma uma letra da extensão em maiúscula.

        // Letras na extensao: 
        // Mudando as letras para maiusculo.
        if ( name[i+1] >= 'a' && 
             name[i+1] <= 'z' )
        {
            name[i+1] -= 0x20;   // Change
            ext[i] = name[i+1];  // Save
        }

        // Bytes especiais na extensao: 
        // Colocando os bytes, sem mudar.
        // Allowed in fat16: $ % ' - _ @ ~ ` ! ( ) { } ^ # &
                
        if ( name[i+1] == '$' ||
             name[i+1] == '%' ||
             name[i+1] == '-' ||
             name[i+1] == '_' ||
             name[i+1] == '(' ||
             name[i+1] == ')' ||
             name[i+1] == '{' ||
             name[i+1] == '}'  )
        {
            ext[i] = name[i+1];  // Save
        }

        // Se a extensao tiver numeros.
        if  ( name[i+1] >= '0' &&
              name[i+1] <= '9' ) 
        {
            ext[i] = name[i+1];  // Save
        }
        
        // E se tiver um 0x00 entre os bytes da extensao?
        // Mas a extensao tinha um ponto.
        if ( name[i+1] == 0x00 )
        {
            fAddNewExt = FALSE;
            goto CompleteName8WithSpaces;
        }
    };

// Acrescentamos ' ' até completarmos as oito letras do nome.
// Acrescentamos a extensão
// Finalizamos.

CompleteName8WithSpaces:

// Save some spaces in the name until 8 bytes.
    while (ns < 8)
    {
        *name++ = ' ';
        ns++;
    };

// Add extension.
AddExt:

// Prepare a new extension if it is the case.
    if (fAddNewExt == TRUE)
    {
        // Pre-load the new extension bytes.
        ext[0] = 'B';  
        ext[1] = 'I';  
        ext[2] = 'N';  
        ext[3] = '\0';
    }

// Save extension bytes.
    for (i=0; i<3; i++)
    {
        // No caso de extensoes com 3 bytes
        // copiamos todos os bytes.
        if ( ext[i] != 0x00 )
        {
            *name++ = ext[i];  
        }
        
        // No caso das extensoes com menos bytes,
        // o buffer pode ter 0x00.
        // No lugar no 0x00 colocaremos ' '.
        if ( ext[i] == 0x00 )
        {
            *name++ = ' ';  
        }
    };

    *name = '\0';
}

// Pega um fd na lista de arquivos do processo, dado o PID.
// Objects[i]
int fs_get_free_fd_from_pid (pid_t pid)
{
    register int __slot=0;
    struct process_d *p;

    if ( pid<0 || pid >= PROCESS_COUNT_MAX ){
        debug_print ("fs_get_free_fd_from_pid: [FAIL] pid\n");
        return -1;
    }

    p = (struct process_d *) processList[pid];
    if ( (void *) p == NULL ){
        debug_print ("fs_get_free_fd_from_pid: p\n");
        return -1;
    }
    if ( p->used != TRUE || p->magic != 1234 ){
        debug_print ("fs_get_free_fd_from_pid: p validation\n");
        return -1;
    }

// Pick a free one.
// and return the index.
    for (__slot=0; __slot<32; __slot++)
    {
         if ( p->Objects[__slot] == 0 ){ return (int) __slot; }
    };

// fail
    return -1;
}


/*
 * fs_initialize_process_cwd:
 *     Cada processo deve inicialiar seus dados aqui. 
 */
// #todo:
// handle return value ...
// What functions is calling us?

int fs_initialize_process_cwd ( pid_t pid, char *string )
{
    struct process_d *p;
    register int i=0;

    if (pid<0 || pid >= PROCESS_COUNT_MAX){
        debug_print ("fs_initialize_process_cwd: pid\n");
        return 1;
    }

// string

    if ( (void *) string == NULL ){
        panic ("fs_initialize_process_cwd: string\n");
        //return 1;
    }
    if (*string == 0){
        panic ("fs_initialize_process_cwd: *string\n");
        //return 1;
    }

// Current process.
// #importante
// Vamos copiar a string para a estrutura do processo atual.

    p = (struct process_d *) processList[pid];

    if ( (void *) p == NULL ){
        panic ("fs_initialize_process_cwd: p\n");
    }else{
        if ( p->used != 1 || p->magic != 1234 ){
            panic ("fs_initialize_process_cwd: validation\n");
        }

        // ?? fixed size.
        for ( i=0; i<32; i++ ){ p->cwd_string[i] = string[i]; }
        
        p->cwd_string[31] = 0; // finalizing 
    };

    return 0;
}


/* 
 * fs_pathname_backup:
 *     Remove n nomes de diret�rio do pathname do processo indicado no 
 * argumento.
 *     Copia o nome para a string global.
 *     Remove the last N directories from PATH.  
 *     Do not leave a blank path.
 *     PATH must contain enough space for MAXPATHLEN characters. 
 *     #obs: O PID costuma ser do processo atual mesmo. 
 *     Credits: bash 1.05 
 */
// #todo: Describe 'n'.

void fs_pathname_backup ( pid_t pid, int n )
{
    struct process_d *p;
    register int i=0;

// CWD
    if ( CWD.initialized != TRUE ){
        printf ("fs_pathname_backup: [FAIL] CWD not initialized\n"); 
        return;
    } 

// pid
    if ( pid<0 || pid >= PROCESS_COUNT_MAX ){
        printf ("fs_pathname_backup: [FAIL] pid\n"); 
        return;
    }

// n
    if (n <= 0) {
        return;
    }

// Process

    p = (struct process_d *) processList[pid];

    if ( (void *) p == NULL ){
        panic ("fsUpdateWorkingDiretoryString: p\n");
    }else{
        if ( p->used != TRUE || p->magic != 1234 ){
            panic ("fsUpdateWorkingDiretoryString: validation\n");
        }

        char *path = (char *) p->cwd_string;

        register char *s = path + strlen( path );
 
        if (*path){ s--; };

        while (n--)
        {
            while (*s == '/'){ s--; };
            while (*s != '/'){ s--; };

            *++s = '\0';
        };

        // Atualizando a string global.
        for ( i=0; i<32; i++ ){
            CWD.path[i] = p->cwd_string[i];
        };

        // Name.
        for ( i=0; i< 11; i++ ){
            current_target_dir.name[i] = '\0';
        };
    };
}


/*
 * fs_print_process_cwd
 *     Cada processo tem seu proprio pwd.
 *     Essa rotina mostra o pathname usado pelo processo. 
 */
// this is used by the pwd command. service 170.

int fs_print_process_cwd(pid_t pid)
{
    struct process_d *p;

    debug_print ("fs_print_process_cwd:\n");
    printf      ("fs_print_process_cwd:\n");

// pid
    if (pid<0 || pid>=PROCESS_COUNT_MAX){
        debug_print ("fs_print_process_cwd: [FAIL] pid\n");
        return -1;
    }

// Process
    p = (struct process_d *) processList[pid];

    if ( (void *) p == NULL ){
        panic ("fs_print_process_cwd: p\n");
    }else{
        if ( p->used != 1 || p->magic != 1234 ){
            panic ("fs_print_process_cwd: validation\n");
        }

        // #bugbug
        // Is this element a pointer or a buffer ?
        // >>> This element is an array.
        
        if ( (void *) p->cwd_string != NULL )
        {
            //p->cwd_string[31] = 0;
            printf ("> PID=%d p->cwd_string {%s} \n", 
                p->pid, p->cwd_string);
        }

        // #bugbug
        // Is this element a pointer or a buffer ?

        if ( (void *) current_target_dir.name != NULL )
        {
            //current_target_dir.name[31] = 0;
            printf ("> PID=%d current_target_dir.name {%s} \n", 
                p->pid, current_target_dir.name);
        }

        //refresh_screen();
        return 0;
    };

// fail
    debug_print ("fs_print_process_cwd: fail\n");   
    return -1;
}


void fs_show_file_info (file *f)
{
    if ((void*)f==NULL){
        debug_print("fs_show_file_info: fail\n");
        return;
    }

    if (f->used == TRUE)
    {
        if ( (void*) f->_tmpfname != NULL )
        {
            printf ("Name={%s}\n",f->_tmpfname);
            //refresh_screen();
        }
    }
}

void fs_show_file_table(void)
{
    file *f;
    register int i=0;

    printf ("\nfile_table:\n");
    
    for (i=0; i<32; i++)
    {
        f = (file*) file_table[i];
        if ( (void*)f != NULL ){
            fs_show_file_info(f);
        }
    };

    //refresh_screen();
}

void fs_show_inode_info (struct inode_d *i)
{
    if ( (void *) i == NULL ){
        debug_print("fs_show_inode_info: fail\n");
        return;
    }

    if (i->used == TRUE)
    {
        if ( (void*)i->path != NULL )
        {
            printf ("Name={%s}\n",i->path);
        }
    }  
}

void fs_show_inode_table(void)
{
    struct inode_d *inode;
    register int i=0;

    printf ("\n inode_table: \n");
    
    for (i=0; i<32; ++i)
    {
        inode = (struct inode_d *) inode_table[i];
        
        if ( (void*)inode != NULL )
        {
            if( inode->used == 1 && inode->magic == 1234 )
            {
                fs_show_inode_info(inode);
            }
        }
    };

    //refresh_screen();
}

void fs_show_root_fs_info(void)
{
    //printf ("\n");
    printf ("fs_show_root_fs_info:\n");

// root fs structure.

    if ( (void *) root == NULL ){
        printf ("No root structure\n");
        goto fail;
    
    }else{

        if ( root->used != 1 || root->magic != 1234 ){
             printf ("Validation fail\n");
             goto fail;
        }

        printf ("name = %s \n",       root->name );
        printf ("Object type %d \n",  root->objectType );
        printf ("Object class %d \n", root->objectClass );
        printf ("type = %d \n",       root->type );
        printf ("Dir entries %d \n",  root->dir_entries );
        printf ("Entry size %d \n",   root->entry_size );
        // ...
        goto done;
    }; 

fail:
    printf("fail\n");
done:
    //refresh_screen();
    return;
}

// sys_pwd -  Service 170.
void sys_pwd(void)
{
    pid_t current_process = (pid_t) get_current_process();
    if ( current_process < 0 || current_process >= PROCESS_COUNT_MAX ){
        panic ("sys_pwd: [FAIL] current_process\n");
    }
    fs_print_process_cwd (current_process);
}

/*
 * fsSaveFile:
 *     Salva um arquivo no disco.
 *     Somente no diretório raiz.
 */
// It was called by sys_write_file() in fs.c.
// It was called by sys_read_file when the file does not exist.
// #obs
// Isso salva um arquivo.
// Também poderia ser usado para criar um arquivo ou diretório ? 
// #todo: #test
// Número máximo de entradas na lista de clusters. 
// Quantas?
// A FAT tem 246 setores, 123 KB
// fsSaveFile:
//     Salva um arquivo. 
//     Onde? #todo vamos fornecer o endereço do diretorio.
// IN: 
// name, size in sectors, size in bytes, adress, flag. 
// OUT:
// #bugbug
// O nome nao esta ficando certo na entrada.
// #todo
// is dir_address virtual or physical?
// Change this name to dir_pa or dir_va.

int
fsSaveFile ( 
    unsigned long fat_address,
    unsigned long dir_address,
    int dir_entries,
    const char *file_name, 
    unsigned long file_size,
    unsigned long size_in_bytes,
    char *file_address,
    char flag )  
{
// #todo
// For now we are using a local buffer to handle
// the target directory entry.
// Use the entry structure (fat16_directory_entry_d) found in fat.h.
// >>> Used to save/create a file or a directory,
// depending on the flag.

    int Status = 0;

    unsigned long i = 0; 
    unsigned long j = 0;    // Deslocamento na lista, tem que ser zero.
    unsigned long c = 0;    // Deslocamento na FAT.

    unsigned short first=0;
    unsigned short next=0;
    unsigned short sector=0;

// #todo: Use the structure.
// Entry structure.
// see: fat.h
    //struct fat16_directory_entry_d entry_fat16;
// Directory entry buffer.
    char DirEntry[32];

//Entry size in words.
    int EntrySize = 0;
    int Offset = 0;
    int FreeIndex = -1;

    // #debug:
    debug_print ("fsSaveFile:\n");
    //printf      ("fsSaveFile:\n");

// Updating fat address and dir address.

    if (fat_address == 0){
        panic("fsSaveFile: fat_address\n");
    }
    if (dir_address == 0){
        panic("fsSaveFile: dir_address\n");
    }

// VOLUME1_FAT_ADDRESS
    unsigned short *fat = 
        (unsigned short *) fat_address;
// VOLUME1_ROOTDIR_ADDRESS
    unsigned short *__dir = 
        (unsigned short *) dir_address;

// #debug
// #todo
// We only support one address for now.

    if (fat_address != VOLUME1_FAT_ADDRESS){
        panic("fsSaveFile: [FIXME] We only support ONE fat address for now!\n");
    }

// Filename

    if ((void *) file_name == NULL){
        debug_print("fsSaveFile: [ERROR] file_name\n");
        printf     ("fsSaveFile: [ERROR] file_name\n"); 
        goto fail;
    }
    if (*file_name == 0){
        debug_print("fsSaveFile: [ERROR] *file_name\n");
        printf     ("fsSaveFile: [ERROR] *file_name\n"); 
        goto fail;
    }

// #bugbug
// Esse endereço eh valido?

/*
    printf ("name address = %x \n", &file_name );
    printf ("name    = %s \n", file_name ); 
    printf ("size    = %d \n", file_size );       // Size in sectors.
    printf ("nbytes  = %d \n", size_in_bytes );
    printf ("address = %x \n", file_address );
    printf ("flag    = %x \n", flag );
    printf ("entries = %d \n", dir_entries );
 */

// How name direntries this directory have?
    if ( dir_entries == 0 || 
         dir_entries > FAT16_ROOT_ENTRIES )
    {
        panic ("fsSaveFile: [FAIL] max dir entries");
    }

// file_size
// #todo: 
// precisamos implementar um limite para o tamanho do arquivo,
// principamente nessa fase de teste.

// #bugbug
// Limite provisorio
// Size in sectors.
    if (file_size > 16)
    {
        debug_print("fsSaveFile: [FIXME] Size in sectors\n");
        printf     ("fsSaveFile: [FIXME] Size in sectors = %d \n", 
            file_size ); 
        goto fail;
    }

// Load root dir and FAT.
// see: dev/blkdev/disk_r.c
    fs_load_rootdir( VOLUME1_ROOTDIR_ADDRESS, VOLUME1_ROOTDIR_LBA, 32 );
    fs_load_fat( VOLUME1_FAT_ADDRESS, VOLUME1_FAT_LBA, 246 );

// Procurando cluster livre na fat.
// Nesse momento construimos uma lista de clusters livres.
// #todo: 
// Essa lista já devia existir e agora somente 
// usaríamos.
// #todo: Essa rotina poderia seruma helper function?

//SearchEmptyEntries:
 
// #bugbug
// Obs: Esse limite é improvisado.

    while (i < CLUSTERS_TO_SAVE_MAX)
    {
        //procurando cluster livre na fat.
        //@todo:isso poderia ser uma função.

        if (fat[c] == 0)
        {
            // Encontrado todos os espaços livres 
            // que o arquivo precisa.
            // Marca o fim.
            // #importante: 
            // Se der certo, saímos do loop.
            // #bugbug: Esse size deve ter um limite.
            // file_size = file size in sectors, (clusters??)
            if (file_size == 0){
                fat16ClustersToSave[j] = (unsigned short) 0xfff8; 
                goto save_file;
            }

            //salva um endereço livre
            //salvamos um índice na fat dentro da lista
            //incrementa a lista

            fat16ClustersToSave[j] = (unsigned short) c; 
            j++; 

            // Decrementa o tamanho do arquivo!
            file_size--; 
        };

        c++;    // Incrementa o deslocamento na fat. 
        i++;    // Incrementa a quantidade de busca.
    }; 

// Fail
// Nossa busca por clusters livres dentro da fat não deu certo.
// Provavelmente não encontramos uma quantidade suficiente.

out_of_range:  

    printf ("fsSaveFile: No free cluster \n");
    goto fail;

    // #debug
    //refresh_screen();
    //while(1){ asm("hlt"); }

// #importante:
// Deu certo. Encontramos na fat todos os clusters que o arquivo precisa.   
// Salva o arquivo.
// O arquivo tem na lista todos os clusters que ele precisa.

// Save!

save_file:

    //#debug
    //printf("fsSaveFile: save_file: \n"); 
    //refresh_screen();

// Início da lista.
    i = 0; 

// Size limits.

// #bugbug
// Limite máximo improvisado.
// 2KB.

    j = (512*4);

// Pegamos o primeiro da lista.
    first = fat16ClustersToSave[i];

    // #debug
    // printf("first={%x}\n",first);

// Create directory entry
// Name/ext 8.3
    DirEntry[0]  = (char) file_name[0];
    DirEntry[1]  = (char) file_name[1];
    DirEntry[2]  = (char) file_name[2];
    DirEntry[3]  = (char) file_name[3];
    DirEntry[4]  = (char) file_name[4];
    DirEntry[5]  = (char) file_name[5];
    DirEntry[6]  = (char) file_name[6];
    DirEntry[7]  = (char) file_name[7];
// extension.
    // Use extension or files.
    DirEntry[8]  = (char) file_name[8];
    DirEntry[9]  = (char) file_name[9];
    DirEntry[10] = (char) file_name[10];
    // No extension for directories. Using spaces. I guess. 
    if (flag == 0x10)
    {
        DirEntry[8]  = (char) 0x20;
        DirEntry[9]  = (char) 0x20;
        DirEntry[10] = (char) 0x20;
    }

    /*
    printf ("flag=%x \n",flag);
    printf ("%c\n",DirEntry[8]);
    printf ("%c\n",DirEntry[9]);
    printf ("%c\n",DirEntry[10]);
    */


// Attribute byte.
// 0x01: read only
// 0x02: hidden
// 0x04: system
// 0x08: volume label
// 0x10: >>>> Directory <<<<
// 0x20: >>>> Archive <<<<
// 0x40: Unused
// 0x80: Unused

    DirEntry[11] = (char) flag; 

// Reserved
    DirEntry[12] = 0; 
// Creation time. 14 15 16
    DirEntry[13] = 0x08;  // Create Time (ms)
    DirEntry[14] = 0x08;  // Create Time (Hrs/Mins/Secs)
    DirEntry[15] = 0xb6;
// Creation date
    DirEntry[16] = 0xb6;
    DirEntry[17] = 0x4c;
// Access date
    DirEntry[18] = 0xb8;
    DirEntry[19] = 0x4c;

// First cluster. (16 bits)
// Only used in FAT32 Systems
// 0 para fat12 ou 16
    DirEntry[20] = 0;  // File/Folder Start Cluster (High)
    DirEntry[21] = 0;

// Modifield time
    DirEntry[22] = 0xa8;
    DirEntry[23] = 0x49;
// Modifield date
    DirEntry[24] = 0xb8;
    DirEntry[25] = 0x4c;

// First cluster. (Low word)
    DirEntry[26] = (char) (first);  // File/Folder Start Cluster (Low)
    DirEntry[27] = (char) (first >> 8);

// File size in bytes.  (32 bits)
// 4 bytes: (28,29,30,31)
    DirEntry[28] = (char) size_in_bytes;
    size_in_bytes = (size_in_bytes >> 8);
    DirEntry[29] = (char) size_in_bytes;
    size_in_bytes = (size_in_bytes >> 8);
    DirEntry[30] = (char) size_in_bytes;
    size_in_bytes = (size_in_bytes >> 8);
    DirEntry[31] = (char) size_in_bytes;
// Folders will have a File Size of 0x0000
    if (flag == 0x10)
    {
        DirEntry[28] = (char) 0;
        DirEntry[29] = (char) 0;
        DirEntry[30] = (char) 0;
        DirEntry[31] = (char) 0;
    }



// #importante:
// Vamos encontrar uma entrada livre no diretório para
// salvarmos o nome do arquivo.
// Copia o nome para dentro da entrada do diretório.
// Obs: As entradas são de 32 bytes. Como root[] é um 
// array de short então faremos um deslocamento de 16 shorts.
// root[]
// #importante: root[] é um array de short.	
// IN: 
// Endereço do diretótio e número máximo de entradas.
// #todo: 
// Talvez possamos ampliar esse número para o máximo 
// de entradas no diretório.
// #bugbug: A quantidade de entrada depende to diretório.
// See: search.c

// IN: 
// directory address, max number of entries.
    
    FreeIndex = 
        (int) findEmptyDirectoryEntry ( 
                  dir_address, 
                  dir_entries );

    if ( FreeIndex == -1 ){
        printf ("fsSaveFile: [FAIL] No empty entry\n");
        goto fail;
    }


//
// Write entry into the directory buffer.
//

// 32/2 = 16 words.
// Offset:
// Deslocamento dentro do diretório.
// representa o início da entrada que encontramos.
// Encontramos multiplicando o índice da entrada pelo 
// tamanho da entrada.
// Copy entry into the root in the memory.
// Copia 32 bytes.
 
    EntrySize = (FAT16_ENTRY_SIZE/2);
    Offset = (int) ( FreeIndex * EntrySize );

// FAT16_DIRENTRY_SIZE = 32
    memcpy ( &__dir[Offset], DirEntry, 32 );

// reset
// Reiniciamos o controlador antes de usarmos.

//resetIDE:

    //#debug
    //printf("fsSaveFile: reset and while\n"); 
    //refresh_screen();

    //reset_ide0 ();

// ## Save ##
// Vamos ao salvamento propriamente dito.

    i=0; 

//#debug 
//improvisando um endereço válido

    unsigned long address = (unsigned long) file_address;

// Save!  
// Saving the file into the disk.
// Cluster by cluster.

//SavingFile:

// Routine:
// +Pega um conteúdo da lista.
// +Encontrada a assinatura na lista!
// ...

// #bugbug
// E se o primeiro for um marcador de fim de arquivo?
    //next = list[0];
    //if (next == 0xFFF8)
        //what??

// Loop
// Pegamos o atual na lista.
// Se ele eh o sinalizador de fim de lista, 
// entao entao colocamos o sinalizador de fim de arquivo
// no offset indicado pelo penultimo elemento da lista.

    while (TRUE){

        next = fat16ClustersToSave[i];

        // #debug
        printf ("fsSaveFile: [DEBUG] next={%x}\n", next);

        // O next é o marcador de fim de lista.
        if (next == 0xFFF8){

            // Então pegamos o penúltimo e colocamos
            // o finalizador lá nesse offset.
            next = fat16ClustersToSave[i-1];
            fat[next] = 0xFFF8;
            goto do_save_dir_and_fat;

        // Se não é assinatura ainda.
        }else{

            // #bugbug
            // Estamos usando isso sem nem mesmo checar seu valor.
            // precisamos de limtes aqui. Para evitarmos overflow.
            
            // Grava na fat o endereço do próximo cluster
            fat[next] = fat16ClustersToSave[i+1];
 
            //#debug 
            //printf("write_lba\n");
            //refresh_screen();
            
            // Wait and write!
            
            // ata_get_current_ide_port_index()
            disk_ata_wait_irq(__IDE_PORT);

            //grava - Aqui next esta certo!
            //write_lba ( (unsigned long) address, VOLUME1_DATAAREA_LBA + next -2 );
            ataWriteSector ( 
                (unsigned long) address, 
                (unsigned long) ( VOLUME1_DATAAREA_LBA + next -2), 
                0, 
                0 );

            address += 512;
        }; 

        //Próximo valor da lista.
        i++;

        // #bugbug
        // Limitando o tamanho do arquivo a 16 entradas.
        // Why??

        // #bugbug
        // Limite provisorio.
        if (i > 16){
            debug_print ("fsSaveFile: [FIXME] write sectors limit\n");
            printf      ("fsSaveFile: [FIXME] write sectors limit\n");
            goto fail;
        }

        // ??
    };

// FAIL
    debug_print ("fsSaveFile: Loop fail\n");
    goto fail;
   
//
// == done ========================================
//

// Saving rood dir and FAT.
// Nesse momento já salvamos os clusters do arquivo.
// OK. Funcionou no qemu.
// #bugbug
// Não vamos mais salvar a fat toda vez que salvarmos
// um arquivo.
// Vamos salvar a FAT apenas no fim da sessão.
// Como ainda não temos shutdown, então vamos salvar 
// quando chamarmos reboot.
// #important
// Updating the cache state.

do_save_dir_and_fat:

    debug_print ("fsSaveFile: [DEBUG] do_save_dir_and_fat\n");
    
// Save root
// #bugbug: We need to save a directory, not the root.
// IN: root dir address, root dir lba, root dir size in sectors.

    //if ( dir_address == ROO...
    fs_save_rootdir( VOLUME1_ROOTDIR_ADDRESS, VOLUME1_ROOTDIR_LBA, 32 );

// Sinalizando que o cache de fat precisa ser salvo.
    fs_fat16_cache_not_saved();

    // #debug
    //debug_print ("fsSaveFile: done\n");
    //printf      ("fsSaveFile: done\n");
    //refresh_screen();

    return 0;

fail:
    debug_print("fsSaveFile: Fail\n");
    printf     ("fsSaveFile: Fail\n");
    //refresh_screen ();
    return (int) 1;  // Why 1?
    //return -1;
}



/*
 * do_read_file_from_disk: 
 *     This is called by sys_open() and sys_read_file_from_disk().
 */
// Usada por open().
// Tem que retornar o fd e colocar o ponteiro na lista de arquivos abertos.
// Carrega um arquivo do disco para a memoria.
// #bugbug
// Na minha maquina real, as vezes da problema no tamanho do arquivo.
// #bugbug
// Precisamos colocar os arquivos também na 
// lista global de arquivos abertos. file_table[]
// E na lista de inodes. inode_table[]
// #bugbug
// Nao seria o read() usado para ler um arquivo ja aberto?
// Sim, a rotina de suporte para read esta em sys_read e 
// nao chama esse worker aqui.
// #todo:
// Essa aqui poderia ter outro nome, pois ela carrega um arquivo
// poderia chamar-se loadxxxx().
// See: fs.c
// IN:
//   + #todo
// Worker
int 
do_read_file_from_disk ( 
    char *file_name, 
    int flags, 
    mode_t mode )
{
    int __ret = -1;
    int Status = -1;

// File pointer.
    file *fp;

    size_t FileSize = -1;
    struct process_d *p;
    int __slot = -1;  // o fd.
    void *buff;

// For now we're just able to get files and info
// in the root dir.
// Default: Root dir.
    unsigned long TargetDirAddress = VOLUME1_ROOTDIR_ADDRESS;
    unsigned long NumberOfEntries = FAT16_ROOT_ENTRIES;

    // debug_print ("do_read_file_from_disk: $\n");

// filename
    if ((void*) file_name == NULL){
        return (int) (-EINVAL);
    }
    if (*file_name == 0){
        return (int) (-EINVAL);
    }


// Root dir?
    if (*file_name == '/')
    {
        TargetDirAddress = VOLUME1_ROOTDIR_ADDRESS;
        NumberOfEntries = FAT16_ROOT_ENTRIES;
        file_name++;
    }

//
// Namespace?
//

// -------------------------------------------
// #test
// Is it inside the PROGRAMS/ folder?
// This is a kind of namespace, but this is not 
// a new root directory for the whole system,
// it is only the directory where we're gonna serch
// for the the desired file.
// Remember: 
// We can setup the memory address for this directory 
// in the structure sdPROGRAMS.

    if (*file_name == '#')
    {
        if (sdPROGRAMS.initialized != TRUE){
            printf("do_read_file_from_disk: sdPROGRAMS.initialized\n");
            goto fail;
        }
        if (sdPROGRAMS.address == 0){
            printf("do_read_file_from_disk: sdPROGRAMS.address\n");
            goto fail;
        }
        TargetDirAddress = sdPROGRAMS.address;
        NumberOfEntries = FAT16_ROOT_ENTRIES;
        file_name++;
    }
// -------------------------------------------

// Convertendo o formato do nome do arquivo.    
// >>> "12345678XYZ"
// #todo: 
// Não fazer isso em ring3.
    fs_fntos((char *) file_name);

// #debug

    //printf ("do_read_file_from_disk: FILE={%s}\n",
    //    file_name);
    //refresh_screen();

// #bugbug
// We need to search in the inode list. inode_table[]
// If the file is found in the inode list, so we don't
// need to load it again, just increment the counter.

    // #todo
    // fs_search_inode_table(file_name);

// Searching for the file only on the root dir.
// Quando não existe, tentamos criar.
// #bugbug: Então 'cat' não deve chamar essa função.

    Status = 
        (int) search_in_dir( file_name, TargetDirAddress );

// Found.
    if (Status == TRUE){
        goto __go;
    }

// Not found.
// Let's create it if the flags tell us to do that.
    if (Status != TRUE)
    {
         //#debug
        printf ("do_read_file_from_disk: [FIXME] File not found!\n");
        //refresh_screen();

        // Create a new one.
        // #todo: Use sys_create_empty_file.
        if (flags & O_CREAT)
        {
            debug_print ("do_read_file_from_disk: [O_CREAT] Creating a new file\n"); 

            // #todo:
            // Define the default value for this case.
            buff = (void*) kmalloc(BUFSIZ);
            if ((void*) buff == NULL)
            {
                printf("do_read_file_from_disk: buff\n");
                goto fail;
            }
            memset(buff,0,BUFSIZ);
            sprintf(buff,"This is a new file.");

            //++
            // See: 
            __ret = 
                (int) fsSaveFile ( 
                          VOLUME1_FAT_ADDRESS, 
                           TargetDirAddress, 
                           NumberOfEntries, 
                           (char *) file_name, 
                           (unsigned long)  1,    // 1, size in sectors 
                           (unsigned long) 512,  // 512, size in bytes  
                           (char *) buff,         // buffer ?
                           (char) 0x20 );         // flag 
              //--

              // Ok
              if (__ret == 0){
                  debug_print("do_read_file_from_disk: Created new file\n");
                  //refresh_screen();
                  goto __go;
              }
         }

         printf("do_read_file_from_disk: [FIXME] Can't create new file\n");
         goto fail;
    }

__go:

// Process
    p = (struct process_d *) get_current_process_pointer();
    if ( (void *) p == NULL ){
        printf("do_read_file_from_disk: p\n");
        goto fail;
    }
    if ( p->used != TRUE || p->magic != 1234 ){
        printf("do_read_file_from_disk: p validation\n");
        goto fail;
    }

// Procurando um slot livre.
    for (__slot=0; __slot<32; __slot++)
    {
         if ( p->Objects[__slot] == 0 ){ goto __OK; }
    };

// fail
    //panic ("do_read_file_from_disk: No slots!\n");
    printf("do_read_file_from_disk: No slots!\n");
    goto fail;

// Slot found.
__OK:

    if ( __slot < 0 || __slot >= 32 ){
        printf ("do_read_file_from_disk: Slot fail\n");
        goto fail;
    }

// File struct
    fp = (file *) kmalloc( sizeof(file) );
    if ( (void *) fp == NULL ){
        printf ("do_read_file_from_disk: fp\n");
        goto fail;
    }

// Initialize.
    fp->used = TRUE;
    fp->magic = 1234;
    fp->pid = (pid_t) p->pid;  //current_process;
    fp->uid = (uid_t) current_user;
    fp->gid = (gid_t) current_group;

// #bugbug [FIXME]
// We need a type in read().

// #bugbug
// This function was called by sys_open, and open
// is able to open any kind of file.
// Why are we using this type here?

    fp->____object = ObjectTypeFile;

// ==================
// #todo #bubug
// Permissoes:
// As permissoes dependem do tipo de arquivo.

// #bugbug: Let's do this for normal files for now.
    fp->sync.can_read  = TRUE;
    fp->sync.can_write = TRUE;

    fp->sync.action = ACTION_NULL;
        // ==================

// #todo:
// We need to get the name in the inode.

    //fp->_tmpfname = NULL;

    fp->_fsize = 0;  // Isso é configurado logo abaixo.
    fp->_lbfsize = BUFSIZ;

// Inicializando apenas.
// #bugbug: Isso é provisório. 
// Caso contrário teremos problemas pra ler.
    fp->_r = 0;
    fp->_w = 0;
    fp->_cnt = BUFSIZ;  // Anda temos bastante espaço. todo o buffer

// #
// This is gonna be the return value.
    fp->_file = __slot;

    fp->fd_counter = 1;  //inicializando. 

// #todo
// Se ele não foi encontrado na lista de inodes
// e tivemos que carrega-lo do disco, então
// precisamos colocar ele na lista de inodes.. inode_table[]
// Atenção: O arquivo será carregado logo abaixo.

// #todo
// atualizar a tabela  global de arquivos. file_table[]

//
// buffer
//

// buffer padrão
// #bugbug: open chama isso. E se o arquivo for maior que o buffer ?
// open() precisa alocar outro buffer.

    fp->_base = (char *) kmalloc(BUFSIZ);
    if ( (void *) fp->_base == NULL ){
        printf ("do_read_file_from_disk: fp->_base\n");
        goto fail;
    }
    //#test provisório
    fp->_lbfsize = BUFSIZ;

// #debug
    //printf ("FILE_AGAIN={%s}\n",file_name);
    //refresh_screen();

// File size.
// #bugbug: 
// OUT: 'unsigned long' ?

    FileSize = 
        (size_t) fsGetFileSize( 
                      (unsigned char *) file_name,
                      (unsigned long) TargetDirAddress );

    if (FileSize <= 0){
        printf ("do_read_file_from_disk: FileSize\n");
        goto fail;
    }
// Structure field for file size.
    fp->_fsize = (int) FileSize;

// #bugbug
// #importante

// Limits.
    //if ( FileSize < fp->_lbfsize ){ 
    //    FileSize = fp->_lbfsize; 
    //}

// Limits.
// Se o arquivo for maior que buffer disponivel.
// Podemos aumentar o buffer.

    if (FileSize >= fp->_lbfsize)
    {
        // #debug
        printf("do_read_file_from_disk: [todo] File size out of limits\n");
        //printf("Size {%d}\n",FileSize);
        //goto fail;

        // #bugbug: Provisório.
        // Limite - 1MB.
        //if (FileSize > 1024*1024)
        if (FileSize > 8*1024)  //8KB
        {
            printf ("do_read_file_from_disk: File size out of limits\n");
            printf ("%d bytes \n",FileSize);
            //refresh_screen();
            return (-1);
        }
        
        // Allocate new buffer.
        // The buffer must to be bigger than the file size.
        size_t buflen = FileSize+8;
        fp->_base = (char *) kmalloc(buflen);
        
        if ( (void *) fp->_base == NULL ){
            printf ("do_read_file_from_disk: Couldn't create a new buffer\n");
            //refresh_screen();
            return -1;             
        }
 
        // Temos um novo buffer size.
        fp->_lbfsize = (int) buflen;
    }

// #paranoia.
// Checando os limites novamente.
// #bugbug: Provisório.
// Limits - 1MB
    //if (FileSize > 1024*1024)
    //{
    //    printf ("do_read_file_from_disk: File size out of limits\n");
    //    refresh_screen();
    //    return -1;
    //}

// #paranoia.
// Checando base novamente.

    if ( (void *) fp->_base == NULL ){
        printf("do_read_file_from_disk: fp->_base (again)\n");
        goto fail;
    }

//
// #todo (Unix-like)
// 

// What is the unix-like standard for this initialization?

// Pointer.
    fp->_p = fp->_base;

// Offsets
// Atualizando os offsets que foram apenas inicializados.

// #importante
// Não poderemos ler se r e w forem iguais.
// vamos ler do começo do arquivo.
    fp->_r = 0;

// #importante
// O ponteiro de escrita mudou 
// pois escrevemos um arquivo inteiro no buffer.
    //fp->_w = FileSize;
    fp->_w = fp->_fsize;
    
// #bugbug

    //if ( FileSize >= BUFSIZ )
    if (fp->_fsize >= fp->_lbfsize)
    {
        printf ("do_read_file_from_disk: the file is larger than the buffer\n");
        //refresh_screen();
        fp->_r = fp->_lbfsize;
        fp->_w = fp->_lbfsize;
        fp->_cnt = 0;
    }

// Agora temos menos espaço no buffer.
    //fp->_cnt = ( BUFSIZ - FileSize );
    fp->_cnt = ( fp->_lbfsize - fp->_fsize );

// Load.
// Load the file into the memory.
    //printf("Load ....\n");
    Status = 
        (int) fsLoadFile ( 
                  VOLUME1_FAT_ADDRESS, 
                  TargetDirAddress, 
                  NumberOfEntries,  //#bugbug: Number of entries.
                  file_name, 
                  (unsigned long) fp->_base,
                  fp->_lbfsize );

    if (Status != 0){
        printf ("do_read_file_from_disk: fsLoadFile fail\n");
        goto fail;
    }
    //printf("Loaded ....\n");

// #bugbug
// Agora é a hora de atualizarmos as tabelas ....
// Depois de carregarmos o arquivo.

//
// Pointer. (mode)
//

// Ajusta o ponteiro para depois do tamanho do arquivo.
// Dependendo do modo.
// See:
// https://linux.die.net/man/2/open

           /*
           O_RDONLY        open for reading only
           O_WRONLY        open for writing only
           O_RDWR          open for reading and writing
           O_NONBLOCK      do not block on open or for data to become available
           O_APPEND        append on each write
           O_CREAT         create file if it does not exist
           O_TRUNC         truncate size to 0
           O_EXCL          error if O_CREAT and the file exists
           O_SHLOCK        atomically obtain a shared lock
           O_EXLOCK        atomically obtain an exclusive lock
           O_NOFOLLOW      do not follow symlinks
           O_SYMLINK       allow open of symlinks
           O_FSYNC         write will save into the disk. :)
           */

// Saving into de disk ?
// You can use the O_FSYNC open mode to make write always 
// store the data to disk before returning;    

     // Default ???

    //if (mode == 0)
    //{
          debug_print ("do_read_file_from_disk: default mode\n");
          fp->_p = fp->_base;
    //}

// Pointer
    fp->_p = fp->_base;

// Offsets
// Atualizando os offsets que foram apenas inicializados.

// #importante
// Não poderemos ler se r e w forem iguais.
// vamos ler do começo do arquivo.
    fp->_r = 0;

// #importante
// O ponteiro de escrita mudou 
// pois escrevemos um arquivo inteiro no buffer.
    //fp->_w = FileSize;
    fp->_w = fp->_fsize;

    fp->_cnt = ( fp->_lbfsize - fp->_fsize );

// The file is opened in append mode. 
// O offset fica no fim do arquivo.
    if (mode & O_APPEND){
        debug_print ("do_read_file_from_disk: O_APPEND\n");
        //fp->_p = fp->_base + s;
    }

    if (mode & O_ASYNC){
         debug_print ("do_read_file_from_disk: O_ASYNC\n");
    }

/* 
    // Enable the close-on-exec flag for the new file descriptor.
    if ( mode & O_CLOEXEC ){ 
         debug_print ("sys_read_file: O_CLOEXEC\n");
    }
 */

    if (mode & O_CREAT){
         debug_print ("do_read_file_from_disk: O_CREAT\n");
    }

// #importante
// Se não liberarmos para leitura então read()
// não poderá ler.

    // ok to read
    fp->_flags = (fp->_flags | __SRD);

    // ok to write
    //fp->_flags = (fp->_flags | __SWR);

// Salva o ponteiro de estrutura de arquivo.  
// Ja checamos fd.
    p->Objects[__slot] = (unsigned long) fp;

    //#debug
    //printf ("process name: %s\n",p->__processname);
    //printf ("fd %d\n",fp->_file);
    //printf("do_read_file_from_disk-OUTPUT: %s \n",fp->_base);
    //refresh_screen();

done:
// Vamos retornar o fd.
// Pois essa rotina eh usada por open();
    return (int) fp->_file;
fail:
    //refresh_screen();
    return -1;
}

// Wrapper
int 
sys_read_file_from_disk ( 
    char *file_name, 
    int flags, 
    mode_t mode )
{

// Check validation
    if ((void*) file_name == NULL){
        return (int) (-EINVAL);
    }
    if (*file_name == 0){
        return (int) (-EINVAL);
    }


//
// Local copy
//

    char pathname_local_copy[256];
    memset(pathname_local_copy,0,256);
    // Coping more than we need, 
    // this way we're coping the 0x00 byte at the and of string
    // and some extra bytes.
    strncpy(pathname_local_copy,file_name,256);


    return (int) do_read_file_from_disk(
                    (char *) pathname_local_copy,
                    (int) flags, 
                    (mode_t) mode );
}


/*
 * do_write_file_to_disk:
 *     This is called by sys_write_file_to_disk().
 *     Interface para salvar arquivo ou diretório.
 *     Isso pode ser usado para criar um diretório ou 
 * copiar um diretório. 
 */
// #todo:
// vamos fazer igual ao sys_read_file 
// e criarmos opções ... se possível.
// IN: 
// name, size in sectors, size in bytes, adress, flag.

// Worker
int
do_write_file_to_disk ( 
    char *file_name, 
    unsigned long file_size,
    unsigned long size_in_bytes,
    char *file_address,
    char flag )  
{
    int __ret = -1;

    debug_print ("do_write_file_to_disk:\n");

    if ( (void*) file_name == NULL ){
        return (int) (-EINVAL);
    }
    if ( *file_name == 0 ){
        return (int) (-EINVAL);
    }

//++
// See: sci/fs/write.c
    //taskswitch_lock ();
    //scheduler_lock ();

    __ret = 
        (int) fsSaveFile ( 
                  VOLUME1_FAT_ADDRESS, 
                  VOLUME1_ROOTDIR_ADDRESS, 
                  FAT16_ROOT_ENTRIES,
                  (char *) file_name,
                  (unsigned long) file_size,
                  (unsigned long) size_in_bytes,
                  (char *) file_address,
                  (char) flag );

    //scheduler_unlock ();
    //taskswitch_unlock ();
//--

    return (int) __ret;
}

// Wrapper
int
sys_write_file_to_disk ( 
    char *file_name, 
    unsigned long file_size,
    unsigned long size_in_bytes,
    char *file_address,
    char flag )
{

// Check validation
    if ((void*) file_name == NULL){
        return (int) (-EINVAL);
    }
    if (*file_name == 0){
        return (int) (-EINVAL);
    }

//
// Local copy
//

    char pathname_local_copy[256];
    memset(pathname_local_copy,0,256);
    // Coping more than we need, 
    // this way we're coping the 0x00 byte at the and of string
    // and some extra bytes.
    strncpy(pathname_local_copy,file_name,256);


    return (int) do_write_file_to_disk(
                     (char *) pathname_local_copy,
                     (unsigned long) file_size,
                     (unsigned long) size_in_bytes,
                     (char *) file_address,
                     (char) flag );
                     
}


// ==============================
// Service 43
// See: see sci.c

int sys_create_empty_file(char *file_name)
{
    int __ret = -1;
    //char *FileName;
    //FileName = (char *) file_name;
// #bugbug: 
// We need a buffer in another place.
// #todo: Allocate space for a new file.
    char buffer[BUFSIZ];
    //char *buf;
// How many bytes.
    int FileSizeInBytes = 512;
// How many sectors.
// (FileSizeInBytes/512)
    int NumberOfSectors = 1;

    debug_print ("sys_create_empty_file:\n");
    printf("sys_create_empty_file:\n");

    if ((void*) file_name == NULL){
        return (int) (-EINVAL);
    }
    if (*file_name == 0){
        return (int) (-EINVAL);
    }

// Change the string format.
    fs_fntos( (char *) file_name );

// #test
// Not empty file.
    memset(buffer,0,BUFSIZ);
    sprintf(buffer,"This is a new file.");

// 0x20 = file.
// See: write.c
    //printf ("0x20 \n");
    __ret = 
        (int) fsSaveFile ( 
                  VOLUME1_FAT_ADDRESS, 
                  VOLUME1_ROOTDIR_ADDRESS, 
                  FAT16_ROOT_ENTRIES,
                  (char *)         file_name,
                  (unsigned long)  NumberOfSectors, 
                  (unsigned long)  FileSizeInBytes,  
                  (char *)         &buffer[0], 
                  (char)           0x20 ); 

    if (__ret<0)
    {
        debug_print("sys_create_empty_file: fail\n");
        return -1;
    }

// #todo
// the file structure.

    return (int) __ret;
}


// ================================
// Service 44
// See: fs_create_empty_directory
int sys_create_empty_directory(char *dir_name)
{
    int __ret=0;

// #bugbug: 
// We need a buffer in another place.
    char buffer[BUFSIZ];
    int size_in_bytes = 512; 
    int number_of_sectors = 1;

    debug_print ("sys_create_empty_directory:\n");
    printf("sys_create_empty_directory:\n");
    
    if ((void*) dir_name == NULL){
        return (int) (-EINVAL);
    }
    if (*dir_name == 0){
        return (int) (-EINVAL);
    }

    fs_fntos((char *) dir_name);

    memset(buffer,0,BUFSIZ);

// See: write.c
// 0x10 = directory. 
    //printf ("0x10 \n");
    __ret = 
        (int) fsSaveFile ( 
                  VOLUME1_FAT_ADDRESS, 
                  VOLUME1_ROOTDIR_ADDRESS, 
                  FAT16_ROOT_ENTRIES,
                  (char *)         dir_name,
                  (unsigned long)  number_of_sectors, 
                  (unsigned long)  size_in_bytes, 
                  (char *)         &buffer[0], 
                  (char)           0x10 ); 

    if (__ret<0){
        debug_print("sys_create_empty_directory: fail\n");
        return -1;
    }

// #todo
// the file structure.

    return (int) __ret;
}

void set_global_open_file ( void *file, int Index )
{

// #todo:
// Limite maximo da lista.

// Structure
    if ( (void *) file == NULL )
    {
        // ?? todo: message
        return;
    }

    if (Index < 0){
        // ?? todo: message
        return;
    }

// Include pointer in the list.

     file_table[Index] = (unsigned long) file;
}

void *get_global_open_file (int Index)
{

//Limits.
//@todo: max. NUMBER_OF_FILES

    if (Index < 0){
        // ?? todo: message
        return NULL;
    }

    return (void *) file_table[Index];
}


/*
 *  sys_cd_command:
 * 
 */
// Service 175. cd command.
// #todo
// ou usamos o cwd do processo ou
// o diretorio raiz para paths absolutos.

void sys_cd_command (const char *string)
{
    //int i=0;

    if ( (void*) string == NULL ){
        debug_print("sys_cd_command: string\n");
        return;
    }
    if ( *string == 0 ){
        debug_print("sys_cd_command: *string\n");
        return;
    }

// Reset global structure and cwd on process structure.

// #bugbug
// Talvez esse tipo de tratamento precise 
// ser feito pelo próprio shell.

    if ( string[1] == 0 )
    {

        // $ cd /
        // We also need to clean the name in the process structure.
        if (string[0] == '/'){
            debug_print("sys_cd_command: reseting\n");
            
            //if ( directory_facility_RootDir.initialized == TRUE){
            //    fsInitTargetDir(
            //        directory_facility_RootDir.dir_address,
            //        directory_facility_RootDir.dir_name );
            //}

            fsInitTargetDir(VOLUME1_ROOTDIR_ADDRESS,"/");
            //#bugbug: invalid pid
            //fs_initialize_process_cwd ( current_directory, "/" );
            return;
        }

        // ...

        return;
    }

// Atualiza na estrutura de processo.
// Atualiza na estrutura global para diretorio alvo.
    fsUpdateWorkingDiretoryString( (char *) string );
// Isso carrega o diretorio que agora 'e o diretorio alvo.
    fsLoadFileFromCurrentTargetDir();
    // ...
}

//
// End
//

