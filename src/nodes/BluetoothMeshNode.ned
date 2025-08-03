package bluetoothmeshsimulation.nodes;

import bluetoothmeshsimulation.protocols.BluetoothMeshProtocol;

//
// Bluetooth Mesh Node with mesh protocol capabilities
//
module BluetoothMeshNode
{
    parameters:
        @display("i=device/cellphone");
        
        // Node parameters
        int nodeId = default(0);
        string nodeRole = default("relay"); // "relay", "friend", "lowpower", "sensor"
        double batteryLevel = default(1.0);
        bool idsEnabled = default(true);
        
    submodules:
        // Bluetooth Mesh Protocol Stack
        meshProtocol: BluetoothMeshProtocol {
            @display("p=100,100");
        }
}