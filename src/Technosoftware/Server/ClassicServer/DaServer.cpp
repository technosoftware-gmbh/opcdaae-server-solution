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
#include "stdafx.h"                             // Generic server part headers

#include "UtilityFuncs.h"                       // for WSTRClone()
#include "MatchPattern.h"                       // for default filtering
// Application specific definitions
#include "DaServer.h"
#ifdef _OPC_NET
#include "app_netserverwrapper.h"
#endif
#include "IClassicBaseNodeManager.h"
#include "Logger.h"

using namespace IClassicBaseNodeManager;

extern int       gdwBrowseMode;
extern bool 	 gUseOnItemRequest;
extern bool      gUseOnRefreshItems;
extern bool 	 gUseOnAddItem;
extern bool 	 gUseOnRemoveItem;
extern WCHAR* 	 vendor_name;

//-----------------------------------------------------------------------------
// CODE
//-----------------------------------------------------------------------------

// Description: 
// ------------
// The Data Server Handler creates:
//    - the supported Device Items
//    - the Server Address Space
//    - the Public Groups (optionally)
//
// and implements the (pure) virtual functions of class DaBaseServer
// which are called from the generic server part:
//
//    Common DA Server Functions
//       - DaBaseServer::OnGetServerState (1)
//       - DaBaseServer::OnCreateCustomServerData (2)
//       - DaBaseServer::OnDestroyCustomServerData  (2)
//    Validate Function
//       - DaBaseServer::OnValidateItems (1)
//    Refresh Functions
//       - DaBaseServer::OnRefreshInputCache (1)
//       - DaBaseServer::OnRefreshOutputDevices (1)
//    Server Address Space Browse Functions
//       - DaBaseServer::OnBrowseOrganization (1)
//       - DaBaseServer::OnBrowseChangeAddressSpacePosition (1)
//       - DaBaseServer::OnBrowseGetFullItemIdentifier  
//       - DaBaseServer::OnBrowseAccessPaths (2)
//       - DaBaseServer::OnBrowseItemIdentifiers
//    Item Property Handling Functions
//       - OnQueryItemProperties
//       - OnGetItemProperty
//       - OnLookupItemId (2)
//       - OnReleasePropertyCookie (2)
//
// (1)   Pure virtual functions of class DaBaseServer which must
//       be implemented in the class derived from the class
//       DaBaseServer.
// (2)   Virtual functions of class DaBaseServer which can be
//       overloaded. The DaBaseServer class has dummy
//       implementations.

//=============================================================================
// Construction
//=============================================================================
DaServer::DaServer()
{
	m_fCreated = FALSE;
	m_fServerStateChanged = FALSE;
	m_hTerminateThraedsEvent = NULL;
	m_hUpdateThread = NULL;
	m_dwServerState = OPC_STATUS_FAILED;
	m_dwBandWith = 0xFFFFFFFF;
}


//=============================================================================
// Initializer
// -----------
//    Initzialization of the Data Server Instance.
//    This function initializes the Data Server Class Handler and
//    creates the supported Device Items and the Public Groups.
//    Also the Update Notification thread is created.
//    If all succeeded then the server state is set to OPC_STATUS_RUNNING.
//=============================================================================
HRESULT DaServer::Create(DWORD   dwUpdateRate,
	LPCTSTR szDLLParams,
	LPCTSTR szInstanceName)
{
	try {
		CHECK_BOOL(!m_fCreated)                    // Check if already initialized
			CHECK_BOOL(m_ItemListLock.Initialize())	   // Initialize class for item list locking
			CHECK_BOOL(m_OnRequestItemsLock.Initialize())	   // Initialize class for item list locking

			USES_CONVERSION;
		CHECK_RESULT(DaBaseServer::Create(
			0,	                           // Instance Index
			T2W((LPTSTR)szInstanceName),  // Server Instance Name
			dwUpdateRate))              // Device Update rate in ms

			CHECK_RESULT(m_SASRoot.CreateAsRoot())	   // Initialize hierarchical SAS
			// Define the callbacks used by this server
#ifdef _OPC_NET
			LOGFMTT("OnDefineCallbacks() called...");
		    CHECK_RESULT(GenericServerAPI::OnDefineCallbacks())
		    // Create the Items supported by this server
			LOGFMTT("OnCreateServerItems() called...");
			CHECK_RESULT(GenericServerAPI::OnCreateServerItems())
#endif

#ifdef _OPC_DLL
			CHECK_RESULT(pOnDefineDaCallbacks(IClassicBaseNodeManager::AddItem, IClassicBaseNodeManager::RemoveItem, IClassicBaseNodeManager::AddProperty, IClassicBaseNodeManager::SetItemValue, IClassicBaseNodeManager::SetServerState, IClassicBaseNodeManager::GetActiveItems, IClassicBaseNodeManager::FireShutdownRequest, IClassicBaseNodeManager::GetClients, IClassicBaseNodeManager::GetGroups, IClassicBaseNodeManager::GetGroupState, IClassicBaseNodeManager::GetItemStates))
			// Create the Items supported by this server
#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
			CHECK_RESULT(pOnDefineAeCallbacks(IClassicBaseNodeManager::AddSimpleEventCategory, IClassicBaseNodeManager::AddTrackingEventCategory, IClassicBaseNodeManager::AddConditionEventCategory, IClassicBaseNodeManager::AddEventAttribute,
				IClassicBaseNodeManager::AddSingleStateConditionDefinition, IClassicBaseNodeManager::AddMultiStateConditionDefinition, IClassicBaseNodeManager::AddSubConditionDefinition, IClassicBaseNodeManager::AddArea, IClassicBaseNodeManager::AddSource, IClassicBaseNodeManager::AddExistingSource,
				IClassicBaseNodeManager::AddCondition, IClassicBaseNodeManager::ProcessSimpleEvent, IClassicBaseNodeManager::ProcessTrackingEvent, IClassicBaseNodeManager::ProcessConditionStateChanges, IClassicBaseNodeManager::AckCondition))
#endif
			// Create the Items supported by this server
			LOGFMTT("OnCreateServerItems() called...");
			CHECK_RESULT(pOnCreateServerItems())
#endif

		    CHECK_RESULT(CreateUpdateThread())

			// Only if the Server state wasn't changed by the customization part setting of the server state is allowed
			if (!m_fServerStateChanged)
			{
				m_dwServerState = OPC_STATUS_RUNNING;		// All succeeded
			}
		m_fCreated = TRUE;
		return S_OK;
	}
	catch (HRESULT hresEx) {
		LOGFMTE("DaServer::Create() returned with error result 0x%x.", hresEx);
		return hresEx;
	}
	catch (...) {
		return E_FAIL;
	}
}


//=============================================================================
// Destructor
//=============================================================================
DaServer::~DaServer()
{
	m_dwServerState = OPC_STATUS_SUSPENDED;
	KillUpdateThread();
	DeleteServerItems();
}


//-----------------------------------------------------------------------------
// Common DA Server Functions
//-----------------------------------------------------------------------------

//=============================================================================
// OnGetServerState
// ----------------
//    Returns the current internal status of the Data Access server
//    to handle the status information request function.
//=============================================================================
HRESULT DaServer::OnGetServerState(
	/*[out]*/   DWORD          & bandWidth,
	/*[out]*/   OPCSERVERSTATE & serverState,
	/*[out]*/   LPWSTR		   & vendor)
{
	HRESULT hres = S_OK;

	switch (InstanceIndex()) {

	case 0:
		bandWidth = BandWidth();          // Current Band With
		serverState = ServerState();        // Current State
		vendor = vendor_name;
		break;

	default:
		hres = E_FAIL;                         // Invalid Instance Index
		break;
	}
	return hres;
}


//=============================================================================
// Add custom specific client related server data
// ----------------------------------------------
//    Add here your own client (connection) related server data.
//    This function is called if a client connects to the server.
//    You have access to this data within the BrowseXXXX functions.
//
// Note:
//    If you return with a failure code then the server object will not
//    be  created and the CreateInstance() call in the client fails.
//
// SampleServer:
//    The custom data is initialized with a pointer to the root
//    of the Server Address Space.
//=============================================================================
HRESULT DaServer::OnCreateCustomServerData(LPVOID* customData)
{
	HRESULT        hres = S_OK;

	//    
	// Add here your own data
	//    
	// e.g.
	//
	//    MyData_t* pMyData = new MyData_t;
	//    if (pMyData == NULL) {
	//       return E_OUTOFMEMORY;
	//    }
	//    
	//    pMyData->XXX = xxx;
	//    pMyData->YYY = yyy;
	//    pMyData->ZZZ = zzz;
	//    
	//    *customData = pMyData;
	//

#ifdef _OPC_NET
	hres = GenericServerAPI::OnClientConnect();
#endif

#ifdef _OPC_DLL
	if (pOnClientConnect != NULL)
	{
		hres = (*pOnClientConnect)();
	}
#endif
	if (hres == S_OK) {
		*customData = SASRoot();
	}

	LOGFMTT("OnClientConnect() finished with hres = 0x%x.", hres);
	return hres;

}


//*****************************************************************************
//* INFO: The following function is declared as virtual and
//*       implementation is therefore optional.
//*       The following code is only here for informational purpose
//*       and if no server specific handling is needed the code
//*       can savely be deleted.
//*****************************************************************************
//=============================================================================
// Release custom specific client related server data
// --------------------------------------------------
//    Remove here your added client (connection) related server data.
//    This function is called if a client disconnects from server.
//=============================================================================
HRESULT DaServer::OnDestroyCustomServerData(LPVOID* customData)
{
	HRESULT        hres = S_OK;

	//    
	// Add here your own data
	//    
	// e.g.
	//
	//    if (*customData) {
	//       delete (Mydata_t *)*customData;
	//       *customData = NULL;
	//    }
	//

#ifdef _OPC_NET
	hres = GenericServerAPI::OnClientDisconnect();
#endif

#ifdef _OPC_DLL
	if (pOnClientDisconnect != NULL)
	{
		hres = (*pOnClientDisconnect)();
	}
#endif
	LOGFMTT("OnClientDisconnect() finished with hres = 0x%x.", hres);
	return hres;
}

//-----------------------------------------------------------------------------
// Validate Function
//-----------------------------------------------------------------------------

//=============================================================================
// OnValidateItems
// ---------------
//    Determines if the specified item definitions are valid (could be
//    used to add items without errors).
//    This function is called from the generic server part by item
//    handling functions (ValidateItems,AddItems, SetDatatypes, ... ).
//
//    Dependent on the reason of the function call this function must
//    return DaDeviceItem pointers or OPCITEMRESULTs.
//    Therefore this function can also be used to create DeviceItems
//    if they are requested by a client.
//
// Inputs:
//    validateRequest  OPC_VALIDATEREQ_DEVICEITEMS or
//                      OPC_VALIDATEREQ_ITEMRESULTS
//
//    blobUpdate       Specifies if updated Blobs must be returned in the
//                      OPCITEMRESULT structure. This flag can be ignored
//                      if eValidateReason is OPC_VALIDATEREQ_DEVICEITEMS.
//
//    numItems        Number of items to be validated.
//
//    itemDefinitions         Array of OPCITEMDEFs with definitions of each item
//                      to be validated.
//
// Inputs/Outputs:
//    daDeviceItems        If validateRequest is OPC_VALIDATEREQ_DEVICEITEMS
//                         Array to store the pointers of successfully
//                         validated DeviceItems. This array is
//                         initialized with NULL-Pointers on entry.
//                         ATTENTION: The Attach() method must be called
//                                    for each returned Device Item.
//
//                      If validateRequest is OPC_VALIDATEREQ_ITEMRESULTS
//                      then this parameter is NULL.
//
//    itemResults   If validateRequest is OPC_VALIDATEREQ_ITEMRESULTS
//                         Array to store OPCITEMRESULTs of successfully
//                         validated DeviceItems. On entry this array
//                         includes OPCITEMRESULT structures initialized
//                         with:
//
//                         hServer              = 0;
//                         vtCanonicalDataType  = VT_EMPTY;
//                         wReserved            = 0;
//                         dwAccessRights       = 0;
//                         dwBlobSize           = 0;
//                         pBlob                = NULL;
//
//                            
//                      If validateRequest is OPC_VALIDATEREQ_DEVICEITEMS
//                      then this parameter is NULL.
//
//    errors           Array with error codes for the individual
//                      validated items.
//                      Is initialized with S_OK on entry.
//
// Return:
//    S_OK              If validation succeeded all specified item
//                      definitions
//    S_FALSE           There are one or more errors in errors
//
//    Do not return other codes !
//
// Note:
//    The Attach() method must be called for each returned Device Item.
//
// SampleServer:
//    See data type table in comment section for
//    supported 'requested data types'.         
//=============================================================================
HRESULT DaServer::OnValidateItems(
	/*[in]                           */ OPC_VALIDATE_REQUEST    validateRequest,
	/*[in]                           */ BOOL                    blobUpdate,
	/*[in]                           */ DWORD                   numItems,
	/*[in, size_is(numItems)]      */ OPCITEMDEF           *  itemDefinitions,
	/*[in, out, size_is(numItems)] */ DaDeviceItem          ** daDeviceItems,
	/*[in, out, size_is(numItems)] */ OPCITEMRESULT        *  itemResults,
	/*[in, out, size_is(numItems)] */ HRESULT              *  errors)
{
	OPCITEMDEF*    pItemDef;
	DaDeviceItem   *pDItem;
	VARTYPE        vtCan, vtReq;
	HRESULT        hres;
	DWORD          i;
	int			  countAddedItems = 0;
	int			   countFailedItems = 0;
	hres = S_OK;

	m_OnRequestItemsLock.BeginWriting();               // protect item list access

#ifdef _OPC_DLL
	LPWSTR		  *fullItemIds;
	VARTYPE		  *dataTypes;
	if (gUseOnItemRequest) {
		for (i = 0; i < numItems; i++) {            // Count all unknown items
			pItemDef = &itemDefinitions[i];
			// Search the Item
			errors[i] = FindDeviceItem(pItemDef->szItemID, &pDItem);
			if (FAILED(errors[i])) {
				countFailedItems++;
			}
		}

		if (countFailedItems > 0) {
			dataTypes = new VARTYPE[countFailedItems];
			fullItemIds = new LPWSTR[countFailedItems];
			int foundItem = 0;

			for (i = 0; i < numItems; i++) {        // Count all unknown items
				pItemDef = &itemDefinitions[i];

				errors[i] = FindDeviceItem(pItemDef->szItemID, &pDItem);
				if (FAILED(errors[i])) {
					dataTypes[foundItem] = pItemDef->vtRequestedDataType;
					fullItemIds[foundItem] = pItemDef->szItemID;
					foundItem++;
				}
			}

			hres = pOnRequestItems(countFailedItems, fullItemIds, dataTypes);

			if (dataTypes)
			{
				delete[] dataTypes;
				dataTypes = NULL;
			}
			if (fullItemIds)
			{
				delete[] fullItemIds;
				fullItemIds = NULL;
			}
		}
	}
#endif


	for (i = 0; i < numItems; i++) {            // Check all specified items
		pItemDef = &itemDefinitions[i];
		// Search the Item
		errors[i] = FindDeviceItem(pItemDef->szItemID, &pDItem);

#ifdef _OPC_NET
		if ((FAILED(errors[i])) && gUseOnItemRequest) {
			// Device item is requested and item not found, so request from DLL
			hres = GenericServerAPI::OnItemRequest(pItemDef->szItemID);
			USES_CONVERSION;
			LOGFMTT("OnItemRequest( %s ) finished with hres = 0x%x.", W2A(pItemDef->szItemID), hres);
			if (hres == S_OK) {
				// Item created by DLL, try to find it again
				errors[i] = FindDeviceItem(pItemDef->szItemID, &pDItem);
			}
		}
#endif

		if (SUCCEEDED(errors[i])) {
			// Now check if requested data type is supported
			vtCan = pDItem->get_CanonicalDataType();
			vtReq = pItemDef->vtRequestedDataType;

			//
			// Supported canonical data types:
			//
			//    VT_I1, VT_UI1, VT_I2, VT_UI2,  VT_I4,  VT_UI4,
			//    VT_R4, VT_R8,  VT_CY, VT_DATE, VT_BSTR, VT_BOOL
			//    and any combination with the VT_ARRAY flag
			// 
			// 
			// Specifies if the requested data type can be
			// returned for this item:
			// 
			// vtReq                vtCan             HRESULT
			// ------------------------------------------------------
			//
			//    VT_EMPTY          x                 S_OK
			//    vtCan             x                 S_OK
			//    VT_BYREF | x      x                 OPC_E_BADTYPE
			//                                                 
			//    VT_ARRAY | x      x                 S_OK
			//    VT_BSTR           x                 S_OK
			//    
			//    else result                         S_OK (result of ::VariantChangeType())

			//
			//    TODO: You can remove existing or add own conversion restrictions.
			//          Please note that in this case the function VariantFromVariant()
			//          in the module variantconversion.cpp must be must be modified too.

			// Implementation of the conversion table above
			if (vtReq != VT_EMPTY && vtReq != vtCan) {

				switch (vtReq & VT_TYPEMASK) {
				case VT_I1:
				case VT_UI1:
				case VT_I2:
				case VT_UI2:
				case VT_I4:
				case VT_UI4:
				case VT_R4:
				case VT_R8:
				case VT_CY:
				case VT_DATE:
				case VT_BSTR:
				case VT_BOOL:  break;

				default:       errors[i] = OPC_E_BADTYPE;
					break;
				}
				if (SUCCEEDED(errors[i])) {

					if (vtReq & VT_BYREF) {
						errors[i] = OPC_E_BADTYPE;
					}
					else if (!(vtReq & VT_ARRAY || vtReq == VT_BSTR) && (vtCan & VT_ARRAY)) {
						errors[i] = OPC_E_BADTYPE;
					}
				}
			}

			if (SUCCEEDED(errors[i])) {
				// All succeeded for this item
				if (validateRequest == OPC_VALIDATEREQ_DEVICEITEMS) {
					daDeviceItems[i] = pDItem;          // Device Item ist requested
					countAddedItems++;
				}
				else {                              // OPCITEMRESULT is requested
					errors[i] = pDItem->Killed() ? OPC_E_UNKNOWNITEMID :
						pDItem->get_OPCITEMRESULT(blobUpdate,
							&itemResults[i]);
					pDItem->Detach();                // DeviceItem is attached by FindDeviceItem()
				}
			}
		} // Item found

		if (FAILED(errors[i])) {               // Specify the function result
			hres = S_FALSE;                        // Function not succeessfully for all specified items
		}

	} // Check all specified items
	m_OnRequestItemsLock.EndWriting();               // release item list access
	return hres;
}

//-----------------------------------------------------------------------------
// Data Cache Refresh Functions
//-----------------------------------------------------------------------------

//=============================================================================
// Reads the input devices and refreshs the chache
// -----------------------------------------------
//    This function is called when ReadThrough function calls are executed.
//    The function may be called from a periodic refresh thread.
//    The call may be ignored if the cache is refreshed event driven.
//    Either all signals or only the specified signals may be refreshed.
//    If numItems is 0 then the Item and the Error Array is undefined
//    and the server should refresh the cache of all items.
//
//    The refresh sets:    - The Data Cache with the current
//                           Signal State Value
//                         - The Time Stamp
//                         - The Quality Field
//
// Inputs:
//       dwReason       OPC_REFRESH_CLIENT if called from the
//                      generic server part.
//       numItems     Number of items to be refreshed
//       ppDevItem      Array with definitions for each item to be
//                      refreshed. Entries may be NULL. In this
//                      case do not use the corresponding Error.
//                      Is NULL if numItems=0.
// Inputs/Outputs:
//    errors           An array with error codes for the individual items
//                      or a NULL pointer if parameter numItems is 0. If
//                      not NULL each element in the array contains the
//                      code S_OK if the corresponding pointer in ppDevItem
//                      is a Device Item pointer, if it is a NULL pointer
//                      then the value may contains an error code and it
//                      is not permitted to change it.
//
// Return:
//    S_OK              If Data Cache refresh for all specified items
//                      succeeded
//    S_FALSE           There are one or more errors in errors
//
//    Do not return other codes !
//
// Note:
//    Data Cache access must be protected with readWriteLock_ functions.
//=============================================================================
HRESULT DaServer::OnRefreshInputCache(
	/*[in]                      */ OPC_REFRESH_REASON  dwReason,   // OPC_REFRESH_CLIENT, OPC_REFRESH_INIT or OPC_REFRESH_PERIODIC
	/*[in]                      */ DWORD               numItems,
	/*[in, size_is(,numItems)]*/ DaDeviceItem      ** ppDevItem,
	/*[in, size_is(,numItems)]*/ HRESULT          *  errors)
{
	DWORD             i;
	HRESULT           hres, hresReturn;
	DeviceItem*    pItem;


	hresReturn = S_OK;

	if (numItems == 0) {
		//
		// All data has to be refreshed.
		//
		// Note :   The pointers to the arrys ppDevItem and errors may be NULL
		//          if numItems is 0 and therefore should not be used. 


		if (gUseOnRefreshItems)
		{
#ifdef _OPC_NET
			hres = GenericServerAPI::OnRefreshItems(0, NULL);		// have the adap DLL read all the inputs
#endif

#ifdef _OPC_DLL
			try
			{
				hres = (*pOnRefreshItems)(0, NULL);	// have the adap DLL read all the inputs
			}
			catch (...)
			{
				LOGFMTE("EXCEPTION CAUGHT !!!");
				hres = S_FALSE;
			}
#endif
			if (FAILED(hres)) {
				LOGFMTE("OnRefreshItems(0, NULL) failed with hres = 0x%x.", hres);
				return S_FALSE;
			}
		}
	}
	else {
		//
		// Only specified data has to be refreshed.
		//
		// make array with item handles

		DeviceItem** pDLLHandles = new DeviceItem*[numItems];
		if (!pDLLHandles) {
			hres = E_OUTOFMEMORY;                  // is the error code for all items
			goto RefreshInputCacheExit1;           // Out of memory
		}
		int dwNumDevices = 0;
		for (i = 0; i < numItems; i++) {            // DLL requires handle array without gaps
			pItem = (DeviceItem *)ppDevItem[i];
			if (pItem) {
				pDLLHandles[dwNumDevices++] = pItem;
			}
		}

#ifdef _OPC_NET
		hres = GenericServerAPI::OnRefreshItems(dwNumDevices, (IntPtr*)pDLLHandles);
#endif

#ifdef _OPC_DLL
		try
		{
			hres = (*pOnRefreshItems)(dwNumDevices, pDLLHandles);
		}
		catch (...)
		{
			LOGFMTE("EXCEPTION CAUGHT !!!");
			hres = S_FALSE;
		}
#endif
		delete[] pDLLHandles;
		if (FAILED(hres)) {                      // hres is the error code for all items
			for (i = 0; i < numItems; i++) {
				if (ppDevItem[i]) {
					errors[i] = hres;
				}
			}
			hres = S_FALSE;
		}
		LOGFMTT("OnRefreshItems(dwNumDevices, pDLLHandles) finished with hres = 0x%x.", hres);
	}
	return hresReturn;

	// Out of memory or ReadHWInputs() failed
RefreshInputCacheExit1:
	for (i = 0; i < numItems; i++) {
		if (ppDevItem[i]) {
			errors[i] = hres;
		}
	}
	return S_FALSE;
}


//=============================================================================
// Writes values to the device and refreshs the cache
// --------------------------------------------------
//    This function must be implemented to refresh the specified devices
//    with the defined values. The data cache of an item must be refreshed
//    if the HW update succeeded.
//    This function is called from the generic server part if a client
//    invokes write functions.
//    If numItems is 0 then the arrays for the Items, Values and Errors
//    are undefined and the server should refresh all signals with the
//    current values stored in the cache. Typically this will be done only
//    one time to initialize the hardware after the server has started.
//    This function is never called from the generic server part with
//    numItems=0. Therefore you must implement 'refresh all from cache'
//    only if you invoke this function within your own code part.
//
// Inputs:
//    dwReason          OPC_REFRESH_CLIENT if called from the
//                      generic server part.
//    szActorID         The identifier of the client which
//                      initiated the refresh if dwReason is
//                      OPC_REFRESH_CLIENT.
//    numItems        Number of items to be refreshed
//    ppDevItem         Array with definitions for each item to be
//                      refreshed. Entries may be NULL.
//                      In this case do not use the corresponding VQT
//                      and do not change the corresponding Error code.
//                      Is NULL if numItems=0.
//    pItemVQTs         Values/Qualities/Timestamps to be written.
//                      Is NULL if numItems=0.
// Inputs/Outputs:
//    errors           An array with error codes for the individual items
//                      or a NULL pointer if parameter numItems is 0. If
//                      not NULL each element in the array contains the
//                      code S_OK if the corresponding pointer in ppDevItem
//                      is a Device Item pointer, if it is a NULL pointer
//                      then the value may contains an error code and it
//                      is not permitted to change it.
//
// Return:
//    S_OK              If HW refresh for all specified items succeeded
//    S_FALSE           There are one or more errors in errors
//
//    Do not return other codes !
//
// Note:
//    Data Cache access must be protected with readWriteLock_ functions.
//    The Data Value in VQT (pItemVQTs[i].vDataValue) may be initialized
//    with VT_EMPTY if there is no value to write.
//=============================================================================
HRESULT DaServer::OnRefreshOutputDevices(
	/*[in]                      */ OPC_REFRESH_REASON  dwReason,   // OPC_REFRESH_CLIENT, OPC_REFRESH_INIT or OPC_REFRESH_PERIODIC
	/*[in, string]              */ LPWSTR              szActorID,
	/*[in]                      */ DWORD               numItems,
	/*[in, size_is(,numItems)]*/ DaDeviceItem      ** ppDevItem,
	/*[in, size_is(,numItems)]*/ OPCITEMVQT       *  pItemVQTs,
	/*[in, size_is(,numItems)]*/ HRESULT          *  errors)
{
	DWORD             i;
	DeviceItem*    pItem;
	HRESULT           hres, hresReturn;
	DWORD             dwNumDevices = 0;


	hresReturn = S_OK;

	if (numItems == 0) {
		// Do nothing for the OPC Server Developer Studio version
	}
	else {
		//return hresReturn;
		//
		// Only specified HW data has to be refreshed.
		//
		LPVARIANT   pVal;
		DaDeviceCache  DLLIO;

		hres = DLLIO.Create(numItems);

		if (FAILED(hres)) {
			goto RefreshOutputDevicesExit1;
		}
		readWriteLock_.BeginWriting();           // We are manipulating the DeviceItem make
		// sure no one else gets access during update.
		for (i = 0; i < numItems; i++) {
			pItem = (DeviceItem *)ppDevItem[i];
			if (pItem == NULL) {
				continue;                           // Handle next item
			}

			DLLIO.m_paItemVQTs[dwNumDevices] = pItemVQTs[i];
			DLLIO.m_paHandles[dwNumDevices] = pItem;
			dwNumDevices++;

		} // Handle all defined items
		readWriteLock_.EndWriting();             // UnLock DeviceItem

		if (dwNumDevices) {
#ifdef _OPC_NET
			hres = GenericServerAPI::OnWriteItems(dwNumDevices, (IntPtr*)DLLIO.m_paHandles, DLLIO.m_paItemVQTs, DLLIO.m_paErrors);
#endif

#ifdef _OPC_DLL
			hres = (HRESULT)(*pOnWriteItems)((int)dwNumDevices, DLLIO.m_paHandles, DLLIO.m_paItemVQTs, DLLIO.m_paErrors);
#endif

			LOGFMTT("OnWriteItems( %i ) finished with hres = 0x%x.", dwNumDevices, hres);
			readWriteLock_.BeginWriting();        // We are manipulating the DeviceItem make
			if (SUCCEEDED(hres)) {

				dwNumDevices = 0;

				WORD		wQuality;
				_FILETIME	fTimeStamp;
				VARIANT		vVal;

				for (i = 0; i < numItems; i++) {
					VariantInit(&vVal);
					pItem = (DeviceItem *)ppDevItem[i];
					if (pItem == NULL) {
						continue;                     // Handle next item.
					}
					hres = DLLIO.m_paErrors[dwNumDevices];
					if (SUCCEEDED(hres)) {
						pVal = &pItemVQTs[i].vDataValue;
						_ASSERTE(V_VT(pVal) != VT_EMPTY);  // Should not be EMPTY if errors[i] is not OK
						V_VT(&vVal) = pItem->get_CanonicalDataType();
						pItem->get_ItemValue(&vVal, &wQuality, &fTimeStamp);
						if (pItemVQTs[i].bQualitySpecified)
						{
							wQuality = pItemVQTs[i].wQuality;
						}
						if (pItemVQTs[i].bTimeStampSpecified)
						{
							fTimeStamp = pItemVQTs[i].ftTimeStamp;
						}
						// Update Data Cache
						hres = pItem->set_ItemValue(
							pVal,
							wQuality,
							&fTimeStamp);
					}
					errors[i] = hres;
					VariantClear(&vVal);

					if (FAILED(hres)) {
						hresReturn = S_FALSE;
					}
					dwNumDevices++;
				}
			}
		}
		else
		{
			hresReturn = S_FALSE;
		}
		readWriteLock_.EndWriting();                // UnLock DeviceItem
	}
	// return S_OK if HW refresh for all specified
	return hresReturn;                           // items succeeded; otherwise S_FALSE.

	// Out of memory or WriteHWOutputs() failed
RefreshOutputDevicesExit1:
	for (i = 0; i < numItems; i++) {
		if (ppDevItem[i]) {
			errors[i] = hres;
		}
	}
	return S_FALSE;
}


//-----------------------------------------------------------------------------
// Server Address Space Browse Functions
//-----------------------------------------------------------------------------

//=============================================================================
// Returns the organisation of the server address space
// ----------------------------------------------------
//    Valid return values are OPC_NS_FLAT or OPC_NS_HIERARCHIAL.
//=============================================================================
OPCNAMESPACETYPE DaServer::OnBrowseOrganization()
{
	return OPC_NS_HIERARCHIAL;
}



//=============================================================================
// Changes the position the server address space
// ---------------------------------------------
//    Returns the new position in the Server Adress Space tree given
//    the actual position and the change to be taken.
// Inputs:
//    dwBrowseDirection    OPC_BROWSE_DOWN, OPC_BROWSE_UP or OPC_BROWSE_TO
//    szString             if OPC_BROWSE_DOWN the BRANCH where to change
//                         if OPC_BROWSE_TO the fully qualified name where
//                         to change or NULL to go to the 'root'.
//    szActualPosition     the actual position in the address tree of the
//                         calling server instance
//    customData         custom specific data.
//
// Outputs:
//    szActualPosition     the new position.
//
// Note:
//    Use SysAllocString to allocate the string for the new position.
//    You must not free the old string. The server will keep the old
//    position if you return a failure result code.
//    The generic server part checks if the parameters are valid.
//=============================================================================
HRESULT DaServer::OnBrowseChangeAddressSpacePosition(
	/*[in]         */          tagOPCBROWSEDIRECTION   dwBrowseDirection,
	/*[in, string] */          LPCWSTR                 szString,
	/*[in, out]    */          LPWSTR                * szActualPosition,
	/*[in,out]     */          LPVOID                * customData)
{
	HRESULT hres;

	if (gdwBrowseMode == 0)
	{
		DaBranch* pCurrentPos = static_cast<DaBranch*>(*customData);
		_ASSERTE(pCurrentPos);
		DaBranch* pNewPos;

		hres = pCurrentPos->ChangeBrowsePosition(dwBrowseDirection, szString, &pNewPos);
		if (SUCCEEDED(hres)) {
			hres = pNewPos->GetFullyQualifiedName(L"", szActualPosition);
			if (SUCCEEDED(hres)) {
				*customData = pNewPos;
			}
		}
	}
	else
	{
#ifdef _OPC_NET
		hres = GenericServerAPI::OnBrowseChangePosition(dwBrowseDirection, szString, szActualPosition);
#endif

#ifdef _OPC_DLL
		if (pOnBrowseChangePosition != NULL)
		{
			hres = (*pOnBrowseChangePosition)(dwBrowseDirection, szString, szActualPosition);
		}
#endif
	}

	USES_CONVERSION;
	LOGFMTT("OnBrowseChangePosition( %i, %s ) finished with hres = 0x%x.", dwBrowseDirection, W2A(szString), hres);
	return hres;
}


//=============================================================================
// Returns the full ItemId from a partial Item ID
// ----------------------------------------------
// Inputs:
//    - actual position in tree of calling client
//    - the partial name (from the actual position
//    - custom specific data
// Output:
//    - the full item identifier that can be used with AddItems
//
// Note:
//    Use SysAllocString to allocate the returned string.
//=============================================================================
HRESULT DaServer::OnBrowseGetFullItemIdentifier(
	/*[in]         */          LPWSTR              szActualPosition,
	/*[in]         */          LPWSTR              szItemDataID,
	/*[out, string]*/          LPWSTR            * szItemID,
	/*[in,out]     */          LPVOID            * customData)
{
	HRESULT hres;
	if (gdwBrowseMode == 0)
	{
		DaBranch* pCurrentPos = static_cast<DaBranch*>(*customData);
		_ASSERTE(pCurrentPos);

		m_ItemListLock.BeginReading();               // protect item list access
		hres = pCurrentPos->GetFullyQualifiedName(szItemDataID, szItemID);
		m_ItemListLock.EndReading();                 // release item list protection
	}
	else
	{
#ifdef _OPC_NET
		hres = GenericServerAPI::OnBrowseGetFullItemId(szActualPosition, szItemDataID, szItemID);
#endif

#ifdef _OPC_DLL
		if (pOnBrowseGetFullItemId != NULL)
		{
			hres = (*pOnBrowseGetFullItemId)(szActualPosition, szItemDataID, szItemID);
		}
#endif
	}

	USES_CONVERSION;
	LOGFMTT("OnBrowseGetFullItemId( %s, %s, %s ) finished with hres = 0x%x.", W2A(szActualPosition), W2A(szItemDataID), W2A(*szItemID), hres);
	return hres;
}



//=============================================================================
// Browse the ItemIds
// ------------------
//    Returns the ItemIds in the server address space tree at the given
//    starting point.
// Inputs:
//    szActualPosition        Position in the server address space (ex. "INTERBUS1:DIGIN")
//    dwBrowseFilterType      OPC_LEAF, OPC_BRANCH, OPC_FLAT
//    szFilterCriteria        (ex. "*")
//    vtDataTypeFilter        VT_EMPTY, VT_I4, ...
//    dwAccessRightsFilter    0(Read + Write), Read, Write
//    customData            custom specific data.
// Outputs:
//    pNrItemIds              Number of items retrieved ( ex. 2 )
//    ppItemIds               array of retrieved items ( ex. "DI_SWITCH1","DI_LED2" )
//                            (the array must be created by this method!)
//
// Note:
//    Use SysAllocString to allocate the returned strings.
//=============================================================================
HRESULT DaServer::OnBrowseItemIdentifiers(
	/*[in          */          LPWSTR              szActualPosition,
#ifdef _OPC_NET
	/*[in]         */          tagOPCBROWSETYPE    dwBrowseFilterType,
#endif
#ifdef _OPC_DLL
	/*[in]         */          OPCBROWSETYPE       dwBrowseFilterType,
#endif
	/*[in, string] */          LPWSTR              szFilterCriteria,
	/*[in]         */          VARTYPE             vtDataTypeFilter,
	/*[in]         */          DWORD               dwAccessRightsFilter,
	/*[out]        */          DWORD             * pNrItemIds,
	/*[out]        */          LPWSTR           ** ppItemIds,
	/*[in,out]     */          LPVOID            * customData)
{
	HRESULT hres;
	if (gdwBrowseMode == 0)
	{

		DaBranch* pCurrentPos = static_cast<DaBranch*>(*customData);
		_ASSERTE(pCurrentPos);

		if (dwAccessRightsFilter == 0) {             // 0 indicates no filtering
			dwAccessRightsFilter = OPC_READABLE | OPC_WRITEABLE;
		}

		hres = E_INVALIDARG;
		switch (dwBrowseFilterType) {

		case OPC_BRANCH:
			hres = pCurrentPos->BrowseBranches(szFilterCriteria, pNrItemIds, ppItemIds);
			break;

		case OPC_LEAF:
			hres = pCurrentPos->BrowseLeafs(szFilterCriteria, vtDataTypeFilter,
				dwAccessRightsFilter,
				pNrItemIds, ppItemIds);
			break;

		case OPC_FLAT:
			m_ItemListLock.BeginReading();         // protect item list access
			hres = pCurrentPos->BrowseFlat(szFilterCriteria, vtDataTypeFilter,
				dwAccessRightsFilter,
				pNrItemIds, ppItemIds);
			m_ItemListLock.EndReading();           // release item list protection
			break;
		}
	}
	else
	{
#ifdef _OPC_NET
		hres = GenericServerAPI::OnBrowseItemIds(
			szActualPosition, dwBrowseFilterType,
			szFilterCriteria, vtDataTypeFilter,
			dwAccessRightsFilter,
			pNrItemIds, ppItemIds);
#endif
#ifdef _OPC_DLL
		if (pOnBrowseItemIds != NULL)
		{
			hres = (*pOnBrowseItemIds)(
				szActualPosition, dwBrowseFilterType,
				szFilterCriteria, vtDataTypeFilter,
				dwAccessRightsFilter,
				(int*)pNrItemIds, ppItemIds);

		}
#endif
	}

	USES_CONVERSION;
	LOGFMTT("OnBrowseItemIds() finished with hres = 0x%x.", hres);
	return hres;
}


//*****************************************************************************
//* INFO: The following function is declared as virtual and
//*       implementation is therefore optional.
//*       The following code is only here for informational purpose
//*       and if no server specific handling is needed the code
//*       can savely be deleted.
//*****************************************************************************
//=============================================================================
// Browse the Access Paths
// -----------------------
//    Returns all the possible AccessPaths of an item.
//
// Note:
//    Use SysAllocString to allocate the returned strings.
//
// SampleServer:
//    This function is not overloaded because there are no items
//    with access paths.
//=============================================================================
// HRESULT DaServer::OnBrowseAccessPaths(
//          /*[in]         */          LPWSTR              szItemID,
//          /*[out]        */          DWORD             * pNrAccessPaths, 
//          /*[out]        */          LPWSTR           ** szAccessPaths,
//          /*[in,out]     */          LPVOID            * customData )
// {
//    return E_NOTIMPL;
// }


//-----------------------------------------------------------------------------
// Item Property Functions
//-----------------------------------------------------------------------------

//=============================================================================
// Item Properties
// ---------------
//    The Item Property related functions are called from the generic 
//    server part with the following order:
//
//   IOPCItemProperties::QueryAvailableProperties
//      OnQueryItemProperties()
//      OnReleasePropertyCookie()
//   
//   IOPCItemProperties::GetItemProperties
//      OnQueryItemProperties()
//      OnGetItemProperty()
//      OnReleasePropertyCookie()
//   
//   IOPCItemProperties::LookupIDs
//      OnQueryItemProperties()
//      OnLookupItemId()
//      OnReleasePropertyCookie()
//
//   IOPCBrowse::GetProperties
//      for each item 
//      {
//          OnQueryItemProperties()
//          [ n * OnLookupItemId() ]            // optional
//          [ n * OnGetItemProperty() ]         // optional
//          OnReleasePropertyCookie()
//      }
//
//    IOPCBrowse::Browse
//      [ like IOPCBrowse::GetProperties ]      // optional
//
//=============================================================================

//=============================================================================
// Query the properties of an item
// -------------------------------
//    Returns all available properties of the specified ItemId.
//    The ItemId can represent some entry from the Server Address Space.
//    So it's also possible to add properties to a branch of the SAS.
//    If the ItemId specifies a branch then the Properties of ID Set 1
//    must not be returned; otherwise return these properties.
//
// Inputs:
//    szItemID                The ItemId for which the caller wants to
//                            lookup the list of Property IDs.
// Outputs:
//    pdwCount                Where to return the number of available
//                            Property IDs.
//    ppdwPropIDs             Where to return the array of available
//                            Property IDs.
//    ppCookie                Where to return application specific
//                            data. You have access to this data with
//                            the functions OnGetItemProperty(),
//                            OnLookupItemId(), OnReleasePropertyCookie().
//
// Return:
//    S_OK                    All succeeded
//    E_FAIL                  The function was not successful
//    E_OUTOFMEMORY           Not enough memory
//    E_INVALIDARG            An invalid argument was passed
//    OPC_E_UNKNOWNITEMID     The ItemId is not in the SAS
//    OPC_E_INVALIDItemId     The ItemId is not syntactically valid
//
// Note:
//    Use operator new to allocated the arrays for the Property IDs.
//
// SwiftServer:
//    There are only properties for Device Items available. The Device
//    Item is attached and the pointer is returned in the 'cookie'
//    parameter.
//=============================================================================
HRESULT DaServer::OnQueryItemProperties(
	/*[in]         */          LPCWSTR        szItemID,
	/*[out]        */          LPDWORD        pdwCount,
	/*[out]        */          LPDWORD     *  ppdwPropIDs,
	/*[out]        */          LPVOID      *  ppCookie)
{
	HRESULT        hres = S_OK;
	DaDeviceItem*   pDItem = NULL;
	DeviceItem* pItem;

	int   numCustomProperties = 0;

	int*  propertyIDs = NULL;

	try {
		// Search the Device Item
		CHECK_RESULT(FindDeviceItem(szItemID, &pDItem))
			// FindDeviceItem() attaches the returned Device Item
			LPDWORD pdwPropIDs = NULL;

		pItem = (DeviceItem *)pDItem;

		DWORD   dwCount = 8;                         // The Standard Properties of ID Set 1 are

		// Device item is requested and item not found, so request from DLL
#ifdef _OPC_NET
		hres = GenericServerAPI::OnQueryProperties((IntPtr)pItem, &numCustomProperties, &propertyIDs);
#endif

#ifdef _OPC_DLL
		if (pOnQueryProperties != NULL)
		{
			hres = (*pOnQueryProperties)(pItem, &numCustomProperties, &propertyIDs);
		}
#endif
		if (hres == S_OK) {
			dwCount += numCustomProperties;
		}

		// available for all Device Items.
		CHECK_PTR(pdwPropIDs = new DWORD[dwCount])

			pdwPropIDs[0] = OPC_PROPERTY_DATATYPE;       // Set the Standard Properties of ID Set 1
		pdwPropIDs[1] = OPC_PROPERTY_VALUE;
		pdwPropIDs[2] = OPC_PROPERTY_QUALITY;
		pdwPropIDs[3] = OPC_PROPERTY_TIMESTAMP;
		pdwPropIDs[4] = OPC_PROPERTY_ACCESS_RIGHTS;
		pdwPropIDs[5] = OPC_PROPERTY_SCAN_RATE;
		pdwPropIDs[6] = OPC_PROPERTY_EU_TYPE;
		pdwPropIDs[7] = OPC_PROPERTY_EU_INFO;

		for (int i = 0; i < numCustomProperties; i++) {
			pdwPropIDs[i + 8] = propertyIDs[i];
		}


		*pdwCount = dwCount;                    // All succeeded
		*ppdwPropIDs = pdwPropIDs;
		*ppCookie = pDItem;
	}
	catch (HRESULT hresEx) {
		if (pDItem) {
			pDItem->Detach();                         // Attach from FindDeviceItem() no longer used
		}
		hres = hresEx;
	}
	LOGFMTT("OnQueryProperties() finished with hres = 0x%x.", hres);
	return hres;
}


//=============================================================================
// Get Item Property Value
// -----------------------
//    Returns the Property Value of the specified Property/ItemId.
//
// Inputs:
//    szItemID                The ItemId
//    dwPropID                The ID of the requested Property.
//    pCookie                 The application specific value set by
//                            function OnQueryItemProperties().
// Outputs:
//    pvPropData              Where to store the value of the requested
//                            property. This parameter of type VARIANT has
//                            already been initialized by the caller.
//
// Note:
//    The generic server part has already checked if the specified ItemId
//    is valid and if the property is supported by the ItemId.
//
// SampleServer:
//    The Device Item pointer stored in the 'cookie' parameter is
//    used to read the values of the standard properties of ID Set 1 and
//    of ID Set 2 with the property IDs OPC_PROP_HIEU and OPC_PROP_LOEU.
//============================================================================
HRESULT DaServer::OnGetItemProperty(
	/*[in]         */          LPCWSTR        szItemID,
	/*[in]         */          DWORD          dwPropID,
	/*[out]        */          LPVARIANT      pvPropData,
	/*[in]         */          LPVOID         pCookie)
{
	HRESULT        hres = S_OK;
	DaDeviceItem*   pDItem = NULL;
	DeviceItem* pItem;

	pDItem = static_cast<DaDeviceItem*>(pCookie);
	pItem = (DeviceItem *)pDItem;


	hres = pDItem->get_PropertyValue(
		this,
		dwPropID,
		pvPropData);

	if (hres != S_OK) {

		// Device item is requested and item not found, so request from DLL
#ifdef _OPC_NET
		VARIANT      propertyValue;
		VariantInit(&propertyValue);
		hres = GenericServerAPI::OnGetPropertyValue((IntPtr)pItem, dwPropID, &propertyValue);
		// Note : VariantCopy() frees the destination variant if all succeeded.
		hres = VariantCopy(pvPropData, &propertyValue);			// Set the new value

#endif

#ifdef _OPC_DLL
		if (pOnGetPropertyValue != NULL)
		{
			hres = (*pOnGetPropertyValue)(pItem, dwPropID, pvPropData);
		}
#endif
	}
	LOGFMTT("OnGetPropertyValue() finished with hres = 0x%x.", hres);
	return hres;

}


//*****************************************************************************
//* INFO: The following function is declared as virtual and
//*       implementation is therefore optional.
//*       The following code is only here for informational purpose
//*       and if no server specific handling is needed the code
//*       can savely be deleted.
//*****************************************************************************
//=============================================================================
// Lookup Item ID
// --------------
//    Returns the ItemId associated with the property of the specified
//    item.
//
// Inputs:
//    szItemID                The ItemId
//    dwPropID                The ID of the requested Property.
//    pCookie                 The application specific value set by
//                            function OnQueryItemProperties().
// Outputs:
//    pszNewItemId            Where to store the text string of the
//                            associated ItemId.
//
// Return:
//    S_OK                    All succeeded
//    E_FAIL                  There is no associated ItemId
//    E_OUTOFMEMORY           Not enough memory
//
// Note:
//    The generic server part has already checked if the specified ItemId
//    is valid and if the property is supported by the ItemId. This
//    function is not called for the properties of ID Set 1 because
//    these properties has never associated ItemIds (see specification).
//    Memory for the returned string must be allocated from the
//    Global COM Memory Manager. You can use the function
//    WSTRClone( ..., pIMalloc );
//
// SampleServer:
//    This function is not overloaded because there are no properties
//    associated with ItemIds.
//=============================================================================
// HRESULT DaServer::OnLookupItemId(
//       /*[in]         */          LPCWSTR        szItemID,
//       /*[in]         */          DWORD          dwPropID,
//       /*[out]        */          LPWSTR      *  pszNewItemId,
//       /*[in]         */          LPVOID         pCookie )
// {
//    return E_FAIL;
// }


//=============================================================================
// Release Property Cookie
// -----------------------
//    This function is called by the generic server part if the 'cookie'
//    used by the Item Property functions is no longer used.
//
// Inputs:
//    pCookie                 The application specific value set by
//                            function OnQueryItemProperties().
//
// SampleServer:
//    Detaches the Device Item stored in the 'cookie' parameter.
//    OnQueryItemProperties() has attached the Device Item.
//=============================================================================
HRESULT DaServer::OnReleasePropertyCookie(
	/*[in]         */          LPVOID         pCookie)
{
	static_cast<DaDeviceItem*>(pCookie)->Detach();
	return S_OK;
}


//-----------------------------------------------------------------------------
// DaServer Specific Functions
//-----------------------------------------------------------------------------

//=============================================================================
// Delete and free the defined Items of the server                     INTERNAL
// -----------------------------------------------
//    This method is used only from the class destructor.
//    The items are deleted independent of their state. It is assumed that
//    all threads making access to the item object were previously deleted.
//    The memory of the item instances is freed. 
//
//    Some servers may need do handle the hardware before the server
//    terminates.
//=============================================================================
HRESULT DaServer::DeleteServerItems()
{
	m_ItemListLock.BeginWriting();               // protect item list access
	// we modify the item list
	m_SASRoot.RemoveAll();
	m_arServerItems.RemoveAll();

	m_ItemListLock.EndWriting();                 // release item list protection
	return S_OK;
}


//=============================================================================
// Adds a Device Item to the Server Address Space                      INTERNAL
//=============================================================================
HRESULT DaServer::AddDeviceItem(DeviceItem* pDItem)
{
	HRESULT hres;
	m_ItemListLock.BeginWriting();               // protect item list access
	// we modify the item list
	hres = m_arServerItems.Add(pDItem) ? S_OK : E_OUTOFMEMORY;

	if (SUCCEEDED(hres)) {
		LPWSTR pwszItemID;
		hres = pDItem->get_ItemIDPtr(&pwszItemID);
		if (SUCCEEDED(hres)) {
			hres = m_SASRoot.AddDeviceItem(pwszItemID, pDItem);
		}
		if (FAILED(hres)) {
			m_arServerItems.Remove(pDItem);
		}
	}

	m_ItemListLock.EndWriting();                 // release item list protection
	return hres;
}

//=============================================================================
// Removes a Device Item from the Server Address Space                 INTERNAL
//=============================================================================
HRESULT DaServer::RemoveDeviceItem(DeviceItem* pDItem)
{
	HRESULT hres;
	LPWSTR pwszItemID;
	m_ItemListLock.BeginWriting();               // protect item list access
	// we modify the item list
	hres = pDItem->get_ItemIDPtr(&pwszItemID);
	if (SUCCEEDED(hres)) {
		hres = m_SASRoot.RemoveDeviceItemAssociatedLeaf(pwszItemID);
	}
	if (SUCCEEDED(hres)) {
		pDItem->Kill(false);
		if (pDItem->get_RefCount() == 0) {
			DeleteDeviceItem(pDItem);
		}
	}
	m_ItemListLock.EndWriting();                 // release item list protection
	return hres;
}

//=============================================================================
// Delete the Device Item from the Server Address Space                INTERNAL
//=============================================================================
void DaServer::DeleteDeviceItem(DeviceItem* pDItem)
{
	if (pDItem->Killed()) {
		m_arServerItems.Remove(pDItem);
	}
}

//=============================================================================
// Searches the Device Item with the specified ItemId                  INTERNAL
// --------------------------------------------------
//    First the syntax of the specified Item is is checked.
//
//    ATTENTION:  The returned Device Item is attached.
//                The caller of this function must use the Detach()
//                function if the Device Item is no longer used.
//
// Return:
//    S_OK                    All succeeded
//    OPC_E_UNKNOWNITEMID     The ItemId is not in the SAS
//    OPC_E_INVALIDItemId     The ItemId is not syntactically valid
//=============================================================================
HRESULT DaServer::FindDeviceItem(LPCWSTR szItemID, DaDeviceItem** ppDItem)
{
	*ppDItem = NULL;

	//
	// ItemId Syntax Check
	//
	if (!szItemID || *szItemID == L'\0') {       // Check ItemId
		return OPC_E_INVALIDITEMID;               // Is not syntactically valid
	}
	else {

		// TODO: for own Server Implementation
		// ...
		// ...   You can do additional syntax checks for the item id here.
		// ...   If the item id is not syntactically valid use the error
		// ...   code OPC_E_INVALIDItemId.
		// ...
		// TODO: for own Server Implementation
	}


	//
	// Search the Item with the specified ID
	//

	m_ItemListLock.BeginReading();               // Protect item list access

	if (SUCCEEDED(m_SASRoot.FindDeviceItem(szItemID, ppDItem))) {
		(*ppDItem)->Attach();
	}

	m_ItemListLock.EndReading();                 // Release item list protection

	return (*ppDItem) ? S_OK : OPC_E_UNKNOWNITEMID;
}


//=============================================================================
// IMPLEMENTATION DaDeviceCache
//=============================================================================

//=============================================================================
// Constructor                                                         INTERNAL
//=============================================================================
DaDeviceCache::DaDeviceCache()
{
	m_paHandles = NULL;
	m_paItemVQTs = NULL;
	m_paErrors = NULL;
	m_paQualities = NULL;
	m_paTimeStamps = NULL;
}



//=========================================================================
// Initializer                                                     INTERNAL
//=========================================================================
HRESULT DaDeviceCache::Create(DWORD dwSize)
{
	try {
		m_paHandles = new void*[dwSize];
		if (!m_paHandles) throw NULL;

		m_paItemVQTs = new OPCITEMVQT[dwSize];
		if (!m_paItemVQTs) throw NULL;

#ifdef _OPC_DLL
		m_paErrors = new HRESULT[dwSize];
#else
		m_paErrors = new HRESULT[dwSize];
#endif
		if (!m_paErrors) throw NULL;

		m_paQualities = new WORD[dwSize];
		if (!m_paQualities) throw NULL;

#ifdef _OPC_NET
		m_paTimeStamps = new _FILETIME[dwSize];
#endif

#ifdef _OPC_DLL
		m_paTimeStamps = new FILETIME[dwSize];
#endif
		if (!m_paTimeStamps) throw NULL;
		return S_OK;
	}
	catch (...) {
		Cleanup();
		return E_OUTOFMEMORY;
	}
}


//=============================================================================
// Destructor                                                          INTERNAL
//=============================================================================
DaDeviceCache::~DaDeviceCache()
{
	Cleanup();
}


//=============================================================================
// Cleanup                                                             INTERNAL
//=============================================================================
void DaDeviceCache::Cleanup()
{
	if (m_paHandles)     delete[] m_paHandles;
	if (m_paItemVQTs)    delete[] m_paItemVQTs;
	if (m_paErrors)      delete[] m_paErrors;
	if (m_paQualities)   delete[] m_paQualities;
	if (m_paTimeStamps)  delete[] m_paTimeStamps;

	m_paHandles = NULL;
	m_paItemVQTs = NULL;
	m_paErrors = NULL;
	m_paQualities = NULL;
	m_paTimeStamps = NULL;
}