#ifndef BLUETOOTHMESHPROTOCOL_H
#define BLUETOOTHMESHPROTOCOL_H

#include "inet/common/INETDefs.h"
#include "inet/linklayer/common/MacAddress.h"
#include "inet/common/packet/Packet.h"
#include <map>
#include <set>
#include <queue>
#include <vector>

using namespace omnetpp;
using namespace inet;

// Message types for Bluetooth Mesh
enum BluetoothMeshMessageType {
    MESH_DATA = 100,
    MESH_CONTROL = 101,
    MESH_BEACON = 102,
    MESH_HEARTBEAT = 103,
    MESH_ADVERTISEMENT = 104,
    MESH_ATTACK = 199
};

// Bluetooth Mesh Message class
class INET_API BluetoothMeshMessage : public cMessage
{
protected:
    MacAddress srcAddr;
    MacAddress destAddr;
    int ttl;
    int sequenceNumber;
    simtime_t timestamp;
    std::vector<MacAddress> path;

public:
    BluetoothMeshMessage(const char* name = nullptr, short kind = 0);
    BluetoothMeshMessage(const BluetoothMeshMessage& other);
    BluetoothMeshMessage& operator=(const BluetoothMeshMessage& other);
    virtual BluetoothMeshMessage* dup() const override;

    // Getters and setters
    MacAddress getSrcAddr() const { return srcAddr; }
    void setSrcAddr(MacAddress addr) { srcAddr = addr; }

    MacAddress getDestAddr() const { return destAddr; }
    void setDestAddr(MacAddress addr) { destAddr = addr; }

    int getTtl() const { return ttl; }
    void setTtl(int t) { ttl = t; }
    void decrementTtl() { if(ttl > 0) ttl--; }

    int getSequenceNumber() const { return sequenceNumber; }
    void setSequenceNumber(int seq) { sequenceNumber = seq; }

    simtime_t getTimestamp() const { return timestamp; }
    void setTimestamp(simtime_t t) { timestamp = t; }

    std::vector<MacAddress>& getPath() { return path; }
    void addToPath(MacAddress addr) { path.push_back(addr); }
    bool isInPath(MacAddress addr) const;

    bool isValid() const;
};

// Routing table entry
struct RoutingEntry {
    MacAddress destination;
    MacAddress nextHop;
    int hopCount;
    simtime_t lastUpdated;
    double reliability;

    RoutingEntry() : hopCount(0), reliability(1.0) {}
    RoutingEntry(MacAddress dest, MacAddress next, int hops)
        : destination(dest), nextHop(next), hopCount(hops),
          lastUpdated(simTime()), reliability(1.0) {}
};

// Message cache for duplicate detection
struct MessageCache {
    MacAddress source;
    int sequenceNumber;
    simtime_t timestamp;

    MessageCache(MacAddress src, int seq)
        : source(src), sequenceNumber(seq), timestamp(simTime()) {}

    bool operator<(const MessageCache& other) const {
        if (source != other.source) return source < other.source;
        return sequenceNumber < other.sequenceNumber;
    }
};

// Main Protocol Class
class INET_API BluetoothMeshProtocol : public cSimpleModule
{
protected:
    // Protocol parameters
    int maxTTL;
    double relayProbability;
    simtime_t beaconInterval;
    simtime_t routeTimeout;

    // Network state
    std::map<MacAddress, RoutingEntry> routingTable;
    std::set<MessageCache> messageCache;
    std::queue<BluetoothMeshMessage*> messageQueue;

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

public:
    BluetoothMeshProtocol();
    virtual ~BluetoothMeshProtocol();

    virtual void initialize(int stage) override;
    virtual int numInitStages() const override { return 2; }
    virtual void handleMessage(cMessage* msg) override;
    virtual void finish() override;

    // Message handling
    virtual void sendMessage(BluetoothMeshMessage* msg);
    virtual void relayMessage(BluetoothMeshMessage* msg);
    virtual bool shouldRelay(BluetoothMeshMessage* msg);

    // Routing functions
    virtual void updateRoutingTable(BluetoothMeshMessage* msg);
    virtual void sendBeacon();

    // Message validation and caching
    virtual bool isDuplicateMessage(BluetoothMeshMessage* msg);
    virtual void cacheMessage(BluetoothMeshMessage* msg);
    virtual void cleanupStaleData();

protected:
    virtual void handleSelfMessage(cMessage* msg);
    virtual int getNextSequenceNumber() { return ++currentSequenceNumber; }
};

#endif // BLUETOOTHMESHPROTOCOL_H
