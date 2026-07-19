#pragma once

#include <list>
#include "glass_client.h"

struct SmallAllocator // sizeof=0x18
{                                       // XREF: GlassRenderer/r
    void *memory;
    void *freeHead;
    unsigned int blockSize;
    unsigned int numBlocks;
    unsigned int numUsed;
    unsigned int maxUsed;

    void __thiscall Init(void *buffer, unsigned int bs, unsigned int nb);
    void **__thiscall Allocate(unsigned int size);
    void __thiscall Free(void **ptr, unsigned int num);
    bool __thiscall IsValidPointer(void *ptr);
};

template <typename T>
struct SmallAllocatorTemplate//<GlassPhysics * *> // sizeof=0x4
{
    using value_type = T;

    SmallAllocator *alloc;

    SmallAllocatorTemplate(SmallAllocator *a) noexcept : alloc(a) {}

    template<class U>
    SmallAllocatorTemplate(const SmallAllocatorTemplate<U> &other) noexcept
        : alloc(other.alloc) {
    }

    T *allocate(std::size_t n)
    {
        return reinterpret_cast<T *>(alloc->Allocate(sizeof(T) * n));
    }

    void deallocate(T *p, std::size_t)
    {
        alloc->Free(reinterpret_cast<void **>(p), 1);
    }

    template<class U>
    struct rebind
    {
        using other = SmallAllocatorTemplate<U>;
    };

    // Two instances are interchangeable iff they wrap the same pool. std::list's
    // splice/move paths compare allocators (libstdc++'s _M_check_equal_allocators),
    // so these are required — MSVC's STL didn't demand them.
    template<class U>
    bool operator==(const SmallAllocatorTemplate<U> &o) const noexcept { return alloc == o.alloc; }
    template<class U>
    bool operator!=(const SmallAllocatorTemplate<U> &o) const noexcept { return alloc != o.alloc; }
};

// aislop
template <typename T>
struct FixedSizeAllocator
{
    //void *memory;

    std::list<T, SmallAllocatorTemplate<T>> used;
    std::list<T, SmallAllocatorTemplate<T>> free;

    unsigned int maxUsed;
    void *memory;
    // ...

    void *operator new(size_t size)
    {
        return GlassesClient::Allocate(size, "C:\\projects_pc\\cod\\codsrc\\src\\glass\\glass_client.cpp", 69);
    }
    void operator delete(void *ptr)
    {
        GlassesClient::Free((char *)ptr);
    }

    FixedSizeAllocator(T mem, unsigned int numObjects, SmallAllocator *allocator)
        : used(SmallAllocatorTemplate<T>(allocator))
        , free(SmallAllocatorTemplate<T>(allocator))
        , maxUsed(0)
        , memory(mem)
    {
        for (unsigned int i = 0; i < numObjects; ++i)
            free.push_front(&mem[i]);
    }

    T Allocate()
    {
        T ret = nullptr;

        if (!free.empty())
        {
            ret = free.front();
            free.erase(free.begin());
            used.push_front(ret);
        }

        if ((unsigned int)used.size() > maxUsed)
            maxUsed = (unsigned int)used.size();

        return ret;
    }

    void Free(T obj)
    {
        used.remove(obj);
        free.push_front(obj);
    }

    void FreeAll()
    {
        while (!used.empty())
            Free(used.front());
    }
};

struct Allocator // sizeof=0x14
{                                       // XREF: GlassRenderer/r
    struct Memory // sizeof=0x14
    {
        Allocator::Memory *next;
        Allocator::Memory *prev;
        void *userData;
        Allocator::Memory *nextFree;
        Allocator::Memory *prevFree;

        void __thiscall Init();
        char __thiscall MakeFree();
    };
    void *buffer;
    Allocator::Memory *head;
    Allocator::Memory *tail;
    Allocator::Memory *freeHead;
    int maxUsed;


    void __thiscall Init(void *buf, int size);
    void __thiscall FreeAll();
    Memory **__thiscall Allocate(int size, void *userData);
    void __thiscall Free(unsigned int *ptr);
    unsigned int __thiscall GetMemorySize(unsigned int *ptr);
    void __thiscall Split(Memory *mem, int size);
    void __thiscall Merge(Memory *mem1, Memory *mem2);
    int __thiscall Defrag(void(__cdecl *func)(void *), int count);
    int __thiscall GetFree();
    int __thiscall GetLargestFree();
};
