#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <Arduino.h>

#define SERVICE_UUID1 "91bad492-b950-4226-aa2b-4ede9fa42f59"
#define CHARACTERISTIC_UUID1 "cba1d466-344c-4be3-ab3f-189f80dd7518"
#define SERVICE_UUID2 "91bad492-b950-4226-aa2b-4ede9fa42f51"
#define CHARACTERISTIC_UUID2 "cba1d466-344c-4be3-ab3f-189f80dd7511"
#define SERVICE_UUID3 "91bad492-b950-4226-aa2b-4ede9fa42333"
#define CHARACTERISTIC_UUID3 "cba1d466-344c-4be3-ab3f-189f80dd7333"
#include "SlaveDevice.h"

BLEScan *pBLEScan;
int currentTarget = 1;
bool targetGotHit;

SlaveDevice slave1("SLAVE_1", BLEUUID(SERVICE_UUID1), BLEUUID(CHARACTERISTIC_UUID1));
SlaveDevice slave2("SLAVE_2", BLEUUID(SERVICE_UUID2), BLEUUID(CHARACTERISTIC_UUID2));
SlaveDevice slave3("SLAVE_3", BLEUUID(SERVICE_UUID3), BLEUUID(CHARACTERISTIC_UUID3));
void chooseTarget();

class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks
{
  void onResult(BLEAdvertisedDevice advertisedDevice)
  {
    Serial.print("Found Device: ");
    Serial.println(advertisedDevice.toString().c_str());
    if (advertisedDevice.getName() == slave1.deviceName)
    {
      slave1.setOnDeviceFound(&advertisedDevice);
    }
    else if (advertisedDevice.getName() == slave2.deviceName)
    {
      slave2.setOnDeviceFound(&advertisedDevice);
    }
    else if (advertisedDevice.getName() == slave3.deviceName)
    {
      slave3.setOnDeviceFound(&advertisedDevice);
    }
       if (slave1.isConnected() && slave2.isConnected() && slave3.isConnected())
  {
    pBLEScan->stop();
    targetGotHit = true;
  }
  }
};

void onSlaveHittedOne()
{
  Serial.println("Callback received: Slave 1 hit!");
  if (currentTarget == 1)
  {
    Serial.println("and its correct target!");
    targetGotHit = true;
  }

  // slave1.sendActivateCommand();
  // slave1.setDoConnect(false);
  
}

void onSlaveHittedTwo()
{
  Serial.println("Callback received: Slave 2 hit!");
  if (currentTarget == 2)
  {
    Serial.println("and its correct target!");
    targetGotHit = true;
  }
}
void onSlaveHittedThree()
{
  Serial.println("Callback received: Slave 2 hit!");
  if (currentTarget == 3)
  {
    Serial.println("and its correct target!");
    targetGotHit = true;
  }
}
void chooseTarget()
{
  int randomDevice = random(1, 4);
  currentTarget = randomDevice;
  Serial.print("Current target: ");
  Serial.println(currentTarget);

  switch (currentTarget)
  {
  case 1:
    slave1.sendActivateCommand();
    break;
  case 2:
    slave2.sendActivateCommand();
    break;
  case 3:
    slave3.sendActivateCommand();
    break;
  default:
    Serial.println("Invalid target selected.");
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting BLE Client");
  BLEDevice::init("ESP32_Master");
  pBLEScan = BLEDevice::getScan(); // Initialize BLE scan instance
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  slave1.setBLEScan(pBLEScan);
  slave2.setBLEScan(pBLEScan);
  slave3.setBLEScan(pBLEScan);

  // Start scanning once
  pBLEScan->setActiveScan(true);
  Serial.println("Starting scan...");
  pBLEScan->start(0, false);
  slave1.setHitCallback(onSlaveHittedOne);
  slave2.setHitCallback(onSlaveHittedTwo);
  slave3.setHitCallback(onSlaveHittedThree);
}

void loop()
{
  slave1.attemptConnection();
  slave2.attemptConnection();
  slave3.attemptConnection();
  if(targetGotHit){
    Serial.println("Sending start to new target!");
    targetGotHit = false;
    chooseTarget();
  } 
  delay(100);
}