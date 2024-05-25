#include "ns3/building-allocator.h"
#include "ns3/building-penetration-loss.h"
#include "ns3/buildings-helper.h"
#include "ns3/class-a-end-device-lorawan-mac.h"
#include "ns3/command-line.h"
#include "ns3/constant-position-mobility-model.h"
#include "ns3/correlated-shadowing-propagation-loss-model.h"
#include "ns3/double.h"
#include "ns3/end-device-lora-phy.h"
#include "ns3/forwarder-helper.h"
#include "ns3/gateway-lora-phy.h"
#include "ns3/gateway-lorawan-mac.h"
#include "ns3/log.h"
#include "ns3/lora-helper.h"
#include "ns3/mobility-helper.h"
#include "ns3/network-server-helper.h"
#include "ns3/node-container.h"
#include "ns3/periodic-sender-helper.h"
#include "ns3/pointer.h"
#include "ns3/position-allocator.h"
#include "ns3/random-variable-stream.h"
#include "ns3/simulator.h"
#include "ns3/socket.h" // Include the Socket header
#include "ns3/ipv4-address.h" // Include the IPv4Address header


#include <algorithm>
#include <ctime>

#define BEACON_DATA_RATE 3 // Example data rate for beacon
#define BEACON_FREQUENCY 923.3 // Example frequency for beacon (in MHz)
#define BEACON_PREAMBLE_LENGTH 10 // Example preamble length for beacon

using namespace ns3;
using namespace lorawan;

// Define variables for throughput and packet statistics tracking
uint64_t totalBitsTransmitted = 0;
uint32_t totalPacketsTransmitted = 0;
uint32_t totalPacketsReceived = 0;
uint32_t totalPacketsDropped = 0;
Time lastStatsUpdateTime = Seconds(0);

void ReceivePacket (Ptr<Socket> socket)
{
  while (socket->Recv ())
  {
    NS_LOG_UNCOND ("Received one packet!");
    totalPacketsReceived++;
  }
}

void FailedReception(Ptr<const Packet> packet)
{
  NS_LOG_UNCOND ("Failed to receive packet!");
  totalPacketsDropped++;
}

void SendBeacon (Ptr<Node> gateway, Ptr<GatewayLorawanMac> gatewayMac, Ptr<LoraPhy> gatewayPhy)
{
  NS_LOG_UNCOND ("Sending beacon");

  Ptr<Packet> beaconPacket = Create<Packet> ();

  LorawanMacHeader beaconHeader;
   beaconHeader.SetMType (LorawanMacHeader::BEACON); // Use SetMType instead of SetType
  beaconPacket->AddHeader (beaconHeader);

  LoraTag tag;
  tag.SetDataRate (BEACON_DATA_RATE);
  tag.SetFrequency (BEACON_FREQUENCY);
  beaconPacket->AddPacketTag (tag);

  // Use the gateway MAC to send the beacon
  gatewayMac->Send (beaconPacket);

  // Update statistics for beacon packet
  totalBitsTransmitted += beaconPacket->GetSize() * 8;
  totalPacketsTransmitted++;

  Simulator::Schedule (Seconds (128), &SendBeacon, gateway, gatewayMac, gatewayPhy);
}

void UpdateStatistics()
{
  Time now = Simulator::Now();

  // Calculate throughput
  double elapsedTime = (now - lastStatsUpdateTime).GetSeconds();
  double throughput = totalBitsTransmitted / elapsedTime;

  // Output statistics
  NS_LOG_UNCOND("Throughput: " << throughput << " bps");
  NS_LOG_UNCOND("Total packets transmitted: " << totalPacketsTransmitted);
  NS_LOG_UNCOND("Total packets received: " << totalPacketsReceived);
  NS_LOG_UNCOND("Total packets dropped: " << totalPacketsDropped);

  // Reset counters
  totalBitsTransmitted = 0;
  totalPacketsTransmitted = 0;
  totalPacketsReceived = 0;
  totalPacketsDropped = 0;
  lastStatsUpdateTime = now;

  // Schedule the next statistics update
  Simulator::Schedule(Seconds(1), &UpdateStatistics);
}

int main (int argc, char *argv[])
{
  // Set up logging
  LogComponentEnable ("LoraChannel", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraPhy", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraInterferenceHelper", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraTxCurrent", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraRxCurrent", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraFrameHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("LoraMacHeader", LOG_LEVEL_ALL);
  LogComponentEnable ("GatewayLorawanMac", LOG_LEVEL_ALL);
  LogComponentEnable ("ClassAEndDeviceLorawanMac", LOG_LEVEL_ALL);

  // Create the helper
  LoraHelper helper;

  // Create a channel
  Ptr<LoraChannel> channel = CreateObject<LoraChannel> ();

  // Create a single gateway node
  NodeContainer gateways;
  gateways.Create (1);
  Ptr<Node> gateway = gateways.Get (0);

  // Create end device nodes
  NodeContainer endDevices;
  endDevices.Create (1);
  Ptr<Node> endDevice = endDevices.Get (0);

  // Create mobility models
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  mobility.Install (gateways);
  mobility.Install (endDevices);

  // Create the LoraPhyHelper
  LoraPhyHelper phyHelper = LoraPhyHelper ();
  phyHelper.SetChannel (channel);

  // Create the LoraMacHelper
  LoraMacHelper macHelper = LoraMacHelper ();

  // Create the LoraNetDeviceHelper
  LoraNetDeviceHelper netDeviceHelper = LoraNetDeviceHelper ();
  netDeviceHelper.SetPhyHelper (phyHelper);
  netDeviceHelper.SetMacHelper (macHelper);

  // Install the net devices on the gateway
  NetDeviceContainer gatewayDevices = netDeviceHelper.Install (gateways);
  Ptr<NetDevice> gatewayDevice = gatewayDevices.Get (0);
  Ptr<GatewayLorawanMac> gatewayMac = gatewayDevice->GetObject<GatewayLorawanMac> ();
  Ptr<LoraPhy> gatewayPhy = gatewayDevice->GetObject<LoraNetDevice> ()->GetPhy ();

  // Install the net devices on the end device
  NetDeviceContainer endDeviceDevices = netDeviceHelper.Install (endDevices);
  Ptr<NetDevice> endDeviceDevice = endDeviceDevices.Get (0);
  Ptr<ClassAEndDeviceLorawanMac> endDeviceMac = endDeviceDevice->GetObject<ClassAEndDeviceLorawanMac> ();
  Ptr<LoraPhy> endDevicePhy = endDeviceDevice->GetObject<LoraNetDevice> ()->GetPhy ();

  // Set up applications
  TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
  Ptr<Socket> recvSink = Socket::CreateSocket (endDevice, tid);
  InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), 80);
  recvSink->Bind (local);
  recvSink->SetRecvCallback (MakeCallback (&ReceivePacket));

  Ptr<Socket> source = Socket::CreateSocket (gateway, tid);
  InetSocketAddress remote = InetSocketAddress (Ipv4Address ("255.255.255.255"), 80);
  source->SetAllowBroadcast (true);
  source->Connect (remote);

  // Schedule the first beacon transmission
  Simulator::Schedule (Seconds (0), &SendBeacon, gateway, gatewayMac, gatewayPhy);

  // Schedule statistics update
  Simulator::Schedule(Seconds(1), &UpdateStatistics);

  // Run simulation
  Simulator::Run ();
  Simulator::Destroy ();

  return 0;
}
