#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#define MAX_VARIABLES 10

typedef struct {
  char file_name[128];
  off_t file_size;
  int fd;
  void* map_start;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} ELf_info;


// Struct for menu options
struct fun_desc {
    char* name;
    void (*MenuFunction)(state*);
};

// Menu options and corresponding function pointers
struct fun_desc menu[] = {
    {"Toggle Debug Mode", toggleDebugMode},
    {"Examine ELF File", ExamineELFFile},
    {"Print Section Names", PrintSectionNames},
    {"Print Symbols", PrintSymbols},
    {"Check Files for Merge", CheckFilesforMerge},
    {"Merge ELF Files", MergeELFFiles},
    {"Quit", quit},
    {NULL, NULL}
};

bool debug_mode = true;
Elf_info ELFArray[MAX_VARIABLES]; // Global array of 10 structs
int ELF_info_index = 0;
int main() {
    while (true) {
        // Print debug information
        if (debug_mode) {
            fprintf(stderr, "Debug Mode Is Now On");
            // fprintf(stderr, "unit_size: %d, file_name: %s, mem_count: %zu\n", 
                // program_state.unit_size, program_state.file_name, program_state.mem_count);
        }

        // Print menu
        int optionCount = 0;
        printf("\nChoose action:\n");
        while (menu[optionCount].name != NULL) {
            printf("%d-%s\n", optionCount, menu[optionCount].name);
            optionCount++;
        }

        // Obtain user choice
        int choice;
        printf("> ");
        scanf("%d", &choice);

        // Check if choice is "quit"
        if (choice == 6) {
            // quit(&program_state);
            break;
        }

        // Validate choice
        if (choice < 0 || choice >= optionCount) {
            printf("Invalid choice. Please try again.\n");
            continue;
        }
        
        // Execute chosen function
        menu[choice].MenuFunction();
    }
    return 0;
}

//choose 0:
 void toggleDebugMode() {
        if (debug_mode) {
            debug_mode = false;
            fprintf(stderr, "Debug flag now off\n");
        }
        else {
            debug_mode = true;
            fprintf(stderr, "Debug flag now on\n");
        }
    }

void ExamineELFFile(){
    fprintf("Please enter an ELF file: \n");
        scanf("%127s", ELFArray[ELF_info_index]->file_name);
        ELFArray[ELF_info_index]-> fd = open(file_name, O_RDONLY);

        // if there was an error opening the file
        if (fd == -1) {
            perror("Error opening file");
            return 1;
        }

        ELFArray[ELF_info_index]-> file_size = lseek(fd, 0, SEEK_END); // the size of the file
        ELFArray[ELF_info_index] -> map_start = mmap(NULL, ELFArray[ELF_info_index]-> file_size
         , PROT_READ, MAP_PRIVATE, ELFArray[ELF_info_index]-> fd, 0); //mapping ths file

        // if there was an error mapping the file
        if (ELFArray[ELF_info_index]-> map_start == MAP_FAILED) {
            perror("Error mapping file to memory");
            ELFArray[ELF_info_index]-> fd = -1;
            return 1;
        }

        // if the mapping was successful
        char* magic_number[3];
        magic_number[0] = ELFArray[ELF_info_index] -> map_start[1];
        magic_number[1] = ELFArray[ELF_info_index] -> map_start[2];
        magic_number[2] = ELFArray[ELF_info_index] -> map_start[3];

        // check if the Bytes 1,2,3 is matching an ELF file Bytes(in ascii)
        if(magic_number[1] != 'E' || magic_number[2] != 'L' || magic_number[3] == 'F'){
            perror("Error: This is not an ELF file");
            close(ELFArray[ELF_info_index]-> fd);
            munmap(ELFArray[ELF_info_index]-> map_start, ELFArray[ELF_info_index]-> file_size);
            ELFArray[ELF_info_index]-> fd = -1;
            return 1;
        }

        // if it is an ELF file
        













        ELF_info_index++;
        if(ELF_info_index == 10)
            ELF_info_index = 0;

        

}
