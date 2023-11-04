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

#ifndef __DASERVER_H_
#define __DASERVER_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "DaBaseServer.h"
#include "DaAddressSpace.h"


//-----------------------------------------------------------------------------
// MACRO
//-----------------------------------------------------------------------------
#define CHECK_RESULT(f) {HRESULT hres = f; if (FAILED( hres )) throw hres;}
#define CHECK_PTR(p) {if ((p)== NULL) throw E_OUTOFMEMORY;}
#define CHECK_ADD(f) {if (!(f)) throw E_OUTOFMEMORY;}
#define CHECK_BOOL(f) {if (!(f)) throw E_FAIL;}


//-----------------------------------------------------------------------------
// INTERFACE DEFINITION
//-----------------------------------------------------------------------------
// Pointers to the functions of the DLL
#ifdef _OPC_DLL
#define DLLCALL __stdcall
#define DLLIMP __declspec(dllimport)
#endif

//-----------------------------------------------------------------------------
// Structure with Server Registry Definitions of the DLL.
// Do not change or delete !
//-----------------------------------------------------------------------------
typedef struct _tagSERVER_REGDEF {
	WCHAR * ClsidServer; // CLSID of current Server
	WCHAR * ClsidApp; // CLSID of current Server Application
	WCHAR * PrgidServer; // Version independent Prog.Id.
	WCHAR * PrgidCurrServer; // Prog.Id. of current Server
	WCHAR * NameServer; // Friendly name of server
	WCHAR * NameCurrServer; // Friendly name of current server version
	WCHAR * CompanyName; // Server vendor name.
} ServerRegDefs;

#ifdef _OPC_DLL
#include "IClassicBaseNodeManager.h"
using namespace IClassicBaseNodeManager;
#endif


HRESULT CreateOneItem(LPWSTR szItemID,
	DWORD dwAccessRights,
	LPVARIANT pvValue,
	BOOL fActive,
	OPCEUTYPE eEUType,
	LPVARIANT pvEUInfo,
	void** deviceItem
	);

#ifdef _OPC_NET
typedef struct _tagSERVER_REGDEF1 {
	WCHAR ClsidServer[256]; // CLSID of current Server
	WCHAR ClsidApp[256]; // CLSID of current Server Application
	WCHAR PrgidServer[256]; // Version independent Prog.Id.
	WCHAR PrgidCurrServer[256]; // Prog.Id. of current Server
	WCHAR NameServer[256]; // Friendly name of server
	WCHAR NameCurrServer[256]; // Friendly name of current server version
	WCHAR CompanyName[256]; // Server vendor name.
} ServerRegDefs1;
#endif

#ifdef _OPC_DLL
HRESULT DLLCALL AddItem(
	LPWSTR szItemID,
	DaAccessRights dwAccessRights,
	LPVARIANT pvValue,
	bool fActive,
	DaEuType eEUType,
	double minValue,
	double maxValue,
	void** deviceItemHandle
	);

HRESULT DLLCALL RemoveItem(void* deviceItem);

HRESULT DLLCALL AddProperty(int propertyID, LPWSTR description, LPVARIANT valueType);


HRESULT DLLCALL SetItemValue(
	void* deviceItem,
	LPVARIANT newValue,
	short quality,
	FILETIME timestamp
	);


void DLLCALL SetServerState(ServerState serverState);

void DLLCALL GetActiveItems(int * dwNumItemHandles, void* **pItemHandles);

HRESULT DLLCALL AddSimpleEventCategory(int categoryID, LPWSTR categoryDescription);

HRESULT DLLCALL AddTrackingEventCategory(int categoryID, LPWSTR categoryDescription);

HRESULT DLLCALL AddConditionEventCategory(int categoryID, LPWSTR categoryDescription);

HRESULT DLLCALL AddEventAttribute(int categoryID, int eventAttribute, LPWSTR attributeDescription, VARTYPE dataType);

HRESULT DLLCALL AddSingleStateConditionDefinition(int categoryId, int conditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired);

HRESULT DLLCALL AddMultiStateConditionDefinition(int categoryId, int conditionId, LPWSTR name);

HRESULT DLLCALL AddSubConditionDefinition(int conditionId, int subConditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired);

HRESULT DLLCALL AddArea(int parentAreaId, int areaId, LPWSTR name);

HRESULT DLLCALL AddSource(int areaId, int sourceId, LPWSTR sourceName, bool multiSource);

HRESULT DLLCALL AddExistingSource(int areaId, int sourceId);

HRESULT DLLCALL AddCondition(int sourceId, int conditionDefinitionId, int conditionId);

HRESULT DLLCALL ProcessSimpleEvent(int categoryId, int sourceId, LPWSTR message, int severity, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp);

HRESULT DLLCALL ProcessTrackingEvent(int categoryId, int sourceId, LPWSTR message, int severity, LPWSTR actorId, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp);

HRESULT DLLCALL ProcessConditionStateChanges(int count, AeConditionState* conditionStateChanges);

HRESULT DLLCALL AckCondition(int conditionId, LPWSTR comment);

void DLLCALL GetConnectedClients(int * numClientHandles, void* ** clientHandles, LPWSTR ** clientNames);

void DLLCALL GetClientGroups(void * clientHandle, int * numGroupHandles, void* ** groupHandles, LPWSTR ** groupNames);

void DLLCALL GetGroupState(void * groupHandle, DaGroupState * groupState);

void DLLCALL GetItemStates(void * groupHandle, DaGroupState * groupState);

void DLLCALL RequestShutdown(LPCWSTR reason);

typedef DLLIMP ServerRegDefs * (DLLCALL * PFNONGETDASEERVERREGISTRYDEFINITION)(void);
typedef DLLIMP ServerRegDefs * (DLLCALL * PFNONGETAESEERVERREGISTRYDEFINITION)(void);
typedef DLLIMP HRESULT(DLLCALL * PFNONGETDASERVERPARAMETERS) (int *, WCHAR *, int *);
typedef DLLIMP HRESULT(DLLCALL * PFNONDEFINEDACALLBACKS) (AddItemPtr AddItem, RemoveItemPtr RemoveItem, AddPropertyPtr AddProperty, SetItemValuePtr SetItemValue, SetServerStatePtr SetServerState, GetActiveItemsPtr GetActiveItems, FireShutdownRequestPtr fireShutdownRequest, GetClientsPtr getClients, GetGroupsPtr getGroups, GetGroupStatePtr getGroupState, GetItemStatesPtr getItemStates);
typedef DLLIMP HRESULT(DLLCALL * PFNONCREATESERVERITEMS) ();
typedef DLLIMP HRESULT(DLLCALL * PFNONCLIENTCONNECT) (void);
typedef DLLIMP HRESULT(DLLCALL * PFNONCLIENTDISCONNECT) (void);
typedef DLLIMP HRESULT(DLLCALL * PFNONREQUESTITEMS) (int numItems, LPWSTR *fullItemIds, VARTYPE *dataTypes);
typedef DLLIMP void (DLLCALL * PFNONSTARTUPSIGNAL) (char* pszParam);
typedef DLLIMP void (DLLCALL * PFNONSHUTDOWNSIGNAL) (void);
typedef DLLIMP HRESULT(DLLCALL * PFNONREFRESHITEMS) (int numItems, void *itemHandles);
typedef DLLIMP HRESULT(DLLCALL * PFNONWRITEITEMS) (int numItems, void *itemHandles, OPCITEMVQT * pItemVQTs, HRESULT *errors);
typedef DLLIMP HRESULT(DLLCALL * PFNONBROWSECHANGEPOSITION) (OPCBROWSEDIRECTION dwBrowseDirection, LPCWSTR szString, LPWSTR * szActualPosition);
typedef DLLIMP HRESULT(DLLCALL * PFNONBROWSEItemIdS) (LPWSTR szActualPosition, OPCBROWSETYPE dwBrowseFilterType, LPWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter, int * pNrItemIds, LPWSTR ** ppItemIds);
typedef DLLIMP HRESULT(DLLCALL * PFNONBROWSEGETFULLItemId) (LPWSTR szActualPosition, LPWSTR szItemDataID, LPWSTR * szItemID);
typedef DLLIMP HRESULT(DLLCALL * PFNONQUERYPROPERTIES) (void* itemHandle, int * pdwCount, int** ppdwPropIDs);
typedef DLLIMP HRESULT(DLLCALL * PFNONGETPROPERTYVALUE) (void* itemHandle, int propertyID, LPVARIANT propertyValue);

typedef DLLIMP HRESULT(DLLCALL * PFNONADDITEM) (void *itemHandle);
typedef DLLIMP HRESULT(DLLCALL * PFNONREMOVEITEM) (void *itemHandle);
typedef DLLIMP HRESULT(DLLCALL * PFNONGETDAOPTIMIZATIONPARAMETERS) (bool *useOnItemRequest, bool * useOnRefreshItems, bool * useOnAddItem, bool * useOnRemoveItem);

typedef DLLIMP HRESULT(DLLCALL * PFNONDEFINEAECALLBACKS) (AddSimpleEventCategoryPtr AddSimpleEventCategory, AddTrackingEventCategoryPtr AddTrackingEventCategory,
	AddConditionEventCategoryPtr AddConditionEventCategory, AddEventAttributePtr AddEventAttribute,
	AddSingleStateConditionDefinitionPtr AddSingleStateConditionDefinition, AddMultiStateConditionDefinitionPtr AddMultiStateCondition,
	AddSubConditionDefinitionPtr addSubCondition, AddAreaPtr AddArea, AddSourcePtr AddSource, AddExistingSourcePtr AddExistingSource,
	AddConditionPtr AddCondition, ProcessSimpleEventPtr ProcessSimpleEvent, ProcessTrackingEventPtr ProcessTrackingEvent,
	ProcessConditionStateChangesPtr ProcessConditionStateChanges, AckConditionPtr AckCondition);

typedef DLLIMP HRESULT(DLLCALL * PFNONTRANSLATETOItemId) (int dwCondID, int dwSubCondDefID, int dwAttrID, LPWSTR* pszItemID, LPWSTR* pszNodeName, CLSID* pCLSID);
typedef DLLIMP HRESULT(DLLCALL * PFNONACKNOTIFICATION) (int dwCondID, int dwSubCondDefID);

typedef DLLIMP HRESULT(DLLCALL * PFNONGETLICENSEINFORMATION) (LPWSTR * userName, LPWSTR * serialNumber);

typedef DLLIMP int (DLLCALL * PFNONGETLOGLEVEL) ();
typedef DLLIMP HRESULT(DLLCALL * PFNONGETLOGPATH) (char * logPath);

extern PFNONGETDASEERVERREGISTRYDEFINITION pOnGetDaServerDefinition;
extern PFNONGETAESEERVERREGISTRYDEFINITION pOnGetAeServerDefinition;
extern PFNONGETDASERVERPARAMETERS pOnGetDaServerParameters;
extern PFNONDEFINEDACALLBACKS pOnDefineDaCallbacks;
extern PFNONCREATESERVERITEMS pOnCreateServerItems;
extern PFNONCLIENTCONNECT pOnClientConnect;
extern PFNONCLIENTDISCONNECT pOnClientDisconnect;
extern PFNONREQUESTITEMS pOnRequestItems;
extern PFNONSTARTUPSIGNAL pOnStartupSignal;
extern PFNONSHUTDOWNSIGNAL pOnShutdownSignal;
extern PFNONREFRESHITEMS pOnRefreshItems;
extern PFNONWRITEITEMS pOnWriteItems;
extern PFNONBROWSECHANGEPOSITION pOnBrowseChangePosition;
extern PFNONBROWSEItemIdS pOnBrowseItemIds;
extern PFNONBROWSEGETFULLItemId pOnBrowseGetFullItemId;
extern PFNONQUERYPROPERTIES pOnQueryProperties;
extern PFNONGETPROPERTYVALUE pOnGetPropertyValue;

extern PFNONGETDAOPTIMIZATIONPARAMETERS pOnGetDaOptimizationParameters;
extern PFNONADDITEM pOnAddItem;
extern PFNONREMOVEITEM pOnRemoveItem;

extern PFNONDEFINEAECALLBACKS pOnDefineAeCallbacks;
extern PFNONTRANSLATETOItemId pOnTranslateToItemId;
extern PFNONACKNOTIFICATION pOnAckNotification;
extern PFNONGETLICENSEINFORMATION pOnGetLicenseInformation;

extern PFNONGETLOGLEVEL pOnGetLogLevel;
extern PFNONGETLOGPATH pOnGetLogPath;


#endif
//-----------------------------------------------------------------------------
// CLASS DaDeviceCache
//-----------------------------------------------------------------------------
// Utility class used by OnRefreshOutputDevices()
class DaDeviceCache
{
	// Constrution
public:
	DaDeviceCache();
	HRESULT Create(DWORD dwSize);

	// Destruction
	~DaDeviceCache();

	// Attributes
public:
	void** m_paHandles;
	OPCITEMVQT* m_paItemVQTs;
	HRESULT* m_paErrors;
	WORD* m_paQualities;
	FILETIME* m_paTimeStamps;

	// Implementation
protected:
	void Cleanup();
};


//-----------------------------------------------------------------------------
// CLASS DeviceItem
//-----------------------------------------------------------------------------
// My Server Specific Item Definition
class DeviceItem : public DaDeviceItem
{
	// Construction
public:
	DeviceItem();
	inline HRESULT Create( // Initializer
		LPWSTR szItemID,
		DWORD dwAccessRights,
		LPVARIANT pvValue,
		BOOL fActive = TRUE,
		DWORD dwBlobSize = 0,
		LPBYTE pBlob = NULL,
		LPWSTR szAccessPath = NULL,
		OPCEUTYPE eEUType = OPC_NOENUM,
		LPVARIANT pvEUInfo = NULL
		) {

		return DaDeviceItem::Create(szItemID,
			dwAccessRights,
			pvValue,
			fActive,
			dwBlobSize,
			pBlob,
			szAccessPath,
			eEUType,
			pvEUInfo);
	}

	// Destruction
	~DeviceItem();

	HRESULT AttachActiveCount(void);
	HRESULT DetachActiveCount(void);

};


//-----------------------------------------------------------------------------
// CLASS DaServer
//-----------------------------------------------------------------------------
// My Server Specific Data Access Implementation
class DaServer : public DaBaseServer
{
	friend unsigned __stdcall NotifyUpdateThread(LPVOID pAttr);

	// Construction
public:
	DaServer();
	HRESULT Create(DWORD dwUpdateRate, LPCTSTR szDLLParams, LPCTSTR szInstanceNamebool);

	// Destruction
	~DaServer();

	// Attributes
public:
	inline OPCSERVERSTATE ServerState() const { return m_dwServerState; }
	inline void SetServerState(OPCSERVERSTATE serverState) { m_dwServerState = serverState; m_fServerStateChanged = TRUE; }
	inline DWORD BandWidth() const { return m_dwBandWith; }
	inline DaBranch* SASRoot() { return &m_SASRoot; }



	// Operations
public:
	HRESULT AddDeviceItem(DeviceItem* pDItem);
	HRESULT RemoveDeviceItem(DeviceItem* pDItem);
	void DeleteDeviceItem(DeviceItem* pDItem);

	// array of items supported by this server
	CSimplePtrArray<DeviceItem*> m_arServerItems;

	// Use the members of this object to lock and unlock
	// critical sections to allow consistent access of item
	// list (m_ServerItems), avoiding interferences with
	// some refresh threads.
	ReadWriteLock m_ItemListLock;
	ReadWriteLock m_OnRequestItemsLock;

	// Implementation
protected:

	/////////////////////////////////////////////////////////////////////////////
	// Implementation of the pure virtual functions of the DaBaseServer //
	/////////////////////////////////////////////////////////////////////////////

	////////////////////////////////
	// Common DA Server Functions //
	////////////////////////////////

	HRESULT OnGetServerState(
		/*[out] */ DWORD & bandWidth,
		/*[out] */ OPCSERVERSTATE & serverState,
		/*[out] */ LPWSTR & vendor);

	// use to add/release custom specific client related data
	HRESULT OnCreateCustomServerData(LPVOID* customData);

	//*************************************************************************
	//* INFO: The following function is declared as virtual and
	//* implementation is therefore optional.
	//* The following code is only here for informational purpose
	//* and if no server specific handling is needed the code
	//* can savely be deleted.
	//*************************************************************************
	HRESULT OnDestroyCustomServerData(LPVOID* customData);

	///////////////////////
	// Validate Function //
	///////////////////////

	HRESULT OnValidateItems(
		/*[in] */ OPC_VALIDATE_REQUEST
		validateRequest,
		/*[in] */ BOOL blobUpdate,
		/*[in] */ DWORD numItems,
		/*[in, size_is(numItems)] */ OPCITEMDEF * itemDefinitions,
		/*[out, size_is(numItems)] */ DaDeviceItem ** daDeviceItems,
		/*[out, size_is(numItems)] */ OPCITEMRESULT * itemResults,
		/*[out, size_is(numItems)] */ HRESULT * errors
		);


	///////////////////////
	// Refresh Functions //
	///////////////////////

	HRESULT OnRefreshInputCache(
		/*[in] */ OPC_REFRESH_REASON dwReason,
		/*[in] */ DWORD numItems,
		/*[in, size_is(,numItems)] */ DaDeviceItem ** pDevItemPtr,
		/*[in, size_is(,numItems)] */ HRESULT * errors
		);

	HRESULT OnRefreshOutputDevices(
		/*[in] */ OPC_REFRESH_REASON dwReason,
		/*[in, string] */ LPWSTR szActorID,
		/*[in] */ DWORD numItems,
		/*[in, size_is(,numItems)] */ DaDeviceItem ** pDevItemPtr,
		/*[in, size_is(,numItems)] */ OPCITEMVQT * pItemVQTs,
		/*[in, size_is(,numItems)] */ HRESULT * errors
		);

	///////////////////////////////////////////
	// Server Address Space Browse Functions //
	///////////////////////////////////////////

	OPCNAMESPACETYPE OnBrowseOrganization();

	HRESULT OnBrowseChangeAddressSpacePosition(
		/*[in] */ OPCBROWSEDIRECTION
		dwBrowseDirection,
		/*[in, string] */ LPCWSTR szString,
		/*[in,out] */ LPWSTR * szActualPosition,
		/*[in,out] */ LPVOID * customData);


	HRESULT OnBrowseGetFullItemIdentifier(
		/*[in] */ LPWSTR szActualPosition,
		/*[in] */ LPWSTR szItemDataID,
		/*[out, string]*/ LPWSTR * szItemID,
		/*[in,out] */ LPVOID * customData);

	HRESULT OnBrowseItemIdentifiers(
		/*[in] */ LPWSTR szActualPosition,
		/*[in] */ OPCBROWSETYPE dwBrowseFilterType,
		/*[in, string] */ LPWSTR szFilterCriteria,
		/*[in] */ VARTYPE vtDataTypeFilter,
		/*[in] */ DWORD dwAccessRightsFilter,
		/*[out] */ DWORD * pNrItemIds,
		/*[out, string]*/ LPWSTR ** ppItemIds,
		/*[in,out] */ LPVOID * customData);

	//*************************************************************************
	//* INFO: The following function is declared as virtual and
	//* implementation is therefore optional.
	//* The following code is only here for informational purpose
	//* and if no server specific handling is needed the code
	//* can savely be deleted.
	//*************************************************************************
	// HRESULT OnBrowseAccessPaths(
	// /*[in] */ LPWSTR szItemID,
	// /*[out] */ DWORD * pNrAccessPaths,
	// /*[out, string]*/ LPWSTR ** szAccessPaths,
	// /*[in,out] */ LPVOID * customData );

	/////////////////////////////
	// Item Property Functions //
	/////////////////////////////

	HRESULT OnQueryItemProperties(
		/*[in] */ LPCWSTR szItemID,
		/*[out] */ LPDWORD pdwCount,
		/*[out] */ LPDWORD * ppdwPropIDs,
		/*[out] */ LPVOID * ppCookie);

	HRESULT OnGetItemProperty(
		/*[in] */ LPCWSTR szItemID,
		/*[in] */ DWORD dwPropID,
		/*[out] */ LPVARIANT pvPropData,
		/*[in] */ LPVOID pCookie);

	//*************************************************************************
	//* INFO: The following function is declared as virtual and
	//* implementation is therefore optional.
	//* The following code is only here for informational purpose
	//* and if no server specific handling is needed the code
	//* can savely be deleted.
	//*************************************************************************
	// HRESULT OnLookupItemId(
	// /*[in] */ LPCWSTR szItemID,
	// /*[in] */ DWORD dwPropID,
	// /*[out] */ LPWSTR * pszNewItemId,
	// /*[in] */ LPVOID pCookie );

	HRESULT OnReleasePropertyCookie(
		/*[in] */ LPVOID pCookie);

	//////////////////////////////////////////////////////////////
	// Implementation internal functions (application specific) //
	//////////////////////////////////////////////////////////////

	HRESULT DeleteServerItems();
	HRESULT CreateUpdateThread();
	HRESULT KillUpdateThread();
	HRESULT FindDeviceItem(LPCWSTR szItemID, DaDeviceItem** ppDItem);


	// Handle of the Refresh Thread
	HANDLE m_hUpdateThread;
	// Signal-Handle to terminate the threads.
	HANDLE m_hTerminateThraedsEvent;

	OPCSERVERSTATE m_dwServerState;
	BOOL m_fServerStateChanged;
	DWORD m_dwBandWith;

	// Root of the hierarchical Server Address Space
	DaBranch m_SASRoot;

	BOOL m_fCreated;
};

// The Global Data Server Handler
extern DaServer* gpDataServer;

#endif // __DASERVER_H_
