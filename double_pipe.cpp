#include <iostream>
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

using namespace std;

/* The pipe for parent-to-child communications */
int parentToChildPipe[2];

/* The pipe for the child-to-parent communication */
int childToParentPipe[2];

/* The read end of the pipe */
#define READ_END 0

/* The write end of the pipe */
#define WRITE_END 1

/**
 * The function called by parent
 */
void parentFunc()
{

	/** I am a parent **/

	/* The string received from the child */
	char strReceived[100];

	/* The string to send to the child */
	char strToSend[] = "Hello child!";

	/* Close the write end of the child-to-parent pipe */
	if (close(childToParentPipe[WRITE_END]) < 0)
	{
		perror("close");
		exit(-1);
	}

	/* Close the read end of the parent-to-child pipe */
	if (close(parentToChildPipe[READ_END]) < 0)
	{
		perror("close");
		exit(-1);
	}

	/* Read the string sent by the child */
	if (read(childToParentPipe[READ_END], strReceived, sizeof(strReceived)) < 0)
	{
		perror("read");
		exit(-1);
	}

	fprintf(stderr, "Parent received a string from the child: %s\n\n", strReceived);

	fprintf(stderr, "Parent sending a string to the child: %s\n", strToSend);

	/* Send the string to the child */
	if (write(parentToChildPipe[WRITE_END], strToSend, sizeof(strToSend)) < 0)
	{
		perror("write");
		exit(-1);
	}

	/* Close the writing end of the parent-to-child pipe; this will cause the child's
	 * read() function to unblock.
	 */
	if (close(parentToChildPipe[WRITE_END]) < 0)
	{
		perror("close");
		exit(-1);
	}


	/* Close the writing end of the parent-to-child pipe; this will cause the child's
	 * read() function to unblock.
	 */
	if (close(childToParentPipe[READ_END]) < 0)
	{
		perror("close");
		exit(-1);
	}

}



/**
 * The function called by a child
 */
void childFunc()
{

	/* The string to send to the parent */
	char strToSend[] = "Hello parent!";

	fprintf(stderr, "Hi, I am a child\n");

	/* Close the write-end of the parent-to-child pipe */
	if(close(parentToChildPipe[WRITE_END]) < 0)
	{
		perror("close");
		exit(-1);
	}


	/* Close the read-end of the child-to-parent pipe */
	if(close(childToParentPipe[READ_END]) < 0)
	{
		perror("close");
		exit(-1);
	}

	fprintf(stderr, "Child sending a string to the parent: %s\n", strToSend);

	/* Pause for 2 seconds */
	sleep(2);

	/* Send a string to the parent */
	if(write(childToParentPipe[WRITE_END], strToSend, sizeof(strToSend)) < 0)
	{
		perror("write");
		exit(-1);
	}


	/* Close write end of the child-to-parent pipe; this will
	 * cause the parent's read() function to unblock.
	 */
	if(close(childToParentPipe[WRITE_END]) < 0)
	{
		perror("close");
		exit(-1);
	}

	/* The string recieved from the parent */
	char strReceived[100];


	/** Now, lets read a message from the parent **/
	if(read(parentToChildPipe[READ_END], strReceived, sizeof(strReceived)) < 0)
	{
		perror("read");
		exit(-1);
	}

	fprintf(stderr, "Child received a string from the parent: %s\n", strReceived);

	/* Close the read end of the parent-to-child pipe */
	if(close(parentToChildPipe[READ_END]) < 0)
	{
		perror("close");
		exit(-1);
	}

	/* The child terminates */
	exit(0);

}

int main(int argc, char** argv)
{

	/* Create a parent-to-child pipe */
	if(pipe(parentToChildPipe) < 0)
	{
		perror("pipe");
		exit(-1);
	}


	/* Create a child-to-parent pipe */
	if(pipe(childToParentPipe) < 0)
	{
		perror("pipe");
		exit(-1);

	}



	/* The process id */
	pid_t pid;

	/* Fork a child process and save the id */
	if((pid = fork()) < 0)
	{
		perror("fork");
		exit(-1);
	}
	/* I am a child */
	else if(pid == 0)
	{
		childFunc();
	}
	else 
	{
		
		parentFunc();
		

		/* Wait for the child process to terminate */
		if (wait(NULL) < 0)
		{
			perror("wait");
			exit(-1);
		}

		
		
	}
	
	return 0;
}
