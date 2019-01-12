#ifndef _MESSAGE_H_
#define _MESSAGE_H_

#define MAX_LENGTH 256

int *makeMessage(int opcode, int length, int* payload);
int parseMess(int *mess, int *opcode, int *length, int *payload);

#endif