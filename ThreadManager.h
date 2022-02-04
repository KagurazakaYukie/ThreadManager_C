#ifndef ThreadManager_H
#define ThreadManager_H

#include <stdio.h>
#include <stdlib.h>
#include "MArrayList.h"
#include <pthread.h>
#include "Queue.h"
#include "Init.h"
#include "MultitThreadMemoryManager.h"

#define ThreadStateStart 0
#define ThreadStateStop 1
#define ThreadStateSuspend 2
#define ThreadStateWaitDestroy 3

typedef struct mtp {
    pthread_t *id;
    int ThreadState,tid;
    MemoryInfo *mi,*mid;
} ThreadID;

typedef struct mtpm {
    MArrayList *tal;
    MTMemoryManager *mm;
    MemoryInfo *mi;
} ThreadManager;

typedef struct mtq {
    QueueManager *qm;
    pthread_mutex_t *mutex;
    MemoryInfo *mi,*mmutex;
} ThreadQueue;

typedef struct mtpa {
    ThreadQueue *tqm;
    ThreadManager *tpm;
    MemoryBigUnit *mbu;
    ThreadID *tid;
    MemoryInfo *mi;
} ThreadPack;

typedef struct tpi {
    ThreadPack *tpa;
    void *data;
} ThreadInfo;

void *TPMThread(void *c);

ThreadManager *ThreadManagerInit(MTMemoryManager *mm);

ThreadQueue *ThreadManagerAddTask(MTMemoryManager *mm, ThreadQueue *tqm, void *ft, void *data);

ThreadQueue *ThreadQueueInit(MTMemoryManager *mm);

ThreadPack *ThreadManagerAddThread(MTMemoryManager *mm, ThreadManager *tpm, ThreadQueue *tqm);

ThreadManager *ThreadManagerTaskState(ThreadManager *tpm, int i, int state);

void ThreadManagerSetWaitDestroy(ThreadManager *tpm);

void ThreadManagerDestroy(ThreadManager *tpm);

void ThreadQueueDestroy(MTMemoryManager *mm, ThreadQueue *tqm);

MArrayList *ThreadManagerGetThreads(ThreadManager *tm);

void ThreadManagerSetDestroy(ThreadManager *tpm);

void ThreadManagerSetThreadState(ThreadID *tid,int i);

#define ThreadPackGetThreadID(a) ({\
a->tid;\
})

#endif
