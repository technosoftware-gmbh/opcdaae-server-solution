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


//-------------------------------------------------------------------------
// INLCUDE
//-------------------------------------------------------------------------
#include "stdafx.h"                             // Generic server part headers
#include "MatchPattern.h"
#include "DaDeviceItem.h"
// Application specific definitions
#include "DaAddressSpace.h"

//-------------------------------------------------------------------------
// STATIC MEMBERS
//-------------------------------------------------------------------------
// Default delimiter character between the branches and the leafs.
WCHAR DaBranch::m_szDelimiter[2] = L".";
DaBranch* DaBranch::m_pRoot = NULL;


//-------------------------------------------------------------------------
// CODE DaLeaf
//-------------------------------------------------------------------------

//=========================================================================
// Construction
//=========================================================================
DaLeaf::DaLeaf()
{
	m_pDItemRef = NULL;
	m_fKillDeviceItemOnDestroy = FALSE;
}



//=========================================================================
// Initializer
// -----------
//    Must be called after construction.
//    This functions sets the leaf name and attaches a device item.
//=========================================================================
HRESULT DaLeaf::Create( LPCWSTR szName, DaDeviceItem* pDItem )
{
	HRESULT hres = m_wsName.SetString( szName );
	if (SUCCEEDED( hres )) {
		m_pDItemRef = pDItem;
	}
	return hres;
}



//=========================================================================
// Destructor
//=========================================================================
DaLeaf::~DaLeaf()
{
	if (m_pDItemRef && m_fKillDeviceItemOnDestroy) {
		m_pDItemRef->Kill( TRUE );
	}
}



//-------------------------------------------------------------------------
// OPERATIONS
//-------------------------------------------------------------------------

//=========================================================================
// GetFullyQualifiedName
// ---------------------
//    Returns the fully qualified name of the leaf.
//    The fully qualified name can be build with the parent and the leaf
//    name or with the attached device item. The implementation
//    of this function is server specific.
//
// Parameters:
//    IN
//       pParent                 The parent branch of this leaf.
//    OUT
//       pszFullyQualifiedName   The fully qualified name of this leaf.
//=========================================================================
HRESULT DaLeaf::GetFullyQualifiedName( DaBranch* pParent, BSTR* pszFullyQualifiedName )
{
	LPWSTR   szID;
	HRESULT  hres;
	hres = m_pDItemRef->get_ItemIDPtr( &szID );
	if (SUCCEEDED( hres )) {
		*pszFullyQualifiedName = SysAllocString( szID );
		if (*pszFullyQualifiedName == 0) {
			hres = E_OUTOFMEMORY;
		}
	}
	return hres;
}



//=========================================================================
// IsDeviceItem
// ------------
//    Checks if the ItemId of the attached Device Item is identical
//    with the specified fully qualified name.
//
// Parameters:
//    szItemID                   fully qualified name.
//
// Return:
//    S_OK                       All succeeded.The ID is identical.
//    E_INVALIDARG               The ID is not identical.
//    E_xxx                      An error occured.
//=========================================================================
HRESULT DaLeaf::IsDeviceItem( LPCWSTR szItemID )
{
	LPWSTR   szID;
	HRESULT  hres;

	hres = m_pDItemRef->get_ItemIDPtr( &szID );
	if (FAILED( hres )) return hres;

	hres = wcscmp( szID, szItemID ) == 0 ? S_OK : E_INVALIDARG;

	return hres;
}



//-------------------------------------------------------------------------
// CODE DaBranch
//-------------------------------------------------------------------------

//=========================================================================
// Construction
//=========================================================================
DaBranch::DaBranch()
{
	m_pParent   = NULL;
}



//=========================================================================
// CreateAsRoot
// ------------
//    Initialized the object as root branch.
//    This function must be called only once.
//=========================================================================
HRESULT DaBranch::CreateAsRoot()
{
	_ASSERTE( m_pRoot ==  NULL );                // Root object is already initialzed
	if (m_pRoot) return E_FAIL;                  // Hint : InitializeAsRoot() call should be made only once!

	HRESULT hres = m_wsName.SetString( L"" );
	if (FAILED( hres )) return hres;

	m_pRoot = this;
	return S_OK;
}



//=========================================================================
// Destructor
//=========================================================================
DaBranch::~DaBranch()
{
	// Remove all branches and leafs at this level
	RemoveAll();
}



//-------------------------------------------------------------------------
// OPERATIONS
//-------------------------------------------------------------------------

//=========================================================================
// AddBranch
// ---------
//    Adds a new branch.
//=========================================================================
HRESULT DaBranch::AddBranch( LPCWSTR szBranchName, DaBranch** ppBranch )
{
	DaBranch* pDummyBranch;
	HRESULT     hres;
	// Check if the branch already exist
	hres = ChangeBrowsePosition( OPC_BROWSE_DOWN, szBranchName, &pDummyBranch );
	if (SUCCEEDED( hres )) return E_INVALIDARG;  // The branch already exist at this level.
	if (hres != E_INVALIDARG) return hres;       // Other error.

	*ppBranch = new DaBranch;                  // The branch does not yet exist. Create it.
	if (*ppBranch == NULL) return E_OUTOFMEMORY;

	hres = (*ppBranch)->Create( this, szBranchName );
	if (FAILED( hres )) {
		delete (*ppBranch);
		*ppBranch = NULL;
	}
	return hres;
}



//=========================================================================
// AddLeaf
// -------
//    Adds a new leaf.
//=========================================================================
HRESULT DaBranch::AddLeaf( LPCWSTR szLeafName, DaDeviceItem* pDItem )
{
	if (ExistLeaf( szLeafName )) {               // Check if the leaf already exist
		return E_INVALIDARG;                      // The leaf already exist at this level.
	}

	DaLeaf* pLeaf = new DaLeaf;              // The leaf does not yet exist. Create it.
	if (pLeaf == NULL) return E_OUTOFMEMORY;

	HRESULT hres = pLeaf->Create( szLeafName, pDItem );
	if (SUCCEEDED( hres )) {
		m_csLeafs.Lock();  
		try {
			m_mapLeafs.SetAt( pLeaf->Name(), pLeaf );
		} catch (...) {
			hres = E_OUTOFMEMORY;
		}
		m_csLeafs.Unlock();
	}
	if (FAILED( hres )) {
		delete pLeaf;
	}
	return hres;
}



//=========================================================================
// AddDeviceItem
// -------------
//    Adds a Device Item under the defined name to the Server
//    Address Space.
//    
//    If the name includes not existing branches then they will be created.
//    The name must include at least a leaf name.
//    The leaf name must be unique at the specified level and may not
//    already exist.
//
//    Format samples:
//       Leaf1
//       Branch1.Branch2.Leaf1
//
//    Note: The name must not be identical with the fully qualified
//          ItemId of the specified Device Item.
//
// Parameters:
//    IN
//       szSASName               The Name in the Server Address Space
//       ppDItem                 The DeviceItem
//=========================================================================
HRESULT DaBranch::AddDeviceItem( LPCWSTR szSASName, DaDeviceItem* pDItem )
{
	HRESULT     hres = S_OK;
	LPWSTR      pLeafName = NULL;
	DaBranch* pBranch = this;                  // Start at this level
	WideString wsName;                          // Copy of the SASName parameter

	hres = wsName.SetString( szSASName );        // Make a copy because the string
	if (FAILED( hres )) return hres;             // is modified within this function.

	// Setup the pointer to the leaf name
	pLeafName = wcsrchr( wsName, m_szDelimiter[0] );
	if (!pLeafName) {
		pLeafName = wsName;                       // Ther are no branches specified
	}
	else {                                       // There is at least one branch
		*pLeafName = NULL;                        // Setup the pointer to the leaf and branch(es) name
		pLeafName++;
		if (!(*pLeafName)) return E_INVALIDARG;   // Invalid SASName format

		LPWSTR   pBranchName = wsName;
		WCHAR*   nextToken = NULL;
		LPCWSTR  pName = NULL;
		DaBranch* pNewBranch = NULL;

		m_csBranches.Lock();                      // Goes to the specified position.
		try
		{
			// Not existing branches are created.
			pName = wcstok_s( pBranchName, m_szDelimiter, &nextToken );
			while (pName) {                           // Handle all defined branches.
				// Move to next branch level
				hres = pBranch->ChangeBrowsePosition( OPC_BROWSE_DOWN, pName, &pNewBranch );
				if (FAILED( hres )) {
					if (hres == E_INVALIDARG) {         // The Branch does not exist. Create it.
						hres = pBranch->AddBranch( pName, &pNewBranch );
						if (FAILED( hres )) {
							break;                        // Brach creation failed.
						}
					}
					else {
						break;                           // Other error.
					}
				}
				pBranch = pNewBranch;                  // Move to the new position.
				// Get the name of the next branch.
				pName = wcstok_s( NULL, m_szDelimiter, &nextToken ); 
			}
		} 
		catch (...) {
			hres = E_FAIL;
		}

		m_csBranches.Unlock();
	}
	// Add the leaf to the specified position.
	if (SUCCEEDED( hres )) {
		if (pBranch->ExistLeaf( pLeafName )) {    // Check if the leaf already exist.
			hres = E_INVALIDARG;                   // The leaf already exist at this level.
		}
		else {                                    // The leaf does not exist at this level. Create it.
			hres = pBranch->AddLeaf( pLeafName, pDItem );
		}
	}
	return hres;
}



//=========================================================================
// FindDeviceItem
// --------------
//    Searches the Device Item with the specified fully qualified ItemId.
//    The Device Item must be attached to a leaf at or below this branch.
//
// Parameters:
//    IN
//       szItemID                The fully qualified ItemId.
//    OUT
//       ppDItem                 The DeviceItem
//=========================================================================
HRESULT DaBranch::FindDeviceItem( LPCWSTR szItemID, DaDeviceItem** ppDItem )
{
	HRESULT  hres = E_INVALIDARG;
	POSITION pos;

	*ppDItem = NULL;

	DaBranch* pBranch;
	m_csBranches.Lock();
	try 
	{
		pos = m_mapBranches.GetStartPosition();
		while (pos) {
			pBranch = m_mapBranches.GetNextValue( pos );
			hres = pBranch->FindDeviceItem( szItemID, ppDItem );
			if (SUCCEEDED( hres )) {
				break;
			}
		}
	} 
	catch (...) {
		hres = E_FAIL;
	}
	m_csBranches.Unlock();

	if (FAILED( hres )) {                        // Not found in branches
		DaLeaf* pLeaf;

		m_csLeafs.Lock();
		try
		{
			pos = m_mapLeafs.GetStartPosition();
			while (pos) {
				pLeaf = m_mapLeafs.GetNextValue( pos );
				hres = pLeaf->IsDeviceItem( szItemID );
				if (SUCCEEDED( hres )) {
					*ppDItem = &pLeaf->DeviceItem();
					break;
				}
			}
		} catch (...) {
			hres = E_FAIL;
		}
		m_csLeafs.Unlock();
	}
	return hres;
}



//=========================================================================
// ChangeBrowsePosition
// --------------------
//    Moves 'up' or 'down' or 'to' in the hierarchical space.
//
// Parameters:
//    IN
//       dwBrowseDirection       OPC_BROWSE_DOWN or OPC_BROWSE_UP or
//                               OPC_BROWSE_TO
//       szPosition              DOWN :   The name of the branch move into.
//                               UP:      Is ignored.
//                               TO:      The fully qualified branch name
//                                        or a NULL-String to go to the
//                                        root.
//    OUT
//       ppNewPos                The new position.
//=========================================================================
HRESULT DaBranch::ChangeBrowsePosition( OPCBROWSEDIRECTION dwBrowseDirection, LPCWSTR szPosition, DaBranch** ppNewPos )
{
	HRESULT hres = S_OK;
	*ppNewPos = NULL;

	m_csBranches.Lock();
	try
	{
		switch (dwBrowseDirection) {

		  case OPC_BROWSE_UP:
			  // ------------------------------------------------------------------
			  if (m_pParent) {
				  *ppNewPos = m_pParent;
			  }
			  else {
				  hres = E_FAIL;                      // Moving up from the 'root'
			  }
			  break;

		  case OPC_BROWSE_DOWN:
			  // ------------------------------------------------------------------
			  {
				  try {
					  DaBranch* pBranch;
					  if (m_mapBranches.Lookup( szPosition, pBranch )) {
						  *ppNewPos = pBranch;
					  }
				  }
				  catch (...) {
				  }
				  if (*ppNewPos == NULL) {
					  hres = E_INVALIDARG;             // Not found
				  }
			  }
			  break;

		  case OPC_BROWSE_TO:
			  // ------------------------------------------------------------------
			  hres = m_pRoot->ChangePositionDown( szPosition, ppNewPos );
			  break;

		  default:
			  // ------------------------------------------------------------------
			  hres = E_INVALIDARG;
			  break;
		}
	} catch (...) {
		hres = E_FAIL;
	}	
	m_csBranches.Unlock();

	return hres;
}



//=========================================================================
// BrowseBranches
// --------------
//    Returns the name of the branches which matches the specified filter.
//    Filtering is optional.
//
// Parameters:
//    IN
//       szFilterCriteria        A filter string. A NULL-String
//                               indicates no filtering.
//    OUT
//       pdwNumOfBranches        The number of branch names being
//                               returned.
//       ppszBranches            Array of strings containing the branch
//                               names.
// Return:
//    S_OK                       All succeeded
//    S_FALSE                    There are no branches which matches
//                               the filter. Note : ppszBranches is NULL !
//    E_xxx                      An error occured. ppszBranches is NULL !
//=========================================================================
HRESULT DaBranch::BrowseBranches( LPCWSTR szFilterCriteria, LPDWORD pdwNumOfBranches, BSTR** ppszBranches )
{
	HRESULT  hres = S_OK;
	DWORD    dwMatch = 0;
	bool     fCleanup = false;

	*pdwNumOfBranches = 0;
	*ppszBranches = NULL;

	m_csBranches.Lock();
	try {
		DaBranch* pBranch;

		DWORD dwSize = (DWORD)m_mapBranches.GetCount();
		if (dwSize == 0) throw S_FALSE;           // There are no branches

		*ppszBranches = new LPWSTR [ dwSize ];    // Max. number of names
		if (*ppszBranches == NULL) throw E_OUTOFMEMORY;

		POSITION pos = m_mapBranches.GetStartPosition();
		while (pos) {
			pBranch = m_mapBranches.GetNextValue( pos );
			WideString& wsTmp = pBranch->m_wsName;
			// Filter the name
			if (FilterName( wsTmp, szFilterCriteria, TRUE )) {
				// Name passes the filter
				(*ppszBranches)[dwMatch] = wsTmp.CopyBSTR();
				if ((*ppszBranches)[dwMatch] == NULL) {
					throw E_OUTOFMEMORY;
				}
				dwMatch++;
			}
		}
		if (dwMatch == 0) {
			throw S_FALSE;                         // No braches matches the filter
		}
		*pdwNumOfBranches = dwMatch;
	}
	catch (HRESULT hresEx) {
		fCleanup = true;
		hres = hresEx;
	}
	catch (...) {
		fCleanup = true;
		hres = E_FAIL;
	}
	m_csBranches.Unlock();

	if (fCleanup) {
		if (*ppszBranches) {
			while (dwMatch--) {
				SysFreeString( (*ppszBranches)[dwMatch] );
			}
			delete [] (*ppszBranches);
			*ppszBranches = NULL;
			*pdwNumOfBranches = 0;
		}
	}

	return hres;
}



//=========================================================================
// BrowseLeafs
// -----------
//    Returns the name of the leafs which matches the specified filters.
//
// Parameters:
//    IN
//       szFilterCriteria        A filter string. A NULL-String
//                               indicates no filtering.
//       vtDataTypeFilter        Filter the returned list based in the
//                               available datatypes.
//                               VT_EMPTY indicates no filtering.
//       dwAccessRightsFilter    Filter based on the DaAccessRights bitmask.
//                               0 indicates no filtering.
//    OUT
//       pdwNumOfLeafs           The number of leaf names being returned.
//       ppszLeafs               Array of strings containing the leaf
//                               names.
// Return:
//    S_OK                       All succeeded
//    S_FALSE                    There are no leafs which matches
//                               the filter. Note : ppszLeafs is NULL !
//    E_xxx                      An error occured. ppszLeafs is NULL !
//=========================================================================
HRESULT DaBranch::BrowseLeafs( LPCWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter,
								LPDWORD pdwNumOfLeafs, BSTR** ppszLeafs )
{
	return BrowseLeafs( FALSE,
		szFilterCriteria, vtDataTypeFilter, dwAccessRightsFilter,
		pdwNumOfLeafs, ppszLeafs );
}



//=========================================================================
// BrowseFlat
// -----------
//    All leaf names at and below this branch which matches the specified
//    filters are returned. For all leafs the fully qualified name is
//    returned.
//
// Parameters:
//    IN
//       szFilterCriteria        A filter string. A NULL-String
//                               indicates no filtering.
//       vtDataTypeFilter        Filter the returned list based in the
//                               available datatypes.
//                               VT_EMPTY indicates no filtering.
//       dwAccessRightsFilter    Filter based on the DaAccessRights bitmask.
//                               0 indicates no filtering.
//    OUT
//       pdwNumOfLeafs           The number of leaf names being returned.
//       ppszLeafs               Array of strings containing the fully
//                               qualified leaf names.
// Return:
//    S_OK                       All succeeded
//    S_FALSE                    There are no leafs which matches
//                               the filter. Note : ppszLeafs is NULL !
//    E_xxx                      An error occured. ppszLeafs is NULL !
//=========================================================================
HRESULT DaBranch::BrowseFlat( LPCWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter,
							   LPDWORD pdwNumOfLeafs, BSTR** ppszLeafs )
{
	HRESULT  hres = S_OK;
	DWORD    dwMatch = 0;
	bool     fCleanup = false;

	*pdwNumOfLeafs = 0;
	*ppszLeafs = NULL;

	m_csBranches.Lock();
	try {
		DWORD       dwNumOfSubLeafs;
		BSTR*       pszLeafs;
		DaBranch* pBranch;

		POSITION pos = m_mapBranches.GetStartPosition();
		while (pos) {
			pBranch = m_mapBranches.GetNextValue( pos );
			hres = pBranch->BrowseFlat(   szFilterCriteria, vtDataTypeFilter, dwAccessRightsFilter,
				&dwNumOfSubLeafs, &pszLeafs );
			if (FAILED( hres )) throw hres;

			if (hres == S_OK) {
				*ppszLeafs = (BSTR*)new_realloc( *ppszLeafs, dwMatch * sizeof(BSTR),             // Existing buffer with size
					(dwMatch + dwNumOfSubLeafs) * sizeof (BSTR) );  // New buffer size
				if (*ppszLeafs == NULL) {
					delete [] pszLeafs;
					throw E_OUTOFMEMORY;
				}
				memcpy( &(*ppszLeafs)[dwMatch], pszLeafs, dwNumOfSubLeafs * sizeof (BSTR) );
				dwMatch += dwNumOfSubLeafs;
				delete [] pszLeafs;
			}

		}
		hres = BrowseLeafs(  TRUE,
			szFilterCriteria, vtDataTypeFilter, dwAccessRightsFilter,
			&dwNumOfSubLeafs, &pszLeafs );
		if (FAILED( hres )) throw hres;

		if (hres == S_OK) {
			*ppszLeafs = (BSTR*)new_realloc( *ppszLeafs, dwMatch * sizeof(BSTR),                // Existing buffer with size
				(dwMatch + dwNumOfSubLeafs) * sizeof (BSTR) );     // New buffer size
			if (*ppszLeafs == NULL) {
				delete [] pszLeafs;
				throw E_OUTOFMEMORY;
			}
			memcpy( &(*ppszLeafs)[dwMatch], pszLeafs, dwNumOfSubLeafs * sizeof (BSTR) );
			dwMatch += dwNumOfSubLeafs;
			delete [] pszLeafs;
		}

		if (dwMatch == 0) {
			throw S_FALSE;                         // No leafs matches the filter
		}
		*pdwNumOfLeafs = dwMatch;
		hres = S_OK;
	}
	catch (HRESULT hresEx) {
		fCleanup = true;
		hres = hresEx;
	}
	catch (...) {
		fCleanup = true;
		hres = E_FAIL;
	}
	m_csBranches.Unlock();

	if (fCleanup) {
		if (*ppszLeafs) {
			while (dwMatch--) {
				SysFreeString( (*ppszLeafs)[dwMatch] );
			}
			delete [] (*ppszLeafs);
			*ppszLeafs = NULL;
			*pdwNumOfLeafs = 0;
		}
	}

	return hres;
}



//=========================================================================
// GetFullyQualifiedName
// ---------------------
//    Returns the fully qualified name of a branch or leaf member at or
//    below this branch.
//
// Parameters:
//    IN
//       szName                  The name of a branch or leaf member
//                               at or below this branch. If this parameter
//                               is a NULL-String then the fully qualified
//                               name of this branch is returned. if this
//                               parameter alread specifies a fully
//                               qualified name the the same string is
//                               returned.
//    OUT
//       pszFullyQualifiedName   The fully qualified name.
//=========================================================================
HRESULT DaBranch::GetFullyQualifiedName( LPCWSTR szName, BSTR* pszFullyQualifiedName )
{
	HRESULT  hres = E_INVALIDARG;

	*pszFullyQualifiedName = NULL;
	m_csBranches.Lock();
	try {

		if (*szName) {                            // Return the fully qualified name of a branch or leaf member

			DaBranch* pBranch;                   // First try if a branch name is specified
			if (SUCCEEDED( ChangePositionDown( szName, &pBranch ) )) {
				hres = pBranch->GetFullyQualifiedName( pszFullyQualifiedName );
				if (FAILED( hres )) throw hres;
			}
			// It is not a branch name. Try if it is a leaf name.
			// Check if szName specifies a leaf of this branch

			WideString wsNameCopy;                // Make a copy because the string will be modified
			HRESULT hresTmp = wsNameCopy.SetString( szName );
			if (FAILED( hresTmp )) throw hresTmp;

			WCHAR* pwc = wcsrchr( wsNameCopy, m_szDelimiter[0] );
			if (pwc) {                             
				*pwc = NULL;                        // szName includes a branch name
				if (SUCCEEDED( ChangePositionDown( wsNameCopy, &pBranch ) )) {
					pwc++;
					if (*pwc) {
						hres = pBranch->GetFullyQualifiedName( pwc, pszFullyQualifiedName );
						if (FAILED( hres )) throw hres;
					}
				}
			}
			else {
				DaLeaf* pLeaf;                    // szName specifies a leaf of this branch

				m_csLeafs.Lock();
				try {
					if (m_mapLeafs.Lookup( szName, pLeaf )) {
						hres = pLeaf->GetFullyQualifiedName( pBranch, pszFullyQualifiedName );
						if (FAILED( hres )) throw hres;
					}
				}
				catch (...) {
					m_csLeafs.Unlock();
					throw hres;
				}
				m_csLeafs.Unlock();
			}

			// Note :
			//    The folowing test is only required if the fully qualified ItemId
			//    is not identical with the branch/leaf names !
			//
			if (FAILED( hres )) {                  // Check if szName specifies a fully qualified name
				DaDeviceItem* pDItem;
				hres = m_pRoot->FindDeviceItem( szName, &pDItem );
				if (SUCCEEDED( hres )) {
					*pszFullyQualifiedName = SysAllocString( szName );
					if (*pszFullyQualifiedName == NULL) throw E_OUTOFMEMORY;
				}
			}
			//
			//
			//
		}
		else {                                    // Return the fully qualified name of this branch
			hres = GetFullyQualifiedName( pszFullyQualifiedName );
		}
	}
	catch (HRESULT hresEx) {
		hres = hresEx;
	}
	catch (...) {
		hres = E_FAIL;
	}
	m_csBranches.Unlock();

	return hres;
}



//=========================================================================
// RemoveAll
// ---------
//    Removes all branches and leaves.
//    If fKillDeviceItems is set then all Device Items associated with
//    leafs are killed.
//=========================================================================
void DaBranch::RemoveAll( BOOL fKillDeviceItems /* = FALSE */ )
{
	// Remove all branches
	DaBranch* pBranch;
	m_csBranches.Lock();
	POSITION pos;
	try
	{
		pos = m_mapBranches.GetStartPosition();
		while (pos) {
			pBranch = m_mapBranches.GetNextValue( pos );
			pBranch->RemoveAll( fKillDeviceItems );
			delete pBranch;
		}
		m_mapBranches.RemoveAll();
	} catch (...) {
	}
	m_csBranches.Unlock();

	// Remove all leafs
	DaLeaf* pLeaf;
	m_csLeafs.Lock();
	try
	{
		pos = m_mapLeafs.GetStartPosition();
		while (pos) {
			pLeaf = m_mapLeafs.GetNextValue( pos );
			pLeaf->m_fKillDeviceItemOnDestroy = fKillDeviceItems;
			delete pLeaf;
		}
		m_mapLeafs.RemoveAll();
	} catch (...) {
	}
	m_csLeafs.Unlock();
}



//=========================================================================
// RemoveLeaf
// ----------
//    Removes the specified leaf.
//    If fKillDeviceItems is set then the Device Item associated with
//    the leaf ist killed.
//=========================================================================
HRESULT DaBranch::RemoveLeaf( LPCWSTR szLeafName, BOOL fKillDeviceItem /* = FALSE */ )
{
	HRESULT hres = E_INVALIDARG;
	DaLeaf* pLeaf;

	m_csLeafs.Lock();
	try {
		if (m_mapLeafs.Lookup( szLeafName, pLeaf )) {
			if (m_mapLeafs.RemoveKey( szLeafName )) {
				pLeaf->m_fKillDeviceItemOnDestroy = fKillDeviceItem;
				delete pLeaf;
				hres = S_OK;
			}        
		}
	}
	catch (...) {
	}
	m_csLeafs.Unlock();

	return hres;
}



//=========================================================================
// RemoveBranch
// ------------
//    Removes the specified branch with all sub-branches and leaves.
//    If fKillDeviceItems is set then all Device Items associated with
//    leafs are killed.
//=========================================================================
HRESULT DaBranch::RemoveBranch( LPCWSTR szBranchName, BOOL fKillDeviceItems /* = FALSE */ )
{
	HRESULT hres = E_INVALIDARG;
	DaBranch* pBranch;

	m_csBranches.Lock();
	try {
		if (m_mapBranches.Lookup( szBranchName, pBranch )) {
			if (m_mapBranches.RemoveKey( szBranchName )) {
				pBranch->RemoveAll( fKillDeviceItems );
				delete pBranch;
				hres = S_OK;
			}
		}
	}
	catch (...) {
	}
	m_csBranches.Unlock();

	return hres;
}



//=========================================================================
// RemoveDeviceItemAssociatedLeaf
// ------------------------------
//    Removes the leaf whose associated Device Item has the specified
//    fully qualified ItemId.
//    If fKillDeviceItems is set then the Device Item associated with
//    the leaf ist killed.
//    The Device Item must be attached to a leaf at or below this branch.
//=========================================================================
HRESULT DaBranch::RemoveDeviceItemAssociatedLeaf( LPCWSTR szItemID, BOOL fKillDeviceItem /* = FALSE */  )
{
	HRESULT  hres = E_INVALIDARG;
	POSITION pos;

	DaBranch* pBranch;
	m_csBranches.Lock();
	try
	{
		pos = m_mapBranches.GetStartPosition();
		while (pos) {
			pBranch = m_mapBranches.GetNextValue( pos );
			hres = pBranch->RemoveDeviceItemAssociatedLeaf( szItemID, fKillDeviceItem );
			if (SUCCEEDED( hres )) {
				break;
			}
		}
	} catch (...) {
		hres = E_FAIL;
	}
	m_csBranches.Unlock();

	if (FAILED( hres )) {                        // Not found in branches
		DaLeaf* pLeaf;

		m_csLeafs.Lock();
		try
		{
			pos = m_mapLeafs.GetStartPosition();
			while (pos) {
				POSITION posDI = pos;
				pLeaf = m_mapLeafs.GetNextValue( pos );
				hres = pLeaf->IsDeviceItem( szItemID );
				if (SUCCEEDED( hres )) {
					if (fKillDeviceItem) {
						pLeaf->DeviceItem().Kill( TRUE );
					}
					delete pLeaf;
					m_mapLeafs.RemoveAtPos( posDI );
					break;
				}
			}
		} catch (...) {
			hres = E_FAIL;
		}
		m_csLeafs.Unlock();
	}
	return hres;
}



//-------------------------------------------------------------------------
// IMPLEMENTATION
//-------------------------------------------------------------------------

//=========================================================================
// Initializer
// -----------
//    Must be called after construction.
//    This function must not be called for the root object.
//    Initializes the new branch and add it to the parent branch.
//=========================================================================
HRESULT DaBranch::Create( DaBranch* pParent, LPCWSTR szBranchName )
{
	_ASSERTE( pParent != NULL );                 // A parent must be specified
	// Use CreateAsRoot() if  there is no
	// parent (e.g. to create the root object')
	_ASSERTE( szBranchName != NULL );            // Must not be NULL
	m_pParent   = pParent;

	_ASSERTE( m_pRoot !=  NULL );                // Root object must be initialzed
	if (!m_pRoot) return E_FAIL;                 // Hint : InitializeAsRoot() not yet called !

	HRESULT hres = m_wsName.SetString( szBranchName );
	if (FAILED( hres )) return hres;

	m_pParent->m_csBranches.Lock();
	try {
		m_pParent->m_mapBranches.SetAt( m_wsName, (DaBranch*)this );
		hres = S_OK;
	}
	catch (...) {
		hres = E_OUTOFMEMORY;
	}
	m_pParent->m_csBranches.Unlock();
	return hres;
}



//=========================================================================
// BrowseLeafs
// -----------
//    Returns the name of the leafs which matches the specified filters.
//
// Parameters:
//    IN
//       fReturnFullyQualifiedNames
//                               If TRUE then the fully qualified leaf 
//                               names are returned; otherwise the short
//                               leaf names.
//       szFilterCriteria        A filter string. A NULL-String
//                               indicates no filtering.
//       vtDataTypeFilter        Filter the returned list based in the
//                               available datatypes.
//                               VT_EMPTY indicates no filtering.
//       dwAccessRightsFilter    Filter based on the DaAccessRights bitmask.
//                               0 indicates no filtering.
//    OUT
//       pdwNumOfLeafs           The number of leaf names being returned.
//       ppszLeafs               Array of strings containing the leaf
//                               names.
// Return:
//    S_OK                       All succeeded
//    S_FALSE                    There are no leafs which matches
//                               the filter. Note : ppszLeafs is NULL !
//    E_xxx                      An error occured. ppszLeafs is NULL !
//=========================================================================
HRESULT DaBranch::BrowseLeafs( BOOL fReturnFullyQualifiedNames,
								LPCWSTR szFilterCriteria, VARTYPE vtDataTypeFilter, DWORD dwAccessRightsFilter,
								LPDWORD pdwNumOfLeafs, BSTR** ppszLeafs )
{
	HRESULT  hres = S_FALSE;
	DWORD    dwMatch = 0;
	bool     fCleanup = false;

	*pdwNumOfLeafs = 0;
	*ppszLeafs = NULL;

	m_csLeafs.Lock();
	try {
		DWORD       dwAccessRights;
		DaLeaf*   pLeaf;

		DWORD dwSize = (DWORD)m_mapLeafs.GetCount();
		if (dwSize == 0) throw S_FALSE;           // There are no leafs

		*ppszLeafs = new LPWSTR [ dwSize ];       // Max. number of names
		if (*ppszLeafs == NULL) throw E_OUTOFMEMORY;

		POSITION pos = m_mapLeafs.GetStartPosition();
		while (pos) {
			pLeaf = m_mapLeafs.GetNextValue( pos );
			DaDeviceItem& DItem = pLeaf->DeviceItem();

			if (dwAccessRightsFilter) {
				hres = DItem.get_AccessRights( &dwAccessRights );
				if (FAILED( hres )) throw hres;
			}
			if ((dwAccessRightsFilter == 0) ||
				(dwAccessRightsFilter & dwAccessRights)) {
					// Item correspond to Access Rights Filter
					if ((vtDataTypeFilter == VT_EMPTY) ||
						(vtDataTypeFilter == DItem.get_CanonicalDataType())) {
							// Item correspond to Data Type Filter
							BSTR szTmp;
							if (fReturnFullyQualifiedNames) {
								pLeaf->GetFullyQualifiedName( this, &szTmp );
							}
							else {
								szTmp = pLeaf->Name().CopyBSTR();
							}
							if (!szTmp) throw E_OUTOFMEMORY;

							if (FilterName( szTmp, szFilterCriteria, FALSE )) {
								(*ppszLeafs)[dwMatch] = szTmp;// Item correspond to Server Specific Filter
								dwMatch++;
							}
							else {
								SysFreeString( szTmp );
							}
					}
			}
		}
		if (dwMatch == 0) {
			throw S_FALSE;                         // No leafs matches the filter
		}
		*pdwNumOfLeafs = dwMatch;
	}
	catch (HRESULT hresEx) {
		fCleanup = true;
		hres = hresEx;
	}
	catch (...) {
		fCleanup = true;
		hres = E_FAIL;
	}
	m_csLeafs.Unlock();

	if (fCleanup) {
		if (*ppszLeafs) {
			while (dwMatch--) {
				SysFreeString( (*ppszLeafs)[dwMatch] );
			}
			delete [] (*ppszLeafs);
			*ppszLeafs = NULL;
			*pdwNumOfLeafs = 0;
		}
	}

	return hres;
}



//=========================================================================
// GetFullyQualifiedName
// ---------------------
//    Returns the fully qualified name of the branch.
//
// Parameters:
//    OUT
//       pszFullyQualifiedName   The fully qualified name of this branch.
//=========================================================================
HRESULT DaBranch::GetFullyQualifiedName( BSTR* pszFullyQualifiedName )
{
	HRESULT hres = S_OK;

	*pszFullyQualifiedName = NULL;

	if (m_pParent && m_pParent->m_pParent) {
		// Non first level branch
		BSTR bstrQualifiedParentName = NULL;   // Only non first-level branches requires
		// the delimiter character               
		hres = m_pParent->GetFullyQualifiedName( &bstrQualifiedParentName );
		if (SUCCEEDED( hres )) {
			unsigned int uLen = SysStringLen( bstrQualifiedParentName ) + (unsigned int)wcslen( m_wsName ) + 2;
			// +2 for EOS and the delimiter character
			*pszFullyQualifiedName = SysAllocStringLen( bstrQualifiedParentName, uLen );
			if (*pszFullyQualifiedName != NULL) {
				wcscpy_s( *pszFullyQualifiedName, uLen, bstrQualifiedParentName );
				wcscat_s( *pszFullyQualifiedName, uLen, m_szDelimiter );
				wcscat_s( *pszFullyQualifiedName, uLen, m_wsName );
			}
			else {
				hres = E_OUTOFMEMORY;
			}
			// Release temporary parent name
			SysFreeString( bstrQualifiedParentName ); 
		}
	}                                         // First level branch
	else {
		*pszFullyQualifiedName = m_wsName.CopyBSTR();
	}
	if (*pszFullyQualifiedName == 0) {
		hres = E_OUTOFMEMORY;
	}
	return hres;
}



//=========================================================================
// ChangePositionDown
// ------------------
//    Returns the branch at the specified position. The position can
//    specifiy more than one branch level.
//    This function assumes that m_mapBranches is already locked
//    by the caller.
//
// Parameters:
//    IN
//       szPosition              The position to return or a NULL-String
//                               to return the root.
//    OUT
//       pszFullyQualifiedName   The fully qualified name of this branch.
//=========================================================================
HRESULT DaBranch::ChangePositionDown( LPCWSTR szPosition, DaBranch** ppNewPos )
{
	HRESULT     hres = S_OK;
	LPCWSTR     pName;
	WCHAR*	   nextToken = NULL;
	WideString wsNameCopy;
	DaBranch* pBranch = this;

	*ppNewPos = NULL;
	try {
		// Make a copy because wcstok() modifies the string
		hres = wsNameCopy.SetString( szPosition );
		if (FAILED( hres )) throw hres;
		// Get first branch name
		pName = wcstok_s( wsNameCopy, m_szDelimiter, &nextToken );
		if (!pName) {                    // Move to the root if the string is empty
			*ppNewPos = m_pRoot;
			throw S_OK;
		}

		while (pName) {                  // Move to next branch level
			hres = pBranch->ChangeBrowsePosition( OPC_BROWSE_DOWN, pName, &pBranch );
			if (FAILED( hres ))
			{
				return E_INVALIDARG;		// Invalid branch name
			}
			// Get next branch name
			pName = wcstok_s( NULL, m_szDelimiter, &nextToken );
		}
		*ppNewPos = pBranch;
	}
	catch (HRESULT hresEx) {
		hres = hresEx;
	}
	catch (...) {
		hres = E_FAIL;
	}
	return hres;
}



//=========================================================================
// FilterName
// ----------
//    Filters a branch name with the specified filter.
//
// Parameters:
//    szName                     The branch name.
//    szFilterCriteria           A filter string.
//    fFilterBranch              If TRUE the parameter szName specifies
//                               a branch name; otherwise a leaf name.
// Return:
//    If szName matches szFilterCriteria, return TRUE; if there is
//    no match, return is FALSE. If either szName or szFilterCriteria is
//    Null, return is FALSE.
//=========================================================================
BOOL DaBranch::FilterName( LPCWSTR szName, LPCWSTR szFilterCriteria, BOOL fFilterBranch )
{
	if (*szFilterCriteria) {
		//
		// TODO: Add server specific filtering if desired
		//

		// Call MatchPattern() to support default filtering
		// specified by the DA specification.
		return ::MatchPattern( szName, szFilterCriteria );
	}
	return TRUE;                                 // No filter is specified
}



//=========================================================================
// ExistLeaf
// ---------
//    Checks if at this level a leaf with the specified name exist.
//
// Parameters:
//    szLeafName                 The leaf name.
//
// Return:
//    Returns TRUE if a leaf with the specified name
//    exist; otherwise FALSE.
//=========================================================================
BOOL DaBranch::ExistLeaf( LPCWSTR szLeafName )
{
	DaLeaf*   pLeaf;
	bool        fFound = false;

	m_csLeafs.Lock();
	try {
		fFound = m_mapLeafs.Lookup( szLeafName, pLeaf );
	}
	catch (...) {
	}
	m_csLeafs.Unlock();

	return fFound ? TRUE : FALSE;
}



//=========================================================================
// new_realloc
// -----------
//    Reallocates memory with the 'new' operator
//
//    Note:
//       The generic server part uses the 'delete' operator to release
//       memory blocks allocated within the application specific part.
//       Mixing the function realloc() and the operator delete may
//       result in trouble.
//
// Parameters:
//    memblock                   Pointer to previously allocated
//                               memory block or NULL.
//    sizeOld                    Size of previously allocated buffer
//                               in bytes.
//    sizeNew                    New buffer size in bytes.
//
// Return:
//    Address of reallocated memory or NULL if not enough memory available.
//=========================================================================
void* DaBranch::new_realloc( void* memblock, size_t sizeOld, size_t sizeNew )
{
	void* pNew = new BYTE[sizeNew];              // Allocate new buffer
	if (pNew != NULL) {
		if (memblock) {                           // There is an existing buffer
			memcpy( pNew, memblock, sizeOld );     // Initialize new buffer with old buffer
			delete [] (BYTE*)memblock;             // Release old buffer
		}
	}
	return pNew;                                 // Return new buffer
}
