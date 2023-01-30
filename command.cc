/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#define _DEBUG_LOG	TRUE
#include <stdio.h>
#include <time.h>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include "command.h"



SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
	_freeonce = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}
	if ( _inputFile ) {
		free( _inputFile );
	}
	
	if (!_freeonce)
	{
		if ( _outFile ) {
			free( _outFile );
		}
		
		if ( _errFile ) {
			free( _errFile );
		}
	}
	else
	{
		if ( _outFile ) {
			free( _outFile );
		}
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_append = 0;
	_freeonce = 0;
}

void
Command::print()
{
	char array[100];
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  ----------------------------------------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background        CWD\n" );
	printf( "  ------------ ------------ ------------ ------------ --------------\n" );
	printf( "  %-12s %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO", getcwd(array,100));
	printf( "\n\n" );
	
}
void
Command::execute()
{
	int pid;
	int inputfile;
	int outputfile;
	int errorfile;
	int inpfd;
	int outfd;
	int errfd;
	char array[100];
	// Don't do anything if there are no simple commands
	if ( _numberOfSimpleCommands == 0 ) {
		prompt();
		return;
	}

	// Print contents of Command data structure
	print();
	if (!strcmp(_simpleCommands[0]->_arguments[0],"cd"))
	{
		printf("Current Working Directory: %s\n",getcwd(array,100));
		if (_simpleCommands[0]->_numberOfArguments > 1)
		{
			chdir(_simpleCommands[0]->_arguments[1]);
		}
		else
		{
			chdir("/home");
		}
		printf("New Working Directory: %s\n",getcwd(array,100));
		
	}
	else
	{

		// Add execution here
		// For every simple command fork a new process
		// Setup i/o redirection
		// and call exec
		
		/*if ( _numberOfSimpleCommands == 1 )
		{
			int defaultin = dup( 0 ); // Default file Descriptor for stdin
			int defaultout = dup( 1 ); // Default file Descriptor for stdout
			int defaulterr = dup( 2 ); // Default file Descriptor for stderr
			
			if (_inputFile)
			{
				// Create file descriptor 
				inpfd = open( _inputFile, O_RDONLY );
				
				if ( inpfd < 0 ) {
					perror( "err : create inputfile\n");
					exit( 2 );
				}
				inputfile=inpfd;
				//close(inpfd);
			}
			else
			{
				inputfile=defaultin;
			}
			if (_outFile)
			{
				
				// Create file descriptor 
				if (_append)
				{
					outfd = open( _outFile, O_APPEND | O_CREAT | O_WRONLY, 0666);
				}
				else
				{
					outfd = creat( _outFile, 0666);
				}
				
				if ( outfd < 0 ) {
					perror( "err : create outfile\n");
					exit( 2 );
				}
				outputfile=outfd;
				//close(outfd);
			}
			else
			{
				outputfile=defaultout;
			}
			if (_errFile)
			{
				// Create file descriptor 
				if (_append)
				{
					errfd = open( _errFile, O_APPEND | O_CREAT | O_WRONLY, 0666);
				}
				else
				{
					errfd = creat( _errFile, 0666);
				}
				
				if ( errfd < 0 ) {
					perror( "err : create outfile\n");
					exit( 2 );
				}
				errorfile=errfd;
				//close(errfd);
			}
			else
			{
				errorfile=defaulterr;
			}
			dup2(inputfile,0);
			dup2(outputfile,1);
			dup2(errorfile,2);
			// Create new process for "ls"
			int pid = fork();
			if ( pid == -1 ) {
				perror( "err : fork\n");
				exit( 2 );
			}
			
			if (pid == 0) {
				//Child
				
				// close file descriptors that are not needed
				close(inputfile);
				close(outputfile);
				close(errorfile);
				close( defaultin );
				close( defaultout );
				close( defaulterr );

				// You can use execvp() instead if the arguments are stored in an array
				//execlp(ls, ls, "-l", (char *) 0);
				execvp(_simpleCommands[0]->_arguments[0] ,_simpleCommands[0]->_arguments);

				// exec() is not suppose to return, something went wrong
				perror( "err: exec ");
				exit( 2 );
			}

			// Restore input, output, and error
			
			dup2( defaultin, 0 );
			dup2( defaultout, 1 );
			dup2( defaulterr, 2 );

			// Close file descriptors that are not needed
			//close( outfd );
			close( defaultin );
			close( defaultout );
			close( defaulterr );
			if(!_background)
			{
			// Wait for last process in the pipe line
			waitpid( pid, 0, 0 );
			}
			//exit( 2 );
			
		}*/
		
		if (_numberOfSimpleCommands >= 1)
		{
			//printf(">>>%d<<<\n",_numberOfSimpleCommands);
			int defaultin = dup( 0 ); // Default file Descriptor for stdin
			int defaultout = dup( 1 ); // Default file Descriptor for stdout
			int defaulterr = dup( 2 ); // Default file Descriptor for stderr
			
			// Create new pipe 
			// Conceptually, a pipe is a connection between two processes, 
			// such that the standard output from one process becomes the standard input of the other process.
			// so if a process writes to fdpipe[1] process be can read from fdpipe[0] 
			int fdpipe[2];
			if ( pipe(fdpipe) == -1) {
				perror( "cat_grep: pipe");
				exit( 2 );
			}
			if (_inputFile)
			{
				inpfd = open( _inputFile, O_RDONLY, 0666 );
				if ( inpfd < 0 )
				{
					perror( "err : create inputfile" );
					exit( 2 );
				}
			}
			else 
			{
				inpfd=defaultin;
			}		
			if (_outFile)
			{
				// Create file descriptor 
				if (_append)
				{
					outfd = open( _outFile, O_APPEND | O_CREAT | O_WRONLY, 0666);
				}
				else if(!_append) 
				{
					outfd = open(_outFile, O_CREAT | O_WRONLY | O_TRUNC,0666);
				}
				
				if ( outfd < 0 ) {
					perror( "err : create outfile" );
					exit( 2 );
				}
			}
			else
			{
				outfd=defaultout;
			}
			if (_errFile)
			{
				// Create file descriptor 
				if (_append)
				{
					errfd = open( _errFile, O_APPEND | O_CREAT | O_WRONLY, 7777);
				}
				else
				{
					errfd = open(_errFile, O_CREAT | O_WRONLY | O_TRUNC,7777);
				}
				
				if ( errfd < 0 ) {
					perror( "err : create outfile" );
					exit( 2 );
				}
			}
			else 
			{
				errfd=defaulterr;
			}	
			for (int i = 0 ; i < _numberOfSimpleCommands ; i++)
			{
			//printf(">>>%d<<<\n",i);	
				if ( i == 0 )
				{
					if (_inputFile)
					{
						// Create file descriptor 
						
						//inputfile=inpfd;
						dup2(inpfd,0);
						close(inpfd);
					}
					else
					{
						dup2(defaultin,0);
					}		
					//outputfile=fdpipe[1];
					//errorfile=defaulterr;
				}
				if (i != 0 )
				{
					dup2(fdpipe[0],0);
					close(fdpipe[0]);
					close(fdpipe[1]);
					  if ( pipe(fdpipe) == -1) {
               						  perror( "cat_grep: pipe");
               						  exit( 2 );
             					}	
				}
				if ( i == _numberOfSimpleCommands - 1 )
				{
					if (_outFile)
					{
						// Create file descriptor 
						dup2(outfd,1);
						close(outfd);
					}
					else
					{
						dup2(defaultout,1);
					}
					if (_errFile)
					{
						// Create file descriptor 
						dup2(errfd,2);
						close(errfd);
					}
					if (!_errFile)
					{
						dup2(defaulterr,2);
					}
				}
				if ( i != _numberOfSimpleCommands - 1 )
				{
					dup2(fdpipe[1],1);
					//close(fdpipe[1]);
					//dup2(defaulterr,2);
				}

			//	dup2(inputfile,0);
			//	dup2(outputfile,1);
			//	dup2(errorfile,2);
				
				// Create new process
				pid = fork();
				if ( pid == -1 ) {
					perror( "err : fork\n");
					exit( 2 );
				}

				if (pid == 0) {
					//Child
					
					// close file descriptors that are not needed
					//close(inputfile);
					//close(outputfile);
					//close(errorfile);
					close(inpfd);
					close(outfd);
					close(fdpipe[0]);
					close(fdpipe[1]);
					close( defaultin );
					close( defaultout );
				        close( defaulterr );
					
					// You can use execvp() instead if the arguments are stored in an array
					execvp(_simpleCommands[i]->_arguments[0] ,_simpleCommands[i]->_arguments);

					// exec() is not suppose to return, something went wrong
					perror( "err: exec");
					exit( 2 );
				}
				if (pid > 0)
				{
					dup2( defaultin, 0 );
					dup2( defaultout, 1 );
					dup2( defaulterr, 2 );
					if (!_background )
					{
						
						// Wait for last process in the pipe line
						waitpid(pid, 0, 0 );
					}
				}
				
			}
			
			// Restore input, output, and error

			dup2( defaultin, 0 );
			dup2( defaultout, 1 );
			dup2( defaulterr, 2 );
			
			// close file descriptors that are not needed
			close(fdpipe[0]);
			close(fdpipe[1]);
			close(inpfd);
			close(outfd);
			close(errfd);
			close( defaultin );
			close( defaultout );
			close( defaulterr );
			
			
			
		}
	}
	// Clear to prepare for next command
	clear();
	
	// Print new prompt
	prompt();
}

// Shell implementation

void
Command::prompt()
{
	printf("myshell>");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

void INThandler(int sig){
	return;
	}
void INTChandler(int sig){
	char buff[100];
	time_t t = time(NULL);
  	struct tm tm = *localtime(&t);
  	snprintf(buff, sizeof(buff),"Process terminated at : %d-%02d-%02d %02d:%02d:%02d\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	std::ofstream outfile;
	outfile.open("log.txt", std::ios_base::app); // append instead of overwrite
        outfile << buff;
	outfile.close();
	return;
	}
int yyparse(void);

int 
main()
{
	Command::_currentCommand.prompt();
	signal(SIGINT,INThandler);
	signal(SIGCHLD,INTChandler);
	yyparse();
	return 0;
}

