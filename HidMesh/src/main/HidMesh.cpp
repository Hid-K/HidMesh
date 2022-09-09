#include "./HidMesh.hpp"

uint8_t HidMesh::calculateCRC(const uint8_t * data, size_t dataLen)
{
  uint8_t crc = 0;
 
  for (size_t i = 0; i < dataLen; i++)
  {
    crc ^= data[i];
    for ( size_t j = 0; j < 8; j++)
    {
      if (crc & 1)
        crc ^= CRC7_POLY;
      crc >>= 1;
    }
  }
  return crc;
}

HidMesh::HidMeshPackage HidMesh::buildPackage(
    HidMesh::UNIT_ID src,
    HidMesh::UNIT_ID dst,
    uint8_t ttl,
    uint8_t payload[ HID_MESH_PACKAGE_PAYLOAD_SIZE ]
)
{
    HidMeshPackage result;

    result.version = HID_MESH_PROTOCOL_VERSION;

    result.src = src;
    result.dst = dst;

    result.crc = 0;
    result.ttl = 0;

    memcpy( &result.payload[0], &payload[0], HID_MESH_PACKAGE_PAYLOAD_SIZE );

    result.crc = calculateCRC( ( ( uint8_t * ) &result ), sizeof( HidMeshPackage ) );
    result.ttl = ttl;

    return result;
}

bool HidMesh::checkPackageCRC( HidMesh::HidMeshPackage package )
{
    uint8_t crcInPackage = package.crc;

    package.crc = 0;
    package.ttl = 0;

    return calculateCRC( ( ( uint8_t * ) &package ), sizeof( HidMesh::HidMeshPackage ) ) == crcInPackage;
}
