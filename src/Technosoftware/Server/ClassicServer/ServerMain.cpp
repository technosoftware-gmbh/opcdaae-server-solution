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
#include <windows.h>
#include <stdio.h>
#include "CoreMain.h"
// Application specific definitions
#ifdef NDEBUG
#include <stdio.h>
#endif
#include <Crtdbg.h>
#include "DaServer.h"
#ifdef _OPC_SRV_AE
#include "AeServer.h"
#endif
#ifdef _OPC_NET
#include "app_netserverwrapper.h" 
#endif

#include "OpcTextReader.h"
#include "Logger.h"

using namespace IClassicBaseNodeManager;

//-----------------------------------------------------------------------------
// DEFINE
//-----------------------------------------------------------------------------
#ifdef _OPC_NET
#define DLL_DEFAULT_NAME      _T("ServerPlugin.dll")
#define CMDLINETOKEN_DLL      _T("DLL")
#define CMDLINETOKEN_PARAM    _T("PARAM")
#define MSGBOX_TITLE          _T("OPC DA/AE Server SDK .NET")
#endif

#ifdef _OPC_DLL
#define DLL_DEFAULT_NAME      _T("ServerPlugin.dll")
#define CMDLINETOKEN_DLL      _T("dll")
#define CMDLINETOKEN_PARAM    _T("param")
#define MSGBOX_TITLE          _T("OPC DA/AE Server SDK DLL")
#endif

//=============================================================================
// The one and only DaServer/AeServer objects
//=============================================================================

// Global pointer to the Application Specific OPC Data Access Handler.
DaServer* gpDataServer = nullptr;

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
// Global pointer to the Application Specific OPC Alarms&Events Handler.
AeServer* gpEventServer = nullptr;
#endif

// The fully qualified name of the loaded DLL
static TCHAR gszFullyQualifiedDLLName[ _MAX_PATH ];

//-----------------------------------------------------------------------------
// Global Variables
//-----------------------------------------------------------------------------
int         gdwBrowseMode = 0;					// Browse Mode
int         gdwUpdateCyclePeriod = 0;           // Device update rate in ms
bool 		gUseOnItemRequest = true;
bool		gUseOnRefreshItems = true;
bool 		gUseOnAddItem = false;
bool 		gUseOnRemoveItem = false;
LPWSTR	    vendor_name;


HINSTANCE   gDLLHandle = nullptr;                     // DLL Handle from LoadLibrary
TCHAR*      gpszDLLParams = nullptr;               // Parameter string for the Application-DLL
BOOL        gfRegister = FALSE;

#ifdef _OPC_DLL
// Technosoftware Server Function Pointers

PFNONGETLICENSEINFORMATION             pOnGetLicenseInformation;
PFNONGETLOGLEVEL					   pOnGetLogLevel;
PFNONGETLOGPATH						   pOnGetLogPath;



PFNONGETDASEERVERREGISTRYDEFINITION    pOnGetDaServerDefinition;
PFNONGETDASERVERPARAMETERS             pOnGetDaServerParameters;
PFNONDEFINEDACALLBACKS                 pOnDefineDaCallbacks;
PFNONCREATESERVERITEMS                 pOnCreateServerItems;
PFNONCLIENTCONNECT                     pOnClientConnect;
PFNONCLIENTDISCONNECT                  pOnClientDisconnect;
PFNONREQUESTITEMS                      pOnRequestItems;
PFNONSTARTUPSIGNAL                     pOnStartupSignal;
PFNONSHUTDOWNSIGNAL                    pOnShutdownSignal;
PFNONREFRESHITEMS                      pOnRefreshItems;
PFNONWRITEITEMS                        pOnWriteItems;
PFNONBROWSECHANGEPOSITION              pOnBrowseChangePosition;
PFNONBROWSEItemIdS					   pOnBrowseItemIds;
PFNONBROWSEGETFULLItemId			   pOnBrowseGetFullItemId;
PFNONQUERYPROPERTIES				   pOnQueryProperties;
PFNONGETPROPERTYVALUE			       pOnGetPropertyValue;

PFNONGETDAOPTIMIZATIONPARAMETERS       pOnGetDaOptimizationParameters;
PFNONADDITEM					       pOnAddItem;
PFNONREMOVEITEM						   pOnRemoveItem;

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
PFNONDEFINEAECALLBACKS                 pOnDefineAeCallbacks;
PFNONTRANSLATETOItemId                 pOnTranslateToItemId;
PFNONACKNOTIFICATION                   pOnAckNotification;
PFNONGETAESEERVERREGISTRYDEFINITION    pOnGetAeServerDefinition;
#endif

#endif

//-----------------------------------------------------------------------------
// LOCAL
//-----------------------------------------------------------------------------

//=============================================================================
// SERVER REGISTRY DEFINITIONS
//=============================================================================

// The following setting are overwritten by the assemply plugin defined values
static SERVERREGDEFS RegistryDefinitions =
{
	//    Application
	L"{8817FE7E-0C33-4ebb-BF7E-6A9C0C17FA02}",                  // Application identifier
	L"Technosoftware DA Sample Server V1.0",                          // Application description

	//    Data Access Server
	L"{1B8200C7-DFA0-473c-AED8-315037F3AAD7}",                  // CLSID of Data Access Server
	L"Technosoftware.DataSample",									// Version independent ProgID
	L"Technosoftware.DataSample.10",								// ProgID of current server version
#ifdef _DEBUG
#define DESCR_POSTFIX L" (Debug)"
#else
#define DESCR_POSTFIX L""
#endif
	L"Technosoftware DA Sample Server" DESCR_POSTFIX,					// Friendly name of version independent server
	L"Technosoftware DA Sample Server V1.0" DESCR_POSTFIX,			// Friendly name of current server version

	FALSE,														// Don't use AE Server
	// Alarms & Events Server
	L"",                                                        // CLSID of Alarms & Events Server
	L"",     // This Sample Server provides                     // Version independent ProgID
	L"",     // no Alarms & Events functionality                // ProgID of current server version
	L"",                                                        // Friendly name of version independent server
	L""                                                         // Friendly name of current server version
};

//-----------------------------------------------------------------------------
// CODE
//-----------------------------------------------------------------------------

//=============================================================================
// IMPLEMENTATION INTERNAL FUNCTIONS
//=============================================================================


#ifdef _OPC_DLL
//=========================================================================
// LoadAppDLL
// ----------
//    Loads the Application DLL and initializes the DLL function pointers.
//=========================================================================
HRESULT LoadAppDLL( LPCTSTR DLLName, BOOL & fErrMsgDisplayed )
{
	HRESULT hres = S_OK;

	fErrMsgDisplayed = FALSE;
	gDLLHandle = LoadLibrary( DLLName );
	if (!gDLLHandle) {
		hres = HRESULT_FROM_WIN32( GetLastError() );
		LOGFMTE("LoadLibrary() failed with hres = 0x%x.", hres );
		return hres;
	}

	// LoadLibrary call was successfully

	pOnGetLogLevel = (PFNONGETLOGLEVEL)GetProcAddress(gDLLHandle, "OnGetLogLevel");
	if (pOnGetLogLevel == nullptr)
	{
		LOGFMTE("Function OnGetLogLevel not found().");
		hres = TYPE_E_DLLFUNCTIONNOTFOUND;
	}

	pOnGetLogPath = (PFNONGETLOGPATH)GetProcAddress(gDLLHandle, "OnGetLogPath");
	if (pOnGetLogPath == nullptr)
	{
		LOGFMTE("Function OnGetLogPath() not found.");
		hres = TYPE_E_DLLFUNCTIONNOTFOUND;
	}

	int logLevel = (*pOnGetLogLevel)();
	LOGFMTT("OnGetLogLevel() returned log level %i.", logLevel);
	char * logPath = "";
	if (logLevel != LOG_LEVEL_DISABLE)
	{
		pOnGetLogPath(logPath);
		USES_CONVERSION;
		if (logPath != nullptr && strlen(logPath) > 0)
		{
			LOGFMTT("OnGetLogPath() returned log path %s.", logPath);
			LogManager::getRef().setLoggerPath(LOGGER_MAIN_LOGGER_ID, logPath);
		}
		else
		{
			LOGFMTT("OnGetLogPath() returned no log path.");
		}
        LogManager::getRef().stop();

		// configure the output behaviour
		LogManager::getRef().setLoggerPath(LOGGER_MAIN_LOGGER_ID, logPath);
		LogManager::getRef().setLoggerLevel(LOGGER_MAIN_LOGGER_ID, logLevel);
		LogManager::getRef().setLoggerDisplay(LOGGER_MAIN_LOGGER_ID, false);
		LogManager::getRef().setLoggerFileLine(LOGGER_MAIN_LOGGER_ID, false);

		// start logger
		LogManager::getRef().start();
	}
	else
	{
		//stop logger
		LOGFMTT("Stop logging requested.");
		LogManager::getRef().stop();
	}

	pOnGetDaServerDefinition = (PFNONGETDASEERVERREGISTRYDEFINITION)GetProcAddress( gDLLHandle, "OnGetDaServerDefinition" );
	if (pOnGetDaServerDefinition == nullptr)  
	{
		LOGFMTE("Function OnGetDaServerDefinition() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
	pOnGetAeServerDefinition = (PFNONGETAESEERVERREGISTRYDEFINITION)GetProcAddress( gDLLHandle, "OnGetAeServerDefinition" );
	if (pOnGetAeServerDefinition == nullptr)  hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
#endif

	pOnGetDaServerParameters = (PFNONGETDASERVERPARAMETERS)GetProcAddress( gDLLHandle, "OnGetDaServerParameters" );
	if (pOnGetDaServerParameters == nullptr)
	{
		LOGFMTE("Function OnGetDaServerParameters() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnDefineDaCallbacks = (PFNONDEFINEDACALLBACKS)GetProcAddress( gDLLHandle, "OnDefineDaCallbacks" );
	if (pOnDefineDaCallbacks == nullptr)
	{
		LOGFMTE("Function OnDefineDaCallbacks() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnCreateServerItems = (PFNONCREATESERVERITEMS)GetProcAddress( gDLLHandle, "OnCreateServerItems" );
	if (pOnCreateServerItems == nullptr)
	{
		LOGFMTE("Function OnCreateServerItems() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnClientConnect = (PFNONCLIENTCONNECT)GetProcAddress( gDLLHandle, "OnClientConnect" );
	/*
	* OnClientConnect is optional and can be missed
	*/
	// if (pOnClientConnect == NULL)     hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnClientDisconnect = (PFNONCLIENTDISCONNECT)GetProcAddress( gDLLHandle, "OnClientDisconnect" );
	/*
	* OnClientDisconnect is optional and can be missed
	*/
	//if (pOnClientDisconnect == NULL)     hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnRefreshItems = (PFNONREFRESHITEMS)GetProcAddress( gDLLHandle, "OnRefreshItems" );
	if (pOnRefreshItems == nullptr)
	{
		LOGFMTE("Function OnRefreshItems() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnWriteItems = (PFNONWRITEITEMS)GetProcAddress( gDLLHandle, "OnWriteItems" );
	if (pOnWriteItems == nullptr)
	{
		LOGFMTE("Function OnWriteItems() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnRequestItems = (PFNONREQUESTITEMS)GetProcAddress( gDLLHandle, "OnRequestItems" );
	/*
	* OnRequestItems is optional and can be missed
	*/
	//if (pOnRequestItems == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnStartupSignal = (PFNONSTARTUPSIGNAL)GetProcAddress( gDLLHandle, "OnStartupSignal" );
	if (pOnStartupSignal == nullptr)
	{
		LOGFMTE("Function OnStartupSignal() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnShutdownSignal = (PFNONSHUTDOWNSIGNAL)GetProcAddress( gDLLHandle, "OnShutdownSignal" );
	/*
	* OnShutdownSignal is optional and can be missed
	*/
	//if (pOnShutdownSignal == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnBrowseChangePosition = (PFNONBROWSECHANGEPOSITION)GetProcAddress( gDLLHandle, "OnBrowseChangePosition" );
	/*
	* OnBrowseChangePosition is optional and can be missed
	*/
	//if (pOnBrowseChangePosition == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnBrowseItemIds = (PFNONBROWSEItemIdS)GetProcAddress( gDLLHandle, "OnBrowseItemIds" );
	/*
	* OnBrowseItemIds is optional and can be missed
	*/
	//if (pOnBrowseItemIds == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnBrowseGetFullItemId = (PFNONBROWSEGETFULLItemId)GetProcAddress( gDLLHandle, "OnBrowseGetFullItemId" );
	/*
	* OnBrowseGetFullItemId is optional and can be missed
	*/
	//if (pOnBrowseGetFullItemId == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnQueryProperties = (PFNONQUERYPROPERTIES)GetProcAddress( gDLLHandle, "OnQueryProperties" );
	/*
	* OnQueryProperties is optional and can be missed
	*/
	//if (pOnQueryProperties == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnGetPropertyValue = (PFNONGETPROPERTYVALUE)GetProcAddress( gDLLHandle, "OnGetPropertyValue" );
	/*
	* OnGetPropertyValue is optional and can be missed
	*/
	//if (pOnGetPropertyValue == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnGetDaOptimizationParameters = (PFNONGETDAOPTIMIZATIONPARAMETERS)GetProcAddress( gDLLHandle, "OnGetDaOptimizationParameters" );
	if (pOnGetDaOptimizationParameters == nullptr)
	{
		LOGFMTE("Function OnGetDaOptimizationParameters() not found." );
		hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
	}

	pOnAddItem = (PFNONADDITEM)GetProcAddress( gDLLHandle, "OnAddItem" );
	/*
	* OnAddItem is optional and can be missed
	*/
	//if (pOnAddItem == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnRemoveItem = (PFNONREMOVEITEM)GetProcAddress( gDLLHandle, "OnRemoveItem" );
	/*
	* OnRemoveItem is optional and can be missed
	*/
	//if (pOnRemoveItem == NULL)        hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
	pOnDefineAeCallbacks = (PFNONDEFINEAECALLBACKS)GetProcAddress( gDLLHandle, "OnDefineAeCallbacks" );	
	if (pOnDefineAeCallbacks == nullptr)     hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnTranslateToItemId = (PFNONTRANSLATETOItemId)GetProcAddress( gDLLHandle, "OnTranslateToItemId" );	
	if (pOnTranslateToItemId == nullptr)     hres = TYPE_E_DLLFUNCTIONNOTFOUND ;

	pOnAckNotification = (PFNONACKNOTIFICATION)GetProcAddress( gDLLHandle, "OnAckNotification" );	
	if (pOnAckNotification == nullptr)     hres = TYPE_E_DLLFUNCTIONNOTFOUND ;
#endif

	// Get the Version Info from the loaded DLL
	if (SUCCEEDED( hres )) {
		// Delete the existing Version info from this application
		// and load the Version Info from the loaded DLL.
		hres = core_generic_main.m_VersionInfo.Create( gDLLHandle );

		// Check the Version Info from the loaded DLL
		LPCTSTR  psz;
		TCHAR    buf[ _MAX_PATH + 150 ] = _T("\0");

		if (SUCCEEDED( hres )) {
			psz = core_generic_main.m_VersionInfo.GetValue( _T("CompanyName") );
			if (!psz) {
				_tcscpy_s( buf, _T("Cannot start the server!\nThe item 'CompanyName' is missing in the version resource\nof the DLL '") );
				_tcscat_s( buf, DLLName );
				_tcscat_s( buf, _T("'!") );
			}
			else if (*psz == '\0') {
				_tcscpy_s( buf, _T("Cannot start the server!\nThe value for the version resource item 'CompanyName' in\nthe DLL '") );
				_tcscat_s( buf, DLLName );
				_tcscat_s( buf, _T("' is not defined!") );
			}
		}
		else
		{
			hres = S_OK;
		}
		if (!(*buf)) {
			psz = core_generic_main.m_VersionInfo.GetValue( _T("FileDescription") );
			if (!psz) {
				_tcscpy_s( buf, _T("Cannot start the server!\nThe item 'FileDescription' is missing in the version resource\nof the DLL '") );
				_tcscat_s( buf, DLLName );
				_tcscat_s( buf, _T("'!") );
			}
			else if (*psz == '\0') {
				_tcscpy_s( buf, _T("Cannot start the server!\nThe value for the version resource item 'FileDescription' in\n the DLL '") );
				_tcscat_s( buf, DLLName );
				_tcscat_s( buf, _T("' is not defined!") );
			}
		}
		if (*buf) {
			MessageBox(nullptr, buf, MSGBOX_TITLE, MB_OK | MB_ICONSTOP ); 
			fErrMsgDisplayed = TRUE;
			hres = E_FAIL;
		}

	}


	if (FAILED( hres )) {                        // Function not found or other error
		LOGFMTE( "LoadLibrary() failed with hres = 0x%x.", hres );
		FreeLibrary( gDLLHandle );
		gDLLHandle = nullptr;
		return hres;
	}
	LOGFMTT( "LoadLibrary() finished with hres = 0x%x.", hres );
	return hres;
}
#endif

void LogEnable(const string loggerDefaultPath, const string arguments)
{
	string applicationDll;
	COpcString cFlags;
	BOOL     fErrMsgDisplayed = FALSE;
    HRESULT  hres = S_OK;
    BOOL     fAbort = FALSE;

#ifdef _OPC_DLL
	// parse command line arguments.
	COpcText cText;
	COpcTextReader cReader(arguments.c_str());

	// skip until first command flag - if any.
	cText.SetType(COpcText::Delimited);
	cText.SetDelims(L"-/");

	if (cReader.GetNext(cText))
	{
		// read first command flag.
		cText.SetType(COpcText::NonWhitespace);
		cText.SetEofDelim();

		while (cReader.GetNext(cText))
		{
			cFlags = ((COpcString&)cText).ToLower();
			int pos = cFlags.Find(CMDLINETOKEN_DLL);
			if (pos >= 0)
			{
				pos = cFlags.Find(":");
                if (pos >= 0)
                {
                    applicationDll = cFlags.SubStr(pos + 1, -1);
                }
			}
		}
	}

	if (applicationDll.length() == 0) {  // No DLL name specified by command line
		applicationDll = DLL_DEFAULT_NAME;
	}
#endif
#ifdef _OPC_DLL
	// Load the application DLL	

	LOGFMTT("Load the application DLL '%s'", applicationDll.c_str());
	hres = LoadAppDLL(applicationDll.c_str(), fErrMsgDisplayed);

	// Get the fully qualified name of the loaded DLL
	if (SUCCEEDED(hres)) {
		if (!GetModuleFileName(gDLLHandle, gszFullyQualifiedDLLName, _countof(gszFullyQualifiedDLLName))) {
			hres = HRESULT_FROM_WIN32(GetLastError());
			LOGFMTE("GetModuleFileName() failed with hres = 0x%x.", hres);
		}
		else
		{
			LOGFMTT("Loaded application DLL is '%s'", T2CA(gszFullyQualifiedDLLName));
		}
	}
#endif

#ifdef _OPC_NET
	int logLevel = GenericServerAPI::OnGetLogLevel();
	const char * logPath = "";
	if (logLevel != LOG_LEVEL_DISABLE)
	{
		logPath = GenericServerAPI::OnGetLogPath();

		if (logPath == NULL || strlen(logPath) == 0)
		{
			logPath = loggerDefaultPath.c_str();
		}

		// configure the output behaviour
		LogManager::getRef().setLoggerPath(LOGGER_MAIN_LOGGER_ID, logPath);
		LogManager::getRef().setLoggerLevel(LOGGER_MAIN_LOGGER_ID, logLevel);
		LogManager::getRef().setLoggerDisplay(LOGGER_MAIN_LOGGER_ID, false);
		LogManager::getRef().setLoggerFileLine(LOGGER_MAIN_LOGGER_ID, false);

		// start logger
		LogManager::getRef().start();

		LOGFMTI("Logging enabled with path '%s' and level = 0x%x", logPath, logLevel);

	}
	else
	{
		// stop logger
		LogManager::getRef().stop();
	}
#endif

    // Error handling
    if (FAILED(hres)) {
        fAbort = TRUE;
        if (!fErrMsgDisplayed) {                  // Display error message if not yet done

            if (applicationDll.length() > 0) {
                LOGFMTE("Server installation aborted. Cannot load Driver DLL '%s'. hres = 0x%x", applicationDll.c_str(), hres);
            }
        }
    }

    if (fAbort) {
        exit(hres);								// Terminate the application if something goes wrong
    }

}

//=============================================================================
// AddParamToRegistry
// ------------------
//    Adds a parameter to the Registry key
//    HKEY_CLASSES_ROOT\CLSID\{<clsid>}\LocalServer32
//=============================================================================
HRESULT AddParamToRegistry( LPCTSTR pszParam )
{
	// Get Registry Defs from DLL
#ifdef _OPC_NET
	ServerRegDefs1* pDefs = GenericServerAPI::OnGetDaServerDefinition();
#endif

#ifdef _OPC_DLL
	ServerRegDefs* pDefs = (*pOnGetDaServerDefinition)();
#endif
	_ASSERTE( pDefs != NULL );
	if (!pDefs)
	{
		LOGFMTE("OnGetDaServerDefinition() failed with hres = 0x%x.", E_FAIL);
		return E_FAIL;
	}

	USES_CONVERSION;
	TCHAR szKey[70] = _T("CLSID\\");
	_tcscat_s( szKey, W2T(pDefs->ClsidServer) );
	_tcscat_s( szKey, _T("\\LocalServer32") );

	CRegKey key;
	LONG lRes = key.Open( HKEY_CLASSES_ROOT, szKey );
	if (lRes != ERROR_SUCCESS)
	{
		LOGFMTE("OnGetDaServerDefinition() failed with hres = 0x%x.", HRESULT_FROM_WIN32(lRes));
		return HRESULT_FROM_WIN32(lRes);
	}

	TCHAR szLocalServer[512];
	DWORD dwc = sizeof (szLocalServer);
	lRes = key.QueryStringValue( _T(""), szLocalServer, &dwc );

	if (lRes != ERROR_SUCCESS)
	{
		LOGFMTE("OnGetDaServerDefinition() failed with hres = 0x%x.", HRESULT_FROM_WIN32(lRes));
		return HRESULT_FROM_WIN32(lRes);
	}

	_tcscat_s( szLocalServer, _T(" ") );
	_tcscat_s( szLocalServer, pszParam );
	lRes = key.SetStringValue( _T(""), szLocalServer );
	if (lRes != ERROR_SUCCESS)
	{
		LOGFMTE("OnGetDaServerDefinition() failed with hres = 0x%x.", HRESULT_FROM_WIN32(lRes));
		return HRESULT_FROM_WIN32(lRes);
	}
	LOGFMTT("OnGetDaServerDefinition() finished with hres = 0x%x.", HRESULT_FROM_WIN32(lRes));
	return HRESULT_FROM_WIN32( lRes );
}


//=============================================================================
// IMPLEMENTATION GLOBAL INTERFACE FUNCTIONS
//=============================================================================

//=============================================================================
// OnProcessParam
// --------------
//    Custom parameter processing when server started from command line
//    or by COM (with switches /Embedding or /Automation).
//    The generic server part handles the following arguments:
//          /Install       
//          /Deinstall
//          /RegServer
//          /UnregServer
//
//    For all other argument strings this function is called.
//    Return E_FAIL to signal that the server must not be run.
//
//    OPC Swift Server specific parameter handling:
//    All Server specific parameters are added to the appropriate
//    Registry entry if the global flag 'gfRegister' is set (the
//    fully qualified name of the DLL is used).
//    A copy of the DLL-Driver specific parameter string is stored
//    in the global variable 'gpszDLLParams'.
//=============================================================================
HRESULT OnProcessParam( COpcString cFlags ) 
{
	HRESULT  hres = S_OK;

	if (cFlags== _T("embedding") || cFlags== _T("automation")) 
	{
		// Ignore the OLE switch parameters 'Embedding' and 'Automation'
		hres = S_OK;
	}
	else
	{
		if (gfRegister) {												// Update the Registry
			hres = AddParamToRegistry( cFlags );
		}
	}
	return hres;
}

//=============================================================================
// OnGetServerRegistryDefs
// -----------------------
//    Returns the address of the structure which specifies all
//    definitions required to register the server in the Registry.
//
//    This function also loads the application DLL.
//
//    Used Application DLL and command-line parameters :
//       Uses the name of the DLL from the command-line if specified.
//       Sets the global flag 'gfRegister' if the command-line
//       option /RegServer  or /Install is specified.
//
//    Also the global members gDLLHandle and gszFullyQualifiedDLLName
//    are initialized.
//
//    Note :   Terminate the application and do not return from this
//             function if something goes wrong.
//=============================================================================
LPSERVERREGDEFS OnGetServerRegistryDefs()
{
	HRESULT  hres = S_OK;
	COpcString cFlags;

#ifdef _OPC_NET
	GenericServerAPI::OnStartupSignal(GetCommandLine());
#endif
#ifdef _OPC_DLL
	(*pOnStartupSignal) (GetCommandLine());
#endif
	LOGFMT_TRACE(LOGGER_MAIN_LOGGER_ID, "OnStartupSignal( %ls ) finished.",GetCommandLine());


	WCHAR    wDelimiter;

	// Get Server Parameters from DLL
#ifdef _OPC_NET
	hres = GenericServerAPI::OnGetDAServerParameters(&gdwUpdateCyclePeriod, &wDelimiter, &gdwBrowseMode);
#endif

#ifdef _OPC_DLL
	hres = (*pOnGetDaServerParameters) ( &gdwUpdateCyclePeriod, &wDelimiter, &gdwBrowseMode );
#endif


	// Error handling
	if (FAILED( hres )) {
		LOGFMTE("OnGetDAServerParameters() failed with hres = 0x%x.", hres);
		exit(hres);								// Terminate the application if something goes wrong
	}

	LOGFMTT("OnGetDAServerParameters() finished with hres = 0x%x.", hres);

	// Get Server Optimization Parameters from DLL
#ifdef _OPC_NET
	hres = GenericServerAPI::OnGetDAOptimizationParameters(&gUseOnItemRequest, &gUseOnRefreshItems, &gUseOnAddItem, &gUseOnRemoveItem);
#endif

#ifdef _OPC_DLL
	hres = (*pOnGetDaOptimizationParameters) ( &gUseOnItemRequest, &gUseOnRefreshItems, &gUseOnAddItem, &gUseOnRemoveItem );
	if (pOnRequestItems == nullptr)
	{
		gUseOnItemRequest = false;
	}
#endif

	// Error handling
	if (FAILED( hres )) {
		LOGFMTE("OnGetDAOptimizationParameters() failed with hres = 0x%x.", hres);
		exit(hres);								// Terminate the application if something goes wrong
	}

	LOGFMTT("OnGetDAOptimizationParameters() finished with hres = 0x%x.", hres);

	DaBranch::SetDelimiter(wDelimiter);
	// Get DA Server Registry Defs from DLL
#ifdef _OPC_NET
	ServerRegDefs1* pRegDefsDLL = GenericServerAPI::OnGetDaServerDefinition();
#endif

#ifdef _OPC_DLL
	ServerRegDefs* pRegDefsDLL = (*pOnGetDaServerDefinition)();
#endif

	if (pRegDefsDLL == nullptr) {
		LOGFMTE("OnGetDaServerDefinition() failed with returning NULL.");
		return nullptr;
	}                                            // Initialize the new Registry Definition Structure
	// Application
	RegistryDefinitions.m_szAppID            = pRegDefsDLL->ClsidApp;
	RegistryDefinitions.m_szAppDescr         = pRegDefsDLL->NameCurrServer;

	// Data Access Server
	RegistryDefinitions.m_szDA_CLSID         = pRegDefsDLL->ClsidServer;
	RegistryDefinitions.m_szDA_IndepProgID   = pRegDefsDLL->PrgidServer;
	RegistryDefinitions.m_szDA_CurrProgID    = pRegDefsDLL->PrgidCurrServer;
	RegistryDefinitions.m_szDA_IndepDescr    = pRegDefsDLL->NameServer;
	RegistryDefinitions.m_szDA_CurrDescr     = pRegDefsDLL->NameCurrServer;

	RegistryDefinitions.m_fUseAE_Server		 = FALSE;

	LOGFMTI(  "OnGetDaServerDefinition() returned DA server definitions:" );
	OPCWSTOAS( RegistryDefinitions.m_szDA_CLSID );
		LOGFMTI( "   CLSID       '%s'", OPCastr );
    }
    OPCWSTOAS( RegistryDefinitions.m_szDA_IndepProgID );  
		LOGFMTI( "   IndepProgID '%s'", OPCastr );
	}
	OPCWSTOAS( RegistryDefinitions.m_szDA_CurrProgID );
		LOGFMTI( "   CurrProgID  '%s'", OPCastr );
	}
	OPCWSTOAS( RegistryDefinitions.m_szDA_IndepDescr );
		LOGFMTI( "   IndepDescr  '%s'", OPCastr );
	}
	OPCWSTOAS( RegistryDefinitions.m_szDA_CurrDescr );
		LOGFMTI( "   CurrDescr   '%s'", OPCastr );
	}

#ifdef _OPC_NET
    vendor_name = pRegDefsDLL->CompanyName;
#elif _OPC_DLL
	vendor_name = pRegDefsDLL->CompanyName;
#else
    vendor_name = _wcsdup( T2CW( _Module.m_VersionInfo.GetValue( _T("CompanyName") ) ) );
#endif

	OPCWSTOAS( vendor_name );
	LOGFMTT( "   VendorName  '%s'", OPCastr );
	}

#ifdef _OPC_SRV_AE
	// Get AE Server Registry Defs from DLL
#ifdef _OPC_NET
	pRegDefsDLL = GenericServerAPI::OnGetAeServerDefinition();
#endif

#ifdef _OPC_DLL
	pRegDefsDLL = (*pOnGetAeServerDefinition)();
#endif
	if (pRegDefsDLL == nullptr || !pRegDefsDLL->ClsidServer) {
		LOGFMTT("OnGetAeServerDefinition() returned NULL.");
		return &RegistryDefinitions;
	}                                            // Initialize the new Registry Definition Structure

	RegistryDefinitions.m_fUseAE_Server		 = TRUE;

	// Alarms&Events Server
	RegistryDefinitions.m_szAE_CLSID         = pRegDefsDLL->ClsidServer;
	RegistryDefinitions.m_szAE_IndepProgID   = pRegDefsDLL->PrgidServer;
	RegistryDefinitions.m_szAE_CurrProgID    = pRegDefsDLL->PrgidCurrServer;
	RegistryDefinitions.m_szAE_IndepDescr    = pRegDefsDLL->NameServer;
	RegistryDefinitions.m_szAE_CurrDescr     = pRegDefsDLL->NameCurrServer;

	LOGFMTI("OnGetAeServerDefinition() returned AE server definitions:");
	OPCWSTOAS(RegistryDefinitions.m_szAE_CLSID);
		LOGFMTI("   CLSID       '%s'", OPCastr);
	}
	OPCWSTOAS(RegistryDefinitions.m_szAE_IndepProgID);
		LOGFMTI("   IndepProgID '%s'", OPCastr);
	}
	OPCWSTOAS(RegistryDefinitions.m_szAE_CurrProgID);
		LOGFMTI("   CurrProgID  '%s'", OPCastr);
	}
	OPCWSTOAS(RegistryDefinitions.m_szAE_IndepDescr);
		LOGFMTI("   IndepDescr  '%s'", OPCastr);
	}
	OPCWSTOAS(RegistryDefinitions.m_szAE_CurrDescr);
		LOGFMTI("   CurrDescr   '%s'", OPCastr);
	}

#endif
	return &RegistryDefinitions;
}



//=============================================================================
// OnInitializeServer
// ------------------
//    This function is called when the server EXE is loaded in memory
//    and creates an instance of the Data Access Server Handler instance.
//    This function is called from the generic part function
//    CoreGenericMain::InitializeServer() in module CoreMain.cpp.
//=============================================================================
HRESULT OnInitializeServer() 
{
    LOGFMTI("OnInitializeServer() called.");
    //_CrtSetDbgFlag( _CRTDBG_LEAK_CHECK_DF | _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) );
	// Init the global Data Access Server Instance
	gpDataServer = new DaServer();
	if (gpDataServer == nullptr) {
		return E_OUTOFMEMORY;
	}
#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
	// Init the global Alarms&Events Server Instance
	if (RegistryDefinitions.m_fUseAE_Server == TRUE)
	{
		gpEventServer = new AeServer();
		if (gpEventServer == nullptr) {
			return E_OUTOFMEMORY;
		}
	}
#endif
	HRESULT hres;
#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
	if (gpEventServer != nullptr)
	{
		hres = gpEventServer->Create();
	}
#endif
	hres = gpDataServer->Create(
		gdwUpdateCyclePeriod,					// Device Update rate in ms
		gpszDLLParams,							// Parameter string for the Application-DLL
		MSGBOX_TITLE );							// Server Instance Name

	if (FAILED( hres )) {
#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
		if (gpEventServer != nullptr)
		{
			delete gpEventServer;
			gpEventServer = nullptr;
		}
#endif
		delete gpDataServer;
		gpDataServer = nullptr;
	}
	return hres;
}



//=============================================================================
// OnTerminateServer
// -----------------
//    This function is called when the server EXE in unloaded from memory.
//    Destroys all what the OnInitializeServer() function has created.
//    This function also frees the application DLL.
//=============================================================================
HRESULT OnTerminateServer() 
{
    LOGFMTI("OnTerminateServer() called.");
    HRESULT hres = S_OK;

#ifdef _OPC_NET
	GenericServerAPI::OnShutdownSignal();
#endif

#ifdef _OPC_DLL
	if (pOnShutdownSignal != nullptr)
	{
		(*pOnShutdownSignal)();
	}
#endif

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server
	if (gpEventServer) {
		delete gpEventServer;
		gpEventServer = nullptr;
	}
#endif
	if (gpDataServer) {
		delete gpDataServer;
		gpDataServer = nullptr;
	}

	if (gDLLHandle) {
		FreeLibrary( gDLLHandle );
		gDLLHandle = nullptr;
	}
	if (gpszDLLParams) {
		free( gpszDLLParams );
		gpszDLLParams = nullptr;
	}
	return hres;
}



//=============================================================================
// OnGetDaBaseServer
// ------------------
//    Returns a pointer to the Data Access Server Handler instance for
//    the given Server Class ID.
// Parameters:
//    IN
//       szServerClassID         OPC:     It's always a NULL-Pointer
//                                        because OPC supports only
//                                        one COM-Server per EXE. 
//                               CALL-R:  Text string with the identifier
//                                        of the requested Data Access
//                                        Server Handler. The identifier
//                                        can be defined with function
//                                        DaBaseServer::Create(). 
//=============================================================================
DaBaseServer* OnGetDaBaseServer( LPCWSTR szServerClassID )
{
	return gpDataServer;
}

#ifdef   _OPC_SRV_AE                            // Alarms & Events Server

//=========================================================================
// OnGetAeBaseServer
// -----------------------
//    Returns a pointer to the Event Server Handler instance.
//=========================================================================
AeBaseServer* OnGetAeBaseServer()
{
	return gpEventServer;
}
#endif

