#pragma once

#include <cstring>

// session key
struct XNKEY // sizeof=0x10
{
    unsigned __int8 ab[16];

    XNKEY()
    {
        memset(this->ab, 1u, sizeof(XNKEY));
    }
    XNKEY(const XNKEY *src)
    {
        memcpy(this->ab, src->ab, sizeof(XNKEY));
    }
};

// session id
struct XNKID // sizeof=0x8
{
    unsigned __int8 ab[8];

    XNKID()
    {
        memset(this->ab, 1u, sizeof(XNKID));
    }
    XNKID(const XNKID *src)
    {
        memcpy(this->ab, src->ab, sizeof(XNKID));
    }

    bool operator==(const XNKID &other) const
    {
        return memcmp(this->ab, other.ab, sizeof(XNKID)) == 0;
    }
    bool operator!=(const XNKID &other) const
    {
        return !(*this == other);
    }
};
