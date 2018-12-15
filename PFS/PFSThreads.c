#include "PFSHeader.h"


int initPFSLoggerThread(pthread_mutex_t *tmutex, pthread_t *tid){

  int terrno = 0;

  // Initialize the mutex
  if( (terrno = pthread_mutex_init( tmutex, NULL )) != PFS_SUCCESS ){
     errno = terrno; return PFS_FAILURE;
  }

  // Start the thread
  if( (terrno = pthread_create( tid, NULL, T_writeInPFSLog, NULL)) != PFS_SUCCESS ){
     errno = terrno; return PFS_FAILURE;
  }  
  return PFS_SUCCESS;
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
int initPFSCliThread(pthread_t *tid){

  int terrno = 0;

  if( (terrno = pthread_create( tid, NULL, T_PFSCli, NULL )) != PFS_SUCCESS ){
     errno = terrno; return PFS_FAILURE;
  }
  return PFS_SUCCESS;
}// end


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //


int initThreads(){

  // Initialize PFSLogger thread
  if( initPFSLoggerThread( &(PFSThread.pfs_logger_mutex) , &(PFSThread.pfs_logger_tid) ) == PFS_FAILURE ){
     return PFS_FAILURE;  
  }

  // initialize PFS Command-line argument( PFSCli ) thread
  if( initPFSCliThread( &(PFSThread.pfs_cli_tid) ) == PFS_FAILURE ){
     return PFS_FAILURE;
  }
  return PFS_SUCCESS;
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* Start all the processes related to PFS */
void startPFSProcesses(){

   if(initThreads() == PFS_FAILURE ){
     printError( __LINE__, __FILENAME__, _process_start_error_ );
   }

}// end
