#include "PFSCDS.h"



// ---------------------------------------------------------------------------- //
/*
*/
FileInfo *allocateFileInfo(char *file_name){

   FileInfo *temp = NULL;

   if( (temp = malloc(ONE*sizeof(FileInfo))) == NULL ){
      return NULL;
   }
   else {
     temp->file_name = file_name;
     temp->e_pfs_index = temp->s_pfs_index = ZERO;
     temp->h_pfs_index = ONE;
     return temp;
   }
}// end


// ---------------------------------------------------------------------------- //
/*

*/
PFSFile *allocatePFSFile( char *file_name, int fd ){

   PFSFile    *temp = NULL;
   static int pfs_file_seq;
   char       buf[PFSLEN] ={0};
   const char *pfs = "PFS";

   if( (temp = malloc(ONE*sizeof(PFSFile))) == NULL ){
      //makeLogMsg( __LINE__, _pfs_malloc_error_ );
      return NULL;
   }
   else {

      if( file_name != NULL ){
         temp->pfs_cur_file_name = file_name;
         temp->pfs_cur_file_fd = fd; 
         temp->c_db_array_index = ZERO;

         // Get the next possible sequence
         pfs_file_seq = (int) strtol( file_name, NULL, BASE_TEN_NUM );
         ++pfs_file_seq; 
      }
      else {

         snprintf( buf, PFSLEN, "%s%s%d", PFS_DIR_PATH, pfs, pfs_file_seq++ );
     
         // Allocate memory for the PFS file
         if( (temp->pfs_cur_file_name = malloc( (strlen(buf)+ONE)*sizeof(char))) == NULL ){
            // error
            return NULL;
         }  
         else {

            strcpy( temp->pfs_cur_file_name, buf );
            temp->c_db_array_index = ZERO;
            // Open up the file
            if( (temp->pfs_cur_file_fd = open( buf, (int)PFS_NEW_FILE_FLAGS, (mode_t)PFS_NEW_FILE_PERMS )) == PFS_FAILURE ){
               // error
               return NULL;
            } 
         }
      }
      return temp;
   } 
}// end

// ---------------------------------------------------------------------------- //
/*
*/
void operationSave( struct pfsargs arg, PFSFile **info ){

   off_t    file_size = 0;
   FileInfo *node     = NULL;
   char     file_char = 0;
   int      ret_value = 0;
   PFSFile  *pfsfile_node = NULL;
   int      is_started = 0;

   // Allocate the file node
   if( (node = allocateFileInfo(arg.file_name)) == NULL ){
      puts("Err: C, 90"); exit(0); 
      return;
   }

   // Read the file character wise
   while( (ret_value = read( arg.file_fd, &file_char, ONE)) ){

      if( ret_value > PFS_SUCCESS ){

         // Write inside the file
         if( write( (*info)->pfs_cur_file_fd, &file_char, ONE) == PFS_FAILURE ){
            puts("101, C"); exit(0); 
         }
         // Save the start index of the file inside PFS
         else if( is_started == ZERO ){
            node->s_pfs_index = (*info)->diskblock[(*info)->c_db_array_index].c_db_index;
            is_started = ONE;
         }    
     
         // Increase the c_db_index
         (*info)->diskblock[(*info)->c_db_array_index].c_db_index += ONE;

         // Check for the index value
         if( (*info)->diskblock[(*info)->c_db_array_index].c_db_index >= MAX_DISK_BLOCK_COL ){

            // Increase the array index
            if( (*info)->c_db_array_index < MAX_DISK_BLOCK_ROW ){
               (*info)->c_db_array_index += ONE;
            } 
            else {

               // Allocate the node 
               if( (pfsfile_node = allocatePFSFile( NULL, ZERO )) == NULL ){
                   puts( "114, C" ); exit(0);
                   return;
               } 
               else {
                  (*info)->next = pfsfile_node;
                  (*info) = pfsfile_node;
                  node->h_pfs_index += ONE;
               }
            }
         } 
      }
      else {

         if( ret_value == PFS_SUCCESS ){
 
            if( is_started == ONE ){
               node->e_pfs_index = (*info)->diskblock[(*info)->c_db_array_index].c_db_index;
            }
            break;
         }
         else if( ret_value == PFS_FAILURE ){
           puts( "101, C");
           return;
         }
      }
   }

   // Add up the node
   if( (*info)->diskblock[(*info)->c_db_array_index].head == NULL ){
      (*info)->diskblock[(*info)->c_db_array_index].head = node;
   }
   else {

     // Find the next possible node
     FileInfo *curr = (*info)->diskblock[(*info)->c_db_array_index].head;
     FileInfo *prev = NULL;

     while( curr != NULL ){
       prev = curr;
       curr = curr->next;
     }
     prev->next = node;
   }
}// end

// ---------------------------------------------------------------------------- //
/*

*/
void PFSOperations(struct pfsargs args){

   static PFSFile  *pfsfile_head_node;
   static PFSFile  *pfsfile_current_node;
   PFSFile         *pfsfile_temp_node = NULL;
   int             pfsfile_iterator = 0;

   // Allocate node
   if( pfsfile_head_node == NULL ){   

       if( (pfsfile_temp_node = allocatePFSFile(args.file_name, args.file_fd)) != NULL ){
          pfsfile_head_node = pfsfile_current_node = pfsfile_temp_node;
       } 
       else {
          return;
       }
   }
   else {

     if( args.cmd_type == PFS_PUT_CMD ){
         operationSave( args, &(pfsfile_current_node) ); 
      } 
   }
}// end

/* ******** */
int main(){

  struct pfsargs temp;
  struct pfsargs temp2;
  struct pfsargs temp3;
  struct pfsargs temp4;
  struct pfsargs temp5;
  struct pfsargs temp6;
 
  temp.file_name = malloc( (strlen("TEMP") + 1)*sizeof(char) );
  strcpy(temp.file_name, "TEMP");
  temp.file_fd = open( temp.file_name, O_RDONLY );
  temp.cmd_type = PFS_PUT_CMD;

  temp6.file_name = malloc( (strlen("TEMP") + 1)*sizeof(char) );
  strcpy(temp6.file_name, "TEMP");
  temp6.file_fd = open( temp6.file_name, O_RDONLY );
  temp6.cmd_type = PFS_PUT_CMD;

  temp3.file_name = malloc( (strlen("TEMP") + 1)*sizeof(char) );
  strcpy(temp3.file_name, "TEMP");
  temp3.file_fd = open( temp3.file_name, O_RDONLY );
  temp3.cmd_type = PFS_PUT_CMD;

  temp4.file_name = malloc( (strlen("TEMP") + 1)*sizeof(char) );
  strcpy(temp4.file_name, "TEMP");
  temp4.file_fd = open( temp4.file_name, O_RDONLY );
  temp4.cmd_type = PFS_PUT_CMD;

  temp5.file_name = malloc( (strlen("TEMP") + 1)*sizeof(char) );
  strcpy(temp5.file_name, "TEMP");
  temp5.file_fd = open( temp5.file_name, O_RDONLY );
  temp5.cmd_type = PFS_PUT_CMD;


  ///int i = 0;
  //for( i = 0; i < 8; ++i )

  temp2.file_name = malloc( (strlen("./PFSDIR/PFS") +1)*sizeof(char) );
  strcpy( temp2.file_name, "./PFSDIR/PFS" );
  temp2.file_fd = open( temp2.file_name, (int)PFS_NEW_FILE_FLAGS, (mode_t)PFS_NEW_FILE_PERMS ); 

    PFSOperations( temp2 );
    PFSOperations( temp );
     PFSOperations( temp3 );
     PFSOperations( temp4 );
     PFSOperations( temp5 );
     PFSOperations( temp6 );

}// end

