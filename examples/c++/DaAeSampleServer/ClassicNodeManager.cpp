/*
 * Copyright (c) 2011-2022 Technosoftware GmbH. All rights reserved
 * Web: http://www.technosoftware.com 
 * 
 * License:
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:

 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * SPDX-License-Identifier: MIT
 */

//DOM-IGNORE-BEGIN
//-----------------------------------------------------------------------------
// INCLUDES
//-----------------------------------------------------------------------------
#include "stdafx.h"
#include <windows.h>
#include <comdef.h>										// For _variant_t and _bstr_t
#include <crtdbg.h>										// For _ASSERTE
#include <process.h>
#include <math.h>										// only for calculation of data simulation values
#include "IClassicBaseNodeManager.h"
#include "ClassicNodeManager.h"

using namespace IClassicBaseNodeManager;

//-----------------------------------------------------------------------------
// MACROS
//-----------------------------------------------------------------------------
#define CHECK_PTR(p) {if ((p)== NULL) throw E_OUTOFMEMORY;}
#define CHECK_ADD(f) {if (!(f)) throw E_OUTOFMEMORY;}
#define CHECK_BOOL(f) {if (!(f)) throw E_FAIL;}
#define CHECK_RESULT(f) {HRESULT hres = f; if (FAILED( hres )) throw hres;}

//-----------------------------------------------------------------------------
// GENERAL DEFINITIONS
//-----------------------------------------------------------------------------
//    Reserved Area IDs
#define AREAID_ROOT						(0xFFFFFFFE)
#define AREAID_UNSPECIFIED				(0xFFFFFFFD)

//
// ----- BEGIN SAMPLE IMPLEMENTATION ------------------------------------------
//

//-----------------------------------------------------------------------------
// Event Category IDs
//-----------------------------------------------------------------------------
// Simple Event Category IDs
#define CATID_DEVFAILURE				0x100
#define CATID_SYSMESSAGE				0x101

// Tracking Event Category IDs   
#define CATID_SYSCONFIG					0x200
#define CATID_ADVCONTROL				0x201

// Condition Event Category IDs  
#define CATID_LEVEL						0x300
#define CATID_SYSFAIL					0x301                                    

//-----------------------------------------------------------------------------
// Attribute IDs                       
//-----------------------------------------------------------------------------
#define ATTRID_LEVEL_CV					0x400
#define ATTRID_DEVFAILURE_ERRORCODE		0x401
#define ATTRID_DEVFAILURE_DEVICENAME	0x402
#define ATTRID_SYSCONFIG_PREVVALUE		0x403
#define ATTRID_SYSCONFIG_NEWVALUE		0x404
#define ATTRID_ADVCONTROL_PREVVALUE		0x405
#define ATTRID_ADVCONTROL_NEWVALUE		0x406                                     

//-----------------------------------------------------------------------------
// Condition Definition IDs            
//-----------------------------------------------------------------------------
#define CONDDEFID_PVLEVEL_RAMP			0x500
#define CONDDEFID_PVLEVEL_FURNACE		0x501
#define CONDDEFID_HILEVEL_TANK			0x502
#define CONDDEFID_HILEVEL_HEATING		0x503
#define CONDDEFID_SYSFAIL_TEMP			0x504
#define CONDDEFID_SYSFAIL_HUMI			0x505

//-----------------------------------------------------------------------------
// Sub-Condition Definition IDs
//-----------------------------------------------------------------------------
#define SUBCONDDEFID_LO_LO_RAMP			0x550
#define SUBCONDDEFID_LO_RAMP			0x551
#define SUBCONDDEFID_HI_RAMP			0x552
#define SUBCONDDEFID_HI_HI_RAMP			0x553

#define SUBCONDDEFID_LO_FURNACE			0x554
#define SUBCONDDEFID_HI_FURNACE			0x555

//-----------------------------------------------------------------------------
// Area IDs
//-----------------------------------------------------------------------------
#define AREAID_NORTH					0x600
#define AREAID_NORTH_DEV1				0x601
#define AREAID_SOUTH					0x602
#define AREAID_SOUTH_DEV1				0x603                                    

//-----------------------------------------------------------------------------
// Source IDs                          
//-----------------------------------------------------------------------------
#define SRCID_SYSTEM					0x700
#define SRCID_NETADAPT					0x701
#define SRCID_SERPORT					0x702
#define SRCID_VALVE						0x703
#define SRCID_MOTOR						0x704
#define SRCID_TANK_1					0x705
#define SRCID_TANK_2					0x706
#define SRCID_HEATING_1					0x707
#define SRCID_HEATING_2					0x708
#define SRCID_MULTISRC					0x709                                      

//-----------------------------------------------------------------------------
// Event Condition IDs                 
//-----------------------------------------------------------------------------
#define CONDID_TANK_1_OVERFLOW			0x800
#define CONDID_TANK_2_OVERFLOW			0x801
#define CONDID_HEATING_1_EXTEMP			0x802
#define CONDID_HEATING_2_EXTEMP			0x803
#define CONDID_WATER_LEVEL				0x804

//-----------------------------------------------------------------------------
// DATA ACCESS ITEM DEFINITIONS
//-----------------------------------------------------------------------------
// ItemIDs of special handled items
#define ITEMID_SPECIAL_PROPERTIES		L"CTT.SpecialItems.WithVendorSpecificProperties"
#define ITEMID_SPECIAL_EU				L"CTT.SpecialItems.WithAnalogEUInfo"
#define ITEMID_SPECIAL_EU2				L"CTT.SpecialItems.WithAnalogEUInfo2"
#define ITEMID_SPECIAL_EU_ENUM			L"CTT.SpecialItems.WithEnumeratedEUInfo"
#define ITEMID_SPECIAL_EU_ENUM2			L"CTT.SpecialItems.WithEnumeratedEUInfo2"

// Valid ranges for the windows data type DATE
#ifndef MIN_DATE
#define MIN_DATE        (-657434L)						// represents 1/1/100
#endif
#ifndef MAX_DATE
#define MAX_DATE        2958465L						// represents 12/31/9999
#endif
// Used maximum date in this sample application
#define MAX_DATE_SAMPLEAPP 73050L						// represents 12/31/2099

#define PROPID_CASING_MATERIAL  5650
#define PROPID_CASING_HEIGHT  5651
#define PROPID_CASING_MANUFACTURER  5652

//-----------------------------------------------------------------------------
// FORWARD DECLARATIONS
//-----------------------------------------------------------------------------
unsigned __stdcall RefreshThread(LPVOID pAttr);
unsigned __stdcall ConfigThread(LPVOID pAttr);
HRESULT KillThreads(void);
void __stdcall ToggleTank1Cond();

//-----------------------------------------------------------------------------
// DATA                                                                  SAMPLE
//-----------------------------------------------------------------------------
// The handles of items with simulated data
void* gDeviceItem_NumberItems = NULL;
void* gDeviceItem_SimRamp = NULL;
void* gDeviceItem_SimSine = NULL;
void* gDeviceItem_SimRandom = NULL;
void* gDeviceItem_RequestShutdownCommand = NULL;
void* gDeviceItem_SpecialEU = NULL;
void* gDeviceItem_SpecialEU2 = NULL;
void* gDeviceItem_SpecialProperties = NULL;

// Handle of the Config Thread
HANDLE               m_hConfigThread;
// Handle of the Refresh Thread
HANDLE               m_hUpdateThread;
// Signal-Handle to terminate the threads.
HANDLE               m_hTerminateThreadsEvent;

ServerState  gServerState = ServerState::NoConfig;

DWORD gNumberItems = 0;

//-----------------------------------------------------------------------------
// CLASS DataSimulation                                                 SAMPLE
//-----------------------------------------------------------------------------
class DataSimulation
{
public:
    DataSimulation()
    {
        m_lCount = 0;
        m_lIntervalCount = 0;
        m_lRamp = 0;
        m_dblSine = 0.0;
        m_lRandom = 0;
    }

    ~DataSimulation() {}

    // Attributes
    long     RampValue() { return m_lRamp; }
    double   SineValue() { return m_dblSine; }
    long     RandomValue() { return m_lRandom; }

    // Operations
    void CalculateNewData()
    {
        if (++m_lIntervalCount >= 1000 / UPDATE_PERIOD) {
            m_lIntervalCount = 0;
            m_lCount++;

            m_lRamp = (++m_lRamp > 100) ? 0 : m_lRamp;
            m_dblSine = sin((m_lCount % 40) * 0.1570796327);
            m_lRandom = rand();
        }
    }

    // Implementation
protected:
    long     m_lCount;
    long     m_lIntervalCount;

    long     m_lRamp;
    double   m_dblSine;
    long     m_lRandom;
};


DataSimulation gDataSimulation;

//-----------------------------------------------------------------------------
// ToggleTank1Cond														 SAMPLE
// ---------------
//    Each call of this function changes the active state of the
//    condition specified by ID CONDID_TANK_1_OVERFLOW.
//    This function also sets the required attribute valus and uses
//    an own message which is used instead of the default message.
//-----------------------------------------------------------------------------
void __stdcall ToggleTank1Cond()
{
    static      BOOL fActive = FALSE;
    _variant_t  devfailattrs[1];

    AeConditionState cs;

    cs.CondID() = CONDID_TANK_1_OVERFLOW;
    cs.SubCondID() = 0;
    cs.ActiveState() = false;
    cs.Quality() = OPC_QUALITY_GOOD;
    cs.AttrCount() = 1;
    cs.AttrValuesPtr() = devfailattrs;

    if (fActive) {
        fActive = FALSE;
        cs.Message() = L"Normal State";					// Own message
        devfailattrs[0] = (long)80;						// Current Value
    }
    else {
        fActive = TRUE;
        cs.Message() = L"Overflow";						// Own message
        devfailattrs[0] = (long)123;					// Current Value
    }
    cs.ActiveState() = fActive;							// Set current active state
    // Process the new state
    ProcessConditionStateChanges(1, &cs);
}



//-----------------------------------------------------------------------------
// ToggleRampCond														 SAMPLE
// --------------
//    Each call of this function changes the active sub condition
//    of the condition specified by ID CONDID_WATER_LEVEL.
//    This function also sets the required attribute values.
//    If the ID of the active sub condition is SUBCONDDEFID_HI_RAMP then
//    an own acknowledge resuest flag is used instead of the default flag.
//-----------------------------------------------------------------------------
static void ToggleRampCond()
{
    static long lVal = 50;
    _variant_t  devfailattrs[1];

    AeConditionState cs;

    cs.CondID() = CONDID_WATER_LEVEL;
    cs.SubCondID() = 0;
    cs.ActiveState() = true;
    cs.Quality() = OPC_QUALITY_GOOD;
    cs.AttrCount() = 1;
    cs.AttrValuesPtr() = devfailattrs;

    switch (lVal) {
    case 10: lVal = 20;  cs.SubCondID() = SUBCONDDEFID_LO_RAMP;
        break;
    case 20: lVal = 50;  cs.ActiveState() = FALSE;
        break;
    case 50: lVal = 80;  cs.SubCondID() = SUBCONDDEFID_HI_RAMP;
    { BOOL f = TRUE; cs.AckRequiredPtr() = &f; }
    break;
    case 80: lVal = 90;  cs.SubCondID() = SUBCONDDEFID_HI_HI_RAMP;
        break;
    case 90: lVal = 10;  cs.SubCondID() = SUBCONDDEFID_LO_LO_RAMP;
        break;
    }
    devfailattrs[0] = (long)lVal;						// Set required attribute

    ProcessConditionStateChanges(1, &cs);
}


//-----------------------------------------------------------------------------
// Heating1Condition														 SAMPLE
// -------------
//    This is a wrapper class for the condition with the ID
//    CONDID_HEATING_1_EXTEMP.
//    Each call of the function ToggleCondition() changes the active
//    state of the condition  and sets the required attribute
//    valus. Also a own message which is used instead of the default
//    message.
//-----------------------------------------------------------------------------
class Heating1Condition
{
public:
    Heating1Condition()
    {
        cs.CondID() = CONDID_HEATING_1_EXTEMP;
        cs.Quality() = OPC_QUALITY_GOOD;
        cs.ActiveState() = FALSE;
        cs.AttrCount() = 1;
        cs.AttrValuesPtr() = devfailattrs;
    };

    ~Heating1Condition() {};

    void ToggleCondition()
    {
        if (cs.ActiveState()) {
            cs.ActiveState() = FALSE;
            // Own message
            cs.Message() = L"Normal Temperature";
            devfailattrs[0] = (long)35;					// Current Value
        }
        else {
            cs.ActiveState() = TRUE;
            // Own message
            cs.Message() = L"Excess Temperature";
            devfailattrs[0] = (long)55;					// Current Value
        }

        ProcessConditionStateChanges(1, &cs);
    }

protected:
    AeConditionState	cs;
    _variant_t			devfailattrs[1];
};


//-----------------------------------------------------------------------------
// Update Thread														 SAMPLE
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
//-----------------------------------------------------------------------------
unsigned __stdcall RefreshThread(LPVOID pAttr)
{

    FILETIME	TimeStamp;
    VARIANT     Value;

    DWORD               dwCount = 0;							// Counter for simulation
    _variant_t          devfailattrs[2];
    Heating1Condition   condHeating1;

	CoFileTimeNow(&TimeStamp);
	
	// Keep this thread running until the Terminate Event is received

    for (;;) {											// Thread Loop

        if (gDeviceItem_NumberItems != NULL) {
            V_I4(&Value) = gNumberItems;
            V_VT(&Value) = VT_I4;

            SetItemValue(gDeviceItem_NumberItems, &Value, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
        }

        if (gServerState == ServerState::Running) {

            /// TODO: Remove sample code later
            //GetClients(&numClientHandles, &clientHandles, &clientNames);
            //if (numClientHandles > 0) {
            //    for (int i = 0; i < numClientHandles; i++) {
            //        if (clientHandles[i] == nullptr) {
            //            continue;				    // Handle next client
            //        }
            //        if (clientNames[i] != nullptr) {
            //            delete clientNames[i];
            //        }
            //        GetGroups(clientHandles[i], &numGroupHandles, &groupHandles, &groupNames);
            //        if (numGroupHandles > 0) {
            //            if (groupHandles[i] == nullptr) {
            //                continue;				    // Handle next group
            //            }
            //            DaGroupState groupInfo;
            //            GetGroupState(groupHandles[i], &groupInfo);
            //            if (groupHandles[i] != nullptr) {
            //                delete groupNames[i];
            //            }
            //        }
            //    } // handle all clients
            //    delete clientHandles;
            //}

            //GetActiveItems(&numClientHandles, &clientHandles);
            //if (numClientHandles > 0) {
            //    for (DWORD i = 0; i < numClientHandles; i++) {
            //        if (clientHandles[i] == nullptr) {
            //            continue;				    // Handle next client
            //        }
            //    } // handle all clients
            //    delete clientHandles;
            //}
            
            gDataSimulation.CalculateNewData();

            CoFileTimeNow(&TimeStamp);

            ++dwCount;

            if ((dwCount % 2) == 0) ToggleTank1Cond();	// every 2s
            if ((dwCount % 3) == 0) ToggleRampCond();	// every 3s
            if ((dwCount % 5) == 0) {					// every 5s
                condHeating1.ToggleCondition();
            }
            if ((dwCount % 120) == 0) {					// every 2 min.
                devfailattrs[0] = (long)WSAENETDOWN;                        // Error Code
                devfailattrs[1] = L"3Com EtherLink XL NIC (3C900B-COMBO)";  // Device Name
                ProcessSimpleEvent(CATID_DEVFAILURE, SRCID_NETADAPT, L"No response", 800, 2, devfailattrs, &TimeStamp);
            }

            // update server cache for this item
            V_I4(&Value) = gDataSimulation.RampValue();
            V_VT(&Value) = VT_I4;

            SetItemValue(gDeviceItem_SimRamp, &Value, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);

            V_R8(&Value) = gDataSimulation.SineValue();
            V_VT(&Value) = VT_R8;

            SetItemValue(gDeviceItem_SimSine, &Value, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);

            V_I4(&Value) = gDataSimulation.RandomValue();
            V_VT(&Value) = VT_I4;

            SetItemValue(gDeviceItem_SimRandom, &Value, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);

        }

        if (WaitForSingleObject(m_hTerminateThreadsEvent,
            1000) != WAIT_TIMEOUT) {
            break;										// Terminate Thread
        }
    }													// Thread Loop

    _endthreadex(0);									// The thread terminates.
    return 0;

} // RefreshThread

//=============================================================================
// KillThreads                                                         INTERNAL
// -----------
//    Kills the Config and Update Thread. This function is used during shutdown
//    of the server.
//=============================================================================
HRESULT KillThreads(void)
{
    if (m_hTerminateThreadsEvent == NULL) {
        return S_OK;
    }

    SetEvent(m_hTerminateThreadsEvent);				// Set the signal to shutdown the threads.

    if (m_hConfigThread) {
        // Wait max 10 secs until the config thread has terminated.
        if (WaitForSingleObject(m_hConfigThread, 10000) == WAIT_TIMEOUT) {
            TerminateThread(m_hConfigThread, 1);
        }
        CloseHandle(m_hConfigThread);
        m_hConfigThread = NULL;
    }
    if (m_hUpdateThread) {
        // Wait max 30 secs until the update thread has terminated.
        if (WaitForSingleObject(m_hUpdateThread, 30000) == WAIT_TIMEOUT) {
            TerminateThread(m_hUpdateThread, 1);
        }
        CloseHandle(m_hUpdateThread);
        m_hUpdateThread = NULL;
    }

    CloseHandle(m_hTerminateThreadsEvent);
    m_hTerminateThreadsEvent = NULL;

    return S_OK;
}

//-----------------------------------------------------------------------------
// CreateSampleVariant													 SAMPLE
// -------------------
//    Utility function to create the sample values for the server items.
//-----------------------------------------------------------------------------
#pragma warning( disable:4290 )
static void CreateSampleVariant(
    /* [in]  */ VARTYPE     vt,
    /* [out] */ LPVARIANT   pvVal) throw (HRESULT)
{
    try {
        if (vt & VT_ARRAY) {
            //
            // Array Type
            //
            vt = vt & VT_TYPEMASK;                  // Remove array flag
            BSTR bstr = NULL;
            if (vt == VT_BSTR) {
                CHECK_PTR(bstr = SysAllocString(L"This is string #n"))
            }


            SAFEARRAYBOUND rgs;
            rgs.cElements = 4;
            rgs.lLbound = 0;

            V_VT(pvVal) = VT_ARRAY | vt;
            CHECK_PTR(V_ARRAY(pvVal) = SafeArrayCreate(vt, 1, &rgs))

                for (long i = 0; i < (long)rgs.cElements; i++) {
                    switch (vt) {
                    case VT_BOOL:
                    {
                        int b = (i & 1) ? VARIANT_TRUE : VARIANT_FALSE;
                        CHECK_RESULT(SafeArrayPutElement(V_ARRAY(pvVal), &i, &b))
                    }
                    break;

                    case VT_I1:
                    case VT_I2:
                    case VT_I4:
                    case VT_UI1:
                    case VT_UI2:
                    case VT_UI4:
                    case VT_CY:
                    case VT_R4:
                    case VT_R8:
                    {
                        LONGLONG ll = rand();
                        CHECK_RESULT(SafeArrayPutElement(V_ARRAY(pvVal), &i, &ll))
                    }
                    break;

                    case VT_DATE:
                    {
                        // Note: Valid date range must be between MIN_DAT and MAX_DATE.
                        // For the sample implementation we use a limited range
                        // between 30/12/1899 and 12/31/2099.
                        DATE date = rand() & MAX_DATE_SAMPLEAPP;
                        CHECK_RESULT(SafeArrayPutElement(V_ARRAY(pvVal), &i, &date))
                    }
                    break;


                    case VT_BSTR:
                    {
                        bstr[16] = (WCHAR)('0' + i + 1);
                        HRESULT hr = SafeArrayPutElement(V_ARRAY(pvVal), &i, bstr);
                        if (FAILED(hr)) {
                            SysFreeString(bstr);
                            throw hr;
                        }
                    }
                    break;


                    default: _ASSERTE(0);			// not supported type                                      
                    }
                }

            if (bstr) {
                SysFreeString(bstr);
            }
        }
        else {
            //
            // Simple Type
            //
            V_VT(pvVal) = vt;

            switch (vt) {
            case VT_I1:    V_I1(pvVal) = 76;
                break;
            case VT_UI1:   V_UI1(pvVal) = 23;
                break;
            case VT_I2:    V_I2(pvVal) = 345;
                break;
            case VT_UI2:   V_UI2(pvVal) = 39874;
                break;
            case VT_I4:    V_I4(pvVal) = 20196;
                break;
            case VT_UI4:   V_UI4(pvVal) = 4230498;
                break;
            case VT_R4:    V_R4(pvVal) = (float)8.123242;
                break;
            case VT_R8:    V_R8(pvVal) = 83289.48243;
                break;
            case VT_CY:    V_CY(pvVal).int64 = 198000;
                break;
            case VT_DATE:  V_DATE(pvVal) = 2.5;     // Noon, January 1, 1900.
                break;
            case VT_BOOL:  V_BOOL(pvVal) = VARIANT_FALSE;
                break;
            case VT_BSTR:  V_BSTR(pvVal) = SysAllocString(L"-- It's a nice day --");
                CHECK_PTR(V_BSTR(pvVal))
                    break;
            default:       _ASSERTE(0);				// not supported type
            }
        }
    }
    catch (HRESULT hrEx) {
        VariantClear(pvVal);
        throw hrEx;
    }
    catch (...) {
        VariantClear(pvVal);
        throw E_FAIL;
    }
}


//-----------------------------------------------------------------------------
// Config Thread														 SAMPLE
// -------------
// 
//-----------------------------------------------------------------------------
unsigned __stdcall ConfigThread(LPVOID pAttr)
{
    HRESULT     hr = S_OK;
    VARIANT     varVal;
    DWORD       dwCount = 0;
    FILETIME	TimeStamp;
    void*		deviceItemHandle = NULL;

    gServerState = ServerState::NoConfig;
    SetServerState(gServerState);

    try {
        // Order of required steps for Alarms&Events Sample
        // ------------------------------------------------

        // 1) Define the Event Categories
        // 2) Add the Attributes to the Event Categories if desired
        // 3) Specify the Condition Definitions
        // 4) Specify the Sub-Condition Definitions
        // 5) Define the Process Areas (is optional)
        // 6) Define the Event Sources
        // 7) Define the Event Conditions

        // 1) Define the Event Categories
        /////////////////////////////////
        CHECK_RESULT(AddSimpleEventCategory(CATID_DEVFAILURE, L"Device Failure"))
            CHECK_RESULT(AddSimpleEventCategory(CATID_SYSMESSAGE, L"System Message"))

            CHECK_RESULT(AddTrackingEventCategory(CATID_SYSCONFIG, L"System Configuration"))
            CHECK_RESULT(AddTrackingEventCategory(CATID_ADVCONTROL, L"Advanced Control"))

            CHECK_RESULT(AddConditionEventCategory(CATID_LEVEL, L"Level"))
            CHECK_RESULT(AddConditionEventCategory(CATID_SYSFAIL, L"System Failure"))

            // 2) Add the Attributes to the Event Categories
            ////////////////////////////////////////////////
            CHECK_RESULT(AddEventAttribute(CATID_LEVEL, ATTRID_LEVEL_CV, L"Current Value", VT_I4))
            CHECK_RESULT(AddEventAttribute(CATID_DEVFAILURE, ATTRID_DEVFAILURE_ERRORCODE, L"Error Code", VT_I4))
            CHECK_RESULT(AddEventAttribute(CATID_DEVFAILURE, ATTRID_DEVFAILURE_DEVICENAME, L"Device Name", VT_BSTR))
            CHECK_RESULT(AddEventAttribute(CATID_SYSCONFIG, ATTRID_SYSCONFIG_PREVVALUE, L"Prev Value", VT_I4))
            CHECK_RESULT(AddEventAttribute(CATID_SYSCONFIG, ATTRID_SYSCONFIG_NEWVALUE, L"New Value", VT_I4))
            CHECK_RESULT(AddEventAttribute(CATID_ADVCONTROL, ATTRID_ADVCONTROL_PREVVALUE, L"Prev Value", VT_I4))
            CHECK_RESULT(AddEventAttribute(CATID_ADVCONTROL, ATTRID_ADVCONTROL_NEWVALUE, L"New Value", VT_I4))

            // 3) Specify the Condition Definitions
            ///////////////////////////////////////
            CHECK_RESULT(AddSingleStateConditionDefinition(CATID_SYSFAIL, CONDDEFID_SYSFAIL_TEMP,
                L"SYSTEM_FAILURE Temperature", L"temp > 100°C", 100, L"Excess Temperature", FALSE))
            CHECK_RESULT(AddSingleStateConditionDefinition(CATID_SYSFAIL, CONDDEFID_SYSFAIL_HUMI,
                L"SYSTEM_FAILURE Humidity", L"humidity > 80%", 100, L"Humidity too high", FALSE))
            CHECK_RESULT(AddSingleStateConditionDefinition(CATID_LEVEL, CONDDEFID_HILEVEL_TANK,
                L"HI Tank", L"level > 80", 100, L"Overflow", TRUE))
            CHECK_RESULT(AddSingleStateConditionDefinition(CATID_LEVEL, CONDDEFID_HILEVEL_HEATING,
                L"HI Heating Temperature", L"temp > 35", 100, L"Excess Temperature", FALSE))
            CHECK_RESULT(AddMultiStateConditionDefinition(CATID_LEVEL, CONDDEFID_PVLEVEL_RAMP, L"PVLEVEL Ramp"))
            CHECK_RESULT(AddMultiStateConditionDefinition(CATID_LEVEL, CONDDEFID_PVLEVEL_FURNACE, L"PVLEVEL Furnace"))

            // 4) Specify the Sub-Condition Definitions
            ///////////////////////////////////////////
            CHECK_RESULT(AddSubConditionDefinition(CONDDEFID_PVLEVEL_RAMP, SUBCONDDEFID_LO_LO_RAMP,
                L"LO_LO", L"Ramp < 15", 400, L"Low Low Alarm", FALSE))
            CHECK_RESULT(AddSubConditionDefinition(CONDDEFID_PVLEVEL_RAMP, SUBCONDDEFID_LO_RAMP,
                L"LO", L"Ramp < 25", 100, L"Low Alarm", FALSE))
            CHECK_RESULT(AddSubConditionDefinition(CONDDEFID_PVLEVEL_RAMP, SUBCONDDEFID_HI_RAMP,
                L"HI", L"Ramp > 75", 100, L"High Alarm", FALSE))
            CHECK_RESULT(AddSubConditionDefinition(CONDDEFID_PVLEVEL_RAMP, SUBCONDDEFID_HI_HI_RAMP,
                L"HI_HI", L"Ramp > 85", 400, L"High High Alarm", FALSE))
            CHECK_RESULT(AddSubConditionDefinition(CONDDEFID_PVLEVEL_FURNACE, SUBCONDDEFID_LO_FURNACE,
                L"LO", L"Temp < 500", 100, L"Low Alarm", FALSE))
            CHECK_RESULT(AddSubConditionDefinition(CONDDEFID_PVLEVEL_FURNACE, SUBCONDDEFID_HI_FURNACE,
                L"HI", L"Temp > 800", 100, L"High Alarm", FALSE))

            // 5) Define the Process Areas
            //////////////////////////////
            CHECK_RESULT(AddArea(AREAID_ROOT, AREAID_NORTH, L"PlantNorth"))
            CHECK_RESULT(AddArea(AREAID_NORTH, AREAID_NORTH_DEV1, L"Device1"))
            CHECK_RESULT(AddArea(AREAID_ROOT, AREAID_SOUTH, L"PlantSouth"))
            CHECK_RESULT(AddArea(AREAID_SOUTH, AREAID_SOUTH_DEV1, L"Device1"))

            // 6) Define the Event Sources
            //////////////////////////////
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_NETADAPT, L"Network Adapter", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_SERPORT, L"Serial Port", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_SYSTEM, L"System", false))
            CHECK_RESULT(AddSource(AREAID_NORTH_DEV1, SRCID_VALVE, L"Valve", false))
            CHECK_RESULT(AddSource(AREAID_SOUTH_DEV1, SRCID_MOTOR, L"Motor", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_TANK_1, L"Level Sensor Tank 1", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_TANK_2, L"Level Sensor Tank 2", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_HEATING_1, L"Heating 1", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_HEATING_2, L"Heating 2", false))
            CHECK_RESULT(AddSource(AREAID_ROOT, SRCID_MULTISRC, L"Multiple Used Source", true))
            CHECK_RESULT(AddExistingSource(AREAID_NORTH_DEV1, SRCID_MULTISRC))
            CHECK_RESULT(AddExistingSource(AREAID_SOUTH_DEV1, SRCID_MULTISRC))

            // 7) Define the Event Conditions
            /////////////////////////////////
            CHECK_RESULT(AddCondition(SRCID_MULTISRC, CONDDEFID_HILEVEL_TANK, CONDID_TANK_1_OVERFLOW))
            CHECK_RESULT(AddCondition(SRCID_TANK_2, CONDDEFID_HILEVEL_TANK, CONDID_TANK_2_OVERFLOW))
            CHECK_RESULT(AddCondition(SRCID_HEATING_1, CONDDEFID_HILEVEL_HEATING, CONDID_HEATING_1_EXTEMP))
            CHECK_RESULT(AddCondition(SRCID_HEATING_2, CONDDEFID_HILEVEL_HEATING, CONDID_HEATING_2_EXTEMP))
            CHECK_RESULT(AddCondition(SRCID_TANK_1, CONDDEFID_PVLEVEL_RAMP, CONDID_WATER_LEVEL))
    }
    catch (HRESULT hresEx) {
        hr = hresEx;
    }

    // Data Access Sample
    // ------------------

    struct
    {
        PWCHAR   pwszItemID;
        VARTYPE  vt;

    } arItemTypes[] = {
        { L"Boolean", VT_BOOL },
        { L"Short", VT_I2 },
        { L"Integer", VT_I4 },
        { L"SingleFloat", VT_R4 },
        { L"DoubleFloat", VT_R8 },
        { L"Date", VT_DATE },
        { L"String", VT_BSTR },
        { L"Byte", VT_UI1 },
        { L"Character", VT_I1 },
        { L"Word", VT_UI2 },
        { L"DoubleWord", VT_UI4 },
        { L"Currency", VT_CY },
        { NULL, VT_EMPTY }
    };

    struct
    {
        PWCHAR			pwszBranch;
        DaAccessRights  dwAccessRights;
        DWORD			dwSignalTypeMask;

    } arIOTypes[] = {
        { L"In.", Readable, SIGMASK_INTERN_IN },
        { L"Out.", Writable, SIGMASK_INTERN_OUT },
        { L"InOut.", ReadWritable, SIGMASK_INTERN_INOUT },
        { NULL, NotKnown }
    };

    VariantInit(&varVal);
    try {

        DWORD i = 0, z = 0;

        // ---------------------------------------------------------------------
        // SimpleTypes In/Out/InOut
        // ---------------------------------------------------------------------

        char szMsg[80];


        // ---------------------------------------------------------------------
        // Simulated Data
        // ---------------------------------------------------------------------

        // SimulatedData.NumberItems
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_I4;							// Canonical data type
        V_I4(&varVal) = 0;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddItem(
            L"SimulatedData.NumberItems",				// ItemID
            Readable,									// DaAccessRights
            &varVal,									// Data Type and Initial Value
            &gDeviceItem_NumberItems))					// It's an item with simulated data               
            gNumberItems++;

        // SimulatedData.Ramp
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_I4;							// Canonical data type
        V_I4(&varVal) = 0;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddItem(
            L"SimulatedData.Ramp",						// ItemID
            Readable,									// DaAccessRights
            &varVal,									// Data Type and Initial Value
            &gDeviceItem_SimRamp))						// It's an item with simulated data               
            gNumberItems++;

        // SimulatedData.Sine
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_R8;							// canonical data type
        V_R8(&varVal) = 0.0;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddItem(
            L"SimulatedData.Sine",						// ItemID
            Readable,									// DaAccessRights
            &varVal,									// Data Type and Initial Value
            &gDeviceItem_SimSine))						// It's an item with simulated data               
            gNumberItems++;

        // SimulatedData.Random
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_I4;							// canonical data type
        V_I4(&varVal) = 0;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddItem(
            L"SimulatedData.Random",					// ItemID
            Readable,									// DaAccessRights
            &varVal,			  						// Data Type and Initial Value
            &gDeviceItem_SimRandom))					// It's an item with simulated data               
            gNumberItems++;

        // Commands.RequestShutdown
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_BSTR;						// canonical data type
        V_BSTR(&varVal) = SysAllocString(L" ");
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddItem(
            L"Commands.RequestShutdown",				// ItemID
            ReadWritable,								// DaAccessRights
            &varVal, 									// Data Type and Initial Value
            &gDeviceItem_RequestShutdownCommand))		// It's an item with simulated data               
            gNumberItems++;

        // ---------------------------------------------------------------------
        // CTT Data
        // ---------------------------------------------------------------------

        i = 0;
        while (arIOTypes[i].pwszBranch) {
            z = 0;
            while (arItemTypes[z].pwszItemID) {

                CreateSampleVariant(arItemTypes[z].vt, &varVal);

                sprintf(szMsg, "CTT.SimpleTypes.");

                _bstr_t bstrItemID(szMsg);
                bstrItemID += arIOTypes[i].pwszBranch;
                bstrItemID += arItemTypes[z].pwszItemID;
                // Create a new item and add it to the Server Address Space
                CHECK_RESULT(AddItem(
                    bstrItemID,						// ItemID
                    arIOTypes[i].dwAccessRights,	// DaAccessRights
                    &varVal,						// Data Type and Initial Value
                    &deviceItemHandle))				// It's an item with simulated data               
                    CreateSampleVariant(arItemTypes[z].vt, &varVal);
                CoFileTimeNow(&TimeStamp);
                SetItemValue(deviceItemHandle, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
                VariantClear(&varVal);
                z++;
                gNumberItems++;
            }
            i++;
        }


        // ---------------------------------------------------------------------
        // Arrays In/Out/InOut
        // ---------------------------------------------------------------------

        i = 0;
        while (arIOTypes[i].pwszBranch) {
            z = 0;
            while (arItemTypes[z].pwszItemID) {

                CreateSampleVariant(arItemTypes[z].vt | VT_ARRAY, &varVal);

                sprintf(szMsg, "CTT.Arrays.");

                _bstr_t bstrItemID(szMsg);
                bstrItemID += arIOTypes[i].pwszBranch;
                bstrItemID += arItemTypes[z].pwszItemID;
                bstrItemID += L"[]";
                // Create a new item and add it to the Server Address Space
                CHECK_RESULT(AddItem(
                    bstrItemID,							// ItemID
                    arIOTypes[i].dwAccessRights,		// DaAccessRights
                    &varVal,							// Data Type and Initial Value
                    &deviceItemHandle))					// It's an item with simulated data               
                    CreateSampleVariant(arItemTypes[z].vt | VT_ARRAY, &varVal);
                CoFileTimeNow(&TimeStamp);
                SetItemValue(deviceItemHandle, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
                VariantClear(&varVal);
                z++;
                gNumberItems++;
            }
            i++;
        }

        // ---------------------------------------------------------------------
        // Special Items
        // ---------------------------------------------------------------------

        // SpecialItems.WithAnalogEUInfo
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_UI1;							// canonical data type
        V_UI1(&varVal) = 89;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddAnalogItem(
            ITEMID_SPECIAL_EU,							// ItemID
            ReadWritable,								// DaAccessRights
            &varVal,									// Data Type and Initial Value
            40.86,										// Low Limit
            92.67,										// High Limit
            &gDeviceItem_SpecialEU))					// It's an item with simulated data               
            CreateSampleVariant(VT_UI1, &varVal);
        CoFileTimeNow(&TimeStamp);
        SetItemValue(gDeviceItem_SpecialEU, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
        VariantClear(&varVal);
        gNumberItems++;

        // SpecialItems.WithAnalogEUInfo2
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_UI1;							// canonical data type
        V_UI1(&varVal) = 21;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddAnalogItem(
            ITEMID_SPECIAL_EU2,							// ItemID
            ReadWritable,								// DaAccessRights
            &varVal,									// Data Type and Initial Value
            12.50,										// Low Limit
            27.90,										// High Limit
            &gDeviceItem_SpecialEU2))					// It's an item with simulated data               
            CreateSampleVariant(VT_UI1, &varVal);
        CoFileTimeNow(&TimeStamp);
        SetItemValue(gDeviceItem_SpecialEU2, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
        VariantClear(&varVal);

        // Add Custom Property Definitions to the generic server
        V_VT(&varVal) = VT_R8;							// canonical data type
        V_R8(&varVal) = 25.34;
        AddProperty(PROPID_CASING_HEIGHT, L"Casing Height", &varVal);

        V_VT(&varVal) = VT_BSTR;						// canonical data type
        V_BSTR(&varVal) = L"Aluminum";
        AddProperty(PROPID_CASING_MATERIAL, L"Casing Material", &varVal);

        V_VT(&varVal) = VT_BSTR;						// canonical data type
        V_BSTR(&varVal) = L"CBM";
        AddProperty(PROPID_CASING_MANUFACTURER, L"Casing Manufacturer", &varVal);
        gNumberItems++;

        // SpecialItems.WithVendorSpecificProperties
        // ---------------------------------------------------------------------
        V_VT(&varVal) = VT_UI1;							// canonical data type
        V_UI1(&varVal) = 111;
        // Create a new item and add it to the Server Address Space
        CHECK_RESULT(AddItem(
            ITEMID_SPECIAL_PROPERTIES,					// ItemID
            ReadWritable,								// DaAccessRights
            &varVal,									// Data Type and Initial Value
            &gDeviceItem_SpecialProperties))			// It's an item with simulated data               
            CreateSampleVariant(VT_UI1, &varVal);
        CoFileTimeNow(&TimeStamp);
        SetItemValue(gDeviceItem_SpecialProperties, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
        VariantClear(&varVal);
        gNumberItems++;


        int maxLoops = 100;								// Can be increased for performance tests

        for (int y = 0; y < maxLoops; y++) {			// Check all specified items
            i = 0;
            while (arIOTypes[i].pwszBranch) {
                z = 0;
                while (arItemTypes[z].pwszItemID) {

                    CreateSampleVariant(arItemTypes[z].vt, &varVal);

                    if (maxLoops == 1)
                    {
                        sprintf(szMsg, "MassItems.SimpleTypes.");
                    }
                    else
                    {
                        sprintf(szMsg, "MassItems.SimpleTypes[%u].", y);
                    }

                    _bstr_t bstrItemID(szMsg);
                    bstrItemID += arIOTypes[i].pwszBranch;
                    bstrItemID += arItemTypes[z].pwszItemID;
                    // Create a new item and add it to the Server Address Space
                    CHECK_RESULT(AddItem(
                        bstrItemID,						// ItemID
                        arIOTypes[i].dwAccessRights,	// DaAccessRights
                        &varVal,						// Data Type and Initial Value
                        &deviceItemHandle))				// It's an item with simulated data               
                        CreateSampleVariant(arItemTypes[z].vt, &varVal);
                    CoFileTimeNow(&TimeStamp);
                    SetItemValue(deviceItemHandle, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
                    VariantClear(&varVal);
                    z++;
                    gNumberItems++;

                }
                i++;
            }
        }


        // ---------------------------------------------------------------------
        // Arrays In/Out/InOut
        // ---------------------------------------------------------------------

        for (int y = 0; y < maxLoops; y++) {			// Check all specified items
            i = 0;
            while (arIOTypes[i].pwszBranch) {
                z = 0;
                while (arItemTypes[z].pwszItemID) {

                    CreateSampleVariant(arItemTypes[z].vt | VT_ARRAY, &varVal);

                    if (maxLoops == 1)
                    {
                        sprintf(szMsg, "MassItems.Arrays.");
                    }
                    else
                    {
                        sprintf(szMsg, "MassItems.Arrays[%u].", y);
                    }

                    _bstr_t bstrItemID(szMsg);
                    bstrItemID += arIOTypes[i].pwszBranch;
                    bstrItemID += arItemTypes[z].pwszItemID;
                    bstrItemID += L"[]";
                    // Create a new item and add it to the Server Address Space
                    CHECK_RESULT(AddItem(
                        bstrItemID,						// ItemID
                        arIOTypes[i].dwAccessRights,	// DaAccessRights
                        &varVal,						// Data Type and Initial Value
                        &deviceItemHandle))				// It's an item with simulated data               
                        CreateSampleVariant(arItemTypes[z].vt | VT_ARRAY, &varVal);
                    CoFileTimeNow(&TimeStamp);
                    SetItemValue(deviceItemHandle, &varVal, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);
                    VariantClear(&varVal);
                    z++;
                    gNumberItems++;
                }
                i++;
            }
            if (WaitForSingleObject(m_hTerminateThreadsEvent,
                10) != WAIT_TIMEOUT) {
                break;									// Terminate Thread
            }
        }

        gServerState = ServerState::Running;
        SetServerState(gServerState);
        _endthreadex(0);								// The thread terminates.
        return 0;

    }
    catch (HRESULT hresEx) {
        hr = hresEx;
    }
    catch (_com_error &e) {
        hr = e.Error();
    }
    catch (...) {
        hr = E_FAIL;
    }

    gServerState = ServerState::Failed;
    SetServerState(gServerState);
    _endthreadex(0);									// The thread terminates.
    return 0;

} // ConfigThread

//
// ----- END SAMPLE IMPLEMENTATION --------------------------------------------
//


//-----------------------------------------------------------------------------
// IClassicBaseNodeManager FUNCTION IMPLEMENTATION
//-----------------------------------------------------------------------------

HRESULT IClassicBaseNodeManager::OnCreateServerItems()
{
    HRESULT     hr = S_OK;
    DWORD       dwCount = 0;
    DWORD       dwItemHandle = 0;

    //
    // ----- BEGIN SAMPLE IMPLEMENTATION -----
    //

    unsigned uThreadID;									// Thread identifier

    m_hTerminateThreadsEvent = CreateEvent(
        NULL,											// Handle cannot be inherited
        TRUE,											// Manually reset requested
        FALSE,											// Initial state is nonsignaled
        NULL);											// No event object name 

    if (m_hTerminateThreadsEvent == NULL) {				// Cannot create event
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_hConfigThread = (HANDLE)_beginthreadex(
        NULL,											// No thread security attributes
        0,												// Default stack size  
        ConfigThread,									// Pointer to thread function 
        NULL,
        0,												// Run thread immediately
        &uThreadID);									// Thread identifier

    if (m_hConfigThread == 0) {							// Cannot create the thread
        return HRESULT_FROM_WIN32(GetLastError());
    }

    m_hUpdateThread = (HANDLE)_beginthreadex(
        NULL,											// No thread security attributes
        0,												// Default stack size  
        RefreshThread,									// Pointer to thread function 
        NULL,
        0,												// Run thread immediately
        &uThreadID);									// Thread identifier


    if (m_hUpdateThread == 0) {							// Cannot create the thread
        return HRESULT_FROM_WIN32(GetLastError());
    }

    return hr;

    //
    // ----- END SAMPLE IMPLEMENTATION -----
    //
}


DLLEXP ClassicServerDefinition* DLLCALL IClassicBaseNodeManager::OnGetDaServerDefinition(void)
{
    // Server Registry Definitions
    // ---------------------------
    //    Specifies all definitions required to register the server in
    //    the Registry.
    static ClassicServerDefinition  DaServerDefinition = {
        // CLSID of current Server 
        L"{5CEE2576-AA37-4D54-B02D-ECABE09A1C1E}",
        // CLSID of current Server AppId
        L"{1F57EEE2-37BB-4194-91AF-7123676605AA}",
        // Version independent Prog.Id. 
        L"SampleCompany.DaSample",
        // Prog.Id. of current Server
        L"SampleCompany.DaSample.30",
        // Friendly name of server
        L"OPC Server SDK C++ DA Sample Server",
        // Friendly name of current server version
        L"OPC Server SDK C++ DA Sample Server V3.0",
        // Companmy Name
        L"SampleCompany"
    };

    return &DaServerDefinition;
}


DLLEXP ClassicServerDefinition* DLLCALL IClassicBaseNodeManager::OnGetAeServerDefinition(void)
{
    // Server Registry Definitions
    // ---------------------------
    //    Specifies all definitions required to register the server in
    //    the Registry.
    static ClassicServerDefinition  AeServerDefinition = {
        // CLSID of current Server 
        L"{71EFE996-DA6C-4256-8523-230647CFC0D0}",
        // CLSID of current Server AppId
        L"{9572A5BD-D0DF-4D01-8B83-DA555D005C61}",
        // Version independent Prog.Id. (must be same as DA)
        L"SampleCompany.AeSample",
        // Prog.Id. of current Server
        L"SampleCompany.AeSample.30",
        // Friendly name of server
        L"OPC Server SDK C++ AE Sample Server",
        // Friendly name of current server version
        L"OPC Server SDK C++ AE Sample Server V3.0",
        // Companmy Name
        L"SampleCompany"
    };

    return &AeServerDefinition;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnGetDaServerParameters(int* updatePeriod, WCHAR* branchDelimiter, DaBrowseMode* browseMode)
{
    // Data Cache update rate in milliseconds
    *updatePeriod = UPDATE_PERIOD;
    *branchDelimiter = '.';
    *browseMode = Generic;            // browse the generic server address space
    return S_OK;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnGetDaOptimizationParameters(
    bool * useOnRequestItems,
    bool * useOnRefreshItems,
    bool * useOnAddItem,
    bool * useOnRemoveItem)
{
    *useOnRequestItems = true;
    *useOnRefreshItems = true;
    *useOnAddItem = false;
    *useOnRemoveItem = false;

    return S_OK;
}


DLLEXP void DLLCALL IClassicBaseNodeManager::OnStartupSignal(char* commandLine)
{
    // no action required in the default implementation
}

DLLEXP void DLLCALL IClassicBaseNodeManager::OnShutdownSignal()
{
    KillThreads();
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnQueryProperties(
    void*   deviceItemHandle,
    int*	noProp,
    int**	ids)

{
    if (deviceItemHandle == gDeviceItem_SpecialProperties)
    {
        // item has custom properties
        *noProp = 3;
        int *propIDs = new int[*noProp];
        propIDs[0] = PROPID_CASING_MATERIAL;
        propIDs[1] = PROPID_CASING_HEIGHT;
        propIDs[2] = PROPID_CASING_MANUFACTURER;
        *ids = propIDs;
        return S_OK;
    }
    else if (deviceItemHandle == gDeviceItem_SpecialEU || deviceItemHandle == gDeviceItem_SpecialEU2)
    {
        // item has custom properties
        *noProp = 2;
        int *propIDs = new int[*noProp];
        propIDs[0] = OPC_PROPERTY_HIGH_EU;
        propIDs[1] = OPC_PROPERTY_LOW_EU;
        *ids = propIDs;
        return S_OK;
    }
    else
    {
        // item has no custom properties
        *noProp = 0;
        *ids = NULL;
        return S_FALSE;
    }
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnGetPropertyValue(
    void* deviceItemHandle,
    int propertyId,
    LPVARIANT propertyValue)
{

    if (deviceItemHandle == gDeviceItem_SpecialProperties)
    {
        // Item property is available
        switch (propertyId)
        {
        case PROPID_CASING_HEIGHT:
            V_VT(propertyValue) = VT_R8;
            V_R8(propertyValue) = 25.45;
            break;
        case PROPID_CASING_MATERIAL:
            V_VT(propertyValue) = VT_BSTR;
            V_BSTR(propertyValue) = SysAllocString(L"Aluminum");
            break;
        case PROPID_CASING_MANUFACTURER:
            V_VT(propertyValue) = VT_BSTR;
            V_BSTR(propertyValue) = SysAllocString(L"CBM");
            break;
        default:
            propertyValue = NULL;
            return S_FALSE;
            break;
        }
        return S_OK;
    }
    else
    {
        // Item property is not available
        propertyValue = NULL;
        return S_FALSE; //E_INVALID_PID;
    }
}


//----------------------------------------------------------------------------
// OPC Server SDK C++ API Dynamic address space Handling Methods 
// (Called by the generic server)
//----------------------------------------------------------------------------


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnBrowseChangePosition(
    DaBrowseDirection browseDirection,
    LPCWSTR position,
    LPWSTR * actualPosition)
{
    // not supported in this default implementation
    return E_INVALIDARG;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnBrowseItemIds(
    LPWSTR actualPosition,
    DaBrowseType browseFilterType,
    LPWSTR filterCriteria,
    VARTYPE dataTypeFilter,
    DaAccessRights accessRightsFilter,
    int * noItems,
    LPWSTR ** itemIds)
{
    // not supported in this default implementation
    *noItems = 0;
    *itemIds = NULL;
    return E_INVALIDARG;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnBrowseGetFullItemId(
    LPWSTR actualPosition,
    LPWSTR itemName,
    LPWSTR * fullItemId)
{
    // not supported in this default implementation
    *fullItemId = NULL;
    return E_INVALIDARG;
}


//----------------------------------------------------------------------------
// OPC Server SDK C++ API additional Methods
// (Called by the generic server)
//----------------------------------------------------------------------------

DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnClientConnect()
{
    // client is allowed to connect to server
    return S_OK;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnClientDisconnect()
{
    return S_OK;
}



DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnRefreshItems(
    /* in */       int        numItems,
    /* in */       void    ** itemHandles)
{
    //VARIANT     Value;
    //FILETIME	TimeStamp;

    //
    // ----- BEGIN SAMPLE IMPLEMENTATION -----
    //

    gDataSimulation.CalculateNewData();

    //if (numItems == 0)
    //{
    //	CoFileTimeNow( &TimeStamp );
    //	for(int i=0; i<100000; i++ ) 
    //	{
    //		// update server cache for this item
    //		V_I4( &Value ) = gDataSimulation.RampValue();
    //		V_VT( &Value ) = VT_I4;                 

    //		SetItemValue(gDeviceItem_SimRamp, &Value, (OPC_QUALITY_GOOD | OPC_LIMIT_OK), TimeStamp);

    //	}
    //}

    //
    // ----- END SAMPLE IMPLEMENTATION -----
    //
    return S_OK;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnAddItem(
    /* in */       void*	  deviceItem)
{
    return S_OK;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnRemoveItem(
    /* in */       void*	  deviceItem)
{
    DeleteItem(deviceItem);
    return S_OK;
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnWriteItems(
    int          numItems,
    void      ** deviceItems,
    OPCITEMVQT*  itemVQTs,
    HRESULT   *  errors)
{
    //
    // ----- BEGIN SAMPLE IMPLEMENTATION -----
    //

    for (int i = 0; i < numItems; ++i)              // handle all items
    {
        if (deviceItems[i] == gDeviceItem_RequestShutdownCommand)
        {
            FireShutdownRequest(V_BSTR(&itemVQTs[i].vDataValue));
        }
        errors[i] = S_OK;						// init to S_OK
    }

    //
    // ----- END SAMPLE IMPLEMENTATION -----
    //
    return S_OK;
}


DLLEXP HRESULT DLLCALL  IClassicBaseNodeManager::OnTranslateToItemId(int conditionId, int subConditionId, int attributeId, LPWSTR* itemId, LPWSTR* nodeName, CLSID* clsid)
{
    return S_OK;
}


DLLEXP HRESULT DLLCALL  IClassicBaseNodeManager::OnAckNotification(int conditionId, int subConditionId)
{
    return S_OK;
}


/**
 * @fn  LogLevel OnGetLogLevel();
 *
 * @brief   Gets the logging level to be used.
 *
 * @return  A LogLevel.
 */

DLLEXP LogLevel DLLCALL IClassicBaseNodeManager::OnGetLogLevel( )
{
	return Info;
}

/**
 * @fn  void OnGetLogPath(char * logPath);
 *
 * @brief   Gets the logging pazh to be used.
 *
 * @param [in,out]  logPath    Path to be used for logging.
 */

DLLEXP void DLLCALL IClassicBaseNodeManager::OnGetLogPath(const char * logPath)
{
	logPath = "";
}


DLLEXP HRESULT DLLCALL IClassicBaseNodeManager::OnRequestItems(int numItems, LPWSTR *fullItemIds, VARTYPE *dataTypes)
{
    // no valid item in this default implementation
    return S_FALSE;
}
//DOM-IGNORE-END