#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

#define SERVICE_UUID1 "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID1 "cba1d466-344c-4be3-ab3f-189f80dd7518"
#define SERVICE_UUID2 "91bad492-b950-4226-aa2b-4ede9fa42f51"
#define CHARACTERISTIC_UUID2 "cba1d466-344c-4be3-ab3f-189f80dd7511"

BLEScan* pBLEScan;
BLEClient* pClient1;
BLEClient* pClient2;
BLERemoteCharacteristic* pRemoteCharacteristic1;
BLERemoteCharacteristic* pRemoteCharacteristic2;
bool deviceConnected1 = false;
bool deviceConnected2 = false;
bool doConnect1 = false;
bool doConnect2 = false;
int currentTarget = 0;
BLEAddress* pServerAddress1;
BLEAddress* pServerAddress2;
int devicesFound = 0;
String start = "start";
const uint8_t notificationOn1[] = {0x1, 0x0};
const uint8_t notificationOff1[] = {0x0, 0x0};
const uint8_t notificationOn2[] = {0x1, 0x0};
const uint8_t notificationOff2[] = {0x0, 0x0};
void chooseTarget();

void notifyCallback1(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  std::string value((char*)pData, length);
  Serial.print("Notification received from device 1: ");
  Serial.println(value.c_str());
  if(currentTarget == 1){
    if(value == "got hit"){
      chooseTarget();
    }
  }
}

void notifyCallback2(BLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  std::string value((char*)pData, length);
  Serial.print("Notification received from device 2: ");
  Serial.println(value.c_str());
   if(currentTarget == 2){
    if(value == "got hit"){
      chooseTarget();
    }
  }
}

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    Serial.print("Found Device: ");
    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.getName() == "BME280_ESP32") {
      if (!doConnect1) {
        pServerAddress1 = new BLEAddress(advertisedDevice.getAddress());
        doConnect1 = true;
        devicesFound++;
        pBLEScan->stop();
        Serial.println("Device 1 found. Connecting...");
      }
    } else if (advertisedDevice.getName() == "SLAVE_2") {
      if (!doConnect2) {
        pServerAddress2 = new BLEAddress(advertisedDevice.getAddress());
        doConnect2 = true;
        devicesFound++;
         pBLEScan->stop();
        Serial.println("Device 2 found. Connecting...");
      }
    }
    if (devicesFound == 2) {
      pBLEScan->stop();
      Serial.println("Both devices found. Stopping scan...");
      //chooseTarget();
    }
  }
};

class MyClientCallback : public BLEClientCallbacks {
  void onConnect(BLEClient* pClient) {
    Serial.println("Connected to server");
  }

  void onDisconnect(BLEClient* pClient) {
    Serial.println("Disconnected from server");
    if (pClient == pClient1) {
      deviceConnected1 = false;
      pClient1 = nullptr;
    } else if (pClient == pClient2) {
      deviceConnected2 = false;
      pClient2 = nullptr;
    }
    if (!deviceConnected1 || !deviceConnected2) {
      devicesFound = 0; // Reset the count to ensure scanning continues
      pBLEScan->start(0, false); // Restart scanning if not both devices connected
    }
  }
};

void sendActivateCommand(BLERemoteCharacteristic* pRemoteCharacteristic) {
  if (pRemoteCharacteristic != nullptr) {
    if (pRemoteCharacteristic->canWrite()) {
      Serial.println(" - Writing 'start' value to server");
      pRemoteCharacteristic->writeValue((uint8_t*)start.c_str(), start.length());
    } else {
      Serial.println(" - No write permission to server");
    }
  } else {
    Serial.println(" - Remote characteristic is null");
  }
}

void chooseTarget() {
  int randomDevice = random(1, 3);
  currentTarget = randomDevice;
  Serial.print("Current target: ");
  Serial.println(currentTarget);

  switch (currentTarget) {
    case 1:
       if (pRemoteCharacteristic1 != nullptr) {
    if (pRemoteCharacteristic1->canWrite()) {
      Serial.println(" - Writing 'start' value to server");
      pRemoteCharacteristic1->writeValue((uint8_t*)start.c_str(), start.length());
    } else {
      Serial.println(" - No write permission to server");
    }
  } else {
    Serial.println(" - Remote characteristic is null");
  }
      break;
    case 2:
          if (pRemoteCharacteristic2 != nullptr) {
    if (pRemoteCharacteristic2->canWrite()) {
      Serial.println(" - Writing 'start' value to server");
      pRemoteCharacteristic2->writeValue((uint8_t*)start.c_str(), start.length());
    } else {
      Serial.println(" - No write permission to server");
    }
  } else {
    Serial.println(" - Remote characteristic is null");
  }
      break;
    default:
      Serial.println("Invalid target selected.");
      break;
  }
}

void setup() {
  Serial.begin(9600);
  Serial.println("Starting BLE Client");
  BLEDevice::init("ESP32_Master");
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);
  Serial.println("Starting scan...");
  pBLEScan->start(0, false); // Set to 0 for continuous scanning
}

bool connectToServer(BLEAddress pAddress, BLEClient** pClient, BLERemoteCharacteristic** pRemoteCharacteristic, notify_callback notifyCallback, const BLEUUID& serviceUUID, const BLEUUID& characteristicUUID) {
  Serial.println("Creating client...");
  *pClient = BLEDevice::createClient();
  Serial.print("Forming a connection to ");
  Serial.println(pAddress.toString().c_str());

  // Connect to the remote BLE Server.
  bool connected = (*pClient)->connect(pAddress);
  if (!connected) {
    Serial.println("Failed to connect.");
    return false;
  }
  Serial.println(" - Connected to server");

  // Obtain a reference to the service we are after in the remote BLE server.
  BLERemoteService* pRemoteService = (*pClient)->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.print("Failed to find our service UUID: ");

    return false;
  }
  Serial.println(" - Found our service");

  // Obtain a reference to the characteristic in the service of the remote BLE server.
  *pRemoteCharacteristic = pRemoteService->getCharacteristic(characteristicUUID);
  if (*pRemoteCharacteristic == nullptr) {
    Serial.print("Failed to find our characteristic UUID: ");

    return false;
  }
  Serial.println(" - Found our characteristic");
  if ((*pRemoteCharacteristic)->canNotify()) {
    (*pRemoteCharacteristic)->registerForNotify(notifyCallback);
    Serial.println(" - Registered for notifications");
  }

  return true;
}

void loop() {
  if (doConnect1) {
    Serial.println("Attempting connection to Device 1");
    if (connectToServer(*pServerAddress1, &pClient1, &pRemoteCharacteristic1, notifyCallback1, BLEUUID(SERVICE_UUID1), BLEUUID(CHARACTERISTIC_UUID1))) {
      Serial.println("We are now connected to the BLE Server 1.");
      pRemoteCharacteristic1->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn1, 2, true);
      deviceConnected1 = true;
       if (pRemoteCharacteristic1->canWrite()) {
    Serial.println(" - Writing value to server");
    pRemoteCharacteristic1->writeValue((uint8_t*)start.c_str(), start.length());
  } else{
    Serial.println(" - No write permission to server");
  }
       pinMode(2, OUTPUT);
  digitalWrite(2, HIGH); 
        pBLEScan->start(0, false);
    } else {
      Serial.println("We have failed to connect to the server 1; Restart your device to scan for nearby BLE server again.");
    }
    doConnect1 = false;
  }

  if (doConnect2) {
    Serial.println("Attempting connection to Device 2");
    if (connectToServer(*pServerAddress2, &pClient2, &pRemoteCharacteristic2, notifyCallback2, BLEUUID(SERVICE_UUID2), BLEUUID(CHARACTERISTIC_UUID2))) {
      Serial.println("We are now connected to the BLE Server 2.");
      pRemoteCharacteristic2->getDescriptor(BLEUUID((uint16_t)0x2902))->writeValue((uint8_t*)notificationOn2, 2, true);
      deviceConnected2 = true;
       pBLEScan->start(0, false);
    } else {
      Serial.println("We have failed to connect to the server 2; Restart your device to scan for nearby BLE server again.");
    }
    doConnect2 = false;
  }

  delay(100); // Delay a second between loops.
}
