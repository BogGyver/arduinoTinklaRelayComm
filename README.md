# arduinoTinklaRelayComm
Allows one to have an Arduino device receive info from Tinkla Relay to control other devices

* Arduino needs to be powered separately and not from Tinkla Relay
* When controlling high output keep in mind the Ardoino can only handle a max of 900mA when powered separately and 400mA when powered from USB
* If you intend to drive LED lights or other devices, please ensure you compute the total max comsumption from all your attached devices
* Overloading the Tinkla Relay can result in the failure of the device
