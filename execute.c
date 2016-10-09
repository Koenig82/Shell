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

int dupPipe(int pip[2], int end, int destfd){

    if(dup2(end,destfd) < 0){
        perror("dup2 error:");
        return -1;
    }

    if(close(end) < 0){
        perror("close error:");
        return -1;
    }
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


}