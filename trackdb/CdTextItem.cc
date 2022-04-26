/*  cdrdao - write audio CD-Rs in disc-at-once mode
 *
 *  Copyright (C) 1998-2001  Andreas Mueller <andreas@daneb.de>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <assert.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#include "CdTextItem.h"
#include "CdTextContainer.h"
#include "util.h"

CdTextItem::CdTextItem(PackType packType, int blockNr, const char *data)
{
  assert(blockNr >= 0 && blockNr <= 7);

  next_ = NULL;

  dataType_ = DataType::SBCC;
  packType_ = packType;
  blockNr_ = blockNr;
  trackNr_ = 0;

  dataLen_ = strlen(data) + 1;

  data_ = new unsigned char[dataLen_];

  strcpy((char *)data_, data);
}


CdTextItem::CdTextItem(PackType packType, int blockNr,
		       const unsigned char *data, long len)
{
  assert(blockNr >= 0 && blockNr <= 7);

  next_ = NULL;

  dataType_ = DataType::BINARY;
  packType_ = packType;
  blockNr_ = blockNr;
  trackNr_ = 0;

  dataLen_ = len;

  if (len > 0) {
    data_ = new unsigned char[len];
    memcpy(data_, data, len);
  }
  else {
    data_ = NULL;
  }
}

CdTextItem::CdTextItem(int blockNr, unsigned char genreCode1,
                       unsigned char genreCode2, const char *description)
{
  assert(blockNr >= 0 && blockNr <= 7);

  next_ = NULL;

  dataType_ = DataType::BINARY;
  packType_ = PackType::GENRE;
  blockNr_ = blockNr;
  trackNr_ = 0;

  dataLen_ = 2;

  if (description != NULL)
    dataLen_ += strlen(description) + 1;

  data_ = new unsigned char[dataLen_];
  data_[0] = genreCode1;
  data_[1] = genreCode2;

  if (description != NULL)
    memcpy(data_ + 2, description, dataLen_ - 2);
}

CdTextItem::CdTextItem(const CdTextItem &obj)
{
  next_ = NULL;

  dataType_ = obj.dataType_;
  packType_ = obj.packType_;
  blockNr_ = obj.blockNr_;
  trackNr_ = obj.trackNr_;

  dataLen_ = obj.dataLen_;

  if (dataLen_ > 0) {
    data_ = new unsigned char[dataLen_];
    memcpy(data_, obj.data_, dataLen_);
  }
  else {
    data_ = NULL;
  }
}

CdTextItem::~CdTextItem()
{
  delete[] data_;
  data_ = NULL;

  next_ = NULL;
}

void CdTextItem::print(std::ostream &out, PrintParams& params) const
{
  int i;
  char buf[20];
  out << packType2String(isTrackPack(), packType_);

  if (dataType() == DataType::SBCC) {
    out << " \"";
    for (i = 0; i < dataLen_ - 1; i++) {
      if (data_[i] == '"') {
	out << "\\\"";
      }
      else if (isprint(data_[i])) {
	out << data_[i];
      }
      else {
	sprintf(buf, "\\%03o", (unsigned int)data_[i]);
	out << buf;
      }
    }

    out << "\"";
  }
  else {
    long i;

    out << " {";
    for (i = 0; i < dataLen_; i++) {
      if (i == 0) {
	sprintf(buf, "%2d", (unsigned int)data_[i]);
	out << buf;
      }
      else {
	if (i % 12 == 0)
	  out << ",\n               ";
	else
	  out << ", ";

	sprintf(buf, "%2d", (unsigned int)data_[i]);
	out << buf;
      }
    }

    out << "}";
  }
}

int CdTextItem::operator==(const CdTextItem &obj)
{
  if (packType_ != obj.packType_ || blockNr_ != obj.blockNr_ ||
      dataType_ != obj.dataType_ || dataLen_ != obj.dataLen_)
    return 0;

  return (memcmp(data_, obj.data_, dataLen_) == 0) ? 1 : 0;
}

int CdTextItem::operator!=(const CdTextItem &obj)
{
  return (*this == obj) ? 0 : 1;
}

const char *CdTextItem::packType2String(int isTrack, PackType packType)
{
  const char *ret = "UNKNOWN";

  switch (packType) {
  case PackType::TITLE:
    ret = "TITLE";
    break;
  case PackType::PERFORMER:
    ret = "PERFORMER";
    break;
  case PackType::SONGWRITER:
    ret = "SONGWRITER";
    break;
  case PackType::COMPOSER:
    ret = "COMPOSER";
    break;
  case PackType::ARRANGER:
    ret = "ARRANGER";
    break;
  case PackType::MESSAGE:
    ret = "MESSAGE";
    break;
  case PackType::DISK_ID:
    ret = "DISC_ID";
    break;
  case PackType::GENRE:
    ret = "GENRE";
    break;
  case PackType::TOC_INFO1:
    ret = "TOC_INFO1";
    break;
  case PackType::TOC_INFO2:
    ret = "TOC_INFO2";
    break;
  case PackType::RES1:
    ret = "RESERVED1";
    break;
  case PackType::RES2:
    ret = "RESERVED2";
    break;
  case PackType::RES3:
    ret = "RESERVED3";
    break;
  case PackType::CLOSED:
    ret = "CLOSED";
    break;
  case PackType::UPCEAN_ISRC:
    if (isTrack)
      ret = "ISRC";
    else
      ret = "UPC_EAN";
    break;
  case PackType::SIZE_INFO:
    ret = "SIZE_INFO";
    break;
  }

  return ret;
}

CdTextItem::PackType CdTextItem::int2PackType(int i)
{
  PackType t = PackType::TITLE;

  switch (i) {
  case 0x80:
    t = PackType::TITLE;
    break;
  case 0x81:
    t = PackType::PERFORMER;
    break;
  case 0x82:
    t = PackType::SONGWRITER;
    break;
  case 0x83:
    t = PackType::COMPOSER;
    break;
  case 0x84:
    t = PackType::ARRANGER;
    break;
  case 0x85:
    t = PackType::MESSAGE;
    break;
  case 0x86:
    t = PackType::DISK_ID;
    break;
  case 0x87:
    t = PackType::GENRE;
    break;
  case 0x88:
    t = PackType::TOC_INFO1;
    break;
  case 0x89:
    t = PackType::TOC_INFO2;
    break;
  case 0x8a:
    t = PackType::RES1;
    break;
  case 0x8b:
    t = PackType::RES2;
    break;
  case 0x8c:
    t = PackType::RES3;
    break;
  case 0x8d:
    t = PackType::CLOSED;
    break;
  case 0x8e:
    t = PackType::UPCEAN_ISRC;
    break;
  case 0x8f:
    t = PackType::SIZE_INFO;
    break;
  }

  return t;
}

int CdTextItem::isBinaryPack(PackType type)
{
  int ret;

  switch (type) {
  case PackType::TOC_INFO1:
  case PackType::TOC_INFO2:
  case PackType::SIZE_INFO:
  case PackType::GENRE:
    ret = 1;
    break;

  default:
    ret = 0;
    break;
  }

  return ret;
}
