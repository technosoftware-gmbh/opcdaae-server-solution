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
#include "CoreMain.h"

												// Application specific definitions
#ifdef NDEBUG
#include <stdio.h>
#endif
#include "DaServer.h"
#include "Logger.h"

//=============================================================================
// CreateOneItem
// -------------
//    Creates one item in the server data structure.
//    This function is called from the user DLL for each item created.
//=============================================================================
HRESULT		CreateOneItem(	LPWSTR						szItemID,
						  DWORD						dwAccessRights,
						  LPVARIANT					pvValue,
						  BOOL						fActive,
						  OPCEUTYPE					eEUType,
							LPVARIANT					pvEUInfo,
						  void**					deviceItem
)

{
	HRESULT           hres;
	DeviceItem*    pItem;

	pItem = new DeviceItem;                   // Allocate space for new server item.
	if (pItem == NULL) {
		hres = E_OUTOFMEMORY;
	}
	else {                                       // Initialize new Item instance
		hres = pItem->Create(szItemID, dwAccessRights, pvValue, fActive, 0, NULL, NULL, eEUType, pvEUInfo);
		if (SUCCEEDED(hres)) {                  // Add new created item to the item list
			hres = gpDataServer->AddDeviceItem(pItem);
		}
	}
	*deviceItem = (void*)pItem;
	VariantClear(pvValue);
	USES_CONVERSION;
	LOGFMTT("CreateOneItem() for item %s finished with hres = 0x%x.", W2A(szItemID), hres);
	return hres;
}

