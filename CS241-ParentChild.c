/*************************************************
 * Id: oste1799
 * Purpose:
 *  To introduce the fork() and execvp() commands to help create
 *  an understanding of parent and child processes.
 * Input:
 *  User enters unix commands.
 * Output:
 *  Is dependent on user input.
 * Compile:  gcc -g -Wall  -o hhshell hhshell.c
 *************************************************************/
//libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#define MAX_WORDS 20
#define MAX_CMD 512
#define MAX_BG 5      //NEW***
#define INVALID_PID 0   //NEW***
//Making our own data type called bgjob_t that contains a
//a process id and the command the user entered
typedef struct {
   int pid; //if pid  == INVALID_PID this is a free slot
   char command[MAX_CMD];   //The command the user entered
} bgjob_t;      //NEW*****
//function Prototypes
int asArrayOfSubstrings(char *cmd, char **argv);
void executeInFork(char **);
void execute(char *command_array[]);
void helpUser();
void bgClean(bgjob_t bglist[]); //pass the actual list of type bgjob_t
void bgFunc(bgjob_t bglist[], char *command_array[], char *cmd);        //pass the command as array of type bgjob_t
void bgListFunc(bgjob_t bglist[]);
void bgKillFunc(bgjob_t bglist[], char * cmdAsArray[]);
int main (void)
{
   char cmd[MAX_CMD];
   char *cmdAsArray[MAX_WORDS];
   bgjob_t bglist[MAX_BG];   //Creating array of max 5 elements that are type bgjob_t
   for (int i =0; i < 5; i++){
      bglist[i].pid = INVALID_PID;   //sets all process ID's to invalid
   }
   int args = 0;
   printf("Created By: Hunter Hawkins\n");
   printf("Type \"help\" to see implemented commands\n");
   for(;;) 
   { //Loop forever until user enters "exit"
      printf("hhshell> "); // Print prompt
      fgets(cmd, MAX_CMD, stdin);
      if(cmd == NULL)
      {
         printf("Unrecognized Command\n");
      }
      args = asArrayOfSubstrings(cmd, cmdAsArray); // Break the command into words
      cmdAsArray[args] = '\0';
      //the blank case
      if(cmdAsArray[0] == '\0')
      {
         //do nothing
      }
      //If user desires to exit shell
      else if(strcmp(cmdAsArray[0],"exit") == 0)
      {
         exit(0);
      }
      //if user needs help with shell
      else if(strcmp(cmdAsArray[0],"help") == 0)
      {
         helpUser();   //Function to help user
      }
      else if (strcmp(cmdAsArray[0],"bg") ==0)
      {
         bgFunc(bglist,&(cmdAsArray[1]),&(cmd[3]));   //pass list and command
      }
      else if (strcmp(cmdAsArray[0],"bglist") ==0)
      {
         bgListFunc(bglist);   //pass list to bgListFunc
      }
      else if (strcmp(cmdAsArray[0],"bgkill") ==0)
      {
         bgKillFunc(bglist, cmdAsArray);   //Pass list to bgKillFunc
      }
      else
      {
         executeInFork(cmdAsArray);     //pass to execute function
      }
   }
}
// Purpose: To move a user entered command into the background
// Inp: The list as an array, and the user entered command
// Out: None to the console, but move a process to the background
// Notes: Catch SIGCHLD and in the the handler, call wait.
// currently the same as exInFork but without while loop
void bgFunc(bgjob_t bglist[], char *cmdAsArray[], char *cmd){
   bgClean(bglist);
   int pid, empSlot;
   empSlot = -1;
   //looking for empty slot
   for (int i =0; i < MAX_BG; i++)
   {
      if (bglist[i].pid == INVALID_PID)
      {
         empSlot =i;
      }
   }
   printf("Slot is %d\n", empSlot);
   //No empty slot found
   if (empSlot == -1)
   {
      printf ("Error, no empty slots avalable\n");
      return;
   }
   pid = fork();
   if(pid < 0)
   {   //Fault in fork creation
      printf("Fork failed\n");
      exit(1);
   }
   if(pid== 0)
   { // This is the child process.
      //pipe the output to dev null
      //function called yes()
      int stdin_fd = open("/dev/null", O_WRONLY);
      if (stdin_fd == -1)
      _exit(127);
      dup2(stdin_fd, STDOUT_FILENO);
      close(stdin_fd);
      execvp(*cmdAsArray, cmdAsArray);
      _exit(127);
   }
   //
   else
   {
      printf("adding pid %d %s\n", pid, cmd);
      bglist[empSlot].pid = pid; // store pid
      strcpy(bglist[empSlot].command, cmd);     //copies command to parent process
   }
}
// Purpose: To list all the background processes
// Inp: The list as an array
// Out: To print out all the background processes
void bgListFunc(bgjob_t bglist[])
{
   int count=0;
   bgClean(bglist);
   for (int i=0; i < MAX_BG; i++)
   {
      if (bglist[i].pid != INVALID_PID)
      {
         printf("  %d. %s",i+1, bglist[i].command);
         count++;
      }
   }
   printf("There are %d processes\n",count);
}
// Purpose: Kill al the background processes
// Inp: The list as an array
// Out: No background processes still running
// Notes: The kill function sens a signal to a process
// Using 0 so should kill every process in the group */
void bgKillFunc(bgjob_t bglist[],char * cmdAsArray[])
{
   //need for loop to kill background processes
   //for (int j=0; j < 5; j++)
   int i = atoi(cmdAsArray[1]);
   bgClean(bglist);
   if (bglist[i-1].pid == INVALID_PID)
   {
      printf("Error, %d does not exist\n", i);
   }
   else
   {
      kill(bglist[i-1].pid, SIGTERM);
   }
   //clean up after
   bgClean(bglist);
}
// Purpose: Function to set all background processes
// to invalid PID, essentially "cleaning them"
// Inp: The list as an Array with data type bgjob_t
// Out: None, other then setting all elements to invalid
void bgClean(bgjob_t bglist[]){
   int status;
   for(int i =0; i < 5; i++){
      if(bglist[i].pid != INVALID_PID){
         if(waitpid(bglist[i].pid, &status, WNOHANG) !=0){
            bglist[i].pid = INVALID_PID;
         }
      }
   }
}
// Purpose: Function to create child processes      
// Inp: char *cmd is the original string            
// Outp: Child processes                            
void executeInFork(char **cmd)
{
   int pid, status;
   pid = fork();
   if(pid < 0)
   {   //Fault in fork creation
      printf("Fork failed\n");
      exit(1);
   }
   if(pid== 0)
   { // This is the child process.
      if(execvp(*cmd, cmd))
      {    //built in linux commands
         printf("error\n");
      }
      exit(0);  //exit when done
   }
   else
   {
      while(wait(&status) != pid)
      {
         //parent process
      }
   }
}
// Purpose:  Help function incase user needs help         
// Inp: None                                              
// Outp: Help menu                                        
void helpUser(){
   printf("pwd - Print the current working directory \n");
   printf("exit - Exit shell\n");
   printf("help - Display this message\n\n");
   printf("bglist - List background programs\n");
   printf("bgkill <int> - Kill background process <int> int values: 1-5\n");
   printf("<UNIX cmd> - Spawn child process, execute <UNIX cmd> in the foreground\n");
   printf("bg <UNIX cmd> - Spawn child process, execute <UNIX cmd> in the background\n");
}
/* Given by Dr. BC         */
void execute(char *command_array[]){
   if(execvp(*command_array, command_array) < 0) 
   {
      printf ("Execution of  supplied command failed\n");
      exit(1);
   }
}
// PROC: asArrayOfSubstrings                                     
// Inp:  cmd: The original string                                
// Outp: argv: The array                                         
//       int: The nubmer of arguments                            
// Desc: Assignment: Part 1                                      
//       Tokeniszes cmd into an array of substings argv on space 
int asArrayOfSubstrings(char *cmd, char **argv) {
   char* token = NULL;
   int argC = 0;
   char cmdCopy[MAX_CMD];
   strcpy(cmdCopy, cmd);
   token = strtok(cmdCopy, " ");
   while(token) 
   { // tokenize cmp into the array argv
      argv[argC] = (char *)malloc(sizeof(char) * (strlen(token) + 1));
      if(token[strlen(token)-1]== '\n')
      {
         token[strlen(token)-1] = 0; // NULL terminate the string
      }
      strncpy(argv[argC], token, strlen(token) + 1);
      token = strtok(NULL, " ");
      argC++;
   }
   return argC;
}