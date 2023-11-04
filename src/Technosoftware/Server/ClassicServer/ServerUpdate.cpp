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
                                                // Application specific definitions
#include "DaServer.h"
#include <process.h>
#include <math.h>                               // for data simulation only
#include "Logger.h"

//-----------------------------------------------------------------------------
// CODE
//-----------------------------------------------------------------------------

// Description: 
// ------------
// This module handles:
//       - Update Rate
//       - Notification

//=============================================================================
// Update Thread
// -------------
//    This thread calls the function UpdateServerClassInstances()
//    to activate the client update threads.
//    There is a thread for each connected client. These threads checks
//    the refresh interval of the defined groups and executes an
//    OnDataChange() callback if required.
// 
//    Typically this thread also refreshes the the input signal cache.
//    The client update is so synchronized with the cache refresh.
// 
//=============================================================================
unsigned __stdcall NotifyUpdateThread( LPVOID pAttr )
{
      DaServer*  pDataServer = static_cast<DaServer *>(pAttr);
      _ASSERTE( pDataServer );                  // Must not be NULL.

      FILETIME          ftStart, ftEnd;
      LARGE_INTEGER     liStart, liEnd;
      DWORD             dwDuration, dwBaseUpdateRate;
      DWORD             dwCount = 0;

   // Writes the output signal states from the item cache to the hardware
   // Do this only one time to initialize the hardware.
   //pDataServer->OnRefreshOutputDevices( OPC_REFRESH_INIT, L"Server Internal", 0, NULL, NULL, NULL );

   // Keep this thread running until the Terminate Event is received

   for (;;) {                                   // Thread Loop

      CoFileTimeNow( &ftStart );

      // Reads the input devices and refreshs the chache
      pDataServer->OnRefreshInputCache( OPC_REFRESH_PERIODIC, 0, NULL, NULL );

      // Activate the client update threads for data callbacks
      if (FAILED( pDataServer->UpdateServerClassInstances() )) {
         //
         // TODO: Server specific error handling
         //
      }

      CoFileTimeNow( &ftEnd );

      // Calculate duration for the periodic cache update in ms
      liStart.u.LowPart = ftStart.dwLowDateTime;
      liStart.u.HighPart= ftStart.dwHighDateTime;
      liEnd.u.LowPart   = ftEnd.dwLowDateTime; 
      liEnd.u.HighPart  = ftEnd.dwHighDateTime;

      dwDuration = (DWORD)((liEnd.QuadPart - liStart.QuadPart) / 10000);

      dwBaseUpdateRate = pDataServer->GetBaseUpdateRate();

      pDataServer->m_dwBandWith = dwDuration * 100 / dwBaseUpdateRate;

      if (dwDuration > dwBaseUpdateRate) {
         //
         // TODO: If AE Server is availabe then you can add
         //       here server specific overrun error handling.
         //

         // Also check the terminate event if there is an overrrun error.
         if (WaitForSingleObject( pDataServer->m_hTerminateThraedsEvent,
                                  0 ) != WAIT_TIMEOUT) {
            break;                              // Terminate Thread
         }
      }
      else {                                    // Sleep the base interval
         if (WaitForSingleObject( pDataServer->m_hTerminateThraedsEvent,
                                  dwBaseUpdateRate - dwDuration ) != WAIT_TIMEOUT) {
            break;                              // Terminate Thread
         }
      }
   }                                            // Thread Loop

   _endthreadex( 0 );                           // The thread terminates.
   return 0;

} // NotifyUpdateThread


//=============================================================================
// CreateUpdateThread                                                  INTERNAL
// ------------------
//    Creates the Update Thread which notifies the sinks.
//=============================================================================
HRESULT DaServer::CreateUpdateThread(void)
{
   unsigned uThreadID;                          // Thread identifier

   m_hTerminateThraedsEvent = CreateEvent(
                           NULL,                // Handle cannot be inherited
                           TRUE,                // Manually reset requested
                           FALSE,               // Initial state is nonsignaled
                           NULL );              // No event object name 

   if (m_hTerminateThraedsEvent == NULL) {      // Cannot create event
      return HRESULT_FROM_WIN32( GetLastError() );
   }

   m_hUpdateThread = (HANDLE)_beginthreadex(
                           NULL,                // No thread security attributes
                           0,                   // Default stack size  
                           NotifyUpdateThread,  // Pointer to thread function 
                           this,                // Pass class to new thread for access to the update functions
                           0,                   // Run thread immediately
                           &uThreadID );        // Thread identifier


   if (m_hUpdateThread == 0) {                  // Cannot create the thread
      return HRESULT_FROM_WIN32( GetLastError() );
   }
   //
   // TODO: Increase the thread priority if required by the application
   //
   return S_OK;
}


//=============================================================================
// KillUpdateThread                                                    INTERNAL
// ----------------
//    Kills the Update Thread. This function is used during shutdown
//    of the server.
//=============================================================================
HRESULT DaServer::KillUpdateThread(void)
{	
	if (m_hTerminateThraedsEvent == nullptr) {
      return S_OK;
   }

    //LOGFMTT("KillUpdateThread() called.");
    
    SetEvent( m_hTerminateThraedsEvent );        // Set the signal to shutdown the threads.

   if (m_hUpdateThread) {  
                                                // Wait max 30 secs until the update thread has terminated.
      if (WaitForSingleObject( m_hUpdateThread, 30000 ) == WAIT_TIMEOUT) {
         TerminateThread( m_hUpdateThread, 1 );
      }
      CloseHandle( m_hUpdateThread );
      m_hUpdateThread = nullptr;
   }

   CloseHandle( m_hTerminateThraedsEvent );
   m_hTerminateThraedsEvent = nullptr;

   //LOGFMTT("KillUpdateThread() finished with hres = 0x%x.", S_OK);
   return S_OK;
}
