#include "PFSHeader.h"


// Open the log 
// Wrie message inside the log
// Thread safe

int openLogger(const int FLAG){

  int log_fd = 0;

  switch (FLAG){

     case PFS_LOG_CREATE_NEW: 
                           if( (log_fd = open( PFS_LOGGER_PATH, (int)PFS_NEW_FILE_FLAGS, (mode_t)PFS_NEW_FILE_PERMS )) == PFS_FAILURE ){
                              return PFS_FAILURE;
                           } 
                           else {
                              return log_fd;
                           }

     case PFS_LOG_OPEN_OLD:  
                         if( (log_fd = open( PFS_LOGGER_PATH, (int)PFS_OLD_FILE_FLAGS )) == PFS_FAILURE ){
                            return PFS_FAILURE;
                         }
                         else {
                            return log_fd;
                         }

     case PFS_LOG_OPEN_FIFO_READ:
                              if( (log_fd = open( PFS_LOG_FIFO_TEMPLATE, O_RDONLY )) == PFS_FAILURE ){
                                 return PFS_FAILURE;
                              }
                              else {
                                 return log_fd;
                              }


     default: return PFS_FAILURE;
  }
  return PFS_FAILURE;
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
void printError(const int line, const char *file, const char *msg ){

  fprintf(stderr, "L: %d |F: %s |M: %s|T: %s\n", line, file, msg, strerror( errno ) );
  exit( EXIT_FAILURE ); 
}// end



// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* Initialize the logger as well as logger components */
int initPFSLogger(){
 
   int logger_fd = 0;
 
    // Create the new logger file   
    if( (logger_fd = openLogger(PFS_LOG_CREATE_NEW)) == PFS_FAILURE ){

       // Open the old logger file
       if( (logger_fd = openLogger(PFS_LOG_OPEN_OLD)) == PFS_FAILURE ){
          return PFS_FAILURE;
       } 
    }  
  return logger_fd;
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
int writeInFile(int fd, const int line, const char *file, const char *msg ){

  char buffer[PFSLEN] = {0};

  snprintf(  buffer, PFSLEN, "%d | %s | %s.\n", line, file, msg );

  if( write( fd, buffer, strlen( buffer ) ) == PFS_FAILURE ){
     return PFS_FAILURE;
  }
  return PFS_SUCCESS;
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
void writeInPFSLog(const int line, const char *file, const char *msg){

  static int log_fd;

  // Initialize the logger
  if( log_fd == 0 ){

     if( (log_fd = initPFSLogger()) == PFS_FAILURE ){
        printError( __LINE__,__FILENAME__,  _file_open_error_msg_ ); 
     }      
  }

  // Write in logger file
  if( writeInFile( log_fd, line, file, msg ) == PFS_FAILURE ){
     printError( __LINE__, __FILENAME__, _file_write_error_msg_ );
  }
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* Parse out the message and save it inside the log */
void parseLogMsg( const char *buf ){

   int  line         = 0;
   char file[PFSLEN] = {0};
   char msg[PFSLEN]  = {0};
   int  count = 0;
   int  index = 0;

   while( LOOP_INFINITE ){

      if( (*buf == '|') || (*buf == '\n') || (*buf == '\0') ){

          switch(count){

             case 0: file[index] = '\0';
                     line = (int) strtol( file, NULL, BASE_TEN_NUM );
                     break;

             case 1: file[index] = '\0';
                     break;
  
             case 2: msg[index] = '\0';
                     break;
          }

        // Time to break
        if( (*buf == '\n') || (*buf == '\0') ) break;
        else {
           buf++;
           count++;
           index = 0;
        } 
      }

      switch(count){

         case 0: file[index++] = *buf;
                 break;

         case 1: file[index++] = *buf; 
                 break;

         case 2: msg[index++] = *buf;
                 break;
      }
      buf++;
   }

   // Write the message inside the log file
   writeInPFSLog( line, file, msg ); 
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* Thread interface for PFS Logger */
void *T_writeInPFSLog( void *args ){

   char readbuf[PFSLEN]  = {0};
   int  fifo_fd          =  0;

   // Open MKFIFO for PFSLogger
   if( (mkfifo( PFS_LOG_FIFO_TEMPLATE, (mode_t) PFS_NEW_FILE_PERMS ) == PFS_FAILURE) && (errno != EEXIST) ){
      printError( __LINE__, __FILENAME__, _mkfifo_error_ );  
   } 
   // Open the FIFO for read only
   else {

      if( (fifo_fd = openLogger( PFS_LOG_OPEN_FIFO_READ )) == PFS_FAILURE ){
         printError( __LINE__, __FILENAME__, _mkfifo_open_read_error_ );
      }
   }

   // Make the thread running
   while( IS_EXIT_PFS_LOGGER ){

       // Wait on read operation
       if( read( fifo_fd, readbuf, PFSLEN ) == PFS_FAILURE ){
          printError( __LINE__, __FILENAME__, _mkfifo_read_error_ );
       } 
       // Parse the message and write it inside the Logger file 
       else {

          pthread_mutex_lock( &(PFSThread.pfs_logger_mutex) );
          readbuf[strlen(readbuf)] = '\0';    // add up the string terminator.
          parseLogMsg( readbuf );

          // Reset the buf
          memset( readbuf, PFS_SUCCESS, (sizeof(char) * PFSLEN) );
          pthread_mutex_unlock( &(PFSThread.pfs_logger_mutex) );

       }   
   }
  return (void*)NULL;
}// end
