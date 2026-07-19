#pragma once
#include <cstddef>
#include <tl/tl_system.h>

#include <Windows.h>

#define JQ_MAX_QUEUES 8

enum jqProcessor : __int32
{                                       // XREF: ?jqFindWorkerForProcessor@@YAPAUjqWorker@@W4jqProcessor@@@Z/r
    JQ_CORE_0   = 0x1,
    JQ_CORE_1   = 0x2,
    JQ_CORE_2   = 0x4,
    JQ_CORE_3   = 0x8,
    JQ_CORE_4   = 0x10,
    JQ_CORE_5   = 0x20,
    JQ_CORE_6   = 0x40,
    JQ_CORE_7   = 0x80,
    JQ_CORE_ALL = 0xFF,
};

struct tlAtomicMutex // sizeof=0x10
{                                       // XREF: .data:tlAtomicMutex g_prolog_task_mutex/r
    volatile LONG64 ThreadId;          // XREF: jqInit(void)+58/w
    int LockCount;                      // XREF: jqInit(void)+64/w
    tlAtomicMutex *ThisPtr;             // XREF: jqInit(void)+4E/w

    tlAtomicMutex()
    {
        ThisPtr = this;
        ThreadId = 0;
        LockCount = 0;
    }
    ~tlAtomicMutex()
    {
        this->ThreadId = 0;
        this->ThisPtr = 0;
    }

    void Lock();
    bool TryLock();
    void Unlock();
};

struct tlAtomicMutexLocker // sizeof=0x4
{
    tlAtomicMutex *Mutex;
};

struct jqAtomicHeap // sizeof=0x110
{                                       // XREF: jqBatchPool/r

    struct LevelInfo // sizeof=0x14
    {                                       // XREF: jqAtomicHeap/r
        unsigned int BlockSize;
        int NBlocks;
        int NCells;
        unsigned __int64 *CellAvailable;
        unsigned __int64 *CellAllocated;
    };
    jqAtomicHeap *ThisPtr;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    tlAtomicMutex Mutex;                // XREF: _dynamic_initializer_for__jqPool__+48/w
                                        // _dynamic_initializer_for__jqPool__+52/w ...
    unsigned __int8 *HeapBase;          // XREF: _dynamic_initializer_for__jqPool__+61/w
    unsigned int HeapSize;              // XREF: jqGetBatchDataAvailable(void)+5/r
                                        // _dynamic_initializer_for__jqPool__+66/w
    unsigned int BlockSize;             // XREF: _dynamic_initializer_for__jqPool__+6B/w
    volatile unsigned int TotalUsed;    // XREF: jqGetBatchDataAvailable(void)/r
    volatile unsigned int TotalBlocks;
    int NLevels;
    jqAtomicHeap::LevelInfo Levels[11];
    unsigned __int8 *LevelData;         // XREF: jqSetBatchDataHeapSize(uint,uint):loc_A0A4F6/r
                                        // _dynamic_initializer_for__jqPool__+70/w

    ~jqAtomicHeap();

    unsigned __int8 *Alloc(unsigned int Size, unsigned int Align);
    char AllocBlock(LevelInfo **FitLevel, int *FitSlot);
    unsigned __int8 *AllocLevel(int LevelIdx);

    void FindAllocatedBlock(
        unsigned int Offset,
        LevelInfo **FitLevel,
        int *FitSlot);

    int FindLevelForSize(unsigned int Size);
    void Free(LevelInfo *Ptr);
    bool GetAvailableBlock(LevelInfo *FitLevel, int *FitSlot);

    void Init(
        unsigned __int8 *_HeapBase,
        unsigned int _HeapSize,
        unsigned int _BlockSize);

    void MergeBlocks(LevelInfo **FitLevel, int *FitSlot);

    int SplitBlock(
        LevelInfo *Level,
        int Slot,
        LevelInfo *LevelTo);
};

struct tlSharedAtomicMutex // sizeof=0x10
{                                       // XREF: jqAtomicQueue<jqBatch,32>/r
    volatile unsigned __int64 ThreadId; // XREF: jqInit(void)+31/w
                                        // jqInit(void)+37/w ...
    volatile int LockCount;             // XREF: jqInit(void)+42/w
                                        // _dynamic_initializer_for__jqPool__+11/w ...
    tlSharedAtomicMutex *ThisPtr;       // XREF: jqInit(void)+27/w

    tlSharedAtomicMutex()
    {
        ThreadId = 0;
        LockCount = 0;
        ThisPtr = this;
    }

    void Lock();
    void Unlock();
};


enum jqWorkerType : __int32
{                                       // XREF: jqModule/r _jqWorker/r
    JQ_WORKER_GENERIC = 0x0,            // XREF: .data:jqModule bp_env_jq_module1Module/s
                                        // .data:jqModule bp_env_jq_module2Module/s ...
    JQ_WORKER_MAX     = 0x1,
    JQ_WORKER_DEFAULT = 0x0,
};

struct alignas(8) jqBatchGroup // sizeof=0x8
{                                       // XREF: jqModule/r
                                        // GfxDrawSurfListInfo/r
    //$F761E618955D9ED935731AE37AFEF266 ___u0;
    union //$F761E618955D9ED935731AE37AFEF266 // sizeof=0x8
    {                                       // XREF: jqGetQueuedBatchCount(jqBatchGroup *)+A/r
                                            // jqGetExecutingBatchCount(jqBatchGroup *)+A/r ...
        //$2BD02F38FBEBD854EF9A531D8B9F9671 __s0;
        struct //$2BD02F38FBEBD854EF9A531D8B9F9671 // sizeof=0x8
        {                                       // XREF: $F761E618955D9ED935731AE37AFEF266/r
            volatile unsigned int QueuedBatchCount;
            volatile unsigned int ExecutingBatchCount;
        };
        unsigned __int64 BatchCount;
    };

    jqBatchGroup();
};
static_assert(alignof(jqBatchGroup) == 8, "jqBatchGroup must be 8-byte aligned for atomic 64-bit reads");

struct jqModule // sizeof=0x18
{                                       // XREF: .data:jqModule bp_env_jq_module1Module/r
                                        // .data:jqModule bp_env_jq_module2Module/r ...
    const char *Name;                   // XREF: _dynamic_initializer_for__fx_add_markModule__+6/w
                                        // _dynamic_initializer_for__nuge_physicsModule__+6/w ...
    jqWorkerType Type;                  // XREF: _dynamic_initializer_for__fx_add_markModule__+10/w
                                        // _dynamic_initializer_for__nuge_physicsModule__+10/w ...
    int (__cdecl *Code)(struct jqBatch *);     // XREF: _dynamic_initializer_for__fx_add_markModule__+1A/w
                                        // _dynamic_initializer_for__nuge_physicsModule__+1A/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    jqBatchGroup Group;                 // XREF: R_FinishedFrontendWorkerCmds(void)+3/o
                                        // R_FinishedFrontendWorkerCmds(void):loc_7B97EE/o ...
};

struct _jqBatch // sizeof=0x0
{                                       // XREF: jqBatch/r
};

struct __declspec(align(4)) jqBatch // sizeof=0x7C
{                                       // XREF: .data:jqBatch g_phys_task_manager_batch/r
                                        // jqAtomicQueue<jqBatch,32>::NodeType/r ...
    void *p3x_info;                     // XREF: jqAddBatch(jqModule const &,void *,void *,jqBatchGroup *,jqQueue *,void *,int)+19/w
                                        // jqWorkerLoop(jqWorker *,jqBatchGroup *,bool,unsigned __int64 *)+50/w ...
    void *Input;                        // XREF: Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+2E/w
                                        // jqAddBatch(jqModule const &,void *,void *,jqBatchGroup *,jqQueue *,void *,int)+2E/w ...
    void *Output;                       // XREF: jqAddBatch(jqModule const &,void *,void *,jqBatchGroup *,jqQueue *,void *,int)+31/w
                                        // jqWorkerLoop(jqWorker *,jqBatchGroup *,bool,unsigned __int64 *)+5C/w ...
    jqModule *Module;                   // XREF: Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+39/w
                                        // R_InitWorkerCmds(void)+34/w ...
    jqBatchGroup *GroupID;              // XREF: Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+3C/w
                                        // jqAddBatch(jqModule const &,void *,void *,jqBatchGroup *,jqQueue *,void *,int)+34/w ...
    void *ConditionalAddress;           // XREF: Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+4E/w
                                        // jqAddBatch(jqModule const &,void *,void *,jqBatchGroup *,jqQueue *,void *,int)+20/w ...
    unsigned int ConditionalValue;      // XREF: Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+57/w
                                        // jqAddBatch(jqModule const &,void *,void *,jqBatchGroup *,jqQueue *,void *,int)+27/w ...
    unsigned int ParamData[23];         // XREF: Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+5D/w
                                        // Sys_AddWorkerCmdInternal(jqWorkerCmd *,void *,WorkerCmdConditional *)+6F/w ...
    _jqBatch _Batch;
    // padding byte
    // padding byte
    // padding byte

    jqBatch();
};
static_assert(sizeof(jqBatch) == 124);

template <typename T, int SIZE>
struct __declspec(align(8)) jqAtomicQueue//<jqBatch,32> // sizeof=0x50
{                                       // XREF: jqQueue/r
    struct NodeType // sizeof=0x80
    {
        jqAtomicQueue<T, SIZE>::NodeType *Next;
        jqBatch Data;
    };

    struct NodeBlockEntry//<jqBatch,32>::NodeBlockEntry // sizeof=0x8
    {
        void *Addr;
        jqAtomicQueue<T,SIZE>::NodeBlockEntry *Next;
    };

    NodeType **FreeListPtr;
    NodeType *_FreeList;
    NodeBlockEntry *NodeBlockListHead;
    NodeType *Head; // XREF: jqInit(void)+9C/w
    NodeType *Tail; // XREF: jqInit(void)+97/w
    tlSharedAtomicMutex FreeLock;       // XREF: jqInit(void)+27/w
    tlAtomicMutex HeadLock;             // XREF: jqInit(void)+4E/w
    tlAtomicMutex TailLock;             // XREF: jqInit(void)+6A/w
    jqAtomicQueue<T,SIZE> *ThisPtr; // XREF: jqInit(void)+D/w

    void Init(jqAtomicQueue<T, SIZE> *SharedFreeList)
    {
        NodeType **p_FreeList = &_FreeList;
        ThisPtr = this;
        *p_FreeList = nullptr;

        if (!SharedFreeList)
        {
            FreeListPtr = p_FreeList;
            FreeLock.ThisPtr = &FreeLock;
        }
        else
        {
            FreeListPtr = SharedFreeList->FreeListPtr;
            FreeLock.ThisPtr = &SharedFreeList->FreeLock;
        }

        FreeLock.ThreadId = 0;
        FreeLock.LockCount = 0;

        NodeBlockListHead = nullptr;

        HeadLock.ThisPtr = &HeadLock;
        HeadLock.ThreadId = 0;
        HeadLock.LockCount = 0;

        TailLock.ThisPtr = &TailLock;
        TailLock.ThreadId = 0;
        TailLock.LockCount = 0;

        NodeType *Node = AllocateNode();
        Node->Next = nullptr;
        Tail = Node;
        Head = Node;
    }

    NodeType *AllocateNode()
    {
        FreeLock.Lock();
        NodeType *Node = *FreeListPtr;
        if (!Node)
        {
            AllocateNodeBlock(SIZE);
            Node = *FreeListPtr;
        }
        *FreeListPtr = Node->Next;
        FreeLock.Unlock();
        return Node;
    }

    void AllocateNodeBlock(int Count)
    {
        int blockSize = Count * sizeof(NodeType);
        NodeType *block = (NodeType *)tlMemAlloc(blockSize + sizeof(NodeBlockEntry), 4u, 0);

        // Chain the nodes
        for (int i = 0; i < Count - 1; ++i)
            block[i].Next = &block[i + 1];
        block[Count - 1].Next = nullptr;

        // Store NodeBlockEntry at end of allocation
        NodeBlockEntry *entry = reinterpret_cast<NodeBlockEntry *>(reinterpret_cast<char *>(block) + blockSize);
        entry->Addr = block;
        entry->Next = NodeBlockListHead;
        NodeBlockListHead = entry;

        *FreeListPtr = block;
    }

    bool Pop(jqBatch *p)
    {
        HeadLock.Lock();
        NodeType *Next = ThisPtr->Head->Next;
        NodeType *OldHead = ThisPtr->Head;

        if (Next)
        {
            memcpy(p, &Next->Data, sizeof(jqBatch));
            ThisPtr->Head = Next;
            HeadLock.Unlock();

            FreeLock.Lock();
            OldHead->Next = *FreeListPtr;
            *FreeListPtr = OldHead;
            FreeLock.Unlock();

            return true;
        }
        else
        {
            HeadLock.Unlock();
            return false;
        }
    }

    void Push(jqBatch *Data)
    {
        NodeType *Node = AllocateNode();
        memcpy(&Node->Data, Data, sizeof(jqBatch));
        Node->Next = nullptr;

        TailLock.Lock();
        ThisPtr->Tail->Next = Node;
        ThisPtr->Tail = Node;
        TailLock.Unlock();
    }
};

typedef jqAtomicQueue<jqBatch, 32> jqAtomicQueueType;

struct jqQueue // sizeof=0x60
{                                       // XREF: .data:jqQueue jqGlobalQueue/r
                                        // .data:jqQueue localQueue/r ...
    jqQueue *ThisPtr;                   // XREF: jqPopNextBatch+22/r
                                        // jqStart(void)+2C6/w ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    jqAtomicQueue<jqBatch,32> Queue;    // XREF: jqInit(void)+D/w
                                        // jqInit(void)+17/w ...
    volatile unsigned int QueuedBatchCount;               // XREF: jqInit(void)+A1/w
                                        // jqStart(void)+2D0/w ...
    unsigned int ProcessorsMask;        // XREF: jqStart(void)+2E5/w
                                        // jqStart(void)+31C/w
    ~jqQueue();
};


struct __declspec(align(8)) jqWorkerCmd // sizeof=0x20
{                                       // XREF: .data:jqWorkerCmd fx_add_markWorkerCmd/r
                                        // .data:jqWorkerCmd nuge_physicsWorkerCmd/r ...
    jqModule *module;
    unsigned int dataSize;
    volatile unsigned int ppu_fence;
    volatile unsigned int spu_fence;
    volatile unsigned int *spuThreadLimit;
    jqQueue *queue;
    unsigned int string_table;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) _jqWorker // sizeof=0x10
{                                       // XREF: jqWorker/r
    jqWorkerType Type;
    void *Thread;
    unsigned int ThreadId;
    bool Idle;
    // padding byte
    // padding byte
    // padding byte
};

struct jqWorker : _jqWorker // sizeof=0xA8
{                                       // XREF: .data:jqWorker jqMainThreadWorker/r
    jqWorker *ThisPtr;
    int Processor;
    int WorkerID;
    int NumQueues;
    jqQueue WorkerSpecific;             // XREF: _dynamic_initializer_for__jqMainThreadWorker__+2/w
                                        // _dynamic_initializer_for__jqMainThreadWorker__+7/w ...
    jqQueue *Queues[8];
    unsigned __int64 WorkTime;
};

struct jqBatchPool // sizeof=0x180
{                                       // XREF: .data:jqBatchPool jqPool/r
    jqBatchPool *ThisPtr;               // XREF: jqInit(void)+3/w
                                        // jqPopNextBatchFromQueue+189/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    jqQueue BaseQueue;                  // XREF: jqInit(void)+D/w
                                        // jqInit(void)+17/w ...
    jqBatchGroup group;                 // XREF: jqGetQueuedBatchCount(jqBatchGroup *)+A/r
                                        // jqGetExecutingBatchCount(jqBatchGroup *)+A/r ...
    jqAtomicHeap BatchDataHeap;         // XREF: jqAllocBatchData(uint)+9/o
                                        // jqFreeBatchData(void *)+7/o ...

    ~jqBatchPool();
};
static_assert(offsetof(jqBatchPool, group) % 8 == 0, "jqPool.group must be 8-byte aligned");


unsigned int __cdecl tlAtomicAdd(volatile unsigned __int32 *var, unsigned int value);
unsigned __int64 __cdecl tlAtomicAnd(volatile unsigned __int64 *var, unsigned __int64 value);
unsigned __int64 __cdecl tlAtomicOr(volatile unsigned __int64 *var, unsigned __int64 value);


void __cdecl jqAttachQueueToWorkers(jqQueue *Queue, unsigned int ProcessorMask);
void __cdecl jqEnableWorkers(unsigned int ProcessorsMask);
int __cdecl jqGetNumWorkers();
unsigned __int64 __cdecl jqGetCurrentThreadID();
unsigned __int64 __cdecl jqGetMainThreadID();
jqBatch *__cdecl jqGetCurrentBatch();
jqWorker *__cdecl jqGetCurrentWorker();
jqQueue *__cdecl jqGetWorkerQueue(int worker);
void __cdecl jqShutdownWorker();
int __cdecl jqGetQueuedBatchCount(jqBatchGroup *GroupID);
int __cdecl jqGetExecutingBatchCount(jqBatchGroup *GroupID);
jqWorker *__cdecl jqFindWorkerForProcessor(jqProcessor Processor);
bool __cdecl jqPoll(jqBatchGroup *GroupID);
bool __cdecl jqAreJobsQueued(jqBatchGroup *GroupID);
void __cdecl jqSetWorkerInitFunction(void (__cdecl *fn)(int));
void __cdecl jqLetWorkersSleep();
unsigned __int8 *__cdecl jqAllocBatchData(unsigned int Size);
void __cdecl jqFreeBatchData(jqAtomicHeap::LevelInfo *Ptr);
unsigned int __cdecl jqGetBatchDataAvailable();
int __cdecl jqExecuteBatch(jqWorker *Worker, jqBatch *Batch);
char __cdecl jqCanBatchExecute();
bool __cdecl jqWorkerSleep();
void __cdecl jqSetCheckContext(const char *desc);
void __cdecl jqCheckDMALS(const void *addr);
void __cdecl jqCheckDMAMain(const void *addr);
void __cdecl jqCheckDMASize(unsigned int size);
void __cdecl jqCheckDMATag(unsigned int tag);
void __cdecl jqCheckRange(int val, int mn, int mx);
void __cdecl jqCheckStack();
unsigned __int8 *__cdecl jqFetch(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size);
void __cdecl jqStore(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size);
unsigned __int8 *__cdecl jqFetchAsync(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size);
void __cdecl jqStoreAsync(unsigned __int8 *dest, unsigned __int8 *src, unsigned int size);
void __cdecl jqWait();
void __cdecl jqWaitMultiple();
void __cdecl jqSetMemBase();
void __cdecl jqSetStackSize();
int __cdecl jqGetMemAvailable();
void *__cdecl jqAlloc();
void *__cdecl jqGetMemBase();
void __cdecl _jqInit();
void __cdecl _jqShutdown();
void __cdecl _jqStop();
void __cdecl _jqAddBatch();
void __cdecl jqAlertWorkers();
void jqLockBatchPoolInternal();
void __cdecl jqUnlockBatchPoolInternal();
void __cdecl jqKeepWorkersAwake();
void jqLockBatchPool();
void __cdecl jqUnlockBatchPool();
void __cdecl jqSetBatchDataHeapSize(unsigned int Size, unsigned int BlockSize);
void __cdecl jqInit();
void __cdecl jqInitQueue(jqQueue *Queue);
void __cdecl jqInitWorker(jqWorker *Worker);
void __cdecl jqAddBatchToQueue(jqBatch *Batch, jqQueue *Queue);
void __cdecl jqAddBatch(jqBatch *Data, jqQueue *Queue);
void __cdecl jqAddBatch(
        jqModule *Module,
        void *Input,
        void *Output,
        jqBatchGroup *GroupID,
        jqQueue *Queue,
        unsigned __int8 *ParamData,
        unsigned int ParamSize);
void __cdecl jqSkipBatch();
char __cdecl jqPopNextBatchFromQueue(jqQueue *Queue, jqBatchGroup *Worker, jqBatch *PoppedBatch);
char  jqPopNextBatch(
        jqWorker *Worker,
        bool *doHighPriority,
        jqBatchGroup *GroupID,
        jqBatch *PoppedBatch);
void __cdecl jqWorkerLoop(jqWorker *Worker, jqBatchGroup *GroupID, bool BreakWhenEmpty, unsigned __int64 *batchCount);
void __cdecl jqTempWorkerLoop(jqWorker *Worker, jqBatchGroup *GroupID, bool (__cdecl *callback)(void *), void *context);
unsigned int __stdcall jqWorkerThread(jqWorker *_this);
void __cdecl _jqStart();
void __cdecl jqFlush(jqBatchGroup *GroupID, unsigned __int64 batchCount);
void __cdecl jqStop();
void __cdecl jqAssistWithBatches(bool (__cdecl *callback)(void *), void *context, jqBatchGroup *GroupID);
void __cdecl jqShutdown();
void __cdecl jqStart();


extern jqWorker *jqWorkers;
