#include <stdio.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <assert.h>
#include "read_program.h"

const size_t MAX_BUF_SIZE = 2048;

//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
// Creating buffer for reading program info from disk

static bool is_stat_err(const char *name_of_file, struct stat *all_info_about_file);

char* read_file_to_string_array(const char *name_of_file){
    assert(name_of_file != NULL);
    FILE *fptr = fopen(name_of_file, "r");
    if(!fptr){
        fprintf(stderr, "Can't open file\n");
        return NULL;
    }

    struct stat file_info = {};
    if(is_stat_err(name_of_file, &(file_info))){
        return NULL;
    }

    char *all_strings_in_file = (char *)calloc(file_info.st_size + 1, sizeof(char));
    if(!all_strings_in_file){
        fprintf(stderr, "Array for strings allocation error\n");
        return NULL;
    }

    if(fread(all_strings_in_file, sizeof(char), file_info.st_size, fptr) != file_info.st_size){
        fprintf(stderr, "Can't read all symbols from file\n");
        return NULL;
    }

    fclose(fptr);
    return all_strings_in_file;
}

static bool is_stat_err(const char *name_of_file, struct stat *all_info_about_file){
    assert(name_of_file != NULL);
    assert(all_info_about_file != NULL);

    if (stat(name_of_file, all_info_about_file) == -1){
        perror("Stat error");
        fprintf(stderr, "Error code: %d\n", errno);
        return true;
    }
    return false;
}

static void buffer_free(char* buffer){
    if(buffer){
        memset(buffer, 0, strlen(buffer));
        free(buffer);
    }
}
