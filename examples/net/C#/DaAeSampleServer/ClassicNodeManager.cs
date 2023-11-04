#region Copyright (c) 2011-2023 Technosoftware GmbH. All rights reserved
//-----------------------------------------------------------------------------
// Copyright (c) 2011-2023 Technosoftware GmbH. All rights reserved
// Web: https://technosoftware.com 
// 
// License: 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#endregion Copyright (c) 2011-2023 Technosoftware GmbH. All rights reserved

#region	Using Directives
using System;
using System.Threading;
using System.Collections;
using System.Collections.Generic;
#endregion

namespace ServerPlugin
{

    /// <summary>
    /// OPC Server Configuration and IO Handling
    ///
    /// This C# based plugin for the OPC Server .NET Classic Edition shows a base 
    /// OPC 2.05a / 3.00 and OPC AE 1.10 server implementation  
    /// At startup items with several data types and access rights are statically 
    /// defined. 
    /// The RefreshThread simulates signal changes for the items
    ///      SimulatedData.Ramp
    ///      SimulatedData.Random
    ///      SimulatedData.Sine
    /// and writes the changed values into the internal cache. The generic server
    /// read the item values from device as required through calling the function 
    /// ReadItem().
    /// Item values written by a client are written into the local buffer only.
    /// </summary>
    public class ClassicNodeManager : ClassicBaseNodeManager
    {
        #region Constants
        #region Data Access IDs
        const int PropertyIdCasingMaterial = 5650;
        const int PropertyIdCasingHeight = 5651;
        const int PropertyIdCasingManufacturer = 5652;
        #endregion

        #region Event Category IDs
        // Simple Event Category IDs
        const int CategoryIdDeviceFailure = 0x100;
        const int CategoryIdSysMessage = 0x101;

        // Tracking Event Category IDs   
        const int CatIdSysConfig = 0x200;
        const int CatidAdvControl = 0x201;

        // Condition Event Category IDs  
        const int CatIdLevel = 0x300;
        const int CatIdSysFail = 0x301;
        #endregion

        #region Attribute IDs
        const int AttrIdLevelCv = 0x400;
        const int AttrIdDevFailureErrorCode = 0x401;
        const int AttrIdDevFailureDeviceName = 0x402;
        const int AttrIdSysConfigPrevValue = 0x403;
        const int AttrIdSysConfigNewValue = 0x404;
        const int AttrIdAdvControlPrevValue = 0x405;
        const int AttrIdAdvControlNewValue = 0x406;
        #endregion

        #region Condition Definition IDs
        const int CondDefIdPvLevelRamp = 0x500;
        const int CondDefIdPvLevelFurnace = 0x501;
        const int CondDefIdHiLevelTank = 0x502;
        const int CondDefIdHiLevelHeating = 0x503;
        const int CondDefIdSysFailTemp = 0x504;
        const int CondDefIdSysFailHumi = 0x505;
        #endregion

        #region Sub-Condition Definition IDs
        const int SubCondDefIdLoLoRamp = 0x550;
        const int SubCondDDefIdLoRamp = 0x551;
        const int SubCondDefIdHiRamp = 0x552;
        const int SubCondDefIdHiHiRamp = 0x553;

        const int SubCondDefIdLoFurnace = 0x554;
        const int SubCondDefIdHiFurnace = 0x555;
        #endregion

        #region Area IDs
        const int AreaIdRoot = unchecked((int)(0xFFFFFFFE));
        //const int AreaIdUnspecified = unchecked((int)(0xFFFFFFFD));

        const int AreaIdNorth = 0x600;
        const int AreaIdNorthDevice = 0x601;
        const int AreaIdSouth = 0x602;
        const int AreaIdSouthDevice = 0x603;
        #endregion

        #region Source IDs
        const int SourceIdSystem = 0x700;
        const int SourceIdNetworkAdapter = 0x701;
        const int SourceIdSerialPort = 0x702;
        const int SourceIdValve = 0x703;
        const int SourceIdMotor = 0x704;
        const int SourceIdTank1 = 0x705;
        const int SourceIdTank2 = 0x706;
        const int SourceIdHeating1 = 0x707;
        const int SourceIdHeating2 = 0x708;
        const int SourceIdMultiSource = 0x709;
        #endregion

        #region Event Condition IDs
        const int ConditionIdTank1Overflow = 0x800;
        const int ConditionIdTank2Overflow = 0x801;
        const int ConditionIdHeating1ExtTemp = 0x802;
        const int ConditionIdHeating2ExtTemp = 0x803;
        const int ConditionIdWaterLevel = 0x804;
        #endregion
        #endregion

        #region Fields
        // Simulated Data Items
        private static MyItem myDynamicRampItem_;
        private static MyItem myDynamicSineItem_;
        private static MyItem myDynamicRandomItem_;
        private static MyItem myDynamicSetItemValueCallsPerSecondItem_;

        private static MyItem myShutdownRequestItem_;

        private static readonly int NumberMassItems = 50000;
        #endregion

        #region Static Fields
        static private Thread myThread_;
        static private ManualResetEvent stopThread_;

        static private bool tank1Active_;
        static private Int32 levelValue_ = 50;
        #endregion

        #region Signal State Data
        // DATA DEFINITIONS
        // Important: All data needs to be defined as STATIC.
        // This is important because this class is used in multiple instances.

        private static readonly Dictionary<IntPtr, MyItem> items_ = new Dictionary<IntPtr, MyItem>();
        private static readonly Dictionary<IntPtr, MyItem> dynamicItems_ = new Dictionary<IntPtr, MyItem>();
        #endregion

        #region General Methods (not related to an OPC specification)

        #region  .NET API Generic Server Default Methods
        //---------------------------------------------------------------------
        //  .NET API Methods 
        // (Called by the generic server)
        //---------------------------------------------------------------------

        /// <inheritdoc/>
        public override int OnGetLogLevel()
        {
            return (int)LogLevel.Info;
        }

        /// <inheritdoc/>
        public override string OnGetLogPath()
        {
            return "";
        }

        /// <inheritdoc/>
        public override int OnCreateServerItems()
        {
            #region Alarms&Events address space

            // Order of implemented steps

            // 1) Define the Event Categories
            // 2) Add the Attributes to the Event Categories if desired
            // 3) Specify the Condition Definitions
            // 4) Specify the Sub-Condition Definitions
            // 5) Define the Process Areas (is optional)
            // 6) Define the Event Sources
            // 7) Define the Event Conditions

            #region 1) Define the Event Categories

            AddSimpleEventCategory(CategoryIdDeviceFailure, "Device Failure");
            AddSimpleEventCategory(CategoryIdSysMessage, "System Message");

            AddTrackingEventCategory(CatIdSysConfig, "System Configuration");
            AddTrackingEventCategory(CatidAdvControl, "Advanced Control");

            AddConditionEventCategory(CatIdLevel, "Level");
            AddConditionEventCategory(CatIdSysFail, "System Failure");

            #endregion

            #region 2) Add the Attributes to the Event Categories

            AddEventAttribute(CatIdLevel, AttrIdLevelCv, "Current Value", (Int32)0);
            AddEventAttribute(CategoryIdDeviceFailure, AttrIdDevFailureErrorCode, "Error Code", (Int32)0);
            AddEventAttribute(CategoryIdDeviceFailure, AttrIdDevFailureDeviceName, "Device Name", (string)"");
            AddEventAttribute(CatIdSysConfig, AttrIdSysConfigPrevValue, "Prev Value", (Int32)0);
            AddEventAttribute(CatIdSysConfig, AttrIdSysConfigNewValue, "New Value", (Int32)0);
            AddEventAttribute(CatidAdvControl, AttrIdAdvControlPrevValue, "Prev Value", (Int32)0);
            AddEventAttribute(CatidAdvControl, AttrIdAdvControlNewValue, "New Value", (Int32)0);

            #endregion

            #region 3) Specify the Condition Definitions

            AddSingleStateConditionDefinition(CatIdSysFail, CondDefIdSysFailTemp,
                "SYSTEM_FAILURE Temperature", "temp > 100�C", 100, "Excess Temperature", false);
            AddSingleStateConditionDefinition(CatIdSysFail, CondDefIdSysFailHumi,
                "SYSTEM_FAILURE Humidity", "humidity > 80%", 100, "Humidity too high", false);
            AddSingleStateConditionDefinition(CatIdLevel, CondDefIdHiLevelTank,
                "HI Tank", "level > 80", 100, "Overflow", true);
            AddSingleStateConditionDefinition(CatIdLevel, CondDefIdHiLevelHeating,
                "HI Heating Temperature", "temp > 35", 100, "Excess Temperature", false);
            AddMultiStateConditionDefinition(CatIdLevel, CondDefIdPvLevelRamp, "PVLEVEL Ramp");
            AddMultiStateConditionDefinition(CatIdLevel, CondDefIdPvLevelFurnace, "PVLEVEL Furnace");

            #endregion

            #region 4) Specify the Sub-Condition Definitions

            AddSubConditionDefinition(CondDefIdPvLevelRamp, SubCondDefIdLoLoRamp,
                "LO_LO", "Ramp < 15", 400, "Low Low Alarm", false);
            AddSubConditionDefinition(CondDefIdPvLevelRamp, SubCondDDefIdLoRamp,
                "LO", "Ramp < 25", 100, "Low Alarm", false);
            AddSubConditionDefinition(CondDefIdPvLevelRamp, SubCondDefIdHiRamp,
                "HI", "Ramp > 75", 100, "High Alarm", false);
            AddSubConditionDefinition(CondDefIdPvLevelRamp, SubCondDefIdHiHiRamp,
                "HI_HI", "Ramp > 85", 400, "High High Alarm", false);


            AddSubConditionDefinition(CondDefIdPvLevelFurnace, SubCondDefIdLoFurnace,
                "LO", "Temp < 500", 100, "Low Alarm", false);
            AddSubConditionDefinition(CondDefIdPvLevelFurnace, SubCondDefIdHiFurnace,
                "HI", "Temp > 800", 100, "High Alarm", false);

            #endregion

            #region 5) Define the Process Areas

            AddArea(AreaIdRoot, AreaIdNorth, "PlantNorth");
            AddArea(AreaIdNorth, AreaIdNorthDevice, "Device1");

            AddArea(AreaIdRoot, AreaIdSouth, "PlantSouth");
            AddArea(AreaIdSouth, AreaIdSouthDevice, "Device1");

            #endregion

            #region 6) Define the Event Sources

            AddSource(AreaIdRoot, SourceIdNetworkAdapter, "Network Adapter", false);
            AddSource(AreaIdRoot, SourceIdSerialPort, "Serial Port", false);
            AddSource(AreaIdRoot, SourceIdSystem, "System", false);
            AddSource(AreaIdNorthDevice, SourceIdValve, "Valve", false);
            AddSource(AreaIdSouthDevice, SourceIdMotor, "Motor", false);
            AddSource(AreaIdRoot, SourceIdTank1, "Level Sensor Tank 1", false);
            AddSource(AreaIdRoot, SourceIdTank2, "Level Sensor Tank 2", false);
            AddSource(AreaIdRoot, SourceIdHeating1, "Heating 1", false);
            AddSource(AreaIdRoot, SourceIdHeating2, "Heating 2", false);

            AddSource(AreaIdRoot, SourceIdMultiSource, "Multiple Used Source", true);
            AddExistingSource(AreaIdNorthDevice, SourceIdMultiSource);
            AddExistingSource(AreaIdSouthDevice, SourceIdMultiSource);

            #endregion

            #region 7) Define the Event Conditions

            AddCondition(SourceIdMultiSource, CondDefIdHiLevelTank, ConditionIdTank1Overflow);
            AddCondition(SourceIdTank2, CondDefIdHiLevelTank, ConditionIdTank2Overflow);
            AddCondition(SourceIdHeating1, CondDefIdHiLevelHeating, ConditionIdHeating1ExtTemp);
            AddCondition(SourceIdHeating2, CondDefIdHiLevelHeating, ConditionIdHeating2ExtTemp);
            AddCondition(SourceIdTank1, CondDefIdPvLevelRamp, ConditionIdWaterLevel);

            #endregion

            #endregion

            #region Data Access address space

            CreateServerAddressSpace();

            #endregion

            // create a thread for simulating signal changes
            // in real application this thread reads from the device
            myThread_ = new Thread(RefreshThread) { Name = "Device Simulation", Priority = ThreadPriority.AboveNormal };
            myThread_.Start();

            return StatusCodes.Good;
        }

        /// <inheritdoc/>
        public override void OnShutdownSignal()
        {
            //////////////////  TO-DO  /////////////////
            // close the device communication

            // terminate the simulation thread
            stopThread_ = new ManualResetEvent(false);
            stopThread_.WaitOne(5000, true);
            stopThread_.Close();
            stopThread_ = null;
        }

        /// <inheritdoc/>
        public override int OnGetDaServerParameters(out int updatePeriod, out char branchDelimiter, out DaBrowseMode browseMode)
        {
            // Default Values
            updatePeriod = 100;                             // ms
            branchDelimiter = '.';
            browseMode = DaBrowseMode.Generic;            // browse the generic server address space
            return StatusCodes.Good;
        }
        #endregion
        #endregion

        #region Data Access related Methods
        #region  .NET API Generic Server Default Methods
        //---------------------------------------------------------------------
        //  .NET API Methods 
        // (Called by the generic server)
        //---------------------------------------------------------------------

        /// <inheritdoc/>
        public override ClassicServerDefinition OnGetDaServerDefinition()
        {
            DaServer = new ClassicServerDefinition
            {
                ClsIdApp = "{9236F2A6-96EA-4D44-8C42-3A6DDA061BC6}",
                CompanyName = "Technosoftware GmbH",
                ClsIdServer = "{9B59C648-8FA5-4BBA-9686-7CDB5041C456}",
                PrgIdServer = "OpcNetDaAe.DaSample",
                PrgIdCurrServer = "OpcNetDaAe.DaSample.30",
                ServerName = "OPC Server Solution .NET DA Sample Server",
                CurrServerName = "OPC Server Solution .NET DA/AE Sample Server V3.0"
            };

            return DaServer;
        }

        /// <inheritdoc/>
        public override int OnQueryProperties(
            IntPtr deviceItemHandle,
            out int noProp,
            out int[] iDs)
        {
            if (items_.TryGetValue(deviceItemHandle, out MyItem item))
            {
                if (item.ItemProperties != null)
                {
                    // item has  custom properties
                    noProp = item.ItemProperties.Length;
                    iDs = new int[noProp];
                    for (int i = 0; i < noProp; ++i)
                    {
                        iDs[i] = item.ItemProperties[i].PropertyId;
                    }
                    return StatusCodes.Good;
                }
            }
            noProp = 0;
            iDs = null;
            return StatusCodes.Bad;
        }

        /// <inheritdoc/>
        public override int OnGetPropertyValue(IntPtr deviceItemHandle, int propertyId, out object propertyValue)
        {
            if (items_.TryGetValue(deviceItemHandle, out MyItem item))
            {
                if (item.ItemProperties != null)
                {

                    int numProp = item.ItemProperties.Length;
                    for (int i = 0; i < numProp; ++i)
                    {
                        if (item.ItemProperties[i].PropertyId == propertyId)
                        {
                            propertyValue = item.ItemProperties[i].PropertyValue;
                            return StatusCodes.Good;
                        }
                    }
                }
            }
            // Item property is not available
            propertyValue = null;
            return StatusCodes.BadInvalidPropertyId;
        }

        /// <inheritdoc/>
        public override int OnWriteItems(DaDeviceItemValue[] values, out int[] errors)
        {
            errors = new int[values.Length];                            // result array
            for (int i = 0; i < values.Length; ++i)                     // init to Good
                errors[i] = StatusCodes.Good;

            // TO-DO: write the new values to the device
            foreach (DaDeviceItemValue t in values)
            {
                if (items_.TryGetValue(t.DeviceItemHandle, out MyItem item))
                {
                    // Only if there is a Value specified write the value into buffer
                    if (t.Value != null)
                        item.Value = t.Value;
                    if (t.QualitySpecified)
                        item.Quality = new DaQuality(t.Quality);
                    if (t.TimestampSpecified)
                        item.Timestamp = t.Timestamp;

                    if (item == myShutdownRequestItem_)
                        FireShutdownRequest((String)item.Value);

                }
            }
            return StatusCodes.Good;
        }
        #endregion
        #endregion

        #region Alarms&Events related Methods
        #region  .NET API Generic Server Default Methods
        /// <inheritdoc/>
        public override ClassicServerDefinition OnGetAeServerDefinition()
        {
            AeServer = new ClassicServerDefinition
            {
                ClsIdApp = "{9236F2A6-96EA-4D44-8C42-3A6DDA061BC6}",
                CompanyName = "Technosoftware GmbH",
                ClsIdServer = "{DD2E86BD-266A-43F9-BDFE-3A0B40B94C20}",
                PrgIdServer = "OpcNetDaAe.AeSample",
                PrgIdCurrServer = "OpcNetDaAe.AeSample.30",
                ServerName = "OPC Server Solution .NET AE Sample Server",
                CurrServerName = "OPC Server Solution .NET DA/AE Sample Server V3.0"
            };

            return AeServer;
        }
        #endregion
        #endregion

        #region Create Data Access Sample Variants
        /// <summary>
        /// Utility function to create the sample values for the server items.
        /// </summary>
        /// <param name="itemType"></param>
        /// <param name="isArray"></param>
        /// <param name="itemValue"></param>
        internal void CreateSampleVariant(
            Type itemType,
            bool isArray,
            out object itemValue)
        {
            #region Data Access address space

            var rand = new Random();

            if (isArray)
            {
                //
                // Array Type
                //
                string bstr = null;
                if (itemType == typeof(string[]))
                {
                    bstr = "This is string #";
                }

                var itemList = new ArrayList();

                for (int i = 0; i < 4; i++)
                {
                    if (itemType == typeof(bool))
                    {
                        itemList.Add(i % 2 == 1);
                    }
                    else if (itemType == typeof(sbyte))
                    {
                        var d = new byte[1];
                        rand.NextBytes(d);
                        itemList.Add((sbyte)d[0]);
                    }
                    else if (itemType == typeof(short))
                    {
                        var d = new byte[1];
                        rand.NextBytes(d);
                        itemList.Add((short)d[0]);
                    }
                    else if (itemType == typeof(int))
                    {
                        itemList.Add(rand.Next() * 100);
                    }
                    else if (itemType == typeof(Int64))
                    {
                        itemList.Add((Int64)rand.Next() * 100);
                    }
                    else if (itemType == typeof(UInt64))
                    {
                        itemList.Add((UInt64)rand.Next() * 100);
                    }
                    else if (itemType == typeof(byte))
                    {
                        var d = new byte[1];
                        rand.NextBytes(d);
                        itemList.Add(d[0]);
                    }
                    else if (itemType == typeof(ushort))
                    {
                        var d = new byte[1];
                        rand.NextBytes(d);
                        itemList.Add((ushort)d[0]);
                    }
                    else if (itemType == typeof(uint))
                    {
                        itemList.Add((uint)rand.NextDouble() * 100);
                    }
                    else if (itemType == typeof(float))
                    {
                        itemList.Add((float)rand.NextDouble() * 100);
                    }
                    else if (itemType == typeof(double))
                    {
                        itemList.Add(rand.NextDouble() * 100);
                    }
                    else if (itemType == typeof(DateTime))
                    {
                        itemList.Add(new DateTime(rand.Next()));
                    }
                    else if (itemType == typeof(string))
                    {
                        itemList.Add(bstr + i);
                    }
                    else
                        itemList.Add(null);    // not supported type
                }
                itemValue = itemList.ToArray(itemType);
            }
            else
            {
                //
                // Simple Type
                //
                if (itemType == typeof(sbyte)) itemValue = (sbyte)76;
                else if (itemType == typeof(byte)) itemValue = (byte)23;
                else if (itemType == typeof(short)) itemValue = (short)345;
                else if (itemType == typeof(ushort)) itemValue = (ushort)39874;
                else if (itemType == typeof(int)) itemValue = 20196;
                else if (itemType == typeof(Int64)) itemValue = Int64.MinValue;
                else if (itemType == typeof(UInt64)) itemValue = UInt64.MaxValue;
                else if (itemType == typeof(uint)) itemValue = (uint)4230498;
                else if (itemType == typeof(float)) itemValue = (float)8.123242;
                else if (itemType == typeof(double)) itemValue = 83289.48243;
                else if (itemType == typeof(DateTime)) itemValue = new DateTime(1900, 1, 1, 12, 0, 0);
                else if (itemType == typeof(bool)) itemValue = false;
                else if (itemType == typeof(string)) itemValue = "-- It's a nice day --";
                else itemValue = null;
            }

            #endregion

        }

        internal class StructItemIDs
        {
            public StructItemIDs(string itemId, Type type)
            { ItemId = itemId; ItemType = type; }

            public string ItemId { get; set; }

            public Type ItemType { get; set; }
        }

        internal class StructIoTypes
        {
            public StructIoTypes(string branch, DaAccessRights accessRights)
            { Branch = branch; AccessRights = accessRights; }

            public string Branch { get; set; }

            public DaAccessRights AccessRights { get; set; }
        }

        /// <summary>
        /// Create all items supported by this server
        /// </summary>
        /// <returns></returns>
        private void CreateServerAddressSpace()
        {
            var arrayItems =
                new[]{
									   new StructItemIDs( "Short",        typeof(short)  ),
									   new StructItemIDs( "Integer",      typeof(int)    ),
									   new StructItemIDs( "Int64",        typeof(Int64)  ),
									   new StructItemIDs( "UInt64",       typeof(UInt64)  ),
									   new StructItemIDs( "SingleFloat",  typeof(float)  ),
									   new StructItemIDs( "DoubleFloat",  typeof(double) ),
									   new StructItemIDs( "String",       typeof(string) ),
									   new StructItemIDs( "Byte",         typeof(byte)   ),
									   new StructItemIDs( "Character",    typeof(sbyte)  ),
									   new StructItemIDs( "Word",         typeof(ushort) ),
									   new StructItemIDs( "DoubleWord",   typeof(uint)   ),
									   new StructItemIDs( "Boolean",      typeof(bool)   ),
									   new StructItemIDs( "DateTime",     typeof(DateTime)   ),
									   new StructItemIDs( null,           null        ) };

            var ioTypes =
                new[] {
										new StructIoTypes( "In",    DaAccessRights.Readable      ),
										new StructIoTypes( "Out",   DaAccessRights.Writable      ),
										new StructIoTypes( "InOut", DaAccessRights.ReadWritable  ),
										new StructIoTypes( null, 0                    ) };
            int z;
            MyItem myItem;

            //------------------------------------------------------------
            // SimpleTypes In/Out/InOut
            //------------------------------------------------------------
            int i = 0;
            while (ioTypes[i].Branch != null)
            {
                z = 0;
                while (arrayItems[z].ItemId != null)
                {
                    CreateSampleVariant(arrayItems[z].ItemType, false, out object initialItemValue);

                    var itemId = "CTT.SimpleTypes.";
                    itemId += ioTypes[i].Branch;


                    myItem = new MyItem(itemId + "." + arrayItems[z].ItemId, initialItemValue);

                    AddItem(itemId + "." + arrayItems[z].ItemId,
                            ioTypes[i].AccessRights, initialItemValue, out myItem.DeviceItemHandle);
                    items_.Add(myItem.DeviceItemHandle, myItem);
                    z++;
                }
                i++;
            }

            //------------------------------------------------------------
            // Arrays In/Out/InOut
            //------------------------------------------------------------
            i = 0;
            while (ioTypes[i].Branch != null)
            {
                z = 0;
                while (arrayItems[z].ItemId != null)
                {
                    CreateSampleVariant(arrayItems[z].ItemType, true, out object initialItemValue);

                    var itemId = "CTT.ArrayTypes.";
                    itemId += ioTypes[i].Branch;

                    myItem = new MyItem(itemId + "." + arrayItems[z].ItemId + "[]", initialItemValue);

                    AddItem(itemId + "." + arrayItems[z].ItemId + "[]",
                            ioTypes[i].AccessRights, initialItemValue, out myItem.DeviceItemHandle);
                    items_.Add(myItem.DeviceItemHandle, myItem);
                    z++;
                }
                i++;
            }


            // SimulatedData/Ramp
            {
                const int itemValue = 0; // canonical data type

                myItem = new MyItem("SimulatedData.Ramp", itemValue);

                myDynamicRampItem_ = myItem;

                AddItem("SimulatedData.Ramp",
                        DaAccessRights.Readable, itemValue, out myItem.DeviceItemHandle);
                items_.Add(myItem.DeviceItemHandle, myItem);
            }

            // SimulatedData/Sine
            {
                const double itemValue = 0.0; // canonical data type

                myItem = new MyItem("SimulatedData.Sine", itemValue);

                myDynamicSineItem_ = myItem;

                AddItem("SimulatedData.Sine",
                        DaAccessRights.Readable, itemValue, out myItem.DeviceItemHandle);
                items_.Add(myItem.DeviceItemHandle, myItem);
            }

            // SimulatedData/Random
            {
                const int itemValue = 0; // canonical data type

                myItem = new MyItem("SimulatedData.Random", itemValue);

                myDynamicRandomItem_ = myItem;

                AddItem(myItem.ItemName, DaAccessRights.Readable, itemValue, out myItem.DeviceItemHandle);
                items_.Add(myItem.DeviceItemHandle, myItem);
            }

            // SpecialItems/WithAnalogEUInfo
            var itemPropertiesAnalog = new MyItemProperty[2];
            itemPropertiesAnalog[0] = new MyItemProperty(DaProperty.LowEu.Code, 40.86);
            itemPropertiesAnalog[1] = new MyItemProperty(DaProperty.HighEu.Code, 92.67);

            myItem = new MyItem("SpecialItems.WithAnalogEUInfo", 20.56, itemPropertiesAnalog);

            AddAnalogItem(myItem.ItemName,
                          DaAccessRights.ReadWritable, myItem.Value, 40.86, 92.67, out myItem.DeviceItemHandle);
            items_.Add(myItem.DeviceItemHandle, myItem);

            // SpecialItems/WithAnalogEUInfo
            itemPropertiesAnalog[0] = new MyItemProperty(DaProperty.LowEu.Code, 12.50);
            itemPropertiesAnalog[1] = new MyItemProperty(DaProperty.HighEu.Code, 27.90);

            myItem = new MyItem("SpecialItems.WithAnalogEUInfo2", 21.00, itemPropertiesAnalog);

            AddAnalogItem(myItem.ItemName,
                          DaAccessRights.ReadWritable, myItem.Value, 12.50, 27.90, out myItem.DeviceItemHandle);
            items_.Add(myItem.DeviceItemHandle, myItem);

            // Add Custom Property Definitions to the generic server
            AddProperty(PropertyIdCasingHeight, "Casing Height", 25.34);
            AddProperty(PropertyIdCasingMaterial, "Casing Material", "Aluminum");
            AddProperty(PropertyIdCasingManufacturer, "Casing Manufacturer", "CBM");
            AddProperty(102, "High EU", 45.86);
            AddProperty(103, "Low EU", 35.86);

            // Create custom item properties for the item
            var itemProperties = new MyItemProperty[3];
            itemProperties[0] = new MyItemProperty(PropertyIdCasingHeight, 25.45);
            itemProperties[1] = new MyItemProperty(PropertyIdCasingMaterial, "Aluminum");
            itemProperties[2] = new MyItemProperty(PropertyIdCasingManufacturer, "CBM");

            myItem = new MyItem("SpecialItems.WithVendorSpecificProperties", 1111, itemProperties);

            AddItem(myItem.ItemName,
                    DaAccessRights.ReadWritable, myItem.Value, out myItem.DeviceItemHandle);
            items_.Add(myItem.DeviceItemHandle, myItem);

            // SimulatedData/SetItemValueCallsPerSecond
            {
                const double itemValue = 0.0; // canonical data type

                myItem = new MyItem("SimulatedData.SetItemValueCallsPerSecond", itemValue);

                myDynamicSetItemValueCallsPerSecondItem_ = myItem;

                AddItem(myItem.ItemName, DaAccessRights.Readable, itemValue, out myItem.DeviceItemHandle);
                items_.Add(myItem.DeviceItemHandle, myItem);
            }

            // Commands/RequestShutdown
            {
                const String itemValue = "";               // canonical data type

                myItem = new MyItem("Commands.RequestShutdown", itemValue);

                myShutdownRequestItem_ = myItem;

                AddItem(myItem.ItemName, DaAccessRights.ReadWritable, itemValue, out myItem.DeviceItemHandle);
                items_.Add(myItem.DeviceItemHandle, myItem);
            }

            // MassItems/Random
            {
                for (int ii = 0; ii < NumberMassItems; ii++)
                {
                    const int itemValue = 0; // canonical data type

                    var itemId = "MassItems.Random_" + ii.ToString("D4");
                    myItem = new MyItem(itemId, itemValue);
                    AddItem(myItem.ItemName, DaAccessRights.Readable, itemValue, out myItem.DeviceItemHandle);
                    items_.Add(myItem.DeviceItemHandle, myItem);
                    dynamicItems_.Add(myItem.DeviceItemHandle, myItem);
                }
            }
        }
        #endregion

        #region Refresh Thread

        //=========================================================================
        // ToggleTank1Cond
        // ---------------
        //    Each call of this function changes the active state of the
        //    condition specified by ID ConditionIdTank1Overflow.
        //    This function also sets the required attribute valus and uses
        //    an own message which is used instead of the default message.
        //=========================================================================
        private void ToggleTank1Cond()
        {
            var cs = new AeConditionState[1];
            var devfailattrs = new object[1];

            cs[0] = new AeConditionState(ConditionIdTank1Overflow, 0, false, DaQuality.Good, 1, devfailattrs);

            if (tank1Active_)
            {
                tank1Active_ = false;
                cs[0].Message = "Normal State";         // Own message
                devfailattrs[0] = 80;                   // Current Value
            }
            else
            {
                tank1Active_ = true;
                cs[0].Message = "Overflow";             // Own message
                devfailattrs[0] = 123;                  // Current Value
            }
            cs[0].ActiveState = tank1Active_;           // Set current active state
            // Process the new state
            ProcessConditionStateChanges(1, cs);
        }

        private void ToggleRampCond()
        {
            var cs = new AeConditionState[1];
            var devfailattrs = new object[1];

            cs[0] = new AeConditionState(ConditionIdWaterLevel, 0, true, DaQuality.Good, 1, devfailattrs) { ActiveState = true };

            switch (levelValue_)
            {
                case 10:
                    levelValue_ = 20;
                    cs[0].SubConditionId = SubCondDDefIdLoRamp;
                    break;
                case 20:
                    levelValue_ = 50;
                    cs[0].ActiveState = false;
                    break;
                case 50:
                    levelValue_ = 80;
                    cs[0].SubConditionId = SubCondDefIdHiRamp;
                    cs[0].AckRequired = true;
                    break;
                case 80:
                    levelValue_ = 90;
                    cs[0].SubConditionId = SubCondDefIdHiHiRamp;
                    break;
                case 90:
                    levelValue_ = 10;
                    cs[0].SubConditionId = SubCondDefIdLoLoRamp;
                    break;
            }
            devfailattrs[0] = levelValue_;                        // Set required attribute

            ProcessConditionStateChanges(1, cs);
        }

        //=========================================================================
        // Heating1Condition
        // -------------
        //    This is a wrapper class for the condition with the ID
        //    ConditionIdHeating1ExtTemp.
        //    Each call of the function ToggleCondition() changes the active
        //    state of the condition  and sets the required attribute
        //    valus. Also a own message which is used instead of the default
        //    message.
        //=========================================================================
        private class Heating1Condition
        {
            readonly AeConditionState[] cs_ = new AeConditionState[1];
            private readonly object[] devfailattrs_ = new object[1];

            public Heating1Condition()
            {
                cs_[0] = new AeConditionState(ConditionIdHeating1ExtTemp, 0, false, DaQuality.Good, 1, devfailattrs_);
            }

            public void ToggleCondition()
            {
                if (cs_[0].ActiveState)
                {
                    cs_[0].ActiveState = false;
                    // Own message
                    cs_[0].Message = "Normal Temperature";
                    devfailattrs_[0] = 35;         // Current Value
                }
                else
                {
                    cs_[0].ActiveState = true;
                    // Own message
                    cs_[0].Message = "Excess Temperature";
                    devfailattrs_[0] = 55;         // Current Value
                }

                ProcessConditionStateChanges(1, cs_);
            }
        }

        // This method simulates item value changes.
        void RefreshThread()
        {
            double count = 0;
            int ramp = 0;
            var rand = new Random();
            var devfailattrs = new object[2];
            var condHeating1 = new Heating1Condition();

            devfailattrs[0] = (long)5555;                               // Error Code
            devfailattrs[1] = "3Com EtherLink XL NIC (3C900B-COMBO)";   // Device Name

            var cs = new AeConditionState[1];

            cs[0] = new AeConditionState(ConditionIdTank1Overflow, 0, false, DaQuality.Good, 1, devfailattrs) { ActiveState = true };

            // Update all used items once
            foreach (MyItem item in items_.Values)
            {
                SetItemValue(item.DeviceItemHandle, item.Value, DaQuality.Good.Code, DateTime.Now);
            }

            for (; ; )   // forever thread loop
            {
                //int numclientHandles;
                //IntPtr[] clientHandles;
                //String[] clientNames;

                //GetClients(out numclientHandles, out clientHandles, out clientNames);
                //if (numclientHandles > 0)
                //{
                //    foreach (var client in clientHandles)
                //    {
                //        int numGroupHandles;
                //        IntPtr[] groupHandles;
                //        String[] groupNames;
                //        GetGroups(client, out numGroupHandles, out groupHandles, out groupNames);
                //        if (numGroupHandles > 0)
                //        {
                //            foreach (var group in groupHandles)
                //            {
                //                DaGroupState groupInfo;
                //                GetGroupState(group, out groupInfo);
                //                int numItemStates;
                //                DaItemState[] itemStates;
                //                GetItemStates(group, out numItemStates, out itemStates);
                //            }
                //        }
                //    }
                //}

                DateTime dateTimeStartRefresh = DateTime.UtcNow;

                if ((count % 2) == 0)
                {
                    ToggleTank1Cond();          // every 2s
                }
                if ((count % 3) == 0)
                {
                    ToggleRampCond();           // every 3s
                }
                if ((count % 5) == 0)
                {                               // every 5s
                    condHeating1.ToggleCondition();
                }
                if ((count % 120) == 0)
                {          // every 2 min.
                    devfailattrs[0] = (long)1234;                               // Error Code
                    devfailattrs[1] = "3Com EtherLink XL NIC (3C900B-COMBO)";   // Device Name
                    ProcessSimpleEvent(CategoryIdDeviceFailure, SourceIdNetworkAdapter, "No response", 800, 2, devfailattrs, DateTime.Now);
                }

                // Sample how to get a list of active items (used at least by one client)
                GetActiveItems(out _, out _);

                count++;
                ramp++;
                myDynamicRampItem_.Value = ramp;
                // update server cache for this item
                SetItemValue(myDynamicRampItem_.DeviceItemHandle, myDynamicRampItem_.Value,
                   DaQuality.Good.Code, DateTime.Now);

                myDynamicRandomItem_.Value = rand.Next();
                SetItemValue(myDynamicRandomItem_.DeviceItemHandle, myDynamicRandomItem_.Value,
                   DaQuality.Good.Code, DateTime.Now);

                myDynamicSineItem_.Value = Math.Sin((count % 40) * 0.1570796327);
                SetItemValue(myDynamicSineItem_.DeviceItemHandle, myDynamicSineItem_.Value,
                   DaQuality.Good.Code, DateTime.Now);

                DateTime dateTimeStart = DateTime.UtcNow;
                // Update all used items once
                foreach (MyItem item in dynamicItems_.Values)
                {
                    item.Value = ramp;
                    SetItemValue(item.DeviceItemHandle, item.Value, DaQuality.Good.Code, DateTime.Now);
                }
                TimeSpan duration = (DateTime.UtcNow - dateTimeStart);

                double callsPerSecond = 1 / (duration.TotalSeconds / NumberMassItems);

                myDynamicSetItemValueCallsPerSecondItem_.Value = Math.Round(callsPerSecond);
                SetItemValue(myDynamicSetItemValueCallsPerSecondItem_.DeviceItemHandle, myDynamicSetItemValueCallsPerSecondItem_.Value, DaQuality.Good.Code, DateTime.Now);

                TimeSpan durationRefresh = (DateTime.UtcNow - dateTimeStartRefresh);

                int waitTime = 1000 - (int)durationRefresh.TotalMilliseconds;

                if (waitTime > 0)
                {
                    Thread.Sleep(waitTime);    // ms
                }

                if (stopThread_ != null)
                {
                    stopThread_.Set();
                    return;               // terminate the thread
                }
            }
        }
        #endregion

    }

    #region My Item Property Class

    /// <summary>
    /// My Item Property Implementation
    /// </summary>
    public class MyItemProperty
    {
        #region Constructors, Destructor, Initialization

        /// <summary>
        /// Constructor
        /// </summary>
        /// <param name="propertyId">ID of the property</param>
        /// <param name="propertyValue">Value of the property</param>
        public MyItemProperty(int propertyId, object propertyValue)
        {
            PropertyId = propertyId;
            PropertyValue = propertyValue;
        }

        #endregion

        #region Properties

        /// <summary>
        /// ID of the property
        /// </summary>
        public int PropertyId { get; private set; }

        /// <summary>
        /// Value of the property
        /// </summary>
        public object PropertyValue { get; private set; }

        #endregion

    }

    #endregion

    #region My Item Class

    /// <summary>
    /// My Item Implementation
    /// </summary>
    class MyItem
    {

        #region Constructors, Destructor, Initialization

        public MyItem(
                        string itemName,
                        object initValue)
        {
            ItemName = itemName;
            Value = initValue;
            Quality = DaQuality.Good;
            Timestamp = DateTime.UtcNow;
        }

        public MyItem(
                string itemName,
                object initValue,
                MyItemProperty[] itemProperties)
        {
            ItemName = itemName;
            Value = initValue;
            Quality = DaQuality.Good;
            ItemProperties = itemProperties;
            Timestamp = DateTime.UtcNow;
        }

        #endregion

        #region Properties

        // Can be used to identify the item, not used in this example. You can use also other information like device
        // specific information (e.g. serial line, datablock and data number for PLC, ...
        public IntPtr DeviceItemHandle;
        public string ItemName { get; private set; }
        public object Value { get; set; }
        public DaQuality Quality { get; set; }
        public DateTime Timestamp { get; set; }
        public MyItemProperty[] ItemProperties { get; private set; }

        #endregion
    }

    #endregion

}