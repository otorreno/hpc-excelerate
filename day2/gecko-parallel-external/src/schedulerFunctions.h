#ifndef SCHEDULER_FUNCTIONS_H
#define SCHEDULER_FUNCTIONS_H

#define MAXLIN 		3000
#define MAXARGS 	100
#define MAXCOMMANDS	20

/**
 * Function to read the workload from a given file
 */
char** readWorkloadFile(char *nomfich, int *n);

/**
 * Function to prepare a message containing several commands separated by ';''
 */
char* prepareMessage(int* countCommand, int totalCommands, char** tabla, int* length);

/**
 * Function to extract the arguments array from a given command
 */
int getCommandLine(char *completeLine, char **arguments);

/**
 * Function to split several command lines separated by ';'
 */
int getMessage(char *completeLine, char **messages);

#endif /* SCHEDULER_FUNCTIONS_H */
