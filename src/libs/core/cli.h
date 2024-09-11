#pragma once
#include "../boot/bootparams.h"
#include "../boot/mbr.h"

void Command_Line_Interface(BootParams* bootParams, Partition* part);
void help();
void info(BootParams* bootParams, Partition* part);