namespace cpp wind
namespace java org.wind
namespace csharp org.wind

include "UUID.thrift"

enum Instructions
{
	Start,
	Stop,
	ResetAll,
	ResetBackground,
	ResetForeground,
	Freeze,
	Thaw,
	Other // Needs command data for further explanation
}

struct Command
{
	1: UUID.UUID componentId; // Who is the command going to?
	2: Instructions instruction;  // The command value
	3: optional string commandData;  // Optional additional data associated with the command
}