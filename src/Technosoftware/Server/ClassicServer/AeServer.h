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
                                                                          
#ifndef __APP_EVENTSERVER_H_
#define __APP_EVENTSERVER_H_

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "AeBaseServer.h"

//-------------------------------------------------------------------------
// Event Category IDs
//-------------------------------------------------------------------------
      // Simple Event Category IDs
#define CATID_SYSMESSAGE               0x100
#define CATID_USERMESSAGE              0x110
      // Tracking Event Category IDs   
#define CATID_PCHANGE                  0x200
      // Condition Event Category IDs  
#define CATID_LEVEL                    0x300
                                       
                                       
//-------------------------------------------------------------------------
// Attribute IDs                       
//-------------------------------------------------------------------------
#define ATTRID_LEVEL_CV                0x400
#define ATTRID_PCHANGE_PREVVALUE       0x401
#define ATTRID_PCHANGE_NEWVALUE        0x402
                                       
                                       
//-------------------------------------------------------------------------
// Area IDs
//-------------------------------------------------------------------------
#define AREAID_RAMPS                   0x500
#define AREAID_LIMITS                  0x501
                                       

//-------------------------------------------------------------------------
// Source IDs                          
//-------------------------------------------------------------------------
#define SRCID_SYSTEM                   0x600

                                       
//-------------------------------------------------------------------------
// CLASS
//-------------------------------------------------------------------------
class AeServer : public AeBaseServer
{
// Construction
public:
   AeServer();
   HRESULT Create();

// Destruction
   ~AeServer();

// Implementation
protected:

   // Implementation of the pure virtual functions of the AeBaseServer

   HRESULT OnGetServerState(			/*[out]*/   OPCEVENTSERVERSTATE & serverState,
										/*[out]*/   LPWSTR				& vendor );

   HRESULT OnTranslateToItemIdentifier(
                           DWORD dwCondID, DWORD dwSubCondDefID, DWORD dwAttrID,
                           LPWSTR* pszItemID, LPWSTR* pszNodeName, CLSID* pCLSID );

   HRESULT OnAcknowledgeNotification( DWORD dwCondID, DWORD dwSubConDefID );

   HRESULT OnConnectClient();

   HRESULT OnDisconnectClient();


   // Data Members
   OPCEVENTSERVERSTATE  m_dwServerState;
};

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
// The Global Alarms&Evenst Server Handler
extern AeServer* gpEventServer;
#endif

#endif // __APP_EVENTSERVER_H_

