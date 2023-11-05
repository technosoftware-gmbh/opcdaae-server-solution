## OPC DA/AE Server Solution

### Introduction
The OPC DA/AE Server Solution offers a fast and easy access to the OPC Data Access (DA) and OPC Alarms&Events (AE) technology. Develop OPC DA 2.05a, 3.00 00 and OPC AE 1.00, 1.10 compliant Servers with with Visual Studio 2022 C++ and either

- generating one server executable without the use of any DLLs.
- generating a .NET 4.8 assembly. This results in a generic server executable plus a .NET 4.8 assembly.

The developer can concentrate on his application and servers can be developed fast and easily without the need to spend a lot of time learning how to implement the OPC specifications. The server API is easy to use and many OPC specific functions, e.g. creating a group or adding an item to a group are handled by the framework. Even the complex asynchronous read/write handling is handled by the framework.

The “Framework” includes all OPC DA 2.05a, 3.00 and OPC AE 1.00, 1.10 handling and ensures the OPC compliance. It is implemented as a generic C++ based executable.

The “Server API” defines easy to use interface functions required for developing OPC DA/AE compliant servers. The OPC server is supplied as an EXE file with full C++ source code and the application adaptation part in 1 file. This imposes some limitations on the adaptation possibilities but makes the adaptation much easier and quicker. By using this API OPC servers can be easily implemented by adapting just a few functions, e.g. there are only 5 functions that have to be implemented for an OPC DA Server. The functions handle the configuration of the server and the read/write functionality of items.

The OPC DA/AE Server Solution offers unique features for performance and functionality improvements of the developed OPC Server like Event Driven Mode for Device Access; Dynamic address space with items added when they are first accessed by a client and removed when they are no longer in use; Item browsing can be implemented to browse the cache or the device/database.

## Online Help

We provide an online help for the current version: 

- [OPC DA/AE Server Solution C++ Online Help](https://technosoftware.com/help/opc-daae-server-solution-cpp/30/)
- [OPC DA/AE Server Solution .NET Online Help](https://technosoftware.com/help/opc-daae-server-solution-net/30/)

### Licenses
TECHNOSOFTWARE provides different licenses depending on the component and on the ownership of a purchased license of the user of the sources. A single ZIP file or a single repository can contain multiple components where the sources have different license models. The valid license is in the header of each source file.

See [LICENSE.md](LICENSE.md) for more details.

### Get Perpetual License without Support

The OPC DA/AE Server One-time fee for lifetime SCLA 1.0 license is available at

 * [OPC DA/AE Server](https://technosoftware.com/product/opc-daae-server/?attribute_pa_license=scla-10)

### Get Support for the Solution under GPL 3.0 or SCLA 1.0

Support for the Solution under the GPL 3.0 or SCLA 1.0 is available [here](https://github.com/technosoftware-gmbh/opc-daae-server-solution/issues).

Please be aware that there is no obligation that Technosoftware will provide maintenance, support or training under GPL 3.0 or SCLA 1.0.

### Contribution

Technosoftware has no plans to add features and fixes will only be done if time allows it. 

Therefore we strongly encourage community participation and contribution to this project. First, please fork the repository and commit your changes there. Once happy with your changes you can generate a 'pull request'.

You must agree to the contributor license agreement before we can accept your changes. The CLA and "I AGREE" button is automatically displayed when you perform the pull request. You can preview CLA [here](https://cla-assistant.io/technosoftware-gmbh/opc-daae-server-solution).
