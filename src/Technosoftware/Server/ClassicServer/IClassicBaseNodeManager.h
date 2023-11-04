/*
 * Copyright (c) 2011-2022 Technosoftware GmbH. All rights reserved
 * Web: https://technosoftware.com 
 * 
 * The source code in this file is covered under a dual-license scenario:
 *   - Owner of a purchased license: SCLA 1.0
 *   - GPL V3: everybody else
 *
 * SCLA license terms accompanied with this source code.
 * See https://technosoftware.com/license/Source_Code_License_Agreement.pdf
 *
 * GNU General Public License as published by the Free Software Foundation;
 * version 3 of the License are accompanied with this source code.
 * See https://technosoftware.com/license/GPLv3License.txt
 *
 * This source code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */

 /** @file
  *  @brief OPC DA/AE Server SDK DLL generic server interface.
 *
 * IMPORTANT: DON'T CHANGE THIS FILE.
 */

#if !defined(__ICLASSICBASENODEMANAGER_H_)
#define __ICLASSICBASENODEMANAGER_H_

class DeviceItem;

namespace IClassicBaseNodeManager
{
#if _MSC_VER > 1000
#pragma once
#endif 

  /**
   * @addtogroup interface_base IClassicBaseNodeManager
   * @{
   */

/**
 * @struct  ClassicServerDefinition
 *
 * @brief   Server information structure returned by OnGetDaServerDefinition and
 *          OnGetAeServerDefinition.
 */

struct ClassicServerDefinition
{
    /** @brief   The DCOM server is registered with this CLSID. */
    WCHAR  * ClsidServer;
    /** @brief   The DCOM server application is registered with this CLSID in the AppID section. */
    WCHAR  * ClsidApp;
    /** @brief   Version independent ProgID of the DCOM server. */
    WCHAR  * PrgidServer;
    /** @brief   ProgID of the current DCOM server version. */
    WCHAR  * PrgidCurrServer;
    /** @brief   Version independent friendly name of the DCOM server. */
    WCHAR  * NameServer;
    /** @brief   Version independent friendly name of the current DCOM server version. */
    WCHAR  * NameCurrServer;
    /** @brief   Server vendor name. */
    WCHAR  * CompanyName;
};

/**
 * @enum    ServerState
 *
 * @brief   The set of possible server states.
 */

enum class ServerState
{
    /// The server state is not known.
    Unknown,

    /// The server is running normally.
    Running,

    /// The server is not functioning due to a fatal error.
    Failed,

    /// The server cannot load its configuration information. 
    NoConfig,

    /// The server has halted all communication with the underlying hardware.
    Suspended,

    /// The server is disconnected from the underlying hardware.
    Test,

    /// The server cannot communicate with the underlying hardware.
    CommFault
};

/**
 * @enum    DaAccessRights
 *
 * @brief   Defines possible item access rights.
 *          Indicates if this item is read only, write only or read/write.
 *          This is NOT related to security but rather to the nature of the underlying
 *          device.
 */

enum class DaAccessRights : int
{
    /// The access rights for this item are unknown.
    NotKnown = 0x00,
    /// The client can read the data item's value.
    Readable = 0x01,
    /// The client can change the data item's value.
    Writable = 0x02,
    /// The client can read and change the data item's value.
    ReadWritable = 0x03
};

/**
 * @enum    DaEuType
 *
 * @brief    Defines possible item engineering unit types
 */

enum class DaEuType : int
{
    /// No engineering unit information available
    NoEnum = 0x00,
    /// Analog engineering unit - will contain a SAFEARRAY of exactly two doubles (VT_ARRAY | VT_R8) corresponding to the LOW and HI EU range.
    Analog = 0x01,
    /// Enumerated engeniring unit - will contain a SAFEARRAY of strings *(VT_ARRAY | VT_BSTR) which contains a list of strings(Example: �OPEN�,
    /// *�CLOSE�, �IN TRANSIT�, etc.) corresponding to sequential numeric values * (0, 1, 2, etc.)
    Enumerated = 0x02
};

/**
 * @enum    DaQualityBits
 *
 * @brief   Defines the possible quality status bits. These flags represent the quality state for
 *          an item's data value. This is intended to be similar to but slightly simpler than the
 *          Fieldbus Data Quality Specification (section 4.4.1 in the H1 Final Specifications).
 *          This design makes it fairly easy for both servers and client applications to
 *          determine how much functionality they want to implement.
 */

enum class DaQualityBits : int
{
    /// The Quality of the value is Good.
    Good = 0x000000C0,
    /// The value has been Overridden. Typically this means the input has been disconnected and a manually entered value has been 'forced'.
    GoodLocalOverride = 0x000000D8,
    /// The value is bad but no specific reason is known.
    Bad = 0x00000000,
    /// There is some server specific problem with the configuration. For example the item in question has been deleted from the configuration.
    BadConfigurationError = 0x00000004,
    /// The input is required to be logically connected to something but is not. This quality may reflect that no value is available at this time, for reasons like the value may have not been provided by the data source.
    BadNotConnected = 0x00000008,
    /// A device failure has been detected.
    BadDeviceFailure = 0x0000000c,
    /// A sensor failure had been detected (the 'Limits' field can provide additional diagnostic information in some situations).
    BadSensorFailure = 0x00000010,
    /// Communications have failed. However, the last known value is available. Note that the 'age' of the value may be determined from the time stamp in the item state.
    BadLastKnownValue = 0x00000014,
    /// Communications have failed. There is no last known value is available.
    BadCommFailure = 0x00000018,
    /// The block is off scan or otherwise locked. This quality is also used when the active state of the item or the group containing the item is InActive.
    BadOutOfService = 0x0000001C,
    /** @brief After Items are added to a group, it may take some time for the server to actually obtain values for these items. In such cases the client might perform a read (from cache), or establish a ConnectionPoint based subscription and/or execute a Refresh on such a subscription before the values are available. This substatus is only
     * available from OPC DA 3.0 or newer servers.
    */
    BadWaitingForInitialData = 0x00000020,
    /// There is no specific reason why the value is uncertain.
    Uncertain = 0x00000040,
    /** @brief
     * Whatever was writing this value has stopped doing so. The returned value should
     * be regarded as 'stale'. Note that this differs from a BAD value with Substatus
     * badLastKnownValue (Last Known Value). That status is associated specifically with a
     * detectable communications error on a 'fetched' value. This error is associated with the
     * failure of some external source to 'put' something into the value within an acceptable
     * period of time. Note that the 'age' of the value can be determined from the time stamp
     * in the item state.
     */
    UncertainLastUsableValue = 0x00000044,
    /** @brief
     * Either the value has 'pegged' at one of the sensor limits (in which case the
     * limit field should be set to low or high) or the sensor is otherwise known to be out of
     * calibration via some form of internal diagnostics (in which case the limit field should
     * be none).
     */
    UncertainSensorNotAccurate = 0x00000050,
    /** @brief
     * The returned value is outside the limits defined for this parameter. Note that in
     * this case (per the Fieldbus Specification) the 'Limits' field indicates which limit has
     * been exceeded but does NOT necessarily imply that the value cannot move farther out of
     * range.
     */
    UncertainEUExceeded = 0x00000054,
    /** @brief
     * The value is derived from multiple sources and has less than the required number
     * of Good sources.
     */
    UncertainSubNormal = 0x00000058
};

/**
 * @enum    DaLimitBits
 *
 * @brief   Defines the possible limit status bits. The Limit Field is valid regardless of the
 *          Quality and Substatus. In some cases such as Sensor Failure it can provide useful
 *          diagnostic information.
 */

enum class DaLimitBits : int
{
    /// The value is free to move up or down
    None = 0x0,
    /// The value has 'pegged' at some lower limit
    Low = 0x1,
    /// The value has 'pegged' at some high limit
    High = 0x2,
    /// The value is a constant and cannot move
    Constant = 0x3
};

/**
 * @enum    DaQualityMasks
 *
 * @brief    Defines bit masks for the quality.
 */

enum class DaQualityMasks : int
{
    /// Quality related bits
    QualityMask = +0x00FC,
    /// Limit related bits
    LimitMask = +0x0003,
    /// Vendor specific bits
    VendorMask = -0x00FD
};

/**
 * @struct    DaQuality
 *
 * @brief    Contains the quality field for an item value.
 */

struct DaQuality
{
    /// The value in the quality bits field.
    DaQualityBits QualityBits;

    /// The value in the limit bits field.
    DaLimitBits LimitBits;

    /// The value in the quality bits field.
    byte VendorBits;

    /// Returns the quality as a 16 bit integer.
    short GetCode() const
    {
        WORD code = 0;

        code |= static_cast<WORD>(QualityBits);
        code |= static_cast<WORD>(LimitBits);
        code |= static_cast<WORD>(VendorBits << 8);

        return code; // (code <= WORD.MaxValue) ? (short)code : (short)-((UInt16.MaxValue + 1 - code));
    }

};

/**
 * @enum    DaBrowseMode
 *
 * @brief    Browse Mode enumerator.
 */

enum class DaBrowseMode
{
    /// Browse calls are handled in the generic server and return the item/branches that are defined in the cache.
    Generic = 0,
    /// Browse calls are handled in the customization plug-in and typically return all items that could be dynamically added to the cache.
    Custom = 1
};

/**
 * @enum    DaBrowseType
 *
 * @brief    Enumerator for browse mode selection.
 */

enum class DaBrowseType
{
    /// Select only branches
    Branch = 1,
    /// Select only leafs
    Leaf = 2,
    /// Select all branches and leafs
    Flat = 3,
};

/**
 * @enum    DaBrowseDirection
 *
 * @brief   Defines the way to move 'up' or 'down' or 'to' in a hierarchical address space.
 */

enum class DaBrowseDirection
{
    /// move 'up' in a hierarchical address space.
    Up = 1,
    /// move 'down' in a hierarchical address space.
    Down = 2,
    /// move 'to' in a hierarchical address space.
    To = 3
};

/**
 * @enum    LogLevel
 *
 * @brief   Represents the log level.
 */

enum class LogLevel
{
    /** @brief
     * Represents the Trace log level. This log level is mostly
     * intended to be used in the debug and development process.
     */
	Trace = 0,

	/** @brief
	* Represents the Debug log level. This log level is mostly
	* intended to be used in the debug and development process.
	*/
	Debug = 1,

    /** @brief
     * Represents the Info log level. This log level is intended
     * to track the general progress of applications at a
     * coarse-grained level.
     */
	Info = 2,

    /** @brief
     * Represents the Warning log level. This log level designates
     * potentially harmful events or situations.
     */
	Warning = 3,

    /** @brief
     * Represents the Error log level. This log level designates
     * error events or situations which are not critical to the
     * entire system. This log level thus describes recoverable
     * or less important errors.
     */
	Error = 4,

	/** @brief
	* Represents the Alarm log level. This log level designates
	* error events or situations which are critical to the
	* entire system. This log level thus describes recoverable
	* or important errors.
	*/
	Alarm = 5,

    /** @brief
     * Represents the Fatal log level. This log level designates
     * errors which are not recoverable and eventually stop the
     * system or application from working.
     */
	Fatal = 6,

	/** @brief
	 * Represents the Disabled log level. This log level disable logging.
	 */
	Disabled = 7,
};

/**
 * @class   AeConditionState
 *
 * @brief   An Alarms&amp;Events condition state.
 */

class AeConditionState
{
    // Construction
public:

    /**
     * @fn  AeConditionState::AeConditionState()
     *
     * @brief   Default constructor.
     */

	AeConditionState()
	{
        conditionId_ = 0;
        subConditionId_ = 0;
        activeState_ = 0;
        quality_ = 0;
        attributeCount_ = 0;
        attributeValues_ = nullptr;
        message_ = nullptr;
        severity_ = nullptr;
        acknowledgeRequired_ = nullptr;
        timeStamp_ = nullptr;
	}

    /**
     * @fn  AeConditionState::~AeConditionState()
     *
     * @brief   Destructor.
     */

	~AeConditionState() {}

	// Attributes
public:
    inline DWORD      &  CondID() { return conditionId_; }
    inline DWORD      &  SubCondID() { return subConditionId_; }
    inline BOOL       &  ActiveState() { return activeState_; }
    inline WORD       &  Quality() { return quality_; }
    inline DWORD      &  AttrCount() { return attributeCount_; }
    inline LPVARIANT  &  AttrValuesPtr() { return attributeValues_; }
    inline LPCWSTR    &  Message() { return message_; }
    inline DWORD*     &  SeverityPtr() { return severity_; }
    inline LPBOOL     &  AckRequiredPtr() { return acknowledgeRequired_; }
    inline LPFILETIME &  TimeStampPtr() { return timeStamp_; }

	// Implementation
protected:
    DWORD       conditionId_;
    DWORD       subConditionId_;
    BOOL        activeState_;
    WORD        quality_;
    DWORD       attributeCount_;
    LPVARIANT   attributeValues_;
    LPCWSTR     message_;
    DWORD*      severity_;
    LPBOOL      acknowledgeRequired_;
    LPFILETIME  timeStamp_;

};

/**
 * @class   DaGroupState
 *
 * @brief   The state of an OPC DA Group.
 */

class DaGroupState
{
    // Attributes
public:
    /**
    * @brief   Name of the group.
    */

    LPWSTR  GroupName;

    /**
    * @brief   the update rate at which the server can refresh the group data in msec.
    */

    long  UpdateRate;

    /**
    * @brief   Handle for this group provided by the client and used during refresh or read of group
    *          within callback functions to identify the group.
    */

    long  ClientGroupHandle;

    /**
    * @brief   The deadband in percent.
    */

    float PercentDeadband;

    /**
    * @brief   Locale ID.
    */

    long  LocaleId;

    /**
    * @brief   Tells whether IOPCDataCallback::OnDataChange callbacks are disabled or enabled.
    */

    BOOL  DataChangeEnabled;

};

/**
 * @class   DaItemState
 *
 * @brief   The state of an OPC DA Item.
 */

class DaItemState
{
    // Attributes
public:
    /**
    * @brief   Name of the item.
    */

    LPWSTR  ItemName;

    /**
    * @brief   recommendation to the server on 'how to get the data'
    */

    LPWSTR  AccessPath;

    /**
     * @brief   item access rights RO,WO,RW : OPCACCESSRIGTHS enum :
     *              OPC_READABLE   (==1)
     *              OPC_WRITEABLE  (==2)
     *          high word available for vendor specific use.
     */

    DWORD   AccessRights;

    /**
    * @brief   Handle of the device item.
    */

    void*   DeviceItemHandle;
};

/**
 * @}
 */

 /**
 * @addtogroup interface_generic_dataaccess Data Access related Generic Server Callback Methods
 * @{
 */

/**
 * @fn  HRESULT AddItem(LPWSTR itemId, DaAccessRights accessRights, LPVARIANT initValue, bool active, DaEuType euType, double minValue, double maxValue, void** deviceItemHandle = nullptr);
 *
 * @brief   This function is called by the customization plugin and add an analog item to the
 *          generic server cache. If
 *          <see cref="DaBrowseMode::Generic" text="DaBrowseMode.Generic" />
 *          is set the item is also added to the the generic server \internal browse hierarchy.
 *          
 *          The generic server sets the EU type to
 *          <see cref="DaEuType::Analog" text="DaEuType.Analog" />
 *          and use the minValue and maxValue for defining the correct EUInfo.
 *          
 *          This method is typically called during the execution of the
 *          <see cref="OnCreateServerItems" text="OnCreateServerItems" />
 *          method, but it can be called anytime an item needs to be added to the generic server
 *          cache.
 *          
 *          Items that are added to the generic server cache can be accessed by OPC clients.
 *
 * @param   itemId                      Fully qualified item name. The OPC clients use this name
 *                                      to access the item. This is a fully qualified item identifier
 *                                      such as "device1.channel5.heater3". The generic server part
 *                                      builds an appropriate hierarchical address space. Note that
 *                                      the separator character ( . in the above sample identifier )
 *                                      can be changed with the <see cref="OnGetDaServerParameters" />
 *                                      function.
 * @param   accessRights                Access rights as defined in the OPC specification\:
 *                                      <see cref="DaAccessRights::Readable" text="DaAccessRights." />
 *                                      , <see cref="DaAccessRights::Writable" text="DaAccessRights.Wri
 *                                      table" />,
 *                                      <see cref="DaAccessRights::ReadWritable" text="DaAccessRights.ReadWritable" />
 * @param   initValue                   Object with initial value and the item's canonical data
 *                                      type. A value must always be specified for the canonical data
 *                                      type to be defined.
 * @param   active                      true to set the item in active state.
 * @param   euType                      Type of the Engineering Unit.
 * @param   minValue                    Analog engineering unit, corresponding to the LOW EU
 *                                      range.
 * @param   maxValue                    Analog engineering unit, corresponding to the HIGH EU
 *                                      range.
 * @param [in,out]  deviceItemHandle    If non\-null, the function returns the created device
 *                                      item. This pointer can the be used in functions requiring the
 *                                      item handle, e.g.
 *                                      <see cref="SetItemValue@void*@LPVARIANT@short@FILETIME" text="SetItemValue" />(),
 *                                      allowing improved performance.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the item was
 *          successfully added to the cache.
 */

HRESULT AddItem(LPWSTR itemId, DaAccessRights accessRights, LPVARIANT initValue, bool active, DaEuType euType, double minValue, double maxValue, void** deviceItemHandle = nullptr);

/**
 * @fn  HRESULT AddItem(LPWSTR itemId, DaAccessRights accessRights, LPVARIANT initValue, void** deviceItemHandle = nullptr);
 *
 * @brief   This function is called by the customization plugin and add an item to the generic
 *          server cache. If <see cref="DaBrowseMode::Generic" text="DaBrowseMode.Generic" />
 *          is set the item is also added to the the generic server \internal browse hierarchy.
 *          
 *          The generic server sets the EU type to
 *          <see cref="DaEuType::noEnum" text="DaEuType.noEnumand" />
 *          define an empty EUInfo.
 *          
 *          This method is typically called during the execution of the
 *          <see cref="OnCreateServerItems" text="OnCreateServerItems" />
 *          method, but it can be called anytime an item needs to be added to the generic server
 *          cache.
 *          
 *          Items that are added to the generic server cache can be accessed by OPC clients.
 *
 * @param   itemId                      Fully qualified item name. The OPC clients use this name
 *                                      to access the item. This is a fully qualified item identifier
 *                                      such as "device1.channel5.heater3". The generic server part
 *                                      builds an appropriate hierarchical address space. Note that
 *                                      the separator character ( . in the above sample identifier )
 *                                      can be changed with the <see cref="OnGetDaServerParameters" />
 *                                      function.
 * @param   accessRights                Access rights as defined in the OPC specification\:
 *                                      DaAccessRights., DaAccessRights.Writable,
 *                                      DaAccessRights.ReadWritable.
 * @param   initValue                   Object with initial value and the item's canonical data
 *                                      type. A value must always be specified for the canonical data
 *                                      type to be defined.
 * @param [in,out]  deviceItemHandle    If non\-null, the function returns the created device
 *                                      item. This pointer can the be used  in functions requiring
 *                                      the item handle, e.g.
 *                                      <see cref="SetItemValue@void*@LPVARIANT@short@FILETIME" text="SetItemValue" />(),
 *                                      allowing improved performance.
 *
 * @return  A HRESULT with the result of the operation. Returns S_OK if the item was successfully
 *          added to the cache.
 */

HRESULT AddItem(LPWSTR itemId, DaAccessRights accessRights, LPVARIANT initValue, void** deviceItemHandle = nullptr);

/**
 * @brief    This function is called by the customization plugin and add an analog item to the
 *             generic server cache. If
 *             <see cref="DaBrowseMode::Generic" text="DaBrowseMode.Generic" />
 *             is set the item is also added to the the generic server \internal browse hierarchy.
 *
 *             The generic server sets the EU type to
 *             <see cref="DaEuType::Analog" text="DaEuType.Analog" />
 *             and use the minValue and maxValue for defining the correct EUInfo.
 *
 *             This method is typically called during the execution of the
 *             <see cref="OnCreateServerItems" text="OnCreateServerItems" />
 *             method, but it can be called anytime an item needs to be added to the generic server
 *             cache.
 *
 *             Items that are added to the generic server cache can be accessed by OPC clients.
 *
 * @param    itemId                    Fully qualified item name. The OPC clients use this name to
 *                                     access the item. This is a fully qualified item identifier such
 *                                     as "device1.channel5.heater3". The generic server part builds an
 *                                     appropriate hierarchical address space. Note that the separator
 *                                     character ( . in the above sample identifier ) can be changed
 *                                     with the <see cref="OnGetDaServerParameters" />
 *                                     function.
 * @param    accessRights            Access rights as defined in the OPC specification\:
 *                                     <see cref="DaAccessRights::Readable" text="DaAccessRights." />
 *                                     , <see cref="DaAccessRights::Writable" text="DaAccessRights.writabl
 *                                     e" />,
 *                                     <see cref="DaAccessRights::ReadWritable" text="DaAccessRights.ReadWritable" />
 * @param    initValue                Object with initial value and the item's canonical data type.
 *                                     A value must always be specified for the canonical data type to
 *                                     be defined.
 * @param    minValue                Analog engineering unit, corresponding to the LOW EU range.
 * @param    maxValue                Analog engineering unit, corresponding to the HIGH EU range.
 * @param    deviceItemHandle [in,out]        If non\-null, the function returns the created device item.
 *                                     This pointer can the be used in functions requiring the item handle, e.g.
 *                                     <see cref="SetItemValue@void*@LPVARIANT@short@FILETIME" text="SetItemValue" />(),
 *                                     allowing improved performance.
 *
 * @return    A HRESULT code with the result of the operation. Returns S_OK if the item was
 *             successfully added to the cache.
 */

HRESULT AddAnalogItem(LPWSTR itemId, DaAccessRights accessRights, LPVARIANT initValue, double minValue, double maxValue, void** deviceItemHandle = nullptr);

/**
 * @brief    This function is called by the customization plugin and removes an item from the
 *             generic server cache. If
 *             <see cref="DaBrowseMode::Generic" text="DaBrowseMode.Generic" />
 *             is set the item is also removed from the the generic server \internal browse
 *             hierarchy.
 *
 *             This method can be called anytime an item needs to be removed from the generic server
 *             cache.
 *
 *             Items that are removed from the generic server cache can no longer be accessed by OPC
 *             clients.
 *
 *             Important: If one or more clients has added the item to a group, the item is only marked
 *             to be removed and not fully removed from cache.
 *
 * @param    deviceItemHandle         Device Item which should be removed from the generic server cache.
 *
 * @return    Returns S_OK if the item was successfully removed from the cache.
*/

HRESULT RemoveItem(void* deviceItemHandle);

/**
* @brief    This function is called by the customization plugin and deletes an item from the
*             generic server cache.
*
*             Important: This should only be called if no client uses this item in a group.
*
* @param    deviceItemHandle         Device Item which should be removed from the generic server cache.
*
* @return    Returns S_OK if the item was successfully removed from the cache.
*/

HRESULT DeleteItem(void* deviceItemHandle);

/**
 * @fn  HRESULT AddProperty(int propertyId, LPWSTR description, LPVARIANT valueType);
 *
 * @brief   Adds a custom specific property to the generic server list of item properties.
 *
 * @param   propertyId  The property ID assigned to the property. 0000 to 4999 are reserved for
 *                      OPC use.
 * @param   description A brief vendor supplied text description of the property.
 * @param   valueType   The default value and type of the property.
 *
 * @return  Returns S_OK if the property was successfully added to the generic server property
 *           list.
 */

HRESULT AddProperty(int propertyId, LPWSTR description, LPVARIANT valueType);

/**
 * @fn  HRESULT SetItemValue(void* deviceItemHandle, LPVARIANT newValue, short quality, FILETIME timestamp);
 *
 * @brief   Generic server callback method.
 *          
 *            Write an item value into the cache.
 *
 * @param [in,out]  deviceItemHandle    Device Item as defined in the AddItem method call.
 * @param           newValue            Object with new item value.The value must match the
 *                                      canonical data type of this item. The canonical date type is
 *                                      the type of the value in the cache and is defined in the
 *                                      AddItem method call. null can be passed to change only the
 *                                      quality and timestamp.
 * @param           quality             New quality of the item value.This is a short value
 *                                      (Int16) with a value of the OPCQuality enumerator.
 * @param   timestamp                   New timestamp of the new item value.
 *
 * @return  A HRESULT code with the result of the operation.
 *          
 *           Returns S_OK if the value was successfully written into the cache.
 */

HRESULT SetItemValue(void* deviceItemHandle, LPVARIANT newValue, short quality, FILETIME timestamp);

/**
 * @fn  void SetServerState(ServerState serverState);
 *
 * @brief   Generic server callback method.
 *          
 *          Set the OPC server state value, that is returned in client GetStatus calls.
 *
 * @param   serverState The new state of the server as an <see cref="ServerState" />
 *                      enumerator value.
 */

void SetServerState(ServerState serverState);

/**
 * @fn  void GetActiveItems(int * numItemHandles, void* ** deviceItemHandles);
 *
 * @brief   Generic server callback method.
 *          
 *          Generic server callback to get a list of items used at least by one client.
 *
 * @param [in,out]  numItemHandles      Number of defined item handles; -1 means that function is
 *                                      not supported by the generic server.
 * @param [in,out]  deviceItemHandles   Array of handles defined in the custom plugin assembly to
 *                                      reference the items currently active by at least one client.
 *
 * ### return   The active items.
 */

void GetActiveItems(int * numItemHandles, void* ** deviceItemHandles);

/**
 * @fn  void GetClients(int * numClientHandles, void* ** clientHandles, LPWSTR ** clientNames);
 *
 * @brief   Generic server callback method.
 *          
 *          Generic server callback to get a list of clients currently connected to the server.
 *
 * @param [in,out]  numClientHandles    If non-null, number of client handles.
 * @param [in,out]  clientHandles       If non-null, the client handles.
 * @param [in,out]  clientNames         If non-null, list of names of the clients.
 */

void GetClients(int * numClientHandles, void* ** clientHandles, LPWSTR ** clientNames);

/**
 * @fn  void GetGroups(void * clientHandle, int * numGroupHandles, void* ** groupHandles, LPWSTR ** groupNames);
 *
 * @brief   Gets the groups.
 *
 * @param [in,out]  clientHandle    If non-null, handle of the client.
 * @param [in,out]  numGroupHandles If non-null, number of group handles.
 * @param [in,out]  groupHandles    If non-null, the group handles.
 * @param [in,out]  groupNames      If non-null, list of names of the groups.
 */

void GetGroups(void * clientHandle, int * numGroupHandles, void* ** groupHandles, LPWSTR ** groupNames);

/**
 * @fn  void GetGroupState(void * groupHandle, DaGroupState * groupState);
 *
 * @brief   Gets group information of the specified group.
 *
 * @param [in,out]  groupHandle         If non-null, handle of the group.
 * @param [in,out]  groupState    If non-null, information describing the group.
 */

void GetGroupState(void * groupHandle, DaGroupState * groupState);

/**
 * @fn  void GetItemStates(void * groupHandle, int * numDaItemStates, DaItemState* * daItemStates);
 *
 * @brief   Get the item information for all items added to the specified group.
 *
 * @param [in,out]  groupHandle        If non-null, handle of the group.
 * @param [in,out]  numDaItemStates    If non-null, number of item state handles.
 * @param [in,out]  daItemStates    If non-null, list of information describing the items.
 */

void GetItemStates(void * groupHandle, int * numDaItemStates, DaItemState* * daItemStates);

/**
 * @fn  void FireShutdownRequest(LPCWSTR reason);
 *
 * @brief   Generic server callback method.
 *
 *          Generic server callback to fire a 'Shutdown Request' to the subscribed clients.
 *
 * @param [in]  reason  The reason why the server wants to shutdown.
 */

void FireShutdownRequest(LPCWSTR reason);

/**
 * @}
 */

 /**
  * @addtogroup interface_generic_alarmsevents Alarms&Events related Generic Server Callback Methods
  * @{
  */

/**
 * @fn  HRESULT AddSimpleEventCategory(int categoryId, LPWSTR categoryDescription);
 *
 * @brief   Adds a Simple Event Category to the Alarms&amp;Event Space.
 *          
 *          The recommended Event Categories are described in Appendix B of the OPC Alarms&amp;
 *          Events Specification.
 *
 * @param   categoryId          Identifier of the new Event Category. This Id must be unique
 *                              within the Event Server.
 * @param   categoryDescription Description of the Event Category.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event category
 *          was successfully added to the event space.
 */

HRESULT AddSimpleEventCategory(int categoryId, LPWSTR categoryDescription);

/**
 * @fn  HRESULT AddTrackingEventCategory(int categoryId, LPWSTR categoryDescription);
 *
 * @brief   Adds a Tracking Event Category to the Alarms&amp;Event Space.
 *          
 *          The recommended Event Categories are described in Appendix B of the OPC Alarms&amp;
 *          Events Specification.
 *
 * @param   categoryId          Identifier of the new Event Category. This Id must be unique
 *                              within the Event Server.
 * @param   categoryDescription Description of the Event Category.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event category
 *           was successfully added to the event space.
 */

HRESULT AddTrackingEventCategory(int categoryId, LPWSTR categoryDescription);

/**
 * @fn  HRESULT AddConditionEventCategory(int categoryId, LPWSTR categoryDescription);
 *
 * @brief   Adds a Condition Event Category to the Alarms&amp;Event Space.
 *
 * @param   categoryId          Identifier of the new Event Category. This Id must be unique
 *                              within the Event Server.
 * @param   categoryDescription Description of the Event Category.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event category
 *           was successfully added to the event space.
 */

HRESULT AddConditionEventCategory(int categoryId, LPWSTR categoryDescription);

/**
 * @fn  HRESULT AddEventAttribute(int categoryId, int eventAttribute, LPWSTR attributeDescription, VARTYPE dataType);
 *
 * @brief   Adds a vendor specific Attribute to the Alarms&amp;Event Category.
 *          
 *          The recommended Event Attributes are described in Appendix C of the OPC OPC
 *          Alarms&amp;Events Specification. It is not required to add the Attributes 'ACK
 *          COMMENT' and 'AREAS' because they are internally added when a new Category is created.
 *
 * @param   categoryId              Identifier of an existing Event Category.
 * @param   eventAttribute          Identifier of the new Event Attribute. This ID must be unique
 *                                  within the Event Category.
 * @param   attributeDescription    Description of the Event Attribute.
 * @param   dataType                Object identifying the data type of the event attribute.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event attribute
 *           was successfully added to the event space.
 */

HRESULT AddEventAttribute(int categoryId, int eventAttribute, LPWSTR attributeDescription, VARTYPE dataType);

/**
 * @fn  HRESULT AddSingleStateConditionDefinition(int categoryId, int conditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired);
 *
 * @brief   Adds a Single State Condition Definition to the Alarms&amp;Event Category.
 *          
 *           Adds a definition to the Event Space which can be used by Single State
 *          Conditions. A Single State Condition has only one sub-state of interest, where the
 *          condition is active or not. The values Severity, Description and AckRequired flag are
 *          the default values for those Conditions which uses this Definition. The default
 *          values are used if no other values are specified with function
 *          ProcessConditionStateChanges().
 *
 * @param   categoryId  Identifier of an existing Event Category.
 * @param   conditionId Identifier of the new Event Condition Definition. This ID must be unique
 *                      within the Alarms&amp;Events Server.
 * @param   name        Text string with the name of the Event Condition Definition. This name
 *                      must be unique within the Alarms&amp;Events Server.
 * @param   condition   Text string with the condition represented by this definition.
 * @param   severity    The urgency in the range of 1 ... \1000. This is a default value.
 * @param   description Text string with description. This text is used as message for the
 *                      generated Events. This is a default value.
 * @param   ackRequired True if the Event Conditions which uses this Definition requires
 *                      acknowledgement; otherwise False. This is a default value.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the Single State
 *           Condition Definition was successfully added to the event space.
 */

HRESULT AddSingleStateConditionDefinition(int categoryId, int conditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired);

/**
 * @fn  HRESULT AddMultiStateConditionDefinition(int categoryId, int conditionId, LPWSTR name);
 *
 * @brief   Adds a Multi State Condition Definition to the Alarms&amp;Event Category.
 *          
 *          Adds a definition to the Event Space which can be used by Multi State Conditions. A
 *          Multi State Condition has at least two sub-states of interest. Sub-states are
 *          mutually exclusive, only one sub-state can be active. Use the function
 *          AddSubConditionDefinition() to add at least two Sub Condition Definitions for the sub-
 *          states.
 *
 * @param   categoryId  Identifier of an existing Event Category.
 * @param   conditionId Identifier of the new Event Condition Definition. This ID must be unique
 *                      within the Alarms&amp;Events Server.
 * @param   name        Text string with the name of the Event Condition Definition. This name
 *                      must be unique within the Alarms&amp;Events Server.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the Single State
 *          Condition Definition was successfully added to the event space.
 */

HRESULT AddMultiStateConditionDefinition(int categoryId, int conditionId, LPWSTR name);

/**
 * @fn  HRESULT AddSubConditionDefinition(int conditionId, int subConditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired);
 *
 * @brief   Adds a Sub Condition Definition to an existing Multi State Condition Definition.
 *          
 *          This function must be called at least twice for each Multi State Condition Definition
 *          added with AddMultiStateConditionDef(). A Multi State Condition has at least two sub-
 *          states of interest. Sub-states are mutually exclusive, only one sub- state can be
 *          active. The values Severity, Description and AckRequired flag are the default values
 *          for those Conditions which uses this Definition. The default values are used if no
 *          other values are specified with function ProcessConditionStateChanges().
 *
 * @param   conditionId     Identifier of an existing Event Condition Definition.
 * @param   subConditionId  Identifier of the new Sub Condition Definition. This ID must be
 *                          unique within the Event Condition Definition. Value 0 is invalid
 *                          because internally used.
 * @param   name            Text string with the name of the Event Sub Condition Definition. This
 *                          name must be unique within the Alarms&amp;Events Server.
 * @param   condition       Text string with the condition represented by this definition.
 * @param   severity        The urgency in the range of 1
 *                          ... 1000. This is a default
 *                          value.
 * @param   description     Text string with description. This text is used as message for the
 *                          generated Events. This is a default value.
 * @param   ackRequired     True if the Event Conditions which uses this Definition requires
 *                          acknowledgement;
 *                          otherwise False. This is a default value.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the Sub Condition
 *           Definition was successfully added to the event space.
 */

HRESULT AddSubConditionDefinition(int conditionId, int subConditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired);

/**
 * @fn  HRESULT AddArea(int parentAreaId, int areaId, LPWSTR name);
 *
 * @brief   Adds an Area to the Alarms&amp;Event Process Area Space.
 *          
 *          The name parts of a fully qualified Area Name are separated by '.' characters. This
 *          default value can be changed by the static function SetDelimiter(). Be sure that the
 *          specified Event Names doesn't include any default delimiter characters.
 *
 * @param   parentAreaId    Identifier of an existing Event Area. Use AREAID_ROOT to add an Area
 *                          to the Root Area.
 * @param   areaId          Identifier of the new Event Area. This ID must be unique within the
 *                          Alarms&amp;Events Server.
 * @param   name            Name of the Event Area. Do not use default delimiter characters. See
 *                          comment below.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the Single State
 *           Condition Definition was successfully added to the event space.
 */

HRESULT AddArea(int parentAreaId, int areaId, LPWSTR name);

/**
 * @fn  HRESULT AddSource(int areaId, int sourceId, LPWSTR sourceName, bool multiSource);
 *
 * @brief   Adds an Event Source object to the Alarms&amp;Event Process Area Space.
 *          
 *          Each event generated by an Alarms&amp;Event Server has an associated source object.
 *
 * @param   areaId      Identifier of an existing Event Area. Use AREAID_ROOT to add a top\-level
 *                      Event Source object.
 * @param   sourceId    Identifier of the new Event Source. This ID must be unique within the
 *                      Alarms&amp;Events Server.
 * @param   sourceName  Name of the Event Source. If parameter 'multiSource' is true then this
 *                      parameter specifies the fully qualified source name;
 *                      otherwise only the partial source name.
 * @param   multiSource true if this Event Source object is shared by multiple Process Areas (see
 *                      function AddExistingSource()); otherwise false.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event source was
 *           successfully added to the process area space.
 */

HRESULT AddSource(int areaId, int sourceId, LPWSTR sourceName, bool multiSource);

/**
 * @fn  HRESULT AddExistingSource(int areaId, int sourceId);
 *
 * @brief   Adds an existing Event Source object to an additional Alarms&amp;Event Process Area
 *          Space.
 *          
 *          Use this function if an Event Source object should be a member of multiple Process
 *          Areas.
 *
 * @param   areaId      Identifier of an existing Event Area. Use AREAID_ROOT to add a top\-level
 *                      Event Source object.
 * @param   sourceId    Identifier of an existing Event Source object. The object with this ID
 *                      must be previously added to the Process Area Space by the function
 *                      AddSource() with an active 'multiSource' flag.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event source was
 *           successfully added to the process area space.
 */

HRESULT AddExistingSource(int areaId, int sourceId);

/**
 * @fn  HRESULT AddCondition(int sourceId, int conditionDefinitionId, int conditionId);
 *
 * @brief   Adds an Event Condition to the Alarms&amp;Event Process Area Space.
 *          
 *          An Event Condition associates a Condition Definition with an Event Source. If the
 *          state of a Condition changes then the server creates events.
 *
 * @param   sourceId                Identifier of an existing Event Source.
 * @param   conditionDefinitionId   Identifier of an existing Event Condition Definition.
 * @param   conditionId             Identifier of the new Event Condition. This ID must be unique
 *                                  within the Alarms&amp;Events Server.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event condition
 *           was successfully added to the process area space.
 */

HRESULT AddCondition(int sourceId, int conditionDefinitionId, int conditionId);

/**
 * @fn  HRESULT ProcessSimpleEvent(int categoryId, int sourceId, LPWSTR message, int severity, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp);
 *
 * @brief   Generates a Simple Event.
 *          
 *          Use this function to generate a Single Event. A Single Events is not associated with
 *          an Event Condition. The generic server part uses the subscription specific filters
 *          and forwards the Event to the subscriptions of all connected clients if the filters
 *          are passed.
 *
 * @param   categoryId      Identifier of an existing Event Category. Specifies the Event
 *                          Category to which this Event belongs.
 * @param   sourceId        Identifier of an existing Event Source. Specifies the object which
 *                          generates the event notification.
 * @param   message         Message text string which describes the Event.
 * @param   severity        The urgency of the Event in the range of 1 ... 1000.
 * @param   attributeCount  Number of attribute values specified in pvAttrValues. This number
 *                          must be identical with the number of attributes added to the
 *                          specified Event Category. This parameter is only used for cross\-
 *                          check.
 * @param   attributeValues Array of attribute values. The order and the types must be identical
 *                          with the attributes of the specified Event Category.
 * @param   timeStamp       Specifies the occurrence time of the event. If this parameter is a
 *                          NULL pointer then the current time is used.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event condition
 *           was successfully added to the process area space.
 */

HRESULT ProcessSimpleEvent(int categoryId, int sourceId, LPWSTR message, int severity, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp);

/**
 * @fn  HRESULT ProcessTrackingEvent(int categoryId, int sourceId, LPWSTR message, int severity, LPWSTR actorId, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp);
 *
 * @brief   Generates a Tracking Event.
 *          
 *          Use this function to generate a Tracking Event. A Tracking Events is not associated
 *          with an Event Condition. The generic server part uses the subscription specific
 *          filters and forwards the Event to the subscriptions of all connected clients if the
 *          filters are passed.
 *
 * @param   categoryId      Identifier of an existing Event Category. Specifies the Event
 *                          Category to which this Event belongs.
 * @param   sourceId        Identifier of an existing Event Source. Specifies the object which
 *                          generates the event notification.
 * @param   message         Message text string which describes the Event.
 * @param   severity        The urgency of the Event in the range of 1 ... 1000.
 * @param   actorId         Text string which identifies the OPC Client which initiated the
 *                          action resulting the tracking\-related Event.
 * @param   attributeCount  Number of attribute values specified in pvAttrValues. This number
 *                          must be identical with the number of attributes added to the
 *                          specified Event Category. This parameter is only used for cross\-
 *                          check.
 * @param   attributeValues Array of attribute values. The order and the types must be identical
 *                          with the attributes of the specified Event Category.
 * @param   timeStamp       Specifies the occurrence time of the event. If this parameter is a
 *                          NULL pointer then the current time is used.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event condition
 *           was successfully added to the process area space.
 */

HRESULT ProcessTrackingEvent(int categoryId, int sourceId, LPWSTR message, int severity, LPWSTR actorId, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp);

/**
 * @fn  HRESULT ProcessConditionStateChanges(int count, AeConditionState* conditionStateChanges);
 *
 * @brief   Changes the state of one or more Event Conditions.
 *          
 *          Changes the state of one or more Event Conditions. If the state of a condition
 *          changes then the generic server part creates Event instances .The generic server part
 *          uses the subscription specific filters and forwards the generated Events to the
 *          subscriptions of all connected clients if the filters are passed.
 *
 * @param   count                           Number of Conditions to be changed.
 * @param [in,out]  conditionStateChanges   Array of class ConditionChangeStates with the new
 *                                          condition states.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event condition
 *           was successfully added to the process area space.
 */

HRESULT ProcessConditionStateChanges(int count, AeConditionState* conditionStateChanges);

/**
 * @fn  HRESULT AckCondition(int conditionId, LPWSTR comment);
 *
 * @brief   Internally acknowledgement of an Event Condition.
 *          
 *          Internally acknowledgement of a Condition by the server if it's no longer required to
 *          be acknowledged by a client. The generic function part calls the function
 *          OnAckNotification().
 *
 * @param   conditionId Identifier of an existing Event Condition.
 * @param   comment     Text string with a comment. A NULL pointer means there is no comment.
 *                      This parameter is optional.
 *
 * @return  A HRESULT code with the result of the operation. Returns S_OK if the event condition
 *           was successfully acknowledged.
 */

HRESULT AckCondition(int conditionId, LPWSTR comment);

/**
 * @}
 */

 /**
  * @addtogroup interface_customization Methods called by the generic Server
  * @{
  */

/**
 * @fn  HRESULT DLLCALL OnCreateServerItems();
 *
 * @brief   This method is called from the generic server at the start up;
 *          when the first client connects or the service is started. All items supported by the
 *          server need to be defined by calling the AddItem callback method for each item.
 *          
 *          The Item IDs are fully qualified names ( e.g. Dev1.Chn5.Temp ).
 *          
 *          If <see cref="DaBrowseMode::Generic" text="DaBrowseMode.Generic" />
 *          is set the generic server part creates an appropriate hierarchical address space. The
 *          sample code defines the application item handle as the the buffer array index. This
 *          handle is passed in the calls from the generic server to identify the item. It should
 *          allow quick access to the item definition / buffer. The handle may be implemented
 *          differently depending on the application.
 *          
 *          The branch separator character used in the fully qualified item name must match the
 *          separator character defined in the
 *          <see cref="OnGetDaServerParameters@int*@WCHAR*@DaBrowseMode*" text="OnGetDaServerParameters" />
 *          method.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnCreateServerItems();

/**
 * @fn  ClassicServerDefinition* OnGetDaServerDefinition(void);
 *
 * @brief   This method is called from the generic server at startup for normal operation or for
 *          registration. It provides server registry information for this application required
 *          for DCOM registration. The generic server registers the OPC server accordingly.
 *          
 *          The definitions can be made in the code to prevent them from being changed without a
 *          recompilation.
 *          
 *          The CLSID definitions need to be unique and can be created with the Visual Studio
 *          Create GUIDtool.
 *
 * @return  Definition structure.
 */

DLLEXP ClassicServerDefinition* DLLCALL OnGetDaServerDefinition(void);

/**
 * @fn  ClassicServerDefinition* OnGetAeServerDefinition(void);
 *
 * @brief   This method is called from the generic server at startup for normal operation or for
 *          registration. It provides server registry information for this application required
 *          for DCOM registration. The generic server registers the OPC server accordingly.
 *          
 *          The definitions can be made in the code to prevent them from being changed without a
 *          recompilation.
 *          
 *          The CLSID definitions need to be unique and can be created with the Visual Studio
 *          Create GUIDtool.
 *
 * @return  Definition structure.
 */

DLLEXP ClassicServerDefinition* DLLCALL OnGetAeServerDefinition(void);

/**
 * @fn  HRESULT DLLCALL OnGetDaServerParameters(int* updatePeriod, WCHAR* branchDelimiter, DaBrowseMode* browseMode);
 *
 * @brief   This method is called from the generic server at startup;
 *          when the first client connects or the service is started.
 *          
 *          It defines the application specific server parameters and operating modes. The
 *          definitions can be made in the code to protect them from being changed without a
 *          recompilation.
 *
 * @param [in,out]  updatePeriod    This interval in ms is used by the generic server as the
 *                                  fastest possible client update rate and also uses this
 *                                  definition when determining the refresh need if no client
 *                                  defined a sampling rate for the item.
 * @param [in,out]  branchDelimiter This character is used as the branch/item separator character
 *                                  in fully qualified item names. It is typically '.' or
 *                                  '/'.This character must match the character used in the fully
 *                                  qualified item IDs specified in the AddItems method call.
 * @param [in,out]  browseMode      Defines how client browse calls are handled.0 (Generic) \:
 *                                  all browse calls are handled in the generic server according
 *                                  the items defined in the server cache.1 (Custom) \: all
 *                                  client browse calls are handled in the customization plug\-in
 *                                  and typically return the items that are or could be
 *                                  dynamically added to the server cache.
 *
 * @return  A HRESULT code with the result of the operation. Always returns S_OK.
 */

DLLEXP HRESULT DLLCALL OnGetDaServerParameters(int* updatePeriod, WCHAR* branchDelimiter, DaBrowseMode* browseMode);

/**
 * @fn  void OnStartupSignal(char* commandLine);
 *
 * @brief   This method is called from the generic server when a Startup is executed. It's the
 *          first DLL method called.
 *
 * @param [in,out]  commandLine String with the command line including parameters.
 */

DLLEXP void DLLCALL OnStartupSignal(char* commandLine);

/**
 * @fn  void OnShutdownSignal();
 *
 * @brief   This method is called from the generic server when a Shutdown is executed. To ensure
 *          proper process shutdown, any communication channels should be closed and all threads
 *          terminated before this method returns.
 */

DLLEXP void DLLCALL OnShutdownSignal();

/**
 * @brief    Query the properties defined for the specified item.
 *
 * @param    deviceItemHandle    Application item handle.
 * @param    noProp                Number of properties returned.
 * @param    iDs                   Array with the the property ID number.
 *
 * @return    A HRESULT code with the result of the operation. S_FALSE if the item has no custom
 *             properties.
 */

DLLEXP HRESULT DLLCALL OnQueryProperties(
    void*   deviceItemHandle,
					int*  noProp,
    int**    iDs);

/**
 * @fn  HRESULT DLLCALL OnGetPropertyValue( void* deviceItemHandle, int propertyId, LPVARIANT propertyValue);
 *
 * @brief   Returns the values of the requested custom properties of the requested item. This
 *          method is not called for the OPC standard properties 1..8. These are handled in the
 *          generic server.
 *
 * @param [in,out]  deviceItemHandle    Item application handle.
 * @param   propertyId                  ID of the property.
 * @param   propertyValue               Property value.
 *
 * @return  HRESULT success/error code. S_FALSE if the item has no custom properties.
 */

DLLEXP HRESULT DLLCALL OnGetPropertyValue(
    void*        deviceItemHandle,
    int          propertyId,
    LPVARIANT    propertyValue);

/**
 * @fn  HRESULT DLLCALL OnWriteItems( int numItems, void ** deviceItemHandles, OPCITEMVQT* itemVQTs, HRESULT * errors);
 *
 * @brief   This method is called when a client executes a 'write' server call. The items
 *          specified in the OPCITEMVQT array need to be written to the device.
 *          
 *          The cache is updated in the generic server after returning from the customization
 *          WiteItems method. Items with write error are not updated in the cache.
 *
 * @param   numItems                    Number of defined item handles.
 * @param [in,out]  deviceItemHandles   Array with the application handles.
 * @param [in,out]  itemVQTs            Object with handle, value, quality, timestamp.
 * @param [in,out]  errors              Array with HRESULT success/error codes on return.
 *
 * @return  A result code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnWriteItems(
					int          numItems,
    void      ** deviceItemHandles,
    OPCITEMVQT*  itemVQTs,
    HRESULT   *  errors);

/**
 * @fn  HRESULT DLLCALL OnRefreshItems( int numItems, void ** deviceItemHandles);
 *
 * @brief   Refresh the items listed in the appHandles array in the cache.
 *          
 *          This method is called when a client executes a read from device. The device read is
 *          called with all client requested items.
 *
 * @param   numItems                    Number of defined item handles.
 * @param [in,out]  deviceItemHandles   Array with the application handle of the items that need
 *                                      to be refreshed.
 *
 * @return  A result code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnRefreshItems(
					int        numItems,
    void    ** deviceItemHandles);

/**
 * @fn  HRESULT DLLCALL OnBrowseChangePosition( DaBrowseDirection browseDirection, LPCWSTR position, LPWSTR * actualPosition);
 *
 * @brief   Custom mode browse handling. Provides a way to move 'up' or 'down' or 'to' in a
 *          hierarchical space.
 *          
 *          Called only from the generic server when
 *          <see cref="DaBrowseMode::Custom" text="DaBrowseMode.Custom" />
 *          is configured.
 *          
 *          Change the current browse branch to the specified branch in virtual address space.
 *          This method has to be implemented according the OPC DA specification. The generic
 *          server calls this function for OPC DA 2.05a and OPC DA 3.00 client calls. The
 *          differences between the specifications is handled within the generic server part.
 *          Please note that flat address space is not supported.
 *          
 *          An error is returned if the passed string does not represent a 'branch'.
 *          
 *          Moving Up from the 'root' will return E_FAIL.
 *          
 *          \Note DaBrowseDirection.To is new for DA version 2.0. Clients should be prepared to
 *          handle E_INVALIDARG if they pass this to a DA 1.0 server.
 *
 * @param   browseDirection         DaBrowseDirection.To, DaBrowseDirection.Up or
 *                                  DaBrowseDirection.Down.
 * @param   position                New absolute or relative branch. If
 *                                  <see cref="DaBrowseDirection::Down" text="DaBrowseDirection.Down" />
 *                                  the branch where to change and if
 *                                  <see cref="DaBrowseDirection" />.To the fully qualified name
 *                                  where to change or NULL to go to the 'root'.For
 *                                  <see cref="DaBrowseDirection::Down" text="DaBrowseDirection.Down" />,
 *                                  the name of the branch to move into. This would be one of the
 *                                  strings returned from <see cref="OnBrowseItemIDs" />.
 *                                  E.g. REACTOR10For
 *                                  <see cref="DaBrowseDirection::Up" text="DaBrowseDirection.Up" />
 *                                  this parameter is ignored and should point to a NULL
 *                                  string.For
 *                                  <see cref="DaBrowseDirection::To" text="DaBrowseDirection.To" />
 *                                  a fully qualified name (e.g. as returned from GetItemID) or a
 *                                  pointer to a NULL string to go to the 'root'. E.g.
 *                                  AREA1.REACTOR10.TIC1001.
 * @param [in,out]  actualPosition  Actual position in the address tree for the calling client.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnBrowseChangePosition(
    DaBrowseDirection browseDirection,
    LPCWSTR position,
    LPWSTR * actualPosition);

/**
 * @fn  HRESULT DLLCALL OnBrowseItemIds( LPWSTR actualPosition, DaBrowseType browseFilterType, LPWSTR filterCriteria, VARTYPE dataTypeFilter, DaAccessRights accessRightsFilter, int * noItems, LPWSTR ** itemIds);
 *
 * @brief   Custom mode browse handling.
 *          
 *          Called only from the generic server when
 *          <see cref="DaBrowseMode::Custom" text="DaBrowseMode.Custom" />
 *          is configured.
 *          
 *          This method browses the items in the current branch of the virtual address space. The
 *          position from the which the browse is done can be set via
 *          <see cref="OnBrowseChangePosition" />. The generic server calls this function for OPC
 *          DA 2.05a and OPC DA 3.00 client calls. The differences between the specifications is
 *          handled within the generic server part. Please note that flat address space is not
 *          supported.
 *          
 *          The returned enumerator may have nothing to enumerate if no ItemIDs satisfied the
 *          filter constraints. The strings returned by the enumerator represent the BRANCHs and
 *          LEAFS contained in the current level. They do NOT include any delimiters or 'parent'
 *          names.
 *          
 *          Whenever possible the server should return strings which can be passed directly to
 *          AddItems. However, it is allowed for the Server to return a 'hint' string rather than
 *          an actual legal Item ID. For example a PLC with 32000 registers could return a single
 *          string of �0 to 31999� rather than return 32,000 individual strings from the
 *          enumerator. For this reason (as well as the fact that browser support is optional)
 *          clients should always be prepared to allow manual entry of ITEM ID strings. In the
 *          case of 'hint' strings, there is no indication given as to whether the returned
 *          string will be acceptable by AddItem or ValidateItem.
 *          
 *          Clients are allowed to get and hold Enumerators for more than one 'browse position'
 *          at a time.
 *          
 *          Changing the browse position will not affect any String Enumerator the client already
 *          has.
 *          
 *          The client must Release each Enumerator when he is done with it.
 *
 * @param   actualPosition      Position in the server address space (ex. "INTERBUS1.DIGIN") for
 *                              the calling client.
 * @param   browseFilterType    Branch/Leaf filter\:
 *                              <see cref="DaBrowseType" />Branch\: @Returns only
 *                              items that have children Leaf\: @Returns only items that don't
 *                              have children Flat\: @Returns everything at and below this level
 *                              including all children of children
 *                              * basically 'pretends' that
 *                              the address space is actually FLAT.
 * @param   filterCriteria      name pattern match expression, e.g. "*".
 * @param   dataTypeFilter      Filter the returned list based on the available datatypes (those
 *                              that would succeed if passed to AddItem). System.Void indicates
 *                              no filtering.
 * @param   accessRightsFilter  Filter based on the DaAccessRights bit mask
 *                              <see cref="DaAccessRights" />.
 * @param [in,out]  noItems     Number of items returned.
 * @param [in,out]  itemIds     Items meeting the browse criteria.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnBrowseItemIds(
    LPWSTR actualPosition,
					DaBrowseType browseFilterType,
    LPWSTR filterCriteria,
    VARTYPE dataTypeFilter,
    DaAccessRights accessRightsFilter,
    int * noItems,
    LPWSTR ** itemIds);

/**
 * @fn  HRESULT DLLCALL OnBrowseGetFullItemId( LPWSTR actualPosition, LPWSTR itemName, LPWSTR * fullItemId);
 *
 * @brief   Custom mode browse handling.
 *          
 *          Called only from the generic server when
 *          <see cref="DaBrowseMode::Custom" text="DaBrowseMode.Custom" />
 *          is configured.
 *          
 *          This method returns the fully qualified name of the specified item in the current
 *          branch in the virtual address space. This name is used to add the item to the real
 *          address space. The generic server calls this function for OPC DA 2.05a and OPC DA
 *          3.00 client calls. The differences between the specifications is handled within the
 *          generic server part. Please note that flat address space is not supported.
 *          
 *          Provides a way to assemble a 'fully qualified' ITEM ID in a hierarchical space. This
 *          is required since the browsing functions return only the components or tokens which
 *          make up an ITEMID and do NOT return the delimiters used to separate those tokens.
 *          Also, at each point one is browsing just the names 'below' the current node (e.g. the
 *          'units' in a 'cell').
 *          
 *          A client would browse down from AREA1 to REACTOR10 to TIC1001 to CURRENT_VALUE. As
 *          noted earlier the client sees only the components, not the delimiters which are
 *          likely to be very server specific. The function rebuilds the fully qualified name
 *          including the vendor specific delimiters for use by ADDITEMs. An extreme example
 *          might be a server that returns: \\AREA1:REACTOR10.TIC1001[CURRENT_VALUE]
 *          
 *          It is also possible that a server could support hierarchical browsing of an address
 *          space that contains globally unique tags. For example in the case above, the tag
 *          TIC1001.CURRENT_VALUE might still be globally unique and might therefore be
 *          acceptable to AddItem. However the expected behavior is that (a) GetItemID will
 *          always return the fully qualified name (AREA1.REACTOR10.TIC1001.CURRENT_VALUE) and
 *          that (b) that the server will always accept the fully qualified name in AddItems
 *          (even if it does not require it).
 *          
 *          It is valid to form an ItemID that represents a BRANCH (e.g. AREA1.REACTOR10). This
 *          could happen if you pass a BRANCH (AREA1) rather than a LEAF (CURRENT_VALUE). The
 *          resulting string might fail if passed to AddItem but could be passed to
 *          ChangeBrowsePosition using OPC_BROWSE_TO.
 *          
 *          The client must free the returned string.
 *          
 *          ItemID is the unique 'key' to the data, it is considered the 'what' or 'where' that
 *          allows the server to connect to the data source.
 *
 * @param   actualPosition      Fully qualified name of the current branch.
 * @param   itemName            The name of a BRANCH or LEAF at the current level. Or a pointer
 *                              to a NULL string. Passing in a NULL string results in a return
 *                              string which represents the current position in the hierarchy.
 * @param [in,out]  fullItemId  Fully qualified name if the item. This name is used to access the
 *                              item or add it to a group.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnBrowseGetFullItemId(
    LPWSTR actualPosition,
    LPWSTR itemName,
    LPWSTR * fullItemId);

/**
 * @fn  HRESULT DLLCALL OnClientConnect();
 *
 * @brief   This method is called when a client connects to the OPC server. If the method returns
 *          an error code then the client connect is refused.
 *
 * @return  A HRESULT code with the result of the operation. S_OK allows the client to connect to
 *          he server.
 */

DLLEXP HRESULT DLLCALL OnClientConnect();

/**
 * @fn  HRESULT DLLCALL OnClientDisconnect();
 *
 * @brief   This method is called when a client disconnects from the OPC server.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnClientDisconnect();

/**
 * @fn  HRESULT DLLCALL OnGetDaOptimizationParameters( bool * useOnRequestItems, bool * useOnRefreshItems, bool * useOnAddItem, bool * useOnRemoveItem);
 *
 * @brief   This method is called from the generic server at startup; when the first client
 *          connects or the service is started.
 *          
 *          It defines the application specific optimization parameters.
 *
 * @param [in,out]  useOnRequestItems   Specifiy whether OnRequestItems is called by the generic
 *                                      server; default is true.
 * @param [in,out]  useOnRefreshItems   Specifiy whether OnRefreshItems is called by the generic
 *                                      server; default is true.
 * @param [in,out]  useOnAddItem        Specifiy whether OnAddItem is called by the generic
 *                                      server; default is false.
 * @param [in,out]  useOnRemoveItem     Specifiy whether OnRemoveItem is called by the generic
 *                                      server; default is false.
 *
 * @return  A result code with the result of the operation. Always returns S_OK.
 */

DLLEXP HRESULT DLLCALL OnGetDaOptimizationParameters(
    bool * useOnRequestItems,
    bool * useOnRefreshItems,
    bool * useOnAddItem,
    bool * useOnRemoveItem);

/**
 * @fn  HRESULT DLLCALL OnAddItem(void* deviceItemHandle);
 *
 * @brief   The device item was added to a group or gets used for item based read/write.
 *          
 *          This method is called when a client adds the items to a group or use item based
 *          read/write functions.
 *
 * @param [in,out]  deviceItemHandle    Device Item that need to be updated.
 *
 * @return  A result code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnAddItem(void* deviceItemHandle);

/**
 * @fn  HRESULT DLLCALL OnRemoveItem(void* deviceItemHandle);
 *
 * @brief   The device item is no longer used by clients.
 *          
 *          This method is called when a client removes an item from a group or no longer use the
 *          item in item based read/write functions. Only an item is specified which is no longer
 *          used by at least one client.
 *
 * @param [in,out]  deviceItemHandle    Device Item that no longer need to be updated.
 *
 * @return  A result code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnRemoveItem(void* deviceItemHandle);

/**
 * @fn  HRESULT DLLCALL OnAckNotification(int conditionId, int subConditionId);
 *
 * @brief   Notification if an Event Condition has been acknowledged.
 *          
 *          Called by the generic server part if the Event Condition specified by the parameters
 *          has been acknowledged. This function is called if the Event Condition is successfully
 *          acknowledged but before the indication events are sent to the clients. If this
 *          function fails then the error code will be returned to the client and no indication
 *          events will be generated.
 *
 * @param   conditionId     Event Category Identifier.
 * @param   subConditionId  Sub Condition Definition Identifier. It's 0 for Single State
 *                          Conditions.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT  DLLCALL OnAckNotification(int conditionId, int subConditionId);

/**
 * @fn  HRESULT DLLCALL OnTranslateToItemId(int conditionId, int subConditionId, int attributeId, LPWSTR* itemId, LPWSTR* nodeName, CLSID* clsid);
 *
 * @brief   Returns information about the OPC DA Item corresponding to an Event Attribute.
 *          
 *          Called by the generic server part to get information about the OPC DA Item
 *          corresponding to an Event Attribute.
 *
 * @param   conditionId         Event Category Identifier.
 * @param   subConditionId      Sub Condition Definition Identifier. It's 0 for Single State
 *                              Conditions.
 * @param   attributeId         Event Attribute Identifier.
 * @param [in,out]  itemId      Pointer to where the text string with the ItemID of the
 *                              associated OPC DA Item will be saved. Use a null string if there is
 *                              no OPC Item corresponding to the Event Attribute specified by the
 *                              parameters conditionId, subConditionId and attributeId.
 * @param [in,out]  nodeName    Pointer to where the text string with the network node name of
 *                              the associated OPC Data Access Server will be saved. Use a null
 *                              string if the server is running on the local node.
 * @param [in,out]  clsid       CLSID of the associated Data Access Server will be saved. Use the
 *                              value null if there is no associated OPC Item.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT  DLLCALL OnTranslateToItemId(int conditionId, int subConditionId, int attributeId, LPWSTR* itemId, LPWSTR* nodeName, CLSID* clsid);

/**
 * @fn  LogLevel OnGetLogLevel();
 *
 * @brief   Gets the logging level to be used.
 *
 * @return  A LogLevel.
 */

DLLEXP LogLevel DLLCALL OnGetLogLevel();

/**
 * @fn  void OnGetLogPath(char * logPath);
 *
 * @brief   Gets the logging pazh to be used.
 *
 * @param [in,out]  logPath    Path to be used for logging.
 */

DLLEXP void DLLCALL OnGetLogPath(char * logPath);


/**
 * @fn  HRESULT DLLCALL OnRequestItems(int numItems, LPWSTR *fullItemIds, VARTYPE *dataTypes);
 *
 * @brief   This method is called when the client accesses items that do not yet exist in the
 *          server's cache.
 *          
 *          OPC DA 2.00 clients typically first call AddItems() or ValidateItems(). OPC DA 3.00
 *          client may access items directly using the ItemIO read/write functions. Within this
 *          function it is possible to:
 *          
 *          * add the items to the servers real address space and return
 *            S_OK. For each item to be added the callback method 'AddItem' has to be called.
 *          * return S_FALSE.
 *
 * @param   numItems            Number of defined item handles.
 * @param [in,out]  fullItemIds Names of the items which does not exist in the server's cache.
 * @param [in,out]  dataTypes   Data Types requested by the client of the items which does not
 *                              exist in the server's cache.
 *
 * @return  A HRESULT code with the result of the operation.
 */

DLLEXP HRESULT DLLCALL OnRequestItems(int numItems, LPWSTR *fullItemIds, VARTYPE *dataTypes);

/**
 * @}
 */

//----------------------------------------------------------------------------
// Type Definitions
//----------------------------------------------------------------------------
typedef HRESULT(DLLCALL * AddItemPtr)(LPWSTR, DaAccessRights, LPVARIANT, bool, DaEuType, double, double, void**);
typedef HRESULT(DLLCALL * RemoveItemPtr)(void*);
typedef HRESULT(DLLCALL * AddPropertyPtr)(int, LPWSTR, LPVARIANT);
typedef HRESULT(DLLCALL * SetItemValuePtr)(void*, LPVARIANT, short, FILETIME);
typedef void (DLLCALL * SetServerStatePtr)(ServerState);
typedef void (DLLCALL * GetActiveItemsPtr)(int * dwNumItemHandles, void* ** ppItemHandles);

typedef HRESULT(DLLCALL * AddSimpleEventCategoryPtr)(int, LPWSTR);
typedef HRESULT(DLLCALL * AddTrackingEventCategoryPtr)(int, LPWSTR);
typedef HRESULT(DLLCALL * AddConditionEventCategoryPtr)(int, LPWSTR);

typedef HRESULT(DLLCALL * AddEventAttributePtr)(int, int, LPWSTR, VARTYPE);

typedef HRESULT(DLLCALL * AddSingleStateConditionDefinitionPtr)(int, int, LPWSTR, LPWSTR, int, LPWSTR, bool);

typedef HRESULT(DLLCALL * AddMultiStateConditionDefinitionPtr)(int, int, LPWSTR);
typedef HRESULT(DLLCALL * AddSubConditionDefinitionPtr)(int, int, LPWSTR, LPWSTR, int, LPWSTR, bool);

typedef HRESULT(DLLCALL * AddAreaPtr)(int, int, LPWSTR);
typedef HRESULT(DLLCALL * AddSourcePtr)(int, int, LPWSTR, bool);
typedef HRESULT(DLLCALL * AddExistingSourcePtr)(int, int);
typedef HRESULT(DLLCALL * AddConditionPtr)(int, int, int);

typedef HRESULT(DLLCALL * ProcessSimpleEventPtr)(int, int, LPWSTR, int, int, LPVARIANT pvAttrValues, LPFILETIME);
typedef HRESULT(DLLCALL * ProcessTrackingEventPtr)(int, int, LPWSTR, int, LPWSTR, int, LPVARIANT, LPFILETIME);
typedef HRESULT(DLLCALL * ProcessConditionStateChangesPtr)(int, IClassicBaseNodeManager::AeConditionState*);
typedef HRESULT(DLLCALL * AckConditionPtr)(int, LPWSTR);

typedef void (DLLCALL * FireShutdownRequestPtr)(LPCWSTR reason);
typedef void (DLLCALL * GetClientsPtr)(int * numClientHandles, void* ** clientHandles, LPWSTR ** clientNames);
typedef void (DLLCALL * GetGroupsPtr)(void * clientHandle, int * numGroupHandles, void* ** groupHandles, LPWSTR ** groupNames);
typedef void (DLLCALL * GetGroupStatePtr)(void * groupHandle, DaGroupState * groupState);
typedef void (DLLCALL * GetItemStatesPtr)(void * groupHandle, int * numDaItemStates, DaItemState* * daItemStates);
    };
#endif

