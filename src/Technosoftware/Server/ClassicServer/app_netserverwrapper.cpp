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

//-----------------------------------------------------------------------------
// INLCUDE
//-----------------------------------------------------------------------------
#include "StdAfx.h"
#include "malloc.h"
#include <comdef.h>
#include "DaServer.h"
#include "AeServer.h"
#include "app_netserver.h"

//-----------------------------------------------------------------------------
// DLL FUNCTION IMPLEMENTATION
//-----------------------------------------------------------------------------

HRESULT DLLCALL OnDefineCallbacks()
{
	return GenericServerAPI::OnDefineCallbacks();
}

/// <summary>
/// This method is called from the generic server at the startup;
/// when the first client connects or the service is started. All
/// items supported by the server need to be defined by calling
/// the AddItem callback method for each item.
/// 
/// The Item IDs are fully qualified names ( e.g. Dev1.Chn5.Temp
/// ).
/// 
/// If <see cref="TsDaBrowseMode::Generic" text="TsDaBrowseMode.Generic" />
/// is set the generic server part creates an approriate
/// hierachical address space. The sample code defines the
/// application item handle as the the buffer array index. This
/// handle is passed in the calls from the generic server to
/// identify the item. It should allow quick access to the item
/// definition / buffer. The handle may be implemented
/// differently depending on the application.
/// 
/// The branch separator character used in the fully qualified
/// item name must match the separator character defined in the <see cref="OnGetDAServerParameters@int*@WCHAR*@TsDaBrowseMode*" text="OnGetDAServerParameters" />
/// method.
/// 
/// 
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>
/// <param name="cmdParamas">String with the command line
///                          \parameters as they were specified
///                          when the server was being
///                          registered.</param>                                                                                                                 
 HRESULT DLLCALL OnCreateServerItems( char*    cmdParamas )
{
	return GenericServerAPI::OnCreateServerItems(T2A((LPTSTR)cmdParamas));

}


/// <summary>
/// Gets a license key
/// </summary>
/// <param name="licenseOwner ">Owner of the license</param>
/// <param name="serialNumber">Serial Number</param>
void DLLCALL OnGetLicenseInformation( char * licenseOwner, char * serialNumber )
{
	GenericServerAPI::OnGetLicenseInformation(licenseOwner, serialNumber);
}


/// <summary>
/// This method is called from the generic server at startup for
/// normal operation or for registration. It provides server
/// registry information for this application required for DCOM
/// registration. The generic server registers the OPC server
/// accordingly.
/// 
/// The definitions can be made in the code to prevent them from
/// being changed without a recompilation.
/// 
/// 
/// </summary>
/// <returns>
/// Definition structure
/// </returns>
/// <remarks>
/// The CLSID definitions need to be unique and can be created
/// with the Visual Studio Create GUIDtool.
/// 
/// 
/// </remarks>                                                  
ServerRegDefs1* DLLCALL OnGetDAServerRegistryDefinition( void )
{
	return GenericServerAPI::OnGetDAServerRegistryDefinition();
}


/// <summary>
/// This method is called from the generic server at startup for
/// normal operation or for registration. It provides server
/// registry information for this application required for DCOM
/// registration. The generic server registers the OPC server
/// accordingly.
/// 
/// The definitions can be made in the code to prevent them from
/// being changed without a recompilation.
/// 
/// 
/// </summary>
/// <returns>
/// Definition structure
/// </returns>
/// <remarks>
/// The CLSID definitions need to be unique and can be created
/// with the Visual Studio Create GUIDtool.
/// 
/// 
/// </remarks>                                                  
ServerRegDefs1* DLLCALL OnGetAEServerRegistryDefinition( void )
{
	return GenericServerAPI::OnGetAEServerRegistryDefinition();
}


/// <summary>
/// This method is called from the generic server at startup;
/// when the first client connects or the service is started.
/// 
/// It defines the application specific server parameters and
/// operating modes. The definitions can be made in the code to
/// protect them from being changed without a recompilation.
/// 
/// 
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation. Always
/// \returns S_OK
/// </returns>
/// <param name="updatePeriod">This interval in ms is used by
///                            the generic server as the
///                            fastest possible client update
///                            rate and also uses this
///                            definition when determining
///                            the refresh need if no client
///                            defined a sampling rate for
///                            the item.</param>
/// <param name="branchDelimiter">This character is used as the
///                               branch/item separator
///                               character in fully qualified
///                               item names. It is typically
///                               '.' or '/'.<para></para>This
///                               character must match the
///                               character used in the fully
///                               qualified item IDs specified
///                               in the AddItems method call.<para></para></param>
/// <param name="browseMode">Defines how client browse
///                          calls are handled.<para></para>0
///                          (Generic) \: all browse calls
///                          are handled in the generic
///                          server according the items
///                          defined in the server cache.<para></para>1
///                          (Custom) \: all client browse
///                          calls are handled in the
///                          customization plug\-in and
///                          typically return the items
///                          that are or could be
///                          dynamically added to the
///                          server cache.<para></para></param>                    
 HRESULT DLLCALL OnGetDAServerParameters( int* updatePeriod, WCHAR* branchDelimiter, int* browseMode)
{
	return GenericServerAPI::OnGetDAServerParameters(updatePeriod, branchDelimiter, browseMode);
}


/// <summary>
/// This method is called from the generic server at startup; when the first client connects or the service is started.
///
/// It defines the application specific optimization parameters.
/// </summary>
/// <param name="useOnItemRequest">Specifiy whether OnItemRequest is called by the generic server; default is true</param>
/// <param name="useOnRefreshItems">Specifiy whether OnRefreshItems is called by the generic server; default is true</param>
/// <param name="useOnAddItems">Specifiy whether OnAddItems is called by the generic server; default is false</param>
/// <param name="useOnRemoveItems">Specifiy whether OnRemoveItems is called by the generic server; default is false</param>
/// <returns>
/// A result code with the result of the operation. Always returns S_OK
/// </returns>
 HRESULT DLLCALL OnGetDAOptimizationParameters( 
	bool * useOnItemRequest, 
	bool * useOnRefreshItems, 
	bool * useOnAddItems, 
	bool * useOnRemoveItems )
{
	return GenericServerAPI::OnGetDAOptimizationParameters(useOnItemRequest, useOnRefreshItems, useOnAddItems, useOnRemoveItems);
}


/// <summary>
/// 	<para>This method is called from the generic server when a Shutdown is
///     executed.</para>
/// 	<para>To ensure proper process shutdown, any communication channels should be
///     closed and all threads terminated before this method returns.</para>
/// </summary>
 void DLLCALL OnShutdownSignal( void )
{
	GenericServerAPI::OnShutdownSignal();
}


/// <summary>
/// Query the properties defined for the specified item
/// </summary>
/// <param name="ItemHandle">Application item handle</param>
/// <param name="noProp">Number of properties returned</param>
/// <param name="IDs">Array with the the property ID
///                   number</param>
/// <returns>
/// A HRESULT code with the result of the operation. S_FALSE if
/// the item has no custom properties.
/// </returns>                                                 
 HRESULT DLLCALL OnQueryProperties(				   
	int   itemHandle, 
	int*  noProp,
	int** IDs )

{
	return GenericServerAPI::OnQueryProperties(itemHandle,noProp,IDs);
}


/// <summary>
/// Returns the values of the requested custom properties of the requested item. This
/// method is not called for the OPC standard properties 1..8. These are handled in the
/// generic server.
/// </summary>
/// <returns>HRESULT success/error code. S_FALSE if the item has no custom properties.</returns>
/// <param name="ItemHandle">Item application handle</param>
/// <param name="propertyID">ID of the property</param>
/// <param name="propertyValue">Property value</param>
 HRESULT DLLCALL OnGetPropertyValue(				   
	int itemHandle, 
	int propertyID,
	LPVARIANT propertyValue )
{
	HRESULT hr = GenericServerAPI::OnGetPropertyValue(itemHandle,propertyID,propertyValue);
	return hr;
}


//----------------------------------------------------------------------------
// Server Developer Studio DLL API Dynamic address space Handling Methods 
// (Called by the generic server)
//----------------------------------------------------------------------------

/// <summary>
/// This method is called when the client accesses items that do
/// not yet exist in the server's cache.
/// 
/// OPC DA 2.00 clients typically first call AddItems() or
/// ValidateItems(). OPC DA 3.00 client may access items directly
/// using the ItemIO read/write functions. Within this function
/// it is possible to:
/// 
///   * add the item to the servers real address space and return
///     S_OK. For each item to be added the callback method 'AddItem'
///     has to be called.
///   * return S_FALSE
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>
/// <param name="fullItemId">Name of the item which does not
///                          exist in the server's cache</param>     
 HRESULT DLLCALL OnItemRequest(LPWSTR fullItemId)
{
	return GenericServerAPI::OnItemRequest(fullItemId);
}


/// <summary>
/// Custom mode browse handling. Provides a way to move �up� or
/// �down� or 'to' in a hierarchical space.
/// 
/// Called only from the generic server when <see cref="TsDaBrowseMode::Custom" text="TsDaBrowseMode.Custom" />
/// is configured.
/// 
/// Change the current browse branch to the specified branch in
/// virtual address space. This method has to be implemented
/// according the OPC DA specification. The generic server calls
/// this function for OPC DA 2.05a and OPC DA 3.00 client calls.
/// The differences between the specifications is handled within
/// the generic server part. Please note that flat address space
/// is not supported.
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>
/// <remarks>
/// An error is returned if the passed string does not represent
/// a �branch�.
/// 
/// Moving Up from the �root� will return E_FAIL.
/// 
/// \Note TsDaBrowseDirection.To is new for DA version 2.0.
/// Clients should be prepared to handle E_INVALIDARG if they
/// pass this to a DA 1.0 server.
/// </remarks>
/// <param name="browseDirection">TsDaBrowseDirection.To,
///                               TsDaBrowseDirection.Up or
///                               TsDaBrowseDirection.Down</param>
/// <param name="position">New absolute or relative
///                        branch. If <see cref="TsDaBrowseDirection::Down" text="TsDaBrowseDirection.Down" />
///                        the branch where to change and
///                        if <see cref="TsDaBrowseDirection" />.To
///                        the fully qualified name where
///                        to change or NULL to go to the
///                        'root'.<para></para>For <see cref="TsDaBrowseDirection::Down" text="TsDaBrowseDirection.Down" />,
///                        the name of the branch to move
///                        into. This would be one of the
///                        strings returned from <see cref="OnBrowseItemIDs" />.
///                        E.g. REACTOR10<para></para>For
///                        <see cref="TsDaBrowseDirection::Up" text="TsDaBrowseDirection.Up" />
///                        this parameter is ignored and
///                        should point to a NULL string.<para></para>For
///                        <see cref="TsDaBrowseDirection::To" text="TsDaBrowseDirection.To" />
///                        a fully qualified name (e.g.
///                        as returned from GetItemID) or
///                        a pointer to a NULL string to
///                        go to the 'root'. E.g.
///                        AREA1.REACTOR10.TIC1001<para></para></param>
/// <param name="actualPosition">Actual position in the address
///                              tree for the calling client.</param>                                                       
HRESULT DLLCALL OnBrowseChangePos(
	int browseDirection, 
	LPCWSTR position, 
	LPWSTR * szActualPosition)
{
	return GenericServerAPI::OnBrowseChangePosition( browseDirection, position, szActualPosition );
}


/// <summary>
/// Custom mode browse handling.
/// 
/// Called only from the generic server when <see cref="TsDaBrowseMode::Custom" text="TsDaBrowseMode.Custom" />
/// is configured.
/// 
/// This method browses the items in the current branch of the
/// virtual address space. The position from the which the browse
/// is done can be set via <see cref="OnBrowseChangePosition" />.
/// The generic server calls this function for OPC DA 2.05a and
/// OPC DA 3.00 client calls. The differences between the
/// specifications is handled within the generic server part.
/// Please note that flat address space is not supported.
/// 
/// 
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation.
/// 
/// 
/// 
/// 
/// </returns>
/// <remarks>
/// The returned enumerator may have nothing to enumerate if no
/// ItemIDs satisfied the filter constraints. The strings
/// returned by the enumerator represent the BRANCHs and LEAFS
/// contained in the current level. They do NOT include any
/// delimiters or �parent� names.
/// 
/// Whenever possible the server should return strings which can
/// be passed directly to AddItems. However, it is allowed for
/// the Server to return a �hint� string rather than an actual
/// legal Item ID. For example a PLC with 32000 registers could
/// return a single string of �0 to 31999� rather than return
/// 32,000 individual strings from the enumerator. For this
/// reason (as well as the fact that browser support is optional)
/// clients should always be prepared to allow manual entry of
/// ITEM ID strings. In the case of �hint� strings, there is no
/// indication given as to whether the returned string will be
/// acceptable by AddItem or ValidateItem.
/// 
/// Clients are allowed to get and hold Enumerators for more than
/// one �browse position� at a time.
/// 
/// Changing the browse position will not affect any String
/// Enumerator the client already has.
/// 
/// The client must Release each Enumerator when he is done with
/// it.
/// 
/// 
/// </remarks>
/// <param name="actualPosition">Position in the server
///                              address space (ex.
///                              "INTERBUS1.DIGIN") for the
///                              calling client </param>
/// <param name="browseFilterType">Branch/Leaf filter\: <see cref="TsDaBrowseType" /><para></para>Branch\:
///                                \returns only items that
///                                have children Leaf\:
///                                \returns only items that
///                                don't have children Flat\:
///                                \returns everything at and
///                                below this level including
///                                all children of children
///                                * basically 'pretends' that
///                                  the address space is
///                                  actually FLAT
///                                </param>
/// <param name="filterCriteria">name pattern match
///                              expression, e.g. "*"</param>
/// <param name="dataTypeFilter">Filter the returned list
///                              based on the available
///                              datatypes (those that would
///                              succeed if passed to
///                              AddItem). System.Void
///                              indicates no filtering. </param>
/// <param name="accessRightsFilter">Filter based on the
///                                  AccessRights bit mask <see cref="TsDaAccessRights" />.
///                                  </param>
/// <param name="noItems">Number of items returned</param>
/// <param name="itemIDs">Items meeting the browse
///                       criteria.</param>                                                                    
 HRESULT DLLCALL OnBrowseItemID(				   
									   LPWSTR actualPosition, 
									   tagOPCBROWSETYPE browseFilterType,
									   LPWSTR filterCriteria, 
									   VARTYPE dataTypeFilter, 
									   DWORD accessRightsFilter, 
									   DWORD * noItems, 
									   LPWSTR ** itemIDs )
{
	return GenericServerAPI::OnBrowseItemIDs( 
			actualPosition, browseFilterType, 
			filterCriteria, dataTypeFilter,
			accessRightsFilter,
			noItems, itemIDs );
}


/// <summary>
/// Custom mode browse handling.
/// 
/// Called only from the generic server when <see cref="TsDaBrowseMode::Custom" text="TsDaBrowseMode.Custom" />
/// is configured.
/// 
/// This method returns the fully qualified name of the specified
/// item in the current branch in the virtual address space. This
/// name is used to add the item to the real address space. The
/// generic server calls this function for OPC DA 2.05a and OPC
/// DA 3.00 client calls. The differences between the
/// specifications is handled within the generic server part.
/// Please note that flat address space is not supported.
/// 
/// 
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>
/// <remarks>
/// Provides a way to assemble a �fully qualified� ITEM ID in a
/// hierarchical space. This is required since the browsing
/// functions return only the components or tokens which make up
/// an ITEMID and do NOT return the delimiters used to separate
/// those tokens. Also, at each point one is browsing just the
/// names �below� the current node (e.g. the �units� in a
/// �cell�).
/// 
/// A client would browse down from AREA1 to REACTOR10 to TIC1001
/// to CURRENT_VALUE. As noted earlier the client sees only the
/// components, not the delimiters which are likely to be very
/// server specific. The function rebuilds the fully qualified
/// name including the vendor specific delimiters for use by
/// ADDITEMs. An extreme example might be a server that returns:
/// \\AREA1:REACTOR10.TIC1001[CURRENT_VALUE]
/// 
/// It is also possible that a server could support hierarchical
/// browsing of an address space that contains globally unique
/// tags. For example in the case above, the tag
/// TIC1001.CURRENT_VALUE might still be globally unique and
/// might therefore be acceptable to AddItem. However the
/// expected behavior is that (a) GetItemID will always return
/// the fully qualified name
/// (AREA1.REACTOR10.TIC1001.CURRENT_VALUE) and that (b) that the
/// server will always accept the fully qualified name in
/// AddItems (even if it does not require it).
/// 
/// It is valid to form an ItemID that represents a BRANCH (e.g.
/// AREA1.REACTOR10). This could happen if you pass a BRANCH
/// (AREA1) rather than a LEAF (CURRENT_VALUE). The resulting
/// string might fail if passed to AddItem but could be passed to
/// ChangeBrowsePosition using OPC_BROWSE_TO.
/// 
/// The client must free the returned string.
/// 
/// ItemID is the unique �key� to the data, it is considered the
/// �what� or �where� that allows the server to connect to the
/// data source.
/// 
/// 
/// </remarks>
/// <param name="actualPosition">Fully qualified name of the
///                              current branch</param>
/// <param name="itemName">The name of a BRANCH or LEAF at
///                        the current level. Or a pointer
///                        to a NULL string. Passing in a
///                        NULL string results in a return
///                        string which represents the
///                        current position in the
///                        hierarchy. </param>
/// <param name="fullItemID">Fully qualified name if the
///                          item. This name is used to
///                          access the item or add it to a
///                          group. </param>                                                                   
 HRESULT DLLCALL OnBrowseGetFullItem(				   
	LPWSTR actualPosition, 
	LPWSTR itemName, 
	LPWSTR * fullItemID  )
{
	return GenericServerAPI::OnBrowseGetFullItemID( actualPosition, itemName, fullItemID );
}


//----------------------------------------------------------------------------
// Server Developer Studio DLL API additional Methods 
// (Called by the generic server)
//----------------------------------------------------------------------------

/// <summary>
/// This method is called when a client connects to the OPC
/// server. If the method returns an error code then the client
/// connect is refused.
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation. S_OK allows
/// the client to connect to the server.
/// </returns>                                                  
 HRESULT DLLCALL OnClientConnect()
{
	// client is allowed to connect to server
	return GenericServerAPI::OnClientConnect();
}


/// <summary>
/// This method is called when a client disconnects from the OPC
/// server.
/// </summary>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>                                                  
 HRESULT DLLCALL OnClientDisconnect()
{
	return GenericServerAPI::OnClientDisconnect();
}



/// <summary>
/// Refresh the items listed in the appHandles array in the
/// cache.
/// 
/// This method is called when a client executes a read from
/// device. The device read is called with all client requested
/// items.
/// 
/// 
/// </summary>
/// <returns>
/// A result code with the result of the operation.
/// </returns>
/// <param name="numItems">Number of defined item handles</param>
/// <param name="pItemHandles">Array with the application handle
///                            of the items that need to be
///                            refreshed.</param>                  
 HRESULT DLLCALL OnRefreshItems(
									  /* in */       int        numItems,
									  /* in */       int     *  pItemHandles )
{
	return GenericServerAPI::OnRefreshItems(  numItems, pItemHandles );
}


/// <summary>
/// The items listed in the appHandles array was added to a group
/// or gets used for item based read/write.
/// 
/// This method is called when a client adds the items to a group
/// or use item based read/write functions.
/// 
/// 
/// </summary>
/// <returns>
/// A result code with the result of the operation.
/// </returns>
/// <param name="numItems">Number of defined item handles</param>
/// <param name="pItemHandles">Array with the application handle
///                            of the items that need to be
///                            updated.</param>                    
 HRESULT DLLCALL OnAddItems(
								  /* in */       int        numItems,
								  /* in */       int     *  pItemHandles )
{
	return GenericServerAPI::OnAddItems(numItems,pItemHandles);
}


/// <summary>
/// The items listed in the appHandles array are no longer used
/// by clients.
/// 
/// This method is called when a client removes items from a
/// group or no longer use the items in item based read/write
/// functions. Only items are listed which are no longer used by
/// at least one client.
/// 
/// 
/// </summary>
/// <returns>
/// A result code with the result of the operation.
/// </returns>
/// <param name="numItems">Number of defined item handles</param>
/// <param name="pItemHandles">Array with the application handle
///                            of the items that no longer need
///                            to be updated.</param>              
 HRESULT DLLCALL OnRemoveItems(
									 /* in */       int        numItems,
									 /* in */       int     *  pItemHandles )
{
	return GenericServerAPI::OnRemoveItems(numItems,pItemHandles);
}


/// <summary>
/// This method is called when a client executes a 'write' server
/// call. The items specified in the OPCITEMVQT array need to be
/// written to the device.
/// 
/// The cache is updated in the generic server after returning
/// from the customization WiteItems method. Items with write
/// error are not updated in the cache.
/// </summary>
/// <param name="numItems">Number of defined item handles</param>
/// <param name="pItemHandles">Array with the application
///                            handles</param>
/// <param name="pItemVQTs">Object with handle, value,
///                         quality, timestamp</param>
/// <param name="errors">Array with HRESULT success/error
///                       codes on return.</param>
/// <returns>
/// A result code with the result of the operation. 
/// </returns>                                                     
 HRESULT DLLCALL OnWriteItems(
									int          numItems,
									int       *  pItemHandles,
									OPCITEMVQT*  pItemVQTs,
									HRESULT   *  errors )
{
	return GenericServerAPI::OnWriteItems( numItems, pItemHandles, pItemVQTs, errors );
}


/// <summary>
/// \Returns information about the OPC DA Item corresponding to
/// an Event Attribute.
/// </summary>
/// <value>
/// Add a description here...
/// </value>
/// <param name="conditionId">Event Category Identifier.</param>
/// <param name="subConditionId">Sub Condition Definition
///                              Identifier. It's 0 for Single
///                              State Conditions.</param>
/// <param name="attributeId">Event Attribute Identifier.</param>
/// <param name="itemId">Pointer to where the text
///                         string with the ItemID of the
///                         associated OPC DA Item will be
///                         saved. Use a null string if
///                         there is no OPC Item
///                         corresponding to the Event
///                         Attribute specified by the
///                         \parameters conditionId,
///                         subConditionId and attributeId.
///                         </param>
/// <param name="nodeName">Pointer to where the text
///                           string with the network node
///                           name of the associated OPC Data
///                           Access Server will be saved.
///                           Use a null string if the server
///                           is running on the local node. </param>
/// <param name="clsid">CLSID of the associated Data
///                      Access Server will be saved.
///                      Use the value null if there is
///                      no associated OPC Item.</param>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>
/// <retval name="V1">\Return value 1</retval>
/// <retval name="V2">\Return value 2</retval>
/// <remarks>
/// Called by the generic server part to get information about
/// the OPC DA Item corresponding to an Event Attribute.
/// </remarks>                                                      
 HRESULT DLLCALL  OnTranslateToItemId( int conditionId, int subConditionId, int attributeId, LPWSTR* itemId, LPWSTR* nodeName, CLSID* clsid  )
{
	return GenericServerAPI::OnTranslateToItemId(conditionId, subConditionId, attributeId, itemId, nodeName, clsid);
}


/// <summary>
/// Notification if an Event Condition has been acknowledged.
/// </summary>
/// <value>
/// Add a description here...
/// </value>
/// <param name="conditionId">Event Category Identifier.</param>
/// <param name="subConditionId">Sub Condition Definition
///                              Identifier. It's 0 for Single
///                              State Conditions.</param>
/// <returns>
/// A HRESULT code with the result of the operation.
/// </returns>
/// <retval name="V1">\Return value 1</retval>
/// <retval name="V2">\Return value 2</retval>
/// <remarks>
/// Called by the generic server part if the Event Condition
/// specified by the parameters has been acknowledged. This
/// function is called if the Event Condition is successfully
/// acknowledged but before the indication events are sent to the
/// clients. If this function fails then the error code will be
/// returned to the client and no indication events will be
/// generated.
/// </remarks>                                                   
 HRESULT DLLCALL  OnAckNotification( int conditionId, int subConditionId )
{
	return GenericServerAPI::OnAckNotification(conditionId, subConditionId);
}

 //----------------------------------------------------------------------------
// Server Developer Studio DLL API additional Methods introduced in V8
// (Called by the generic server)
//----------------------------------------------------------------------------
                                               
HRESULT DLLCALL  OnLog( int level, char* message )
{
	return GenericServerAPI::OnLog(level, message);
}

int DLLCALL  OnLogEnable( bool enable )
{
	return GenericServerAPI::OnLogEnable(enable);
}

