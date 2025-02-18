
// sched.h

#ifndef __PS_SCHED_H
#define __PS_SCHED_H    1

//#define NR_TASKS ??
//#define FIRST_TASK ?
//#define LAST_TASK ? 

// Status do scheduler.
#define LOCKED    0
#define UNLOCKED  1  

/*
 * Scheduling policies
 */

#define SCHED_RR    0
//#define SCHED_??    1
// ...

struct scheduler_info_d
{
    int initialized;
    int policy;
    unsigned long rr_round_counter;
    unsigned long flags;
};

extern struct scheduler_info_d  SchedulerInfo;


// ==========================================
// Input responder
// See: schedi.c
int flagUseThisInputResponder;
int input_responder_tid;
int check_for_input_responder(void);
void set_input_responder_tid(int tid);
// ==========================================

//
// == prototypes ============
//

int init_scheduler (unsigned long sched_flags);

tid_t scheduler(void);

void scheduler_lock (void);
void scheduler_unlock (void);
unsigned long scheduler_get_status (void);

void cut_round(struct thread_d *last_thread);

tid_t psScheduler(void);

void do_credits(struct thread_d *thread);
void do_credits_by_tid(tid_t tid);

// Switching states.
void do_thread_initialized(tid_t tid);
void do_thread_standby(tid_t tid);
void do_thread_running(tid_t tid);
void do_thread_ready(tid_t tid);

void do_thread_waiting(tid_t tid, unsigned long ms);

void do_thread_blocked(tid_t tid);
void do_thread_zombie(tid_t tid);
void do_thread_dead(tid_t tid);

void drop_quantum(struct thread_d *thread);

int do_waitpid (pid_t pid, int *status, int options);

int get_current_thread (void);
void set_current_thread (int tid);

void wait_for_a_reason ( int tid, int reason );
int wakeup_thread_reason ( int tid, int reason );
int wakeup_scan_thread_reason ( int reason );
void wakeup_thread (int tid);
void yield (tid_t tid);
void sleep (tid_t tid, unsigned long ms);
void sleep_until (tid_t tid, unsigned long ms);

void sys_broken_vessels(tid_t tid);

void check_for_standby (void);

#endif    



















