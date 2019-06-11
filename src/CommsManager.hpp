// CommsManager.hpp

#ifndef COMMSMANAGER_HPP
#define COMMSMANAGER_HPP

#include <functional>
#include <string>
#include <unordered_map>

#include "cinder/Log.h"
#include "cinder/osc/Osc.h"
#include "cinder/Timeline.h"


namespace ch {

using namespace ci;

using Receiver = osc::ReceiverUdp;
using protocol = asio::ip::udp;
using Sender = osc::SenderUdp;

class CommsManager {
public:
    CommsManager() :
        mReceiver{localPortReceive},
        mSender{localPortSend, destinationHost, destinationPort} {};
    void setup(const std::string& listenPath,
            const std::function<void(int, int)>& updateCallback);
    void addListener(const std::string& listenPath,
            const std::function<void(int, int)>& updateCallback);
    void generateEvent();

private:
    void onSendError(asio::error_code error);

    const uint16_t localPortReceive = 12000;
    const std::string destinationHost = "127.0.0.1";
    const uint16_t destinationPort = 5556;
    const uint16_t localPortSend = 5557;

    std::unordered_map<std::string, std::function<void(int, int)> > mUpdateCallbacks;

    Receiver mReceiver;
    std::map<uint64_t, protocol::endpoint> mConnections;

    Sender mSender;
    bool mIsConnected = false;
}; // class CommsManager

} // namespace ch

#endif // COMMSMANAGER_HPP
