/**
 * Copyright (C) 2007 Doug Judd (Zvents, Inc.)
 * 
 * This file is part of Hypertable.
 * 
 * Hypertable is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 * 
 * Hypertable is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <cassert>
#include <iostream>

#include "Common/Logger.h"

#include "Hypertable/Lib/Key.h"

#include "CellCache.h"
#include "CellCacheScanner.h"

using namespace Hypertable;
using namespace std;


CellCache::~CellCache() {
  for (CellMapT::iterator iter = m_cell_map.begin(); iter != m_cell_map.end(); iter++)
    delete [] (*iter).first;
  //cout << "DELETE CellCache" << endl;
}


/**
 * 
 */
int CellCache::add(const ByteString32T *key, const ByteString32T *value) {
  size_t kvLen = key->len + (2*sizeof(int32_t));
  ByteString32T *newKey;
  ByteString32T *newValue;

  if (value) {
    kvLen += value->len;
    newKey = (ByteString32T *)new uint8_t [ kvLen ];
    newValue = (ByteString32T *)(newKey->data + key->len);
    memcpy(newKey, key, sizeof(int32_t) + key->len);
    memcpy(newValue, value, sizeof(int32_t) + value->len);
  }
  else {
    newKey = (ByteString32T *)new uint8_t [ kvLen ];
    newValue = (ByteString32T *)(newKey->data + key->len);
    memcpy(newKey, key, sizeof(int32_t) + key->len);
    newValue->len = 0;
  }

  CellMapT::iterator iter = m_cell_map.lower_bound(key);

  m_cell_map.insert(iter, CellMapT::value_type(newKey, newValue));

  m_memory_used += sizeof(CellMapT::value_type) + kvLen;

  return 0;
}



const char *CellCache::get_split_row() {
  assert(!"CellCache::get_split_row not implemented!");
  return 0;
}



void CellCache::get_split_rows(std::vector<std::string> &split_rows) {
  boost::mutex::scoped_lock lock(m_mutex);
  if (m_cell_map.size() > 2) {
    CellMapT::const_iterator iter = m_cell_map.begin();
    size_t i=0, mid = m_cell_map.size() / 2;
    for (i=0; i<mid; i++)
      iter++;
    split_rows.push_back((const char *)(*iter).first->data);
  }
}



void CellCache::get_rows(std::vector<std::string> &rows) {
  boost::mutex::scoped_lock lock(m_mutex);
  const char *last_row = "";
  for (CellMapT::const_iterator iter = m_cell_map.begin(); iter != m_cell_map.end(); iter++) {
    if (strcmp((const char *)(*iter).first->data, last_row)) {
      rows.push_back((const char *)(*iter).first->data);
      last_row = (const char *)(*iter).first->data;
    }
  }
}



CellListScanner *CellCache::create_scanner(ScanContextPtr &scanContextPtr) {
  CellCachePtr cellCachePtr(this);
  return new CellCacheScanner(cellCachePtr, scanContextPtr);
}



/**
 * This must be called with the cell cache locked
 */
CellCache *CellCache::slice_copy(uint64_t timestamp) {
  CellCache *cache = new CellCache();
  Key keyComps;
  ByteString32T *key;
  ByteString32T *value;
  size_t kvLen;

  for (CellMapT::iterator iter = m_cell_map.begin(); iter != m_cell_map.end(); iter++) {

    if (!keyComps.load((*iter).first)) {
      LOG_ERROR("Problem deserializing key/value pair");
      continue;
    }

    if (keyComps.timestamp > timestamp) {
      kvLen = (*iter).first->len + (*iter).second->len + (2*sizeof(int32_t));
      key = (ByteString32T *) new uint8_t [ kvLen ];
      memcpy(key, (*iter).first, kvLen);
      value = (ByteString32T *)(key->data + key->len);
      cache->add(key, value);
      /**
      if (keyComps.timestamp == timestamp+1)
	cout << "drj (2) BOUNDARY row-key=" << keyComps.row << " qualifier=" << keyComps.column_qualifier << endl << flush;
      */
    }
    /*
    else if (keyComps.timestamp == timestamp) {
      cout << "drj BOUNDARY row-key=" << keyComps.row << " qualifier=" << keyComps.column_qualifier << endl << flush;
    }
    */
  }

  return cache;
}

