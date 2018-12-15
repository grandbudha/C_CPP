/*
 crafted by: grandbudha@gmail.com
 INFO: Very basic version of CRM in C language. 
*/




#define _GNU_SOURCE
 #include <stdlib.h>
 #include <stdio.h>
 #include <string.h>
 #include <errno.h>

 #ifndef BUFSIZ
 #define BUFSIZ 8192
 #endif

 #define SUCCESS 0
 #define FAILURE -1
 #define NULLCHAR '\0'

 #define NULLIFY(obj)  obj[SUCCESS] = NULLCHAR;



// ############################################################################ //
 /* Typedef for string abstract data type */
 typedef struct adt_string { char value[BUFSIZ]; } string; 

 /* Global variables to keep recrod for products */
 unsigned long long g_bottles;
 unsigned long long g_diapers;
 unsigned long long g_rattles;

 /* List to save purchase( here we should've used file, in case we need the system state back on startup ) */
 typedef struct purchase {
   char            *value;
   struct purchase *next;
 } List;

 /* List's global variable( not a good thing, but very flexible choice ) */
 List *g_head;

// **************************************************************************** //
 void printError(){

   fprintf(stderr, "Error: %s\n", strerror(errno));
   errno = SUCCESS;
}// end

// **************************************************************************** //
 void makeList(const string cmd){

   static List *tail;
   List        *node = NULL;

   // Allocate memory
   if( (node = malloc(sizeof(List))) == NULL ){
     printError();
     return;
   }
   else {

     if( (node->value = malloc((strlen(cmd.value) +1) *sizeof(char))) == NULL ){
       printError();
       free(node);
       return;
     }
     else {
       strcpy(node->value, cmd.value);
     }
   }

   // Make the list
   if( g_head == NULL ){
     g_head = node;
     tail   = node;
   }
   // Why loop, when we got static :)
   else {
      tail->next = node;
      tail       = node;
   }
}// end


// **************************************************************************** //
 int processInventory(const string cmd){

   // Type of products available inside the company
   const char         *is_bottles   = "Bottles ";
   const char         *is_diapers   = "Diapers ";
   const char         *is_rattles   = "Rattles ";
   char               *pointer      = NULL;
 
   // Parse the line
   pointer = (strchr(cmd.value, ' ') +1);

   // Check for the product type
   if( strncmp(pointer, is_bottles, strlen(is_bottles)) == SUCCESS ){

     pointer = (strchr(pointer, ' ') +1);
     g_bottles += (unsigned long long) strtol(pointer, NULL, 10);
   }
   else if( strncmp(pointer, is_diapers, strlen(is_diapers)) == SUCCESS ){

     pointer = (strchr(pointer, ' ') +1);
     g_diapers += (unsigned long long) strtol(pointer, NULL, 10); 
   }
   else if( strncmp(pointer, is_rattles, strlen(is_rattles)) == SUCCESS ){
     
     pointer = (strchr(pointer, ' ') +1);
     g_rattles += (unsigned long long) strtol(pointer, NULL, 10);
   }
   else {
     return FAILURE;
   }
   return SUCCESS;
}// end

// **************************************************************************** //
 int processPurchase(const string cmd){

   // Type of products available inside the company
   const char         *is_bottles   = "Bottles ";
   const char         *is_diapers   = "Diapers ";
   const char         *is_rattles   = "Rattles ";
   const char         *message      = "Not enough items to process purchase || ";
   char               *pointer      = NULL;
   unsigned long long value         = 0;

   // Parse the line
   pointer = (strchr(cmd.value, ' ') +1);

   // Check for the product type
   if( strncmp(pointer, is_bottles, strlen(is_bottles)) == SUCCESS ){

     if( g_bottles == SUCCESS ){
       printf("%s", message);
       return FAILURE;		
     }
     else {

       // Get the value
       value = (unsigned long long) strtol((strchr(pointer, ' ') +1), NULL, 10);        

       // Process purchase
       if( value < g_bottles ){

         // Make the list
         makeList(cmd);

         // Decrease product
         g_bottles -= value;
       }
       else {
         printf("%s", message);
         return FAILURE;
       }
     }
   }
   else if( strncmp(pointer, is_diapers, strlen(is_diapers)) == SUCCESS ){

     if( g_diapers == SUCCESS ){
       printf("%s", message);
       return FAILURE;
     }
     else {
 
       // Get the value
       value = (unsigned long long) strtol((strchr(pointer, ' ') +1), NULL, 10);        

       // Process purchase
       if( value < g_diapers ){

         // Make list
         makeList(cmd);

         // Decrease products
         g_diapers -= value; 
       }
       else {
         printf("%s", message);
         return FAILURE;
       }
     }
   }
   else if( strncmp(pointer, is_rattles, strlen(is_rattles)) == SUCCESS ){

     if( g_rattles == SUCCESS ){
       printf("%s", message);
       return FAILURE;
     }
     else {
 
       // Get the value
       value = (unsigned long long) strtol((strchr(pointer, ' ') +1), NULL, 10);        

       // Process purchase
       if( value < g_rattles ){

         // Make list
         makeList(cmd);

         // Decrease products
         g_rattles -= value;
       }
       else {
         printf("%s", message);
         return FAILURE;
       }
     }     
   }
   else {
     return FAILURE;
   }
   return SUCCESS;
}// end

// **************************************************************************** //
 void processSummarize(){
 
   List *iterator = g_head;

   printf( "\nPrinting List \n");
   if( iterator == NULL ){
     puts( " No items to print");
   }

   // Print the list
   while( iterator != NULL ){
     puts(iterator->value);
     iterator = iterator->next;
   }

   // Print the remaining items in the inventory
   printf("\nTotal items available\n"
          "-Bottles : %lld\n"
          "-Diapers : %lld\n"
          "-Rattles : %lld\n", g_bottles, g_diapers, g_rattles );
}// end 

// **************************************************************************** //
 int parseFile(const string crm_cmd){

   // Type of file commands
   const char *is_inventory = "Inventory ";
   const char *is_purchase  = "Purchase ";
   const char *is_summarize = "Summarize";

   // Is it inventory cmd
   if( strncmp(crm_cmd.value, is_inventory, sizeof(is_inventory)) == SUCCESS ){

     // Process inventory
     if( processInventory(crm_cmd) == FAILURE ) return FAILURE;
   }
   // Is it purchase cmd
   else if( strncmp(crm_cmd.value, is_purchase, sizeof(is_purchase)) == SUCCESS ){

     // Process purchase
     if( processPurchase(crm_cmd) == FAILURE ) return FAILURE;
   }
   // Is it summarize cmd
   else if( strcmp(crm_cmd.value, is_summarize) == SUCCESS ){

      // Process summarize
      processSummarize();
   } 
   else {
     return FAILURE;
   }
  return SUCCESS;
}// end


// **************************************************************************** //
 int checkFile(const string source){

   FILE       *file_pointer = NULL;
   const char *flag         = "r";  
   string     buffer;

   // Open up the file
   if( (file_pointer = fopen(source.value, flag)) == NULL ){
     return FAILURE;
   } 

   // Read file and process 
   while( fgets(buffer.value, BUFSIZ, file_pointer) != NULL ){

      // Remove the new line character
      strtok( buffer.value, "\n" );

      // Remove the last line if it's a new line/space character
      if( buffer.value[0] == '\n' || buffer.value[0] == ' ' ) continue;

      if( parseFile(buffer) == SUCCESS ){        
        printf("%s || PROCESSED\n", buffer.value);
      }
      else {
        printf("%s || NOT PROCESSED\n", buffer.value); 
      }

      // Nullify the object
      NULLIFY(buffer.value);
   }

   // Close the file
   fclose(file_pointer);
}// end

// **************************************************************************** //
 void cleanTheHeap(){

   while( g_head != NULL ){

     List *node = g_head;
     g_head     = node->next;

     // Free
     free(node->value); free(node);
   }

   // Last check, just for confirmation
   if(g_head != NULL ){

     if(g_head->value != NULL ) free(g_head->value);
     free(g_head);
   }
}// end

// **************************************************************************** //
 int readFile(string file_name){

   const char *is_exit  = "exit";
   string     *crm_file = NULL; 

   // Check if the file name is already available
   if( strlen(file_name.value) > SUCCESS ){

     // Check file name and get the all the values
     if( checkFile(file_name) != SUCCESS ){
       printError();
     }

     // Reset the file name for further processing
     NULLIFY(file_name.value);
   }
   // Ask for the file name
   else {

      printf(">> Enter the file name for CRM(type exit to quit): ");
      fgets(file_name.value, BUFSIZ, stdin);
      strtok(file_name.value, "\n" );

      // Check for the exit command
      if( strcmp(file_name.value, is_exit) == SUCCESS ){

        // Deallocate the already allocated resources
        if( g_head != NULL ){
          cleanTheHeap();
        }
        puts( "Bye.");
        exit(EXIT_SUCCESS);
      }
      // Use recurssion
      else {
        readFile(file_name);
      }
   }


}// end

// **************************************************************************** //


// ############################################################################ //
 int main(int argc, char *argv[]){

   string  buffer; 

   while(1) readFile(buffer);

  exit(0);
}// end

// Need to write an CRM system
// Company name: Ellie's Baby Emporium.
/*
 Need to make file reader and parser 

*/


/*
 Types of item in inventory: Bottles, Diapers, Rattles

 Need to read data from a file.
 4 commands -> Inventory <type> # -- "Inventory Bottles 50": means that 50 bottles should be added to the store's inventory.

*/

