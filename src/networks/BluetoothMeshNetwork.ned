package bluetoothmeshsimulation.networks;

import inet.networklayer.configurator.ipv4.Ipv4NetworkConfigurator;
import inet.physicallayer.wireless.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.visualizer.canvas.integrated.IntegratedCanvasVisualizer;
import bluetoothmeshsimulation.nodes.BluetoothMeshNode;

//
// Bluetooth Mesh Network with Attack Simulation and IDS
//
network BluetoothMeshNetwork
{
    parameters:
        // Network topology parameters
        int numLegitimateNodes = default(20);
        int numAttackerNodes = default(3);
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
        // Network infrastructure
        configurator: Ipv4NetworkConfigurator {
            @display("p=100,100;is=s");
        }
        
        radioMedium: Ieee80211ScalarRadioMedium {
            @display("p=100,200;is=s");
        }
        
        visualizer: IntegratedCanvasVisualizer if enableVisualization {
            @display("p=100,300;is=s");
        }
        
        // Legitimate mesh nodes
        legitimateNode[numLegitimateNodes]: BluetoothMeshNode {
            @display("p=uniform(100,900),uniform(100,700)");
            nodeId = index;
            idsEnabled = parent.enableIDS;
            
            // Mobility configuration
            mobility.constraintAreaMinX = 50m;
            mobility.constraintAreaMaxX = parent.areaWidth - 50m;
            mobility.constraintAreaMinY = 50m;
            mobility.constraintAreaMaxY = parent.areaHeight - 50m;
            mobility.speed = uniform(0.5mps, 2.5mps);
            mobility.waitTime = uniform(5s, 20s);
        }
        
    connections allowunconnected:
        // Wireless connections are handled by the radio medium
}