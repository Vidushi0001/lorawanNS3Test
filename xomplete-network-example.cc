/*
 * Example script to simulate a LoRaWAN network with a single end device and gateway.
 */

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
#include "ns3/lora-packet-tracker.h"
#include "ns3/lora-frame-header.h"
#include "ns3/network-server.h"
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/lora-channel.h"


using namespace ns3;
using namespace lorawan;

NS_LOG_COMPONENT_DEFINE("SimpleLorawanNetworkExample");

int main(int argc, char *argv[]) 
{
    // Enable logging for debugging purposes (optional)
    LogComponentEnable("LoraHelper", LOG_LEVEL_INFO);
    LogComponentEnable("LoraChannel", LOG_LEVEL_INFO);
    LogComponentEnable("LoraPhy", LOG_LEVEL_INFO);
    LogComponentEnable("LoraMac", LOG_LEVEL_INFO);

    // Create a NodeContainer
    NodeContainer endDevices;
    endDevices.Create(2); // Create two nodes: one for gateway, one for end device

    // Set up the mobility model
    MobilityHelper mobility;
    mobility.SetMobilityModel("ns3::ConstantPositionMobilityModel");
    mobility.Install(endDevices);

    // Create a LoraHelper
    Ptr<LoraHelper> loraHelper = CreateObject<LoraHelper>();

    // Create a LoraChannelHelper
   Ptr<LoraChannel> channelHelper = CreateObject<LoraChannel>();

    // Create a LoraPhyHelper
    LoraPhyHelper phyHelper;
   

    // Create a LorawanMacHelper
    LorawanMacHelper macHelper = LorawanMacHelper();

    // Install devices on the nodes
    NetDeviceContainer devices = loraHelper->Install(phyHelper, macHelper, endDevices);

    // Set frequency for both devices
    double frequency = 868000000.0; // Example frequency in Hz (868 MHz for EU868 band)
     // Set the channel for the physical layer helper
phyHelper.SetChannel(channelHelper);
channelHelper->SetFrequency(frequency);

    // Create a beacon packet
    Ptr<Packet> beaconPacket = Create<Packet>(100); // Example of creating a packet with 100 bytes payload

    // Create a GatewayLorawanMac instance
    Ptr<GatewayLorawanMac> gatewayMac = CreateObject<GatewayLorawanMac>();
    Ptr<ClassAEndDeviceLorawanMac> classAEndDeviceLorawanMac = CreateObject<ClassAEndDeviceLorawanMac>();

    // Set the devices to use these MACs
    devices.Get(0)->GetObject<LoraNetDevice>()->SetMac(gatewayMac);
    devices.Get(1)->GetObject<LoraNetDevice>()->SetMac(classAEndDeviceLorawanMac);

    // Send the beacon packet using the GatewayLorawanMac
    gatewayMac->Send(beaconPacket);
    classAEndDeviceLorawanMac->Receive(beaconPacket);

    // After a few seconds of sending beacon, create a normal packet to be transmitted to the gateway
    Simulator::Schedule(Seconds(2.0), &ClassAEndDeviceLorawanMac::Send, classAEndDeviceLorawanMac, beaconPacket);

    // Set up a periodic sender to send beacon packets periodically
    ApplicationContainer senderApps;
    Ptr<UniformRandomVariable> rv = CreateObject<UniformRandomVariable>();
    for (uint32_t i = 0; i < endDevices.GetN(); ++i)
    {
        Ptr<PeriodicSender> app = CreateObject<PeriodicSender>();
        app->SetInterval(Seconds(10)); // Set the interval for sending packets (e.g., every 10 seconds)
        app->SetPacketSize(100);
        endDevices.Get(i)->AddApplication(app);
        app->SetStartTime(Seconds(0));
        app->SetStopTime(Seconds(100));
        senderApps.Add(app);
    }

    Simulator::Stop(Seconds(100)); // Assuming you want to stop simulation after 100 seconds
    Simulator::Run();
    Simulator::Destroy();

    return 0;
}
