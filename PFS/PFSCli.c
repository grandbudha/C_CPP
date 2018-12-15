#include "PFSHeader.h"


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
int openFile(const int FLAG){

  int log_fd = 0;

  switch(FLAG){
 
      case PFS_LOG_OPEN_FIFO_WRITE:
                               if( (log_fd = open( PFS_LOG_FIFO_TEMPLATE, O_WRONLY )) == PFS_FAILURE ){
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
 void makeLogMsg( const int line, const char *msg ){

    static int log_fd;
    char  buf[PFSLEN] = {0};

    if( log_fd == 0 ){

     if( (log_fd = openFile( PFS_LOG_OPEN_FIFO_WRITE )) == PFS_FAILURE ){
         printError( __LINE__, __FILENAME__, _mkfifo_open_write_error_ );
      } 
    }
 
    // Make log message
    snprintf( buf, PFSLEN, "%d | %s | %s", line, __FILENAME__, msg ); 
   
    // Write it
    if( write( log_fd, buf, strlen( buf )) == PFS_FAILURE ){
       printError( __LINE__, __FILENAME__, _mkfifo_write_error_ );
    }
}// end

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
int validatePFSCmd( char *str ){

  const char *is_open = "open";
  const char *is_put  = "put";
  const char *is_get  = "get";
  const char *is_rm   = "rm";
  const char *is_dir  = "dir";
  const char *is_putr = "putr";
  const char *is_kill = "kill";
  const char *is_quit = "quit";

  // Check for null
  if( str == NULL ){
     return PFS_FAILURE;
  }

  // Check the first occurence of the command
  if( strstr(str, is_open) != NULL ){
     return PFS_OPEN_CMD;
  }
  else if( strstr(str, is_put) != NULL ){
     return PFS_PUT_CMD;
  }
  else if( strstr(str, is_get) != NULL ){
     return PFS_GET_CMD;
  }
  else if( strstr(str, is_rm) != NULL ){
     return PFS_RM_CMD;
  }
  else if( strstr(str, is_dir) != NULL ){
     return PFS_DIR_CMD;
  }
  else if( strstr(str, is_putr) != NULL ){
     return PFS_PUTR_CMD;
  }
  else if( strstr(str, is_kill) != NULL ){
     return PFS_KILL_CMD;
  }
  else if( strstr(str, is_quit) != NULL ){
     return PFS_QUIT_CMD; 
  }
  else {
     return PFS_FAILURE;
  }
}// end


// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ //
/* */
void *T_PFSCli(void *args){

   char       inputbuf[PFSLEN]  = {0}; 
   char       outputbuf[PFSLEN] = {0};
   int        command_type      = 0;
   const char *is_null          = "NULL";

   // Open Fifo for logger
   if( (mkfifo( PFS_LOG_FIFO_TEMPLATE, (mode_t) PFS_NEW_FILE_PERMS ) == PFS_FAILURE) && (errno != EEXIST) ){
      printError( __LINE__, __FILENAME__, _mkfifo_error_ );
   }

   while( IS_EXIT_PFS_CLI ){

       // Ask for the command input
       memset( inputbuf, RESET_MEMSET, sizeof(char)*PFSLEN );
       memset( outputbuf, RESET_MEMSET, sizeof(char)*PFSLEN );

       printf( "\nPFS> " );
       fgets( inputbuf, PFSLEN, stdin );
       strtok( inputbuf, "\n" );               // remove the last newline

       // Put the command in logger
       makeLogMsg( PFS_FAILURE, inputbuf );

       // Validate for the command
       if( (command_type = validatePFSCmd(inputbuf)) == PFS_FAILURE ){
          puts( _cli_invalid_cmd_ );
          makeLogMsg( PFS_FAILURE, inputbuf );
       }
       else {

          // Hamdle the command
          strcpy( outputbuf, executePFSCmd(inputbuf,command_type) );

          // Compare if there is any error message while processing the command
          if( strcmp( outputbuf, is_null ) != PFS_SUCCESS ){
             puts( outputbuf );
             makeLogMsg( PFS_FAILURE, outputbuf );
          }
       }
   }
  return (void*)NULL;
}// end
