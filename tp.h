#ifndef ThreadManager_H
#define ThreadManager_H
#include <stdio.h>
#include <stdlib.h>
#include "MArrayList.h"
#include <pthread.h>
#include "queue.h"
#include "init.h"
#include "mtmm.h"

#define ThreadStateStart 0
#define ThreadStateStop 1
#define ThreadStateSuspend 2
#define ThreadStateWait 3

typedef struct mtp {
  pthread_t id;
  int ThreadState;
  int tid;
  MemoryInfo *mi;
} ThreadID;

typedef struct mtpm {
  MArrayList *tal;
  MTMemoryManager *mm;
  QueueManager *qm;
  pthread_mutex_t mutex;
  MemoryInfo *mi;
} ThreadManager;

typedef struct mtpa {
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

ThreadManager *ThreadManagerInit(MTMemoryManager *mm,int init);

ThreadManager *ThreadManagerAddTask(MTMemoryManager *mm,ThreadManager * tpm, void *ft, void *data);

int ThreadManagerAddThread(MTMemoryManager *mm,ThreadManager * tpm);

ThreadManager *ThreadManagerTaskState(ThreadManager * tpm, int i, int state);

void ThreadManagerWaitDestroy(ThreadManager * tpm);

void ThreadManagerWaitDestroy2(ThreadManager * tpm);

MArrayList *ThreadManagerGetThreads(ThreadManager *tm);

void ThreadManagerDestroy(ThreadManager * tpm);

void ThreadManagerDestroy2(ThreadManager * tpm);

#endif
