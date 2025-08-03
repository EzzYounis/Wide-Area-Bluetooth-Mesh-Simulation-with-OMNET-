#ifndef BLUETOOTHMESHPROTOCOL_H
#define BLUETOOTHMESHPROTOCOL_H

#include <omnetpp.h>
#include <map>
#include <set>
#include <queue>
#include <vector>
#include <string>

using namespace omnetpp;

namespace bluetoothmeshsimulation {

// Routing table entry
struct RoutingEntry {
    std::string destination;
    std::string nextHop;
    int hopCount;
    simtime_t lastUpdated;
    double reliability;

    RoutingEntry() : hopCount(0), reliability(1.0) {}
    RoutingEntry(const std::string& dest, const std::string& next, int hops)
        : destination(dest), nextHop(next), hopCount(hops),
          lastUpdated(simTime()), reliability(1.0) {}
};

// Message cache for duplicate detection
struct MessageCache {
    std::string source;
    int sequenceNumber;
    simtime_t timestamp;

    MessageCache(const std::string& src, int seq)
        : source(src), sequenceNumber(seq), timestamp(simTime()) {}

    bool operator<(const MessageCache& other) const {
        if (source != other.source) return source < other.source;
        return sequenceNumber < other.sequenceNumber;
    }
};

// Main Protocol Class
class BluetoothMeshProtocol : public cSimpleModule
{
protected:
    // Protocol parameters
    int maxTTL;
    double relayProbability;
    double beaconInterval;
    double routeTimeout;

    // Network state
    std::map<std::string, RoutingEntry> routingTable;
    std::set<MessageCache> messageCache;
    std::queue<BluetoothMeshMessage*> messageQueue;

    // Node identification
    std::string nodeAddress;
    int nodeId;

    // Sequence numbers
    int currentSequenceNumber;

    // Statistics
    simsignal_t messagesSent;
    simsignal_t messagesReceived;
    simsignal_t messagesRelayed;
    simsignal_t routingTableSize;

    // Timers
    cMessage* beaconTimer;
    cMessage* cleanupTimer;
    cMessage* heartbeatTimer;

public:
    BluetoothMeshProtocol();
    virtual ~BluetoothMeshProtocol();

    virtual void initialize() override;
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override;

    // Message handling
    virtual void sendMessage(BluetoothMeshMessage* msg);
    virtual void relayMessage(BluetoothMeshMessage* msg);
    virtual bool shouldRelay(BluetoothMeshMessage* msg);

    // Routing functions
    virtual void updateRoutingTable(BluetoothMeshMessage* msg);
    virtual void sendBeacon();
    virtual void sendHeartbeat();
    virtual void sendDataMessage();

    // Message validation and caching
    virtual bool isDuplicateMessage(BluetoothMeshMessage* msg);
    virtual void cacheMessage(BluetoothMeshMessage* msg);
    virtual void cleanupStaleData();

    // Utility functions
    virtual bool isInPath(BluetoothMeshMessage* msg, const std::string& addr);
    virtual void addToPath(BluetoothMeshMessage* msg, const std::string& addr);

protected:
    virtual void handleSelfMessage(cMessage* msg);
    virtual int getNextSequenceNumber() { return ++currentSequenceNumber; }
    virtual std::string getNodeAddress();
    virtual BluetoothMeshMessage* createMeshMessage(const char* name, BluetoothMeshMessageType type);
};

} // namespace bluetoothmeshsimulation

#endif // BLUETOOTHMESHPROTOCOL_H
