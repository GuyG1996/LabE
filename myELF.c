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
            fprintf(stderr, "Debug Mode Is Now ON\n");
        }
        else{
            fprintf(stderr, "Debug Mode Is Now OFF\n");
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
            quit();
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
            printf("Error opening file\n");
            return;
        }

        ELFArray[ELF_info_index].file_size = lseek(ELFArray[ELF_info_index].fd, 0, SEEK_END); // the size of the file
        ELFArray[ELF_info_index].map_start = mmap(NULL, ELFArray[ELF_info_index].file_size
         , PROT_READ, MAP_PRIVATE, ELFArray[ELF_info_index].fd, 0); //mapping the file

        // if there was an error mapping the file
        if (ELFArray[ELF_info_index].map_start == MAP_FAILED) {
            printf("Error mapping file to memory\n");
            close(ELFArray[ELF_info_index].fd);
            ELFArray[ELF_info_index].fd = -1;
            return;
        }

        // if the mapping was successful
        char magic_number[3];
        magic_number[0] = ((unsigned char*)ELFArray[ELF_info_index].map_start)[1];
        magic_number[1] = ((unsigned char*)ELFArray[ELF_info_index].map_start)[2];
        magic_number[2] = ((unsigned char*)ELFArray[ELF_info_index].map_start)[3];

        printf("\n%s ELF Header: \n", ELFArray[ELF_info_index].file_name);
        printf("1. Magic number (ASCII):\t\t %c%c%c\n", magic_number[0], magic_number[1], magic_number[2]);

        // check if the Bytes 1,2,3 is matching an ELF file Bytes(in ascii)
        if((char)magic_number[0] != 'E' || (char)magic_number[1] != 'L' || (char)magic_number[2] != 'F'){
            printf("Error: This is not an ELF file\n");
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
// helper function for get the section type name when you choose 2
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
            printf("File Name %s Section Headers:\n", ELFArray[i].file_name);
            count++;
            Elf32_Ehdr* elf_header = ELFArray[i].map_start;
            Elf32_Shdr* section_header_table = (Elf32_Shdr*)((char*)ELFArray[i].map_start + elf_header->e_shoff);

            int section_header_count = elf_header->e_shnum;
            int string_table_index = elf_header->e_shstrndx;
            Elf32_Shdr* string_table_header = &section_header_table[string_table_index];
            char* string_table = (char*)ELFArray[i].map_start + string_table_header->sh_offset;

            printf("\n[index]\tsection_name\tsection_address\tsection_offset\tsection_size\tsection_type\tshstrndx\tsection_name_offset\n");

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
        printf("Error: There are no file currently mapped\n");
        return;
    }
}

// helper function for printing the symbols tables when you choose 3
void PrintSymbolTable(Elf32_Shdr* section_header, int num_symbols, Elf32_Shdr* section_header_table, int index) {
    printf("[Index]\tValue\t\tSection_Index\tSection_Name\tSymbol_Name\n");
    
    Elf32_Ehdr* elf_start = ELFArray[index].map_start;
    Elf32_Sym* symbols = (Elf32_Sym*)((char*)elf_start + section_header->sh_offset);
    Elf32_Shdr* strtab_section = &section_header_table[section_header->sh_link];
    char* string_table = (char*)elf_start + strtab_section->sh_offset;

    // char* curr_sectionHeader_name = map + str_table->sh_offset + curr_sectionHeader_entry->sh_name;

    // Iterate over the symbols and print their information
    for (int i = 0; i < num_symbols; i++) {
        Elf32_Sym* symbol = &symbols[i];
        char* symbol_name = string_table + symbol->st_name;
        
        // Retrieve the section name
        Elf32_Shdr* symbol_section = &section_header_table[symbol->st_shndx];
        char* section_name;
        
        if (symbol->st_shndx == SHN_ABS) {
            section_name = "ABS";
        } else if (symbol->st_shndx == SHN_UNDEF) {
            section_name = "UND";
        } else {
            section_name = string_table + symbol_section->sh_name;
        }

        // printf("Symbol Section Name: %s\n", section_name);
        printf("[%d]\t%08x\t%d\t\t%-10s\t%s\n", i, symbol->st_value, symbol->st_shndx, section_name, symbol_name);
    }
    printf("\n");
}

//choose 3:
void PrintSymbols(){
    int count = 0;
    for(int i = 0; i<MAX_VARIABLES; i++){
        if(ELFArray[i].fd != -1){
            printf("File %s\n", ELFArray[i].file_name);
            count++;
            Elf32_Ehdr* elf_header = ELFArray[i].map_start;
            Elf32_Shdr* section_header_table = (Elf32_Shdr*)((char*)elf_header + elf_header->e_shoff);

            // Find the dynamic symbol table (.dynsym) and symbol table (.symtab)
            Elf32_Shdr* dynsym_section = NULL;
            Elf32_Shdr* symtab_section = NULL;

            // Find the symbol table sections and calculate sizes and number of symbols
            int dynsym_size = 0;
            int dynsym_num_symbols = 0;
            int symtab_size = 0;
            int symtab_num_symbols = 0;

            for (int j = 0; j < elf_header->e_shnum; j++) {
                Elf32_Shdr* section_header = &section_header_table[j];
                if (section_header->sh_type == SHT_DYNSYM) {
                    dynsym_size = section_header->sh_size;
                    dynsym_num_symbols = section_header->sh_size / sizeof(Elf32_Sym);
                    dynsym_section = section_header;
                }

                if (section_header->sh_type == SHT_SYMTAB) {
                    symtab_size = section_header->sh_size;
                    symtab_num_symbols = section_header->sh_size / sizeof(Elf32_Sym);
                    symtab_section = section_header;
                }
            }

            // Print error if no symbol tables found
            if (dynsym_section == NULL && symtab_section == NULL) {
                printf("No symbol tables found.\n");
                return;
            }

            // Print symbols if dynamic symbol table (.dynsym) exists
            if (dynsym_section != NULL) {
                if(debug_mode){
                    printf("Symbol table '.dynsym', size: %06X, contains %d entries:\n", dynsym_size, dynsym_num_symbols);
                    // Print any other useful information in debug mode
                }
                PrintSymbolTable(dynsym_section, dynsym_num_symbols, section_header_table, i);
            }

            // Print symbols if symbol table (.symtab) exists
            if (symtab_section != NULL) {
                if(debug_mode){
                    printf("Symbol table'.symtab', size: %06X, contains %d entries:\n", symtab_size, symtab_num_symbols);
                    // Print any other useful information in debug mode
                }
                PrintSymbolTable(symtab_section, symtab_num_symbols, section_header_table, i);
            }
        }
    }
    if(count == 0){
        printf("Error: There are no file currently mapped\n");
        return;
    }
}

// choose 4:
void CheckFilesforMerge(){
    int lastIndexes[2];
    if(ELF_info_index > 1){
        lastIndexes[0] = ELF_info_index - 2;
        lastIndexes[1] = ELF_info_index - 1;
    }
    else if(ELF_info_index == 1){
        lastIndexes[0] = MAX_VARIABLES - 1;
        lastIndexes[1] = ELF_info_index -1;
        
    }
    else{
        lastIndexes[0] = MAX_VARIABLES - 2;
        lastIndexes[1] = MAX_VARIABLES - 1;
    }

    int fd1 = ELFArray[lastIndexes[0]].fd;
    int fd2 = ELFArray[lastIndexes[1]].fd;
    if(fd1 == -1 || fd2 == -1){
        printf("there are no 2 ELF files that have been opened and mapped\n");
        return;
    }
    Elf32_Ehdr* elf_header1 = ELFArray[lastIndexes[0]].map_start;
    Elf32_Ehdr* elf_header2 = ELFArray[lastIndexes[1]].map_start;
    Elf32_Shdr* section_header_table1 = (Elf32_Shdr*)((char*)elf_header1 + elf_header1->e_shoff);
    Elf32_Shdr* section_header_table2 = (Elf32_Shdr*)((char*)elf_header2 + elf_header2->e_shoff);
    Elf32_Shdr* symtab_section1 = NULL;
    Elf32_Shdr* symtab_section2 = NULL;
    char* string_table1 = NULL;
    char* string_table2 = NULL;
    int i,j;
    int count1 = 0;
    int count2 = 0;

    for(i = 0;i < elf_header1->e_shnum;i++ ){
        Elf32_Shdr* section_header1 = &section_header_table1[i];
        if (section_header1->sh_type == SHT_SYMTAB) {
            count1++;
            if(count1 > 1){
                printf("feature not supported\n");
                return;
            }
            symtab_section1 = section_header1;
            string_table1 = (char*)((char*)elf_header1 + section_header_table1[symtab_section1->sh_link].sh_offset);
        }
    }
    
    for(i = 0;i < elf_header2->e_shnum;i++ ){
        Elf32_Shdr* section_header2 = &section_header_table2[i];
        if (section_header2->sh_type == SHT_SYMTAB) {
            count2++;
            if(count2 > 1){
                printf("feature not supported\n");
                return;
            }
            symtab_section2 = section_header2;
            string_table2 = (char*)((char*)elf_header2 + section_header_table2[symtab_section2->sh_link].sh_offset);
        }
    }

    if(count1 != 1 || count2 != 1){
        printf("feature not supported\n");
        return;
    }
    
    Elf32_Sym* symbols1 = (Elf32_Sym*)((char*)ELFArray[lastIndexes[0]].map_start + symtab_section1->sh_offset);
    Elf32_Sym* symbols2 = (Elf32_Sym*)((char*)ELFArray[lastIndexes[1]].map_start + symtab_section2->sh_offset);
    for(i = 1; i < symtab_section1->sh_size / sizeof(Elf32_Sym); i++ ){
        
        Elf32_Sym sym1 = symbols1[i];
        if(sym1.st_name == 0)
            continue;

        char* symbol_name1 = string_table1 + sym1.st_name;
        int countmetched1 = 0;

        // Check if the symbols is DEFINED:
        bool syn1_defined = (sym1.st_shndx != SHN_UNDEF);;
        
        
        for(j = 1; j < symtab_section2->sh_size / sizeof(Elf32_Sym);j++){
            
            Elf32_Sym sym2 = symbols2[j];
            if(sym2.st_name == 0)
                continue;

            char* symbol_name2 = string_table2 + sym2.st_name;
            if(strcmp(symbol_name1, symbol_name2) == 0){
                countmetched1++;
                if((sym2.st_shndx == SHN_UNDEF) && !syn1_defined){
                    printf("Symbol sym undefined1 with %s and %s\n", symbol_name1, symbol_name2);
                }
                else if((sym2.st_shndx != SHN_UNDEF) && syn1_defined){
                    printf("Symbol sym multiply defined2 with %s and %s \n",symbol_name1, symbol_name2);
                }
            }
        }
        if((countmetched1 < 1) & !syn1_defined){
            printf("Symbol sym undefined3 with %s\n", symbol_name1);
        }
    }

    // check for symbols from the second table:
    for(i = 1; i < symtab_section2->sh_size / sizeof(Elf32_Sym); i++ ){
        
        Elf32_Sym sym2 = symbols2[i];
        if(sym2.st_name == 0)
            continue;

        char* symbol_name2 = string_table2 + sym2.st_name;
        int countmetched2 = 0;
        bool syn2_defined = false;

        // Check if the symbols is DEFINED:
        if(sym2.st_shndx != SHN_UNDEF){
            syn2_defined = true;
        }

        for(j = 1; j < symtab_section1->sh_size / sizeof(Elf32_Sym);j++){
            
            Elf32_Sym sym1 = symbols1[j];
            if(sym1.st_name == 0)
                continue;
            char* symbol_name1 = string_table1 + sym1.st_name;
            if(strcmp(symbol_name2, symbol_name1) == 0){
                countmetched2++;
                if((sym1.st_shndx == SHN_UNDEF) && !syn2_defined){
                    printf("Symbol sym undefined4 with %s and %s\n", symbol_name1, symbol_name2);
                }
                else if((sym1.st_shndx != SHN_UNDEF) && syn2_defined){
                    printf("Symbol sym multiply defined5 with %s and %s \n",symbol_name1, symbol_name2);
                }
            }
        }
        if((countmetched2 < 1) & !syn2_defined){
            printf("Symbol sym undefined6 with %s\n", symbol_name2);
        }
    }
}

// choose 5:
void MergeELFFiles(){
    int lastIndexes[2];
    if(ELF_info_index > 1){
        lastIndexes[0] = ELF_info_index - 2;
        lastIndexes[1] = ELF_info_index - 1;
    }
    else if(ELF_info_index == 1){
        lastIndexes[0] = MAX_VARIABLES - 1;
        lastIndexes[1] = ELF_info_index -1;
        
    }
    else{
        lastIndexes[0] = MAX_VARIABLES - 2;
        lastIndexes[1] = MAX_VARIABLES - 1;
    }

    int fd1 = ELFArray[lastIndexes[0]].fd;
    int fd2 = ELFArray[lastIndexes[1]].fd;
    if(fd1 == -1 || fd2 == -1){
        printf("there are no 2 ELF files that have been opened and mapped to merge\n");
        return;
    }
    Elf32_Ehdr* elf_header1 = ELFArray[lastIndexes[0]].map_start;
    Elf32_Ehdr* elf_header2 = ELFArray[lastIndexes[1]].map_start;

    // Create the output ELF file
    int fd_out = open("out.ro", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd_out == -1) {
        printf("Error creating output ELF file\n");
        return;
    }

    // Copy the ELF header from the first file to the output file
    if (write(fd_out, elf_header1, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error writing ELF header\n");
        close(fd_out);
        return;
    }
    Elf32_Shdr* section_header_table1 = (Elf32_Shdr*)((char*)elf_header1 + elf_header1->e_shoff);
    Elf32_Shdr* section_header_table2 = (Elf32_Shdr*)((char*)elf_header2 + elf_header2->e_shoff);
    size_t section_header_table_size = sizeof(Elf32_Shdr) * elf_header1->e_shnum;

    // Copy the section header table of the first file to the output file
    if (write(fd_out, section_header_table1, section_header_table_size) != section_header_table_size) {
        printf("Error writing section header table\n");
        close(fd_out);
        return;
    }

    // Loop over the entries of the new section header table
    for (int i = 0; i < elf_header1->e_shnum; i++) {
        Elf32_Shdr* section_header1 = &section_header_table1[i];
        // Process each section according to the merging scheme
        lseek(fd1, section_header_table1[i].sh_offset, SEEK_SET);
        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(fd1, buffer, sizeof(buffer))) > 0) {
            if (write(fd_out, buffer, bytes_read) != bytes_read) {
                printf("Error writing merged section\n");
                close(fd_out);
                return;
            }
        }
        
        // if it is one of those four enter the second loop
        if(strcmp((char*)section_header_table1[i].sh_name, ".text") == 0 ||
            strcmp((char*)section_header_table1[i].sh_name, ".rodata") == 0 ||
            strcmp((char*)section_header_table1[i].sh_name, ".data") == 0 || 
            strcmp((char*)section_header_table1[i].sh_name, ".symtab") == 0){
        
            // Find the corresponding section in the second file
            for (int j = 0; j < elf_header2->e_shnum; j++) {
                Elf32_Shdr* section_header2 = &section_header_table2[j];
                if (strcmp((char*)section_header_table2[j].sh_name, (char*)section_header_table1[i].sh_name) == 0) {
                    if(strcmp((char*)section_header_table1[i].sh_name, ".symtab") == 0){

                        Elf32_Sym* symbol_table1 = (Elf32_Sym*)((char*)elf_header1 + section_header_table1[i].sh_offset);
                        int symbol_table_size1 = section_header_table1[i].sh_size / sizeof(Elf32_Sym);

                        Elf32_Sym* symbol_table2 = (Elf32_Sym*)((char*)elf_header2 + section_header_table2[j].sh_offset);
                        int symbol_table_size2 = section_header_table2[i].sh_size / sizeof(Elf32_Sym);
                        for(int k = 0;k < symbol_table_size1; k++){
                            Elf32_Sym sym1 = symbol_table1[k];
                            if((sym1.st_name == 0) | (sym1.st_shndx != SHN_UNDEF)){
                                continue;
                            }
                            char* string_table1 = (char*)((char*)elf_header1 + section_header_table1[section_header1->sh_link].sh_offset);
                            char* symbol_name1 = string_table1 + sym1.st_name;
                                                        
                            for(int p = 0;p < symbol_table_size2; p++){
                                Elf32_Sym sym2 = symbol_table2[p];
                                char* string_table2 = (char*)((char*)elf_header2 + section_header_table2[section_header2->sh_link].sh_offset);
                                char* symbol_name2 = string_table2 + sym2.st_name;
                                if(strcmp(symbol_name2, symbol_name1) == 0){
                                    // Update the symbol entry in the first file with the details from the second file
                                    symbol_table1[k].st_value = symbol_table2[p].st_value;
                                    symbol_table1[k].st_size = symbol_table2[p].st_size;
                                    symbol_table1[k].st_info = symbol_table2[p].st_info;
                                    symbol_table1[k].st_other = symbol_table2[p].st_other;
                                    symbol_table1[k].st_shndx = symbol_table2[p].st_shndx;
                                    break;
                                }
                            }
                        }
                    }
                    else{
                        // Append the relevant section from the second file
                        lseek(fd2, section_header_table2[j].sh_offset, SEEK_SET);
                        while ((bytes_read = read(fd2, buffer, sizeof(buffer))) > 0) {
                            if (write(fd_out, buffer, bytes_read) != bytes_read) {
                                printf("Error writing merged section\n");
                                close(fd_out);
                                return;
                            }
                        }
                    }

                    // Update the section header table entry for the merged section
                    section_header_table1[i].sh_size += section_header_table2[j].sh_size;
                    section_header_table1[i].sh_offset = lseek(fd_out, 0, SEEK_CUR) - section_header_table1[i].sh_size;
                    if (write(fd_out, &section_header_table1[i], sizeof(Elf32_Shdr)) != sizeof(Elf32_Shdr)) {
                        printf("Error writing merged section header\n");
                        close(fd_out);
                        return;
                    }
                    break; // Found the relevant section in the second file, exit the loop
                }
            }
        }
    }
    // Fix the "e_shoff" field in the ELF header
    elf_header1->e_shoff = lseek(fd_out, 0, SEEK_CUR);
    lseek(fd_out, 0, SEEK_SET);
    if (write(fd_out, elf_header1, sizeof(Elf32_Ehdr)) != sizeof(Elf32_Ehdr)) {
        printf("Error writing updated ELF header\n");
        close(fd_out);
        return;
    }

    // Close the output ELF file
    close(fd_out);

    printf("Merged ELF files successfully\n");
}

// choose 6:
void quit(){
    // Unmap and close mapped files
    for (int i = 0; i < MAX_VARIABLES; i++) {
        if (ELFArray[i].fd != -1) {
            // Unmap the file
            if (ELFArray[i].map_start != MAP_FAILED) {
                if (munmap(ELFArray[i].map_start, ELFArray[i].file_size) == 0) {
                    printf("Unmapped file: %s\n", ELFArray[i].file_name);
                    ELFArray[i].map_start = MAP_FAILED;
                }
                else {
                    printf("Failed to unmap file: %s\n", ELFArray[i].file_name);
                }
            }
            // Close the file descriptor
            close(ELFArray[i].fd);
            ELFArray[i].fd = -1;
        }
    }
    // Exit normally
    exit(0);
}