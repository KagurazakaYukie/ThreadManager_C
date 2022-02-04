#include "ThreadManager.h"
#include <pthread.h>
#include "Queue.h"
#include "MultitThreadMemoryManager.h"
#include <stdbool.h>

//ぜんぶかんせい
void *TPMThread(void *c) {
    ThreadPack *tpa = (ThreadPack *) c;
    tpa->mbu->id = pthread_self();
    ThreadID *tp = (ThreadID *) tpa->tid;
    int state = 0;
    void *ft = NULL, *data = NULL;
    while (true) {
        state = tp->ThreadState;
        if (state != ThreadStateSuspend && state != ThreadStateStop) {
            if (pthread_mutex_trylock(tpa->tqm->mutex) == 0) {
                if (QueueManagerGet((tpa->tqm)->qm) != NULL && tpa->tqm->qm->Size != 0) {
                    Queue *a1 = QueueManagerOut((tpa->tqm)->qm);
                    data = a1->data;
                    ft = a1->ft;
                    QueueDestroy2(tpa->tqm->qm, a1);
                    pthread_mutex_unlock(tpa->tqm->mutex);
                    MemoryInfo *timi = MTMemoryManagerUnitCalloc(tpa->tpm->mm, tpa->mbu, sizeof(ThreadInfo));
                    ThreadInfo *in = (ThreadInfo *) timi->m;
                    in->tpa = tpa;
                    in->data = data;
                    (*((Ft) (ft)))(in);
                    MTMemoryManagerUnitFree(tpa->mbu, timi);
                    in = NULL;
                } else {
                    if (state == ThreadStateWaitDestroy && tpa->tqm->qm->Size == 0) {
                        pthread_mutex_unlock(tpa->tqm->mutex);
                        MTMemoryManagerUnitFree(tpa->mbu, tpa->mi);
                        MTMemoryManagerUnitFree(tpa->mbu, tp->mi);
                        break;
                    } else {
                        pthread_mutex_unlock(tpa->tqm->mutex);
                    }
                }
            }
        } else {
            if (state == ThreadStateStop) {
                MTMemoryManagerUnitFree(tpa->mbu, tpa->mi);
                MTMemoryManagerUnitFree(tpa->mbu, tp->mi);
                break;
            }
        }
    }
    MTMemoryManagerAppointComleteInitUnit(tpa->mbu);
    pthread_exit(NULL);
    return 0;
}

ThreadManager *ThreadManagerInit(MTMemoryManager *mm) {
    MemoryInfo *mi = MTMemoryManagerCalloc(mm, sizeof(ThreadManager));
    ThreadManager *tpm = (ThreadManager *) mi->m;
    tpm->mm = mm;
    tpm->tal = MArrayListInit(mm);
    tpm->mi = mi;
    return tpm;
}

ThreadQueue *ThreadManagerAddTask(MTMemoryManager *mm, ThreadQueue *tqm, void *ft, void *data) {
    while (true) {
        if (pthread_mutex_trylock(tqm->mutex) == 0) {
            break;
        }
    }
    QueueManagerAdd(mm, tqm->qm, ft, data);
    pthread_mutex_unlock(tqm->mutex);
    return tqm;
}

ThreadQueue *ThreadQueueInit(MTMemoryManager *mm) {
    MemoryInfo *mi = MTMemoryManagerCalloc(mm, sizeof(ThreadQueue));
    MemoryInfo *mmutex = MTMemoryManagerCalloc(mm, sizeof(pthread_mutex_t));
    ThreadQueue *tqm = (ThreadQueue *) mi->m;
    tqm->mi = mi;
    tqm->mmutex = mmutex;
    tqm->mutex = (pthread_mutex_t *) mmutex->m;
    pthread_mutex_init(tqm->mutex, NULL);
    tqm->qm = QueueManagerInit(mm);
    return tqm;
}

ThreadPack *ThreadManagerAddThread(MTMemoryManager *mm, ThreadManager *tpm, ThreadQueue *tqm) {
    MemoryBigUnit *mbu = MTMemoryManagerBindingThread(mm, 999);
    MemoryInfo *tpami = MTMemoryManagerUnitCalloc(mm, mbu, sizeof(ThreadPack));
    MemoryInfo *tpmi = MTMemoryManagerUnitCalloc(mm, mbu, sizeof(ThreadID));
    MemoryInfo *mid = MTMemoryManagerUnitCalloc(mm, mbu, sizeof(pthread_t));

    ThreadPack *tpa = (ThreadPack *) tpami->m;
    ThreadID *tp = (ThreadID *) tpmi->m;

    tpa->mbu = mbu;
    tpa->mi = tpami;
    tpa->tqm = tqm;
    tpa->tpm = tpm;
    tpa->tid = tp;

    tp->ThreadState = ThreadStateStart;
    tp->mi = tpmi;

    if (pthread_create((pthread_t *) mid->m, NULL, TPMThread, tpa) == 0) {
        tp->id = (pthread_t *) mid->m;
        tp->mid = mid;
        tp->tid = MArrayListSize(tpm->tal) - 1;
        MArrayListAddIndex(mm, tpm->tal, tp);
        return tpa;
    } else {
        MTMemoryManagerAppointComleteInitUnit(mbu);
        return NULL;
    }
}


ThreadManager *ThreadManagerTaskState(ThreadManager *tpm, int i, int state) {
    ((ThreadID *) MArrayListGetIndex(tpm->tal, i))->ThreadState = state;
    return tpm;
}

void ThreadManagerSetWaitDestroy(ThreadManager *tpm) {
    ThreadID *tp;
    for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        tp->ThreadState = ThreadStateWaitDestroy;
    }
    for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        pthread_join(*(tp->id), NULL);
    }
}

void ThreadManagerDestroy(ThreadManager *tpm) {
    MArrayListDestroy(tpm->mm, tpm->tal);
    MTMemoryManagerFree(tpm->mm, tpm->mi);
}

void ThreadQueueDestroy(MTMemoryManager *mm, ThreadQueue *tqm) {
    pthread_mutex_destroy(tqm->mutex);
    QueueManagerDestroy(mm, tqm->qm);
    MTMemoryManagerFree(mm, tqm->mi);
    MTMemoryManagerFree(mm, tqm->mmutex);
}

MArrayList *ThreadManagerGetThreads(ThreadManager *tm) {
    return tm->tal;
}

void ThreadManagerSetDestroy(ThreadManager *tpm) {
    ThreadID *tp;
    for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        tp->ThreadState = ThreadStateStop;
    }
    for (int i = 0; i < MArrayListSize(tpm->tal); i++) {
        tp = (ThreadID *) MArrayListGetIndex(tpm->tal, i);
        pthread_join(*(tp->id), NULL);
    }
}

void ThreadManagerSetThreadState(ThreadID *tid, int i) {
    if (i == ThreadStateWaitDestroy | i == ThreadStateStop) {
        tid->ThreadState = i;
        pthread_join(*(tid->id), NULL);
    } else {
        tid->ThreadState = i;
    }
}
