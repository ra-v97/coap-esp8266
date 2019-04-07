# coap-esp8266
Implementation of basic functionality of CoAP protocol based on ESP8266WiFi library for Arduino. Protocol not contains all of functions described in RFC 7252 and was designed for real time control of robot, but can be used for another purposes.

## CoAP communication protocol - RFC 7252
Protocol description is available under <a href="https://datatracker.ietf.org/doc/rfc7252/?include_text=1">RFC 7252</a>

## Repository Content
* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager.
* **keywords.txt** - Contains the keywords for Arduino IDE.

## How to use
1. Download this library 
2. Put it into libraries folder of Arduino IDE
3. Create instance of COAP class
4. Add proper callacks and resources
5. Test server with <a href="https://github.com/mkovatsc/Copper4Cr">Copper4Cr plugin</a> or manualny send informations compatible with CoAP protocol.
6. Test client by sending messages.

### Library features
- Server Side Functionality:
	- Methods - sends proper message request like REST methods
	  - GET
	  - PUT
	  - POST 
	  - DELETE 
  - Observe - add client to subscription list of resource and sends notifications to subscriers when resource has changed.
	- Ping 
	- Resource Discovery 
	- Block Transfer - not implemented because controll messages are small and doesn't require this.

- Client Side Functionality:
	- Methods - sends proper message request like REST methods
	  - GET
	  - PUT
	  - POST 
	  - DELETE 
	- Observe - register client as observer of given resource.
	- Ping 

## Sources and ideas
Some ideas and implementation parts comes from:
* <a href="https://github.com/nodemcu/nodemcu-firmware/tree/master/app/coap"> NodeMCU core </a>
* <a href="https://github.com/automote/ESP-CoAP">automote/ESP-CoAP library</a>

The library extends, supplements and adjusts automote/ESP-CoAP library.
