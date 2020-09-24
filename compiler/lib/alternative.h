#ifndef ALTERNATIVE_H
#define ALTERNATIVE_H

#include "../../interpreter/function.h"
enum Phase phase = INIT_PROGRAM;

void freeEverything();
void breakLoop();
void continueLoop();

char *program_file_path = "main.kaos";

#endif
