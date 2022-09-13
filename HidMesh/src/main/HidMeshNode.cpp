#include "./HidMesh.hpp"

HidMesh::NetworkNode::NetworkNode( UNIT_ID id )
{
    this->id = id;

    packagesCache = HidMeshNodePackagesCachesStorage();
}

HidMesh::NetworkNode::~NetworkNode() {};

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
            this->onPackage( package );
            
        } else
        {
            this->relayPackage( package );
        }
    }
};


HidMesh::UNIT_ID HidMesh::NetworkNode::getId()
{
    return this->id;
}

void HidMesh::NetworkNode::setId( UNIT_ID id )
{
    this->id = id;
}

HidMesh::HidMeshNodePackagesCachesStorage * HidMesh::NetworkNode::getCache()
{
    return &this->packagesCache;
}

//
//HidMesh::NetworkNode * HidMesh::NetworkNode::operator=( NetworkNode * object )
//{
//    this->id = object->getId();
//    
//    return this;
//}

void HidMesh::NetworkNode::relayPackage( const HidMesh::HidMeshPackage * package )
{
    HidMesh::HidMeshPackage packageBuf;

    memcpy( &packageBuf, package, sizeof( HidMesh::HidMeshPackage ) );

    packageBuf.ttl--;
    this->sendPackage( &packageBuf );
}
