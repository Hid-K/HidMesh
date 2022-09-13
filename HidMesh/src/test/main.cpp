#include "../main/HidMesh.hpp"

#include <iostream>
#include <chrono>
#include <thread>

size_t nodesCount = 100;

class HidMeshTestNode : public HidMesh::NetworkNode
{
    private:
        NetworkNode ** linkedNodes = nullptr;
        size_t linkedNodesCount;

        void (*microcode)( HidMeshTestNode * th );

        std::thread microcodeThread;
    
    public:
        HidMeshTestNode(
                        HidMesh::UNIT_ID id,
                        void (*microcode)( HidMeshTestNode * th )
                        ) : HidMesh::NetworkNode( id )
        {
            this->microcode = microcode;
        }
    
        void onPackage( const HidMesh::HidMeshPackage * package )
        {
            std::cout << "Node " << this->getId() << " got package from " << package->src << " with data:\n";

            for (size_t i = 0; i < HidMesh::HID_MESH_PACKAGE_PAYLOAD_SIZE; ++i)
            {
                std::cout<<package->payload[i];
            }

            std::cout<<std::endl;
        }
    
        void sendData( HidMesh::UNIT_ID dst, uint8_t payload[ HidMesh::HID_MESH_PACKAGE_PAYLOAD_SIZE ] )
        {
            HidMesh::HidMeshPackage package = HidMesh::buildPackage( this->getId(), dst, 255, payload );
            
            this->sendPackage( &package );
        }

        void sendPackage( const HidMesh::HidMeshPackage * package )
        {
            if( this->linkedNodes != nullptr  && linkedNodesCount > 0 )
            {
                for (size_t i = 0; i < linkedNodesCount; ++i)
                {
                    linkedNodes[i]->processPackage( package );
                }
            }
        }
    
        void start()
        {
            this->microcodeThread = std::thread( this->microcode, this );
            this->microcodeThread.detach();
        }

        void linkNode( HidMeshTestNode * node )
        {
            if( this->linkedNodes != nullptr  && linkedNodesCount > 0 )
            {
                NetworkNode ** linkedNodesBuf = (NetworkNode **)malloc( sizeof( NetworkNode * ) * ( this->linkedNodesCount ) );

                memcpy( linkedNodesBuf, this->linkedNodes, sizeof( NetworkNode * ) * ( this->linkedNodesCount ) );

                this->linkedNodesCount++;

                free( linkedNodes );

                this->linkedNodes = (NetworkNode **)malloc( sizeof( NetworkNode * ) * ( this->linkedNodesCount ) );

                memcpy( this->linkedNodes, linkedNodesBuf, this->linkedNodesCount );

                this->linkedNodes[ this->linkedNodesCount - 1 ] = node;
            } else
            {
                this->linkedNodes = (NetworkNode **)malloc( sizeof( NetworkNode * ) );

                this->linkedNodes[ 0 ] = node;

                this->linkedNodesCount = 1;
            }

//            for (size_t i = 0; i < this->linkedNodesCount-1; ++i)
//            {
//                std::cout << "Node " << this->linkedNodes[i]->getId() << "are linked to " << this->getId() << "now.\n";
//            }
            
        }
    
        HidMeshTestNode * operator=( HidMeshTestNode * node )
        {
            this->microcode = node->microcode;
            this->setId( node->getId() );
            this->linkedNodes = node->linkedNodes;
            this->linkedNodesCount = node->linkedNodesCount;
            
            return this;
        }
    
};

void nodeMicrocode( HidMeshTestNode * th )
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

int main()
{
    HidMeshTestNode * nodes = ( HidMeshTestNode * )malloc( sizeof( HidMeshTestNode ) * nodesCount );

    for (size_t i = 0; i < nodesCount; ++i)
    {
        nodes[i] = new HidMeshTestNode( i, nodeMicrocode );
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
