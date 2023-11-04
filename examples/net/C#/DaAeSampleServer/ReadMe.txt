-------------------------------------------------------------------------
OPC DA/AE Server Solution .NET          DA/AE Sample Server Customization
-------------------------------------------------------------------------
Copyright (c) 2011-2023 Technosoftware GmbH. All rights reserved

This C# based customization for the OPC DA/AE Server Solution .NET is the 
reference sample showing a OPC DA 2.05a/3.00 and OPC AE 1.1 server 
implementation.
 
At startup items with several data types and access rights are statically 
defined. 

The RefreshThread simulates signal changes for the items
	SimulatedData.Ramp
	SimulatedData.Random
	SimulatedData.Sine
and writes the changed values into the internal cache and the generic 
server cache.
Item values written by a client are written into the local buffer only.

Files in this sample:
- ClassicBaseNodeManager.cs
    This node manager is a base class used as base node manager for the 
    OPC DA/AE Server Solution .NET. It implements the methods that are 
    called by the generic server and allows sub-classes to override 
    only the methods that they need. 
- ClassicNodeManager.cs
    The ClassicNodeManager class is a subclass of the ClassicBaseNodeManagerClass
    and is used by the generic server. It only overrides the methods is needs. 
    The name of this class (ClassicNodeManager) can't be changed because it is 
    called from the generic server executable.
- AssemblyInfo.cs
    Standard .NET assembly definitions.

Post Build Steps
After a successful compilation the following steps are executed in the post 
build event:
1.  copy the file OpcNetDaAeServer*.exe from the project directory 
    into the bin directory

Debugging
To debug the plug-in assembly you need to:
1. Register the server by executing OpcNetDaAeServer*.exe -regserver
   The server is now registered and can be accessed by OPC 2.05a/3.00 
   and OPC AE 1.00/1.10 clients.
2. Open project properties and select  
   Configuration Properties  -  Debugging
   Select 'Start external program' and browse to OpcNetDaAeServer*.exe
   in bin
2.  Set Breakpoints
3.  Start the program execution

The generic server is initialized and calls the plug-in methods 

	OnGetDaServerRegistryDefinition()
	OnGetDaServerParameters() 
	OnCreateServerItems() 
	
Then the server is idle until a client connects.

The further activity depends on the plug-in and the client access.


