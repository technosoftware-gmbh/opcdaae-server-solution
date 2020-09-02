/*
 * Copyright (c) 2020 Technosoftware GmbH. All rights reserved
 * Web: https://technosoftware.com 
 * 
 * The source code in this file is covered under a dual-license scenario:
 *   - Owner of a purchased license: RPL 1.5
 *   - GPL V3: everybody else
 *
 * RPL license terms accompanied with this source code.
 * See https://technosoftware.com/license/RPLv15License.txt
 *
 * GNU General Public License as published by the Free Software Foundation;
 * version 3 of the License are accompanied with this source code.
 * See https://technosoftware.com/license/GPLv3License.txt
 *
 * This source code is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.
 */

#ifndef  __OPCSDK_H_230AC692_5F6C_11d2_88DD_00104B965F5E
#define  __OPCSDK_H_230AC692_5F6C_11d2_88DD_00104B965F5E

#ifdef _DEBUG
#define DEBUG_NEW   new( _NORMAL_BLOCK, __FILE__, __LINE__)
#else
#define DEBUG_NEW
#endif

#ifdef _WIN32

      #include "inc32\opccomn.h"		// IIDs and CLSIDs for OPC Common Definitions
      #include "inc32\OpcEnum.h"        // IIDs and CLSIDs for OPC Server List / Enumerator
      #include "inc32\opcda.h"          // IIDs and CLSIDs for OPC Data Access
      #include "inc32\opc_ae.h"         // IIDs and CLSIDs for OPC Alarsm & Events
      #include "inc32\opcaedef.h"		// OPC Alarms & Events Definitions

#else

   #ifdef _WIN64

      #include "inc64\opccomn.h"		// IIDs and CLSIDs for OPC Common Definitions
      #include "inc64\OpcEnum.h"        // IIDs and CLSIDs for OPC Server List / Enumerator
      #include "inc64\opcda.h"          // IIDs and CLSIDs for OPC Data Access
      #include "inc64\opc_ae.h"         // IIDs and CLSIDs for OPC Alarsm & Events
      #include "inc64\opcaedef.h"		// OPC Alarms & Events Definitions

   #else

      #error "Platform not supported"

   #endif // _WIN64

#endif // _WIN32

#endif // __OPCSDK_H_230AC692_5F6C_11d2_88DD_00104B965F5E