// Author: Idan Twito
// ID: 311125249
#include <stdio.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <ctype.h>

#define ERROR_MSG "Error in system call\n"
#define ERROR_MSG_LENGTH 21
#define FILE_DESC 2
#define INVALID_INPUT "Invalid input\n"
#define CHAR_SIZE 1
#define SAME_LETTER_DIFF 32
#define UPPER_A 65
#define LOWER_A 97
#define UPPER_Z 90
#define LOWER_Z 122

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
 * Function Name: seekToBeginning
 * Function Input: int fdin - file descriptor
 * Function Output: if an error occurred return -1, else 1.
 * Function Operation: the function get a file descriptor and puts the cursor at the
 *                     beginning of the file
 */
int seekToBeginning(int fdin) {
    if ((lseek(fdin, 0, SEEK_SET)) < 0) {
        errorPrint();
        return -1;
    }
    return 1;
}

/**
 * Function Name: numOfChars
 * Function Input: int fdin - file descriptor
 * Function Output: -1 if an error occurred, otherwise return the characters amount in the file.
 * Function Operation: the function get a file descriptor and returns the number of the
 *                     the characters in the file.
 */
int numOfChars(int fdin) {
    int readChar;
    char buffer[1] = {0};
    int counter = 0;
    // as long as there are chars to read - counter++.
    while ((readChar = read(fdin, buffer, CHAR_SIZE)) > 0) {
        counter++;
    }
    //if an error occurred while trying to read from file return -1 indicates error
    if (readChar == -1) {
        errorPrint();
        return -1;
    }
    //if an error occurred while trying to get back to beginning of file return -1 indicates error
    if (seekToBeginning(fdin) == -1) {
        return -1;
    }
    return counter;
}

/**
 * Function Name: filesSimilarityCheck
 * Function Input: int firstFdin, int secondFdin - two file descriptors with the same amount
 *                 of chars.
 * Function Output: -1 if an error occurred
 *                   0 if the two files are not similar
 *                   1 if the two files are similar
 * Function Operation: the function gets two file descriptors and returns -1/0/1 as
 *                     as described above.
 */
int filesSimilarityCheck(int firstFdin, int secondFdin) {
    int firstReadChar;
    int secondReadChar;
    // get the current char of the first file that was read
    char firstBuffer[1] = {0};
    // get the current char of the second file that was read
    char secondBuffer[1] = {0};
    // read char in each file (the two files have the same amount of chars).
    while ((firstReadChar = read(firstFdin, firstBuffer, CHAR_SIZE)) > 0 && (secondReadChar = read(
            secondFdin, secondBuffer, CHAR_SIZE)) > 0) {
        if (firstBuffer[0] != secondBuffer[0]) {
            return 0;
        }
    }
    if (firstReadChar == -1 || secondReadChar == -1) {
        errorPrint();
        return -1;
    }
    // move the cursor to the beginning of both files.
    if (seekToBeginning(firstFdin) == -1 || seekToBeginning(secondFdin) == -1) {
        return -1;
    }
    return 1;
}

/**
 * Function Name: sensitiveCaseIgnoring
 * Function Input: char firstChar, char secondChar - two chars.
 * Function Output: true - if the letters are the same (ignoring sensitive case)
 *                  false - if the letters are not the same (ignoring sensitive case)
 * Function Operation: the function gets two chars and returns true of false as described above.
 */
bool sensitiveCaseIgnoring(char firstChar, char secondChar) {
    // if the first char is lower case and the second char is upper case:
    if ((UPPER_A <= firstChar) && (firstChar <= UPPER_Z) &&
        (LOWER_A <= secondChar) && (secondChar <= LOWER_Z)) {
        // and if the two chars are same letter, return true
        if (secondChar - firstChar == SAME_LETTER_DIFF) {
            return true;
        }
        // if the second char is lower case and the first char is upper case:
    } else if ((UPPER_A <= secondChar) && (secondChar <= UPPER_Z) &&
               (LOWER_A <= firstChar) && (firstChar <= LOWER_Z)) {
        // and if the two chars are same letter, return true
        if (firstChar - secondChar == SAME_LETTER_DIFF) {
            return true;
        }
    }
    return false;
}

/**
 * Function Name: filesPartialSimilarityCheck
 * Function Input: int firstFdin, int secondFdin - two file descriptors
 * Function Output: -1 if an error occurred
 *                   0 if the two files are not partially similar
 *                   1 if the two files are partially similar
 * Function Operation: the function gets two file descriptors and returns -1/0/1 as
 *                     as described above.
 */
int filesPartialSimilarityCheck(int firstFdin, int secondFdin) {
    int firstReadChar;
    int secondReadChar;
    char firstBuffer[1] = {0};
    char secondBuffer[1] = {0};
    char currentFirstChar;
    char currentSecondChar;
    bool shouldRun = true;
    bool firstFileIsShorter = false;
    bool secondFileIsShorter = false;
    // if gets false then return 0.
    bool shouldReturnOne = true;
    while (shouldRun) {
        // read a character in each file
        firstReadChar = read(firstFdin, firstBuffer, CHAR_SIZE);
        secondReadChar = read(secondFdin, secondBuffer, CHAR_SIZE);
        currentFirstChar = firstBuffer[0];
        currentSecondChar = secondBuffer[0];
        // if one file has no has no more chars to read mark it as shorter, if both
        // of the file reached to the end - break
        if (firstReadChar == 0 || secondReadChar == 0) {
            if (firstReadChar == 0 && secondReadChar != 0) { firstFileIsShorter = true; }
            else if (firstReadChar != 0) { secondFileIsShorter = true; }
            break;
        }// skip all the white spaces in the first file
        while (isspace(currentFirstChar) != 0 && firstReadChar != 0) {
            firstReadChar = read(firstFdin, firstBuffer, CHAR_SIZE);
            currentFirstChar = firstBuffer[0];
        }// skip all the white spaces in the second file
        while (isspace(currentSecondChar) != 0 && secondReadChar != 0) {
            secondReadChar = read(secondFdin, secondBuffer, CHAR_SIZE);
            currentSecondChar = secondBuffer[0];
        }// check if the letters are the same or the same but differs with lower/upper case
        if (currentFirstChar == currentSecondChar ||
            sensitiveCaseIgnoring(currentFirstChar, currentSecondChar)) {
            continue;
            // if the previous case is false the return to beginning of file and return 0
        } else {
            seekToBeginning(firstFdin);
            seekToBeginning(secondFdin);
            return 0;
        }
    }
    // if the first file is shorter and the only char that were not read are white space
    // then return 1, else 0.
    if (firstFileIsShorter) {
        while ((secondReadChar = read(secondFdin, secondBuffer, CHAR_SIZE)) > 0) {
            if (!(isspace(secondBuffer[0]))) {
                shouldReturnOne = false;
            }
        }
        // if the second file is shorter and the only char that were not read are white space
        // then return 1, else 0.
    } else if (secondFileIsShorter) {
        while ((firstReadChar = read(firstFdin, firstBuffer, CHAR_SIZE)) > 0) {
            if (!(isspace(firstBuffer[0]))) {
                shouldReturnOne = false;
            }
        }
    }
    if (firstReadChar == -1 || secondReadChar == -1) {
        errorPrint();
        return -1;
    }
    seekToBeginning(firstFdin);
    seekToBeginning(secondFdin);
    if (shouldReturnOne) {
        return 1;
    }
    return 0;
}

/**
 * Function Name: main
 * Function Input: 2 file paths in the argument line
 * Function Output: 0 - if an error occurred
 *                  1 - if the files are similar
 *                  2 - if the files are different
 *                  3 - if the files are partially similar
 * Function Operation:
 *
 */
int main(int argc, char *argv[]) {
    // file descriptor of first file
    int firstFdin;
    // file descriptor of second file
    int secondFdin;
    // gets the chars amount of the first file
    int firstFileSize = 0;
    // gets the chars amount of the second file
    int secondFileSize = 0;
    int similarityCheckResult;
    int partialSimilarityResult;
    // if gets false then the first and second file have different amount of chars
    bool sameSizeFiles = true;
    // if the program didn't get exactly 2 paths then print error and return 0
    if (argc != 3) {
        printf(INVALID_INPUT);
        return 0;
    }
    // open the 2 given files
    firstFdin = open(argv[1], O_RDONLY);
    secondFdin = open(argv[2], O_RDONLY);
    if (firstFdin < 0 || secondFdin < 0) {
        errorPrint();
        if (firstFdin >= 0) { close(firstFdin); }
        if (secondFdin >= 0) { close(secondFdin); }
        return 0;
    }
    // get the chars amount of each file
    firstFileSize = numOfChars(firstFdin);
    secondFileSize = numOfChars(secondFdin);
    if (firstFileSize != secondFileSize) {
        sameSizeFiles = false;
    }
    // if the chars sizes of both files similar, then they can be similar.
    if (sameSizeFiles) {
        // if both files are similar then return 1, if an error occurred return 0
        similarityCheckResult = filesSimilarityCheck(firstFdin, secondFdin);
        if (similarityCheckResult == 1) {
            close(firstFdin);
            close(secondFdin);
            return 1;
        } else if (similarityCheckResult == -1) {
            close(firstFdin);
            close(secondFdin);
            return 0;
        }
    }
    // if both files are partial similar then return 3, if an error occurred return 0
    // else, return 2 because the files are different
    partialSimilarityResult = filesPartialSimilarityCheck(firstFdin, secondFdin);
    if (partialSimilarityResult == 1) {
        close(firstFdin);
        close(secondFdin);
        return 3;
        // in that case an error occurred while checking partial similarity
    } else if (partialSimilarityResult == -1) {
        close(firstFdin);
        close(secondFdin);
        return 0;
        // in any other case return 2 - files are different
    } else {
        close(firstFdin);
        close(secondFdin);
        return 2;
    }
}
