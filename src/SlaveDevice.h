#ifndef SLAVEDEVICE_H
#define SLAVEDEVICE_H

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>
#include <functional>

class SlaveDevice {
  public:
  const char* deviceName;

    SlaveDevice(const char* deviceName, BLEUUID serviceUUID, BLEUUID characteristicUUID)
      : deviceName(deviceName), serviceUUID(serviceUUID), characteristicUUID(characteristicUUID) {
        pClient = nullptr;
        pRemoteCharacteristic = nullptr;
        deviceConnected = false;
        doConnect = false;
        pServerAddress = nullptr;
        gotHit = false;
    }

   
    void setBLEScan(BLEScan* scan) {
      pBLEScan = scan;
    }
       void setHitCallback(std::function<void()> callback) {
      hitCallback = callback;
    }
    bool isConnected() {
      return deviceConnected;
    }

    void attemptConnection() {
      if (doConnect) {
        Serial.print("Attempting connection to ");
        Serial.println(deviceName);
        if (connectToServer()) {
          Serial.print("We are now connected to the BLE Server: ");
          Serial.print(deviceName);
          pRemoteCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*) notificationOn, 2, true);
          deviceConnected = true;
          Serial.println( deviceConnected ? " - Connected" : " - Failed to connect");
          pBLEScan->start(0, false);
        } else {
          Serial.print("Failed to connect to ");
          Serial.println(deviceName);
        }
        doConnect = false;
      }
    }
    void setOnDeviceFound(BLEAdvertisedDevice* advertisedDevice){
        if (!doConnect) {
          pServerAddress = new BLEAddress(advertisedDevice->getAddress());
          doConnect = true;
          pBLEScan->stop();
          Serial.print("Device ");
          Serial.print(deviceName);
          Serial.println(" found. Connecting...");
        }

    }
  

    void sendActivateCommand() {
      if (pRemoteCharacteristic != nullptr && pRemoteCharacteristic->canWrite()) {
        Serial.print(" - Writing 'start' value to server ");
        Serial.println(deviceName);
        pRemoteCharacteristic->writeValue("start");
        Serial.println(" - Write completed");
      } else {
        Serial.println(" - No write permission to server or characteristic is null");
      }
    }



    void setDoConnect(bool flag) {
      doConnect = flag;
    }

    void setServerAddress(BLEAddress* address) {
      pServerAddress = address;
    }

    bool isHit() {
      return gotHit;
    }

  private:


    bool connectToServer() {
      Serial.println("Creating client...");
      pClient = BLEDevice::createClient();
      Serial.print("Forming a connection to ");
      Serial.println(pServerAddress->toString().c_str());

      // Connect to the remote BLE Server.
      if (!pClient->connect(*pServerAddress)) {
        Serial.println("Failed to connect.");
        return false;
      }
      Serial.println(" - Connected to server");

      // Obtain a reference to the service we are after in the remote BLE server.
      BLERemoteService* pRemoteService = pClient->getService(serviceUUID);
      if (pRemoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        return false;
      }
      Serial.println(" - Found our service");

      // Obtain a reference to the characteristic in the service of the remote BLE server.
      pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristicUUID);
      if (pRemoteCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        return false;
      }
      Serial.println(" - Found our characteristic");
      if (pRemoteCharacteristic->canNotify()) {
        pRemoteCharacteristic->registerForNotify([this](BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
          this->notifyCallback(pBLERemoteCharacteristic, pData, length, isNotify);
        });
        Serial.println(" - Registered for notifications");
      }

      return true;
    }

    void notifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
      std::string value((char*)pData, length);
      Serial.print("Notification received from device: ");
      Serial.println(deviceName);
      Serial.println(value.c_str());
      if (value == "got hit") {
        gotHit = true;
        Serial.print("Slave got hit: ");
        Serial.println(deviceName);
        if (hitCallback) {
          hitCallback();
        }
      }
    }


    BLEUUID serviceUUID;
    BLEUUID characteristicUUID;
    BLEClient* pClient;
    BLERemoteCharacteristic* pRemoteCharacteristic;
    BLEAddress* pServerAddress;
    BLEScan* pBLEScan;
    bool deviceConnected;
    bool doConnect;
    bool gotHit;
    SlaveDevice* parent;
    std::function<void()> hitCallback;

    static const uint8_t notificationOn[2];
};

const uint8_t SlaveDevice::notificationOn[2] = {0x1, 0x0};

#endif // SLAVEDEVICE_H
