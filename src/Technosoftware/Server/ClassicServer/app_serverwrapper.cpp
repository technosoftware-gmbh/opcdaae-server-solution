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

#include "StdAfx.h"
#include "DaServer.h"
#include "DaItemProperty.h"
#include "Logger.h"
#ifdef _OPC_SRV_AE
#include "AeServer.h"
#endif

static CSimplePtrArray<WideString*> m_apPropDescr;

namespace IClassicBaseNodeManager
{
	//=========================================================================
	// AddItem
	// -------
	//    Creates one item in the server data structure.
	//    This function is called from the user DLL for each item created.
	//=========================================================================
	HRESULT DLLCALL AddItem(
		LPWSTR				szItemID,
		DaAccessRights		dwAccessRights,
		LPVARIANT			pvValue,
		bool				fActive,
		DaEuType			eEUType,
		double				minValue,
		double				maxValue,
		void**				deviceItemHandle
		)
	{
		HRESULT           hres = E_FAIL;

		LOGFMTT("AddItem() called from plugin.");

		// Initialize new Item instance
		if ((OPCEUTYPE)eEUType == OPC_NOENUM)
		{
			hres = CreateOneItem(
				szItemID,						// ItemId
				(DWORD)dwAccessRights,			// DaAccessRights
				pvValue,					 	// Initial Value
				fActive,						// Active State
				OPC_NOENUM,						// EU Type
				NULL,							// EU Info
				deviceItemHandle
				);
		}
		else
		{
			if ((OPCEUTYPE)eEUType == OPC_ANALOG)
			{
				// EU Info must contain exaclty two doubles 
				// corresponding to the LOW and HI EU range.
				VARIANT  varEUInfo;
				VariantInit(&varEUInfo);
				V_VT(&varEUInfo) = VT_ARRAY | VT_R8;

				SAFEARRAYBOUND rgs;
				rgs.cElements = 2;
				rgs.lLbound = 0;

				try {
					CHECK_PTR(V_ARRAY(&varEUInfo) = SafeArrayCreate(VT_R8, 1, &rgs))
					// put initialized variant into array
					double   dLow = minValue;
					double   dHi = maxValue;
					long     lElIndex;

					lElIndex = 0;                       // LOW EU range
					CHECK_RESULT(SafeArrayPutElement(V_ARRAY(&varEUInfo), &lElIndex, &dLow))
					lElIndex++;                         // HI  EU range
					CHECK_RESULT(SafeArrayPutElement(V_ARRAY(&varEUInfo), &lElIndex, &dHi))
				}
				catch (HRESULT hresEx) {				// cleanup if failure
					VariantClear(&varEUInfo);
					LOGFMTE("AddItem() failed with hres = 0x%x.", hresEx);
					throw hresEx;
				}
				hres = CreateOneItem(
					szItemID,							// ItemId
					(DWORD)dwAccessRights,				// DaAccessRights
					pvValue,							// Initial Value
					fActive,							// Active State
					OPC_ANALOG,							// EU Type
					&varEUInfo,							// EU Info
					deviceItemHandle
					);
				VariantClear(&varEUInfo);

			}
		}
		VariantClear(pvValue);
		LOGFMTT("AddItem() finished with hres = 0x%x.", hres);
		return hres;
	}

	HRESULT DLLCALL RemoveItem(void* deviceItem)
	{
		HRESULT			hres = E_FAIL;

		LOGFMTT("RemoveItem() called from plugin.");
		if (deviceItem == NULL) {
			LOGFMTE("RemoveItem() failed with hres = 0x%x (deviceItem is NULL).", S_FALSE);
			return(S_FALSE);
		}
		hres = gpDataServer->RemoveDeviceItem((DeviceItem*)deviceItem);

		LOGFMTT("RemoveItem() finished with hres = 0x%x.", hres);
		return hres;
	};

	HRESULT DLLCALL AddProperty(int propertyID, LPWSTR description, LPVARIANT valueType)
	{
		char *			pStr = NULL;
		HRESULT			hres;
		DaItemProperty*	pProp;

		LOGFMTT("AddProperty() called from plugin.");
		hres = gpDataServer->GetItemProperty(propertyID, &pProp);
		if (FAILED(hres)) {

			WideString* p1 = new WideString;
			hres = p1->SetString(description);
			if (FAILED(hres)) {
				LOGFMTE("AddProperty() failed with hres = 0x%x.", hres);
				return hres;
			}

			if (m_apPropDescr.Add(p1))
			{
				pProp = new  DaItemProperty(propertyID, V_VT(valueType), (LPWSTR)*p1);
				hres = gpDataServer->AddItemProperty(pProp);
			}
			else
			{
				LOGFMTE("AddProperty() failed with hres = 0x%x.", S_FALSE);
				return S_FALSE;
			}
		}

		LOGFMTT("AddProperty() finished with hres = 0x%x.", hres);
		return hres;
	};

	HRESULT DLLCALL SetItemValue(
		void*			deviceItem,
		LPVARIANT		newValue,
		short   		quality,
		FILETIME		timestamp
		)
	{
		HRESULT			hres = S_OK;
		DeviceItem*  pItem = (DeviceItem*)deviceItem;

		LOGFMTT("SetItemValue() called from plugin.");
		gpDataServer->readWriteLock_.BeginWriting();	// We are manipulating the DeviceItem make
		// sure no one else gets access during update.
		if (pItem == NULL) {
			gpDataServer->readWriteLock_.EndWriting(); // UnLock DeviceItem
			hres = S_FALSE;
			LOGFMTE("AddProperty() failed with hres = 0x%x (deviceItem not found).", hres);
			return(hres);
		}

		if (newValue == NULL || V_VT(newValue) == VT_NULL) {
			// Update only quality and time stamp
			hres = pItem->set_ItemQuality(quality, &timestamp);
		}
		else {
			hres = pItem->set_ItemValue(newValue, quality, &timestamp);
		}
		gpDataServer->readWriteLock_.EndWriting();     // UnLock DeviceItem

		LOGFMTT("SetItemValue() finished with hres = 0x%x.", hres);
		return hres;
	}

	void DLLCALL SetServerState(ServerState serverState)
	{
		LOGFMTT("SetServerState() called from plugin.");
	
		gpDataServer->SetServerState((OPCSERVERSTATE)serverState);

		LOGFMTT("SetServerState() finished.");
	};

	void DLLCALL GetActiveItems(int * dwNumItemHandles, void* **pItemHandles)
	{
		DWORD				dwSize;
		DeviceItem*			pItem;
		int					numHandles = 0;

		LOGFMTT("GetActiveItems() called from plugin.");
		gpDataServer->m_ItemListLock.BeginReading();			// Protect item list access
		// Handle all items
		dwSize = (DWORD)gpDataServer->m_arServerItems.GetSize();
		void** activeItemHandles;

		activeItemHandles = new void*[dwSize];
		for (DWORD i = 0; i < dwSize; i++) {
			pItem = gpDataServer->m_arServerItems[i];
			if (pItem == NULL) {
				continue;										// Handle next item
			}

			if (pItem->get_ActiveCount() > 0) {
				activeItemHandles[numHandles] = pItem;
				numHandles++;
			}
		} // handle all items
		gpDataServer->m_ItemListLock.EndReading();				// Release item list protection

		*dwNumItemHandles = numHandles;
		*pItemHandles = new void*[numHandles];
		for (int i = 0; i < numHandles; i++) {
			*pItemHandles[i] = (void*)activeItemHandles[i];
		}
		delete activeItemHandles;

		LOGFMTT("GetActiveItems() finished.");
	};

	HRESULT DLLCALL AddSimpleEventCategory(int categoryID, LPWSTR categoryDescription)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddSimpleEventCategory() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres =  gpEventServer->AddSimpleEventCategory(categoryID, categoryDescription);

		LOGFMTT("AddSimpleEventCategory() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddSimpleEventCategory() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT DLLCALL AddTrackingEventCategory(int categoryID, LPWSTR categoryDescription)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddTrackingEventCategory() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddTrackingEventCategory(categoryID, categoryDescription);

		LOGFMTT("AddTrackingEventCategory() finished with hres = 0x%x.", hres);

		return hres;
	#else
		LOGFMTE("AddTrackingEventCategory() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT DLLCALL AddConditionEventCategory(int categoryID, LPWSTR categoryDescription)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddConditionEventCategory() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres =  gpEventServer->AddConditionEventCategory(categoryID, categoryDescription);

		LOGFMTT("AddConditionEventCategory() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddConditionEventCategory() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT DLLCALL AddEventAttribute(int categoryID, int eventAttribute, LPWSTR attributeDescription, VARTYPE dataType)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddEventAttribute() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddEventAttribute(categoryID, eventAttribute, attributeDescription, dataType);

		LOGFMTT("AddEventAttribute() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddEventAttribute() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddSingleStateConditionDefinition(int categoryId, int conditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddSingleStateConditionDefinition() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres =  gpEventServer->AddSingleStateConditionDef(categoryId, conditionId, name, condition, severity, description, ackRequired);

		LOGFMTT("AddSingleStateConditionDefinition() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddSingleStateConditionDefinition() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddMultiStateConditionDefinition(int categoryId, int conditionId, LPWSTR name)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddMultiStateConditionDefinition() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddMultiStateConditionDef(categoryId, conditionId, name);

		LOGFMTT("AddMultiStateConditionDefinition() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddMultiStateConditionDefinition() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddSubConditionDefinition(int conditionId, int subConditionId, LPWSTR name, LPWSTR condition, int severity, LPWSTR description, bool ackRequired)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddSubConditionDefinition() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres =  gpEventServer->AddSubConditionDef(conditionId, subConditionId, name, condition, severity, description, ackRequired);

		LOGFMTT("AddSubConditionDefinition() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddSubConditionDefinition() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddArea(int parentAreaId, int areaId, LPWSTR name)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddArea() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddArea(parentAreaId, areaId, name);

		LOGFMTT("AddArea() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddArea() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddSource(int areaId, int sourceId, LPWSTR sourceName, bool multiSource)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddSource() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddSource(areaId, sourceId, sourceName, multiSource);

		LOGFMTT("AddSource() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddSource() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddExistingSource(int areaId, int sourceId)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddExistingSource() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddExistingSource(areaId, sourceId);

		LOGFMTT("AddExistingSource() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddExistingSource() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AddCondition(int sourceId, int conditionDefinitionId, int conditionId)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("AddCondition() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AddCondition(sourceId, conditionDefinitionId, conditionId);

		LOGFMTT("AddCondition() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("AddCondition() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL ProcessSimpleEvent(int categoryId, int sourceId, LPWSTR message, int severity, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("ProcessSimpleEvent() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->ProcessSimpleEvent(categoryId, sourceId, message, severity, attributeCount, attributeValues, timeStamp);

		LOGFMTT("ProcessSimpleEvent() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("ProcessSimpleEvent() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL ProcessTrackingEvent(int categoryId, int sourceId, LPWSTR message, int severity, LPWSTR actorId, int attributeCount, LPVARIANT attributeValues, LPFILETIME timeStamp)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("ProcessTrackingEvent() called from plugin.");

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->ProcessTrackingEvent(categoryId, sourceId, message, severity, actorId, attributeCount, attributeValues, timeStamp);

		LOGFMTT("ProcessTrackingEvent() finished with hres = 0x%x.", hres);

		return hres;

	#else
		LOGFMTE("ProcessTrackingEvent() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL ProcessConditionStateChanges(int count, AeConditionState* conditionStateChanges)
	{
		HRESULT			hres = S_OK;

		LOGFMTT("ProcessConditionStateChanges() called from plugin.");

	#ifdef _OPC_SRV_AE
		HRESULT hResult = S_OK;
		AeConditionChangeStates* cs = new AeConditionChangeStates[count];

		for (int i = 0; i < count; i++)
		{
			cs[i] = AeConditionChangeStates((conditionStateChanges[i]).CondID(), (conditionStateChanges[i]).SubCondID(), (conditionStateChanges[i]).ActiveState(), (conditionStateChanges[i]).Quality(), (conditionStateChanges[i]).AttrCount(), (conditionStateChanges[i]).AttrValuesPtr());
		}

		hResult = gpEventServer->ProcessConditionStateChanges(count, cs, FALSE);
		delete cs;

		LOGFMTT("ProcessConditionStateChanges() finished with hres = 0x%x.", hres);
		return hResult;
	#else
		LOGFMTE("ProcessConditionStateChanges() not implemented.");
		return E_NOTIMPL;
	#endif
	}

	HRESULT  DLLCALL AckCondition(int conditionId, LPWSTR comment)
	{
		HRESULT hres = S_OK;

	#ifdef _OPC_SRV_AE
		hres = gpEventServer->AckCondition(conditionId, comment);

		LOGFMTT("AckCondition() finished with hres = 0x%x.", hres);

		return hres;
	#else
		LOGFMTE("AddEventAttribute() not implemented.");
		return E_NOTIMPL;
	#endif
	}


	void DLLCALL GetClients(int * numClientHandles, void* ** clientHandles, LPWSTR ** clientNames)
	{
		LOGFMTT("GetClients() called from plugin.");

		gpDataServer->GetClients(numClientHandles, clientHandles, clientNames);

		LOGFMTT("GetClients() finished.");
	}

	void DLLCALL GetGroups(void * clientHandle, int * numGroupHandles, void* ** groupHandles, LPWSTR ** groupNames)
	{
		LOGFMTT("GetGroups() called from plugin.");

		gpDataServer->GetGroups(clientHandle, numGroupHandles, groupHandles, groupNames);

		LOGFMTT("GetGroups() finished.");
	}

	void DLLCALL GetGroupState(void * groupHandle, DaGroupState * groupState)
	{
		LOGFMTT("GetGroupState() called from plugin.");

		gpDataServer->GetGroupState(groupHandle, groupState);

		LOGFMTT("GetGroupState() finished.");
	}

	void DLLCALL GetItemStates(void * groupHandle, int * numDaItemStates, DaItemState* * daItemStates)
	{
		LOGFMTT("GetItemStates() called from plugin.");

		gpDataServer->GetItemStates(groupHandle, numDaItemStates, daItemStates);

		LOGFMTT("GetItemStates() finished.");
	}

	void DLLCALL FireShutdownRequest(LPCWSTR reason)
	{
		LOGFMTT("FireShutdownRequest() called from plugin.");

		gpDataServer->FireShutdownRequest(reason);
		LOGFMTT("FireShutdownRequest() for DA finished.");
	#ifdef _OPC_SRV_AE
		gpEventServer->FireShutdownRequest(reason);
		LOGFMTT("GetItemStates() for AE finished.");
	#endif
	}

}