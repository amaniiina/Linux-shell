#include <stdio.h>
#include <stdlib.h>
#include <pwd.h>
#include <sys/types.h>
#include <unistd.h>
#include <assert.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/signal.h>
#include <fcntl.h>
#include <sys/stat.h>

#define MAX_LEN 510 //maximum length of the whole command
#define COMBI 0
#define NO_COMBI 1

double numOfCmnds=0;
double lenCmnds=0;
int combi= NO_COMBI;    //set flag for redirect function in case of pipe and redirection combination to default

int countWords(char* str);   //count number of words inputted
char** createArgv(char* str, int numOfWords);   // create array of words from given string
void checkCommands(char* str);  // check commands printed and call appropriate function
void createPipe(char** argv1, int numOfWords1, char** argv2, int numOfWords2);  // execute command with pipe
void redirect(char** argv1, int numOfWords1, char** argv2, int numOfWords2, char** argv3, int numOfWords3, int c);  // execute command with redirection
void oneCmd(char** argv, int numOfWords);   // execute only one command
int redirectionType(char* str, int i);      // return type of redirection 1= >  2= >>  3= <  4= 2>

int main(){

    uid_t userID = getuid();    //get user ID
    struct passwd *p= getpwuid(userID); //get username
    assert(p);

    char cwd[100];
    assert(getcwd(cwd, 100));   //get current directory

    while(1){
        numOfCmnds++;

        printf("%s@%s>", p->pw_name, cwd);
        char str[MAX_LEN];
        fgets(str, sizeof(str), stdin);

        lenCmnds+= strlen(str);

        char strCopy1[strlen(str)+1];    //create copy of the string to send to countWords function
        char strCopy2[strlen(str)+1];
        strncpy(strCopy1, str, strlen(str)+1);
        strncpy(strCopy2, str, strlen(str)+1);

        int numOfWords= countWords(strCopy1); //number of words inputted 

        char** argv=createArgv(strCopy2, numOfWords); // create arguments array

        if(strcmp(argv[0], "done")==0){ //check for done command 
            printf("Num of commands: %g\n", numOfCmnds);
            printf("Total length of all commands: %g\n", lenCmnds);
            printf("Average length of all commands: %f\n", lenCmnds/numOfCmnds);
            printf("See you next time!\n");

            for(int i=0; i<numOfWords; i++) //free memory before exiting
                free(*(argv+i));
            free(argv);

            exit(EXIT_SUCCESS);
        }
        checkCommands(str);     // check commands and execute them accordingly
               
    }
    return 0;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

int countWords(char* str){
    int count=0;
    char* word= strtok(str, "> \n\"");
    while(word!=NULL){
        count++;
        word= strtok(NULL,"> \n\"");
    }
    return count;
}

/////////////////////////////////////////////////////////////////////////

char** createArgv(char* str, int numOfWords){
    // allocate memory for the arguments array (+1 for NULL at the end of the array)
    char** argv=(char**) malloc(sizeof(char*) * numOfWords+1);
    assert(argv);

    char* word= strtok(str, "> \n\""); // use strtok function to seperate each word
    int i=0;
    while(word!=NULL){
        argv[i]= (char*)malloc(strlen(word)+1); //allocate memory for the word + '\0'
        assert(*(argv+i));
        strncpy(argv[i], word, strlen(word)+1);
        i++;
        word= strtok(NULL,"> \n\"");   // >, space, new line and "" are the tokenisers
    }
    argv[numOfWords]= NULL;   // NULL in last cell
    return argv;
}

/////////////////////////////////////////////////////////////////////////

void checkCommands(char* str){
    
    for(int i=0; i<strlen(str); i++){

        if(str[i]== '|' || str[i]== '>' || str[i]=='<' || str[i]=='2'){

            //split str into two substrings
            int str1len= i*sizeof(char);        // from beginning of string till before | or < or > or 2
            char str1[str1len+1];
            memset(str1, '\0', sizeof(str1));
            strncpy(str1, str, str1len);

            int str2len=(strlen(str)+1)-(i*sizeof(char));   //after | or redirection char till end of string
            char str2[str2len+1];
            memset(str2, '\0', sizeof(str2));
            strncpy(str2, str+i+1,str2len+1);

            //copy substring 1 twice for countWords and createArgv
            char str1Copy[strlen(str1)+1];
            memset(str1Copy, '\0', sizeof(str1Copy));   //make sure memory is clean
            strncpy(str1Copy, str1, strlen(str1)+1);    //copy substring 1 and count words
            int numOfWords1= countWords(str1Copy);

            memset(str1Copy, '\0', sizeof(str1Copy));   //make sure memory is clean
            strncpy(str1Copy, str1, strlen(str1)+1);    //copy substring 1 and create arguments array
            char** argv1= createArgv(str1Copy, numOfWords1);

            //copy substring 2 twice for countWords and createArgv
            char str2Copy[strlen(str2)+1];
            memset(str2Copy, '\0', sizeof(str2Copy));
            strncpy(str2Copy, str2, strlen(str2)+1);
            int numOfWords2= countWords(str2Copy);

            memset(str2Copy, '\0', sizeof(str2Copy));
            strncpy(str2Copy, str2, strlen(str2)+1);
            char** argv2= createArgv(str2Copy, numOfWords2);
        
            if(str[i]== '|' ){

                for(int j=i; j<strlen(str); j++){   // if pipe is found check for redirection after it

                    if(str[j]== '>' || str[j]=='<' || str[j]=='2'){
                        combi = COMBI;  // flag for redirect function in case of pipe and redirection combination

                        int str3len=(strlen(str)+1)-(j*sizeof(char)) ; //after redirection char till the end of string

                        str2len= (strlen(str2)+1)-str3len;  //after pipe char till before redirection char
                        char str22[str2len+1];
                        memset(str22, '\0', sizeof(str22));
                        strncpy(str22, str2, str2len);
                        str22[str2len+1]= '\0';

                        //copy substring 2 twice for countWords and createArgv
                        char str2Copy[strlen(str22)+1];
                        memset(str2Copy, '\0', sizeof(str2Copy));
                        strncpy(str2Copy, str22, strlen(str22)+1);
                        int numOfWords2= countWords(str2Copy);

                        memset(str2Copy, '\0', sizeof(str2Copy));
                        strncpy(str2Copy, str22, strlen(str22)+1);
                        char** argv2= createArgv(str2Copy, numOfWords2);

                        char str3[str3len+1];
                        memset(str3, '\0', sizeof(str3));
                        strncpy(str3, str+j+1, str2len+1);

                        //copy substring 3 twice for countWords and createArgv
                        char str3Copy[strlen(str3)+1];
                        memset(str3Copy, '\0', sizeof(str3Copy));
                        strncpy(str3Copy, str3, strlen(str3)+1);
                        int numOfWords3= countWords(str3Copy);

                        memset(str3Copy, '\0', sizeof(str3Copy));
                        strncpy(str3Copy, str3, strlen(str3)+1);
                        char** argv3= createArgv(str3Copy, numOfWords3);

                        // call redirect function with 3 arrays
                        redirect(argv1, numOfWords1, argv2, numOfWords2, argv3, numOfWords3,redirectionType(str, j));

                        combi= NO_COMBI;    //reset flag to default
                        return; 
                    }
                }
                createPipe(argv1, numOfWords1, argv2, numOfWords2);
                return;  
            }
            else{
                // call rediret function with 2 arrays with NULL and 0 inplace of the third
                // since second array in parameters is not used in this case
                redirect(argv1, numOfWords1, NULL, 0, argv2, numOfWords2, redirectionType(str, i));
                return;
            } 
        }
    }
    //single command with no pipe or redirection
    char strCopy1[strlen(str)+1];
    strncpy(strCopy1, str, strlen(str)+1);

    int numOfWords= countWords(strCopy1);
    char ** argv= createArgv(str, numOfWords);
            
    oneCmd(argv, numOfWords);
}

///////////////////////////////////////////////////////////////////////////

void oneCmd(char** argv, int numOfWords){ 
    pid_t pid= fork();  //create child process
    if(pid<0){
        perror("fork failed");
    }

    else if(pid==0){ //child process
        if(strcmp(argv[0], "cd")==0){  // print and terminate child process
            printf("Command not supported yet.\n");
            exit(0);        
        }
        else if(execvp(argv[0], argv) == -1){ //execute command
            perror("Execution failed\n");
            exit(EXIT_FAILURE);
        }
    }
        
    else{ //parent process
        wait(NULL); // wait for child process to finish
        for(int i=0; i<numOfWords; i++) // free memory after each command
            free(*(argv+i));
        free(argv);
    }
}
///////////////////////////////////////////////////////////////////////////

int redirectionType(char* str, int i){
    if( i<strlen(str)-1 ){
        if(str[i]=='>' && str[i+1]=='>')
            return 2;
        else if(str[i]=='2' && str[i+1]=='>')
            return 4;
    }
    if(str[i]== '>')
        return 1;
    if(str[i]=='<')
        return 3;
}

///////////////////////////////////////////////////////////////////////////
void redirect(char** argv1, int numOfWords1, char** argv2, int numOfWords2,char** argv3, int numOfWords3, int c){
    pid_t pid= fork();
    if(pid<0){
        perror("Fork failed\n");
    }
    int fd;
    if(pid ==0){    // in child 
        switch(c){
            case 1:    // >
                fd= open(argv3[0], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG |S_IRWXO); // open file
                if(fd == -1){
                    perror("file couldn't open\n");
                    exit(EXIT_FAILURE);
                }
                if(dup2(fd, STDOUT_FILENO) == -1){  //redirect output to file opened
                    perror("Dup2 error\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 2:   // >>
                fd= open(argv3[0],O_RDWR | O_CREAT | O_APPEND, S_IRWXU | S_IRWXG |S_IRWXO); // open file
                if(fd == -1){
                    perror("file couldn't open\n");
                    exit(EXIT_FAILURE);
                }
                if(dup2(fd, STDOUT_FILENO) == -1){  //redirect output to file opened
                    perror("Dup2 error\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 3:     // <
                fd= open(argv3[0],O_RDWR);  // open file
                if(fd == -1){
                    perror("file couldn't open\n");     
                    exit(EXIT_FAILURE);
                }
                if(dup2(fd, STDIN_FILENO) == -1){   // redirect input to file opened
                    perror("Dup2 error\n");
                    exit(EXIT_FAILURE);
                }
                break;
            case 4:    // 2>
                fd= open(argv3[0], O_RDWR | O_CREAT | O_TRUNC, S_IRWXU | S_IRWXG |S_IRWXO); // open file
                if(fd == -1){
                    perror("file couldn't open\n");
                    exit(EXIT_FAILURE);
                }
                if( dup2(fd, STDERR_FILENO) == -1 ){    //redirect output to file opened
                    perror("Dup2 error\n");
                    exit(EXIT_FAILURE);
                }            
                break;
        }
        if(combi == COMBI && c != 3){   //create pipe in case of combination
            createPipe(argv1, numOfWords1, argv2, numOfWords2);
            close(fd);
            exit(EXIT_SUCCESS);
        }
        close(fd);
        if(execvp(argv1[0], argv1) == -1){  // execute command
            perror("Execution failed in redirection\n");
            exit(EXIT_FAILURE);
        }
        
        exit(EXIT_SUCCESS);
    }
    // in parent
    else{
        wait(NULL);     // wait for child process to finish
        int i;
        for(i=0; i<numOfWords1; i++) // free all memory after each command
            free(argv1[i]);
        free(argv1);
        for(i=0; i<numOfWords2; i++) 
            free(argv2[i]);
        free(argv2);
        for(i=0; i<numOfWords3; i++) 
            free(argv3[i]);
        free(argv3);
    }
}

///////////////////////////////////////////////////////////////////////////
void createPipe(char** argv1, int numOfWords1, char** argv2, int numOfWords2){

    //creating pipe
    int pfd[2];
    if(pipe(pfd)==-1){
        perror("Pipe failed\n");
        exit(EXIT_FAILURE);
    }

    //create 1st child process
    pid_t pid= fork();
    if(pid<0){
        perror("Fork failed\n");
        exit(EXIT_FAILURE);
    }
    if(pid==0){ // in 1st child process
        close(pfd[0]);      // close input cell in pipe
        if( dup2(pfd[1], STDOUT_FILENO) == -1){ // redirect output to pipe through cell 1
            perror("Dup2 error\n");
            exit(EXIT_FAILURE);   
        }
        if(execvp(argv1[0], argv1)==-1){        // execute first command
            perror("Execution failed in child 1\n");
            exit(EXIT_FAILURE);
        }
        close(pfd[1]);      // close output cell in pipe
        exit(EXIT_SUCCESS);
    }

    // create 2nd child process
    pid_t pid2= fork(); 
    if(pid2<0){
        perror("Fork failed");
        exit(EXIT_FAILURE);
    }
    if(pid2==0){ // in 2nd child process
        close(pfd[1]);      // close output cell in pipe
        if(dup2(pfd[0], STDIN_FILENO) == -1){   // redirect input from pipe through cell 0
            perror("Dup2 error\n");
            exit(EXIT_FAILURE);
        }
        if(execvp(argv2[0], argv2) == -1){      // execute second command
            perror("Execution failed in child 2\n");
            exit(EXIT_FAILURE);
        }
        close(pfd[0]);      // close input cell in pipe
        exit(EXIT_SUCCESS);
    }
    // in parent
    close(pfd[0]);  // close both cells of pipe
    close(pfd[1]);
    wait(NULL); // wait for child process 1 to finish
    wait(NULL); // wait for child process 2 to finish
    for(int i=0; i<numOfWords1; i++) // free memory after each command
        free(*(argv1+i));
    free(argv1);
    for(int i=0; i<numOfWords2; i++)
        free(*(argv2+i));
    free(argv2);
}