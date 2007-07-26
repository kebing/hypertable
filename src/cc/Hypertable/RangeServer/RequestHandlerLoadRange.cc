/**
 * Copyright (C) 2007 Doug Judd (Zvents, Inc.)
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "Common/Error.h"
#include "Common/Logger.h"

#include "AsyncComm/ResponseCallback.h"

#include "RangeServer.h"
#include "Request.h"
#include "RequestHandlerLoadRange.h"

using namespace hypertable;

/**
 *
 */
void RequestHandlerLoadRange::run() {
  ResponseCallback cb(mComm, mEventPtr);
  TabletIdentifierT tablet;
  size_t skip;
  size_t remaining = mEventPtr->messageLen - sizeof(int16_t);
  uint8_t *msgPtr = mEventPtr->message + sizeof(int16_t);
  std::string errMsg;

  /**
   * Deserialize Tablet Identifier
   */
  if ((skip = DeserializeTabletIdentifier(msgPtr, remaining, &tablet)) == 0)
    goto abort;

  mRangeServer->LoadRange(&cb, &tablet);

  return;

 abort:
  LOG_ERROR("Encoding problem with LoadRange message");
  errMsg = "Encoding problem with LoadRange message";
  cb.error(Error::PROTOCOL_ERROR, errMsg);
  return;
}