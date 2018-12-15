#include "PFSHeader.h"
//#include "PFSCDS.h"

/*
 PFS file size : 10KBytes = 10 * 1000

*/
const char *PFSOpen( const char *cmd, char *file_name ){

   char       *name  = NULL;
   const char *pfs   = "pfs";
   const char *c_pfs = "PFS";

   name = strchr( cmd, ' ' );    // get the first occurence of ' '

   // user has not provided any name
   if( name == NULL ){
      
      // Save the file name
      strcpy( file_name, PFS_DIR_PATH );
      strcat( file_name, c_pfs );
   }
   else {
  
      // Check if the name is valid
      if( (strncmp(name, pfs, strlen(pfs)) == PFS_SUCCESS) || (strncmp(name, c_pfs, strlen(c_pfs)) == PFS_SUCCESS) ){
         strcpy( file_name, PFS_DIR_PATH );
         strcat( file_name, name );
      }      
      // Invalid file name
      else {
         return _pfs_invalid_pfs_file_name_;
      }
   }
  return _pfs_no_error_msg_;
}// end

// ---------------------------------------------------------------------------- //
/*
WHT : Check if the file name/path is provided.
      Check if the file name/path is valid.
*/
const char *PFSPut( const char *cmd, char *file_name, int *fd ){

   char *file_path_ptr         = NULL;

   // Get the file path
   if( (file_path_ptr = strcpy( file_name, (strchr( cmd, ' ' )+ONE))) == NULL ){
      return _pfs_cmd_put_path_missing_error_;
   }
   // Check for the path
   else {

      if( (*fd = open( file_path_ptr, O_RDONLY )) == PFS_FAILURE ){
         return _pfs_cmd_put_invalid_path_error_;
      }
   }
  return _pfs_no_error_msg_;
}// end

// ---------------------------------------------------------------------------- //
/*

*/
 void makePFSArgs( struct pfsargs *args, char *file_name, int type, int fd ){

    // filename
    if( (args->file_name = malloc( (sizeof(file_name)+ADD_TERMINATOR)*sizeof(char) )) == NULL ){
       makeLogMsg( __LINE__, _pfs_malloc_error_ );
    }
    else {
       strcpy( args->file_name, file_name );   
       args->cmd_type = type;
       args->file_fd = fd;
    } 
}// end



// ---------------------------------------------------------------------------- //
/* */
char *executePFSCmd( char *cmd, const int cmd_type ){

  static char retbuf[PFSLEN]   = {0}; 
  char        pfs_name[PFSLEN] = {0};  // remove static
  int         pfs_fd = 0;
  static int  is_cmd_open_ran = 0;
  struct pfsargs  args;

  // Guard for open cmd
  if( (is_cmd_open_ran == IS_PFS_OPEN_CMD_ALREADY_RUN) && (cmd_type != PFS_OPEN_CMD) ){
     strcpy( retbuf, _first_cmd_error_); 
     return retbuf;
  } 

  switch( cmd_type ){

     case PFS_OPEN_CMD:

      // First command must be open, ignore open cmd after first
      if( is_cmd_open_ran != IS_PFS_OPEN_CMD_ALREADY_RUN ){
         strcpy( retbuf, _first_cmd_again_error_);
         return retbuf;
      } 
      else {
 
        // Handle the open command
        if( strcmp( strcpy(retbuf,PFSOpen(cmd, pfs_name)), _pfs_invalid_pfs_file_name_ ) == PFS_SUCCESS ){
           return retbuf;
        } 

        // Open or create the file
        if( (pfs_fd = open( pfs_name, (int)PFS_OLD_FILE_FLAGS )) == PFS_FAILURE ){

           // Create the file
           if( (pfs_fd = open( pfs_name, (int)PFS_NEW_FILE_FLAGS, (mode_t)PFS_NEW_FILE_PERMS )) == PFS_FAILURE ){
              strcpy( retbuf, strerror( errno ) );
              return retbuf;
           } 
        }

        strcpy( retbuf, _pfs_file_open_success_ );
        ++is_cmd_open_ran;
      }   
     break;

     case PFS_PUT_CMD: 

        if( (strcmp(strcpy(retbuf, PFSPut(cmd, pfs_name, &pfs_fd)), _pfs_cmd_put_path_missing_error_ ) == PFS_SUCCESS) ||
            (strcmp(strcpy(retbuf, PFSPut(cmd, pfs_name, &pfs_fd)), _pfs_cmd_put_invalid_path_error_ ) == PFS_SUCCESS) ){
           return retbuf;
        }   
     break; 

  }

   // Create linked list data to store the info of PFS files
   //makePFSArgs( &args, pfs_name, cmd_type, pfs_fd );
   //PFSOperations( args );                
  return retbuf;
}// end
