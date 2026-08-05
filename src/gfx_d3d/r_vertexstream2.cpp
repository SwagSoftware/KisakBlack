#include "r_vertexstream2.h"
#include <qcommon/common.h>

#include <cstring>
#include "r_buffers.h"

IDirect3DVertexBuffer9 *stream2VertexBuffer;
unsigned int stream2AllocatedSize;

void __cdecl R_VertexStream2_Init(unsigned int stream2ByteSize)
{
    float *vbPtr; // [esp+0h] [ebp-8h]

    Com_Printf(8, "vertex stream 2: --------------------------\n");
    Com_Printf(8, "vertex stream 2: needs %u bytes\n", stream2ByteSize);
    if ( stream2AllocatedSize == stream2ByteSize )
    {
        Com_Printf(8, "vertex stream 2: already allocated %u bytes\n", stream2AllocatedSize);
    }
    else if ( stream2ByteSize )
    {
        if ( stream2AllocatedSize )
            R_VertexStream2_Shutdown();
        if ( AllocateVertexStream2Buffer(stream2ByteSize) )
        {
            Com_Printf(8, "vertex stream 2: vertex buffer allocate ok\n");
            vbPtr = (float *)R_LockVertexBuffer(stream2VertexBuffer, 0, 0, 0);
            if ( vbPtr )
            {
                memset(vbPtr, 0, stream2ByteSize);
                R_UnlockVertexBuffer(stream2VertexBuffer);
            }
            stream2AllocatedSize = stream2ByteSize;
        }
        else
        {
            Com_Printf(8, "vertex stream 2: vertex buffer allocate failed\n");
        }
    }
    Com_Printf(8, "vertex stream 2: --------------------------\n");
}

bool __cdecl AllocateVertexStream2Buffer(unsigned int stream2ByteSize)
{
    R_AllocDynamicVertexBuffer(&stream2VertexBuffer, stream2ByteSize);
    return stream2VertexBuffer != 0;
}

void __cdecl R_VertexStream2_Shutdown()
{
    if ( stream2AllocatedSize )
        FreeVertexStream2Buffer();
    stream2AllocatedSize = 0;
}

void FreeVertexStream2Buffer()
{
    R_FreeStaticVertexBuffer(stream2VertexBuffer);
}

unsigned __int8 *__cdecl R_VertexStream2_Lock(unsigned int *stream2LockedSize)
{
    unsigned __int8 *result; // eax

    if ( stream2AllocatedSize )
    {
        result = (unsigned __int8 *)R_LockVertexBuffer(stream2VertexBuffer, 0, 0, 0);
        *stream2LockedSize = stream2AllocatedSize;
    }
    else
    {
        *stream2LockedSize = 0;
        return 0;
    }
    return result;
}

void __cdecl R_VertexStream2_Unlock()
{
    if ( stream2AllocatedSize )
        R_UnlockVertexBuffer(stream2VertexBuffer);
}

void __cdecl R_VertexStream2_Clear()
{
    unsigned __int8 *stream2Ptr; // [esp+0h] [ebp-8h]
    unsigned int stream2Size; // [esp+4h] [ebp-4h] BYREF

    stream2Ptr = R_VertexStream2_Lock(&stream2Size);
    if ( stream2Ptr )
        memset(stream2Ptr, 0, stream2Size);
    R_VertexStream2_Unlock();
}

IDirect3DVertexBuffer9 **__cdecl R_VertexStream2_GetVB()
{
    return &stream2VertexBuffer;
}

