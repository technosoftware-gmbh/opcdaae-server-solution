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

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server

//-------------------------------------------------------------------------
// INLCUDE
//-------------------------------------------------------------------------
#include "stdafx.h"                             // Generic server part headers
#include "UtilityFuncs.h"                       // for WSTRClone()
#include "MatchPattern.h"                       // for default filtering
// Application specific definitions
#include "ServerMain.h"
#ifdef _OPC_NET
#include "app_netserverwrapper.h"
#endif

#include "Logger.h"

using namespace IClassicBaseNodeManager;

//-------------------------------------------------------------------------
// CODE
//-------------------------------------------------------------------------
extern LPWSTR 	 vendor_name;

//=========================================================================
// Construction
//=========================================================================
AeServer::AeServer()
{
	m_dwServerState = OPCAE_STATUS_FAILED;
}



//=========================================================================
// Initializer
// -----------
//    Initzialization of the Event Server Instance.
//    This function initializes the Event Server Class Handler and
//    creates the Event and Process Area Space. If all succeeded then the
//    server state is set to OPCAE_STATUS_RUNNING.
//    
//    In this combined DA/AE Server only the Event Categories, the
//    Event Attributes, the Process Areas and Event Sources which are not 
//    DA Items are specified within this initializer function.
//    The Event Sources which are DA Items, Condition Definitions and
//    Conditions are specified after creation of the DA Server Address
//    Space in DaServer::Create().
//=========================================================================
HRESULT AeServer::Create()
{
	EventArea::SetDelimiter( L'\\' );           // Do not use the same delimiter character
	// like the DA Server Address Space.

	// First the base Server Class Handler must be initialized.
	HRESULT hres = AeBaseServer::Create();
	if (FAILED( hres )) return hres;

	// Order of implemented steps

	// 1) Define the Event Categories
	// 2) Add the Attributes to the Event Categories
	// 3) Define the Process Areas
	// 4) Define the Event Sources which are not DA Items

	try {

		// 1) Define the Event Categories
		/////////////////////////////////
		//CHECK_RESULT( AddSimpleEventCategory(   CATID_SYSMESSAGE, L"System Message" ) )
		//CHECK_RESULT( AddSimpleEventCategory(   CATID_USERMESSAGE,L"User Message" ) )
		//CHECK_RESULT( AddTrackingEventCategory( CATID_PCHANGE,    L"Operator Process Change" ) )
		//CHECK_RESULT( AddConditionEventCategory(CATID_LEVEL,      L"Level" ) )


		// 2) Add the Attributes to the Event Categories
		////////////////////////////////////////////////
		//CHECK_RESULT( AddEventAttribute( CATID_LEVEL,   ATTRID_LEVEL_CV,           L"Current Value",   VT_I2 ) )
		//CHECK_RESULT( AddEventAttribute( CATID_PCHANGE, ATTRID_PCHANGE_PREVVALUE,  L"Prev Value",      VT_I2 ) )
		//CHECK_RESULT( AddEventAttribute( CATID_PCHANGE, ATTRID_PCHANGE_NEWVALUE,   L"New Value",       VT_I2 ) )


		// 3) Define the Process Areas
		//////////////////////////////
		//CHECK_RESULT( AddArea( AREAID_ROOT, AREAID_RAMPS,  L"Ramps" ) )
		//CHECK_RESULT( AddArea( AREAID_ROOT, AREAID_LIMITS, L"Limits" ) )


		// 4) Define the Event Sources which are not DA Items
		/////////////////////////////////////////////////////
		//CHECK_RESULT( AddSource( AREAID_ROOT, SRCID_SYSTEM, L"System" ) )

		// Event and process Area Space created successfully.
		m_dwServerState = OPCAE_STATUS_RUNNING;   // All succeeded.
	}
	catch( HRESULT hresEx ) {
		hres = hresEx;
	}           
	return hres;
}



//=========================================================================
// Destructor
//=========================================================================
AeServer::~AeServer()
{
	m_dwServerState = OPCAE_STATUS_SUSPENDED;
}

HRESULT AeServer::OnConnectClient(  )
{
	HRESULT  hres = S_OK;
#ifdef _OPC_NET
	hres = GenericServerAPI::OnClientConnect();
#endif

#ifdef _OPC_DLL
	if (pOnClientConnect != NULL)
	{
		hres = (*pOnClientConnect)();
	}
#endif

	LOGFMTT("OnClientConnect() finished with hres = 0x%x.", hres);
	return hres;
}

HRESULT AeServer::OnDisconnectClient(  )
{
	HRESULT  hres = S_OK;
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


//=========================================================================
// OnTranslateToItemIdentifier
// ---------------------------
//    Returns the info of the Data Access Item associated with the
//    attribute which is specified by the parameters.
//    If there is no associated Item then the result parameters must be
//    initialized with NULL strings.
//
//    Attention:  Memory for the strings must be allocated from the
//                Global COM Memoyr Manager.
//
// Parameters:
//    IN
//       dwCondID                ID of the condition
//       dwSubCondDefIDia        ID of the sub condition definition.
//                               It's 0 for single state conditions.
//       dwAttrID                ID of the attribute
//    OUT
//       pszItemID               ItemId of associated Data Access Item
//                               or NULL string if there is no associated
//                               OPC Item.
//       pszNodeName             Network node name of the associated OPC
//                               Data Access Server or NULL string if the
//                               server is running on the local node.
//       pCLSID                  CLSID of the associated Data Access
//                               Server or CLSID_NULL if there is no
//                               associated Item.
//=========================================================================
HRESULT AeServer::OnTranslateToItemIdentifier(
	DWORD dwCondID, DWORD dwSubCondDefID, DWORD dwAttrID, 
	LPWSTR* pszItemID, LPWSTR* pszNodeName, CLSID* pCLSID )
{
	HRESULT  hres = S_OK;
#ifdef _OPC_NET
	hres = GenericServerAPI::OnTranslateToItemId(dwCondID, dwSubCondDefID, dwAttrID, pszItemID, pszNodeName, pCLSID);
#endif

#ifdef _OPC_DLL
	hres = (*pOnTranslateToItemId)(dwCondID, dwSubCondDefID, dwAttrID, pszItemID, pszNodeName, pCLSID);
#endif

#ifdef TEST
	BOOL     fItemExist = TRUE;
	*pszItemID     = NULL;                       // Initialize result parameters
	*pszNodeName   = NULL;
	*pCLSID        = CLSID_NULL;

	fItemExist = FALSE;

	//
	// TODO:          Set the info for the specified attribute if available as 
	//                OPC Data Access Item.
	// SampleServer:  Only attribute 'Current Value' of condition category 'Level'
	//                is available as OPC Data Access Item.
	//

	try {
		if (dwAttrID == ATTRID_LEVEL_CV) {        // Only 'Current Value' attribute supported
			// The condition instanmce is used as Condition Identifier.
			DeviceItem::LPCONDITION pCond = (DeviceItem::LPCONDITION)dwCondID;
			// On Local Machine
			CHECK_PTR( *pszNodeName = WSTRClone( L"", pIMalloc ) )   
				CHECK_RESULT( pCond->m_pOwner->TranslateAttrToItemId( dwSubCondDefID, dwAttrID, pszItemID ) )
				CHECK_RESULT( CLSIDFromString( gpMyApp->DataServerCLSID(), pCLSID ) )
				fItemExist = TRUE;
		}

		if (!fItemExist) {
			CHECK_PTR( *pszItemID   = WSTRClone( L"", pIMalloc ) )
				CHECK_PTR( *pszNodeName = WSTRClone( L"", pIMalloc ) )
		}
		hres = S_OK;                              // All succeeded
	}
	catch (HRESULT hresEx) {
		hres = hresEx;
	}
	catch (...) {
		hres = E_FAIL;
	}

	if (FAILED( hres )) {                        // Cleanup all if not all succeeded
		if (*pszItemID) {
			WSTRFree( *pszItemID, pIMalloc );
			*pszItemID = NULL;
		}
		if (*pszNodeName) {
			WSTRFree( *pszNodeName, pIMalloc );
			*pszNodeName = NULL;
		}
		*pCLSID = CLSID_NULL;
	}
#endif
	LOGFMTT("OnTranslateToItemId() finished with hres = 0x%x.", hres);
	return hres;
}



//=========================================================================
// OnAcknowledgeNotification
// -------------------------
//    Notification if the condition specified by the parameters is
//    acknowledged. This function is called from the generic server part
//    if the condition is successfully acknowledged but before the
//    indication events are sent to the clients.
//    If this function fails then the error code will be returned to the
//    client and no indication events will be generated.
//
// Parameters:
//    dwCondID                   ID of the condition
//    dwSubCondDefIDia           ID of the sub condition definition.
//                               It's 0 for single state conditions.
// Return:
//    S_OK                       All succeeded
//    E_xxx                      failure code, no indication events will
//                               be generated.
//=========================================================================
HRESULT AeServer::OnAcknowledgeNotification(DWORD dwCondID, DWORD dwSubConDefID)
{
	HRESULT  hres = S_OK;

	//
	// TODO:          Add server specific acknowledgement (e.g. reset HW signal).
	// SampleServer:  There is nothing to acknowledge internally.
	//
#ifdef _OPC_NET
	hres = GenericServerAPI::OnAckNotification(dwCondID, dwSubConDefID);
#endif

#ifdef _OPC_DLL
	hres = (*pOnAckNotification)(dwCondID, dwSubConDefID);
#endif

	LOGFMTT("OnAckNotification() finished with hres = 0x%x.", hres);
	return hres;

}



//=========================================================================
// OnGetServerState
// ----------------
//    Returns the current status of the event server to handle the status
//    information request function.
//=========================================================================
HRESULT AeServer::OnGetServerState(	/*[out]*/   OPCEVENTSERVERSTATE & serverState,
										 /*[out]*/   LPWSTR				& vendor )
{
	serverState = m_dwServerState;
	vendor		= vendor_name;
	LOGFMTT("OnGetServerState() finished with hres = 0x%x.", S_OK);
	return S_OK;
}



//=========================================================================
// FilterName
// ----------
//    Filters an Process Area or Event Source name with the specified
//    filter. This function is called if the client browse the Process
//    Area Space with the function IEventAreaBrowser::BrowseOPCAreas().
//
// Parameters:
//    szName                     The Process Area or Event Source name
//    szFilterCriteria           A filter string.
// Return:
//    If szName matches szFilterCriteria, return TRUE; if there is
//    no match, return is FALSE. If either szName or szFilterCriteria is
//    a NULL pointer, return is FALSE.
//=========================================================================
BOOL EventArea::FilterName( LPCWSTR szName, LPCWSTR szFilterCriteria )
{
	//
	// TODO: Add server specific filtering if desired
	//

	// Call ::MatchPattern() to support default filtering
	// specified by the AE specification.
	return ::MatchPattern( szName, szFilterCriteria );
}

#endif