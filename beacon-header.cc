#include "beacon-header.h"
#include "ns3/buffer.h"
#include "ns3/nstime.h"



namespace ns3 {
namespace lorawan {

BeaconHeader::BeaconHeader () : m_time (0) {}
BeaconHeader::~BeaconHeader () {}

void BeaconHeader::SetTime (double time)
{
    m_time = time;
}

double BeaconHeader::GetTime () const
{
    return m_time;
}

TypeId
BeaconHeader::GetTypeId (void)
{
    static TypeId tid = TypeId ("ns3::lorawan::BeaconHeader")
        .SetParent<Header> ()
        .AddConstructor<BeaconHeader> ();
    return tid;
}

TypeId
BeaconHeader::GetInstanceTypeId (void) const
{
    return GetTypeId ();
}

void
BeaconHeader::Print (std::ostream &os) const
{
    os << "Time=" << m_time;
}

uint32_t
BeaconHeader::GetSerializedSize (void) const
{
    return sizeof(double);
}

void
BeaconHeader::Serialize (Buffer::Iterator start) const
{
    start.WriteHtonU64 (static_cast<uint64_t>(m_time * 1000)); // Convert seconds to milliseconds
}

uint32_t
BeaconHeader::Deserialize (Buffer::Iterator start)
{
    m_time = start.ReadNtohU64 () / 1000.0; // Convert milliseconds back to seconds
    return GetSerializedSize ();
}

} // namespace lorawan
} // namespace ns3
