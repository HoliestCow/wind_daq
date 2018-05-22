namespace cpp wind
namespace java org.wind
namespace csharp org.wind

struct AngleEfficiencyPair
{
  /**
   * Angle is counter clockwise from the direction travel (front of person) 
   *  units are radians
   */
  1: double angle;

  /** efficiency should be between 0 and 1. */
  2: double efficiency;
}

/** Angular Efficiency is needed to compute the direction of a source */
struct AngularEfficiencyDefinition
{
  /** what energy this applies to */
  1: double energy;  

  /**
   * Polar efficiency map
   *   - points in between should be interpolated
   *   - both 0 and 2pi should appear in the list
   */
  2: list<AngleEfficiencyPair> efficiency;
}