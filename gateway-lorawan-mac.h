/*
 * Copyright (c) 2017 University of Padova
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Davide Magrin <magrinda@dei.unipd.it>
 */

#ifndef GATEWAY_LORAWAN_MAC_H
#define GATEWAY_LORAWAN_MAC_H

#include "lora-tag.h"
#include "lorawan-mac.h"

namespace ns3
{
namespace lorawan
{

/**
 * \ingroup lorawan
 *
 * Class representing the MAC layer of a LoRaWAN gateway.
 */
class GatewayLorawanMac : public LorawanMac
{
  public:
    /**
     *  Register this type.
     *  \return The object TypeId.
     */
    static TypeId GetTypeId();

    GatewayLorawanMac();           //!< Default constructor
    ~GatewayLorawanMac() override; //!< Destructor

    // Implementation of the LorawanMac interface
    void Send(Ptr<Packet> packet) override;

    /**
     * Check whether the underlying PHY layer of the gateway is currently transmitting.
     *
     * \return True if it is transmitting, false otherwise.
     */
    bool IsTransmitting();

    // Implementation of the LorawanMac interface
    void Receive(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void FailedReception(Ptr<const Packet> packet) override;

    // Implementation of the LorawanMac interface
    void TxFinished(Ptr<const Packet> packet) override;

    /**
     * Return the next time at which we will be able to transmit on the specified frequency.
     *
     * \param frequency The frequency value [MHz].
     * \return The next transmission time.
     */
    Time GetWaitingTime(double frequency);
     void SendBeacon();
    void ScheduleNextBeacon();
    void HandlePingSlot(Ptr<Packet> packet, uint8_t dataRate, uint32_t frequency);


  private:
  
   Time m_beaconInterval;
    EventId m_beaconEvent;
  protected:
};

} // namespace lorawan

} // namespace ns3
#endif /* GATEWAY_LORAWAN_MAC_H */
