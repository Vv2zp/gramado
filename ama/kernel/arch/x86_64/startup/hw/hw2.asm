
; hw2.asm

;
; Imports
;


;
; Callback support.
;

; Flag
extern _asmflagDoCallbackAfterCR3
; Address
extern _ring3_callback_address


; ts
extern _task_switch_status


;;
;; == Context =============================================
;;

; See:
; x64cont.c

extern _contextSS        ; User Mode.
extern _contextRSP       ; User Mode.
extern _contextRFLAGS    ; User Mode.
extern _contextCS        ; User Mode.
extern _contextRIP       ; User Mode.

extern _contextDS
extern _contextES
extern _contextFS  ;?
extern _contextGS  ;?

extern _contextRAX
extern _contextRBX
extern _contextRCX
extern _contextRDX

extern _contextRSI
extern _contextRDI

extern _contextRBP

extern _contextR8
extern _contextR9
extern _contextR10
extern _contextR11
extern _contextR12
extern _contextR13
extern _contextR14
extern _contextR15

; ...


; -------------------------------------
; callback restorer.
; temos que terminal a rotina do timer e
; retornarmos para ring 3 com o contexto o ultimo contexto salvo.
; #bugbug
; We gotta check what is the process calling this routine.

global _callback_restorer
_callback_restorer:

; Drop the useless stack frame.
; We were in the middle of the timer interrupt,
; so, we're gonna use the saved context to release the next thread.

    pop rax  ; rip
    pop rax  ; cs
    pop rax  ; rflags
    pop rax  ; rsp
    pop rax  ; ss

; #bugbug
; We gotta check what is the process calling this routine.
     ;call __xxxxCheckCallerPID

; Clear the variables.
; Clear the flag and the procedure address.
; Desse jeito a rotina de saida não tentará
; chamar o callback novamente.
    mov qword [_asmflagDoCallbackAfterCR3], 0
    mov qword [_ring3_callback_address], 0

; Normal timer exit. (after cr3).
; temos que terminal a rotina do timer e
; retornarmos para ring 3 com o contexto o ultimo contexto salvo.
; que ainda é o window server.
    jmp irq0_release

;------------------------------------------------
; Salta para um callback no window server em ring3.
; + Essa rotina foi chamada somente quando o kernel estava usando
;   a paginação do aplicativo window server.
; + Interrupçoes desabilitadas
; + eoi não acionado.
; Nessa hora vamos para ring3 executar um código no aplicativo,
; uma interrupção gerada pelo aplicativo nos trará de volta
; par ao kernel na rotina callback_restorer, e por fim
; devemos definitivamente voltarmos para o aplicativo em ring 3,
; através da rotina irq0_release.
; IN:
; ss     - Reusado do contexto salvo do aplicativo.
; rsp    - Reusado do contexto salvo do aplicativo.
; rflags - ring3, interrupções desabilitadas.
; cs     - Reusado do contexto salvo do aplicativo.
; rbx    - rip
;

__doRing3Callback:
; Get the RIP address.
    mov rbx, qword [_ring3_callback_address]
; Setup the stack frame.
    push qword [_contextSS]   ; ss
    push qword [_contextRSP]  ; rsp
    push qword 0x3000         ; rflags interrupçoes desabilitadas.
    push qword [_contextCS]   ; cs
    push rbx                  ; rip
; Go to the ring3 procedure.
    iretq

; -------------------------------------
; Irq0 release.
; Timer interrupt.
; See: _irq0 in unit1hw.asm.
; See: ts.c, pit.c, sci.c.

irq0_release:

; ring3 callback
; Se a flag indicar que sim,
; então iremos efetuar um iretq para dentro
; do processo window server.
; Quem aciona essa flag é psTaskSwitch() em ts.c.
; Depois que o contexto esta salve e se certificou
; que estamos rodando a thread de controle do window server.
; #importante: Antes da troca de tarefa, 
; então cr3 permanece sendo o do processo window server.
; #bugbug: 
; Precisa realmente ser antes de ts.c trocar de tarefa e 
; o contexto precisa estar salvo.
; Se fosse depois, então saltaríamos para outra tarefa
; diferente do window server e usaríamos endereços errados.
    mov rax, qword [_asmflagDoCallbackAfterCR3]
; Callback ring3 procedure.
    cmp rax, 0x1234
    je __doRing3Callback


; 64bit
; This is a 64bit pointer to the pml4 table.

; #bugbug
; Não precisamos fazer refresh em todo tick,
; somente quando houver troca de tarefa.

    mov RAX, CR3  
    IODELAY 
    mov CR3, RAX  

; Wait TLB.
    ;IODELAY 
;
; == Restore context ====================
;

    ; Segments
    xor rax, rax
    mov ax, word [_contextDS]
    mov ds, ax
    mov ax, word [_contextES]
    mov es, ax
    mov ax, word [_contextFS]
    mov fs, ax
    mov ax, word [_contextGS]
    mov gs, ax

    mov rsi, qword [_contextRSI] 
    mov rdi, qword [_contextRDI] 
    
    mov rbp, qword [_contextRBP] 
    
    mov rax, qword [_contextRAX] 
    mov rbx, qword [_contextRBX] 
    mov rcx, qword [_contextRCX] 
    mov rdx, qword [_contextRDX] 


    ;; Stack frame. (all double)
    push qword [_contextSS]      ; ss
    push qword [_contextRSP]     ; rsp
    push qword [_contextRFLAGS]  ; rflags
    push qword [_contextCS]      ; cs
    push qword [_contextRIP]     ; rip

    ; EOI - Only the first PIC.
    mov al, 20h
    out 20h, al  
    IODELAY  

    ;; variável usada pelo dispatcher.
    ;mov dword [_irq0PendingEOI], 0


    ; Acumulator.
    mov rax, qword [_contextRAX]

; #bugbug
; We do NOT need the 'sti'. 
; The flags in the 'eflags' will reenable it.

    sti
    iretq

;----------------------------------------------
; _turn_task_switch_on:
;     + Cria um vetor para o timer, IRQ0.
;     + Habilita o funcionamento do mecanismo de taskswitch.
;       Essa função é chamada em microkernel\cpu\start.c
;

global _turn_task_switch_on
_turn_task_switch_on:

    cli 
    mov rax, qword _irq0        ;definitivo
    mov rbx, qword 32
    call _setup_system_interrupt

    ;recarrega a nova idt (talvez seja necessario)
    ;lidt [IDT_register]

    ;status do mecanismo de taskswitch.
    mov qword [_task_switch_status], qword 1    ;UNLOCKED.
    ret



;-------------------------------------------------------------
; _turn_task_switch_off:
;     Desabilita a IRQ0 responsável por fazer o task switch.
;
global _turn_task_switch_off
_turn_task_switch_off:

    cli
    mov rax, qword unhandled_irq
    mov rbx, qword 32
    call _setup_system_interrupt

    ;status do mecanismo de taskswitch.
    mov qword [_task_switch_status], qword 0    ;LOCKED.
    ret




