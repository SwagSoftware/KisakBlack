#include "jobqueue_all.h"

#include <Windows.h>
#include "../tl_system.h"
#include <universal/q_shared.h>
#include <qcommon/threads.h>

thread_local jqBatch *jqCurBatch;
thread_local jqWorker *jqCurWorker;
thread_local jqQueue *jqCurQueue;

unsigned __int64 jqMainThreadID;

jqQueue jqHighPriorityQueue;

jqBatchPool jqPool;

void(__cdecl *jqWorkerInitFn)(int);
HANDLE jqNewJobAdded;
bool jqStopSignal;
volatile unsigned int jqPoolLock;
jqQueue jqGlobalQueue;
jqWorker jqMainThreadWorker;
jqWorker *jqTempWorkers;

volatile unsigned int jqKeepWorkersAwakeCount;
volatile unsigned int jqSleepingWorkersCount;
volatile unsigned int jqNextAvailTempWorker;
volatile unsigned int jqBatchPoolExternallyLockedCount;
volatile unsigned int jqNWorkers;
jqWorker *jqWorkers;

const char *jqCheckContext;

unsigned int __cdecl tlAtomicAdd(volatile unsigned __int32 *var, unsigned int value)
{
  volatile signed __int32 i; // ecx

  for ( i = *var; _InterlockedCompareExchange(var, *var + value, *var) != i; i = *var )
    Sleep(0);
  return i + value;
}

unsigned __int64 __cdecl tlAtomicAnd(volatile unsigned __int64 *var, unsigned __int64 value)
{
#if 0
  volatile signed __int64 *v2; // edi
  unsigned int v3; // esi
  signed __int64 v4; // rax
  unsigned int v6; // [esp+10h] [ebp-Ch]
  volatile unsigned __int64 v; // [esp+14h] [ebp-8h]

  v2 = (volatile signed __int64 *)var;
  for ( v = *var; ; v = *var )
  {
    v6 = HIDWORD(v);
    v3 = v;
    v4 = _InterlockedCompareExchange64(v2, value & v, v);
    if ( __PAIR64__(v4, HIDWORD(v4)) == __PAIR64__(v3, v6) )
      break;
    Sleep(0);
    v2 = (volatile signed __int64 *)var;
  }
  return value & v;
#else // aislop
    uint64_t oldValue;
    uint64_t newValue;

    do
    {
        oldValue = *var;
        newValue = oldValue & value;
    } while (_InterlockedCompareExchange64(
        (volatile LONG64 *)var,
        newValue,
        oldValue) != oldValue);

    return newValue;
#endif
}

unsigned __int64 __cdecl tlAtomicOr(volatile unsigned __int64 *var, unsigned __int64 value)
{
#if 0
  volatile signed __int64 *v2; // edi
  unsigned int v3; // esi
  signed __int64 v4; // rax
  unsigned int v6; // [esp+10h] [ebp-Ch]
  volatile unsigned __int64 v; // [esp+14h] [ebp-8h]

  v2 = (volatile signed __int64 *)var;
  for ( v = *var; ; v = *var )
  {
    v6 = HIDWORD(v);
    v3 = v;
    v4 = _InterlockedCompareExchange64(v2, value | v, v);
    if ( __PAIR64__(v4, HIDWORD(v4)) == __PAIR64__(v3, v6) )
      break;
    Sleep(0);
    v2 = (volatile signed __int64 *)var;
  }
  return value | v;
#else // aislop
    uint64_t oldValue;
    uint64_t newValue;

    do
    {
        oldValue = *var;
        newValue = oldValue | value;
    } while (_InterlockedCompareExchange64(
        (volatile LONG64 *)var,
        newValue,
        oldValue) != oldValue);

    return newValue;
#endif
}

void __thiscall tlAtomicMutex::Lock()
{
    //LONG Target; // [esp+1Ch] [ebp-10h] BYREF
    tlAtomicMutex *ThisPtr2; // [esp+20h] [ebp-Ch]
    unsigned __int64 CurThread; // [esp+24h] [ebp-8h]

    CurThread = GetCurrentThreadId();

    if (this->ThreadId == CurThread)
    {
        ++this->LockCount;
    }
    else
    {
        while (1)
        {
            if (!_InterlockedCompareExchange64(&this->ThisPtr->ThreadId, CurThread, 0))
                break;
            SwitchToThread();
        }
        //Target = 0;
        //InterlockedExchange(&Target, 0);
        this->LockCount = 1;
    }
}

void tlAtomicMutex::Unlock()
{
  if ( this->LockCount-- == 1 )
  {
      volatile int barrier = 0;
      InterlockedExchange((volatile long *)&barrier, 0);

      ThreadId = 0;
  }
}

bool __thiscall tlAtomicMutex::TryLock()
{
    volatile unsigned int Target; // [esp+1Ch] [ebp-10h] BYREF
    tlAtomicMutex *ThisPtr2; // [esp+20h] [ebp-Ch]
    unsigned __int64 CurThread; // [esp+24h] [ebp-8h]

    CurThread = GetCurrentThreadId();
    if (this->ThreadId == CurThread)
    {
        ++this->LockCount;
        return 1;
    }
    else
    {
        ThisPtr2 = this->ThisPtr;
        if (_InterlockedCompareExchange64((volatile signed __int64 *)ThisPtr2, CurThread, 0))
        {
            return 0;
        }
        else
        {
            Target = 0;
            InterlockedExchange(&Target, 0);
            this->LockCount = 1;
            return 1;
        }
    }
}

// this was added later as aislop, seems inlined 
void tlSharedAtomicMutex::Unlock()
{
    tlSharedAtomicMutex *m = ThisPtr;
    const uint32_t tid = GetCurrentThreadId();

    int newCount = InterlockedDecrement((volatile long *)&m->LockCount);

    if (newCount == 0)
    {
        InterlockedExchange64((volatile long long *)&m->ThreadId, 0);
    }
}

void tlSharedAtomicMutex::Lock()
{
#if 0
  unsigned int CurrentThreadId; // ebx
  tlSharedAtomicMutex *ThisPtr; // eax
  int ThreadId; // ecx
  int ThreadId_high; // eax
  unsigned int CurThread; // [esp+8h] [ebp-Ch]
  volatile unsigned int Target; // [esp+10h] [ebp-4h] BYREF

  CurrentThreadId = GetCurrentThreadId();
  ThisPtr = this->ThisPtr;
  ThreadId = ThisPtr->ThreadId;
  ThreadId_high = HIDWORD(ThisPtr->ThreadId);
  CurThread = CurrentThreadId;
  if ( ThreadId != CurrentThreadId || ThreadId_high )
  {
    while ( _InterlockedCompareExchange64((volatile signed __int64 *)this->ThisPtr, CurrentThreadId, 0) )
    {
      SwitchToThread();
      CurrentThreadId = CurThread;
    }
    Target = 0;
    InterlockedExchange(&Target, 0);
    this->ThisPtr->LockCount = 1;
  }
  else
  {
    ++this->ThisPtr->LockCount;
  }
#else // aislop
    const uint32_t tid = GetCurrentThreadId();
    tlSharedAtomicMutex *m = ThisPtr;

    uint32_t owner = (uint32_t)m->ThreadId;
    uint32_t high = (uint32_t)(m->ThreadId >> 32);

    // recursive lock
    if (owner == tid && high == 0)
    {
        ++m->LockCount;
        return;
    }

    // spin until CAS succeeds
    for (;;)
    {
        uint64_t expected = 0;
        uint64_t desired = tid;

        if (_InterlockedCompareExchange64(
            (volatile long long *)&m->ThreadId,
            desired,
            expected) == expected)
        {
            break;
        }

        SwitchToThread();
    }

    // weird compiler artifact from original code
    volatile int target = 0;
    InterlockedExchange((volatile long *)&target, 0);

    m->LockCount = 1;
#endif
}

bool jqAtomicHeap::GetAvailableBlock(jqAtomicHeap::LevelInfo *FitLevel, int *FitSlot)
{
  int NCells; // edx
  int v4; // eax
  signed __int64 v5; // rdi
  unsigned int v7; // ebx
  unsigned int v8; // edx
  int v9; // [esp+18h] [ebp-4h]

  NCells = FitLevel->NCells;
  v4 = 0;
  if ( NCells <= 0 )
    return 0;
  while ( 1 )
  {
    v5 = FitLevel->CellAvailable[v4];
    if ( v5 )
      break;
    if ( ++v4 >= NCells )
      return 0;
  }
  v7 = 0x80000000;
  v8 = 0;
  v9 = 0;
  if ( v5 >= 0 )
  {
    do
    {
      ++v9;
      v8 = __PAIR64__(v7, v8) >> 1;
      v7 >>= 1;
    }
    while ( !(HIDWORD(v5) & v7 | (unsigned int)v5 & v8) );
  }
  *FitSlot = 63 - v9 + (v4 << 6);
  return _InterlockedCompareExchange64(
           (volatile signed __int64 *)&FitLevel->CellAvailable[v4],
           v5 & ~(1LL << ((63 - v9) & 0x3F)),
           v5) == v5;
}

char jqAtomicHeap::AllocBlock(jqAtomicHeap::LevelInfo **FitLevel, int *FitSlot)
{
  if ( *FitLevel >= &this->Levels[this->NLevels] )
    return 0;
  while ( !jqAtomicHeap::GetAvailableBlock(*FitLevel, FitSlot) )
    {
    if ( ++*FitLevel >= &this->Levels[this->NLevels] )
      return 0;
    }
  return 1;
}

int jqAtomicHeap::SplitBlock(
        jqAtomicHeap::LevelInfo *Level,
        int Slot,
        jqAtomicHeap::LevelInfo *LevelTo)
{
  jqAtomicHeap::LevelInfo *i; // edi

  for ( i = Level; i > LevelTo; tlAtomicOr(&i->CellAvailable[(Slot + 1) / 64], 1LL << ((Slot + 1) & 0x3F)) )
  {
    Slot *= 2;
    --i;
  }
  tlAtomicOr(&i->CellAllocated[Slot / 64], 1LL << (Slot & 0x3F));
  return Slot;
}

unsigned __int8 *jqAtomicHeap::AllocLevel(int LevelIdx)
{
  jqAtomicHeap::LevelInfo *v3; // edi
  int v5; // ebx
  jqAtomicHeap::LevelInfo *FitLevel; // [esp+8h] [ebp-4h] BYREF

  v3 = &this->Levels[LevelIdx];
  FitLevel = v3;
    LevelIdx = 0;
  if ( !jqAtomicHeap::AllocBlock(&FitLevel, &LevelIdx) )
        return 0;
  v5 = jqAtomicHeap::SplitBlock(FitLevel, LevelIdx, v3);
  tlAtomicAdd(&this->ThisPtr->TotalBlocks, 1u);
  tlAtomicAdd(&this->ThisPtr->TotalUsed, v3->BlockSize);
  return &this->HeapBase[v5 * v3->BlockSize];
}

int jqAtomicHeap::FindLevelForSize(unsigned int Size)
{
  unsigned int BlockSize; // ecx

  BlockSize = this->BlockSize;
  return (BlockSize < Size)
       + (2 * BlockSize < Size)
       + (4 * BlockSize < Size)
       + (8 * BlockSize < Size)
       + (16 * BlockSize < Size)
       + (32 * BlockSize < Size)
       + (BlockSize << 6 < Size)
       + (BlockSize << 7 < Size)
       + (BlockSize << 8 < Size)
       + (BlockSize << 9 < Size)
       + (BlockSize << 10 < Size);
}

unsigned __int8 *jqAtomicHeap::Alloc(unsigned int Size, unsigned int Align)
{
    this->Mutex.Lock();

    if (Size < Align)
    {
        Size = Align;
    }

    if (Size <= this->HeapSize )
    {
        unsigned __int8 *mem = this->AllocLevel(this->FindLevelForSize(Size));
        this->Mutex.Unlock();
        return mem;
    }
    else
    {
        tlPrintf("Size (%d) > HeapSize (%d), return NULL\n", Size, this->HeapSize);
        return 0;
    }
}

void jqAtomicHeap::FindAllocatedBlock(
        unsigned int Offset,
        jqAtomicHeap::LevelInfo **FitLevel,
        int *FitSlot)
{
  signed int v4; // ecx
  jqAtomicHeap::LevelInfo *v5; // ebx
  jqAtomicHeap *v6; // [esp+4h] [ebp-4h]

  *FitLevel = this->Levels;
  v6 = this;
  if ( this->Levels < &this->Levels[this->NLevels] )
  {
    do
    {
      v4 = Offset / (*FitLevel)->BlockSize;
      *FitSlot = v4;
      if ( ((*FitLevel)->CellAllocated[v4 / 64] & (1LL << (v4 & 0x3F))) != 0 )
        break;
      v5 = *FitLevel + 1;
      *FitLevel = v5;
    }
    while ( v5 < &v6->Levels[v6->NLevels] );
    //this = v6;
  }
  if ( *FitLevel >= &this->Levels[this->NLevels] )
  {
    if ( _tlAssert(
           "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_atomic_heap.h",
           343,
           "FitLevel < Levels+NLevels",
           "jqAtomicHeap error detected, block not found in heap.") )
    {
      __debugbreak();
    }
  }
}

void jqAtomicHeap::MergeBlocks(jqAtomicHeap::LevelInfo **FitLevel, int *FitSlot)
{
#if 0
  jqAtomicHeap::LevelInfo **v3; // edi
  int *v4; // ecx
  int v5; // ecx
  int v6; // esi
  int v7; // ebx
  signed __int64 v8; // rax
  volatile signed __int64 *v9; // edi
  unsigned int v10; // esi
  __int64 v11; // [esp+Ch] [ebp-10h]

  v3 = FitLevel;
  if ( *FitLevel < (jqAtomicHeap::LevelInfo *)&this->HeapSize + this->NLevels )
  {
    v4 = FitSlot;
LABEL_3:
    v5 = *v4 ^ 1;
    v6 = 8 * (v5 / 64);
    v7 = 1LL << (v5 & 0x3F);
    v11 = 1LL << (v5 & 0x3F);
    while ( 1 )
    {
      LODWORD(v8) = (*v3)->CellAvailable;
      HIDWORD(v8) = *(unsigned int *)(v8 + v6 + 4);
      v9 = (volatile signed __int64 *)(v8 + v6);
      v10 = *(unsigned int *)(v8 + v6);
      if ( !(HIDWORD(v8) & HIDWORD(v11) | v10 & v7) )
        break;
      LODWORD(v8) = v10;
      if ( _InterlockedCompareExchange64(v9, __SPAIR64__(HIDWORD(v8) & (unsigned int)~HIDWORD(v11), v10 & ~v7), v8) == __PAIR64__(HIDWORD(v8), v10) )
      {
        v3 = FitLevel;
        ++*FitLevel;
        v4 = FitSlot;
        *FitSlot >>= 1;
        if ( *FitLevel < (jqAtomicHeap::LevelInfo *)&this->HeapSize + this->NLevels )
          goto LABEL_3;
        return;
      }
      v3 = FitLevel;
      v7 = 1LL << (v5 & 0x3F);
      v6 = 8 * (v5 / 64);
    }
  }
#else // aislop
    // Compute highest level pointer:
    LevelInfo *topLevel = &this->Levels[this->NLevels - 1];

    // Stop if already at top
    while (*FitLevel < topLevel)
    {
        LevelInfo *level = *FitLevel;

        int slot = *FitSlot;
        int buddy = slot ^ 1;

        int cellIndex = buddy >> 6;      // / 64
        int bitIndex = buddy & 63;      // % 64

        uint64_t mask = 1ULL << bitIndex;

        volatile uint64_t *cell =
            &level->CellAvailable[cellIndex];

        while (true)
        {
            uint64_t oldValue = *cell;

            if ((oldValue & mask) == 0)
                return;

            uint64_t newValue = oldValue & ~mask;

            // cmpxchg8b equivalent
            if (_InterlockedCompareExchange64(
                (volatile LONG64 *)cell,
                newValue,
                oldValue) == (LONG64)oldValue)
            {
                // Successful merge

                ++(*FitLevel);   // move to parent level
                *FitSlot >>= 1;  // parent slot
                break;           // continue outer loop
            }

        }
    }
#endif
}

void jqAtomicHeap::Free(jqAtomicHeap::LevelInfo *Ptr)
{
#if 0
  tlAtomicMutex *p_Mutex; // edi
  jqAtomicHeap::LevelInfo *v4; // ebx
  int v5; // esi
  __int64 v6; // rax
  unsigned int v7; // ebx
  unsigned int v9; // [esp-Ch] [ebp-20h]
  int FitSlot; // [esp+10h] [ebp-4h] BYREF

  p_Mutex = &this->Mutex;
  tlAtomicMutex::Lock(&this->Mutex);
  v9 = (char *)Ptr - (char *)this->HeapBase;
  FitSlot = 0;
  jqAtomicHeap::FindAllocatedBlock(this, v9, &Ptr, &FitSlot);
  v4 = Ptr;
  tlAtomicAnd(&Ptr->CellAllocated[FitSlot / 64], ~(1LL << (FitSlot & 0x3F)));
  tlAtomicAdd((volatile signed __int32 *)&this->ThisPtr->TotalBlocks, 0xFFFFFFFF);
  tlAtomicAdd((volatile signed __int32 *)&this->ThisPtr->TotalUsed, -v4->BlockSize);
  jqAtomicHeap::MergeBlocks(this, &Ptr, &FitSlot);
  v5 = FitSlot / 64;
  v6 = 1LL << (FitSlot & 0x3F);
  v7 = v6;
  LODWORD(v6) = HIDWORD(v6) & HIDWORD(Ptr->CellAvailable[v5]) | v6 & LODWORD(Ptr->CellAvailable[v5]);
  FitSlot = (unsigned __int64)(1LL << (FitSlot & 0x3F)) >> 32;
  if ( (unsigned int)v6
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_atomic_heap.h",
         402,
         "(jqGet(&FitLevel->CellAvailable[BlockCell(FitSlot)]) & BlockBit(FitSlot)) == 0",
         "jqAtomicHeap error detected, invalid tree state.") )
  {
    __debugbreak();
  }
  tlAtomicOr(&Ptr->CellAvailable[v5], __PAIR64__(FitSlot, v7));
  if ( p_Mutex->LockCount-- == 1 )
  {
    Ptr = 0;
    InterlockedExchange((volatile int *)&Ptr, 0);
    LODWORD(p_Mutex->ThreadId) = 0;
    HIDWORD(p_Mutex->ThreadId) = 0;
  }
#else // aislop
    //tlAtomicMutex::Lock(&this->Mutex);
    this->Mutex.Lock();

    // Compute offset into heap
    unsigned int offset = (unsigned int)((unsigned char *)Ptr - this->HeapBase);

    LevelInfo *level = nullptr;
    int slot = 0;

    // Find which level + slot this allocation belongs to
    this->FindAllocatedBlock(offset, &level, &slot);

    // Clear allocated bit
    unsigned int cellIndex = slot >> 6;          // /64
    unsigned int bitIndex = slot & 63;

    unsigned long long bitMask = 1ULL << bitIndex;

    tlAtomicAnd(&level->CellAllocated[cellIndex], ~bitMask);

    // Update heap stats
    tlAtomicAdd((volatile unsigned int *)&this->TotalBlocks, -1);
    tlAtomicAdd((volatile unsigned int *)&this->TotalUsed, -(int)level->BlockSize);

    // Try to merge upward (buddy merge)
    this->MergeBlocks(&level, &slot);

    // Recompute cell index after merge
    cellIndex = slot >> 6;
    bitIndex = slot & 63;
    bitMask = 1ULL << bitIndex;

    // Validate: block must not already be marked available
    if (level->CellAvailable[cellIndex] & bitMask)
    {
        _tlAssert(
            "jobqueue_atomic_heap.h",
            402,
            "(jqGet(&FitLevel->CellAvailable[BlockCell(FitSlot)]) & BlockBit(FitSlot)) == 0",
            "jqAtomicHeap error detected, invalid tree state."
        );
    }

    // Mark block available
    tlAtomicOr(&level->CellAvailable[cellIndex], bitMask);

    //tlAtomicMutex::Unlock(&this->Mutex);
    this->Mutex.Unlock();
#endif
}

unsigned int jqProcessorsMask = 255;


void __cdecl jqAttachQueueToWorkers(jqQueue *Queue, unsigned int ProcessorMask)
{
#if 0
  unsigned int v2; // eax
  int v3; // ecx
  int v4; // edi
  char *v5; // edi
  volatile unsigned __int32 *v6; // ebx
  int v7; // esi
  int v8; // [esp+0h] [ebp-Ch]
  unsigned int BaseProcessorsMask; // [esp+4h] [ebp-8h]
  unsigned int Processor; // [esp+8h] [ebp-4h]

  v2 = jqProcessorsMask;
  v3 = 1;
  Processor = 1;
  if ( jqProcessorsMask )
  {
    v4 = -168;
    do
    {
      if ( (v3 & v2) == 0 )
      {
        do
          v3 *= 2;
        while ( (v3 & v2) == 0 );
        Processor = v3;
      }
      v4 += 168;
      v2 ^= v3;
      v8 = v4;
      BaseProcessorsMask = v2;
      if ( (v3 & ProcessorMask) != 0 )
      {
        v5 = (char *)jqWorkers + v4;
        v6 = (volatile unsigned __int32 *)(v5 + 28);
        do
        {
          v7 = *v6;
          if ( *(int *)v6 >= 8 )
          {
            if ( _tlAssert(
                   "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
                   108,
                   "numQueues < JQ_MAX_QUEUES",
                   "Ran out of queues for this worker!") )
            {
              __debugbreak();
            }
            v3 = Processor;
          }
        }
        while ( _InterlockedCompareExchange((volatile unsigned __int32 *)&v5[4 * v7 + 128], (signed __int32)Queue, 0) );
        _InterlockedExchangeAdd(v6, 1u);
        Queue->ProcessorsMask |= *((unsigned int *)v5 + 5);
        v2 = BaseProcessorsMask;
        v4 = v8;
      }
    }
    while ( v2 );
  }
#else
    if (!jqProcessorsMask)
        return;

    unsigned int remainingMask = jqProcessorsMask;

    // Iterate over each processor in the global mask
    while (remainingMask)
    {
        unsigned int processorBit = 1;

        // Find the next available processor in remainingMask
        while ((processorBit & remainingMask) == 0)
            processorBit <<= 1;

        remainingMask ^= processorBit; // Mark processor as used

        if ((processorBit & ProcessorMask) == 0)
            continue; // Skip if this processor is not in the requested mask

        // Compute worker pointer from processorBit
        int workerIndex = 0;
        unsigned int mask = 1;
        while (mask != processorBit)
        {
            mask <<= 1;
            workerIndex++;
        }

        jqWorker *worker = &jqWorkers[workerIndex];

        // Find the next free queue slot
        int queueSlot = worker->NumQueues;
        if (queueSlot >= 8)
        {
            if (_tlAssert(
                "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
                108,
                "numQueues < JQ_MAX_QUEUES",
                "Ran out of queues for this worker!"))
            {
                __debugbreak();
            }
        }

        // Atomically assign the queue to the worker
        while (_InterlockedCompareExchange(
            reinterpret_cast<volatile LONG *>(&worker->Queues[queueSlot]),
            reinterpret_cast<LONG>(Queue),
            0) != 0)
        {
            // retry until successful
        }

        // Increment worker's queue count
        _InterlockedExchangeAdd(reinterpret_cast<volatile LONG *>(&worker->NumQueues), 1);

        // Add this worker's processor bit to the queue's ProcessorsMask
        Queue->ProcessorsMask |= (1 << workerIndex);
    }
#endif
}

void __cdecl jqEnableWorkers(unsigned int ProcessorsMask)
{
  jqProcessorsMask = ProcessorsMask;
}

int __cdecl jqGetNumWorkers()
{
  return jqNWorkers;
}

unsigned __int64 __cdecl jqGetCurrentThreadID()
{
  return GetCurrentThreadId();
}

unsigned __int64 __cdecl jqGetMainThreadID()
{
  return jqMainThreadID;
}

jqBatch *__cdecl jqGetCurrentBatch()
{
  //return *(jqBatch **)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8316);
  return jqCurBatch;
}

jqWorker *__cdecl jqGetCurrentWorker()
{
  //return *(jqWorker **)(*((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index) + 8312);
    return jqCurWorker;
}

jqQueue *__cdecl jqGetWorkerQueue(int worker)
{
  if ( jqNWorkers <= 0 )
    return 0;

  for (unsigned int i = 0; i < jqNWorkers; ++i)
  {
      if (jqWorkers[i].Processor == worker)
          return &jqWorkers[i].WorkerSpecific;
  }

  return 0;
}

void __cdecl jqShutdownWorker()
{
  ;
}

int __cdecl jqGetQueuedBatchCount(jqBatchGroup *GroupID)
{
  if ( GroupID )
    return GroupID->QueuedBatchCount;
  else
    return jqPool.group.QueuedBatchCount;
}

int __cdecl jqGetExecutingBatchCount(jqBatchGroup *GroupID)
{
  if ( GroupID )
    return GroupID->ExecutingBatchCount;
  else
    return jqPool.group.ExecutingBatchCount;
}

jqWorker *__cdecl jqFindWorkerForProcessor(jqProcessor Processor)
{
  unsigned int v1; // eax
  int *i; // ecx

  v1 = 0;
  if ( jqNWorkers <= 0 )
    return 0;
  for ( i = &jqWorkers->Processor; *i != Processor; i += 42 )
  {
    if ( ++v1 >= jqNWorkers )
      return 0;
  }
  return &jqWorkers[v1];
}

bool __cdecl jqPoll(jqBatchGroup *GroupID)
{
    jqBatchGroup *p_group; // ebx

    p_group = GroupID;
    if (!GroupID)
        p_group = &jqPool.group;
    if (((unsigned __int8)p_group & 7) != 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
            390,
            "((u32)BatchCount & 0x7)==0",
            "Data not aligned correctly, read won't be atomic!\n"))
    {
        __debugbreak();
    }
    return p_group->BatchCount != 0;
}

bool __cdecl jqAreJobsQueued(jqBatchGroup *GroupID)
{
    jqBatchGroup *p_group; // eax

    p_group = GroupID;
    if (!GroupID)
        p_group = &jqPool.group;
    return p_group->QueuedBatchCount != 0;
}

void __cdecl jqSetWorkerInitFunction(void (__cdecl *fn)(int))
{
  jqWorkerInitFn = fn;
}

void __cdecl jqLetWorkersSleep()
{
  if ( jqKeepWorkersAwakeCount <= 0
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         441,
         "jqKeepWorkersAwakeCount > 0",
         "jqKeepWorkersAwakeCount underflow.") )
  {
    __debugbreak();
  }
  _InterlockedExchangeAdd(&jqKeepWorkersAwakeCount, 0xFFFFFFFF);
}

unsigned __int8 *__cdecl jqAllocBatchData(unsigned int Size)
{
  //return jqAtomicHeap::Alloc(&jqPool.BatchDataHeap, Size, 0x10u);
    return jqPool.BatchDataHeap.Alloc(Size, 0x10);
}

void __cdecl jqFreeBatchData(jqAtomicHeap::LevelInfo *Ptr)
{
  //jqAtomicHeap::Free(&jqPool.BatchDataHeap, Ptr);
    jqPool.BatchDataHeap.Free(Ptr);
}

unsigned int __cdecl jqGetBatchDataAvailable()
{
  return jqPool.BatchDataHeap.HeapSize - jqPool.BatchDataHeap.TotalUsed;
}

int __cdecl jqExecuteBatch(jqWorker *Worker, jqBatch *Batch)
{
  PROF_SCOPED_RUNTIME_NAME(Batch->Module->Name); // LWSS ADD (worker batch visibility)
  return Batch->Module->Code(Batch);
}

char __cdecl jqCanBatchExecute()
{
  return 1;
}

bool __cdecl jqWorkerSleep()
{
  volatile unsigned int Target; // [esp+0h] [ebp-4h] BYREF

  while ( !jqPool.group.QueuedBatchCount )
  {
    if ( jqStopSignal )
      break;
    if ( jqKeepWorkersAwakeCount )
      break;
    _InterlockedExchangeAdd(&jqSleepingWorkersCount, 1u);
    WaitForSingleObject(jqNewJobAdded, 1u);
    _InterlockedExchangeAdd(&jqSleepingWorkersCount, 0xFFFFFFFF);
  }
  SwitchToThread();
  Target = 0;
  InterlockedExchange(&Target, 0);
  return !jqStopSignal;
}

void __cdecl jqSetCheckContext(const char *desc)
{
  jqCheckContext = desc;
}

void __cdecl jqCheckDMALS(const void *addr)
{
  if ( !addr )
    tlFatal("%s (LS) is NULL.", jqCheckContext);
  if ( ((unsigned __int8)addr & 0xF) != 0 )
    tlFatal("%s 0x%x (LS) not 16byte aligned.", jqCheckContext, addr);
  if ( (unsigned int)addr > 0x40000 )
    tlFatal("%s 0x%x (LS) is > 256k.", jqCheckContext, addr);
  if ( (unsigned int)addr < 0x4000 )
    tlFatal("%s 0x%x (LS) is in kernel memory.", jqCheckContext, addr);
}

void __cdecl jqCheckDMAMain(const void *addr)
{
  if ( !addr )
    tlFatal("%s (Main) is NULL.", jqCheckContext);
  if ( ((unsigned __int8)addr & 0xF) != 0 )
    tlFatal("%s 0x%x (Main) not 16byte aligned.", jqCheckContext, addr);
  if ( (unsigned int)addr < 0x40000 )
    tlFatal("%s 0x%x (Main) is < 256k.", jqCheckContext, addr);
}

void __cdecl jqCheckDMASize(unsigned int size)
{
  if ( (size & 0xF) != 0 )
    tlFatal("%s size %d not 16byte aligned.", jqCheckContext, size);
}

void __cdecl jqCheckDMATag(unsigned int tag)
{
  if ( tag > 0x1E )
    tlFatal("%s invalid DMA tag %d.", jqCheckContext, tag);
}

void __cdecl jqCheckRange(int val, int mn, int mx)
{
  if ( val < mn || val > mx )
    tlFatal("%s %d out of range [%d-%d].", jqCheckContext, val, mn, mx);
}

void __cdecl jqCheckStack()
{
  ;
}

unsigned __int8 *__cdecl jqFetch(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size)
{
  if ( size )
    memcpy(dest, src, size);
  return dest;
}

void __cdecl jqStore(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size)
{
  if ( size )
    memcpy(dest, src, size);
}

unsigned __int8 *__cdecl jqFetchAsync(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size)
{
  if ( size )
    memcpy(dest, src, size);
  return dest;
}

void __cdecl jqStoreAsync(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size)
{
  if ( size )
    memcpy(dest, src, size);
}

void __cdecl jqWait()
{
  ;
}

void __cdecl jqWaitMultiple()
{
  ;
}

void __cdecl jqSetMemBase()
{
  ;
}

void __cdecl jqSetStackSize()
{
  ;
}

int __cdecl jqGetMemAvailable()
{
  return 0;
}

void *__cdecl jqAlloc()
{
  return 0;
}

void *__cdecl jqGetMemBase()
{
  return 0;
}

void __cdecl _jqInit()
{
  unsigned int v0; // ecx
  unsigned int v1; // edx
  int v2; // esi
  _SYSTEM_INFO SystemInfo; // [esp+0h] [ebp-24h] BYREF

  jqNewJobAdded = CreateEventA(0, 1, 0, 0);
  jqProcessorsMask = 0;
  GetSystemInfo(&SystemInfo);
  v0 = 0;
  if ( SystemInfo.dwNumberOfProcessors )
  {
    v1 = jqProcessorsMask;
    do
    {
      v2 = 1 << v0++;
      v1 |= v2;
    }
    while ( v0 < SystemInfo.dwNumberOfProcessors );
    jqProcessorsMask = v1;
  }
}

void __cdecl _jqShutdown()
{
  CloseHandle(jqNewJobAdded);
}

void __cdecl _jqStop()
{
  unsigned int v0; // edi
  int v1; // esi
  void *Thread; // eax

  jqStopSignal = 1;
  SetEvent(jqNewJobAdded);
  v0 = 0;
  if ( jqNWorkers > 0 )
  {
    v1 = 0;
    do
    {
      Thread = jqWorkers[v1].Thread;
      if ( Thread )
      {
        WaitForSingleObject(Thread, 0xFFFFFFFF);
        CloseHandle(jqWorkers[v1].Thread);
      }
      ++v0;
      ++v1;
    }
    while ( v0 < jqNWorkers );
  }
  jqStopSignal = 0;
  ResetEvent(jqNewJobAdded);
}

void __cdecl _jqAddBatch()
{
  ;
}

void __cdecl jqAlertWorkers()
{
  PulseEvent(jqNewJobAdded);
}

void jqLockBatchPoolInternal()
{
    // some bogus?
  //int Target; // [esp+0h] [ebp-4h] BYREF
  //
  //Target = (int)this;
  while ( _InterlockedCompareExchange(&jqPoolLock, 1, 0) )
    SwitchToThread();
  //Target = 0;
  //InterlockedExchange(&Target, 0);
}

void __cdecl jqUnlockBatchPoolInternal()
{
  volatile unsigned int Target; // [esp+0h] [ebp-4h] BYREF

  Target = 0;
  InterlockedExchange(&Target, 0);
  while ( _InterlockedCompareExchange(&jqPoolLock, 0, 1) != 1 )
    ;
}

void __cdecl jqKeepWorkersAwake()
{
  _InterlockedExchangeAdd(&jqKeepWorkersAwakeCount, 1u);
  if ( jqKeepWorkersAwakeCount > 0 )
    PulseEvent(jqNewJobAdded);
}

void jqLockBatchPool()
{
  jqLockBatchPoolInternal();
  _InterlockedExchangeAdd(&jqBatchPoolExternallyLockedCount, 1u);
}

void __cdecl jqUnlockBatchPool()
{
  if ( jqBatchPoolExternallyLockedCount <= 0
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         460,
         "jqBatchPoolExternallyLockedCount > 0",
         "Batch pool unlocked without being locked first.") )
  {
    __debugbreak();
  }
  _InterlockedExchangeAdd(&jqBatchPoolExternallyLockedCount, 0xFFFFFFFF);
  if ( !jqBatchPoolExternallyLockedCount )
    PulseEvent(jqNewJobAdded);
  jqUnlockBatchPoolInternal();
}

void __cdecl jqSetBatchDataHeapSize(unsigned int Size, unsigned int BlockSize)
{
  unsigned __int8 *v2; // eax

  if ( jqNWorkers
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         469,
         "!jqNWorkers",
         "This function can only be called when the JobQueue is stopped.") )
  {
    __debugbreak();
  }
  if ( !Size
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         470,
         "Size > 0",
         "Invalid batch pool size.") )
  {
    __debugbreak();
  }

  if ( jqPool.BatchDataHeap.LevelData )
    tlMemFree(jqPool.BatchDataHeap.LevelData);
  v2 = (unsigned __int8 *)tlMemAlloc(Size, 0x80u, 0);
  //jqAtomicHeap::Init(&jqPool.BatchDataHeap, v2, Size, BlockSize);
  jqPool.BatchDataHeap.Init(v2, Size, BlockSize);
}

void __cdecl jqInit()
{
  //jqAtomicQueue<jqBatch,32>::NodeType *Node; // eax
  unsigned int CurrentThreadId; // eax

  jqPool.ThisPtr = &jqPool;
  jqPool.BaseQueue.Queue.ThisPtr = &jqPool.BaseQueue.Queue;
  jqPool.BaseQueue.Queue._FreeList = 0;
  jqPool.BaseQueue.Queue.FreeListPtr = &jqPool.BaseQueue.Queue._FreeList;
  jqPool.BaseQueue.Queue.FreeLock.ThisPtr = &jqPool.BaseQueue.Queue.FreeLock;
  jqPool.BaseQueue.Queue.FreeLock.ThreadId = 0;
  jqPool.BaseQueue.Queue.FreeLock.LockCount = 0;
  jqPool.BaseQueue.Queue.NodeBlockListHead = 0;
  jqPool.BaseQueue.Queue.HeadLock.ThisPtr = &jqPool.BaseQueue.Queue.HeadLock;
  jqPool.BaseQueue.Queue.HeadLock.ThreadId = 0;
  jqPool.BaseQueue.Queue.HeadLock.LockCount = 0;
  jqPool.BaseQueue.Queue.TailLock.ThisPtr = &jqPool.BaseQueue.Queue.TailLock;
  jqPool.BaseQueue.Queue.TailLock.ThreadId = 0;
  jqPool.BaseQueue.Queue.TailLock.LockCount = 0;
  //Node = jqAtomicQueue<jqBatch,32>::AllocateNode(&jqPool.BaseQueue.Queue);
  auto Node = jqPool.BaseQueue.Queue.AllocateNode(); // another shitty use of auto 
  Node->Next = 0;
  jqPool.BaseQueue.Queue.Tail = Node;
  jqPool.BaseQueue.Queue.Head = Node;
  jqPool.BaseQueue.QueuedBatchCount = 0;
  //jqAtomicQueue<jqBatch,32>::AllocateNodeBlock(&jqPool.BaseQueue.Queue, 128);
  jqPool.BaseQueue.Queue.AllocateNodeBlock(128);
  CurrentThreadId = GetCurrentThreadId();
  jqSleepingWorkersCount = 0;
  jqMainThreadID = CurrentThreadId;
  _jqInit();
}

void __cdecl jqInitQueue(jqQueue *Queue)
{
  Queue->ThisPtr = Queue;
  Queue->QueuedBatchCount = 0;
  //jqAtomicQueue<jqBatch,32>::Init(&Queue->Queue, &jqPool.BaseQueue.Queue);
  Queue->Queue.Init(&jqPool.BaseQueue.Queue);
  Queue->ProcessorsMask = 0;
}

void __cdecl jqInitWorker(jqWorker *Worker)
{
  Worker->ThisPtr = Worker;
  Worker->WorkerSpecific.ThisPtr = &Worker->WorkerSpecific;
  Worker->WorkerSpecific.QueuedBatchCount = 0;
  //jqAtomicQueue<jqBatch,32>::Init(&Worker->WorkerSpecific.Queue, &jqPool.BaseQueue.Queue);
  Worker->WorkerSpecific.Queue.Init(&jqPool.BaseQueue.Queue);
  Worker->WorkerSpecific.ProcessorsMask = 0;
  Worker->NumQueues = 0;
  Worker->Queues[0] = 0;
  Worker->Queues[1] = 0;
  Worker->Queues[2] = 0;
  Worker->Queues[3] = 0;
  Worker->Queues[4] = 0;
  Worker->Queues[5] = 0;
  Worker->Queues[6] = 0;
  Worker->Queues[7] = 0;
}

void __cdecl jqAddBatchToQueue(jqBatch *Batch, jqQueue *Queue)
{
  jqBatchGroup *GroupID; // eax

  if ( !Batch->Module
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         295,
         "Batch.Module != NULL",
         "Batch has no module assigned.") )
  {
    __debugbreak();
  }
  GroupID = Batch->GroupID;
  if ( GroupID )
    _InterlockedExchangeAdd(&GroupID->QueuedBatchCount, 1u);
  _InterlockedExchangeAdd(&Batch->Module->Group.QueuedBatchCount, 1u);
  _InterlockedExchangeAdd(&jqPool.group.QueuedBatchCount, 1u);
  _InterlockedExchangeAdd(&Queue->QueuedBatchCount, 1u);
  //jqAtomicQueue<jqBatch,32>::Push(&Queue->Queue, Batch);
  Queue->Queue.Push(Batch);
  PulseEvent(jqNewJobAdded);
}

void __cdecl jqAddBatch(jqBatch *Batch, jqQueue *Queue)
{
  jqQueue *p_queue; // edi
  jqBatchGroup *GroupID; // eax

  p_queue = Queue;
  if ( !Queue )
      p_queue = &jqGlobalQueue;

  iassert(Batch->Module != NULL);

  GroupID = Batch->GroupID;
  if ( GroupID )
    _InterlockedExchangeAdd(&GroupID->QueuedBatchCount, 1u);

  _InterlockedExchangeAdd(&Batch->Module->Group.QueuedBatchCount, 1u);
  _InterlockedExchangeAdd(&jqPool.group.QueuedBatchCount, 1u);
  _InterlockedExchangeAdd(&p_queue->QueuedBatchCount, 1u);
  //jqAtomicQueue<jqBatch,32>::Push(&v2->Queue, Data);
  p_queue->Queue.Push(Batch);
  PulseEvent(jqNewJobAdded);
}

void __cdecl jqAddBatch(
        jqModule *Module,
        void *Input,
        void *Output,
        jqBatchGroup *GroupID,
        jqQueue *Queue,
        unsigned __int8 *ParamData,
        unsigned int ParamSize)
{
  jqBatch Batch; // [esp+4h] [ebp-7Ch] BYREF

  Batch.Module = Module;
  Batch.p3x_info = 0;
  Batch.ConditionalAddress = 0;
  Batch.ConditionalValue = 0;
  Batch.Input = Input;
  Batch.Output = Output;
  Batch.GroupID = GroupID;
  if ( ParamSize > 0x5C
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         356,
         "ParamSize >= 0 && ParamSize <= (int)sizeof(Batch.ParamData)",
         "Invalid Batch ParamSize.") )
  {
    __debugbreak();
  }

  if ( ParamData && ParamSize )
    memcpy((unsigned __int8 *)Batch.ParamData, ParamData, ParamSize);

  memset(Batch.ParamData + ParamSize, 0xCDu, 92 - ParamSize);
  jqAddBatch(&Batch, Queue);
}

void __cdecl jqSkipBatch()
{
  ///int v0; // eax
  ///
  ///v0 = *((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index);
  ///jqAddBatch(*(jqBatch **)(v0 + 8316), *(jqQueue **)(v0 + 8308));
  /// 
  //  TLS_HACK *v0; // eax
  //
  //  v0 = NtCurrentTeb()->ThreadLocalStoragePointer[_tls_index];
    jqAddBatch(jqCurBatch, jqCurQueue);
}

char __cdecl jqPopNextBatchFromQueue(jqQueue *Queue, jqBatchGroup *Worker, jqBatch *PoppedBatch)
{
    int QueuedBatchCount; // eax
    int v5; // ecx
    jqAtomicQueue<jqBatch, 32> *p_Queue; // edi
    jqBatchGroup *p_Group; // eax
    jqAtomicQueue<jqBatch, 32>::NodeType **FreeListPtr; // eax
    jqAtomicQueue<jqBatch, 32>::NodeType *list; // esi
    //tlSharedAtomicMutex *ThisPtr; // eax
    bool v11; // zf
    //tlSharedAtomicMutex *v12; // eax
    jqBatchGroup *v13; // eax
    int CheckedBatches; // [esp+Ch] [ebp-Ch]
    //LONG v16; // [esp+10h] [ebp-8h] BYREF
    LONG Target; // [esp+14h] [ebp-4h] BYREF

    QueuedBatchCount = Queue->ThisPtr->QueuedBatchCount;
    v5 = 0;
    if (QueuedBatchCount)
    {
        while (v5 <= QueuedBatchCount)
        {
            CheckedBatches = v5 + 1;
            p_Queue = &Queue->Queue;
            if (!Queue->Queue.Pop(PoppedBatch))
                break;
            p_Group = PoppedBatch->GroupID;
            if (!p_Group)
                p_Group = &PoppedBatch->Module->Group;

            jqCurQueue = Queue;

            if (!Worker || Worker == p_Group)
            {
                v13 = PoppedBatch->GroupID;
                if (v13)
                {
                    _InterlockedExchangeAdd(&v13->ExecutingBatchCount, 1u);
                    _InterlockedExchangeAdd(&PoppedBatch->GroupID->QueuedBatchCount, 0xFFFFFFFF);
                    if (PoppedBatch->GroupID->QueuedBatchCount < 0)
                    {
                        if (_tlAssert(
                            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_kernel.cpp",
                            79,
                            "PoppedBatch->GroupID->QueuedBatchCount >= 0",
                            "JobQueue group queued count went below zero!"))
                        {
                            __debugbreak();
                        }
                    }
                }
                _InterlockedExchangeAdd(&PoppedBatch->Module->Group.ExecutingBatchCount, 1u);
                _InterlockedExchangeAdd(&PoppedBatch->Module->Group.QueuedBatchCount, 0xFFFFFFFF);
                _InterlockedExchangeAdd(&jqPool.ThisPtr->group.ExecutingBatchCount, 1u);
                _InterlockedExchangeAdd(&jqPool.ThisPtr->group.QueuedBatchCount, 0xFFFFFFFF);
                _InterlockedExchangeAdd(&Queue->ThisPtr->QueuedBatchCount, 0xFFFFFFFF);
                if (jqPool.group.QueuedBatchCount < 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_kernel.cpp",
                        88,
                        "jqGetPool()->QueuedBatchCount >= 0",
                        "JobQueue global queued count went below zero!"))
                {
                    __debugbreak();
                }
                if (Queue->QueuedBatchCount < 0
                    && _tlAssert(
                        "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_kernel.cpp",
                        89,
                        "Queue->QueuedBatchCount >= 0",
                        "JobQueue queue count went below zero!"))
                {
                    __debugbreak();
                }
                return 1;
            }
            Queue->Queue.FreeLock.Lock();
            FreeListPtr = p_Queue->FreeListPtr;
            list = *p_Queue->FreeListPtr;
            if (!list)
            {
                p_Queue->AllocateNodeBlock(32);
                FreeListPtr = p_Queue->FreeListPtr;
                list = *p_Queue->FreeListPtr;
            }
            *FreeListPtr = list->Next;
            Queue->Queue.FreeLock.Unlock();
            //ThisPtr = p_Queue->FreeLock.ThisPtr;
            //v11 = ThisPtr->LockCount-- == 1;
            //if (v11)
            //{
            //    p_Queue->FreeLock.ThisPtr->ThreadId = 0;
            //}
            memcpy((unsigned __int8 *)&list->Data, (unsigned __int8 *)PoppedBatch, sizeof(list->Data));
            list->Next = 0;
            p_Queue->TailLock.Lock();
            p_Queue->ThisPtr->Tail->Next = list;
            p_Queue->ThisPtr->Tail = list;
            p_Queue->TailLock.Unlock();
            //v11 = p_Queue->TailLock.LockCount-- == 1;
            //if (v11)
            //{
            //    p_Queue->TailLock.ThreadId = 0;
            //}
            QueuedBatchCount = Queue->ThisPtr->QueuedBatchCount;
            if (!QueuedBatchCount)
                return 0;
            v5 = CheckedBatches;
        }
    }
    return 0;
}

char  jqPopNextBatch(
        jqWorker *Worker,
        bool *doHighPriority,
        jqBatchGroup *GroupID,
        jqBatch *PoppedBatch)
{
  int v7; // esi
  jqQueue **i; // edi

  if ( !jqPopNextBatchFromQueue(&Worker->WorkerSpecific, GroupID, PoppedBatch) )
  {
    if ( jqHighPriorityQueue.ThisPtr
      && *doHighPriority
      && jqPopNextBatchFromQueue(&jqHighPriorityQueue, GroupID, PoppedBatch) )
    {
      *doHighPriority = 0;
    }
    else
    {
      v7 = 0;
      for ( i = Worker->Queues; !*i || !jqPopNextBatchFromQueue(*i, GroupID, PoppedBatch); ++i )
      {
        if ( ++v7 >= 8 )
          return 0;
      }
    }
  }
  return 1;
}


void __cdecl jqWorkerLoop(jqWorker *Worker, jqBatchGroup *GroupID, bool BreakWhenEmpty, unsigned __int64 *batchCount)
{
    //unsigned __int64 v9; // rax
    //bool v10; // cf
    bool v11; // zf
    jqAtomicQueue<jqBatch,32> *p_Queue; // esi
    jqAtomicQueue<jqBatch,32>::NodeType **FreeListPtr; // eax
    jqAtomicQueue<jqBatch,32>::NodeType *freeList; // edi
    jqAtomicQueue<jqBatch,32>::NodeType *block; // eax
    //jqAtomicQueue<jqBatch,32>::NodeType *v17; // ecx
    //int v18; // edi
    //tlSharedAtomicMutex *ThisPtr; // eax
    jqBatch CurBatch; // [esp+Ch] [ebp-A4h] BYREF
    //LONG Target; // [esp+8Ch] [ebp-24h] BYREF
    //LONG v22; // [esp+90h] [ebp-20h] BYREF
    void *CachedConditionalAddress; // [esp+94h] [ebp-1Ch]
    unsigned __int64 lastConditionalCheckTime; // [esp+98h] [ebp-18h]
    unsigned int CachedConditionalValue; // [esp+A0h] [ebp-10h]
    int ret; // [esp+A8h] [ebp-8h]
    bool doHighPriority; // [esp+AFh] [ebp-1h] BYREF
    uint64_t totalTime;
    uint64_t tick64;

    if (jqWorkerInitFn && Worker->WorkerID > 0)
        jqWorkerInitFn(Worker->WorkerID);

    lastConditionalCheckTime = 0;
    CachedConditionalAddress = 0;
    CachedConditionalValue = 0;
    jqCurWorker = Worker;
    doHighPriority = 1;

    while (1)
    {
        if (jqPopNextBatch(Worker, &doHighPriority, GroupID, &CurBatch))
        {
            while (1)
            {
                jqCurBatch = &CurBatch;
                ret = 1;
                tick64 = tlPcGetTick().QuadPart;

                if (CachedConditionalAddress == CurBatch.ConditionalAddress
                    && CachedConditionalValue == CurBatch.ConditionalValue)
                {
                    break;
                }
                if (!CurBatch.ConditionalAddress)
                    goto LABEL_9;

                if (tick64 - lastConditionalCheckTime > 50000)
                {
                    if (*(_DWORD *)CurBatch.ConditionalAddress >= CurBatch.ConditionalValue)
                    {
                        CachedConditionalAddress = CurBatch.ConditionalAddress;
                        CachedConditionalValue = CurBatch.ConditionalValue;
                        CurBatch.ConditionalAddress = 0;
                        {
                            PROF_SCOPED_RUNTIME_NAME(CurBatch.Module->Name); // LWSS ADD (worker batch visibility)
                            ret = CurBatch.Module->Code(&CurBatch);
                        }
                    }
                    lastConditionalCheckTime = tick64;
                    goto LABEL_10;
                }
            LABEL_21:
                v11 = ret == 0;
                jqCurBatch = 0;
                if (!v11)
                {
                    _InterlockedExchangeAdd(&jqPool.ThisPtr->group.QueuedBatchCount, 1u);
                    _InterlockedExchangeAdd(&CurBatch.Module->Group.QueuedBatchCount, 1u);
                    _InterlockedExchangeAdd(&jqCurQueue->ThisPtr->QueuedBatchCount, 1u);

                    if (CurBatch.GroupID)
                    {
                        _InterlockedExchangeAdd(&CurBatch.GroupID->QueuedBatchCount, 1u);
                    }
                    jqCurQueue->Queue.Push(&CurBatch);
#if 0
                    p_Queue = (jqAtomicQueue<jqBatch,32> *)&jqCurQueue->Queue;
                    //tlSharedAtomicMutex::Lock(&jqCurQueue->Queue.FreeLock);
                    jqCurQueue->Queue.FreeLock.Lock();
                    FreeListPtr = p_Queue->FreeListPtr;
                    freeList = *p_Queue->FreeListPtr;
                    if (!freeList)
                    {
                        block = (jqAtomicQueue<jqBatch,32>::NodeType *)tlMemAlloc(sizeof(jqAtomicQueue<jqBatch, 32>::NodeType) * 32 + 8, 4, 0);
                        static_assert(sizeof(jqAtomicQueue<jqBatch, 32>::NodeType) * 32 + 8 == 0x1008);

                        //v17 = block;
                        //v18 = 31;
                        //do
                        //{
                        //    --v18;
                        //    v17->Next = v17 + 1;
                        //    ++v17;
                        //} while (v18);

                        for (int i = 0; i < 31; i++)
                            block[i].Next = &block[i + 1];

                        block[31].Next = 0;

                        block[32].Next = block; // The extra 8 bytes in the alloc() above cover the first bits of element [32]
                        block[32].Data.p3x_info = p_Queue->NodeBlockListHead;

                        p_Queue->NodeBlockListHead = (jqAtomicQueue<jqBatch,32>::NodeBlockEntry *)&block[32];
                        *p_Queue->FreeListPtr = block;
                        FreeListPtr = p_Queue->FreeListPtr;
                        freeList = *p_Queue->FreeListPtr;
                    }

                    *FreeListPtr = freeList->Next;
                    //ThisPtr = p_Queue->FreeLock.ThisPtr;
                    //v11 = ThisPtr->LockCount-- == 1;
                    //if (v11)
                    //{
                    //    //Target = 0;
                    //    //InterlockedExchange(&Target, 0);
                    //    p_Queue->FreeLock.ThisPtr->ThreadId = 0;
                    //}

                    p_Queue->FreeLock.Unlock();

                    memcpy((unsigned __int8 *)&freeList->Data, (unsigned __int8 *)&CurBatch, sizeof(freeList->Data));
                    freeList->Next = 0;
                    //tlAtomicMutex::Lock(&p_Queue->TailLock);
                    p_Queue->TailLock.Lock();
                    p_Queue->ThisPtr->Tail->Next = freeList;
                    p_Queue->ThisPtr->Tail = freeList;
                    p_Queue->TailLock.Unlock();
                    //v11 = p_Queue->TailLock.LockCount-- == 1;
                    //if (v11)
                    //{
                    //    v22 = 0;
                    //    InterlockedExchange(&v22, 0);
                    //    p_Queue->TailLock.ThreadId = 0;
                    //}
#endif
                }
                _InterlockedExchangeAdd(&jqPool.ThisPtr->group.ExecutingBatchCount, 0xFFFFFFFF);
                _InterlockedExchangeAdd(&CurBatch.Module->Group.ExecutingBatchCount, 0xFFFFFFFF);
                if (CurBatch.GroupID)
                    _InterlockedExchangeAdd(&CurBatch.GroupID->ExecutingBatchCount, 0xFFFFFFFF);
                if (BreakWhenEmpty && ret
                    || batchCount && !*batchCount
                    || !jqPopNextBatch(Worker, &doHighPriority, GroupID, &CurBatch))
                {
                    goto LABEL_40;
                }
            }
            CurBatch.ConditionalAddress = 0;
        LABEL_9:
            {
                PROF_SCOPED_RUNTIME_NAME(CurBatch.Module->Name); // LWSS ADD (worker batch visibility)
                ret = CurBatch.Module->Code(&CurBatch);
            }
        LABEL_10:
            if (ret)
            {
                if (ret == 2)
                    doHighPriority = 1;
            }
            else
            {
                //v9 = *(_QWORD *)&tlPcGetTick() - __PAIR64__(HighPart, LowPart);
                totalTime = tlPcGetTick().QuadPart - tick64;
                Worker->WorkTime += totalTime;
                //v10 = __CFADD__((_DWORD)v9, v4->WorkTime);
                //LODWORD(v4->WorkTime) += v9;
                doHighPriority = 1;
                //HIDWORD(v4->WorkTime) += HIDWORD(v9) + v10;
            }
            goto LABEL_21;
        }
    LABEL_40:
        doHighPriority = 1;
        if (BreakWhenEmpty)
            break;
        if (!jqWorkerSleep())
        {
            jqCurWorker = 0;
            return;
        }
    }
    jqCurWorker = 0;
}

void __cdecl jqTempWorkerLoop(jqWorker *Worker, jqBatchGroup *GroupID, bool (__cdecl *callback)(void *), void *context)
{
  jqWorker *v4; // ebx
  //int v5; // esi
  jqModule *Module; // ecx
  //int v7; // eax
  jqAtomicQueue<jqBatch,32> *p_queue; // esi
  jqAtomicQueue<jqBatch, 32>::NodeType **FreeListPtr; // eax
  jqAtomicQueue<jqBatch,32>::NodeType *freeList; // edi
  jqAtomicQueue<jqBatch, 32>::NodeType *block; // eax
  unsigned int *v12; // ecx
  int v13; // edi
  int v14; // eax
  bool v15; // zf
  unsigned int *v16; // ebx
  jqBatch CurBatch; // [esp+Ch] [ebp-94h] BYREF
  int v18; // [esp+88h] [ebp-18h] BYREF
  int Target; // [esp+8Ch] [ebp-14h] BYREF
  //int v20; // [esp+90h] [ebp-10h]
  int ret; // [esp+94h] [ebp-Ch]
  int numJobs; // [esp+98h] [ebp-8h]
  bool doHighPriority; // [esp+9Fh] [ebp-1h] BYREF

  v4 = Worker;
  //v5 = *((unsigned int *)NtCurrentTeb()->ThreadLocalStoragePointer + _tls_index);
  //v20 = v5;
  jqCurWorker = Worker;
  doHighPriority = 1;
  memset(&CurBatch, 0, 28);

  for ( ; !callback(context); doHighPriority = 1 )
  {
    numJobs = jqGetQueuedBatchCount(0);
    if ( jqPopNextBatch(v4, &doHighPriority, GroupID, &CurBatch) )
    {
      do
      {
        Module = CurBatch.Module;
        jqCurBatch = &CurBatch;
        {
            PROF_SCOPED_RUNTIME_NAME(Module->Name); // LWSS ADD (main-thread assist batch visibility)
            ret = Module->Code(&CurBatch);
        }
        jqCurBatch = 0;
        if ( !ret)
          goto LABEL_15;
        _InterlockedExchangeAdd(&jqPool.group.QueuedBatchCount, 1u);
        _InterlockedExchangeAdd(&CurBatch.Module->Group.QueuedBatchCount, 1u);
        _InterlockedExchangeAdd(&jqCurQueue->QueuedBatchCount, 1u);
        if ( CurBatch.GroupID )
          _InterlockedExchangeAdd(&CurBatch.GroupID->QueuedBatchCount, 1u);
        p_queue = &jqCurQueue->Queue;
        //tlSharedAtomicMutex::Lock((tlSharedAtomicMutex *)(p_queue + 24));
        p_queue->FreeLock.Lock();

        FreeListPtr = p_queue->FreeListPtr;
        freeList = *p_queue->FreeListPtr;

        if ( !freeList )
        {

          block = (jqAtomicQueue<jqBatch, 32>::NodeType *)tlMemAlloc(sizeof(jqAtomicQueue<jqBatch, 32>::NodeType) * 32 + 8, 4u, 0);
          static_assert(sizeof(jqAtomicQueue<jqBatch, 32>::NodeType) * 32 + 8 == 0x1008);

          //v12 = block;
          //v13 = 31;
          //do
          //{
          //  --v13;
          //  *v12 = v12 + 32;
          //  v12 += 32;
          //}
          //while ( v13 );

          for (int jk = 0; jk < 31; jk++)
              block[jk].Next = &block[jk + 1];

          block[31].Next = 0;
          block[32].Next = block;
          block[32].Data.p3x_info = p_queue->NodeBlockListHead;

          //*(unsigned int *)(p_queue + 8) = block + 1024;
          p_queue->NodeBlockListHead = (jqAtomicQueue<jqBatch,32>::NodeBlockEntry *)&block[32];

          //**(unsigned int **)p_queue = block;
          *p_queue->FreeListPtr = block;


          FreeListPtr = p_queue->FreeListPtr;
          freeList = *p_queue->FreeListPtr;
        }

        *FreeListPtr = freeList->Next;

        //v14 = *(unsigned int *)(p_queue + 36);
        p_queue->FreeLock.Unlock();
        //v15 = (*(unsigned int *)(v14 + 8))-- == 1;
        //if ( v15 )
        //{
        //  Target = 0;
        //  InterlockedExchange(&Target, 0);
        //  v16 = *(unsigned int **)(p_queue + 36);
        //  *v16 = 0;
        //  v16[1] = 0;
        //}
        memcpy((unsigned __int8 *)&freeList->Data, (unsigned __int8 *)&CurBatch, sizeof(freeList->Data));
        freeList->Next = 0;
        //*(unsigned int *)v10 = 0;
        //tlAtomicMutex::Lock((tlAtomicMutex *)(p_queue + 56));

        p_queue->TailLock.Lock();

        p_queue->ThisPtr->Tail->Next = freeList;
        p_queue->ThisPtr->Tail = freeList;

        p_queue->TailLock.Unlock();

        //v15 = (*(unsigned int *)(p_queue + 64))-- == 1;
        //if ( v15 )
        //{
        //  v18 = 0;
        //  InterlockedExchange(&v18, 0);
        //  *(unsigned int *)(p_queue + 56) = 0;
        //  *(unsigned int *)(p_queue + 60) = 0;
        //}
        // 
        //v5 = v20;

        if ( ret == 2 )
LABEL_15:
          doHighPriority = 1;
        _InterlockedExchangeAdd(&jqPool.group.ExecutingBatchCount, 0xFFFFFFFF);
        _InterlockedExchangeAdd(&CurBatch.Module->Group.ExecutingBatchCount, 0xFFFFFFFF);
        if ( CurBatch.GroupID )
          _InterlockedExchangeAdd(&CurBatch.GroupID->ExecutingBatchCount, 0xFFFFFFFF);
        if ( !ret || (--numJobs, numJobs <= 0) )
        {
          if ( callback(context) )
            goto LABEL_24;
          numJobs = jqGetQueuedBatchCount(0);
        }
        v4 = Worker;
      }
      while ( jqPopNextBatch(Worker, &doHighPriority, GroupID, &CurBatch) );
    }
  }
LABEL_24:
  jqCurWorker = 0;
}

unsigned int __stdcall jqWorkerThread(jqWorker *_this)
{
  char ThreadName[256]; // [esp+4h] [ebp-104h] BYREF

  sprintf(ThreadName, "JQ Worker %d (Processor 0x%x)", _this->WorkerID, _this->Processor);
  SetThreadName(0xFFFFFFFF, ThreadName);
  _this->ThreadId = GetCurrentThreadId();
  jqWorkerLoop(_this, 0, 0, 0);
  return 0;
}

void __cdecl _jqStart()
{
  int v0; // ebx
  bool v1; // zf
  jqWorker *v2; // esi
  HANDLE thr; // edi
  DWORD ThreadId; // [esp+4h] [ebp-8h] BYREF
  unsigned int i; // [esp+8h] [ebp-4h]

  v0 = 0;
  for ( i = 0; i < jqNWorkers; ++i )
  {
    v1 = jqWorkers[v0].Processor == 1;
    v2 = &jqWorkers[v0];
    jqWorkers[v0].Type = JQ_WORKER_GENERIC;
    if ( !v1 )
    {
      thr = CreateThread(0, 0x10000u, (LPTHREAD_START_ROUTINE)jqWorkerThread, v2, 4u, &ThreadId);
      if ( !thr )
      {
        if ( _tlAssert(
               "c:\\projects_pc\\cod\\codsrc\\tl\\base\\include\\tl_thread.h",
               311,
               "thr != 0",
               "tlCreateThread failed to create thread.") )
        {
          __debugbreak();
        }
      }
      v2->Thread = thr;
      ResumeThread(thr);
    }
    ++v0;
  }
}

void __cdecl jqFlush(jqBatchGroup *GroupID, unsigned __int64 batchCount)
{
    jqBatchGroup *p_group; // ebx
    unsigned __int64 zero; // [esp+Ch] [ebp-14h] BYREF
    unsigned __int64 *workerBatchCount; // [esp+18h] [ebp-8h]

    PulseEvent(jqNewJobAdded);
    if (GroupID)
    {
        p_group = GroupID;
    }
    else
    {
        p_group = &jqPool.group;
    }

    if (((uintptr_t)p_group & 7) != 0
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\xenon/jobqueue_xenon.cpp",
            140,
            "((u32)BatchCount & 0x7)==0",
            "Data not aligned correctly, read won't be atomic!\n"))
    {
        __debugbreak();
    }

    zero = 0;

    workerBatchCount = &p_group->BatchCount;
    if (batchCount)
        workerBatchCount = &zero;

    int  i = 0;
    while (1)
    {
        // Stop if nothing queued/executing
        if ((p_group->QueuedBatchCount + p_group->ExecutingBatchCount) == 0)
            break;

        // Stop if a max `batchCount` was specified and we've hit that limit
        if (batchCount && *workerBatchCount == 0)
            break;

        //Target = 0;
        //InterlockedExchange(&Target, 0);
        //if (!(QueuedBatchCount + *ExecutingBatchCount) && p_group->BatchCount <= batchCount)
        //    break;

        jqWorkerLoop(jqWorkers, GroupID, true, workerBatchCount);

        // blops2 added this
        if (i++ > 1)
            Sleep(0);
    }
}

void __cdecl jqStop()
{
  if ( jqMainThreadID != GetCurrentThreadId()
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         256,
         "jqGetCurrentThreadID() == jqGetMainThreadID()",
         "jqStop can only be called from the main thread.") )
  {
    __debugbreak();
  }
  if ( jqWorkers )
  {
    jqFlush(0, 0);
    if ( jqPool.group.QueuedBatchCount )
    {
      if ( _tlAssert(
             "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
             261,
             "jqPool.QueuedBatchCount == 0",
             "Stop failed to stop all batches.") )
      {
        __debugbreak();
      }
    }
    _jqStop();
    tlMemFree(jqTempWorkers);
    jqNWorkers = 0;
    tlMemFree(jqWorkers);
    jqWorkers = 0;
  }
}

void __cdecl jqAssistWithBatches(bool (__cdecl *callback)(void *), void *context, jqBatchGroup *GroupID)
{
  signed __int32 v3; // eax
  jqWorker *v4; // [esp-14h] [ebp-14h]

  if ( jqTempWorkers )
  {
    if ( !callback
      && _tlAssert(
           "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
           417,
           "callback",
           "You must supply a callback function") )
    {
      __debugbreak();
    }
    if ( jqNextAvailTempWorker >= 16 )
    {
      if ( _tlAssert(
             "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
             418,
             "jqNextAvailTempWorker < JQ_MAX_TEMP_WORKERS",
             "Ran out of temp workers - worker thread calling jqAssistWithBatches, or jqAssistWithBatches being called recursively?") )
      {
        __debugbreak();
      }
    }
    v3 = _InterlockedExchangeAdd(&jqNextAvailTempWorker, 1u);
    v4 = &jqTempWorkers[v3];
    v4->Processor = -1;
    v4->NumQueues = 1;
    v4->Queues[0] = &jqGlobalQueue;
    jqTempWorkerLoop(v4, GroupID, callback, context);
    _InterlockedExchangeAdd(&jqNextAvailTempWorker, 0xFFFFFFFF);
  }
}

void __cdecl jqShutdown()
{
  //jqAtomicQueue<jqBatch,32>::NodeBlockEntry *NodeBlockListHead; // esi
  void *Addr; // eax

  if ( jqMainThreadID != GetCurrentThreadId()
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
         119,
         "jqGetCurrentThreadID() == jqGetMainThreadID()",
         "jqShutdown() can only be called from the main thread.") )
  {
    __debugbreak();
  }
  jqStop();
  auto NodeBlockListHead = jqPool.BaseQueue.Queue.NodeBlockListHead; // shitty use of auto lol
  while ( NodeBlockListHead )
  {
    Addr = NodeBlockListHead->Addr;
    NodeBlockListHead = NodeBlockListHead->Next;
    tlMemFree(Addr);
  }
  CloseHandle(jqNewJobAdded);
  jqSleepingWorkersCount = 0;
}

void __cdecl jqStart()
{
#if 0
    unsigned int v0; // ecx
    int v1; // eax
    unsigned int v2; // eax
    int v3; // ecx
    char *v4; // edi
    int **v5; // esi
    int *v6; // ecx
    int *v7; // eax
    int v8; // ecx
    _DWORD *v10; // ebx
    int v11; // esi
    int v12; // eax
    int v13; // ecx
    int i; // ebx
    jqWorker *v15; // esi
    jqQueue *p_WorkerSpecific; // edi
    LONG v17; // [esp+Ch] [ebp-18h] BYREF
    unsigned int Processor; // [esp+10h] [ebp-14h]
    int v19; // [esp+14h] [ebp-10h]
    int v20; // [esp+18h] [ebp-Ch]
    int v21; // [esp+1Ch] [ebp-8h]
    int v22; // [esp+20h] [ebp-4h]

    if (jqMainThreadID != GetCurrentThreadId()
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
            191,
            "jqGetCurrentThreadID() == jqGetMainThreadID()",
            "jqStart can only be called from the main thread."))
    {
        __debugbreak();
    }
    jqStop();
    v0 = jqProcessorsMask | 1;
    jqProcessorsMask |= 1u;
    v1 = 0;
    do
    {
        v0 &= v0 - 1;
        ++v1;
    } while (v0);
    jqNWorkers = v1;
    jqWorkers = (jqWorker *)tlMemAlloc(168 * v1, 8u, 0);
    memset(jqWorkers, 0, 168 * jqNWorkers);
    v2 = jqProcessorsMask;
    v3 = 1;
    v19 = 0;
    v21 = 1;
    if (jqProcessorsMask)
    {
        v20 = 0;
        while (1)
        {
            if ((v3 & v2) == 0)
            {
                do
                    v3 *= 2;
                while ((v3 & v2) == 0);
                v21 = v3;
            }
            Processor = v3 ^ v2;
            v4 = (char *)jqWorkers + v20;
            *((_DWORD *)v4 + 4) = (DWORD)v4;
            *((_DWORD *)v4 + 30) = 0;
            *((_DWORD *)v4 + 8) = (DWORD)(v4 + 32);
            *((_DWORD *)v4 + 11) = 0;
            v5 = (int **)(v4 + 40);
            *((_DWORD *)v4 + 28) = (DWORD)(v4 + 40);
            *((_DWORD *)v4 + 10) = (DWORD)jqPool.BaseQueue.Queue.FreeListPtr;
            *((_DWORD *)v4 + 19) = (DWORD)&jqPool.BaseQueue.Queue.FreeLock;
            *((_DWORD *)v4 + 16) = 0;
            *((_DWORD *)v4 + 17) = 0;
            *((_DWORD *)v4 + 18) = 0;
            *((_DWORD *)v4 + 12) = 0;
            *((_DWORD *)v4 + 20) = 0;
            *((_DWORD *)v4 + 21) = 0;
            *((_DWORD *)v4 + 22) = 0;
            *((_DWORD *)v4 + 23) = (DWORD)(v4 + 80);
            *((_DWORD *)v4 + 24) = 0;
            *((_DWORD *)v4 + 25) = 0;
            *((_DWORD *)v4 + 26) = 0;
            *((_DWORD *)v4 + 27) = (DWORD)(v4 + 96);
            //tlSharedAtomicMutex::Lock((tlSharedAtomicMutex *)v4 + 4);
            ((tlSharedAtomicMutex *)v4 + 4)->Lock();
            v6 = (int *)*((_DWORD *)v4 + 10);
            v7 = (int *)*v6;
            v22 = (int)v7;
            if (!v7)
            {
                //jqAtomicQueue<jqBatch, 32>::AllocateNodeBlock((jqAtomicQueue<jqBatch, 32> *)(v4 + 40), 32);
                ((jqAtomicQueue<jqBatch, 32> *)(v4 + 40))->AllocateNodeBlock(32);
                v6 = *v5;
                v22 = **v5;
                v7 = (int *)v22;
            }
            *v6 = *v7;
            v8 = *((_DWORD *)v4 + 19);
            if ((*(_DWORD *)(v8 + 8))-- == 1)
            {
                v17 = 0;
                InterlockedExchange(&v17, 0);
                v10 = (_DWORD *)*((_DWORD *)v4 + 19);
                v7 = (int *)v22;
                *v10 = 0;
                v10[1] = 0;
            }
            *v7 = 0;
            *((_DWORD *)v4 + 14) = (DWORD)v7;
            *((_DWORD *)v4 + 13) = (DWORD)v7;
            v11 = v21;
            *((_DWORD *)v4 + 31) = 0;
            *((_DWORD *)v4 + 7) = 0;
            *((_DWORD *)v4 + 32) = 0;
            *((_DWORD *)v4 + 33) = 0;
            *((_DWORD *)v4 + 34) = 0;
            *((_DWORD *)v4 + 35) = 0;
            *((_DWORD *)v4 + 36) = 0;
            *((_DWORD *)v4 + 37) = 0;
            *((_DWORD *)v4 + 38) = 0;
            *((_DWORD *)v4 + 39) = 0;
            v12 = v20;
            *(int *)((char *)&jqWorkers->Processor + v20) = v11;
            v13 = v19;
            *(int *)((char *)&jqWorkers->WorkerID + v12) = v19;
            v19 = v13 + 1;
            v20 = v12 + 168;
            if (!Processor)
                break;
            v2 = Processor;
            v3 = v21;
        }
    }
    if (v19 != jqNWorkers
        && _tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
            220,
            "id == jqNWorkers",
            "Worker count mismatch."))
    {
        __debugbreak();
    }
    jqTempWorkers = (jqWorker *)tlMemAlloc(0xA80u, 8u, 0);
    memset(jqTempWorkers, 0, 0xA80u);
    for (i = 0; i < 16; ++i)
    {
        v15 = &jqTempWorkers[i];
        p_WorkerSpecific = &jqTempWorkers[i].WorkerSpecific;
        jqTempWorkers[i].ThisPtr = &jqTempWorkers[i];
        p_WorkerSpecific->ThisPtr = p_WorkerSpecific;
        p_WorkerSpecific->QueuedBatchCount = 0;
        //jqAtomicQueue<jqBatch, 32>::Init(&p_WorkerSpecific->Queue, &jqPool.BaseQueue.Queue);
        p_WorkerSpecific->Queue.Init(&jqPool.BaseQueue.Queue);
        p_WorkerSpecific->ProcessorsMask = 0;
        v15->NumQueues = 0;
        v15->Queues[0] = 0;
        v15->Queues[1] = 0;
        v15->Queues[2] = 0;
        v15->Queues[3] = 0;
        v15->Queues[4] = 0;
        v15->Queues[5] = 0;
        v15->Queues[6] = 0;
        v15->Queues[7] = 0;
    }
    jqGlobalQueue.ThisPtr = &jqGlobalQueue;
    jqGlobalQueue.QueuedBatchCount = 0;
    //jqAtomicQueue<jqBatch, 32>::Init(&jqGlobalQueue.Queue, &jqPool.BaseQueue.Queue);
    jqGlobalQueue.Queue.Init(&jqPool.BaseQueue.Queue);
    jqGlobalQueue.ProcessorsMask = 0;
    jqAttachQueueToWorkers(&jqGlobalQueue, 0xFFu);
    jqHighPriorityQueue.ThisPtr = &jqHighPriorityQueue;
    jqHighPriorityQueue.QueuedBatchCount = 0;
    //jqAtomicQueue<jqBatch, 32>::Init(&jqHighPriorityQueue.Queue, &jqPool.BaseQueue.Queue);
    jqHighPriorityQueue.Queue.Init(&jqPool.BaseQueue.Queue);
    jqHighPriorityQueue.ProcessorsMask = 0;
    jqAttachQueueToWorkers(&jqHighPriorityQueue, 0xFFu);
    _jqStart();
#else // aislop
    // Ensure called from main thread
    if (jqMainThreadID != GetCurrentThreadId())
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
            191,
            "jqGetCurrentThreadID() == jqGetMainThreadID()",
            "jqStart can only be called from the main thread."))
        {
            __debugbreak();
        }
    }

    // Stop any existing workers
    jqStop();

    // Always include the main processor
    jqProcessorsMask |= 1;

    // Count set bits to determine number of workers
    unsigned int mask = jqProcessorsMask;
    jqNWorkers = 0;
    while (mask)
    {
        mask &= mask - 1;
        jqNWorkers++;
    }

    // Allocate and zero-initialize worker array
    jqWorkers = (jqWorker *)tlMemAlloc(sizeof(jqWorker) * jqNWorkers, 8, 0);
    memset(jqWorkers, 0, sizeof(jqWorker) * jqNWorkers);

    // Initialize each worker
    int workerId = 0;
    unsigned int remainingMask = jqProcessorsMask;
    for (int processorBit = 1; remainingMask; processorBit <<= 1)
    {
        if (remainingMask & processorBit)
        {
            jqWorker *worker = &jqWorkers[workerId];
            worker->ThisPtr = worker;
            worker->Processor = processorBit;
            worker->WorkerID = workerId;
            worker->NumQueues = 0;

            // Initialize the worker-specific queue
            jqQueue *queue = &worker->WorkerSpecific;
            queue->ThisPtr = queue;
            queue->QueuedBatchCount = 0;
            queue->ProcessorsMask = 0;
            queue->Queue.Init(&jqPool.BaseQueue.Queue);

            // Clear worker queues
            for (int i = 0; i < 8; ++i)
                worker->Queues[i] = nullptr;

            // Mark this processor as used
            remainingMask ^= processorBit;
            workerId++;
        }
    }

    if (workerId != jqNWorkers)
    {
        if (_tlAssert(
            "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue.cpp",
            220,
            "id == jqNWorkers",
            "Worker count mismatch."))
        {
            __debugbreak();
        }
    }

    // Initialize temporary workers (for parallel batches, etc.)
    jqTempWorkers = (jqWorker *)tlMemAlloc(sizeof(jqWorker) * 16, 8, 0);
    memset(jqTempWorkers, 0, sizeof(jqWorker) * 16);
    for (int i = 0; i < 16; ++i)
    {
        jqWorker *temp = &jqTempWorkers[i];
        temp->ThisPtr = temp;
        jqQueue *queue = &temp->WorkerSpecific;
        queue->ThisPtr = queue;
        queue->QueuedBatchCount = 0;
        queue->ProcessorsMask = 0;
        queue->Queue.Init(&jqPool.BaseQueue.Queue);

        temp->NumQueues = 0;
        for (int q = 0; q < 8; ++q)
            temp->Queues[q] = nullptr;
    }

    // Initialize global queues
    jqGlobalQueue.ThisPtr = &jqGlobalQueue;
    jqGlobalQueue.QueuedBatchCount = 0;
    jqGlobalQueue.ProcessorsMask = 0;
    jqGlobalQueue.Queue.Init(&jqPool.BaseQueue.Queue);
    jqAttachQueueToWorkers(&jqGlobalQueue, 0xFF);

    jqHighPriorityQueue.ThisPtr = &jqHighPriorityQueue;
    jqHighPriorityQueue.QueuedBatchCount = 0;
    jqHighPriorityQueue.ProcessorsMask = 0;
    jqHighPriorityQueue.Queue.Init(&jqPool.BaseQueue.Queue);
    jqAttachQueueToWorkers(&jqHighPriorityQueue, 0xFF);

    // Call internal start routine
    _jqStart();
#endif
}

//void jqAtomicQueue<jqBatch,32>::AllocateNodeBlock(jqAtomicQueue<jqBatch,32> *this, int Count)
//{
//  int v2; // esi
//  jqAtomicQueue<jqBatch,32>::NodeType *v4; // eax
//  jqAtomicQueue<jqBatch,32>::NodeType *v5; // ecx
//  int v6; // ebx
//  jqAtomicQueue<jqBatch,32>::NodeType **v7; // ecx
//
//  v2 = Count << 7;
//  v4 = (jqAtomicQueue<jqBatch,32>::NodeType *)tlMemAlloc((Count << 7) + 8, 4u, 0);
//  if ( Count - 1 > 0 )
//  {
//    v5 = v4;
//    v6 = Count - 1;
//    do
//    {
//      --v6;
//      v5->Next = v5 + 1;
//      ++v5;
//    }
//    while ( v6 );
//  }
//  v7 = (jqAtomicQueue<jqBatch,32>::NodeType **)((char *)&v4->Next + v2);
//  *(v7 - 32) = 0;
//  *v7 = v4;
//  v7[1] = (jqAtomicQueue<jqBatch,32>::NodeType *)this->NodeBlockListHead;
//  this->NodeBlockListHead = (jqAtomicQueue<jqBatch,32>::NodeBlockEntry *)((char *)v4 + v2);
//  *this->FreeListPtr = v4;
//}

//jqAtomicQueue<jqBatch,32>::NodeType *jqAtomicQueue<jqBatch,32>::AllocateNode()
//{
//  tlSharedAtomicMutex *p_FreeLock; // ebx
//  jqAtomicQueue<jqBatch,32>::NodeType **FreeListPtr; // eax
//  jqAtomicQueue<jqBatch,32>::NodeType *v4; // esi
//  Ptr; // eax
//  tlSharedAtomicMutex *v7; // ebx
//  int Target; // [esp+Ch] [ebp-4h] BYREF
//
//  p_FreeLock = &this->FreeLock;
//  tlSharedAtomicMutex::Lock(&this->FreeLock);
//  FreeListPtr = this->FreeListPtr;
//  v4 = *this->FreeListPtr;
//  if ( !v4 )
//  {
//    jqAtomicQueue<jqBatch,32>::AllocateNodeBlock(this, 32);
//    FreeListPtr = this->FreeListPtr;
//    v4 = *this->FreeListPtr;
//  }
//  *FreeListPtr = v4->Next;
//  ThisPtr = p_FreeLock->ThisPtr;
//  if ( ThisPtr->LockCount-- == 1 )
//  {
//    Target = 0;
//    InterlockedExchange(&Target, 0);
//    v7 = p_FreeLock->ThisPtr;
//    LODWORD(v7->ThreadId) = 0;
//    HIDWORD(v7->ThreadId) = 0;
//  }
//  return v4;
//}

jqAtomicHeap::~jqAtomicHeap()
{
  unsigned __int8 *LevelData; // eax

  LevelData = this->LevelData;
  if ( LevelData )
    tlMemFree(LevelData);
  LODWORD(this->Mutex.ThreadId) = 0;
  HIDWORD(this->Mutex.ThreadId) = 0;
  this->Mutex.ThisPtr = 0;
}

void jqAtomicHeap::Init(
        unsigned __int8 *_HeapBase,
        unsigned int _HeapSize,
        unsigned int _BlockSize)
{
  unsigned int BlockSize; // edx
  unsigned int HeapSize; // edi
  unsigned int v7; // eax
  int v8; // ebx
  int *p_NBlocks; // edi
  int v10; // ecx
  unsigned __int8 *v11; // eax
  unsigned __int8 *LevelData; // ebx
  unsigned __int64 *v13; // edi
  unsigned __int64 **p_CellAvailable; // ecx
  signed int v15; // kr04_4
  unsigned int *v16; // esi
  int i; // [esp+14h] [ebp+8h]
  int ia; // [esp+14h] [ebp+8h]

  this->HeapBase = _HeapBase;
  this->HeapSize = _HeapSize;
  this->BlockSize = _BlockSize;
  BlockSize = this->BlockSize;
  this->ThisPtr = this;
  this->TotalUsed = 0;
  HeapSize = this->HeapSize;
  this->TotalBlocks = 0;
  v7 = BlockSize;
  for ( this->NLevels = 1; v7 < HeapSize; v7 *= 2 )
    ++this->NLevels;
  if ( BlockSize << (LOBYTE(this->NLevels) - 1) != HeapSize
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_atomic_heap.h",
         167,
         "(BlockSize << (NLevels-1)) == HeapSize",
         "HeapSize must be a power of 2 of BlockSize.") )
  {
    __debugbreak();
  }
  if ( this->NLevels > 11
    && _tlAssert(
         "c:\\projects_pc\\cod\\codsrc\\tl\\jobqueue\\jobqueue_atomic_heap.h",
         168,
         "NLevels <= JQ_ATOMIC_HEAP_MAX_LEVELS",
         "Maximum heap depth is 11 levels.") )
  {
    __debugbreak();
  }
  v8 = 0;
  i = 0;
  if ( this->NLevels > 0 )
  {
    p_NBlocks = &this->Levels[0].NBlocks;
    do
    {
      p_NBlocks += 5;
      *(p_NBlocks - 6) = this->BlockSize << v8;
      v10 = this->NLevels - v8++ - 1;
      *(p_NBlocks - 5) = 1 << v10;
      *(p_NBlocks - 4) = (unsigned int)((1 << v10) + 63) >> 6;
      i += (int)((*(p_NBlocks - 5) + 1023) & 0xFFFFFC00) / 8;
    }
    while ( v8 < this->NLevels );
  }
  v11 = (unsigned __int8 *)tlMemAlloc(2 * i, 0x80u, 0);
  this->LevelData = v11;
  memset(v11, 0, 2 * i);
  LevelData = this->LevelData;
  v13 = (unsigned __int64 *)&LevelData[i];
  ia = 0;
  if ( this->NLevels > 0 )
  {
    p_CellAvailable = &this->Levels[0].CellAvailable;
    do
    {
      v15 = ((unsigned int)*(p_CellAvailable - 2) + 1023) & 0xFFFFFC00;
      *p_CellAvailable = (unsigned __int64 *)LevelData;
      p_CellAvailable[1] = v13;
      LevelData += v15 / 8;
      v13 = (unsigned __int64 *)((char *)v13 + v15 / 8);
      p_CellAvailable += 5;
      ++ia;
    }
    while ( ia < this->NLevels );
  }
  v16 = (unsigned int *)*((unsigned int *)&this->TotalBlocks + 5 * this->NLevels);
  *v16 = 1;
  v16[1] = 0;
}

//void jqAtomicQueue<jqBatch,32>::Init(jqAtomicQueue<jqBatch,32> *SharedFreeList)
//{
//  jqAtomicQueue<jqBatch,32>::NodeType **p_FreeList; // ecx
//  tlSharedAtomicMutex *p_FreeLock; // eax
//  jqAtomicQueue<jqBatch,32>::NodeType *Node; // eax
//
//  p_FreeList = &this->_FreeList;
//  this->ThisPtr = this;
//  *p_FreeList = 0;
//  if ( !SharedFreeList )
//  {
//    this->FreeListPtr = p_FreeList;
//    p_FreeLock = &this->FreeLock;
//    goto LABEL_5;
//  }
//  this->FreeListPtr = SharedFreeList->FreeListPtr;
//  p_FreeLock = &this->FreeLock;
//  if ( SharedFreeList == (jqAtomicQueue<jqBatch,32> *)-24 )
//  {
//LABEL_5:
//    p_FreeLock->ThisPtr = p_FreeLock;
//    goto LABEL_6;
//  }
//  this->FreeLock.ThisPtr = &SharedFreeList->FreeLock;
//LABEL_6:
//  LODWORD(p_FreeLock->ThreadId) = 0;
//  HIDWORD(p_FreeLock->ThreadId) = 0;
//  p_FreeLock->LockCount = 0;
//  this->NodeBlockListHead = 0;
//  this->HeadLock.ThisPtr = &this->HeadLock;
//  LODWORD(this->HeadLock.ThreadId) = 0;
//  HIDWORD(this->HeadLock.ThreadId) = 0;
//  this->HeadLock.LockCount = 0;
//  this->TailLock.ThisPtr = &this->TailLock;
//  LODWORD(this->TailLock.ThreadId) = 0;
//  HIDWORD(this->TailLock.ThreadId) = 0;
//  this->TailLock.LockCount = 0;
//  Node = jqAtomicQueue<jqBatch,32>::AllocateNode(this);
//  Node->Next = 0;
//  this->Tail = Node;
//  this->Head = Node;
//}

//void jqAtomicQueue<jqBatch,32>::Push(jqAtomicQueue<jqBatch,32> *this, jqBatch *Data)
//{
//  jqAtomicQueue<jqBatch,32>::NodeType *Node; // edi
//
//  Node = jqAtomicQueue<jqBatch,32>::AllocateNode(this);
//  memcpy((unsigned __int8 *)&Node->Data, (unsigned __int8 *)Data, sizeof(Node->Data));
//  Node->Next = 0;
//  tlAtomicMutex::Lock(&this->TailLock);
//  this->ThisPtr->Tail->Next = Node;
//  this->ThisPtr->Tail = Node;
//  if ( this->TailLock.LockCount-- == 1 )
//  {
//    Data = 0;
//    InterlockedExchange((volatile int *)&Data, 0);
//    LODWORD(this->TailLock.ThreadId) = 0;
//    HIDWORD(this->TailLock.ThreadId) = 0;
//  }
//}
//
//char jqAtomicQueue<jqBatch,32>::Pop(jqAtomicQueue<jqBatch,32> *this, jqBatch *p)
//{
//  tlAtomicMutex *p_HeadLock; // edi
//  jqAtomicQueue<jqBatch,32>::NodeType *Next; // ebx
//  bool v5; // zf
//  jqAtomicQueue<jqBatch,32>::NodeType *v7; // eax
//  Ptr; // eax
//  tlSharedAtomicMutex *v9; // edi
//  jqAtomicQueue<jqBatch,32>::NodeType *Node; // [esp+Ch] [ebp-8h] BYREF
//  int Target; // [esp+10h] [ebp-4h] BYREF
//
//  p_HeadLock = &this->HeadLock;
//  tlAtomicMutex::Lock(&this->HeadLock);
//  Next = this->ThisPtr->Head->Next;
//  Node = this->ThisPtr->Head;
//  if ( Next )
//  {
//    memcpy((unsigned __int8 *)p, (unsigned __int8 *)&Next->Data, sizeof(jqBatch));
//    this->ThisPtr->Head = Next;
//    v5 = p_HeadLock->LockCount-- == 1;
//    if ( v5 )
//    {
//      Target = 0;
//      InterlockedExchange(&Target, 0);
//      LODWORD(p_HeadLock->ThreadId) = 0;
//      HIDWORD(p_HeadLock->ThreadId) = 0;
//    }
//    tlSharedAtomicMutex::Lock(&this->FreeLock);
//    v7 = Node;
//    Node->Next = *this->FreeListPtr;
//    *this->FreeListPtr = v7;
//    ThisPtr = this->FreeLock.ThisPtr;
//    v5 = ThisPtr->LockCount-- == 1;
//    if ( v5 )
//    {
//      Node = 0;
//      InterlockedExchange((volatile int *)&Node, 0);
//      v9 = this->FreeLock.ThisPtr;
//      LODWORD(v9->ThreadId) = 0;
//      HIDWORD(v9->ThreadId) = 0;
//    }
//    return 1;
//  }
//  else
//  {
//    v5 = p_HeadLock->LockCount-- == 1;
//    if ( v5 )
//    {
//      p = 0;
//      InterlockedExchange((volatile int *)&p, 0);
//      LODWORD(p_HeadLock->ThreadId) = 0;
//      HIDWORD(p_HeadLock->ThreadId) = 0;
//    }
//    return 0;
//  }
//}

jqQueue::~jqQueue()
{
  this->Queue.TailLock.ThreadId = 0;
  this->Queue.TailLock.ThisPtr = 0;
  this->Queue.HeadLock.ThreadId = 0;
  this->Queue.HeadLock.ThisPtr = 0;
}

jqBatchPool::~jqBatchPool()
{
  //jqAtomicHeap::~jqAtomicHeap(&this->BatchDataHeap);
  this->BatchDataHeap.~jqAtomicHeap();
  this->BaseQueue.Queue.TailLock.ThreadId = 0;
  this->BaseQueue.Queue.TailLock.ThisPtr = 0;
  this->BaseQueue.Queue.HeadLock.ThreadId = 0;
  this->BaseQueue.Queue.HeadLock.ThisPtr = 0;
}

