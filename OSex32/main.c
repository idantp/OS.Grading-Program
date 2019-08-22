// Author: Idan Twito
// ID: 311125249
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <time.h>
#include <sys/times.h>


#define ERROR_MSG_LENGTH 21
#define ERROR_MSG "Error in system call\n"
#define FILE_DESC 2
#define INVALID_INPUT "Invalid input\n"
#define MAX_PATH_LEN 1024
#define SLASH_STRING "/"
#define COMMA_STRING ","
#define DOT_CHAR '.'
#define C_CHAR 'c'
#define NEW_LINE_CHAR '\n'
#define NEW_LINE_STR "\n"
#define GCC_COMMAND "gcc"
#define MAX_IN_LINE 151
#define CHAR_SIZE 1
#define O_FILE "-o"
#define EXECUTABLE_NAME "myExec.out"
#define EXECUTABLE_NAME_PATH "./myExec.out"
#define COMP_OUT_FILE "./comp.out"
#define SOLUTION_FILE_NAME "solution.txt"
#define RESULTS_CSV_FILE "results.csv"
#define NO_C_FILE_SCORE "0"
#define COMPILATION_ERROR_SCORE "20"
#define TIMEOUT_SCORE "40"
#define BAD_OUTPUT_SCORE "60"
#define SIMILAR_OUTPUT_SCORE "80"
#define GREAT_JOB_SCORE "100"
#define NO_C_FILE "NO_C_FILE"
#define COMPILATION_ERROR "COMPILATION_ERROR"
#define TIMEOUT "TIMEOUT"
#define BAD_OUTPUT "BAD_OUTPUT"
#define SIMILAR_OUTPUT "SIMILAR_OUTPUT"
#define GREAT_JOB "GREAT_JOB"
#define TIMEOUT_REACHED 5

typedef struct ConfigurationData {
    char mainDirectoryPath[MAX_IN_LINE];
    char inputFilePath[MAX_IN_LINE];
    char correctOutputPath[MAX_IN_LINE];
} ConfigurationData;


/**
 * Function Name: errorPrint
 * Function Input: void
 * Function Output: void
 * Function Operation: writes a system call error message
 */
void errorPrint() {
    char *buffer = ERROR_MSG;
    write(FILE_DESC, buffer, ERROR_MSG_LENGTH);
}
/**
* Function Name: isASourceFile
* Function Input: char *fileName
* Function Output: true if the given file is a c source file, otherwise false
* Function Operation: the function returns true if the given file is a c source
*                    file, otherwise false
*/
bool isASourceFile(char *fileName) {
    if ((fileName[strlen(fileName) - 2] == DOT_CHAR) &&
        (fileName[strlen(fileName) - 1] == C_CHAR)) {
        return true;
    }
    return false;
}


/**
 * Function Name: fullPathConcatenate
 * Function Input: char *str, char *pathToConcat, char *fileNameToConcat
 * Function Output: void
 * Function Operation: the function concatenates pathToConcat with "/" with
 *                     fileNameToConcat into str. in order to create full path to file
 */
void fullPathConcatenate(char *str, char *pathToConcat, char *fileNameToConcat) {
    strcpy(str, pathToConcat);
    strcat(str, SLASH_STRING);
    strcat(str, fileNameToConcat);
}

/**
 * Function Name: compileSourceFile
 * Function Input: char *currentPath, char *sourceFileName, DIR *dir
 * Function Output: -1 if an error.
 *                   0 compilation process completed successfully.
 *                   1 couldn't compile the source file.
 * Function Operation: the function compiles the given source code in the given full-path
 *                     and returns as described above.
 */
int compileSourceFile(char *currentPath, char *sourceFileName) {
    char sourceFileFullPath[MAX_PATH_LEN] = {0};
    fullPathConcatenate(sourceFileFullPath, currentPath, sourceFileName);
    char *gccCommand[] = {GCC_COMMAND, O_FILE, EXECUTABLE_NAME, sourceFileFullPath, NULL};
    pid_t pid;
    int status;
    pid = fork();
    if (pid > 0) {
        //parent
        wait(&status);
        // if the compilation finished successfully
        if ((WEXITSTATUS(status)) == 0) {
            // source file compiled successfully.
            return 0;
        }
    } else if (pid == 0) {
        //child
        if (execvp(gccCommand[0], gccCommand) == -1) {
            errorPrint();
            return -1;
        }
    } else {
        //error
        errorPrint();
        return -1;
    }
    // didn't compile
    return 1;
}

/**
 * Function Name: runExecutable
 * Function Input: char *fileInput, DIR *dir
 * Function Output: -1 if an error.
 *                   0 if program execution completed successfully.
 *                   1 if the execution lasted more than 5 second - timeout error
  * Function Operation: the function executes the given executable file in the given full-path
 *                     and returns as described above.
 */
int runExecutable(char *fileInput) {
    time_t startTime, endTime;
    double cpuTime = 0;
    pid_t pid;
    int status;
    int newFdIn, newFdOut;
    char *runCommand[] = {EXECUTABLE_NAME_PATH, fileInput, NULL};
    time(&startTime);
    pid = fork();
    if (pid > 0) {
        //parent
        while (waitpid(pid, &status, WNOHANG) != pid) {
            time(&endTime);
            cpuTime = difftime(endTime, startTime);
            // if the execution time lasted more than 5 seconds return 1
            if (cpuTime > TIMEOUT_REACHED) {
                kill(pid, SIGKILL);
                wait(NULL);
                return 1;
            }
        }
    } else if (pid == 0) {
        //child
        if ((newFdIn = open(fileInput, O_RDONLY, 0644)) < 0) {
            errorPrint();
            return -1;
        }

        if ((newFdOut = open(SOLUTION_FILE_NAME, O_CREAT | O_TRUNC | O_RDWR, 0644)) < 0) {
            errorPrint();
            return -1;
        }
        // change the stdin to the text in the following file
        // change the stdout to the solution.txt file
        dup2(newFdIn, 0);
        dup2(newFdOut, 1);
        close(newFdIn);
        close(newFdOut);
        if (execvp(runCommand[0], runCommand) == -1) {
            errorPrint();
            return -1;
        }
    } else {
        //error
        errorPrint();
        return -1;
    }
    return 0;
}

/**
 * Function Name: runCompOut
 * Function Input: char *fileOutput, char *correctOutput
 * Function Output: -1 if an error.
 *                   1 if the output file and the correct output file are identical.
 *                   2 if the output file and the correct output file are different.
 *                   3 if the output file and the correct output file are partially identical.
 * Function Operation: the function compile comp.out with the output file and
 *                     the correct output file, check their similarity and returns as
 *                     described above.
 */
int runCompOut(char *fileOutput, char *correctOutput) {
    pid_t pid;
    int status;
    char *compExecArgs[] = {COMP_OUT_FILE, fileOutput, correctOutput, NULL};
    pid = fork();
    if (pid > 0) {
        //parent
        do {
            waitpid(pid, &status, WUNTRACED);
        } while ((!WIFEXITED(status) && !WIFSIGNALED(status)));
    } else if (pid == 0) {
        //child
        if (execvp(compExecArgs[0], compExecArgs) == -1) {
            errorPrint();
            return -1;
        }
        return 1;
    } else {
        //error
        errorPrint();
        return -1;
    }
    return WEXITSTATUS(status);
}


/**
 * Function Name: resultsCsvHandler
 * Function Input: char *score, char *userName
 * Function Output: -1 if a system-call error occurred
 *                   1 if the function has written successfully to results.csv
 * Function Operation: the function gets score that describes the grade that the given
 *                     userName (student) should get. it writes to results.csv in a line
 *                     the name of the student, his score (grade) and a short description
 *                     that explains the reason for his grade.
 */
int resultsCsvHandler(char *score, char *userName, int *lineNumber) {
    int fd;
    if ((fd = open(RESULTS_CSV_FILE, O_CREAT | O_APPEND | O_WRONLY, 0644)) < 0) {
        errorPrint();
        return -1;
    }
    char temp[MAX_PATH_LEN] = {0};
    if ((*lineNumber) != 0) {
        strcpy(temp, NEW_LINE_STR);
        strcat(temp, userName);
    } else { strcpy(temp, userName); }
    strcat(temp, COMMA_STRING);
    strcat(temp, score);
    strcat(temp, COMMA_STRING);
    if (strcmp(score, NO_C_FILE_SCORE) == 0) { strcat(temp, NO_C_FILE); }
    else if (strcmp(score, COMPILATION_ERROR_SCORE) == 0) { strcat(temp, COMPILATION_ERROR); }
    else if (strcmp(score, TIMEOUT_SCORE) == 0) { strcat(temp, TIMEOUT); }
    else if (strcmp(score, BAD_OUTPUT_SCORE) == 0) { strcat(temp, BAD_OUTPUT); }
    else if (strcmp(score, SIMILAR_OUTPUT_SCORE) == 0) { strcat(temp, SIMILAR_OUTPUT); }
    else { strcat(temp, GREAT_JOB); }
    if (write(fd, temp, strlen(temp)) == -1) {
        errorPrint();
        return -1;
    }
    (*lineNumber) = (*lineNumber) + 1;
    return 1;
}

/**
 * Function Name: listDir
 * Function Input: full-path to current scanned folder, path to the input txt file, path
 *                 to the correct output txt file name of the student's file name, int*
 *                 cFilesNum counts the number of c files in the student's folder
 * Function Output: -1 if an error occurred
 *                   0 if the given student's folder or its sub-folders don't
 *                   contain any c source file
 *                   1 if the function has written the appropriate into results.csv file
 * Function Operation: the function scans all the folders and the sub-folders of the
 *                     current path in order to find a c source file and to write the
 *                     appropriate line into results.csv file, according to the correspondence
 *                     of the student's program output and the correct output(which is given).
 */
int listDir(char *currentPath, char *inputPath, char *correctOutputPath, char *studentName,
            int *cFilesNum, int *lineNumber) {
    int compilationResult;
    DIR *dir;
    int compResult;
    struct dirent *entry;
    if ((dir = opendir(currentPath)) != NULL) {
        //scan all the files, folders
        while ((entry = readdir(dir)) != NULL) {
            char *fileName = entry->d_name;
            // if the current file is a directory, then keep scanning recursively
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
                strcmp(fileName, "..") != 0) {
                char nextDirectory[MAX_PATH_LEN] = {0};
                fullPathConcatenate(nextDirectory, currentPath, fileName);
                listDir(nextDirectory, inputPath, correctOutputPath, studentName, cFilesNum,
                        lineNumber);
                //else if the current file is a c source file
            } else if (isASourceFile(fileName)) {
                (*cFilesNum) = (*cFilesNum) + 1;
                // compile the c source file
                compilationResult = compileSourceFile(currentPath, fileName);
                // if a system call error occurred in the function - close folder and return -1
                if (compilationResult == -1) {
                    closedir(dir);
                    return -1;
                    // if a compilation error occurred, then write an appropriate line
                    // into results.csv
                } else if (compilationResult == 1) {
                    resultsCsvHandler(COMPILATION_ERROR_SCORE, studentName, lineNumber);
                    closedir(dir);
                    return 1;
                }
                // execute the student's program, if the execution time takes more than
                // 5 seconds, write timeout error and grade 40 to result.csv.
                if (runExecutable(inputPath) == 1) {
                    resultsCsvHandler(TIMEOUT_SCORE, studentName, lineNumber);
                    closedir(dir);
                    return 1;
                }
                // compare the student's program output and the correct output using
                // comp.out which compares texts.
                compResult = runCompOut(SOLUTION_FILE_NAME, correctOutputPath);
                if (compResult == 1) {
                    //files are similar
                    resultsCsvHandler(GREAT_JOB_SCORE, studentName, lineNumber);
                    closedir(dir);
                    return 1;
                } else if (compResult == 3) {
                    //file are partially similar
                    resultsCsvHandler(SIMILAR_OUTPUT_SCORE, studentName, lineNumber);
                    closedir(dir);
                    return 1;
                    //if the files are different
                } else if (compResult == 2) {
                    resultsCsvHandler(BAD_OUTPUT_SCORE, studentName, lineNumber);
                    closedir(dir);
                    return 1;
                }
            }
        }
        closedir(dir);
    } else {
        errorPrint();
        return -1;
    }
    //return 0 if no c source file was found in the
    return 0;
}

/**
 * Function Name: lineParser
 * Function Input: char str[MAX_IN_LINE], int fdin
 * Function Output: void
 * Function Operation: the function reads a line from the given file descriptor
 *                     and stores it in the given str.
 */
int lineParser(char str[MAX_IN_LINE], int fdin) {
    int index = 0;
    char buffer[1];
    int firstRead;
    while ((firstRead = read(fdin, buffer, CHAR_SIZE) > 0 && buffer[0] != NEW_LINE_CHAR) > 0) {
        str[index] = buffer[0];
        index++;
    }
    if (firstRead < 0) {
        errorPrint();
        return -1;
    }
    return 0;
}

/**
 * Function Name: fromTxtToStruct
 * Function Input: ConfigurationData *configurationFile, char *args[]
 * Function Output: -1 if an error occurred
 *                   1 if function succeeded
 * Function Operation: the function stores each line of the 3 lines in the
 *                     configuration file into the correlate variable of the struct.
 */
int fromTxtToStruct(ConfigurationData *configurationFile, char *args[]) {
    int fdin = open(args[1], O_RDONLY, 0644);
    if (fdin < 0) {
        errorPrint();
        return -1;
    }
    // initialize the array with \0's
    memset(configurationFile->mainDirectoryPath, '\0', MAX_IN_LINE);
    memset(configurationFile->inputFilePath, '\0', MAX_IN_LINE);
    memset(configurationFile->correctOutputPath, '\0', MAX_IN_LINE);
    if ((lineParser(configurationFile->mainDirectoryPath, fdin)) == -1) {
        close(fdin);
        return -1;
    }
    if ((lineParser(configurationFile->inputFilePath, fdin)) == -1) {
        close(fdin);
        return -1;
    }
    if ((lineParser(configurationFile->correctOutputPath, fdin)) == -1) {
        close(fdin);
        return -1;
    }
    close(fdin);
    return 1;
}

/**
 * Function Name: unlinkFiles
 * Function Input: char *path
 * Function Output: void
 * Function Operation: the function deletes the file with the given path.
 */
void unlinkFiles(char *path) {
    if (unlink(path) == -1) { errorPrint(); }
}

/**
 * Function Name: main
 * Function Input: a configuration txt file path
 * Function Output: results.csv file
 * Function Operation: the function gets a path to a configuration txt file path that is
 *                     is consists of 3 lines: 1. a path to a students folder
 *                                             2. a path to an input txt file
 *                                             3. a path to a correct output.
 *                     the function appends (creates if results.csv is not exist) to
 *                     results.csv lines, each one indicates the student's name, his
 *                     score and a short description of his mistakes/why he got 100.
 */
int main(int argc, char *argv[]) {
    struct dirent *entry;
    DIR *dir;
    if (argc != 2) {
        printf(INVALID_INPUT);
        return 0;
    }
    // indicates the next line to which the program will write in results.csv file
    int lineNumber = 0;
    ConfigurationData configurationFile;
    if (fromTxtToStruct(&configurationFile, argv) == -1) { return 0; }
    char *studentsPath = configurationFile.mainDirectoryPath;
    char *inputPath = configurationFile.inputFilePath;
    char *correctOutputPath = configurationFile.correctOutputPath;
    // indicates how many c source files were found in each student folder
    int cFilesNum = 0;
    // looping through the students' folders
    if ((dir = opendir(studentsPath)) != NULL) {
        while ((entry = readdir(dir)) != NULL) {
            char *fileName = entry->d_name;
            if (entry->d_type == DT_DIR && strcmp(entry->d_name, ".") != 0 &&
                strcmp(fileName, "..") != 0) {
                char nextDirectory[MAX_PATH_LEN] = {0};
                // creating full path to students' folders
                fullPathConcatenate(nextDirectory, studentsPath, fileName);
                // scanning all the folders and files inside the given student's folder
                listDir(nextDirectory, inputPath, correctOutputPath, fileName, &cFilesNum,
                        &lineNumber);
                // if no source files were found at all, grade the student with 0
                if (cFilesNum == 0) {
                    resultsCsvHandler(NO_C_FILE_SCORE, fileName, &lineNumber);
                }
                cFilesNum = 0;
            }
        }
        //deleting the unnecessary files that were created through this program.
        unlinkFiles(SOLUTION_FILE_NAME);
        unlinkFiles(EXECUTABLE_NAME);
        closedir(dir);
    }
    return 0;
}
