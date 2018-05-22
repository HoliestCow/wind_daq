namespace cpp wind
namespace java org.wind
namespace csharp org.wind

/** Representation of a 2d or 3d vector.
  Units will be defined where it is used.
*/
struct Vector
{
  1: double x;
  2: double y;
  3: optional double z;
}

struct Quaternion
{
  1: double w;
  2: double x;
  3: double y;
  4: double z;
}

/** Standard Euler angle representation for vessels and airplanes.
 * All units are degrees.
 * See https://en.wikipedia.org/wiki/Euler_angles#Tait%E2%80%93Bryan_angles
 */
struct NauticalAngles
{
  1: double yaw;
  2: double pitch;
  3: double roll;
}

enum AlgorithmDetectorType
{  
   GAMMA,
   NEUTRON,
   GAMMA_NEUTRON
}

