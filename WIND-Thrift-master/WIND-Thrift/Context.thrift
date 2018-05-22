namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "Common.thrift"
include "Navigation.thrift"

// FIXME this needs a contract explaining how it is used.
struct MapOrigin
{
   // Relative origin of the frame in meters
   1: optional Common.Vector originPosition
   2: optional Navigation.Location originLocation

   // Orientation of the point cloud axis
   // FIXME units (deg or radians).
   // FIXME is this relative to current direction or absolute.
   3: optional double heading 
}

struct PointCloudData
{
    1: list<i32> x;                  // X values in fixed-point coordinates
    2: list<i32> y;                  // Y values in fixed-point coordinates
    3: list<i32> z;                  // Z values in fixed-point coordinates
    4: double divisor;               // Divide X/Y/Z values by this to obtain values in meters
    5: optional list<i16> red;       // Red values (0-255)
    6: optional list<i16> green;     // Green values (0-255)
    7: optional list<i16> blue;      // Blue values (0-255)
    8: optional list<i16> intensity; // Intensity values (0-255)
}
struct PointCloud
{
    1: PointCloudData pointData;     // 3D point data
/*
    2: double originX;               // Relative location of the origin in meters from the start pose
    3: double originY;
    4: double originZ;
    5: optional double originLat;    // Geographic location of the origin (WGS-84)
    6: optional double originLon;
    7: optional double originAlt;
    8: optional double heading;      // Orientation of the point cloud
*/
    9: MapOrigin origin
}


struct VoxelMapData
{
    1: list<i32> x;                  // Voxel position in X
    2: list<i32> y;                  // Voxel position in Y
    3: list<i32> z;                  // Voxel position in Z
    4: optional list<i16> red;       // Red values (0-255)
    5: optional list<i16> green;     // Green values (0-255)
    6: optional list<i16> blue;      // Blue values (0-255)
    7: optional list<i16> intensity; // Intensity values (0-255)
}
struct VoxelMap
{
    1: i32 sizeX;                    // Size of the voxel map in the x dimension
    2: i32 sizeY;                    // Size of the voxel map in the y dimension
    3: i32 sizeZ;                    // Size of the voxel map in the z dimension
    4: double edgeLength;            // Physical size of each voxel in meters
    5: VoxelMapData voxelData;       // Occupied Voxels
/*
    6: double originX;               // Relative location of the origin in meters from the start pose
    7: double originY;
    8: double originZ;
    9: optional double originLat;    // Geographic location of the origin (WGS-84)
    10: optional double originLon;
    11: optional double originAlt;
    12: optional double heading;     // Orientation of the voxel map
*/
    13: MapOrigin origin
}


struct VertexData
{
    1: list<i32> x;       // X fixed-point coordinate
    2: list<i32> y;       // Y fixed-point coordinate
    3: list<i32> z;       // Z fixed-point coordinate
    4: double divisor;    // Divide coordinates by this to obtain values in meters
}
struct TriangleVertexData
{
    1: list<i32> vertexIndex;          // Vertex index
    2: optional list<i32> textureU;    // Texture U fixed-point coordinate
    3: optional list<i32> textureV;    // Texture V fixed-point coordinate
    4: optional double divisor;        // Divide texture coordinates by this to obtain values in texels
}
struct TriangleData
{
    1: TriangleVertexData vertex1;    // Data for the first vertex
    2: TriangleVertexData vertex2;    // Data for the second vertex
    3: TriangleVertexData vertex3;    // Data for the third vertex
    6: optional list<i16> red;        // Face color - red (0-255)
    7: optional list<i16> green;      // Face color - green (0-255)
    8: optional list<i16> blue;       // Face color - blue (0-255)
    9: optional list<i16> intensity;  // Face color - intensity (0-255)
}
struct MeshData
{
    1: VertexData vertices;
    2: TriangleData triangles;
}
struct Mesh
{
    1: MeshData meshData;          // Triangular mesh elements composing a fully constructed 3D mesh object
    2: optional binary texture;    // Texture image. Use header to determine format/codec
/*
    3: double originX;             // Relative location of the origin in meters from the start pose
    4: double originY;
    5: double originZ;
    6: optional double originLat;  // Geographic location of the origin (WGS-84)
    7: optional double originLon;
    8: optional double originAlt;
    9: optional double heading;    // Orientation of the mesh
*/
    10: MapOrigin origin
}

struct VelodyneDataBlock
{
    1: i32 azimuth;           // Hundredths of a degree
    2: list<i32> distance;    // 32 measurements per block from 2 sets of laser firings, 2mm increments
    3: list<i16> reflectance; // Diffuse reflectors 0-100 map to 0-100%
                              //   Retro reflectors 101-255; 255 for perfect retro-reflector
}
struct VelodyneDataPacket
{
    1: i64 timeStampUs;                    // Microseconds since the top of the hour
    2: list<VelodyneDataBlock> dataBlocks; // Range data - 12 blocks per packet
    3: i32 factory;                        // Factory value indicating mode
    4: i64 receivedTime;                   // POSIX Time * 1000, when the packet was received by the system
}
struct VelodyneData
{
    1: list<VelodyneDataPacket> packets;   // Data packets
    2: list<i32> elevation;                // Elevation values in hundredths of degrees. 
                                           //   These are the same for every data block.
}
