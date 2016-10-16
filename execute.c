/* Duplicate a pipe to a standard I/O file descriptor, and close both pipe ends
 * Arguments:	pip	the pipe
 *		end	tells which end of the pipe shold be dup'ed; it can be
 *			one of READ_END or WRITE_END
 *			the standard I/O file descriptor to be replaced
 * Returns:	-1 on error, else destfd
 */
#include "execute.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <errno.h>

int dupPipe(int pip[2], int end, int destfd){
    //duplicates the end-fd to destfd
    if(dup2(end,destfd) < 0){
        perror("dup2 error:");
        return -1;
    }
    //close the end fd
    if(close(end) < 0){
        perror("close error:");
        return -1;
    }
    fprintf(stderr, "Closing fd %d\n", end);
    fflush(stderr);
    return destfd;

}


/* Redirect a standard I/O file descriptor to a file
 * Arguments:	filename	the file to/from which the standard I/O file
 * 				descriptor should be redirected
 * 		flags	indicates whether the file should be opened for reading
 * 			or writing
 * 			the standard I/O file descriptor which shall be
 *			redirected
 * Returns:	-1 on error, else destfd
 */
int redirect(char *filename, int flags, int destfd){

    if (flags == 0){

        int read = open (filename, O_RDONLY);

        if (read < 0){

            perror(filename);

        }else{

            return read;
        }

    }else{

        if(access(filename, W_OK) == 0){

            errno = EEXIST;
            perror("write error");

        }else{

            int write = open(filename, O_WRONLY | O_CREAT | O_EXCL, S_IRWXU );

            if (write < 0){
                perror(filename);
            }
            else{

                return write;
            }
        }
    }
    return -1;
}