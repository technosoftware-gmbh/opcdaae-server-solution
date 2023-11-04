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
#include "DaServer.h"
#include "AeServer.h"
#include <comdef.h>
#include <msclr\marshal_cppstd.h>
#include "IClassicBaseNodeManager.h" 

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

//-----------------------------------------------------------------------------
// USINGS
//-----------------------------------------------------------------------------
using namespace System;
using namespace System::Runtime::CompilerServices;
using namespace System::Runtime::InteropServices;
using namespace ServerPlugin;

//-----------------------------------------------------------------------------
// GLOBALS
//-----------------------------------------------------------------------------
static ServerRegDefs1* srvRegDefsDa;
static ServerRegDefs1* srvRegDefsAe;

//-----------------------------------------------------------------------------
// CODE
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// This class defines the generic server interface.
// The GenericServerCallbacks provides a set of generic server callback 
// methods. These methods can be used to read information from the generic 
// server or change data in the generic server. They are always called by the 
// customization plugin.
//-----------------------------------------------------------------------------
ref struct GenericServerCallbacks {
	
	//=========================================================================
    // This function is called by the customization plugin and add an item to 
	// the generic server cache. If DaBrowseMode.Generic is set the item is 
	// also added to the the generic server internal browse hierarchy. 
	//
	// The generic server sets the EU type to DaEuType.noEnum and define an 
	// empty EUInfo. 
	//
	// This method is typically called during the execution of the 
	// OnCreateServerItems method, but it can be called anytime an item needs 
	// to be added to the generic server cache.
	//
	// Items that are added to the generic server cache can be accessed by 
	// OPC clients.
	//=========================================================================
	static Int32 AddItem( String^ ItemId, 
						  ServerPlugin::DaAccessRights AccessRights, 
						  Object^ InitValue, 
						  Boolean Active, 
						  ServerPlugin::DaEuType EUType, 
						  double MinValue, 
						  double MaxValue,
						  IntPtr% deviceItem)
	{
		HRESULT			hres = S_OK;
		char *			pStr = NULL;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc = NULL;
		VARIANT			varVal;
		VariantInit( &varVal );
		DeviceItem*	classiDaDeviceItem = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(ItemId).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (pwc == NULL)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwc, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwc);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			Marshal::GetNativeVariantForObject(InitValue, IntPtr(&varVal));

			if ((OPCEUTYPE)EUType == OPC_NOENUM)
			{
				hres = CreateOneItem(
					pwc,								// ItemID
					(DWORD)AccessRights,				// AccessRights
					&varVal,							// Initial Value
					Active,								// Active State
					OPC_NOENUM,							// EU Type
					NULL,								// EU Info
					(void**)&classiDaDeviceItem
					);
			}
			else
			{
				if ((OPCEUTYPE)EUType == OPC_ANALOG)
				{
					// EU Info must contain exaclty two doubles 
					// corresponding to the LOW and HI EU range.
					VARIANT  varEUInfo;
					VariantInit( &varEUInfo );
					V_VT( &varEUInfo ) = VT_ARRAY | VT_R8;

					SAFEARRAYBOUND rgs;
					rgs.cElements  = 2;
					rgs.lLbound    = 0;

					try {
						CHECK_PTR( V_ARRAY( &varEUInfo ) = SafeArrayCreate( VT_R8, 1, &rgs ) )
							// put initialized variant into array
							double   dLow = MinValue;
						double   dHi  = MaxValue;
						long     lElIndex;

						lElIndex = 0;                       // LOW EU range
						CHECK_RESULT( SafeArrayPutElement( V_ARRAY( &varEUInfo ), &lElIndex, &dLow ) )
							lElIndex++;                         // HI  EU range
						CHECK_RESULT( SafeArrayPutElement( V_ARRAY( &varEUInfo ), &lElIndex, &dHi ) )
					}
					catch ( HRESULT hresEx ) {             // cleanup if failure
						VariantClear( &varEUInfo );
						throw hresEx;
					}
					hres = CreateOneItem(
						pwc,								// ItemID
						(DWORD)AccessRights,				// AccessRights
						&varVal,							// Initial Value
						Active,								// Active State
						OPC_ANALOG,							// EU Type
						&varEUInfo,							// EU Info
						(void**)&classiDaDeviceItem
						);
					VariantClear( &varEUInfo );

				}
			}
			deviceItem = (IntPtr)classiDaDeviceItem;
			VariantClear( &varVal );
			free(pwc);
		}
		return hres;
	};


	//=========================================================================
    // This function is called by the customization plugin and removes an item 
	// from the generic server cache. If DaBrowseMode.Generic is set the item 
	// is also removed from the the generic server internal browse hierarchy. 
	//
	// This method can be called anytime an item needs to be removed from the 
	// generic server cache.
	// 
	// Items that are removed from the generic server cache can no longer be 
	// accessed by OPC clients.
	//=========================================================================
	//static Int32 RemoveItem(Int32 AppItemHandle)
	//{
	//	HRESULT			hres;
	//	DeviceItem*  pItem = NULL;
	//	DWORD           i,dwSize;

	//	gpDataServer->readWriteLock_.BeginWriting();	// We are manipulating the DeviceItem make
	//	// sure no one else gets access during update.
	//	gpDataServer->m_ItemListLock.BeginReading();	// Protect item list access
	//	// Handle all items
	//	dwSize = (DWORD)gpDataServer->m_arServerItems.GetSize();
	//	for (i=0; i < dwSize; i++) {
	//		pItem = gpDataServer->m_arServerItems[i];
	//		if (pItem == NULL) {
	//			continue;								// Handle next item
	//		}
	//		if (pItem->m_dwAdaptHandle == AppItemHandle ) {
	//			break;
	//		}
	//		else
	//		{
	//			pItem = NULL;
	//		}
	//	}
	//	if (pItem == NULL) {
	//		gpDataServer->m_ItemListLock.EndReading();		// Release item list protection
	//		gpDataServer->readWriteLock_.EndWriting();     // UnLock DeviceItem
	//		return( S_FALSE );
	//	}
	//	gpDataServer->m_ItemListLock.EndReading();		// Release item list protection
	//	gpDataServer->readWriteLock_.EndWriting();     // UnLock DeviceItem
	//	hres = gpDataServer->RemoveDeviceItem( pItem );
	//	return hres;
	//};

	//=========================================================================
	// This function is called by the customization plugin and removes an item 
	// from the generic server cache. If DaBrowseMode.Generic is set the item 
	// is also removed from the the generic server internal browse hierarchy. 
	//
	// This method can be called anytime an item needs to be removed from the 
	// generic server cache.
	// 
	// Items that are removed from the generic server cache can no longer be 
	// accessed by OPC clients.
	//=========================================================================
	static Int32 RemoveItem(IntPtr deviceItem)
	{
		HRESULT			hres = S_OK;
		DeviceItem*  pItem = NULL;

		pItem = (DeviceItem*)(void*)deviceItem;
		if (pItem == NULL) {
			return(S_FALSE);
		}

		hres = gpDataServer->RemoveDeviceItem(pItem);
		return hres;
	};

	//=========================================================================
	// Adds a custom specific property to the generic server list of item 
	// properties. 
	//=========================================================================
	static Int32 AddProperty(Int32 propertyID, String^ description, Object^ valueType)
	{
		char *			pStr = NULL;
		HRESULT			hres = S_OK;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc;
		DaItemProperty*	pProp;
		VARIANT			varVal;
		VariantInit( &varVal );

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(description).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwc)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwc, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwc);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			Marshal::GetNativeVariantForObject(valueType, IntPtr(&varVal));
			hres = gpDataServer->GetItemProperty( propertyID, &pProp );
			if (FAILED( hres )) {	
				pProp = new DaItemProperty( propertyID, V_VT(&varVal), pwc);
				hres = gpDataServer->AddItemProperty( pProp );
			}

			VariantClear( &varVal );
			free(pwc);
		}
		return hres;
	};

	
	//=========================================================================
	// Write an item value into the cache.
	//=========================================================================
	static Int32 SetItemValue(IntPtr deviceItemHandle, Object^ NewValue, Int16 quality, DateTime timestamp)
	{
		HRESULT			hres = S_OK;
		VARIANT			varVal;
		DeviceItem*  pItem = NULL;
		_FILETIME 		ftimeStamp;

		gpDataServer->readWriteLock_.BeginWriting();	// We are manipulating the DeviceItem make
														// sure no one else gets access during update.
		pItem = (DeviceItem*)(void*)deviceItemHandle;
		if (pItem == NULL) {
			return(S_FALSE);
		}

		VariantInit( &varVal );
		Marshal::GetNativeVariantForObject(NewValue, IntPtr(&varVal));
		if (V_VT( &varVal ) != VT_NULL) {
			__int64 fileTime = timestamp.ToFileTime();
			DWORD test = (DWORD)fileTime;
			ftimeStamp.dwLowDateTime = test;
			test = (DWORD)(fileTime >> 32);
			ftimeStamp.dwHighDateTime = test;

			if (V_VT( &varVal ) == VT_EMPTY) {
				// Update only quality and time stamp
				hres = pItem->set_ItemQuality( quality, &ftimeStamp );
			}
			else {
				hres = pItem->set_ItemValue( &varVal, quality, &ftimeStamp );
				VariantClear( &varVal );
			}
		}
		gpDataServer->readWriteLock_.EndWriting();     // UnLock DeviceItem

		if (FAILED( hres )) {
			return( S_FALSE );
		}

		return S_OK;
	};

	
	//=========================================================================
	// Set the OPC server state value, that is returned in client GetStatus 
	// calls.
	//=========================================================================
	static void SetServerState( ServerPlugin::ServerState serverState )
	{
		gpDataServer->SetServerState((OPCSERVERSTATE)serverState);
	};

	//=========================================================================
	// Get a list of items used at least by one client.
	//=========================================================================
	static void GetActiveItems(Int32% numItemHandles, array<IntPtr>^% deviceItemHandles)
	{
	  DWORD             i, dwSize;
      DeviceItem*   pItem = NULL;

	  DWORD numHandles = 0;

      gpDataServer->m_ItemListLock.BeginReading();          // Protect item list access
															// Handle all items
      dwSize = (DWORD)gpDataServer->m_arServerItems.GetSize();
	  DeviceItem** activeItemHandles;

	  activeItemHandles = new DeviceItem*[dwSize];
      for (i=0; i < dwSize; i++) {
         pItem = gpDataServer->m_arServerItems[i];
         if (pItem == NULL) {
            continue;										// Handle next item
         }

         if (pItem->get_ActiveCount() > 0) {
			 activeItemHandles[numHandles] = pItem;
			 numHandles++;
		 }
      } // handle all items
      gpDataServer->m_ItemListLock.EndReading();			// Release item list protection

	  if (numHandles > 0)
	  {
			numItemHandles = numHandles;
			deviceItemHandles = gcnew array<IntPtr>(numHandles);
			for (i=0; i < numHandles; i++) {
				deviceItemHandles[i] = (IntPtr)activeItemHandles[i];
			}
	  }
	  else
	  {
		  numItemHandles = 0;
		  deviceItemHandles = gcnew array<IntPtr>(0);
	  }

	  delete[] activeItemHandles;
	};

    //=========================================================================
    // Get a list of clients connected to the server.
    //=========================================================================
    static void GetClients(Int32% numClientHandles, array<IntPtr>^% clientHandles, array<String^>^% clientNames)
    {
        int numHandles = 0;
        void** connectedClients;
        LPWSTR * connectedClientNames;

        gpDataServer->GetClients(&numHandles, &connectedClients, &connectedClientNames);

        if (numHandles > 0)
        {
            numClientHandles = numHandles;
            clientHandles = gcnew array<IntPtr>(numHandles);
            clientNames = gcnew array<String^>(numHandles);
            for (int i = 0; i < numHandles; i++) {
                if (connectedClients[i] == nullptr) {
                    continue;				    // Handle next client
                }
                clientHandles[i] = (IntPtr)connectedClients[i];
                if (connectedClientNames[i] != nullptr) {
                    msclr::interop::marshal_context context;
                    clientNames[i] = context.marshal_as<String^>(connectedClientNames[i]);
                    delete connectedClientNames[i];
                }
            }
            delete[] connectedClients;
        }

    };

    static void GetGroups(IntPtr clientHandle, Int32% numGroupHandles, array<IntPtr>^% groupHandles, array<String^>^% groupNames)
    {
        int numHandles = 0;
        void** groups;
        LPWSTR * grouNames;

        gpDataServer->GetGroups((void *)clientHandle, &numHandles, &groups, &grouNames);

        if (numHandles > 0)
        {
            numGroupHandles = numHandles;
            groupHandles = gcnew array<IntPtr>(numHandles);
            groupNames = gcnew array<String^>(numHandles);
            for (int i = 0; i < numHandles; i++) {
                if (groups[i] == nullptr) {
                    continue;				    // Handle next client
                }
                groupHandles[i] = (IntPtr)groups[i];
                if (grouNames[i] != nullptr) {
                    msclr::interop::marshal_context context;
                    groupNames[i] = context.marshal_as<String^>(grouNames[i]);
                    delete grouNames[i];
                }
            }
            delete[] groups;
        }

    };

    static void GetGroupState(IntPtr groupHandle, ServerPlugin::DaGroupState% groupState)
    {
        IClassicBaseNodeManager::DaGroupState groupInfo;

        gpDataServer->GetGroupState((void *)groupHandle, &groupInfo);

        msclr::interop::marshal_context context;
        groupState.GroupName = context.marshal_as<String^>(groupInfo.GroupName);
        groupState.ClientGroupHandle = groupInfo.ClientGroupHandle;
        groupState.DataChangeEnabled = groupInfo.DataChangeEnabled != 0;
        groupState.LocaleId = groupInfo.LocaleId;
        groupState.PercentDeadband = groupInfo.PercentDeadband;
        groupState.UpdateRate = groupInfo.UpdateRate;
    };

    static void GetItemState(IntPtr groupHandle, Int32% numItemStates, array<ServerPlugin::DaItemState>^% itemStates)
    {
        IClassicBaseNodeManager::DaItemState *deviceItems;
        int countStates = 0;

        gpDataServer->GetItemStates((void *)groupHandle, &countStates, &deviceItems);

        if (countStates >0)
        {
            msclr::interop::marshal_context context;
            numItemStates = countStates;
            itemStates = gcnew array<ServerPlugin::DaItemState>(countStates);
            for (int i = 0; i < countStates; i++) {
                itemStates[i].ItemName = context.marshal_as<String^>(deviceItems[i].ItemName);
                itemStates[i].AccessPath = context.marshal_as<String^>(deviceItems[i].AccessPath);
                itemStates[i].AccessRights = (ServerPlugin::DaAccessRights)deviceItems[i].AccessRights;
                itemStates[i].DeviceItemHandle = (IntPtr)deviceItems[i].DeviceItemHandle;
                delete deviceItems[i].ItemName;
            }
            delete[] deviceItems;
        }
    };

    static int FireShutdownRequest(String^ reason)
    {
        char *			pStr = NULL;
        int				requiredSize;
        size_t			size;
        wchar_t *		pwc;

        pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(reason).ToPointer());
        if (pStr) {
            requiredSize = mbstowcs(NULL, pStr, 0);
            /* Add one to leave room for the NULL terminator */
            pwc = (wchar_t *)malloc((requiredSize + 1) * sizeof(wchar_t));
            if (!pwc)
            {
                return S_FALSE;
            }
            size = mbstowcs(pwc, pStr, requiredSize + 1);
            if (size == (size_t)(-1))
            {
                free(pwc);
                return S_FALSE;
            }
            // Always free the unmanaged string.
            Marshal::FreeHGlobal(IntPtr(pStr));

            gpDataServer->FireShutdownRequest(pwc);
#ifdef   _OPC_SRV_AE                                    // Alarms & Events Server
            if (gpEventServer) {
                gpEventServer->FireShutdownRequest(pwc);
            }
#endif
            free(pwc);
        }
        return S_OK;
	};

    static int AddSimpleEventCategory(Int32 categoryId, String^ categoryDescription)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(categoryDescription).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwc)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwc, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwc);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	

			hres =  gpEventServer->AddSimpleEventCategory(categoryId, pwc );
			free(pwc);
		}
		return hres;
#else
		return E_NOTIMPL;
#endif
	}

    static int AddTrackingEventCategory(Int32 categoryId, String^ categoryDescription)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(categoryDescription).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwc)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwc, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwc);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	

			hres = gpEventServer->AddTrackingEventCategory(categoryId, pwc );
			free(pwc);
		}
		return hres;
#else
		return E_NOTIMPL;
#endif
	}

	static int AddConditionEventCategory(Int32 categoryId, String^ categoryDescription)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(categoryDescription).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwc)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwc, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwc);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	

			hres = gpEventServer->AddConditionEventCategory(categoryId, pwc );
			free(pwc);
		}
		return hres;
#else
		return E_NOTIMPL;
#endif
	}

	static int AddEventAttribute(Int32 categoryId, Int32 eventAttribute, String^ attributeDescription, Object^ dataType)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_OK;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc;
		VARIANT			varVal;
		VariantInit( &varVal );

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(attributeDescription).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwc)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwc, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwc);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			Marshal::GetNativeVariantForObject(dataType, IntPtr(&varVal));

			hres = gpEventServer->AddEventAttribute(categoryId, eventAttribute, pwc, varVal.vt );
			free(pwc);
		}

		return hres;
#else
		return E_NOTIMPL;
#endif
	}

	static int AddSingleStateConditionDefinition(Int32 categoryId, Int32 conditionId, String^ name, String^ condition, Int32 severity, String^ description, Boolean ackRequired)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;
		wchar_t *		pwcCondition = NULL;
		wchar_t *		pwcDescription = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(name).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(condition).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcCondition = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcCondition)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcCondition, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				free(pwcCondition);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(description).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcDescription = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcDescription)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcDescription, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				free(pwcCondition);
				free(pwcDescription);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		hres = gpEventServer->AddSingleStateConditionDef(categoryId, conditionId, pwcName, pwcCondition, severity, pwcDescription, ackRequired );
		free(pwcName);
		free(pwcCondition);
		free(pwcDescription);

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AddMultiStateConditionDefinition(Int32 categoryId, Int32 conditionId, String^ name)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(name).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		hres = gpEventServer->AddMultiStateConditionDef(categoryId, conditionId, pwcName );
		free(pwcName);

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AddSubConditionDefinition(Int32 conditionId, Int32 subConditionId, String^ name, String^ condition, Int32 severity, String^ description, Boolean ackRequired)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;
		wchar_t *		pwcCondition = NULL;
		wchar_t *		pwcDescription = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(name).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(condition).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcCondition = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcCondition)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcCondition, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				free(pwcCondition);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(description).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcDescription = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcDescription)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcDescription, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				free(pwcCondition);
				free(pwcDescription);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		hres = gpEventServer->AddSubConditionDef(conditionId, subConditionId, pwcName, pwcCondition, severity, pwcDescription, ackRequired );
		free(pwcName);
		free(pwcCondition);
		free(pwcDescription);

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AddArea(Int32 parentAreaId, Int32 areaId, String^ name)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(name).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		hres = gpEventServer->AddArea(parentAreaId, areaId, pwcName );
		free(pwcName);

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AddSource(Int32 areaId, Int32 sourceId, String^ name, Boolean multiSource)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(name).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		hres = gpEventServer->AddSource(areaId, sourceId, pwcName, multiSource );
		free(pwcName);

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AddExistingSource(Int32 areaId, Int32 sourceId)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;

		hres = gpEventServer->AddExistingSource(areaId, sourceId );

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AddCondition(Int32 sourceId, Int32 conditionDefinitionId, Int32 conditionId)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;

		hres = gpEventServer->AddCondition(sourceId, conditionDefinitionId, conditionId );

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int ProcessSimpleEvent(Int32 categoryId, Int32 sourceId, String^ message, Int32 severity, Int32 attributeCount, array<Object ^> ^ attributeValues, DateTime timeStamp)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;
		_FILETIME 		ftimeStamp;

	   LPVARIANT        attributes;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(message).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		__int64 fileTime = timeStamp.ToFileTime();
		DWORD test = (DWORD)fileTime;
		ftimeStamp.dwLowDateTime = test;
		test = (DWORD)(fileTime >> 32);
		ftimeStamp.dwHighDateTime = test;

		attributes = new VARIANT[attributeCount];

		for (int i=0; i<attributeCount; i++)
		{
			Marshal::GetNativeVariantForObject(attributeValues[i], IntPtr(&attributes[i]));
		}
		hres = gpEventServer->ProcessSimpleEvent(categoryId, sourceId, pwcName, severity, attributeCount, attributes, &ftimeStamp  );
		free(pwcName);

		for (int j = 0; j < attributeCount; j++)
		{
			VariantClear(&attributes[j]);
	}
		if (attributes) {
			delete[] attributes;
		}

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int ProcessTrackingEvent(Int32 categoryId, Int32 sourceId, String^ message, Int32 severity, String^ actorId, Int32 attributeCount, array<Object ^> ^ attributeValues, DateTime timeStamp)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;
		wchar_t *		pwcActorId = NULL;
		_FILETIME 		ftimeStamp;

	   LPVARIANT        attributes;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(message).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(actorId).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcActorId = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcActorId)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcActorId, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				free(pwcActorId);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		__int64 fileTime = timeStamp.ToFileTime();
		DWORD test = (DWORD)fileTime;
		ftimeStamp.dwLowDateTime = test;
		test = (DWORD)(fileTime >> 32);
		ftimeStamp.dwHighDateTime = test;

		attributes = new VARIANT[attributeCount];

		for (int i=0; i<attributeCount; i++)
		{
			Marshal::GetNativeVariantForObject(attributeValues[i], IntPtr(&attributes[i]));
		}
		hres = gpEventServer->ProcessTrackingEvent(categoryId, sourceId, pwcName, severity, pwcActorId, attributeCount, attributes, &ftimeStamp  );
		free(pwcName);
		free(pwcActorId);

		for (int j = 0; j < attributeCount; j++)
		{
			VariantClear(&attributes[j]);
		}
		if (attributes) {
			delete[] attributes;
		}
		
		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int ProcessConditionStateChanges(Int32 count, array<ServerPlugin::AeConditionState ^> ^ conditionStates )
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;
		_FILETIME 		ftimeStamp;
		VARIANT			varVal;
		VariantInit( &varVal );

		for (int i=0; i < count; i++)
		{
			pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi((conditionStates[i])->Message).ToPointer());
			if (pStr) {
				requiredSize = mbstowcs(NULL, pStr, 0); 
				/* Add one to leave room for the NULL terminator */
				pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
				if (pwcName == NULL)
				{
					return S_FALSE;
				}
				size = mbstowcs( pwcName, pStr, requiredSize + 1); 
				if (size == (size_t) (-1))
				{
					free(pwcName);
					return S_FALSE;
				}
				// Always free the unmanaged string.
				Marshal::FreeHGlobal(IntPtr(pStr));	
			}
	
			_variant_t * attributes = new _variant_t[(conditionStates[i])->AttributeCount];

			AeConditionChangeStates cs ( (conditionStates[i])->ConditionId, (conditionStates[i])->SubConditionId, (conditionStates[i])->ActiveState, (conditionStates[i])->Quality.Code, (conditionStates[i])->AttributeCount, attributes );

			for (int j=0; j<(conditionStates[i])->AttributeCount; j++)
			{
				Object ^ attribute = (conditionStates[i])->AttributeValues[j];
				Marshal::GetNativeVariantForObject(attribute, IntPtr(&varVal));
				attributes[j] = varVal;
			}

			DWORD severity = (conditionStates[i])->Severity;
			BOOL ackRequired = (conditionStates[i])->AckRequired;
			cs.AttrValuesPtr() = attributes;
			cs.Message() = pwcName;
			if (severity == 0)
			{
				cs.SeverityPtr() = NULL;
			}
			else
			{
				cs.SeverityPtr() = &severity;
			}
			cs.AckRequiredPtr() = &ackRequired;

			__int64 fileTime = (conditionStates[i])->TimeStamp.ToFileTime();
			DWORD test = (DWORD)fileTime;
			ftimeStamp.dwLowDateTime = test;
			test = (DWORD)(fileTime >> 32);
			ftimeStamp.dwHighDateTime = test;
			cs.TimeStampPtr() = &ftimeStamp;

			hres = gpEventServer->ProcessConditionStateChanges(1, &cs, FALSE);
			for (int j = 0; j < (conditionStates[i])->AttributeCount; j++)
			{
				VariantClear(&attributes[j]);
				attributes[j].Clear();
			}
			if (attributes) {
				delete[] attributes;
			}
			free(pwcName);
		}
		return hres;
#else
		return E_NOTIMPL;
#endif
	};

	static int AckCondition(Int32 conditionId, String^ comment)
	{
#ifdef _OPC_SRV_AE
		char *			pStr = NULL;
		HRESULT			hres = S_FALSE;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwcName = NULL;

		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(comment).ToPointer());
		if (pStr) {
			requiredSize = mbstowcs(NULL, pStr, 0); 
			/* Add one to leave room for the NULL terminator */
			pwcName = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
			if (! pwcName)
			{
				return S_FALSE;
			}
			size = mbstowcs( pwcName, pStr, requiredSize + 1); 
			if (size == (size_t) (-1))
			{
				free(pwcName);
				return S_FALSE;
			}
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
		}

		hres = gpEventServer->AckCondition(conditionId, pwcName  );
		free(pwcName);

		return hres;
#else
		return E_NOTIMPL;
#endif
	};

};



//-----------------------------------------------------------------------------
// Defines classes and enumerators used in the data exchange with the generic 
// server and contains a standard implementation of the methods called by the 
// generic server, e.g. OnCreateServerItems.
//-----------------------------------------------------------------------------
public  ref class GenericServerAPI : public Object
{
	public: static Boolean DelegatesAreCreated;
	public: static ServerPlugin::ClassicNodeManager^ m_drv;
	public: static ServerPlugin::AddItem^ dlgCreateOneItem;

	inline static int GenericServerAPI::OnGetLogLevel()
	{
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		return m_drv->OnGetLogLevel();
	}

	inline static char * GenericServerAPI::OnGetLogPath()
	{
		char *			pStr = NULL;
		String^			sPath;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}

		sPath =  m_drv->OnGetLogPath();
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(sPath).ToPointer());

		return pStr;
	}

	//-------------------------------------------------------------------------
	// This method is called from the generic server at startup for normal 
	// operation or for registration. It provides server registry information 
	// for this application required for DCOM registration. The generic server 
	// registers the OPC server accordingly.
	//-------------------------------------------------------------------------
	inline static ServerRegDefs1* GenericServerAPI::OnGetDaServerDefinition()
	{
		int l=0;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		ServerPlugin::ClassicServerDefinition srvRegDef = m_drv->OnGetDaServerDefinition();
		srvRegDefsDa = new ServerRegDefs1;
		char  * pStr = NULL;

		// Get Application CLSID
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.ClsIdApp).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->ClsidApp,pStr,_mbstrlen(pStr));
			srvRegDefsDa->ClsidApp[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Get CLSID of server
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.ClsIdServer).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->ClsidServer,pStr,_mbstrlen(pStr));
			srvRegDefsDa->ClsidServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Get ProgID of Server 
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.PrgIdServer).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->PrgidServer,pStr,_mbstrlen(pStr));
			srvRegDefsDa->PrgidServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Server ProgID current version
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.PrgIdCurrServer).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->PrgidCurrServer,pStr,_mbstrlen(pStr));
			srvRegDefsDa->PrgidCurrServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Server friendly name
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.ServerName).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->NameServer,pStr,_mbstrlen(pStr));
			srvRegDefsDa->NameServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Server friendly name current version
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.CurrServerName).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->NameCurrServer,pStr,_mbstrlen(pStr));
			srvRegDefsDa->NameCurrServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Company name
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.CompanyName).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsDa->CompanyName,pStr,_mbstrlen(pStr));
			srvRegDefsDa->CompanyName[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		} 
		return srvRegDefsDa;
	};

	//-------------------------------------------------------------------------
	// This method is called from the generic server at startup for normal 
	// operation or for registration. It provides server registry information 
	// for this application required for DCOM registration. The generic server 
	// registers the OPC server accordingly.
	//-------------------------------------------------------------------------
	inline static ServerRegDefs1* GenericServerAPI::OnGetAeServerDefinition()
	{
		int l=0;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		ServerPlugin::ClassicServerDefinition srvRegDef = m_drv->OnGetAeServerDefinition();
		srvRegDefsAe = new ServerRegDefs1;
		char  * pStr = NULL;

		// Get Application CLSID
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.ClsIdApp).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->ClsidApp,pStr,_mbstrlen(pStr));
			srvRegDefsAe->ClsidApp[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Get CLSID of server
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.ClsIdServer).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->ClsidServer,pStr,_mbstrlen(pStr));
			srvRegDefsAe->ClsidServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Get ProgID of Server 
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.PrgIdServer).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->PrgidServer,pStr,_mbstrlen(pStr));
			srvRegDefsAe->PrgidServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Server ProgID current version
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.PrgIdCurrServer).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->PrgidCurrServer,pStr,_mbstrlen(pStr));
			srvRegDefsAe->PrgidCurrServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Server friendly name
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.ServerName).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->NameServer,pStr,_mbstrlen(pStr));
			srvRegDefsAe->NameServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Server friendly name current version
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.CurrServerName).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->NameCurrServer,pStr,_mbstrlen(pStr));
			srvRegDefsAe->NameCurrServer[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		}
		else
		{
			return NULL;
		}

		// Company name
		pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(srvRegDef.CompanyName).ToPointer());
		if (pStr) {
			l = mbstowcs(srvRegDefsAe->CompanyName,pStr,_mbstrlen(pStr));
			srvRegDefsAe->CompanyName[l] = '\0';
			// Always free the unmanaged string.
			Marshal::FreeHGlobal(IntPtr(pStr));	
			pStr = NULL;
		} 
		return srvRegDefsAe;
	};

	inline static void GenericServerAPI::OnCleanup()
	{
		delete srvRegDefsDa;
		delete srvRegDefsAe;
	};

	//-------------------------------------------------------------------------
	// This method is called from the generic server at startup; when the first 
	// client connects or the service is started.
	//
	// It defines the application specific server parameters and operating 
	// modes. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnGetDAServerParameters(int* updatePeriod,  Char* branchDelimiter, int* browseMode)
	{
		int		UpdatePeriod;
		Char	BranchDelimiter;
		ServerPlugin::DaBrowseMode		BrowseMode;
		int result;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		result = m_drv->OnGetDaServerParameters(UpdatePeriod, BranchDelimiter, BrowseMode);
		*updatePeriod = UpdatePeriod;
		*branchDelimiter = BranchDelimiter;
		*browseMode = (int)BrowseMode;
		return result;
	};

	//-------------------------------------------------------------------------
	// This method is called from the generic server at startup; when the first 
	// client connects or the service is started.
	//
	// It defines the application specific server parameters and operating 
	// modes. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnGetDAOptimizationParameters(bool* useOnItemRequest, bool* useOnRefreshItems, bool* useOnAddItems, bool* useOnRemoveItems)
	{
		bool 	UseOnItemRequest;
		bool 	UseOnRefreshItems;
		bool 	UseOnAddItems;
		bool 	UseOnRemoveItems;
		int result;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		result = m_drv->OnGetDaOptimizationParameters(UseOnItemRequest, UseOnRefreshItems, UseOnAddItems, UseOnRemoveItems);
		*useOnItemRequest = UseOnItemRequest;
		*useOnRefreshItems = UseOnRefreshItems;
		*useOnAddItems = UseOnAddItems;
		*useOnRemoveItems = UseOnRemoveItems;
		return result;
	};

	//-------------------------------------------------------------------------
	// This method is called from the generic server at startup. It passes the 
	// callback methods supported by the generic server. These callback methods 
	// can be called anytime to exchange data with the generic server.
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnDefineCallbacks()
	{
		ServerPlugin::AddItem ^ StaticAddItem = gcnew ServerPlugin::AddItem(&GenericServerCallbacks::AddItem);
		ServerPlugin::RemoveItem ^ StaticRemoveItem = gcnew ServerPlugin::RemoveItem(&GenericServerCallbacks::RemoveItem);
		ServerPlugin::AddProperty ^ StaticAddProperty = gcnew ServerPlugin::AddProperty(&GenericServerCallbacks::AddProperty);
		ServerPlugin::SetItemValue ^ StaticSetItemValue = gcnew ServerPlugin::SetItemValue(&GenericServerCallbacks::SetItemValue);
		ServerPlugin::SetServerState ^ StaticSetServerState = gcnew ServerPlugin::SetServerState(&GenericServerCallbacks::SetServerState);
		ServerPlugin::GetActiveItems ^ StaticGetActiveItems = gcnew ServerPlugin::GetActiveItems(&GenericServerCallbacks::GetActiveItems);
        ServerPlugin::GetClients ^ StaticGetClients = gcnew ServerPlugin::GetClients(&GenericServerCallbacks::GetClients);
        ServerPlugin::GetGroups ^ StaticGetGroups = gcnew ServerPlugin::GetGroups(&GenericServerCallbacks::GetGroups);
        ServerPlugin::GetGroupState ^ StaticGetGroupState = gcnew ServerPlugin::GetGroupState(&GenericServerCallbacks::GetGroupState);
        ServerPlugin::GetItemStates ^ StaticGetItemState = gcnew ServerPlugin::GetItemStates(&GenericServerCallbacks::GetItemState);
        ServerPlugin::FireShutdownRequest ^ StaticFireShutdownRequest = gcnew ServerPlugin::FireShutdownRequest(&GenericServerCallbacks::FireShutdownRequest);

		ServerPlugin::AddSimpleEventCategory ^ StaticAddSimpleEventCategory = gcnew ServerPlugin::AddSimpleEventCategory(&GenericServerCallbacks::AddSimpleEventCategory);
		ServerPlugin::AddTrackingEventCategory ^ StaticAddTrackingEventCategory = gcnew ServerPlugin::AddTrackingEventCategory(&GenericServerCallbacks::AddTrackingEventCategory);
		ServerPlugin::AddConditionEventCategory ^ StaticAddConditionEventCategory = gcnew ServerPlugin::AddConditionEventCategory(&GenericServerCallbacks::AddConditionEventCategory);
		ServerPlugin::AddEventAttribute ^ StaticAddEventAttribute = gcnew ServerPlugin::AddEventAttribute(&GenericServerCallbacks::AddEventAttribute);
		ServerPlugin::AddSingleStateConditionDefinition ^ StaticAddSingleStateConditionDefinition = gcnew ServerPlugin::AddSingleStateConditionDefinition(&GenericServerCallbacks::AddSingleStateConditionDefinition);
		ServerPlugin::AddMultiStateConditionDefinition ^ StaticAddMultiStateConditionDefinition = gcnew ServerPlugin::AddMultiStateConditionDefinition(&GenericServerCallbacks::AddMultiStateConditionDefinition);
		ServerPlugin::AddSubConditionDefinition ^ StaticAddSubConditionDefinition = gcnew ServerPlugin::AddSubConditionDefinition(&GenericServerCallbacks::AddSubConditionDefinition);
		ServerPlugin::AddArea ^ StaticAddArea = gcnew ServerPlugin::AddArea(&GenericServerCallbacks::AddArea);
		ServerPlugin::AddSource ^ StaticAddSource = gcnew ServerPlugin::AddSource(&GenericServerCallbacks::AddSource);
		ServerPlugin::AddExistingSource ^ StaticAddExistingSource = gcnew ServerPlugin::AddExistingSource(&GenericServerCallbacks::AddExistingSource);
		ServerPlugin::AddCondition ^ StaticAddCondition = gcnew ServerPlugin::AddCondition(&GenericServerCallbacks::AddCondition);
		ServerPlugin::ProcessSimpleEvent ^ StaticProcessSimpleEvent = gcnew ServerPlugin::ProcessSimpleEvent(&GenericServerCallbacks::ProcessSimpleEvent);
		ServerPlugin::ProcessTrackingEvent ^ StaticProcessTrackingEvent = gcnew ServerPlugin::ProcessTrackingEvent(&GenericServerCallbacks::ProcessTrackingEvent);
		ServerPlugin::ProcessConditionStateChanges ^ StaticProcessConditionStateChanges = gcnew ServerPlugin::ProcessConditionStateChanges(&GenericServerCallbacks::ProcessConditionStateChanges);
		ServerPlugin::AckCondition ^ StaticAckCondition = gcnew ServerPlugin::AckCondition(&GenericServerCallbacks::AckCondition);

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		m_drv->OnDefineDaCallbacks(StaticAddItem, StaticRemoveItem, StaticAddProperty, StaticSetItemValue, StaticSetServerState, StaticGetActiveItems, StaticGetClients, StaticGetGroups, StaticGetGroupState, StaticGetItemState, StaticFireShutdownRequest);

		m_drv->OnDefineAeCallbacks(StaticAddSimpleEventCategory, StaticAddTrackingEventCategory, StaticAddConditionEventCategory, StaticAddEventAttribute, StaticAddSingleStateConditionDefinition, StaticAddMultiStateConditionDefinition, StaticAddSubConditionDefinition, StaticAddArea, StaticAddSource, StaticAddExistingSource, StaticAddCondition, StaticProcessSimpleEvent, StaticProcessTrackingEvent, StaticProcessConditionStateChanges, StaticAckCondition);

		return S_OK;
	};


	//-------------------------------------------------------------------------
	// This method is called from the generic server at the startup; when the 
	// first client connects or the service is started. All items supported by 
	// the server need to be defined by calling the AddItem or AddAnalogItem 
	// callback method for each item. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnCreateServerItems()
	{
		HRESULT hres;
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnCreateServerItems();
		return hres;
	};


	//-------------------------------------------------------------------------
	// Query the properties defined for the specified item 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnQueryProperties(IntPtr deviceItemHandle, int * dwNumProp, int** propertyIDs)
	{
		HRESULT hres;
		int  dwNumCustomProps;
		array<Int32>^ itemIDs;
		int* pdwPropIDs = NULL;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnQueryProperties(deviceItemHandle, dwNumCustomProps, itemIDs);
		*dwNumProp = dwNumCustomProps;

		pdwPropIDs = new int [*dwNumProp];

		for (int i=0; i < *dwNumProp; i++ ) {       
			pdwPropIDs[i] = itemIDs[i];
		}
		*propertyIDs   = pdwPropIDs;
		return hres;
	};


	//-------------------------------------------------------------------------
	// Returns the values of the requested custom properties of the requested 
	// item. This method is not called for the OPC standard properties 1..8. 
	// These are handled in the generic server. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnGetPropertyValue(IntPtr deviceItemHandle, UInt32 propertyID, VARIANT * value)
	{
		HRESULT hres;
		Object^ valueTypes;
		LPDWORD pdwPropIDs = NULL;
		VARIANT	varVal;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnGetPropertyValue(deviceItemHandle, propertyID, valueTypes);
		Marshal::GetNativeVariantForObject(valueTypes, IntPtr(&varVal));
		*value   = varVal;
		VariantClear( &varVal );
		return hres;
	};


	//-------------------------------------------------------------------------
	// This method is called when a client executes a 'write' server call. 
	// The items specified in the DaDeviceItemValue array need to be written 
	// to the device.
	//
	// The cache is updated in the generic server after returning from the 
	// customization WiteItems method. Items with write error are not updated 
	// in the cache.
	//-------------------------------------------------------------------------
	inline static Int32 GenericServerAPI::OnWriteItems(DWORD numHandles, IntPtr * appHandles, OPCITEMVQT * pItemVQTs, HRESULT * errors)

	{
		array<ServerPlugin::DaDeviceItemValue^>^ arrDeviceItemValue = gcnew array<ServerPlugin::DaDeviceItemValue^>(numHandles);
		array<Int32>^ Errors;
		//errors = new HRESULT[numHandles];

		Int32 hResult;

		for (DWORD i=0; i<numHandles; i++) {
			Object^ NewValue = Marshal::GetObjectForNativeVariant(IntPtr(&pItemVQTs[i].vDataValue));

			arrDeviceItemValue[i] = gcnew ServerPlugin::DaDeviceItemValue;
			arrDeviceItemValue[i]->Value = NewValue;
			arrDeviceItemValue[i]->DeviceItemHandle = appHandles[i];
		}


		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hResult =  m_drv->OnWriteItems(arrDeviceItemValue, Errors);
		for (DWORD i=0; i<numHandles; i++) {
			errors[i] = Errors[i];
		}

		return hResult;

	};


	//-------------------------------------------------------------------------
	// Refresh the items listed in the appHandles array in the cache.
	//
	// This method is called when a client executes a read from device. 
	// The device read is called with all client requested items.
	//-------------------------------------------------------------------------
	inline static Int32 GenericServerAPI::OnRefreshItems(DWORD numHandles, IntPtr * appHandles)
	{
		Int32 hResult = S_OK;
		DWORD i;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}

		if (numHandles > 0)
		{
			array<IntPtr>^ arrHandle = gcnew array<IntPtr>(numHandles);

			for (i=0; i<numHandles; i++) {
				arrHandle[i] = appHandles[i];
			}
			hResult =  m_drv->OnRefreshItems(arrHandle);
		}
		else
		{
			array<IntPtr>^ arrHandle;

			hResult =  m_drv->OnRefreshItems(arrHandle);
		}
		return hResult;

	};

	//-------------------------------------------------------------------------
	// The items listed in the appHandles array was added to a group or gets 
	// used for item based read/write.
	//
	// This method is called when a client adds the items to a group or use 
	// item based read/write functions.
	//-------------------------------------------------------------------------
	inline static Int32 GenericServerAPI::OnAddItem(IntPtr appHandle)
	{
		Int32 hResult = S_OK;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}

		if (appHandle != IntPtr::Zero)
		{
			hResult = m_drv->OnAddItem(appHandle);
		}
		else
		{
			hResult = S_FALSE;
		}
		return hResult;

	};

	//-------------------------------------------------------------------------
	// The items listed in the appHandles array are no longer used by clients.
	//
	// This method is called when a client removes items from a group or no 
	// longer use the items in item based read/write functions. 
    // Only items are listed which are no longer used by at least one client
	//-------------------------------------------------------------------------
	inline static Int32 GenericServerAPI::OnRemoveItem(IntPtr appHandle)
	{
		Int32 hResult = S_OK;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}

		if (appHandle != IntPtr::Zero)
		{
			hResult = m_drv->OnRemoveItem(appHandle);
		}
		else
		{
			hResult =  S_FALSE;
		}
		return hResult;

	};

	inline static void GenericServerAPI::OnStartupSignal(LPWSTR pszParam)
	{
		String^	sParam = gcnew String(pszParam);
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		m_drv->OnStartupSignal(sParam);
	};

	//-------------------------------------------------------------------------
	// This method is called from the generic server when a Shutdown is 
	// executed.
	//
	// To ensure proper process shutdown, any communication channels should be 
	// closed and all threads terminated before this method returns.
	//-------------------------------------------------------------------------
	inline static void GenericServerAPI::OnShutdownSignal()
	{
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		m_drv->OnShutdownSignal();
	};

	//-------------------------------------------------------------------------
	// This method is called when the client accesses items that do not yet 
	// exist in the server's cache.
    //
	// OPC DA 2.00 clients typically first call AddItems() or ValidateItems(). 
	// OPC DA 3.00 client may access items directly using the ItemIO read/write 
	// functions. Within this function it is possible to:
	//
	//    - add the item to the servers real address space and return 
	//      OpcResult.S_OK. For each item to be added the callback method 
	//      'AddItem' has to be called.
	//	  - return OpcResult.S_FALSE
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnItemRequest(LPWSTR fullItemID)
	{
		HRESULT hres;
		String^	sParam = gcnew String(fullItemID);
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnItemRequest(sParam);
		return hres;
	};


	//-------------------------------------------------------------------------
	// Custom mode browse handling. Provides a way to move ‘up’ or ‘down’ or 
	// 'to' in a hierarchical space.
	//
	// Called only from the generic server when DaBrowseMode.Custom is 
	// configured. 
	//
	// Change the current browse branch to the specified branch in virtual 
	// address space. This method has to be implemented according the OPC DA 
	// specification. The generic server calls this fuction for OPC DA 2.05a 
	// and OPC DA 3.00 client calls. The differences between the specifications 
	// is handled within the generic server part. Please note that flat address 
	// space is not supported.
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnBrowseChangePosition(int dwBrowseDirection,LPCWSTR szString,  LPWSTR  * szActualPosition)
	{
		HRESULT hres;
		char *			pStr = NULL;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc = NULL;
		WideString		wString;

		String^	sParam = gcnew String(szString);

		String^ str;
		str = gcnew String(*szActualPosition);


		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnBrowseChangePosition((ServerPlugin::DaBrowseDirection)dwBrowseDirection, sParam, str);
		if (SUCCEEDED( hres )) {               
			pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(str).ToPointer());
			if (pStr) {
				requiredSize = mbstowcs(NULL, pStr, 0); 
				/* Add one to leave room for the NULL terminator */
				pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
				if (pwc == NULL)
				{
					return S_FALSE;
				}
				size = mbstowcs( pwc, pStr, requiredSize + 1); 
				if (size == (size_t) (-1))
				{
					free(pwc);
					return S_FALSE;
				}
				// Always free the unmanaged string.
				Marshal::FreeHGlobal(IntPtr(pStr));	
				hres = wString.SetString( pwc );
				*szActualPosition = SysAllocString(pwc);
				free(pwc);
			}
		}
		return hres;
	};


	//-------------------------------------------------------------------------
	// Custom mode browse handling.
	//
	// Called only from the generic server when DaBrowseMode.Custom is 
	// configured. 
	//
	// This method returns the fully qualified name of the specified item in the 
	// current branch in the virtual address space. This name is used to add the 
	// item to the real address space. The generic server calls this fuction for 
	// OPC DA 2.05a and OPC DA 3.00 client calls. The differences between the 
	// specifications is handled within the generic server part. 
	// Please note that flat address space is not supported.
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnBrowseGetFullItemId(LPCWSTR szActualPosition, LPCWSTR szItemDataID, LPWSTR  * szItemID)
	{
		HRESULT hres;
		char *			pStr = NULL;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc = NULL;
		WideString		wString;

		String^	sItemDataID = gcnew String(szItemDataID);

		String^ sActualPosition = gcnew String(szActualPosition);

		String^	str;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnBrowseGetFullItemId(sActualPosition, sItemDataID, str);
		if (SUCCEEDED( hres )) {               
			pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(str).ToPointer());
			if (pStr) {
				requiredSize = mbstowcs(NULL, pStr, 0); 
				/* Add one to leave room for the NULL terminator */
				pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
				if (pwc == NULL)
				{
					return S_FALSE;
				}
				size = mbstowcs( pwc, pStr, requiredSize + 1); 
				if (size == (size_t) (-1))
				{
					free(pwc);
					return S_FALSE;
				}
				// Always free the unmanaged string.
				Marshal::FreeHGlobal(IntPtr(pStr));	
				hres = wString.SetString( pwc );
				*szItemID = SysAllocString(pwc);
				free(pwc);
			}
		}
		return hres;
	};


	//-------------------------------------------------------------------------
	// Custom mode browse handling.
	//
	// Called only from the generic server when DaBrowseMode.Custom is 
	// configured. 
	//
	// This method browses the items in the current branch of the virtual 
	// address space. The position from the which the browse is done can be set 
	// via OnBrowseChangePosition. The generic server calls this fuction for 
	// OPC DA 2.05a and OPC DA 3.00 client calls. The differences between the 
	// specifications is handled within the generic server part. 
	// Please note that flat address space is not supported. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnBrowseItemIds(
		/*[in          */          LPWSTR              szActualPosition,
		/*[in]         */          tagOPCBROWSETYPE    dwBrowseFilterType,
		/*[in, string] */          LPWSTR              szFilterCriteria,
		/*[in]         */          VARTYPE             vtDataTypeFilter,
		/*[in]         */          DWORD               dwAccessRightsFilter,
		/*[out]        */          DWORD             * pNrItemIDs,
		/*[out]        */          LPWSTR           ** ppItemIDs )

	{
		HRESULT hres;
		char *			pStr = NULL;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc = NULL;
		WideString		wString;
		int				NrItemIDs;
		array<String^>^ itemIDs;

		String^	sFilterCriteria = gcnew String(szFilterCriteria);

		String^ sActualPosition = gcnew String(szActualPosition);

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnBrowseItemIds(	sActualPosition, (ServerPlugin::DaBrowseType)dwBrowseFilterType, sFilterCriteria,
			vtDataTypeFilter.GetType(), (ServerPlugin::DaAccessRights)dwAccessRightsFilter, NrItemIDs, itemIDs);
		if (SUCCEEDED( hres )) {       
			*ppItemIDs = new LPWSTR[ NrItemIDs ] ;

			for(int i=0;i<NrItemIDs;i++)
			{
				pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(itemIDs[i]).ToPointer());
				if (pStr) {
					requiredSize = mbstowcs(NULL, pStr, 0); 
					/* Add one to leave room for the NULL terminator */
					pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
					if (pwc == NULL)
					{
						return S_FALSE;
					}
					size = mbstowcs( pwc, pStr, requiredSize + 1); 
					if (size == (size_t) (-1))
					{
						free(pwc);
						return S_FALSE;
					}
					// Always free the unmanaged string.
					Marshal::FreeHGlobal(IntPtr(pStr));	
					hres = wString.SetString( pwc );
					(*ppItemIDs)[i] = SysAllocString(pwc);
					free(pwc);
				}
			}
			*pNrItemIDs = NrItemIDs;
		}
		return hres;
	};


	//-------------------------------------------------------------------------
	// This method is called when a client connects to the OPC server. 
	// If the method returns an error code then the client connect is refused. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnClientConnect()
	{
		HRESULT hres;
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnClientConnect();
		return hres;
	};


	//-------------------------------------------------------------------------
	// This method is called when a client disconnects from the OPC server. 
	//-------------------------------------------------------------------------
	inline static int GenericServerAPI::OnClientDisconnect()
	{
		HRESULT hres;
		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnClientDisconnect();
		return hres;
	};

	inline static int GenericServerAPI::OnAckNotification(DWORD conditionId,  DWORD subConditionId)
	{
		int		condId = conditionId;
		int		subCondId = subConditionId;

		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		return m_drv->OnAckNotification(condId, subCondId);
	};


	inline static int GenericServerAPI::OnTranslateToItemId(DWORD dwCondID, DWORD dwSubCondDefID, DWORD dwAttrID,
                              LPWSTR* pszItemID, LPWSTR* pszNodeName, CLSID* pCLSID)
	{
		HRESULT hres;
		char *			pStr = NULL;
		int				requiredSize;
		size_t			size;
		wchar_t *		pwc = NULL;
		WideString		wString;

		String^	itemId;
		String^	nodeName;
		String^	clsid;


		if (!m_drv)
		{
			m_drv = gcnew ServerPlugin::ClassicNodeManager();
		}
		hres = m_drv->OnTranslateToItemId(dwCondID, dwSubCondDefID, dwAttrID, itemId, nodeName, clsid);
		if (SUCCEEDED( hres )) {               
			pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(itemId).ToPointer());
			if (pStr) {
				requiredSize = mbstowcs(NULL, pStr, 0); 
				/* Add one to leave room for the NULL terminator */
				pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
				if (pwc == NULL)
				{
					return S_FALSE;
				}
				size = mbstowcs( pwc, pStr, requiredSize + 1); 
				if (size == (size_t) (-1))
				{
					free(pwc);
					return S_FALSE;
				}
				// Always free the unmanaged string.
				Marshal::FreeHGlobal(IntPtr(pStr));	
				hres = wString.SetString( pwc );
				*pszItemID = SysAllocString(pwc);
				free(pwc);
			}
			pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(nodeName).ToPointer());
			if (pStr) {
				requiredSize = mbstowcs(NULL, pStr, 0); 
				/* Add one to leave room for the NULL terminator */
				pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
				if (pwc == NULL)
				{
					return S_FALSE;
				}
				size = mbstowcs( pwc, pStr, requiredSize + 1); 
				if (size == (size_t) (-1))
				{
					free(pwc);
					return S_FALSE;
				}
				// Always free the unmanaged string.
				Marshal::FreeHGlobal(IntPtr(pStr));	
				hres = wString.SetString( pwc );
				*pszNodeName = SysAllocString(pwc);
				free(pwc);
			}
			pStr = static_cast<char*>(Marshal::StringToHGlobalAnsi(clsid).ToPointer());
			if (pStr) {
				requiredSize = mbstowcs(NULL, pStr, 0); 
				/* Add one to leave room for the NULL terminator */
				pwc = (wchar_t *)malloc( (requiredSize + 1) * sizeof( wchar_t ));
				if (pwc == NULL)
				{
					return S_FALSE;
				}
				size = mbstowcs( pwc, pStr, requiredSize + 1); 
				if (size == (size_t) (-1))
				{
					free(pwc);
					return S_FALSE;
				}
				// Always free the unmanaged string.
				Marshal::FreeHGlobal(IntPtr(pStr));	
				hres = wString.SetString( pwc );
				hres = CLSIDFromString( pwc, pCLSID );
				free(pwc);
			}
		}
		return hres;

	};
};



