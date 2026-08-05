#include "bg_weapons_util.h"
#include "bg_weapons_load_obj.h"
#include <qcommon/common.h>
#include <cstring>

#include <algorithm>

// (aislop used to decipher std:: aids)
struct compare_weapon_component_sort
{
    bool operator()(const WeaponComponent &a, const WeaponComponent &b) const
    {
        // Primary sort: type (descending)
        if (a.type != b.type)
            return a.type > b.type;

        // Secondary sort: attachment point (only for attachments)
        if (a.type == WEAPON_COMPONENT_ATTACHMENT)
            return a.attachmentPoint < b.attachmentPoint;

        return false;
    }
};

void __cdecl BG_WeaponComponentListSort(const char *originalName, WeaponComponentList *componentList, char *outputName)
{
    //std::_Sort<WeaponComponent *,int,compare_weapon_component_sort>(
    //    componentList->components,
    //    &componentList->components[componentList->numComponents],
    //    44 * componentList->numComponents / 44,
    //    0);
    std::sort(
        &componentList->components[0],
        &componentList->components[componentList->numComponents],
        compare_weapon_component_sort());
    BG_WeaponComponentListToName(componentList, outputName, 64);
    if ( I_strcmp(originalName, outputName) )
        Com_PrintWarning(17, "Weapon %s attachments in the wrong order in name. Changing to %s\n", originalName, outputName);
}

void __cdecl BG_WeaponNameToComponentList(const char *name, WeaponComponentList *componentList)
{
    int charIdx; // [esp+0h] [ebp-8h]

    memset(componentList, 0, sizeof(WeaponComponentList));
    charIdx = 0;
    while ( *name )
    {
        if ( *name == 95 )
        {
            componentList->components[componentList->numComponents++].name[charIdx] = 0;
            charIdx = 0;
            if ( componentList->numComponents >= 7
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_util.cpp",
                            59,
                            0,
                            "%s",
                            "componentList->numComponents < MAX_WEAPON_COMPONENTS") )
            {
                __debugbreak();
            }
        }
        else
        {
            componentList->components[componentList->numComponents].name[charIdx] = *name;
            if ( ++charIdx >= 32
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_util.cpp",
                            65,
                            0,
                            "%s",
                            "charIdx < MAX_WEAPON_COMPONENT_LEN") )
            {
                __debugbreak();
            }
        }
        ++name;
    }
    componentList->components[componentList->numComponents++].name[charIdx] = 0;
    if ( componentList->numComponents > 7
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\bgame\\bg_weapons_util.cpp",
                    73,
                    0,
                    "%s",
                    "componentList->numComponents <= MAX_WEAPON_COMPONENTS") )
    {
        __debugbreak();
    }
}

void __cdecl BG_WeaponComponentListIdentify(WeaponComponentList *componentList)
{
    eAttachment attachment; // [esp+0h] [ebp-Ch]
    int compIdx; // [esp+4h] [ebp-8h]

    for ( compIdx = 0; compIdx < componentList->numComponents; ++compIdx )
    {
        if ( compIdx )
        {
            attachment = (eAttachment)BG_GetAttachmentIndex(componentList->components[compIdx].name);
            if ( attachment && attachment != ATTACHMENT_COUNT )
            {
                componentList->components[compIdx].type = WEAPON_COMPONENT_ATTACHMENT;
                componentList->components[compIdx].attachment = attachment;
                componentList->components[compIdx].attachmentPoint = BG_GetAttachmentPointIndexFromAttachment(attachment);
            }
            else
            {
                componentList->components[compIdx].type = WEAPON_COMPONENT_UNKNOWN;
            }
        }
        else
        {
            componentList->components[0].type = WEAPON_COMPONENT_BASE;
        }
    }
}

int __cdecl BG_WeaponComponentListCountAttachments(const WeaponComponentList *componentList)
{
    const AttachmentTableEntry *entry; // [esp+0h] [ebp-10h]
    signed int attachIdx; // [esp+4h] [ebp-Ch]
    int compIdx; // [esp+8h] [ebp-8h]
    int count; // [esp+Ch] [ebp-4h]

    count = 0;
    for ( compIdx = 0; compIdx < componentList->numComponents; ++compIdx )
    {
        for ( attachIdx = 0; attachIdx < BG_GetWeaponAttachmentCount(); ++attachIdx )
        {
            entry = BG_GetAttachmentTableEntry(attachIdx);
            if ( !I_stricmp(entry->values.referenceName, componentList->components[compIdx].name) )
            {
                ++count;
                break;
            }
        }
    }
    return count;
}

void __cdecl BG_WeaponComponentListRemove(
                const WeaponComponentList *componentListIn,
                WeaponComponentList *componentListOut,
                bool lookingForAttachements)
{
    char v3; // [esp+3h] [ebp-21h]
    WeaponComponent *v4; // [esp+8h] [ebp-1Ch]
    const WeaponComponent *v5; // [esp+Ch] [ebp-18h]
    const AttachmentTableEntry *entry; // [esp+10h] [ebp-14h]
    signed int attachIdx; // [esp+14h] [ebp-10h]
    int compIdx; // [esp+18h] [ebp-Ch]
    bool found; // [esp+23h] [ebp-1h]

    memset(componentListOut, 0, sizeof(WeaponComponentList));
    for ( compIdx = 0; compIdx < componentListIn->numComponents; ++compIdx )
    {
        found = 0;
        for ( attachIdx = 0; attachIdx < BG_GetWeaponAttachmentCount(); ++attachIdx )
        {
            entry = BG_GetAttachmentTableEntry(attachIdx);
            if ( !I_stricmp(entry->values.referenceName, componentListIn->components[compIdx].name) )
            {
                found = 1;
                break;
            }
        }
        if ( found == lookingForAttachements )
        {
            v5 = &componentListIn->components[compIdx];
            v4 = &componentListOut->components[componentListOut->numComponents];
            do
            {
                v3 = v5->name[0];
                v4->name[0] = v5->name[0];
                v5 = (WeaponComponent *)((char *)v5 + 1);
                v4 = (WeaponComponent *)((char *)v4 + 1);
            }
            while ( v3 );
            ++componentListOut->numComponents;
        }
    }
}

void __cdecl BG_WeaponComponentListRemoveAllAttachments(
                const WeaponComponentList *componentListIn,
                WeaponComponentList *componentListOut)
{
    BG_WeaponComponentListRemove(componentListIn, componentListOut, 0);
}

void __cdecl BG_WeaponComponentListNthAttachment(
                const WeaponComponentList *componentListIn,
                int nth,
                WeaponComponentList *componentListOut)
{
    const AttachmentTableEntry *AttachmentTableEntry; // eax
    char v4; // [esp+3h] [ebp-1Dh]
    WeaponComponent *v5; // [esp+8h] [ebp-18h]
    const WeaponComponent *v6; // [esp+Ch] [ebp-14h]
    signed int attachIdx; // [esp+10h] [ebp-10h]
    int compIdx; // [esp+14h] [ebp-Ch]
    int count; // [esp+18h] [ebp-8h]
    bool found; // [esp+1Fh] [ebp-1h]

    count = 0;
    memset(componentListOut, 0, sizeof(WeaponComponentList));
    for ( compIdx = 0; compIdx < componentListIn->numComponents; ++compIdx )
    {
        found = 0;
        for ( attachIdx = 0; attachIdx < BG_GetWeaponAttachmentCount(); ++attachIdx )
        {
            AttachmentTableEntry = BG_GetAttachmentTableEntry(attachIdx);
            if ( !I_stricmp(AttachmentTableEntry->values.referenceName, componentListIn->components[compIdx].name) )
            {
                found = 1;
                ++count;
                break;
            }
        }
        if ( !found || count == nth )
        {
            v6 = &componentListIn->components[compIdx];
            v5 = &componentListOut->components[componentListOut->numComponents];
            do
            {
                v4 = v6->name[0];
                v5->name[0] = v6->name[0];
                v6 = (WeaponComponent *)((char *)v6 + 1);
                v5 = (WeaponComponent *)((char *)v5 + 1);
            }
            while ( v4 );
            ++componentListOut->numComponents;
        }
    }
}

void __cdecl BG_WeaponComponentListToName(const WeaponComponentList *componentList, char *name, int size)
{
    int compIdx; // [esp+0h] [ebp-4h]

    *name = 0;
    if ( componentList->numComponents )
    {
        I_strncat(name, size, componentList->components[0].name);
        for ( compIdx = 1; compIdx < componentList->numComponents; ++compIdx )
        {
            I_strncat(name, size, "_");
            I_strncat(name, size, componentList->components[compIdx].name);
        }
    }
}

