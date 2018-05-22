namespace cpp wind
namespace java org.wind
namespace csharp org.wind

struct CylindricalDimensions
{
        /** Centimeters */
        1: double radius;
        /** Centimeters */
        2: double length;
}

struct SphericalDimensions
{
        /** Centimeters */
        1: double radius;
}

struct RectangularDimensions
{
        /** Centimeters */
        1: double depth;
        /** Centimeters */
        2: double width;
        /** Centimeters */
        3: double length;
        //here depth=x, width=y, length=z (as in Component.thrift)
}

union Dimensions
{
        1: CylindricalDimensions cylindricalDimensions;
        2: SphericalDimensions sphericalDimensions;
        3: RectangularDimensions rectangularDimensions;
}


