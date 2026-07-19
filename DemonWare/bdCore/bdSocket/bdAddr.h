#pragma once
#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include <DemonWare//bdCore/bdContainers/bdArray.h>
#include <cstdint>
#include <DemonWare/bdCore/bdUtilities/bdBytePacker.h>
#include <winsock2.h>   // inet_addr, inet_ntoa
#include <DW/dwUtils.h>
#include <DW/dwNet.h>
#include <DemonWare/bdCore/bdCrypto/bdHashTiger192.h>
#include <DW/dwUtils.h>
#include <server_mp/sv_main_mp.h>


// Bro forward your ports!!! Your NAT is strict!! We can't even match!
enum bdNATType : __int32
{                                       // XREF: bdCommonAddr/r
    BD_NAT_OPEN     = 0x1,
    BD_NAT_MODERATE = 0x2,
    BD_NAT_STRICT   = 0x3,
};

struct bdInAddr
{
    union
    {
        struct
        {
            uint8_t m_b1;
            uint8_t m_b2;
            uint8_t m_b3;
            uint8_t m_b4;
        } m_caddr;

        uint32_t m_iaddr;
    } inUn;

public:

    /* =========================
       Constructors
       ========================= */

    bdInAddr()
    {
        // Decompiled default ctor:
        // this->inUn.m_iaddr = -16711936;
        // which is 0xFF00FF00
        inUn.m_iaddr = 0xFF00FF00;
    }

    explicit bdInAddr(const char *str)
    {
        fromString(str);
    }

    /* =========================
       Conversion
       ========================= */

    void fromString(const char *cp)
    {
        // Matches binary exactly
        inUn.m_iaddr = inet_addr(cp);
    }

    int toString(char *const dst, unsigned int size) const;

    /* =========================
       Operators
       ========================= */

    bool operator==(const bdInAddr &other) const
    {
        return inUn.m_iaddr == other.inUn.m_iaddr;
    }

    bool operator!=(const bdInAddr &other) const
    {
        return !(*this == other);
    }

    /* =========================
       Helpers
       ========================= */

    operator in_addr() const
    {
        in_addr addr;
        addr.s_addr = inUn.m_iaddr;
        return addr;
    }
};

struct bdInetAddr
{
    bdInAddr m_addr;

public:

    /* =========================
       Constructors
       ========================= */

    bdInetAddr()
    {
        // bdInAddr default ctor
    }

    explicit bdInetAddr(const bdInAddr &addr)
        : m_addr(addr)
    {
    }

    bdInetAddr(const bdInetAddr &other)
        : m_addr(other.m_addr)
    {
    }

    explicit bdInetAddr(const char *str)
    {
        set(str);
    }

    explicit bdInetAddr(unsigned int addr)
    {
        set(addr);
    }

    ~bdInetAddr()
    {
        // Matches binary behavior (debug pattern fill)
        m_addr.inUn.m_iaddr = 0xDEADBEEF;
    }

    /* =========================
       Static helpers
       ========================= */

    static bdInetAddr Any()
    {
        bdInetAddr result;
        result.set(0u);
        return result;
    }

    static bdInetAddr Broadcast()
    {
        bdInetAddr result;
        result.set(0xFFFFFFFFu);
        return result;
    }

    /* =========================
       Setters
       ========================= */

    void set(const bdInetAddr &other)
    {
        m_addr = other.m_addr;
    }

    void set(const char *str)
    {
        //bdInAddr::fromString(&m_addr, str);
        m_addr.fromString(str);
    }

    void set(unsigned int addr)
    {
        m_addr.inUn.m_iaddr = addr;
    }

    /* =========================
       Getters
       ========================= */

    bdInAddr getInAddr() const
    {
        return m_addr;
    }

    bool isValid() const
    {
        // Decompiled version compares against default-constructed address
        bdInetAddr empty;
        return !(*this == empty);
    }

    int toString(char *const dst, unsigned int capacity) const
    {
        if (isValid())
        {
            //return bdInAddr::toString(&m_addr, dst, capacity);
            m_addr.toString(dst, capacity);
        }

        return 0;
    }

    /* =========================
       Serialization
       ========================= */

    bool deserialize(
        unsigned char *src,
        unsigned int srcCapacity,
        unsigned int offset,
        unsigned int *newOffset)
    {
        return bdBytePacker::removeBuffer(
            src,
            srcCapacity,
            offset,
            newOffset,
            reinterpret_cast<unsigned char *>(this),
            4u
        );
    }

    bool serialize(
        unsigned char *dst,
        unsigned int dstCapacity,
        unsigned int offset,
        unsigned int *newOffset) const
    {
        return bdBytePacker::appendBuffer(
            dst,
            dstCapacity,
            offset,
            newOffset,
            (unsigned char *)this,
            4u
        );
    }

    /* =========================
       Operators
       ========================= */

    bool operator==(const bdInetAddr &other) const
    {
        return m_addr.inUn.m_iaddr == other.m_addr.inUn.m_iaddr;
    }

    bool operator!=(const bdInetAddr &other) const
    {
        return !(*this == other);
    }
};


#if 0
struct __declspec(align(4)) bdAddr
{
    bdInetAddr    m_address;
    uint16_t      m_port;

    uint16_t      _pad;

    inline bdAddr() = default;

    inline bdAddr(const bdAddr *other)
    {
        m_address = other->m_address;
        m_port = other->m_port;
    }
};
#endif

// aislopped addr classes
struct __declspec(align(4)) bdAddr
{
    bdInetAddr m_address;
    uint16_t   m_port;
    uint16_t   _pad;   // preserved for layout

    // ---------------------------------------------------------
    // Constructors
    // ---------------------------------------------------------

    inline bdAddr()
        : m_address(), m_port(0), _pad(0)
    {
    }

    inline bdAddr(const bdAddr *other)
        : m_address(other->m_address),
        m_port(other->m_port),
        _pad(0)
    {
    }

    inline bdAddr(const bdInetAddr *addr, uint16_t port)
        : m_address(*addr),
        m_port(port),
        _pad(0)
    {
    }

    inline bdAddr(char *string)
        : m_address(), m_port(0), _pad(0)
    {
        set(string);
    }

    // ---------------------------------------------------------
    // Destructor
    // ---------------------------------------------------------

    inline ~bdAddr()
    {
        // Preserved for parity with original
        m_address.~bdInetAddr();
    }

    // ---------------------------------------------------------
    // Accessors
    // ---------------------------------------------------------

    inline bdAddr *getAddress()
    {
        return this;
    }

    inline uint16_t getPort() const
    {
        return m_port;
    }

    inline void setPort(uint16_t port)
    {
        m_port = port;
    }

    // ---------------------------------------------------------
    // Setters
    // ---------------------------------------------------------

    inline void set(const bdInetAddr *addr, uint16_t port)
    {
        //bdInetAddr::set(&m_address, addr);
        m_address.set(*addr);
        m_port = port;
    }

    inline void set(char *string)
    {
        char *colon = strchr(string, ':');

        if (colon)
        {
            char ipBuffer[16] = {};
            size_t len = static_cast<size_t>(colon - string);

            if (len > 15)
                len = 15;

            memcpy(ipBuffer, string, len);
            ipBuffer[len] = '\0';

            bdInAddr in(ipBuffer);
            //bdInetAddr::set(&m_address, in.inUn.m_iaddr);
            m_address.set(in.inUn.m_iaddr);

            m_port = static_cast<uint16_t>(
                strtoul(colon + 1, nullptr, 10)
                );
        }
        else
        {
            bdInetAddr tmp(string);
            set(&tmp, 0);
        }
    }

    // ---------------------------------------------------------
    // Serialization
    // ---------------------------------------------------------

    inline bool serialize(
        unsigned char *dst,
        unsigned int capacity,
        unsigned int offset,
        unsigned int *newOffset)
    {
        *newOffset = offset;

        //if (!bdInetAddr::serialize(&m_address, dst, capacity, *newOffset, newOffset))
        if (!m_address.serialize(dst, capacity, *newOffset, newOffset))
        {
            *newOffset = offset;
            return false;
        }

        unsigned char portBytes[2];
        *reinterpret_cast<uint16_t *>(portBytes) = m_port;

        if (!bdBytePacker::appendBuffer(dst, capacity, *newOffset, newOffset, portBytes, 2))
        {
            *newOffset = offset;
            return false;
        }

        return true;
    }

    inline bool deserialize(
        unsigned char *src,
        unsigned int capacity,
        unsigned int offset,
        unsigned int *newOffset)
    {
        *newOffset = offset;

        //if (!bdInetAddr::deserialize(&m_address, src, capacity, *newOffset, newOffset))
        if (!m_address.deserialize(src, capacity, *newOffset, newOffset))
        {
            *newOffset = offset;
            return false;
        }

        unsigned char portBytes[2];

        if (!bdBytePacker::removeBuffer(src, capacity, *newOffset, newOffset, portBytes, 2))
        {
            *newOffset = offset;
            return false;
        }

        m_port = *reinterpret_cast<uint16_t *>(portBytes);
        return true;
    }

    // ---------------------------------------------------------
    // String Conversion
    // ---------------------------------------------------------

    unsigned int toString(char *const dst, unsigned int capacity);

    // ---------------------------------------------------------
    // Comparison
    // ---------------------------------------------------------

    inline bool operator==(const bdAddr &other) const
    {
        return (m_port == other.m_port) &&
            (m_address == other.m_address);
    }

    inline bool operator!=(const bdAddr &other) const
    {
        return !(*this == other);
    }
};


static_assert(sizeof(bdAddr) == 8, "bdAddr size mismatch");

//struct __declspec(align(4)) bdCommonAddr : bdReferencable // sizeof=0x28
//{
//    bdArray<bdAddr> m_localAddrs;
//    bdAddr m_publicAddr;
//    bdNATType m_natType;
//    unsigned int m_hash;
//    bool m_isLoopback;
//    // padding byte
//    // padding byte
//    // padding byte
//};

struct __declspec(align(4)) bdCommonAddr : bdReferencable
{
    bdArray<bdAddr> m_localAddrs;
    bdAddr          m_publicAddr;
    bdNATType       m_natType;
    unsigned int    m_hash;
    bool            m_isLoopback;

    // ---------------------------------------------------------
    // Constructors
    // ---------------------------------------------------------

    inline bdCommonAddr()
        : m_natType(BD_NAT_OPEN),
        m_hash(0),
        m_isLoopback(false)
    {
    }

    inline bdCommonAddr(bdAddr *addr)
        : m_natType(BD_NAT_OPEN),
        m_isLoopback(false)
    {
        m_localAddrs.pushBack(addr);
        m_publicAddr = *addr;
        calculateHash();
    }

    inline bdCommonAddr(const bdArray<bdAddr> &locals,
        const bdAddr &publicAddr,
        bdNATType natType)
        : m_localAddrs(locals),
        m_publicAddr(publicAddr),
        m_natType(natType),
        m_isLoopback(true)
    {
        calculateHash();
    }

    // ---------------------------------------------------------
    // Destructor
    // ---------------------------------------------------------

    inline ~bdCommonAddr()
    {
        m_localAddrs.clear();
    }

    // ---------------------------------------------------------
    // Hash
    // ---------------------------------------------------------

    void calculateHash();

    inline unsigned int getHash() const
    {
        return m_hash;
    }

    // ---------------------------------------------------------
    // Accessors
    // ---------------------------------------------------------

    inline const bdAddr *getLocalAddrByIndex(unsigned int i) const
    {
        return &m_localAddrs.m_data[i];
    }

    inline bdArray<bdAddr> &getLocalAddrs()
    {
        return m_localAddrs;
    }

    inline bool isLoopback() const
    {
        return m_isLoopback;
    }

    // ---------------------------------------------------------
    // Comparison
    // ---------------------------------------------------------

    inline bool operator==(const bdCommonAddr &other) const
    {
        if (m_hash != other.m_hash)
            return false;

        //bdAddr lhs = bdInetAddr::isValid(&m_publicAddr.m_address)
        bdAddr lhs = m_publicAddr.m_address.isValid()
            ? m_publicAddr
            : (m_localAddrs.m_size ? m_localAddrs.m_data[0] : bdAddr());

        //bdAddr rhs = bdInetAddr::isValid(&other.m_publicAddr.m_address)
        bdAddr rhs = other.m_publicAddr.m_address.isValid()
            ? other.m_publicAddr
            : (other.m_localAddrs.m_size ? other.m_localAddrs.m_data[0] : bdAddr());

        return lhs == rhs;
    }

    inline bool operator!=(const bdCommonAddr &other) const
    {
        return !(*this == other);
    }

    // ---------------------------------------------------------
    // Serialization
    // ---------------------------------------------------------

    inline void serialize(unsigned char *const dst)
    {
        unsigned int offset = 0;
        const unsigned int maxSize = 25;

        bdAddr empty;

        for (unsigned int i = 0; i < 3; ++i)
        {
            if (i < m_localAddrs.m_size)
            {
                //bdAddr::serialize(&m_localAddrs.m_data[i], dst, maxSize, offset, &offset);
                m_localAddrs.m_data[i].serialize(dst, maxSize, offset, &offset);
            }
            else
            {
                //bdAddr::serialize(&empty, dst, maxSize, offset, &offset);
                empty.serialize(dst, maxSize, offset, &offset);
            }
        }

        //bdAddr::serialize(&m_publicAddr, dst, maxSize, offset, &offset);
        m_publicAddr.serialize(dst, maxSize, offset, &offset);

        unsigned char nat = static_cast<unsigned char>(m_natType);
        bdBytePacker::appendBuffer(dst, maxSize, offset, &offset, &nat, 1u);
    }

    inline bool deserialize(bdReference<bdCommonAddr> ref, unsigned char *src)
    {
        unsigned int offset = 0;
        const unsigned int maxSize = 25;

        m_localAddrs.clear();

        for (unsigned int i = 0; i < 3; ++i)
        {
            bdAddr addr;

            //if (!bdAddr::deserialize(&addr, src, maxSize, offset, &offset))
            if (!addr.serialize(src, maxSize, offset, &offset))
                return false;

            //if (bdInetAddr::isValid(&addr.m_address))
            if (addr.m_address.isValid())
                m_localAddrs.pushBack(&addr);
        }

        //if (!bdAddr::deserialize(&m_publicAddr, src, maxSize, offset, &offset))
        if (!m_publicAddr.deserialize(src, maxSize, offset, &offset))
            return false;

        unsigned char nat;
        if (!bdBytePacker::removeBuffer(src, maxSize, offset, &offset, &nat, 1u))
            return false;

        m_natType = static_cast<bdNATType>(nat);

        calculateHash();

        m_isLoopback = (ref.m_ptr && (*ref.m_ptr == *this));

        return true;
    }
};

struct bdSecurityKey // sizeof=0x10
{                                                                             // XREF: .data:g_secKey/r
    unsigned __int8 ab[16];                         // XREF: dwCreateSession(overlappedTask * const,MatchMakingInfo * const)+139/r

    bdSecurityKey(const struct bdSecurityKey *src)
    {
        memcpy(this->ab, src->ab, sizeof(bdSecurityKey));
    }
    bdSecurityKey()
    {
        memset(this->ab, 1u, sizeof(bdSecurityKey));
    }
};

struct bdSecurityID // sizeof=0x8
{                                                                             // XREF: bdQoSProbe::bdQoSProbeEntryWrapper/r
    unsigned __int8 ab[8];                            // XREF: PM_Weapon_FireWeapon+17D/o

    bdSecurityID(const struct bdSecurityID *src)
    {
        memcpy(this->ab, src->ab, sizeof(bdSecurityID));
    }

    bdSecurityID()
    {
        memset(this->ab, 1u, sizeof(bdSecurityID));
    }

    bool operator==(const bdSecurityID &other) const
    {
        return memcmp(this->ab, other.ab, sizeof(bdSecurityID)) == 0;
    }

    bool operator!=(const bdSecurityID &other) const
    {
        return !(*this == other);
    }
};

struct bdEndpoint // sizeof=0xC
{                                       // XREF: bdAddrHandle/r
                                        // ?pump@bdIPDiscoveryClient@@QAEXVbdAddr@@QAEH@Z/r
    bdReference<bdCommonAddr> m_ca;
    bdSecurityID m_secID;               // XREF: bdIPDiscoveryClient::pump(bdAddr,uchar * const,int)+10D/o
};

struct bdAddrHandle : bdReferencable // sizeof=0x20
{                                       // XREF: ?acceptPacket@bdQoSProbe@@UAE_NPAVbdSocket@@VbdAddr@@PAXIE@Z/r
    enum bdAddrHandleStatus : __int32
    {                                       // XREF: bdAddrHandle/r
                                            // ?pump@bdSocketRouter@@UAEXXZ/r ...
        BD_ADDR_NOT_CONSTRUCTED = 0x0,
        BD_ADDR_NOT_RESOLVED    = 0x1,
        BD_ADDR_ERROR           = 0x2,
        BD_ADDR_RESOLVED        = 0x3,
        BD_ADDR_UNRESOLVED      = 0x4,
    };                   // ?receive@bdUnicastConnection@@MAE_NPBEI@Z/r
    bdEndpoint m_endpoint;
    bdAddrHandle::bdAddrHandleStatus m_status;
    bdAddr m_realAddr;                  // XREF: bdQoSProbe::acceptPacket(bdSocket *,bdAddr,void *,uint,uchar)+D7/o
                                        // bdQoSProbe::acceptPacket(bdSocket *,bdAddr,void *,uint,uchar)+F4/r
};