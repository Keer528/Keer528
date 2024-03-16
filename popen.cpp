#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char** argv)
{

	/* Launch the program md5sum to compute the MD5 hash of file
	 * /bin/ls. Also, get a file pointer representing the output
	 * stream of the program
	 */
	FILE* progOutput = popen("md5sum /bin/ls", "r");

	/* Make sure that popen succeeded */
	if(!progOutput)
	{
		perror("popen");
		exit(-1);
	}

	/* The maximum output size */
	#define MAX_OUTPUT_SIZE 1000

	/* A buffer to store the output */
	char buffer[MAX_OUTPUT_SIZE];

	/* Reset the buffer to all NULLS */
	memset(buffer, (char)NULL, MAX_OUTPUT_SIZE);


	/* Read the program output into the buffer */
	if(fread(buffer, sizeof(char), sizeof(char) * MAX_OUTPUT_SIZE, progOutput) < 0)
	{
		perror("fread");
		exit(-1);
	}

	/* Close the file pointer representing the program output */
	if(pclose(progOutput) < 0)
	{
		perror("perror");
		exit(-1);
	}

	fprintf(stdout, "Program output: %s\n", buffer);
	fflush(stdout);

	return 0;
}
