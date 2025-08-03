//
// This file defines the Bluetooth Mesh message format
//

cplusplus {{
#include <vector>
#include <string>
}}

namespace bluetoothmeshsimulation;

//
// Message types for Bluetooth Mesh protocol
//
enum BluetoothMeshMessageType {
    MESH_DATA = 100;
    MESH_CONTROL = 101;
    MESH_BEACON = 102;
    MESH_HEARTBEAT = 103;
    MESH_ADVERTISEMENT = 104;
    MESH_ATTACK = 199;
}

//
// Bluetooth Mesh Protocol Message
// This message represents data packets in the mesh network
//
message BluetoothMeshMessage {
    // Source and destination addresses
    string srcAddr;
    string destAddr;
    
    // Mesh protocol fields
    int ttl = 10;                    // Time to live (hop count)
    int sequenceNumber = 0;          // Message sequence number for duplicate detection
    simtime_t timestamp;             // When message was created
    
    // Routing information
    string path[];                   // Path taken by the message (for loop prevention)
    
    // Message payload
    int dataSize = 0;                // Size of the data payload
    string payload;                  // Actual message content
    
    // Protocol specific fields
    bool isRelay = false;            // Is this a relayed message
    int hopCount = 0;                // Number of hops taken
    double reliability = 1.0;        // Reliability metric
    
    // Security fields (for future extension)
    bool isEncrypted = false;        // Is message encrypted
    string securityKey;              // Security key identifier
    
    // QoS fields
    int priority = 0;                // Message priority (0=normal, 1=high, 2=emergency)
    simtime_t deadline;              // Delivery deadline
}