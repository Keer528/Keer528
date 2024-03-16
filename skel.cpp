#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

using namespace std;

/* The pipe for parent-to-child communications */
int parentToChildPipe[2];

/* The pipe for the child-to-parent communication */
int childToParentPipe[2];

/* The read end of the pipe */
#define READ_END 0

/* The write end of the pipe */
#define WRITE_END 1

/* The maximum size of the array of hash programs */
#define HASH_PROG_ARRAY_SIZE 6

/* The maximum length of the hash value */
#define HASH_VALUE_LENGTH 1000

/* The maximum length of the file name */
#define MAX_FILE_NAME_LENGTH 1000

/* The array of names of hash programs */
const string hashProgs[] = {"md5sum", "sha1sum", "sha224sum", "sha256sum", "sha384sum", "sha512sum"};

string fileName;

/**
 * The function called by a child
 * @param hashProgName - the name of the hash program
 */
void computeHash(const string& hashProgName)
{
    /* The hash value buffer */
    char hashValue[HASH_VALUE_LENGTH];

    /* Reset the value buffer */
    memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

    /* The received file name string */
    char fileNameRecv[MAX_FILE_NAME_LENGTH];

    /* Fill the buffer with 0's */
    memset(fileNameRecv, (char)NULL, MAX_FILE_NAME_LENGTH);

    /** Read a message from the parent **/
    read(parentToChildPipe[READ_END], fileNameRecv, MAX_FILE_NAME_LENGTH);

    /* Glue together a command line <PROGRAM NAME>. 
    * For example, sha512sum fileName.
    */
    string cmdLine(hashProgName);
    cmdLine += " ";
    cmdLine += fileNameRecv;

    /* Open the pipe to the program (specified in cmdLine) 
    * using popen() and save the output into hashValue.
    */
    FILE* hashPipe = popen(cmdLine.c_str(), "r");
    if (hashPipe == NULL) {
        perror("popen");
        exit(EXIT_FAILURE);
    }

    fgets(hashValue, HASH_VALUE_LENGTH, hashPipe);

    pclose(hashPipe);

    /* Send the hash value to the parent */
    ssize_t bytesSent = write(childToParentPipe[WRITE_END], hashValue, strlen(hashValue) + 1);
    if (bytesSent == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Print the sent message */
    cout << "Sent hash value to parent process: " << hashValue << endl;

    /* The child terminates */
    exit(0);
}

void parentFunc(const string& hashProgName)
{
    /* Close the unused ends of two pipes */
    close(parentToChildPipe[READ_END]);
    close(childToParentPipe[WRITE_END]);

    /* Send the file name to the child */
    ssize_t bytesSent = write(parentToChildPipe[WRITE_END], fileName.c_str(), fileName.size() + 1);
    if (bytesSent == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }

    /* Print the sent message */
    cout << "Sent file name to child process: " << fileName << endl;

    /* The buffer to hold the string received from the child */
    char hashValue[HASH_VALUE_LENGTH];

    /* Reset the hash buffer */
    memset(hashValue, (char)NULL, HASH_VALUE_LENGTH);

    /* Read the hash value sent by the child */
    ssize_t bytesRead = read(childToParentPipe[READ_END], hashValue, HASH_VALUE_LENGTH);
    if (bytesRead == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
     /* Close the pipes after finishing communication */
    close(parentToChildPipe[WRITE_END]);
    close(childToParentPipe[READ_END]);
    /* Print the received message */
    cout << "Received hash value from child process: " << hashValue << endl;
}

int main(int argc, char** argv)
{
    /* Check for errors */
    if(argc < 2)
    {
        cerr << "USAGE: " << argv[0] << " <FILE NAME>" << endl;
        exit(-1);
    }   
    
    /* Save the name of the file */
    fileName = argv[1];
    
    /* Run a program for each type of hashing algorithm hash algorithm */    
    for (int hashAlgNum = 0; hashAlgNum < HASH_PROG_ARRAY_SIZE; ++hashAlgNum)
    {
        /* Create pipes */
        if (pipe(parentToChildPipe) == -1 || pipe(childToParentPipe) == -1) {
            perror("pipe");
            exit(EXIT_FAILURE);
        }

        /* Fork a child process and save the id */
        pid_t pid = fork();
        if (pid < 0)
        {
            perror("fork");
            exit(-1);
        }
        /* I am a child */
        else if (pid == 0)
        {
            /* Close the unused ends of two pipes */
            close(parentToChildPipe[WRITE_END]);
            close(childToParentPipe[READ_END]);

            /* Compute the hash */
            computeHash(hashProgs[hashAlgNum]);
        }
        /* I am the parent */
        else
        {
            /* Call parent function to communicate with child */
            parentFunc(hashProgs[hashAlgNum]);

            /* Wait for the child to terminate */
            if (wait(NULL) < 0)
            {
                perror("wait");
                exit(-1);
            }
        }
    }
    
    return 0;
}
