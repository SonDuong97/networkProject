#include "message.h"

int *makeMessage(int opcode, int length, int* payload)
{
    int* message = malloc((MAX_LENGTH+2)*sizeof(int));
    bzero(message, (MAX_LENGTH+2)*4);
    message[0] = opcode;
    message[1] = length;
    memcpy(message+2, payload, length);
    return message; 
}

int parseMess(int *mess, int *opcode, int *length, int *payload) {
    *opcode = mess[0];
    *length = mess[1];
    memcpy(payload, mess+2, *length);
    return 0;
}