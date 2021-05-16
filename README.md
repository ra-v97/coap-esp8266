# CoAP-ESP8266
Implementation of basic functionality of CoAP protocol based on ESP8266WiFi library for Arduino. Protocol not contains all of functions described in RFC 7252 and was designed for real time control of robot, but can be used for another purposes.

## CoAP communication protocol - RFC 7252
Protocol description is available under <a href="https://datatracker.ietf.org/doc/rfc7252/?include_text=1">RFC 7252</a>

## Repository Content
* **/examples** - Example sketches for the library (.ino). Run these from the Arduino IDE. 
* **/src** - Source files for the library (.cpp, .h).
* **library.properties** - General library properties for the Arduino package manager.

## How to use
1. Download this library 
2. Put it into libraries folder of Arduino IDE
3. Create instance of COAP class
4. Add proper callbacks and resources
5. Test server with <a href="https://github.com/mkovatsc/Copper4Cr">Copper4Cr plugin</a> or manually send information compatible with CoAP protocol.

### Library features
- Server Side Functionality:
	- Methods - sends proper message request like REST methods
	  - GET
	  - PUT
	  - POST 
	  - DELETE 
    - Observe - add client to subscription list of resource and sends notifications to subscribers when resource has changed.
	- Ping 
	- Resource Discovery 
	- Block Transfer - not implemented because control messages are small and doesn't require this.

### Usage examples
- Server initialisation
    ```
    /*Define global at the beginning of sketch*/

    CoAP_Server server(); //use with default CoAP port: 5683

    CoAP_Server server(5000); //use custom port 5000 (Udp port)
    ```

- Add resource to server (see Resources info section to get more info)
    - Define callback function, supported are callback for **get** and **post** request. Callback has to be define in the sketch.
    ```
    void getCallback(){
      Serial.println("Received get request");
    }

    void postCallback(){
      char buf[20];
      size_t len;
      server.getResourceValueString("Temperature", buf, 20, &len);
      Serial.print("Received post: ");
      Serial.println(buf);
    }
    ```
    - Register resource on server
    ```
    /*registration of "Message" resource with initial byte value of "Hello world!", size 12 bytes and getCallback (pointer to function)*/
    server.resourceRegister("Message", (uint8_t*)"Hello world!", 12, getCallback);

    /*registration of "Temperature" resource with empty initial value, size 0, getCallback (pointer to function) and postCallback*/
    server.resourceRegister("Temperature", (uint8_t*)"", 0, getCallback, postCallback);
    ```
- Handling with resources
    - Reading resource value
    ```
    /*Reading value stored in "Message" resource and saving it to buf, in len size of resource is returned*/
    size_t bufferForResultSize = 20;
    char bufferForResult[bufferForResultSize];
    size_t lengthOfReturnedValue;
    server.getResourceValueString("Message", bufferForResultSize, bufferForResultSize, &lengthOfReturnedValue);

    /*Reading value stored in "IntResource" resource and returning as int value, Int must be stored on first four byte in buffer*/
    int value =  getResourceValueInt("IntResource");

    /*Reading value stored in "IntResource" resource and returning as float value, float must be stored on first four byte in buffer*/
    float value = getResourceValueFloat("FloatResource");
    ```
    - Updating resource on server
    ```
    /*Setting byte value into resource*/
    String resourceUri = String("Resource");
    String content = "Content";
    int correctlyAddedFlag =  updateResource(resourceUri, (uint8_t*) content.c_str(), content.length());

    /*Setting int value into buffer*/
    int newValueInt = 15;
    int correctlyAddedFlag =  updateResource(resourceUri, newValueInt);

    /*Setting float value into resource*/
    float newValueFloat = 1.0;
    int correctlyAddedFlag =  updateResource(resourceUri, newValueFloat);
    ```

- Loop function (must be called at least evey 5s in sketch *loop()* function)

   ```
   void loop() {
         server.communicationLoop();
   }
   ```

## Resource info
1. Resources are identified by *uri*. Each *uri* must by Arduino String.
2. Client can subscribe for the resource change notification by sending get request with *observe option*.
3. Removing from subscribers' list is performed by sending reset request to server.
4. Resources for notification are checked every 500ms, it can be change in *CoAP_Server.h* server configuration.
5. Available operations on server:
    ```
    bool resourceRegister(String uri, uint8_t *content, size_t bufSize, Callback resourceCallback);

    bool resourceRegister(String uri, uint8_t *content, size_t bufSize, Callback getCallback, Callback postCallback);

    bool resourceRegister(String uri, uint8_t *content, size_t bufSize);

    int deleteResource(String uri);

    int updateResource(String uri, uint8_t *content, size_t bufSize);

    int updateResource(String uri, int value);

    int updateResource(String uri, float value);

    int getResourceValueString(String uri, char* buffer, size_t bufSize, size_t* outputSize);

    int getResourceValueInt(String uri);

    float getResourceValueFloat(String uri);
    ```
## Sources and ideas
Some ideas comes from:
* <a href="https://github.com/nodemcu/nodemcu-firmware/tree/master/app/coap"> NodeMCU core </a>
* <a href="https://github.com/automote/ESP-CoAP">automote/ESP-CoAP library</a>

The library extends, supplements and adjusts automote/ESP-CoAP library.

## Author

* **Rafal Stachura**  -  [github](https://github.com/ra-v97)
