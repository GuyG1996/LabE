#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <elf.h>

#define MAX_VARIABLES 10

typedef struct {
  char file_name[128];
  off_t file_size;
  int fd;
  Elf32_Ehdr * map_start;
  /*
   .
   .
   Any additional fields you deem necessary
  */
} ELF_info;

// Function prototypes
void toggleDebugMode();
void ExamineELFFile();
void PrintSectionNames();
void PrintSymbols();
void CheckFilesforMerge();
void MergeELFFiles();
void quit();


// Struct for menu options
struct fun_desc {
    char* name;
    void (*MenuFunction)();
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
ELF_info ELFArray[MAX_VARIABLES]; // Global array of 10 structs
int ELF_info_index = 0;


int main() {
    for (int j = 0; j < MAX_VARIABLES; j++) {// init the fd value to be -1
        ELFArray[j].fd = -1;
    }
    while (true) {
        // Print debug information
        if (debug_mode) {
            fprintf(stderr, "Debug Mode Is Now ON");
            // fprintf(stderr, "unit_size: %d, file_name: %s, mem_count: %zu\n", 
                // program_state.unit_size, program_state.file_name, program_state.mem_count);
        }
        else{
            fprintf(stderr, "Debug Mode Is Now OFF");
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
        }
        else {
            debug_mode = true;
        }
    }

//choose 1:
void ExamineELFFile(){
    printf("Please enter an ELF file: \n");
        scanf("%127s", ELFArray[ELF_info_index].file_name);
        ELFArray[ELF_info_index].fd = open(ELFArray[ELF_info_index].file_name, O_RDONLY);

        // if there was an error opening the file
        if (ELFArray[ELF_info_index].fd == -1) {
            perror("Error opening file");
            return;
        }

        ELFArray[ELF_info_index].file_size = lseek(ELFArray[ELF_info_index].fd, 0, SEEK_END); // the size of the file
        ELFArray[ELF_info_index].map_start = mmap(NULL, ELFArray[ELF_info_index].file_size
         , PROT_READ, MAP_PRIVATE, ELFArray[ELF_info_index].fd, 0); //mapping the file

        // if there was an error mapping the file
        if (ELFArray[ELF_info_index].map_start == MAP_FAILED) {
            perror("Error mapping file to memory");
            close(ELFArray[ELF_info_index].fd);
            ELFArray[ELF_info_index].fd = -1;
            return;
        }

        // if the mapping was successful
        char magic_number[3];
        magic_number[0] = ((unsigned char*)ELFArray[ELF_info_index].map_start)[1];
        magic_number[1] = ((unsigned char*)ELFArray[ELF_info_index].map_start)[2];
        magic_number[2] = ((unsigned char*)ELFArray[ELF_info_index].map_start)[3];

        printf("1. Magic number (ASCII):\t\t %c%c%c\n", magic_number[0], magic_number[1], magic_number[2]);

        // check if the Bytes 1,2,3 is matching an ELF file Bytes(in ascii)
        if((char)magic_number[0] != 'E' || (char)magic_number[1] != 'L' || (char)magic_number[2] != 'F'){
            perror("Error: This is not an ELF file\n");
            close(ELFArray[ELF_info_index].fd);
            munmap(ELFArray[ELF_info_index].map_start, ELFArray[ELF_info_index].file_size);
            ELFArray[ELF_info_index].fd = -1;
            return;
        }

        // if it is an ELF file
        else{
            printf("2. Data:\t\t\t\t");

            // Check the data encoding type
            switch (ELFArray[ELF_info_index].map_start->e_ident[EI_DATA]) {
                case ELFDATA2LSB:
                    printf(" 2's complement, little endian\n");
                    break;
                case ELFDATA2MSB:
                    printf(" 2's complement, big endian\n");
                    break;
                default:
                    printf(" Unknown data format\n");
                    break;
                }
            printf("3. Entry point (hexadecimal):\t\t 0x%08X\n", ELFArray[ELF_info_index].map_start->e_entry);
            // Print the section header table offset
            printf("4. Section header table offset:\t\t %d\n", ELFArray[ELF_info_index].map_start->e_shoff);

            // Print the number of section header entries
            printf("5. Number of section header entries:\t %u\n", ELFArray[ELF_info_index].map_start->e_shnum);

            // Print the size of each section header entry
            printf("6. Size of each section header entry:\t %u\n", ELFArray[ELF_info_index].map_start->e_shentsize);

            // Print the program header table offset
            printf("7. Program header table offset:\t\t %d\n", ELFArray[ELF_info_index].map_start->e_phoff);

            // Print the number of program header entries
            printf("8. Number of program header entries:\t %u\n", ELFArray[ELF_info_index].map_start->e_phnum);

            // Print the size of each program header entry
            printf("9. Size of each program header entry:\t %u\n", ELFArray[ELF_info_index].map_start->e_phentsize);

            ELF_info_index++;
        if(ELF_info_index == 10)
            ELF_info_index = 0;
        }
        
        

        

}
// help function for get the section type name
const char* getSectionTypeName(int section_type) {
    switch (section_type) {
        case 0:
            return "NULL";
        case 1:
            return "PROGBITS";
        case 2:
            return "SYMTAB";
        case 3:
            return "STRTAB";
        case 4:
            return "RELA";
        case 5:
            return "HASH";
        case 6:
            return "DYNAMIC";
        case 7:
            return "NOTE";
        case 8:
            return "NOBITS";
        case 9:
            return "REL";
        case 10:
            return "SHLIB";
        case 11:
            return "DYNSYM";
        default:
            return "UNKNOWN";
    }
}

//choose 2:
void PrintSectionNames(){
    int count = 0;
    for(int i = 0; i<MAX_VARIABLES; i++){
        if(ELFArray[i].fd != -1){
            printf("File Name: %s \n", ELFArray[i].file_name);
            count++;
            Elf32_Ehdr* elf_header = ELFArray[i].map_start;
            Elf32_Shdr* section_header_table = (Elf32_Shdr*)((char*)ELFArray[i].map_start + elf_header->e_shoff);

            int section_header_count = elf_header->e_shnum;
            int string_table_index = elf_header->e_shstrndx;
            Elf32_Shdr* string_table_header = &section_header_table[string_table_index];
            char* string_table = (char*)ELFArray[i].map_start + string_table_header->sh_offset;

            printf("[index]\tsection_name\tsection_address\tsection_offset\tsection_size\tsection_type\tshstrndx\tOffset\n");

            for (int j = 0; j < section_header_count; j++) {
                Elf32_Shdr* section_header = &section_header_table[j];
                char* section_name = &string_table[section_header->sh_name];
                int section_name_offset = elf_header->e_shoff + section_header->sh_name;
                if(!debug_mode){
                    printf("[%2d]\t%-15s\t0x%08X\t0x%08X\t%06X\t\t%s\n", j, section_name, section_header->sh_addr,
                       section_header->sh_offset, section_header->sh_size, getSectionTypeName(section_header->sh_type));
                }
                else{
                   printf("[%2d]\t%-15s\t0x%08X\t0x%08X\t%06X\t\t%-15s\t%-8d\t%d\n", j, section_name,
                        section_header->sh_addr, section_header->sh_offset, section_header->sh_size,
                        getSectionTypeName(section_header->sh_type), elf_header->e_shstrndx, section_name_offset);
                }  
            }
            printf("\n");
        }
    }
    if(count ==0){
        perror("Error: There are no file currently mapped\n");
        return;
    }



   
    

}

void PrintSymbols(){

}

void CheckFilesforMerge(){

}

void MergeELFFiles(){}
void quit(){}