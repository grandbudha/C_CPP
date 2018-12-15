#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>


typedef struct workqueue {

   char              *data;
   struct workqueue  *next;
} WQ ;

typedef struct pool {

   pthread_mutex_t        mutex;
   pthread_cond_t         cond;
   pthread_attr_t         attr;
   WQ                     *head;
   WQ                     *tail;
   
   int                    magic_num;        // Initialized while initializing the work queue
   int                    shutdown;         // Tells thread to shutdown as pool is closed now
   int                    maximum_thread;   // Number of threads in pool
   int                    current_thread;   // Number of threads already created
   int                    idle_thread;      // Number of threads in idle state

   void                   (*engine)( char * arg );   
} TPOOL;


//---------------------------
 static void *worker_thread( void *args ){

   struct timespec timeout;
   TPOOL *wq = (TPOOL *) args;
   WQ *we;
   int terror, timedout;

   // Lock the mutex
   pthread_mutex_lock( &wq->mutex );

   while( 1 ){

      timedout = 0;
      clock_gettime( CLOCK_REALTIME, &timeout );
      timeout.tv_sec += 2;

      while( (wq->head == NULL) && !wq->shutdown ){

          if( (terror = pthread_cond_timedwait( &wq->cond, &wq->mutex, &timeout)) !=0 ){

              // In case when timedout
              if( terror == ETIMEDOUT ){

                 puts( "Worker timed out");
                 timedout = 1;
                 break;
              } 
              else {

                 puts( "Worker wait fail");
                 wq->current_thread -= 1;
                 pthread_mutex_unlock( &wq->mutex );
                 return NULL;
              }
          }
      } 
      
      // Take out the work from WQ
      we = wq->head;
 
      if( we != NULL ){

         wq->head = we->next;
         if( wq->tail == we ) wq->tail = NULL;

         // Unlock the mutex
         pthread_mutex_unlock( &wq->mutex ); 

         // Call the function
         wq->engine( we->data );

         free( we );
      }
  
      // In case if there are no more requests and server has asked to quit
      if( wq->head == NULL && wq->shutdown ){

         puts( "shutting down" );
         wq->current_thread -= 1;

         if( wq->current_thread == 0 ) 
            pthread_cond_broadcast( &wq->cond );

         pthread_mutex_unlock( &wq->mutex );
         return NULL; 
      }

      // If there is no work and we have terminated because of timedout
      if( (wq->head == NULL) && timedout ){

         puts("Termination because of timedout" );
         wq->current_thread -= 1;
         break;
      }
   }

   // Unlock
   pthread_mutex_unlock( &wq->mutex );
   puts("BYE");
   return NULL;
}// end 

//---------------------------

 int TPOOL_INIT( TPOOL *queue, int num_of_threads, void (*engine)(char *) ){

   int terror = 0;

   // Initialize attribute variable for threads
   if( (terror = pthread_attr_init( &queue->attr )) !=0 ){
      return terror;
   }
  
   // Initialize the type of attribute
   if( (terror = pthread_attr_setdetachstate( &queue->attr, PTHREAD_CREATE_DETACHED )) !=0 ){
      pthread_attr_destroy( &queue->attr );
      return terror;
   }

   // Initialize the Mutex
   if( (terror = pthread_mutex_init( &queue->mutex, NULL )) !=0 ){
      pthread_attr_destroy( &queue->attr );
      return terror;
   }

   // Initialize the condition varaibles
   if( (terror = pthread_cond_init( &queue->cond, NULL )) !=0 ){
      pthread_mutex_destroy( &queue->mutex );
      pthread_attr_destroy( &queue->attr );
      return terror;
   }

   // Initialize other varaibles
   queue->head = queue->tail = NULL;
   queue->shutdown = queue->idle_thread = queue->current_thread = 0;
   queue->maximum_thread = num_of_threads;
   queue->engine = engine;
   queue->magic_num = 12345;
    
  return 0;
}// end  

//---------------------------

 int TPOOL_ADD( TPOOL *queue, char *element ){

   WQ *temp = NULL;
   pthread_t id;
   int terror = 0;

   // Check for valid number
   if( queue->magic_num != 12345 ){
      return EINVAL;
   }

   // Allocate memory
   if( (temp = malloc( sizeof( WQ ) )) == NULL ){
      return ENOMEM;
   }
   else {

      if( (temp->data = malloc( strlen( element ) * sizeof( char ))) == NULL ){
         free( temp );
         return ENOMEM;
      }
      else {
         strcpy( temp->data, element );
      }
      temp->next = NULL;  
   }

   // Lock the mutex
   pthread_mutex_lock( &queue->mutex );

      // Add node 
      if( queue->head == NULL )
         queue->head = temp;
      else
         queue->tail->next = temp;
      queue->tail = temp;

      // Wake up any idle thread
      if( queue->idle_thread > 0 ){

         pthread_cond_signal( &queue->cond );
      }
      else if( queue->current_thread < queue->maximum_thread ){

         if( (terror = pthread_create( &id, &queue->attr, worker_thread, (void *) queue )) !=0 ){
            
            pthread_mutex_unlock( &queue->mutex );
            return terror;
         }
         else {
            queue->current_thread += 1;
         } 
      } 

   // Unlock the mutext
   pthread_mutex_unlock( &queue->mutex );
 return 0;
}// end

//---------------------------

 int TPOOL_DESTROY( TPOOL *queue ){

    int terrno = 0;
  
    // Check for valid magic number
    if( queue->magic_num != 12345 ){
       return EINVAL;
    } 
    
    // Acquire lock
    pthread_mutex_lock( &queue->mutex );

       // Prevent any other operations
       queue->magic_num = 0;

       // Check if threads are running, give them signal to shutdown ones the work is done
       if( queue->current_thread > 0 ){

          // Set the shutdown flag
          queue->shutdown = 1;

          // Wake up the idle threads
          if( queue->idle_thread > 0 )
             if( (terrno = pthread_cond_broadcast( &queue->cond )) !=0 ){
                pthread_mutex_unlock( &queue->mutex );
                return terrno;
             } 
       }

       // Wait for conditon variable
       while( queue->current_thread > 0 ){
 
          if( (terrno = pthread_cond_wait( &queue->cond, &queue->mutex )) != 0){
             pthread_mutex_unlock( &queue->mutex );
             return terrno;
          }
       }

    // Unlock the mutex
    pthread_mutex_unlock( &queue->mutex );  

    // Destroy all
    pthread_mutex_destroy( &queue->mutex );
    pthread_cond_destroy( &queue->cond );
    pthread_attr_destroy( &queue->attr );

    return 0;
}// end

int count = 1;

//----------------------------------
 void thread_function( char *data ){  

    printf( "%d. TF: %s | %d\n", count++, data, (int) pthread_self() );
}// end


TPOOL workQueue;



//--------------------------- MAIN
 int main( int argc, char **argv ){

   int terrno = 0;
   int i = 0;

   // Initialize the thread pool
   if( (terrno = TPOOL_INIT( &workQueue, 5, thread_function )) !=0 ){
      fprintf( stderr, "TPOOL_INIT. %s\n", strerror( terrno ) );
      exit( EXIT_FAILURE ); 
   } 


   // Add up the work
   for( i = 0; i < 20; ++i ){

       if( (terrno = TPOOL_ADD( &workQueue, "Hello" )) !=0 ){
          fprintf( stderr, "TPPOL_ADD. %s\n", strerror( terrno ) );
          exit( EXIT_FAILURE );
       }
   } 

  
   // Destroy thread pool
   if( (terrno = TPOOL_DESTROY( &workQueue )) !=0 ){
      fprintf( stderr, "TPPOL_DESTROY. %s\n", strerror( terrno ) );
      exit( EXIT_FAILURE );

   }

/*
   queue->shutdown = queue->idle_thread = queue->current_thread = 0;
   queue->maximum_thread = num_of_threads;
   queue->engine = engine;
   queue->magic_num = 12345;
*/ 

  printf( "Shutdown: %d | IT: %d | CT: %d\n", workQueue.shutdown, workQueue.idle_thread, 
                     workQueue.current_thread );
  
    
  exit( EXIT_SUCCESS );
}// end



