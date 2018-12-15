#ifndef SCRAPPER_H
#define SCRAPPER_H

/*
 > Master has to start from CLI
 > It contains the arguments which will tell how many thread we have to create per process wise
 > File can also contain several commands
 > cat commands | ./piazza num_of_threads > output_file
 
 > ./piazza num_of_threads 
 < In the above case it has to possible to give commands at runtime.
 > Master process will schedule tasks one by one and dispatch them equally to the slave processes.
 > When all slaves are occupied then Master will create new slave.
 > Master must always dispatch work equally to all slave processes.
 > When information has been found, the main program must display results to user and make an entry inside    the log file. 
 > 

*/


class MasterProcess {

   public:




   private:


   protected:
};

#endif
