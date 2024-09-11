#include "cli.h"
#include "stdio.h"
#include "../util/string.h"
#include "../core/fat.h"
#include "../../kernel/arch/i686/io.h"

typedef enum {
    CMD_HELP,
    CMD_INFO,
    CMD_LS,
    CMD_CAT,
    CMD_CLEAR,
    CMD_EXIT,
    CMD_UNKNOWN
} CommandType;

typedef struct {
    CommandType type;
    char parameter[100];
} Command;

Command Command_Parse(char *input) {

    Command cmd;
    cmd.type = CMD_UNKNOWN;
    cmd.parameter[0] = '\0';

    if (strncmp(input, "help", 4) == 0) {
        cmd.type = CMD_HELP;
    } else if (strncmp(input, "info", 4) == 0) {
        cmd.type = CMD_INFO;
    } else if (strncmp(input, "ls", 2) == 0) { //TODO: Make parameter
        cmd.type = CMD_LS;
    } else if (strncmp(input, "cat", 3) == 0) {
        cmd.type = CMD_CAT;
        char *param = input + 4;
        while (*param == ' ') param++;
        strncpy(cmd.parameter, param, sizeof(cmd.parameter) - 1);
        cmd.parameter[sizeof(cmd.parameter) - 1] = '\0';
    } else if (strncmp(input, "clear", 5) == 0) {
        cmd.type = CMD_CLEAR;
    } else if (strncmp(input, "exit", 4) == 0) {
        cmd.type = CMD_EXIT;
    }

    return cmd;
}

void Command_Line_Interface(BootParams* bootParams, Partition* part) {
    char buffer[256];
    printf("Type 'help' for a list of commands.\n");
    for (;;) {
        printf("> ");
        fgets(buffer, sizeof(buffer));
        Command cmd = Command_Parse(buffer);
        switch (cmd.type) {
            case CMD_HELP:
                help();
                break;
            case CMD_INFO:
                info(bootParams, part);
                break;
            case CMD_LS:
                Kernel_FAT_ReadDirectory(part, "/"); //TODO: Make parameter
                break;
            case CMD_CAT:
                Kernel_FAT_ReadFile(part, cmd.parameter); //"root/test.txt"
                break;
            case CMD_CLEAR:
                clrscr();
                break;
            case CMD_EXIT:
                i686_outw(0x604, 0x2000);
                break;
            case CMD_UNKNOWN:
            default:
                printf("Unknown command: %s", buffer);
                break;
        }
    }
}

void help() {
    printf("Available commands:\n");
    printf("  help  - Display this help message\n");
    printf("  info  - Display important information\n");
    printf("  ls    - Display a directory\n");
    printf("  cat <filepath> - Display the contents of the specified file\n");
    printf("  clear - Clear the screen\n");
    printf("  exit  - Exit the OS\n");
}

void info(BootParams* bootParams, Partition* part) {
    printf("ArcOS v0.1\n");
    printf("This operating system is under construction.\n");
    for (int i = 0; i < bootParams->Memory.RegionCount; i++) {
        printf("MEM: start=0x%llx length=0x%llx type=%x\n", bootParams->Memory.Regions[i].Begin, bootParams->Memory.Regions[i].Length, bootParams->Memory.Regions[i].Type);
    }
    printf("DISK: id=%u cylinders=%u heads=%u sectors=%u\n", part->disk->id, part->disk->cylinders, part->disk->heads, part->disk->sectors);
    printf("MBR: partitionOffset=%u partitionSize=%u\n", part->partitionOffset, part->partitionSize);
}