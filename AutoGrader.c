//Tomer Peisikov 209549542

#include "string.h"
#include "stdlib.h"
#include <unistd.h>
#include <fcntl.h>
#include "stdio.h"
#include <dirent.h>
#include <sys/wait.h>
#include <sys/time.h>

//function that prints the error type if it occurs, and exits if received 1
void fatalErr(char *errType) {
    if (write(2, errType, strlen(errType)) == -1) {

    }
   exit(-1);
}

//function that prints the error type if it occurs, and exits if received 1
void printError(char *errType, int saveStdErr) {
    int tmp = dup(2);
    dup2(saveStdErr, 2);

    if (write(2, errType, strlen(errType)) == -1) {

    }
    dup2(tmp,2);
    close(tmp);
}

//reads the lines from configuration
void readLine(char *line, int *fd) {
    int i = 0;
    char c = ' ';
    while (c != '\n') {
        //reads char char until \n
        if (read(*fd, &c, 1) == -1) {
            fatalErr("Error in: read\n");
        }
        line[i] = c;
        i++;
    }
    i--;
    line[i] = '\0';
}


//this function compiles the student's program and returns 0 in success
int compile(char *program, char *newLine, int saveStdErr) {
    pid_t pid = fork();
    if (pid == -1) {
        printError("Error in: fork\n",saveStdErr);
    }
        //child will compile
    else if (pid == 0) {
        strcat(newLine, "/");
        strcat(newLine, program);
        char *command = "gcc";
        char *args[] = {"gcc", newLine, "-o", "ex.out", NULL};
        //compiling students program
        int e = execvp(command, args);
        if (e == -1) {
            printError("Error in: execvp\n",saveStdErr);
            exit(1);
        }
    } else {
        int status;
        //father waits for child to finish compiling
        wait(&status);
        if (WIFEXITED(status)) {
            if (WEXITSTATUS(status) != 0) {
                return 1;
            }
        }
    }
    return 0;
}

//this function runs student's program
int run(int saveStdErr) {
    struct timeval start, end;
    // get current time
    if (gettimeofday(&start, NULL) != 0) {
        printError("Error in: gettimeofday\n",saveStdErr);
    }
    pid_t pid = fork();
    if (pid == -1) {
        printError("Error in: fork\n",saveStdErr);
    }
        // child process
    else if (pid == 0) {
        int newFd = open("out", O_CREAT | O_RDWR, 0777);
        if (newFd == -1) {
            printError("Error in: open\n",saveStdErr);
        }
        if (dup2(newFd, 1) == -1) {
            printError("Error in: dup2\n",saveStdErr);
        }
        char *args[] = {"./ex.out", NULL};
        int e = execvp(args[0], args);
        if (e == -1) {
            exit(1);
        }
        if (access("ex.out", F_OK) != -1){
            if(unlink("ex.out") == -1){
                printError("Error in: unlink\n",saveStdErr);
            }
        }
        if (close(newFd) == -1) {
            printError("Error in: close\n",saveStdErr);
        }
        // parent process
    } else {
        int status;
        // check if child has finished
        while (waitpid(pid, &status, WNOHANG) == 0) {
            // get current time
            if (gettimeofday(&end, NULL) != 0) {
                printError("Error in: gettimeofday\n",saveStdErr);
            }
            // check if 5 seconds have passed
            if ((end.tv_sec - start.tv_sec) >= 5) {
                return -1;
            }
        }
    }
    return 1;
}

//this function takes two files and with the help of comp.out compares them
int compare(char *cmp, char *solution, int saveStdErr) {
    pid_t pid = fork();
    if (pid == -1) {
        printError("Error in: fork\n",saveStdErr);
    }
        //child will compare between two files
    else if (pid == 0) {
        char cwd[200];
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            printError("Error in: getcwd\n",saveStdErr);
        }
        strcat(cwd, "/out");
        char *args[] = {cmp, solution, cwd, NULL};
        int e = execvp(cmp, args);
        if (e == -1) {
            printError("Error in: execvp\n",saveStdErr);
            exit(1);
        }
    } else {
        //father waits for child to finish running
        int status;
        waitpid(pid, &status, 0); // wait for child to terminate
        if (WIFEXITED(status)) { // check if child terminated normally
            return WEXITSTATUS(status);
        }
    }
    return -1;
}

//this function prints to the csv file the output grade of each student
void print(int grade, int resFd, char *name) {
    char str[100];
    strcpy(str, name);
    switch (grade) {
        case 0:
            strcat(str, ",0,NO_C_FILE\n");
            break;
        case 10:
            strcat(str, ",10,COMPILATION_ERROR\n");
            break;
        case 20:
            strcat(str, ",20,TIMEOUT\n");
            break;
        case 50:
            strcat(str, ",50,WRONG\n");
            break;
        case 75:
            strcat(str, ",75,SIMILAR\n");
            break;
        case 100:
            strcat(str, ",100,EXCELLENT\n");
            break;
    }
    //writes to csv file
    if (write(resFd, str, strlen(str)) == -1) {

    }
}

//this function delete out file that a student made, if he didn't make any out file just returns
void delete(int saveStdErr) {
    if (access("out", F_OK) != -1) {
        if (remove("out") == -1) {
            printError("Error in: remove\n", saveStdErr);
        }
    }
}

int main(int argc, char *argv[]) {
    //initializations
    char *configuration = argv[1];
    char cwd[200], cmp[200];
    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        fatalErr("Error in: getcwd\n");
    }
    strcpy(cmp, cwd);
    strcat(cmp, "/comp.out");
    char line1[200], line2[200], line3[200];
    int fd;
    DIR *dir;
    struct dirent *pDirent;
    const char subString[] = ".c";
    const char current[] = ".";
    const char father[] = "..";
    int grade;
    int input = 0, output;
    int resFd;

    //saving the fd of std input and output
    int saveStdIoInput = dup(0);
    int saveStdIoOutPut = dup(1);
    int saveStdErr = dup(2);

    if(saveStdIoInput == -1 || saveStdIoOutPut == -1 || saveStdErr == -1){
        fatalErr("Error in: dup\n");
    }

    //check if able to access and open the configuration file
    if (access(configuration, F_OK) == 0 && access(configuration, R_OK) == 0) {
        fd = open(configuration, O_RDONLY);
        if (fd == -1) {
            fatalErr("Error in: open\n");
        }
    } else {
        fatalErr("Error in: access\n");
    }

    //read all three lines from configuration
    readLine(line1, &fd);
    readLine(line2, &fd);
    readLine(line3, &fd);

    //error output redirection
    int errFd = open("errors.txt", O_CREAT | O_RDWR | O_APPEND, 0777);
    if (errFd == -1) {
        fatalErr("Error in: open\n");
    }

    if(dup2(errFd, 2) == -1){
        fatalErr("Error in: dup2\n");
    }

    //results.csv
    resFd = open("results.csv", O_CREAT | O_RDWR | O_APPEND, 0777);
    if (resFd == -1) {
        printError("Error in: open\n",saveStdErr);
    }



    //finished using configuration file
    if (close(fd) == -1) {
        printError("Error in: close\n",saveStdErr);
    }

    if ((dir = opendir(line1)) == NULL) {
        printError("Not a valid directory\n",saveStdErr);
    }

    //check if input file is a file and can be accessed
    if (access(line2, F_OK) == 0 && access(line2, R_OK) == 0) {
        input = open(line2, O_RDONLY);
        if (input == -1) {
            printError("Error in: open\n",saveStdErr);
        }
    } else {
        printError("Input file not exist\n",saveStdErr);
    }

    //save standard input
    if (dup2(input, 0) == -1) {
        printError("Error in: dup2\n",saveStdErr);
    }

    //check if output file is a file and can be accessed
    if (access(line3, F_OK) == 0 && access(line3, R_OK) == 0) {
        output = open(line3, O_RDONLY);
        if (output == -1) {
            printError("Error in: open\n",saveStdErr);
        }
    } else {
        printError("Output file not exist\n",saveStdErr);
    }

    //run on all subfolders
    while ((pDirent = readdir(dir)) != NULL) {
        grade = 0;
        //check if folder and not '.' or '..'
        if (pDirent->d_type == DT_DIR && strcmp(pDirent->d_name, father) != 0 &&
            strcmp(pDirent->d_name, current) != 0) {
            char newLine[200];
            strcpy(newLine, line1);
            strcat(newLine, "/");
            strcat(newLine, pDirent->d_name);
            DIR *newDir;
            struct dirent *pNewDirent;
            if ((newDir = opendir(newLine)) == NULL) {
                continue;
            }
            //run on all the files and folders the student has in his folder
            while ((pNewDirent = readdir(newDir)) != NULL) {
                unsigned long len = strlen(pNewDirent->d_name);
                //checks if this is a file and if it ends with .c it means we need to compile it
                if (pNewDirent->d_name[len - 2] == subString[0] && pNewDirent->d_name[len - 1] == subString[1] &&
                    pNewDirent->d_type != DT_DIR) {
                    // Set the file offset to the beginning of the file
                    if (lseek(input, 0, SEEK_SET) == -1) {
                        printError("Error in: lseek\n",saveStdErr);
                    }
                    //compile program and if not compiling the grade is 10
                    if (compile(pNewDirent->d_name, newLine, saveStdErr)) {
                        grade = 10;
                        break;
                    }
                    //run program, if it takes more than 5 seconds grade is 20
                    if (run(saveStdErr) == -1) {
                        grade = 20;
                        break;
                    }
                    int correctness = compare(cmp, line3,saveStdErr);
                    //both files are exactly the same grade is 100
                    if (correctness == 1) {
                        grade = 100;
                        //both files are different grade is 50
                    } else if (correctness == 2) {
                        grade = 50;
                        //both files similar but not exactly the same grade is 75
                    } else if (correctness == 3) {
                        grade = 75;
                    }
                }
            }
            //delete the out file that been created
            delete(saveStdErr);
            if (chdir(cwd) == -1) {
                printError("Error in: chdir\n",0);
            }
            //print student's grade
            print(grade, resFd, pDirent->d_name);
        }
    }


    if (dup2(saveStdIoInput, 0) == -1) {
        printError("Error in: dup2\n",1);
    }

    if (dup2(saveStdIoOutPut, 1) == -1) {
        printError("Error in: dup2\n",1);
    }

    if (close(input) == -1) {
        printError("Error in: close\n",1);
    }

    if (close(resFd) == -1) {
        printError("Error in: close\n",1);
    }
    if (close(errFd) == -1) {
        printError("Error in: close\n",1);
    }

    // Close the file descriptors that were opened
    if(close(saveStdIoInput) == -1){
        printError("Error in: close\n",1);
    }
    if(close(saveStdIoOutPut) == -1){
        printError("Error in: close\n",1);
    }

    return 0;
}
