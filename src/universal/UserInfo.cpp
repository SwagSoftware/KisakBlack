#include "UserInfo.h"

PublicProfileInfo::PublicProfileInfo() : ProfileInfo()
{
}

unsigned int PublicProfileInfo::sizeOf()
{
    return 280;
}

PrivateProfileInfo::PrivateProfileInfo() : ProfileInfo()
{
}

unsigned int PrivateProfileInfo::sizeOf()
{
    return 608;
}
