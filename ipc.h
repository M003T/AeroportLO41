#ifndef IPC_H
#define IPC_H

#include <sys/ipc.h>
#include <sys/types.h>

#define IFLAGS (PERM | IPC_CREAT)
#define KEY   (key_t) IPC_PRIVATE
#define PERM 0600

#endif