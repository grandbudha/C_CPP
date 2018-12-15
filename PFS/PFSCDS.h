/* BELOW IS FOR PFS DISK BLOCK WHICH USES CONTIGUOUS DISK ALLOCATION */
#include "PFSHeader.h"

/*
 PUR: To store the file info inside PFS directory.
*/
typedef struct file_info {

   char *file_name;                                           // Stores the file name.
   int  s_pfs_index;                                          // Stores the starting index inside Disk Block.
   int  e_pfs_index;                                          // Stores the last index of file inside the Disk Block.
   int  h_pfs_index;                                          // Stores the info if data has stored in numerous files.
   struct file_info *next;                                    // Pointer to save next possible node.
} FileInfo;

/*
 PUR: Disk block info for per block of size 257bytes.
*/
typedef struct disk_block_info {

    int c_db_index;                                           // Current disk block index.
//    int f_db_index;                                           // Final disk block index = 257.
//    int r_db_index;                                           // Remaining disk block index(f_db_index - c_db_index).
    FileInfo *head;                                           // Stores the next possible file info for current Disk Block.
}DiskBlock;

/*
 PUR: Stores the info related to PFS disk block and file inside it data.
*/
typedef struct pfs_file_info {

    int                  c_db_array_index;                    // Stores the current disk block array index.                                    
    DiskBlock            diskblock[MAX_DISK_BLOCK_ROW];       // Total number of disk blocks inside the one file =>39*257 = 10023 bytes.
    char                 *pfs_cur_file_name;                  // Current file name of the PFS.
    int                  pfs_cur_file_fd;                     // Current file descriptor of the PFS current file.
    struct pfs_file_info *next; 
}PFSFile;
