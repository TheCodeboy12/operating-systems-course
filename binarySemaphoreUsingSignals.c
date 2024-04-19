#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <pthread.h>
#define NUM_THREADS 5
sigset_t SIGNAL_MASK;
void sem_init(int status){
    // first we check that the given status is correct. Else exit with an error message.
    if(status != 1 && status != 0){
        perror("The semaphore ie binary, please enter 1 or 0\n");
        pthread_exit((void * )1) ;
    }
    if (status == 1 ) // Semaphore is unlocked
    {
        // now we add SIGSUR to the signal mask of this process and we make the process ignore it.
        sigemptyset(&SIGNAL_MASK);
        if(sigaddset(&SIGNAL_MASK , SIGUSR1) == -1 ){ // set the signal and deal with errors
            perror("sigaddset error");
            pthread_exit((void * )1);
        }
        if (sigprocmask (SIG_BLOCK, &SIGNAL_MASK, NULL) != 0){ // Add the set to the list of signals that the process should ignore.
            perror("pthread_sigmask\n");
            pthread_exit((void * )1) ;
        }
        // we send a signal to add it to the signal queue.
        // That means that the first thread to call the sem_down function will run and not wait since it will have the OK
        kill(getpid() , SIGUSR1) ;
    }
    else {
        // if the status is locked
        perror("Status should always be unlocked initially");
        pthread_exit((void * )1) ;
    }
    return ;
}

void sem_down() {
    int received_signal;
    sigwait(&SIGNAL_MASK, &received_signal); 
}


void sem_up() {
    kill(getpid(), SIGUSR1); //we unlock the sempahore
    // this works becuase my semdown function puts the thread to sleep (or makes it 'wait') 
    // until the process receives SIGSUR1 only then it can continue
}
void *someCriticalSection(void *arg) {
    sem_down(); // we check if there is SIGSUR1 in the queue
    printf("Thread %d: Entering critical section... \n", (int)(size_t)arg);
    sleep(2); // Simulate some work
    printf("Thread %d: Leaving critical section...\n", (int)(size_t)arg);
    sem_up();
    return NULL;
}
int main() {
    pthread_t thread[NUM_THREADS];
    int i;
    // Initialize the semaphore as locked (binary semaphore)
    sem_init(1); 

    // Create threads
    for (i = 0 ; i < NUM_THREADS ; i++){
        pthread_create(&thread[i], NULL, someCriticalSection, (void*) (size_t) i);
    }
    //kill thrads
    for (i = 0; i < NUM_THREADS; i++) {
        pthread_join(thread[i], NULL);
    }

    printf("All threads finished.\n");
    return 0;
}
