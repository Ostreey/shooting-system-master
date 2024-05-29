#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

class BLEDeviceManager {
  public:
    BLEDeviceManager(const char* deviceName, const char* serviceUUID, const char* characteristicUUID){
        this->deviceName = deviceName;
        this->serviceUUID = serviceUUID;
        this->characteristicUUID = characteristicUUID;
    }

    void begin() {
      BLEDevice::init("ESP32_Master");
      pBLEScan = BLEDevice::getScan();
      pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks(this));
      pBLEScan->setActiveScan(true);
      pBLEScan->start(30);
    }

     void stop() {
      pBLEScan->stop();
    }
    bool update() {
      if (doConnect) {
        if (connectToServer(*pServerAddress)) {
          Serial.print("We are now connected to the BLE Server: ");
          Serial.println(deviceName);
          temperatureCharacteristic->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn, 2, true);
          connected = true;
        } else {
          Serial.print("Failed to connect to the server: ");
          Serial.println(deviceName);
        }
        doConnect = false;
      }
      return connected;
    }

    static void temperatureNotifyCallback(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
      // Store temperature value
      temperatureChar = (char*)pData;
      Serial.println(temperatureChar);
    }

  private:
    const char* deviceName;
    const char* serviceUUID;
    const char* characteristicUUID;
    BLEScan* pBLEScan;
    BLEClient* pClient;
    BLERemoteCharacteristic* temperatureCharacteristic;
    bool doConnect;
    bool connected;
    BLEAddress* pServerAddress;
    static char* temperatureChar;
    const uint8_t notificationOn[2] = {0x1, 0x0};
    const uint8_t notificationOff[2] = {0x0, 0x0};

    bool connectToServer(BLEAddress pAddress) {
      pClient = BLEDevice::createClient();
      Serial.print("Forming a connection to ");
      Serial.println(pAddress.toString().c_str());

      // Connect to the remote BLE Server.
      pClient->connect(pAddress);
      Serial.println(" - Connected to server");

      // Obtain a reference to the service we are after in the remote BLE server.
      BLERemoteService* pRemoteService = pClient->getService(BLEUUID(serviceUUID));
      if (pRemoteService == nullptr) {
        Serial.print("Failed to find our service UUID: ");
        Serial.println(serviceUUID);
        return false;
      }

      // Obtain a reference to the characteristic in the service of the remote BLE server.
      temperatureCharacteristic = pRemoteService->getCharacteristic(BLEUUID(characteristicUUID));
      if (temperatureCharacteristic == nullptr) {
        Serial.print("Failed to find our characteristic UUID: ");
        Serial.println(characteristicUUID);
        return false;
      }

      if (temperatureCharacteristic->canNotify()) {
        temperatureCharacteristic->registerForNotify(temperatureNotifyCallback);
      }

      return true;
    }

    class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
      public:
        MyAdvertisedDeviceCallbacks(BLEDeviceManager* manager) : manager(manager) {}

        void onResult(BLEAdvertisedDevice advertisedDevice) {
          if (advertisedDevice.getName() == manager->deviceName) { // Check if the name of the advertiser matches
    
            manager->pServerAddress = new BLEAddress(advertisedDevice.getAddress()); // Address of advertiser is the one we need
            manager->doConnect = true; // Set indicator, stating that we are ready to connect
            Serial.println("Device found. Connecting!");
          }
        }

      private:
        BLEDeviceManager* manager;
    };
};

// Static member initialization
char* BLEDeviceManager::temperatureChar = nullptr;
