package bluetoothmeshsimulation.networks;

import bluetoothmeshsimulation.nodes.BluetoothMeshNode;

//
// Bluetooth Mesh Network with Attack Simulation and IDS
//
network BluetoothMeshNetwork
{
    parameters:
        // Network topology parameters
        int numLegitimateNodes = default(10);
        int numAttackerNodes = default(0);
        double areaWidth @unit(m) = default(1000m);
        double areaHeight @unit(m) = default(800m);
        
        // Simulation parameters
        bool enableAttacks = default(false);
        bool enableIDS = default(true);
        bool enableVisualization = default(true);
        
        @display("bgb=1200,900;bgg=100,1,grey95");
        @figure[title](type=label; pos=10,10; anchor=nw; color=darkblue; 
                      text="Bluetooth Mesh Network Simulation");
        
    submodules:
        // Legitimate mesh nodes
        legitimateNode[numLegitimateNodes]: BluetoothMeshNode {
            @display("p=uniform(100,900),uniform(100,700)");
            nodeId = index;
        }
}