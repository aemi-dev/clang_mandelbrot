#include <stdio.h>
#include <stdlib.h>

#include <pthread.h>

/*
[...]
[...]
*/
#define NBTHREAD 4

struct thread_param {
  double toto;
  int * tabTiti;
/*
  [...]
  [...]
*/
};

void * threadFonction (void * p_arg)
{
  struct thread_param * arg = (struct thread_param *) p_arg;
  double internal_toto = arg->toto;
  int * internalTabTiti = arg->tabTiti
/*
  [...]
  [...]
*/
/*sortie du thread*/
  pthread_exit(NULL);
}


int main(int argc,char **argv)
{
/*
  [...]
  [...]
*/
  int * commonTab;

  int thread_id;
  struct thread_param thread_arg[NBTHREAD];
  pthread_t thread[NBTHREAD];
/*
  [...]
  [...]
*/

/*initialisation et lancement des threads*/
  for(thread_id=0;thread_id<NBTHREAD;thread_id++) {
    int rc=0;
/*initialisation des parametres*/
    thread_arg[thread_id].toto = 3*34 + 99 - 128 + 77 * thread_id;
    thread_arg[thread_id].tabTiti = & commonTab[437 * thread_id];
/*
    [...]
    [...]
*/
/*lancement des threads*/
    if( (rc=pthread_create(&thread[thread_id], NULL, threadFonction, (void *) &thread_arg[thread_id])) != 0 ) {
      printf("Erreur a la creation du thread %d, rc : %d .\n",thread_id,rc);
      return EXIT_FAILURE;
    }
  }

/*attente des threads*/
  for(thread_id=0;thread_id<NBTHREAD;thread_id++) {
    void * ret_ptr;
    pthread_join(thread[thread_id],&ret_ptr);
  }
/*
  [...]
  [...]
*/

/*sortie du programme*/
  return EXIT_SUCCESS;
}
