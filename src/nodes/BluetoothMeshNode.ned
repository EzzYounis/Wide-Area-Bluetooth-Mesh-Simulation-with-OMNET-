package bluetoothmeshsimulation.nodes;

import inet.node.inet.StandardHost;
import bluetoothmeshsimulation.protocols.BluetoothMeshProtocol;

//
// Bluetooth Mesh Node with IDS capabilities
//
module BluetoothMeshNode extends StandardHost
{
    parameters:
        @networkNode();
        @display("i=device/cellphone");
        
        // Node parameters
        int nodeId = default(0);
        string nodeRole = default("relay"); // "relay", "friend", "lowpower", "sensor"
        double batteryLevel = default(1.0);
        bool idsEnabled = default(true);
        
        // Mobility
        mobility.typename = default("RandomWaypointMobility");
        
        // Interface configuration
        numWlanInterfaces = 1;
        wlan[0].typename = default("Ieee80211Interface");
        wlan[0].radio.typename = default("Ieee80211ScalarRadio");
        
        // Remove unused interfaces
        numEthInterfaces = 0;
        numPppInterfaces = 0;
        numTunInterfaces = 0;
        
        // Application configuration
        numApps = 1;
        app[0].typename = default("UdpBasicApp");
        
    submodules:
        // Bluetooth Mesh Protocol Stack
        meshProtocol: BluetoothMeshProtocol {
            @display("p=200,100");
        }
        
    connections:
        // Connect mesh protocol to network layer
        meshProtocol.upperLayerOut --> networkLayer.ifIn++;
        meshProtocol.upperLayerIn <-- networkLayer.ifOut++;
        meshProtocol.lowerLayerOut --> wlan[0].upperLayerIn;
        meshProtocol.lowerLayerIn <-- wlan[0].upperLayerOut;
}