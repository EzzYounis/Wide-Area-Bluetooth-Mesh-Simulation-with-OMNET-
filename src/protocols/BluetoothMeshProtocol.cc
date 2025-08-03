#include "BluetoothMeshProtocol.h"

Define_Module(BluetoothMeshProtocol);

BluetoothMeshProtocol::BluetoothMeshProtocol()
{
    heartbeatTimer = nullptr;
    dataTimer = nullptr;
    messageCount = 0;
    nodeId = 0;
}

BluetoothMeshProtocol::~BluetoothMeshProtocol()
{
    cancelAndDelete(heartbeatTimer);
    cancelAndDelete(dataTimer);
}

void BluetoothMeshProtocol::initialize()
{
    // Get node ID from parent module index
    nodeId = getParentModule()->getIndex();

    EV << "BluetoothMeshProtocol initialized for Node " << nodeId << endl;

    // Initialize timers
    heartbeatTimer = new cMessage("heartbeat");
    dataTimer = new cMessage("dataMsg");

    // Schedule first events
    scheduleAt(simTime() + uniform(1, 5), heartbeatTimer);
    scheduleAt(simTime() + uniform(10, 20), dataTimer);

    messageCount = 0;
}

void BluetoothMeshProtocol::handleMessage(cMessage* msg)
{
    if (msg == heartbeatTimer) {
        sendHeartbeat();
        // Schedule next heartbeat
        scheduleAt(simTime() + 10 + uniform(-2, 2), heartbeatTimer);
    }
    else if (msg == dataTimer) {
        sendDataMessage();
        // Schedule next data message
        scheduleAt(simTime() + exponential(30), dataTimer);
    }
    else {
        // Handle received messages
        EV << "Node " << nodeId << " received message: " << msg->getName() << endl;
        delete msg;
    }
}

void BluetoothMeshProtocol::sendHeartbeat()
{
    messageCount++;
    EV << "Node " << nodeId << " sending heartbeat #" << messageCount
       << " at time " << simTime() << endl;

    // Create heartbeat message
    cMessage* heartbeat = new cMessage("heartbeat");
    heartbeat->addPar("sourceNode") = nodeId;
    heartbeat->addPar("messageId") = messageCount;
    heartbeat->addPar("timestamp") = simTime().dbl();

    // Simulate sending to network (for now, just log it)
    EV << "Heartbeat sent from Node " << nodeId << endl;
    delete heartbeat; // In real implementation, this would be sent to other nodes
}

void BluetoothMeshProtocol::sendDataMessage()
{
    messageCount++;
    EV << "Node " << nodeId << " sending data message #" << messageCount
       << " at time " << simTime() << endl;

    // Create data message
    cMessage* dataMsg = new cMessage("dataMessage");
    dataMsg->addPar("sourceNode") = nodeId;
    dataMsg->addPar("messageId") = messageCount;
    dataMsg->addPar("dataSize") = intuniform(50, 200); // Random data size
    dataMsg->addPar("timestamp") = simTime().dbl();

    // Simulate mesh routing
    EV << "Data message sent from Node " << nodeId << endl;
    delete dataMsg; // In real implementation, this would be routed through mesh
}

void BluetoothMeshProtocol::finish()
{
    EV << "Node " << nodeId << " finishing. Total messages sent: "
       << messageCount << endl;

    // Record statistics
    recordScalar("totalMessagesSent", messageCount);
    recordScalar("nodeId", nodeId);
}
