#include "BluetoothMeshProtocol.h"
#include <sstream>

namespace bluetoothmeshsimulation {

Define_Module(BluetoothMeshProtocol);

BluetoothMeshProtocol::BluetoothMeshProtocol()
{
    beaconTimer = nullptr;
    cleanupTimer = nullptr;
    heartbeatTimer = nullptr;
    currentSequenceNumber = 0;
    nodeId = 0;
    maxTTL = 10;
    relayProbability = 0.8;
    beaconInterval = 10.0;
    routeTimeout = 60.0;
}

BluetoothMeshProtocol::~BluetoothMeshProtocol()
{
    cancelAndDelete(beaconTimer);
    cancelAndDelete(cleanupTimer);
    cancelAndDelete(heartbeatTimer);

    // Clean up message queue
    while (!messageQueue.empty()) {
        delete messageQueue.front();
        messageQueue.pop();
    }
}

void BluetoothMeshProtocol::initialize()
{
    // Read parameters
    maxTTL = par("maxTTL").intValue();
    relayProbability = par("relayProbability").doubleValue();
    beaconInterval = par("beaconInterval").doubleValue();
    routeTimeout = par("routeTimeout").doubleValue();

    // Get node identification
    nodeId = getParentModule()->getIndex();
    nodeAddress = getNodeAddress();

    currentSequenceNumber = 0;

    // Register signals
    messagesSent = registerSignal("messagesSent");
    messagesReceived = registerSignal("messagesReceived");
    messagesRelayed = registerSignal("messagesRelayed");
    routingTableSize = registerSignal("routingTableSize");

    // Initialize timers
    beaconTimer = new cMessage("beaconTimer");
    cleanupTimer = new cMessage("cleanupTimer");
    heartbeatTimer = new cMessage("heartbeatTimer");

    // Schedule first events
    scheduleAt(simTime() + uniform(0, beaconInterval), beaconTimer);
    scheduleAt(simTime() + routeTimeout, cleanupTimer);
    scheduleAt(simTime() + uniform(1, 5), heartbeatTimer);

    EV << "BluetoothMeshProtocol initialized for Node " << nodeId
       << " with address " << nodeAddress << endl;
}

void BluetoothMeshProtocol::handleMessage(cMessage* msg)
{
    if (msg->isSelfMessage()) {
        handleSelfMessage(msg);
    }
    else {
        // Handle received messages
        BluetoothMeshMessage* meshMsg = dynamic_cast<BluetoothMeshMessage*>(msg);
        if (meshMsg) {
            EV << "Received mesh message: " << meshMsg->getName()
               << " from " << meshMsg->getSrcAddr() << endl;
            emit(messagesReceived, 1L);

            if (!isDuplicateMessage(meshMsg)) {
                cacheMessage(meshMsg);
                updateRoutingTable(meshMsg);

                if (shouldRelay(meshMsg)) {
                    relayMessage(meshMsg);
                    emit(messagesRelayed, 1L);
                }
            } else {
                EV << "Duplicate message detected, dropping" << endl;
            }
        } else {
            EV << "Received non-mesh message: " << msg->getName() << endl;
        }
        delete msg;
    }
}

void BluetoothMeshProtocol::handleSelfMessage(cMessage* msg)
{
    if (msg == beaconTimer) {
        sendBeacon();
        scheduleAt(simTime() + beaconInterval, beaconTimer);
    }
    else if (msg == cleanupTimer) {
        cleanupStaleData();
        scheduleAt(simTime() + routeTimeout, cleanupTimer);
    }
    else if (msg == heartbeatTimer) {
        sendHeartbeat();
        // Also send periodic data messages
        if (uniform(0, 1) < 0.3) { // 30% chance to send data message
            sendDataMessage();
        }
        scheduleAt(simTime() + 10.0 + uniform(-2, 2), heartbeatTimer);
    }
    else {
        EV << "Unknown self message: " << msg->getName() << endl;
    }
}

BluetoothMeshMessage* BluetoothMeshProtocol::createMeshMessage(const char* name, BluetoothMeshMessageType type)
{
    BluetoothMeshMessage* msg = new BluetoothMeshMessage(name, type);
    msg->setSrcAddr(nodeAddress);
    msg->setTtl(maxTTL);
    msg->setSequenceNumber(getNextSequenceNumber());
    msg->setTimestamp(simTime());
    msg->setDataSize(uniform(50, 200));
    msg->setHopCount(0);
    msg->setReliability(1.0);
    msg->setPriority(0);

    return msg;
}

void BluetoothMeshProtocol::sendMessage(BluetoothMeshMessage* msg)
{
    if (!msg) {
        EV << "Null message, dropping" << endl;
        return;
    }

    // Set source address if not set
    if (msg->getSrcAddr().empty()) {
        msg->setSrcAddr(nodeAddress);
    }

    // Add to path
    addToPath(msg, nodeAddress);

    EV << "Sending mesh message: " << msg->getName()
       << " (TTL: " << msg->getTtl() << ", Type: " << msg->getKind() << ")" << endl;
    emit(messagesSent, 1L);

    // In a real implementation, this would be sent to the radio/network layer
    // For simulation purposes, we'll just log it and clean up
    // This is a simplified approach for demonstration

    // Simulate message transmission delay
    simtime_t delay = uniform(0.001, 0.01); // 1-10ms delay
    EV << "Message sent with " << delay << "s transmission delay" << endl;

    delete msg; // Remove this line when integrating with actual networking
}

void BluetoothMeshProtocol::relayMessage(BluetoothMeshMessage* msg)
{
    if (!msg || msg->getTtl() <= 0) {
        EV << "Message TTL expired, not relaying" << endl;
        return;
    }

    // Check if we're already in the path to avoid loops
    if (isInPath(msg, nodeAddress)) {
        EV << "Loop detected, not relaying message" << endl;
        return;
    }

    BluetoothMeshMessage* relayMsg = msg->dup();
    relayMsg->setTtl(relayMsg->getTtl() - 1);
    relayMsg->setHopCount(relayMsg->getHopCount() + 1);
    relayMsg->setIsRelay(true);

    EV << "Relaying message: " << relayMsg->getName()
       << " (TTL: " << relayMsg->getTtl() << ", Hops: " << relayMsg->getHopCount() << ")" << endl;

    sendMessage(relayMsg);
}

bool BluetoothMeshProtocol::shouldRelay(BluetoothMeshMessage* msg)
{
    if (!msg || msg->getTtl() <= 0) {
        return false;
    }

    // Don't relay our own messages
    if (msg->getSrcAddr() == nodeAddress) {
        return false;
    }

    // Don't relay if we're in the path
    if (isInPath(msg, nodeAddress)) {
        return false;
    }

    // Use relay probability
    return uniform(0, 1) < relayProbability;
}

void BluetoothMeshProtocol::updateRoutingTable(BluetoothMeshMessage* msg)
{
    if (!msg || msg->getSrcAddr().empty()) return;

    std::string srcAddr = msg->getSrcAddr();
    int hopCount = msg->getHopCount();

    // Update or add routing entry
    auto it = routingTable.find(srcAddr);
    if (it == routingTable.end() || it->second.hopCount > hopCount) {
        RoutingEntry entry(srcAddr, srcAddr, hopCount);
        routingTable[srcAddr] = entry;

        EV << "Updated routing table for " << srcAddr << " (hops: " << hopCount << ")" << endl;
        emit(routingTableSize, (long)routingTable.size());
    }
}

void BluetoothMeshProtocol::sendBeacon()
{
    BluetoothMeshMessage* beacon = createMeshMessage("beacon", MESH_BEACON);
    beacon->setTtl(1); // Beacons are only sent to direct neighbors
    beacon->setPayload("BEACON");

    EV << "Sending beacon message from node " << nodeId << endl;
    sendMessage(beacon);
}

void BluetoothMeshProtocol::sendHeartbeat()
{
    BluetoothMeshMessage* heartbeat = createMeshMessage("heartbeat", MESH_HEARTBEAT);

    std::stringstream payload;
    payload << "HEARTBEAT from Node " << nodeId << " at " << simTime();
    heartbeat->setPayload(payload.str());

    EV << "Node " << nodeId << " sending heartbeat #" << currentSequenceNumber << endl;
    sendMessage(heartbeat);
}

void BluetoothMeshProtocol::sendDataMessage()
{
    BluetoothMeshMessage* dataMsg = createMeshMessage("dataMessage", MESH_DATA);

    std::stringstream payload;
    payload << "DATA from Node " << nodeId << " seq:" << currentSequenceNumber
            << " size:" << dataMsg->getDataSize() << " bytes";
    dataMsg->setPayload(payload.str());

    // Set random priority
    dataMsg->setPriority(intuniform(0, 2));

    // Set deadline for high priority messages
    if (dataMsg->getPriority() > 0) {
        dataMsg->setDeadline(simTime() + 30.0); // 30 second deadline
    }

    EV << "Node " << nodeId << " sending data message #" << currentSequenceNumber
       << " (priority: " << dataMsg->getPriority() << ")" << endl;
    sendMessage(dataMsg);
}

bool BluetoothMeshProtocol::isDuplicateMessage(BluetoothMeshMessage* msg)
{
    if (!msg) return true;

    MessageCache cache(msg->getSrcAddr(), msg->getSequenceNumber());
    return messageCache.find(cache) != messageCache.end();
}

void BluetoothMeshProtocol::cacheMessage(BluetoothMeshMessage* msg)
{
    if (!msg) return;

    MessageCache cache(msg->getSrcAddr(), msg->getSequenceNumber());
    messageCache.insert(cache);

    // Limit cache size
    if (messageCache.size() > 1000) {
        auto oldest = messageCache.begin();
        messageCache.erase(oldest);
    }
}

void BluetoothMeshProtocol::cleanupStaleData()
{
    simtime_t now = simTime();

    // Clean up routing table
    auto it = routingTable.begin();
    while (it != routingTable.end()) {
        if (now - it->second.lastUpdated > routeTimeout) {
            EV << "Removing stale route to " << it->first << endl;
            it = routingTable.erase(it);
        } else {
            ++it;
        }
    }

    emit(routingTableSize, (long)routingTable.size());

    // Clean up message cache
    auto cacheIt = messageCache.begin();
    while (cacheIt != messageCache.end()) {
        if (now - cacheIt->timestamp > routeTimeout) {
            cacheIt = messageCache.erase(cacheIt);
        } else {
            ++cacheIt;
        }
    }

    EV << "Cleanup complete. Routing table size: " << routingTable.size()
       << ", Message cache size: " << messageCache.size() << endl;
}

bool BluetoothMeshProtocol::isInPath(BluetoothMeshMessage* msg, const std::string& addr)
{
    if (!msg) return false;

    for (int i = 0; i < msg->getPathArraySize(); i++) {
        if (msg->getPath(i) == addr) {
            return true;
        }
    }
    return false;
}

void BluetoothMeshProtocol::addToPath(BluetoothMeshMessage* msg, const std::string& addr)
{
    if (!msg) return;

    // Check if address is already in path
    if (!isInPath(msg, addr)) {
        int currentSize = msg->getPathArraySize();
        msg->setPathArraySize(currentSize + 1);
        msg->setPath(currentSize, addr);
    }
}

std::string BluetoothMeshProtocol::getNodeAddress()
{
    std::stringstream ss;
    ss << "Node" << nodeId;
    return ss.str();
}

void BluetoothMeshProtocol::finish()
{
    EV << "BluetoothMeshProtocol finishing for Node " << nodeId << endl;
    EV << "Final routing table size: " << routingTable.size() << endl;
    EV << "Final message cache size: " << messageCache.size() << endl;
    EV << "Total messages generated: " << currentSequenceNumber << endl;

    recordScalar("finalRoutingTableSize", routingTable.size());
    recordScalar("finalMessageCacheSize", messageCache.size());
    recordScalar("totalMessagesGenerated", currentSequenceNumber);
    recordScalar("nodeId", nodeId);
}

} // namespace bluetoothmeshsimulation
