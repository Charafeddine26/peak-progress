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

#include "local/BLELocalService.h"
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
#include "remote/BLERemoteService.h"
#endif

#include "BLEService.h"

extern "C" int strcasecmp(char const *a, char const *b);

BLEService::BLEService() :
  BLEService((BLELocalService*)NULL)
{
}

BLEService::BLEService(BLELocalService* local) :
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
BLEService::BLEService(BLERemoteService* remote) :
  _local(NULL),
  _remote(remote)
{
  if (_remote) {
    _remote->retain();
  }
}
#endif

BLEService::BLEService(const char* uuid) :
  BLEService(new BLELocalService(uuid))
{
}

BLEService::BLEService(const BLEService& other)
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

void BLEService::clear()
{
  if (_local) {
    _local->clear();
  }
}

BLEService::~BLEService()
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

const char* BLEService::uuid() const
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

void BLEService::addCharacteristic(BLECharacteristic& characteristic)
{
  if (_local) {
    _local->addCharacteristic(characteristic);
  }
}

BLEService::operator bool() const
{
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  return (_local != NULL) || (_remote != NULL);
#else
  return (_local != NULL);
#endif
}

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
int BLEService::characteristicCount() const
{
  if (_remote) {
    return _remote->characteristicCount();
  }

  return 0;
}

bool BLEService::hasCharacteristic(const char* uuid) const
{
  return hasCharacteristic(uuid, 0);
}

bool BLEService::hasCharacteristic(const char* uuid, int index) const
{
  if (_remote) {
    int count = 0;
    int numCharacteristics = _remote->characteristicCount();

    for (int i = 0; i < numCharacteristics; i++) {
      BLERemoteCharacteristic* c = _remote->characteristic(i);

      if (strcasecmp(uuid, c->uuid()) == 0) {
        if (count == index) {
          return true;
        }

        count++;
      }
    }
  }

  return false;
}

BLECharacteristic BLEService::characteristic(int index) const
{
  if (_remote) {
    return BLECharacteristic(_remote->characteristic(index));
  }

  return BLECharacteristic();
}

BLECharacteristic BLEService::characteristic(const char * uuid) const
{
  return characteristic(uuid, 0);
}

BLECharacteristic BLEService::characteristic(const char * uuid, int index) const
{
  if (_remote) {
    int count = 0;
    int numCharacteristics = _remote->characteristicCount();

    for (int i = 0; i < numCharacteristics; i++) {
      BLERemoteCharacteristic* c = _remote->characteristic(i);

      if (strcasecmp(uuid, c->uuid()) == 0) {
        if (count == index) {
          return BLECharacteristic(c);
        }

        count++;
      }
    }
  }

  return BLECharacteristic();
}
#endif

BLELocalService* BLEService::local()
{
  return _local;
}
