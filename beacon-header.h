#ifndef BEACON_HEADER_H
#define BEACON_HEADER_H

#include "ns3/header.h"

namespace ns3 {
namespace lorawan {

class BeaconHeader : public Header
{
public:
    BeaconHeader ();
    virtual ~BeaconHeader ();

    void SetTime (double time);
    double GetTime () const;

    static TypeId GetTypeId (void);
    virtual TypeId GetInstanceTypeId (void) const;
    virtual void Print (std::ostream &os) const;
    virtual uint32_t GetSerializedSize (void) const;
    virtual void Serialize (Buffer::Iterator start) const;
    virtual uint32_t Deserialize (Buffer::Iterator start);

private:
    double m_time;
};

} // namespace lorawan
} // namespace ns3

#endif // BEACON_HEADER_H
