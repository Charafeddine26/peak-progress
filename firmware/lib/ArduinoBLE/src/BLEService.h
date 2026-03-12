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

#ifndef _BLE_SERVICE_H_
#define _BLE_SERVICE_H_

#include "BLECharacteristic.h"

class BLELocalService;
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
class BLERemoteService;
#endif

class BLEService {
public:
  BLEService();
  BLEService(const char* uuid);
  BLEService(const BLEService& other);
  virtual ~BLEService();

  const char* uuid() const;
  void clear();

  void addCharacteristic(BLECharacteristic& characteristic);

  operator bool() const;

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  int characteristicCount() const;
  bool hasCharacteristic(const char* uuid) const;
  bool hasCharacteristic(const char* uuid, int index) const;
  BLECharacteristic characteristic(int index) const;
  BLECharacteristic characteristic(const char * uuid) const;
  BLECharacteristic characteristic(const char * uuid, int index) const;
#endif

protected:
  friend class GATTClass;

  BLEService(BLELocalService* local);

  BLELocalService* local();

  void addCharacteristic(BLELocalCharacteristic* characteristic);

protected:
  friend class BLEDevice;

#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  BLEService(BLERemoteService* remote);
#endif

private:
  BLELocalService* _local;
#ifndef ARDUINOBLE_PERIPHERAL_ONLY
  BLERemoteService* _remote;
#endif
};

#endif
