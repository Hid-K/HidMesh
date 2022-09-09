#include "../main/HidMesh.hpp"

#include <iostream>
#include <chrono>
#include <thread>

size_t nodesCount = 100;

void nodeMicrocode( HidMesh::NetworkNode * th )
{
    std::cout << "Node " << th->getId() << " started!\n";

    for(;;)
    {
        unsigned char payload[ HidMesh::HID_MESH_PACKAGE_PAYLOAD_SIZE ];

        memset( &payload[0], '\0', HidMesh::HID_MESH_PACKAGE_PAYLOAD_SIZE );

        memcpy( &payload[0], "Hello!", 7 );

        th->sendData( rand() % nodesCount, payload );

        // std::cout << "Node " << th->getId() << " sent package\n";

        std::this_thread::sleep_for( std::chrono::milliseconds(1000) );
    };
}

void nodeOnPackage( HidMesh::NetworkNode * th, const HidMesh::HidMeshPackage * package )
{
    std::cout << "Node " << th->getId() << " got package from " << package->src << " with data:\n";

    for (size_t i = 0; i < HidMesh::HID_MESH_PACKAGE_PAYLOAD_SIZE; ++i)
    {
        std::cout<<package->payload[i];
    }

    std::cout<<std::endl;
}

int main()
{
    HidMesh::NetworkNode * nodes = ( HidMesh::NetworkNode * )malloc( sizeof( HidMesh::NetworkNode ) * nodesCount );

    for (size_t i = 0; i < nodesCount; ++i)
    {
        nodes[i] = new HidMesh::NetworkNode( i, nodeMicrocode, nodeOnPackage );
    }

    for (size_t i = 0; i < nodesCount; ++i)
    {
        for (size_t j = 0; j < 2; ++j)
        {
            nodes[i].linkNode(&nodes[ rand() % nodesCount ]);
        }    
    }

    for (size_t i = 0; i < nodesCount; ++i)
    {
        nodes[i].start(); 
    }
    
    sleep(1000000);

    return 0;
}
