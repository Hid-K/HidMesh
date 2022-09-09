#include "./HidMesh.hpp"
#ifdef HID_MESH_SIMULATION_TEST
HidMesh::NetworkNode::NetworkNode( UNIT_ID id,
                void (*microcode)( NetworkNode * th ),
                void (*onPackage)( NetworkNode * th, const HidMeshPackage * package )
                )
{
    this->id = id;

    this->microcode = microcode;
    this->onPackage = onPackage;

    // this->microcodeThread = std::thread( this->microcode, this );
    packagesCache = HidMeshNodePackagesCachesStorage();
};
#else
NetworkNode( UNIT_ID id,
             void (*onPackage)( NetworkNode * th, const HidMeshPackage * package ) )
{
    this->id = id;

    this->onPackage = onPackage;

    packagesCache = HidMeshNodePackagesCachesStorage();
}
#endif

HidMesh::NetworkNode::~NetworkNode()
{
    this->microcodeThread.~thread();
};

void HidMesh::NetworkNode::processPackage( const HidMeshPackage * package )
{
    if(
        !this->getCache()->isCached( package ) &&
        package->ttl > 0 &&
        HidMesh::checkPackageCRC( *package ) &&
        package->src != this->getId()
       )
    {
        if( package->dst == this->getId() || package->dst == BROADCAST_PACKAGE_DST )
        {
            this->onPackage( this, package );
            
        } else
        {
            this->relayPackage( package );
        }
    }
};

void HidMesh::NetworkNode::sendDataToAllLinkedNodes( const HidMeshPackage * package )
{
    if( this->linkedNodes != nullptr  && linkedNodesCount > 0 )
    {
        for (size_t i = 0; i < linkedNodesCount; ++i)
        {
            linkedNodes[i]->processPackage( package );
        }
    }
}

//void HidMesh::NetworkNode::onPackage(const HidMeshPackage *package)
//{
//    std::cout << "Node " << this->getId() << " got package from " << package->src << " with data:\n";
//
//    for (size_t i = 0; i < HidMesh::HID_MESH_PACKAGE_PAYLOAD_SIZE; ++i)
//    {
//        std::cout<<package->payload[i];
//    }
//
//    std::cout<<std::endl;
//}

#ifdef HID_MESH_SIMULATION_TEST
/*
 This functional is available in simulation mode only
 */
void HidMesh::NetworkNode::start()
{
    this->microcodeThread = std::thread( this->microcode, this );
    this->microcodeThread.detach();
}

/*
 This functional is available in simulation mode only
 */
void HidMesh::NetworkNode::linkNode( NetworkNode * node )
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

    for (size_t i = 0; i < this->linkedNodesCount-1; ++i)
    {
        std::cout << "Node " << this->linkedNodes[i]->getId() << "are linked to " << this->id << "now.\n";
    }
    
}
#endif


HidMesh::UNIT_ID HidMesh::NetworkNode::getId()
{
    return this->id;
}

HidMesh::HidMeshNodePackagesCachesStorage * HidMesh::NetworkNode::getCache()
{
    return &this->packagesCache;
}

HidMesh::NetworkNode * HidMesh::NetworkNode::operator=( NetworkNode * object )
{
    this->id = object->getId();
    this->linkedNodes = object->linkedNodes;
    this->linkedNodesCount = object->linkedNodesCount;
    this->microcode = object->microcode;
    this->onPackage = object->onPackage;

    return this;
}

void HidMesh::NetworkNode::sendData( UNIT_ID dst, uint8_t payload[ HID_MESH_PACKAGE_PAYLOAD_SIZE ] )
{
    HidMesh::HidMeshPackage package = HidMesh::buildPackage( this->getId(), dst, 255, payload );

    this->sendDataToAllLinkedNodes( &package );
}

void HidMesh::NetworkNode::relayPackage( const HidMesh::HidMeshPackage * package )
{
    HidMesh::HidMeshPackage packageBuf;

    memcpy( &packageBuf, package, sizeof( HidMesh::HidMeshPackage ) );

    packageBuf.ttl--;
    this->sendDataToAllLinkedNodes( &packageBuf );
}
