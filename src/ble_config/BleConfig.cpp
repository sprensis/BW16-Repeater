/*                                      _     
        ___ _ __  _ __ ___ _ __  ___(_)___ 
        / __| '_ \| '__/ _ \ '_ \/ __| / __| 
        \__ \ |_) | | |  __/ | | \__ \ \__ \ 
        |___/ .__/|_|  \___|_| |_|___/_|___/ 
            |_|                             
                © Copyright 2025 
            ✈ `https://github.com/sprensis` 
    Name: BleConfig 
    Description: BLE GATT-based configurator 
    Author: @sprensis 
    Platform: BW16 (RTL8720dn) - Ameba Arduino 
    License: MIT 
*/

#include "BleConfig.h"
#include "../utils/BuildConfig.h"
#if HAVE_BLE
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

static BLEServer* srv;
static BLEService* svc;
static BLECharacteristic* chStaSsid;
static BLECharacteristic* chStaPass;
static BLECharacteristic* chApSsid;
static BLECharacteristic* chApPass;
static BLECharacteristic* chMode;
static BLECharacteristic* chCommit;

class Cb : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic* c) {
    std::string v = c->getValue();
    if (c==chStaSsid) strncpy(store->mutableRef().staSsid, v.c_str(), 63);
    else if (c==chStaPass) strncpy(store->mutableRef().staPass, v.c_str(), 63);
    else if (c==chApSsid) strncpy(store->mutableRef().apSsid, v.c_str(), 63);
    else if (c==chApPass) strncpy(store->mutableRef().apPass, v.c_str(), 63);
    else if (c==chMode) store->mutableRef().mode = (v=="BRIDGE"?RepeaterMode::BRIDGE:RepeaterMode::NAT);
    else if (c==chCommit) store->save();
  }
 public: ConfigStore* store; } cb;

void BleConfig::begin(ConfigStore& cfg, Logger& log) {
  store=&cfg; logger=&log; cb.store=store;
  BLEDevice::init("BW16-Repeater");
  srv = BLEDevice::createServer();
  svc = srv->createService(BLEUUID((uint16_t)0xFFF0));
  chStaSsid = svc->createCharacteristic(BLEUUID((uint16_t)0xFFF1), BLECharacteristic::PROPERTY_WRITE);
  chStaPass = svc->createCharacteristic(BLEUUID((uint16_t)0xFFF2), BLECharacteristic::PROPERTY_WRITE);
  chApSsid  = svc->createCharacteristic(BLEUUID((uint16_t)0xFFF3), BLECharacteristic::PROPERTY_WRITE);
  chApPass  = svc->createCharacteristic(BLEUUID((uint16_t)0xFFF4), BLECharacteristic::PROPERTY_WRITE);
  chMode    = svc->createCharacteristic(BLEUUID((uint16_t)0xFFF5), BLECharacteristic::PROPERTY_WRITE);
  chCommit  = svc->createCharacteristic(BLEUUID((uint16_t)0xFFF6), BLECharacteristic::PROPERTY_WRITE);
  chStaSsid->setCallbacks(&cb); chStaPass->setCallbacks(&cb); chApSsid->setCallbacks(&cb); chApPass->setCallbacks(&cb); chMode->setCallbacks(&cb); chCommit->setCallbacks(&cb);
  svc->start();
  BLEDevice::startAdvertising();
}

void BleConfig::loop() {}
#else
void BleConfig::begin(ConfigStore& cfg, Logger& log) { store=&cfg; logger=&log; }
void BleConfig::loop() {}
#endif