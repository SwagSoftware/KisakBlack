#pragma once

#include <demo/demo_common.h>

struct ProfileInfo : TaskResult // sizeof=0x10
{                                       // XREF: PrivateProfileInfo/r
                                        // PublicProfileInfo/r
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    unsigned __int64 m_entityID;        // XREF: PCache_GetPublicProfilesCompleted+27C/r
                                        // Live_SetPrivateProfile(void)+52/w ...
};

struct __declspec(align(4)) PublicProfileInfo : ProfileInfo // sizeof=0x118
{                                       // XREF: .data:s_userData/r
                                        // .data:s_taskData/r
    int m_memberVERSION;                // XREF: PCache_BatchUpdatePublicProfiles(int,PCachePublicProfile *,int)+239/w
    unsigned __int8 m_memberddl[257];   // XREF: PCache_BatchUpdatePublicProfiles(int,PCachePublicProfile *,int)+252/o
    // padding byte
    // padding byte
    // padding byte

    PublicProfileInfo();

    unsigned int sizeOf();
};


struct __declspec(align(4)) PrivateProfileInfo : ProfileInfo // sizeof=0x260
{                                       // XREF: .data:s_profileInfo/r
    unsigned __int8 m_memberfavsblob[253];
                                        // XREF: Live_GetPrivateProfileComplete(TaskRecord *)+8/o
                                        // CL_SetFavourites_f(void)+8/o ...
    unsigned __int8 m_memberuids[337];  // XREF: Live_GetPrivateProfileComplete(TaskRecord *)+3/o
                                        // CL_SetFavourites_f(void)+3/o ...
    // padding byte
    // padding byte

    PrivateProfileInfo();
    unsigned int sizeOf();
};



