#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>


#define __FILENAME__ (strrchr( __FILE__, '/') ? strrchr( __FILE__, '/' ) +1 : __FILE__ ) 


#define  PFS_LOGGER_PATH  "./PFSLog/pfslogger.pfs"
#define  PFS_FAILURE  -1
#define  PFS_SUCCESS   0
#define  PFSLEN        1000
#define  BASE_TEN_NUM  10
#define  ONE           1
#define  ZERO          0
#define  LOOP_INFINITE 1
#define  ADD_TERMINATOR 1
#define  RESET_MEMSET  0
#define  MAX_DISK_BLOCK_ROW 39
#define  MAX_DISK_BLOCK_COL 257


#define  PFS_NEW_FILE_FLAGS    (O_CREAT | O_EXCL | O_RDWR | O_APPEND)
#define  PFS_NEW_FILE_PERMS    (S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH) 
#define  PFS_OLD_FILE_FLAGS    (O_RDWR | O_APPEND)

#define  PFS_LOG_FIFO_TEMPLATE "/tmp/pfslogger_namedfifo"
#define  PFS_DIR_PATH          "./PFSDIR/"


/* Static Constants */
static const char *_file_open_error_msg_ = "Error in opening the file";
static const char *_file_write_error_msg_ = "Error in writing inside the file";
static const char *_process_start_error_ = "Error in starting the processes for PFS";
static const char *_mkfifo_error_ = "Error in opening the mkfifo pipe";
static const char *_mkfifo_open_read_error_ = "Error in opening the FIFO file for read purposes";
static const char *_mkfifo_open_write_error_ = "Error in opening the FIFO file for write purposes"; 
static const char *_mkfifo_read_error_ = "Error in reading the message from FIFO";
static const char *_mkfifo_write_error_ = "Error in writing the message inside FIFO";
static const char *_cli_invalid_cmd_ = "Invalid command for PFS";
static const char *_first_cmd_error_ = "Open command has to be run first.";
static const char *_first_cmd_again_error_ = "Open command is only acceptable at the start.";
static const char *_pfs_cmd_put_path_missing_error_ = "Put commands requires file path/name."; 
static const char *_pfs_cmd_put_invalid_path_error_ = "Path/name is invalid or does not exist.";

static const char *_pfs_invalid_pfs_file_name_ = "Invalid PFS file name.";
static const char *_pfs_no_error_msg_ = "NULL";
static const char *_pfs_malloc_error_ = "Error in allocating memory." ;

static const char *_pfs_file_open_success_ = "PFS file opened successfully.";

/* Flag to exit all the PFS processes */
static int IS_EXIT_PFS_LOGGER = 1;  
static int IS_EXIT_PFS_CLI = 1;

/* Enum for constants */
enum { 
  PFS_LOG_CREATE_NEW = 0,
  PFS_LOG_OPEN_OLD   = 1,
  PFS_LOG_OPEN_FIFO_READ = 2,
  PFS_LOG_OPEN_FIFO_WRITE = 3,

  PFS_INPUT_CMD = 0,
  
  PFS_OPEN_CMD  = 1,
  IS_PFS_OPEN_CMD_ALREADY_RUN = 0,  

  PFS_PUT_CMD = 2,
  PFS_GET_CMD = 3,
  PFS_RM_CMD = 4,
  PFS_DIR_CMD = 5,
  PFS_PUTR_CMD = 6,
  PFS_KILL_CMD = 7,
  PFS_QUIT_CMD = 8,
};


/* Struct for all the thread related data */
typedef struct {

   pthread_t          pfs_logger_tid;
   pthread_mutex_t    pfs_logger_mutex;
   pthread_t          pfs_cli_tid;

} PFSThreadInfo;

/* Struct for pfsoperation argument */
struct pfsargs{

   char *file_name;
   int  cmd_type;
   int  file_fd;
};


static PFSThreadInfo   PFSThread;


/* INTERFACE */

void printError( const int, const char *, const char * ); 

/* Interface for PFSLogger */
void *T_writeInPFSLog( void * );
//void writeInPFSLog( const int, const char *, const char * );

/* Interfase for PFSCLI */
void *T_PFSCli(void *);

/* Interface for starting all the PFS Processes */
void startPFSProcesses();

/* Interface for executing PFS commands */
char *executePFSCmd(char *,const int);

/* Interface to write inside the logger */
void makeLogMsg( const int , const char *);

/* Handle PFS operation */
//void PFSOperations(struct pfsargs );
