/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include <stddef.h>

#include "local/BLELocalDescriptor.h"
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
#include "remote/BLERemoteDescriptor.h"
#endif

#include "BLEDescriptor.h"

BLEDescriptor::BLEDescriptor() :
  BLEDescriptor((BLELocalDescriptor*)NULL)
{
}

BLEDescriptor::BLEDescriptor(BLELocalDescriptor* local) :
  _local(local)
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  ,_remote(NULL)
#endif
{
  if (_local) {
    _local->retain();
  }
}

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
BLEDescriptor::BLEDescriptor(BLERemoteDescriptor* remote) :
  _local(NULL),
  _remote(remote)
{
  if (_remote) {
    _remote->retain();
  }
}
#endif

BLEDescriptor::BLEDescriptor(const char* uuid, const uint8_t value[], int valueSize) :
  BLEDescriptor(new BLELocalDescriptor(uuid, value, valueSize))
{
}

BLEDescriptor::BLEDescriptor(const char* uuid, const char* value) :
  BLEDescriptor(new BLELocalDescriptor(uuid, value))
{
}

BLEDescriptor::BLEDescriptor(const BLEDescriptor& other)
{
  _local = other._local;
  if (_local) {
    _local->retain();
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  _remote = other._remote;
  if (_remote) {
    _remote->retain();
  }
#endif
}

BLEDescriptor::~BLEDescriptor()
{
  if (_local && _local->release() == 0) {
    delete _local;
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote && _remote->release() == 0) {
    delete _remote;
  }
#endif
}

const char* BLEDescriptor::uuid() const
{
  if (_local) {
    return _local->uuid();
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote) {
    return _remote->uuid();
  }
#endif

  return "";
}

int BLEDescriptor::valueSize() const
{
  if (_local) {
    return _local->valueSize();
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote) {
    return _remote->valueLength();
  }
#endif

  return 0;
}

const uint8_t* BLEDescriptor::value() const
{
  if (_local) {
    return _local->value();
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote) {
    return _remote->value();
  }
#endif

  return NULL;
}

int BLEDescriptor::valueLength() const
{
  return valueSize();
}

uint8_t BLEDescriptor::operator[] (int offset) const
{
  if (_local) {
    return (*_local)[offset];
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote) {
    return (*_remote)[offset];
  }
#endif

  return 0;
}

int BLEDescriptor::readValue(uint8_t value[], int length)
{
  int bytesRead = 0;

  if (_local) {
    bytesRead = min(length, _local->valueSize());

    memcpy(value, _local->value(), bytesRead);
  }

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote) {
    if (!read()) {
      // read failed
      return 0;
    }

    bytesRead = min(length, _remote->valueLength());

    memcpy(value, _remote->value(), bytesRead);
  }
#endif

  return bytesRead;
}

int BLEDescriptor::readValue(void* value, int length)
{
  return readValue((uint8_t*)value, length);
}

int BLEDescriptor::readValue(uint8_t& value)
{
  value = 0;

  return readValue((uint8_t*)&value, sizeof(value));
}

int BLEDescriptor::readValue(int8_t& value)
{
  value = 0;

  return readValue((uint8_t*)&value, sizeof(value));
}

int BLEDescriptor::readValue(uint16_t& value)
{
  value = 0;

  return readValue((uint8_t*)&value, sizeof(value));
}

int BLEDescriptor::readValue(int16_t& value)
{
  value = 0;

  return readValue((uint8_t*)&value, sizeof(value));
}

int BLEDescriptor::readValue(uint32_t& value)
{
  value = 0;

  return readValue((uint8_t*)&value, sizeof(value));
}

int BLEDescriptor::readValue(int32_t& value)
{
  value = 0;

  return readValue((uint8_t*)&value, sizeof(value));
}

BLEDescriptor::operator bool() const
{
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  return (_local != NULL) || (_remote != NULL);
#else
  return (_local != NULL);
#endif
}

bool BLEDescriptor::read()
{
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  if (_remote) {
    return _remote->read();
  }
#endif

  return false;
}

BLELocalDescriptor* BLEDescriptor::local()
{
  return _local;
}
