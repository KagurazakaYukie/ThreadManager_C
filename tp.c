#include "tp.h"
#include <pthread.h>
#include "queue.h"
#include "mtmm.h"
#include <stdbool.h>
//ぜんぶかんせい
void *TPMThread(void *c) {
  ThreadPack *tpa = (ThreadPack *) c;
  tpa->mbu->id=pthread_self();
  ThreadID *tp = (ThreadID *) tpa->tid;
  int state = 0;
  void *ft=NULL,*data=NULL;
  while (true) {
    state = tp->ThreadState;
    if (state != ThreadStateSuspend && state != ThreadStateStop) {
      if (pthread_mutex_trylock(&(tpa->tpm->mutex)) == 0) {
        if (QueueManagerGet((tpa->tpm)->qm) != NULL && tpa->tpm->qm->Size != 0) {
          Queue *a1 = QueueManagerOut((tpa->tpm)->qm);
          data=a1->data;
          ft=a1->ft;
          QueueDestroy2(tpa->tpm->qm,a1);
          pthread_mutex_unlock(&(tpa->tpm->mutex));
          MemoryInfo *timi=MTMemoryManagerUnitCalloc(tpa->tpm->mm,tpa->mbu,sizeof(ThreadInfo));
          ThreadInfo *in = (ThreadInfo *) timi->m;
          in->tpa = tpa;
          in->data = data;
          (*((Ft)(ft))) (in);
          MTMemoryManagerUnitFree(tpa->mbu,timi);
          in = NULL;
        } else {
          if (state == ThreadStateWait && tpa->tpm->qm->Size == 0) {
            pthread_mutex_unlock(&(tpa->tpm->mutex));
            MTMemoryManagerUnitFree(tpa->mbu,tpa->mi);
            MTMemoryManagerUnitFree(tpa->mbu,tp->mi);
            break;
          } else {
            pthread_mutex_unlock(&(tpa->tpm->mutex));
          }
        }
      }
    } else {
      if (state == ThreadStateStop) {
        MTMemoryManagerUnitFree(tpa->mbu,tpa->mi);
        MTMemoryManagerUnitFree(tpa->mbu,tp->mi);
        break;
      }
    }
  }
  MTMemoryManagerCompleteInitUnit(tpa->tpm->mm,pthread_self());
  pthread_exit(NULL);
  return 0;
}

ThreadManager *ThreadManagerInit(MTMemoryManager *mm,int init) {
  MemoryInfo *mi=MTMemoryManagerCalloc(mm,sizeof(ThreadManager));
  ThreadManager *tpm = (ThreadManager *)mi->m;
  pthread_mutex_init(&(tpm->mutex), NULL);
  tpm->mm=mm;
  tpm->tal = MArrayListInit(mm,init);
  tpm->mi=mi;
  tpm->qm = QueueManagerInit(mm);
  return tpm;
}

ThreadManager *ThreadManagerAddTask(MTMemoryManager *mm,ThreadManager * tpm, void *ft, void *data) {
  while (true) {
    if (pthread_mutex_trylock(&(tpm->mutex)) == 0) {
      break;
    }
  }
  QueueManagerAdd(mm,tpm->qm, ft, data);
  pthread_mutex_unlock(&(tpm->mutex));
  return tpm;
}

int ThreadManagerAddThread(MTMemoryManager *mm,ThreadManager * tpm) {
  pthread_t id;

  MemoryBigUnit *mbu=MTMemoryManagerBindingThread(mm,999);
  MemoryInfo *tpami=MTMemoryManagerUnitCalloc(mm,mbu,sizeof(ThreadPack));
  MemoryInfo *tpmi=MTMemoryManagerUnitCalloc(mm,mbu,sizeof(ThreadID));
  
  ThreadPack *tpa = (ThreadPack *) tpami->m;
  ThreadID *tp = (ThreadID *) tpmi->m;
  
  MArrayListAddIndex(tpm->mm,tpm->tal, tp);
  int si = MArrayListSize(tpm->tal);
  int u = pthread_create(&id, NULL, TPMThread, tpa);
  
  tpa->mbu=mbu;
  tpa->mi=tpami;
  tpa->tpm = tpm;
  tpa->tid = tp;
  
  tp->id = id;
  tp->ThreadState = ThreadStateStart;
  tp->tid = si - 1;
  tp->mi=tpmi;
  return u;
}

ThreadManager *ThreadManagerTaskState(ThreadManager * tpm, int i, int state) {
  while (true) {
    if (pthread_mutex_trylock(&(tpm->mutex)) == 0) {
      break;
    }
  }
  ((ThreadID *) MArrayListGetIndex(tpm->tal, i))->ThreadState = state;
  pthread_mutex_unlock(&(tpm->mutex));
  return tpm;
}

void ThreadManagerWaitDestroy(ThreadManager * tpm) {
  while (true) {
    if (QueueManagerGet(tpm->qm) == NULL) {
    ThreadID *tp;
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        tp->ThreadState = ThreadStateWait;
      }
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        pthread_join(tp->id, NULL);
      }
      break;
    }
  }
  pthread_mutex_destroy(&(tpm->mutex));
  MArrayListDestroy(tpm->mm,tpm->tal);
  QueueManagerDestroy(tpm->mm,tpm->qm);
  MTMemoryManagerFree(tpm->mm,tpm->mi);
}

void ThreadManagerWaitDestroy2(ThreadManager * tpm) {
  while (true) {
    if (QueueManagerGet(tpm->qm) == NULL) {
    	ThreadID *tp;
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        tp->ThreadState = ThreadStateWait;
      }
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        pthread_join(tp->id, NULL);
      }
      break;
    }
  }
  pthread_mutex_destroy(&(tpm->mutex));
  MArrayListDestroy(tpm->mm,tpm->tal);
  QueueManagerDestroy2(tpm->qm);
  MTMemoryManagerFree(tpm->mm,tpm->mi);
}

MArrayList *ThreadManagerGetThreads(ThreadManager *tm){
  return tm->tal;
}

void ThreadManagerDestroy(ThreadManager * tpm) {
  while (true) {
    if (QueueManagerGet(tpm->qm) == NULL) {
    ThreadID *tp;
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        tp->ThreadState = ThreadStateStop;
      }
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        pthread_join(tp->id, NULL);
      }
      break;
    }
  }
  pthread_mutex_destroy(&(tpm->mutex));
  MArrayListDestroy(tpm->mm,tpm->tal);
  QueueManagerDestroy(tpm->mm,tpm->qm);
  MTMemoryManagerFree(tpm->mm,tpm->mi);
}

void ThreadManagerDestroy2(ThreadManager * tpm) {
  while (true) {
    if (QueueManagerGet(tpm->qm) == NULL) {
      ThreadID *tp;
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        tp->ThreadState = ThreadStateStop;
      }
      for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        pthread_join(tp->id, NULL);
      }
      break;
    }
  }
  pthread_mutex_destroy(&(tpm->mutex));
  MArrayListDestroy(tpm->mm,tpm->tal);
  QueueManagerDestroy2(tpm->qm);
  MTMemoryManagerFree(tpm->mm,tpm->mi);
}
