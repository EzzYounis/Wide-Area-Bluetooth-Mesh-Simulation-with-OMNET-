package bluetoothmeshsimulation.protocols;

//
// Bluetooth Mesh Protocol Module
// Implements mesh networking functionality including flooding,
// routing, neighbor discovery, and duplicate detection
//
simple BluetoothMeshProtocol
{
    parameters:
        @display("i=block/protocol");
        @class(BluetoothMeshProtocol);
        
        // Basic protocol parameters
        int maxTTL = default(10);                           // Maximum hops for messages
        double relayProbability = default(0.8);             // Probability of relaying messages
        double beaconInterval @unit(s) = default(10s);      // Beacon sending interval
        double routeTimeout @unit(s) = default(60s);        // When to remove stale routes
        
        // Statistics signals
        @signal[messagesSent](type=long);
        @signal[messagesReceived](type=long);
        @signal[messagesRelayed](type=long);
        @signal[routingTableSize](type=long);
        
        // Statistics recording
        @statistic[messagesSent](title="Messages Sent"; record=count,sum,vector);
        @statistic[messagesReceived](title="Messages Received"; record=count,sum,vector);
        @statistic[messagesRelayed](title="Messages Relayed"; record=count,sum,vector);
        @statistic[routingTableSize](title="Routing Table Size"; record=last,vector);
}