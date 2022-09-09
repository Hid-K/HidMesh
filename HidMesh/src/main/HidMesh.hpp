#ifndef HID_MESH_H
#define HID_MESH_H

#include <stdint.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <pthread.h>
#include "unistd.h"
#include <stdint.h>
#include <iostream>
#include <chrono>
#include <thread>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

namespace HidMesh
{
    typedef uint64_t UNIT_ID;

    const uint8_t HID_MESH_PROTOCOL_VERSION = 0;

    const uint64_t HID_MESH_PACKAGE_PAYLOAD_SIZE = 255;

    const UNIT_ID BROADCAST_PACKAGE_DST = 0;

    const unsigned char CRC7_POLY = 0x91;

    typedef struct 
    {
        uint8_t version;
        uint8_t crc;

        uint8_t ttl;

        UNIT_ID src;
        UNIT_ID dst;

        uint8_t payload[HID_MESH_PACKAGE_PAYLOAD_SIZE];
    } HidMeshPackage;


    uint8_t calculateCRC( const uint8_t * data, size_t dataLen );

    HidMeshPackage buildPackage(
        UNIT_ID src,
        UNIT_ID dst,
        uint8_t ttl,
        uint8_t payload[255]
    );

    bool checkPackageCRC( HidMeshPackage package );

    class HidMeshNodePackageCache
    {
        public:
            UNIT_ID src;
            UNIT_ID dst;

            uint8_t crc;
    };

    class HidMeshNodePackagesCachesStorage
    {
        #define HID_MEST_PACKAGES_CACHE_SIZE 255
        private:
            HidMeshNodePackageCache cache[HID_MEST_PACKAGES_CACHE_SIZE];
            size_t lastCachePosition = 0;

        public:
            HidMeshNodePackagesCachesStorage();

            /**
             * @brief 
             * 
             * @param package 
             * @return true if package was in cache
             * @return false if package was not in cache, but now added
             */
            bool isCached( const HidMeshPackage * package );
    };

    class NetworkNode
    {
        private:
            UNIT_ID id;
            void (*onPackage)( NetworkNode * th, const HidMeshPackage * package );
            HidMeshNodePackagesCachesStorage packagesCache;

            #ifdef HID_MESH_SIMULATION_TEST
            NetworkNode ** linkedNodes = nullptr;
            size_t linkedNodesCount;
        
            void (*microcode)( NetworkNode * th );

            std::thread microcodeThread;
            #endif
        
        public:
            #ifdef HID_MESH_SIMULATION_TEST
            NetworkNode( UNIT_ID id,
                            void (*microcode)( NetworkNode * th ),
                            void (*onPackage)( NetworkNode * th, const HidMeshPackage * package )
                            );
            #else
            NetworkNode( UNIT_ID id,
                         void (*onPackage)( NetworkNode * th, const HidMeshPackage * package ) );
            #endif

            ~NetworkNode();

            void processPackage( const HidMeshPackage * package );

            void sendDataToAllLinkedNodes( const HidMeshPackage * package );

            void sendData( UNIT_ID dst, uint8_t payload[ HID_MESH_PACKAGE_PAYLOAD_SIZE ] );
        
            void relayPackage( const HidMeshPackage * package );

            #ifdef HID_MESH_SIMULATION_TEST
                void start();
                void linkNode( NetworkNode * node );
            #endif

            UNIT_ID getId();

            HidMeshNodePackagesCachesStorage * getCache();

            NetworkNode * operator=( NetworkNode * object );
    };
}
#endif
