#include "HidMesh.hpp"

HidMesh::HidMeshNodePackagesCachesStorage::HidMeshNodePackagesCachesStorage()
{
    memset( &cache[0], 0, HID_MEST_PACKAGES_CACHE_SIZE );
}

bool HidMesh::HidMeshNodePackagesCachesStorage::isCached( const HidMeshPackage * package )
{
    for (size_t i = 0; i < HID_MEST_PACKAGES_CACHE_SIZE; ++i)
    {
        if( cache[i].crc == package->crc &&
            cache[i].src == package->src &&
            cache[i].dst == package->dst )
            {
                return true;
            }
    }

    cache[lastCachePosition].crc = package->crc;
    cache[lastCachePosition].src = package->src;
    cache[lastCachePosition].dst = package->dst;

    if(lastCachePosition >= HID_MEST_PACKAGES_CACHE_SIZE - 1) lastCachePosition = 0;
    else lastCachePosition++;

    return false;
}