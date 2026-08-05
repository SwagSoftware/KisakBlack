#include "enthandle.h"

#include <string.h>
#include <qcommon/common.h>
#include <game_mp/g_main_mp.h>

EntHandleInfo g_entHandleInfoArray[1024];
EntHandleList g_entitiesHandleList[1024];
unsigned int g_entHandleInfoHead;
unsigned int g_usedEntHandle;
unsigned int g_maxUsedEntHandle;

EntHandleList g_sentientsHandleList[48];

void __cdecl EntHandle::Init()
{
    unsigned int i; // [esp+4h] [ebp-4h]

    for ( i = 1; i < 0x400; ++i )
    {
        g_entHandleInfoArray[i].next = i + 1;
        g_entHandleInfoArray[i].handle = 0;
    }
    g_entHandleInfoArray[1023].next = 0;
    g_entHandleInfoHead = 1;
    memset(g_entitiesHandleList, 0, sizeof(g_entitiesHandleList));
    g_usedEntHandle = 0;
}

void __cdecl EntHandle::Shutdown()
{
    const char *v0; // eax
    unsigned int usedEntHandle; // [esp+0h] [ebp-Ch]
    unsigned int i; // [esp+8h] [ebp-4h]

    if ( g_usedEntHandle )
    {
        Com_Printf(1, "EntHandle BEGIN\n");
        usedEntHandle = 0;
        for ( i = 1; i < 0x400; ++i )
        {
            if ( g_entHandleInfoArray[i].handle )
            {
                Com_Printf(1, "%p\n", &g_entHandleInfoArray[i]);
                ++usedEntHandle;
            }
        }
        Com_Printf(1, "EntHandle END\n");
        if ( usedEntHandle != g_usedEntHandle )
        {
            v0 = va("%d %d", usedEntHandle, g_usedEntHandle);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\enthandle.cpp",
                            105,
                            0,
                            "%s\n\t%s",
                            "usedEntHandle == g_usedEntHandle",
                            v0) )
                __debugbreak();
        }
        if ( g_usedEntHandle
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\enthandle.cpp",
                        106,
                        0,
                        "%s\n\t(g_usedEntHandle) = %i",
                        "(!g_usedEntHandle)",
                        g_usedEntHandle) )
        {
            __debugbreak();
        }
    }
}

void __cdecl SentientHandle::Init()
{
    memset(g_sentientsHandleList, 0, sizeof(g_sentientsHandleList));
}

void __cdecl EntHandleDissociate(gentity_s *ent)
{
    //EntHandleDissociateInternal((EntHandleList *)(2 * (ent - g_entities) + 63686360));
    EntHandleDissociateInternal(&g_entitiesHandleList[ent - g_entities]);
}

void __cdecl EntHandleDissociateInternal(EntHandleList *entHandleList)
{
    char *v2; // eax
    EntHandleInfo *info; // [esp+0h] [ebp-Ch]
    unsigned int infoIndexHead; // [esp+4h] [ebp-8h]
    unsigned int infoIndex; // [esp+8h] [ebp-4h]

    infoIndexHead = entHandleList->infoIndex;
    if (entHandleList->infoIndex)
    {
        entHandleList->infoIndex = 0;
        infoIndex = infoIndexHead;
        do
        {
            info = &g_entHandleInfoArray[infoIndex];
            if (!info->handle)
            {
                v2 = va("%d %p", infoIndex, info);
                if (!Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\enthandle.cpp",
                    139,
                    0,
                    "%s\n\t%s",
                    "info->handle",
                    v2))
                    __debugbreak();
            }
            *(_DWORD *)info->handle = 0;
            info->handle = 0;
            if (!g_usedEntHandle
                && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\enthandle.cpp", 144, 0, "%s", "g_usedEntHandle"))
            {
                __debugbreak();
            }
            --g_usedEntHandle;
            infoIndex = info->next;
        } while (infoIndex != infoIndexHead);
        g_entHandleInfoArray[g_entHandleInfoArray[infoIndexHead].prev].next = g_entHandleInfoHead;
        g_entHandleInfoHead = infoIndexHead;
    }
}

void __cdecl SentientHandleDissociate(sentient_s *sentient)
{
    EntHandleDissociateInternal(&g_sentientsHandleList[sentient - g_sentients]);
}

void __thiscall EntHandle::setEnt(gentity_s *ent)
{
    gentity_s *oldEnt; // [esp+4h] [ebp-8h]

    if ( EntHandle::isDefined() )
    {
        oldEnt = EntHandle::ent();
        if ( ent == oldEnt )
            return;
        RemoveEntHandleInfo(&g_entitiesHandleList[oldEnt - g_entities], this->infoIndex);
        if ( !ent )
        {
            this->number = 0;
            this->infoIndex = 0;
            return;
        }
    }
    else if ( !ent )
    {
        return;
    }
    this->infoIndex = AddEntHandleInfo(&g_entitiesHandleList[ent - g_entities], this);
    this->number = ent - g_entities + 1;
}

bool __thiscall EntHandle::isDefined() const
{
    if ( this->number
        && !g_entities[this->number - 1].r.inuse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
                    389,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(!number || g_entities[number - 1].r.inuse)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    return this->number != 0;
}

gentity_s *__thiscall EntHandle::ent()
{
    if ( (unsigned int)this->number - 1 >= 0x3FF
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
                    396,
                    0,
                    "number - 1 doesn't index ENTITYNUM_NONE\n\t%i not in [0, %i)",
                    this->number - 1,
                    1023) )
    {
        __debugbreak();
    }
    if ( !g_entities[this->number - 1].r.inuse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
                    397,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(g_entities[number - 1].r.inuse)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    return &g_entities[this->number - 1];
}

const gentity_s *__thiscall EntHandle::ent() const
{
    if ((unsigned int)this->number - 1 >= 0x3FF
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
            396,
            0,
            "number - 1 doesn't index ENTITYNUM_NONE\n\t%i not in [0, %i)",
            this->number - 1,
            1023))
    {
        __debugbreak();
    }
    if (!g_entities[this->number - 1].r.inuse
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\game\\../game_mp/g_public_mp.h",
            397,
            0,
            "%s\n\t(number - 1) = %i",
            "(g_entities[number - 1].r.inuse)",
            this->number - 1))
    {
        __debugbreak();
    }
    return &g_entities[this->number - 1];
}

void __cdecl RemoveEntHandleInfo(EntHandleList *entHandleList, unsigned int oldInfoIndex)
{
    char *v3; // eax
    EntHandleInfo *info; // [esp+0h] [ebp-10h]
    EntHandleInfo *prev; // [esp+Ch] [ebp-4h]

    if (oldInfoIndex)
    {
        if (entHandleList->infoIndex == oldInfoIndex)
            entHandleList->infoIndex = oldInfoIndex != g_entHandleInfoArray[oldInfoIndex].next
            ? g_entHandleInfoArray[oldInfoIndex].next
            : 0;
        info = &g_entHandleInfoArray[oldInfoIndex];
        if (!info->handle)
        {
            v3 = va("%d %p", oldInfoIndex, info);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\game\\enthandle.cpp",
                196,
                0,
                "%s\n\t%s",
                "info->handle",
                v3))
                __debugbreak();
        }
        info->handle = 0;
        if (!g_usedEntHandle
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\enthandle.cpp", 198, 0, "%s", "g_usedEntHandle"))
        {
            __debugbreak();
        }
        --g_usedEntHandle;
        prev = &g_entHandleInfoArray[info->prev];
        g_entHandleInfoArray[info->next].prev = info->prev;
        prev->next = info->next;
        info->next = g_entHandleInfoHead;
        g_entHandleInfoHead = oldInfoIndex;
    }
}

unsigned int __cdecl AddEntHandleInfo(EntHandleList *entHandleList, void *handle)
{
    unsigned int v4; // [esp+0h] [ebp-14h]
    unsigned int infoIndexHead; // [esp+8h] [ebp-Ch]
    unsigned int infoIndex; // [esp+Ch] [ebp-8h]
    EntHandleInfo *infoHead; // [esp+10h] [ebp-4h]

    infoIndex = g_entHandleInfoHead;
    if (!g_entHandleInfoHead)
        Com_Error(ERR_DROP, "ENT_HANDLE_COUNT exceeded - increase size");
    g_entHandleInfoHead = g_entHandleInfoArray[infoIndex].next;
    if ((int)g_maxUsedEntHandle < (int)++g_usedEntHandle)
        v4 = g_usedEntHandle;
    else
        v4 = g_maxUsedEntHandle;
    g_maxUsedEntHandle = v4;
    g_entHandleInfoArray[infoIndex].handle = handle;
    infoIndexHead = entHandleList->infoIndex;
    if (entHandleList->infoIndex)
    {
        infoHead = &g_entHandleInfoArray[infoIndexHead];
        g_entHandleInfoArray[infoIndex].next = infoHead->next;
        g_entHandleInfoArray[infoIndex].prev = infoIndexHead;
        g_entHandleInfoArray[infoHead->next].prev = infoIndex;
        infoHead->next = infoIndex;
    }
    else
    {
        g_entHandleInfoArray[infoIndex].next = infoIndex;
        g_entHandleInfoArray[infoIndex].prev = infoIndex;
        entHandleList->infoIndex = infoIndex;
    }
    return infoIndex;
}

void __thiscall SentientHandle::setSentient(sentient_s *sentient)
{
    sentient_s *oldSentient; // [esp+4h] [ebp-8h]

    if (SentientHandle::isDefined())
    {
        oldSentient = this->sentient();
        if (sentient == oldSentient)
            return;
        RemoveEntHandleInfo(&g_sentientsHandleList[oldSentient - g_sentients], this->infoIndex);
        if (!sentient)
        {
            this->number = 0;
            this->infoIndex = 0;
            return;
        }
    }
    else if (!sentient)
    {
        return;
    }
    this->infoIndex = AddEntHandleInfo(&g_sentientsHandleList[sentient - g_sentients], this);
    this->number = sentient - g_sentients + 1;
}

bool __thiscall SentientHandle::isDefined() const
{
    if ( this->number
        && !g_sentients[this->number - 1].ent
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
                    98,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(!number || g_sentients[number - 1].ent)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    if ( this->number
        && !g_sentients[this->number - 1].ent->r.inuse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
                    99,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(!number || g_sentients[number - 1].ent->r.inuse)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    return this->number != 0;
}

sentient_s *__thiscall SentientHandle::sentient() 
{
    if ( (unsigned int)this->number - 1 >= 0x30
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
                    106,
                    0,
                    "number - 1 doesn't index MAX_SENTIENTS\n\t%i not in [0, %i)",
                    this->number - 1,
                    48) )
    {
        __debugbreak();
    }
    if ( !g_sentients[this->number - 1].ent
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
                    107,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(g_sentients[number - 1].ent)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    if ( !g_sentients[this->number - 1].ent->r.inuse
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
                    108,
                    0,
                    "%s\n\t(number - 1) = %i",
                    "(g_sentients[number - 1].ent->r.inuse)",
                    this->number - 1) )
    {
        __debugbreak();
    }
    return &g_sentients[this->number - 1];
}

const sentient_s *__thiscall SentientHandle::sentient() const
{
    if ((unsigned int)this->number - 1 >= 0x30
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
            106,
            0,
            "number - 1 doesn't index MAX_SENTIENTS\n\t%i not in [0, %i)",
            this->number - 1,
            48))
    {
        __debugbreak();
    }
    if (!g_sentients[this->number - 1].ent
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
            107,
            0,
            "%s\n\t(number - 1) = %i",
            "(g_sentients[number - 1].ent)",
            this->number - 1))
    {
        __debugbreak();
    }
    if (!g_sentients[this->number - 1].ent->r.inuse
        && !Assert_MyHandler(
            "c:\\projects_pc\\cod\\codsrc\\src\\game\\sentient.h",
            108,
            0,
            "%s\n\t(number - 1) = %i",
            "(g_sentients[number - 1].ent->r.inuse)",
            this->number - 1))
    {
        __debugbreak();
    }
    return &g_sentients[this->number - 1];
}
