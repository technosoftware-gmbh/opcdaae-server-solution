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
#include "stdafx.h"
#include "DaComBaseServer.h"
#include "Logger.h"

//-----------------------------------------------------------------------------
// CODE
//-----------------------------------------------------------------------------

// Description: 
// ------------
// This is the module for handling of configuration files.
//
// This module defines the application specific implementation of
// the Custom and Dispatch interface functions used for handling
// of configuration files.
// The following interface functions are implemented in this module:
//
//    IOPCServerDisp       LoadConfig
//                         SaveConfig 
//
//    IPersistFile         IsDirty
//                         Load
//                         Save
//                         SaveCompleted
//                         GetCurFile
//                         GetClassID

///////////////////////////////////////////////////////////////////////////////
/////////////////////////////// IOPCServerDisp ////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// IOPCServerDisp::SaveConfig                                         INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::SaveConfig(
	/* [in] */                    BSTR        FileName)
{
	return E_NOTIMPL;
}


//=============================================================================
// IOPCServerDisp::LoadConfig                                         INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::LoadConfig(
	/*[in]*/                      BSTR        FileName)
{
	return E_NOTIMPL;
}


///////////////////////////////////////////////////////////////////////////////
//////////////////////////////// IPersistFile /////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//=============================================================================
// IPersistFile::IsDirty                                              INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::IsDirty(void)
{
	LOGFMTI("IPersistFile::IsDirty");
	return E_NOTIMPL;
}


//=============================================================================
// IPersistFile::Load                                                 INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::Load(
	/* [in] */                    LPCOLESTR   pszFileName,
	/* [in] */                    DWORD       dwMode)
{
	USES_CONVERSION; 
	LOGFMTI("IPersistFile::Load file %s", W2A(pszFileName));
	return E_NOTIMPL;
}


//=============================================================================
// IPersistFile::Save                                                 INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::Save(
	/* [in, unique] */            LPCOLESTR   pszFileName,
	/* [in]         */            BOOL        fRemember)
{
	USES_CONVERSION;
	LOGFMTI("IPersistFile::Save file %s", W2A(pszFileName));
	return E_NOTIMPL;
}


//=============================================================================
// IPersistFile::SaveCompleted                                        INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::SaveCompleted(
	/* [in, unique] */            LPCOLESTR   pszFileName)
{
	USES_CONVERSION;
	LOGFMTI("IPersistFile::SaveCompleted file %s", W2A(pszFileName));
	return E_NOTIMPL;
}


//=============================================================================
// IPersistFile::GetCurFile                                           INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::GetCurFile(
	/* [out] */                   LPOLESTR *  ppszFileName)
{
	LOGFMTI("IPersistFile::GetCurFile");

	*ppszFileName = NULL;
	return E_NOTIMPL;
}


//=============================================================================
// IPersistFile::GetClassID                                           INTERFACE
//=============================================================================
STDMETHODIMP DaComBaseServer::GetClassID(
	/* [out] */                   CLSID    *  pClassID)
{
	LOGFMTI("IPersistFile::GetClassID");

	*pClassID = CLSID_NULL;
	return E_NOTIMPL;
}