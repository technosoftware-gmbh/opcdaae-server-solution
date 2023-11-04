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

#ifndef __HIERARCHICALSAS_H
#define __HIERARCHICALSAS_H

#if _MSC_VER >= 1000
#pragma once
#endif // _MSC_VER >= 1000

#include "WideString.h"
#include "UtilityDefs.h"
#include <atlcoll.h>


class DaDeviceItem;
class DaLeaf;

//-----------------------------------------------------------------------------
// TEMPLATE LPCWSTRRefElementTraits
//-----------------------------------------------------------------------------
// Used by the ATL Maps for branches and leaves
template< typename T >
class LPCWSTRRefElementTraits : public CElementTraitsBase< T >
{
public:
   static ULONG Hash( INARGTYPE str )
      {
         const WCHAR* pch = str;
         #ifdef ATLENSURE
            ATLENSURE( pch != NULL );
         #else
            if (!pch) AtlThrow( E_FAIL );
         #endif

         ULONG nHash = 0;

         while( *pch != 0 ) {
            nHash = (nHash<<5)+nHash+(*pch);
            pch++;
         }
         return( nHash );
      }

   static bool CompareElements( INARGTYPE element1, INARGTYPE element2 ) throw()
      {
         return (wcscmp( element1, element2 ) == 0) ? true : false;
      }

   static int CompareElementsOrdered( INARGTYPE str1, INARGTYPE str2 ) throw()
      {
         return wcscmp( str1, str2 );
      }
};


//-----------------------------------------------------------------------------
// CLASS DaBranch
//-----------------------------------------------------------------------------
class DaBranch
{
// Construction / Destruction
public:
   DaBranch();
   HRESULT CreateAsRoot();
   ~DaBranch();

// Operations
public:
   static void SetDelimiter( WCHAR wc ) {
      m_szDelimiter[0] = wc;
   }
   HRESULT  AddBranch( LPCWSTR szBranchName, DaBranch** ppBranch );
   HRESULT  AddLeaf( LPCWSTR szLeafName, DaDeviceItem* pDItem );
   HRESULT  AddDeviceItem( LPCWSTR szSASName, DaDeviceItem* pDItem );
   HRESULT  FindDeviceItem( LPCWSTR szItemID, DaDeviceItem** ppDItem );

   HRESULT  ChangeBrowsePosition( OPCBROWSEDIRECTION dwBrowseDirection, LPCWSTR szPosition, DaBranch** ppNewPos );
   HRESULT  BrowseBranches( LPCWSTR szFilterCriteria, LPDWORD pdwNumOfBranches, BSTR** ppszBranches );

   HRESULT  BrowseLeafs( LPCWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter,
                         LPDWORD pdwNumOfLeafs, BSTR** ppszLeafs );

   HRESULT  BrowseFlat( LPCWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter,
                        LPDWORD pdwNumOfLeafs, BSTR** ppszLeafs );

   HRESULT  GetFullyQualifiedName( LPCWSTR szName, BSTR* pszFullyQualifiedName );
   void     RemoveAll( BOOL fKillDeviceItems = FALSE );
   HRESULT  RemoveLeaf( LPCWSTR szLeafName, BOOL fKillDeviceItem = FALSE );
   HRESULT  RemoveBranch( LPCWSTR szBranchName, BOOL fKillDeviceItems = FALSE );
   HRESULT  RemoveDeviceItemAssociatedLeaf( LPCWSTR szItemID, BOOL fKillDeviceItem = FALSE );

// Implementation
protected:
   HRESULT  Create( DaBranch* pParent, LPCWSTR szBranchName );

   HRESULT  BrowseLeafs( BOOL fReturnFullyQualifiedNames,
                         LPCWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter,
                         LPDWORD pdwNumOfLeafs, BSTR** ppszLeafs );

   HRESULT  GetFullyQualifiedName( BSTR* pszFullyQualifiedName );
   HRESULT  ChangePositionDown( LPCWSTR szPosition, DaBranch** ppNewPos );
   BOOL     FilterName( LPCWSTR szName, LPCWSTR szFilterCriteria, BOOL fFilterBranch );
   BOOL     ExistLeaf( LPCWSTR szLeafName );
   void*    new_realloc( void* memblock, size_t sizeOld, size_t sizeNew );

   static WCHAR m_szDelimiter[2];

      static
   DaBranch*    m_pRoot;

   DaBranch*    m_pParent;
   WideString    m_wsName;                     // The name of the branch

      // Map with the branch members of this branch
   CAtlMap<LPCWSTR,DaBranch*,LPCWSTRRefElementTraits<LPCWSTR>>  m_mapBranches;

      // Map with the leaf members of this branch
   CAtlMap<LPCWSTR,DaLeaf*,LPCWSTRRefElementTraits<LPCWSTR>>    m_mapLeafs;

      // The critical section to lock/unlock the map of branch members
   CComAutoCriticalSection m_csBranches;

      // The critical section to lock/unlock the map of leaf members
   CComAutoCriticalSection m_csLeafs;
};


//-----------------------------------------------------------------------------
// CLASS DaBranch
//-----------------------------------------------------------------------------
class DaLeaf
{
// Construction / Destruction
public:
   DaLeaf();
   HRESULT Create( LPCWSTR szName, DaDeviceItem* pDItem );
   ~DaLeaf();

// Attributes
public:
   inline WideString&     Name()               { return m_wsName; }
   inline DaDeviceItem&     DeviceItem() const   { return *m_pDItemRef; }

// Operations
public:
   HRESULT  GetFullyQualifiedName( DaBranch* pParent, BSTR* pszFullyQualifiedName );
   HRESULT  IsDeviceItem( LPCWSTR szItemID );

// Implementation
protected:
   WideString    m_wsName;                     // The name of the leaf
   DaDeviceItem*   m_pDItemRef;

   friend HRESULT DaBranch::RemoveLeaf( LPCWSTR, BOOL );
   friend void    DaBranch::RemoveAll( BOOL );
   BOOL           m_fKillDeviceItemOnDestroy;
};

#endif // __HIERARCHICALSAS_H
