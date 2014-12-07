#include "http_client.h"

#define DEBUG_SONOS
#define REQUEST_LEN 800


/*----------------------------------------------------------------------*/
/* Global variables */
/*----------------------------------------------------------------------*/


/* Ethernet control */
TCPClient       myClient; 

// response mainbuffer
char mainbuffer[1024];
char smallbuffer[85];


static const uint16_t REQUEST_TIMEOUT = 5000; // Allow maximum 5s between data packets.
int readbytes=0;

HTTPClient::HTTPClient()
{
}

int sendRequest (byte* host, unsigned short port, char* response, unsigned short responseSize) 
{
  if (myClient.connect(host, port)) {
      uint32_t startTime = millis();
      
      myClient.write((const uint8_t *)mainbuffer, strlen(mainbuffer));
      myClient.flush();
      
      while(!myClient.available() && (millis() - startTime) < REQUEST_TIMEOUT){
          SPARK_WLAN_Loop();
      };
      
      while(myClient.available()) {
          readbytes = myClient.read((uint8_t*) response, responseSize);
          if (readbytes == -1) break;
      }
      
      myClient.flush();
      myClient.stop();
      
  } else {
      // unable to connect
      return 1; 
  }
  
  return 0;
}



/*----------------------------------------------------------------------*/
/* make request - sends http request to a given host                    */

/* in the end the whole request is in the mainbuffer
   the smallbuffer is used to add the header files in the first part
   finally sendRequest(..) is called to actually execute the request
   
   PARAMETERS:
    type: 0 = GET, 1 = POST, 2 = PUT
    url: Path to the request
    host: bytefield with length of 4
          containing the bytes of the hosts IP
    keepAlive: boolean, send keep-alive otherwise send close
    userHeader1, userHeader2 additional headers, use empty string to skip
    content: the requests content
    //TODO: add parameter to make the requests flush() instead of read()
    
    HISTORY:
      12/7/14 PHHE
          Creted the method.


*/


int HTTPClient::makeRequest(unsigned short type, 
                const char* url, 
                byte* host, 
                unsigned short port,  
                boolean keepAlive, 
                const char* contentType, 
                const char* userHeader1, 
                const char* userHeader2, 
                const char* content,
                char* response,
                unsigned short responseSize)
{
  
  // clear both buffers
  //TODO there was a & in front of smallbuffer
  memset(&smallbuffer, 0, sizeof(smallbuffer));
  memset(&mainbuffer, 0, sizeof(mainbuffer));
  
  
  
  // add the first line of the header
  // scheme: [POST|GET] /url/... HTTP/1.1\r\n
  if (type == 0) {
    sprintf(mainbuffer, "GET ");
  } else if (type==1) {
    sprintf(mainbuffer, "POST ");    
  } else if (type==2) {
    sprintf(mainbuffer, "PUT ");
  } else {
    // undefined request type
    // return
    return 1;
  }
  strcat(mainbuffer, url);
  strcat(mainbuffer, " HTTP/1.1\r\n");
  
  // add connection field, either keep-alive or close
  if (keepAlive) {
    strcat(mainbuffer,  "CONNECTION: keep-alive\r\n");    
  } else {
    strcat(mainbuffer,  "CONNECTION: close\r\n");        
  }
  
  // add the host IP and port to the header
  snprintf(smallbuffer, 85, "HOST: %d.%d.%d.%d:%d\r\n", host[0], host[1], host[2], host[3], port);
  strcat(mainbuffer, smallbuffer);
  memset(&smallbuffer, 0, sizeof(smallbuffer));
    
  // add content length to the header
  snprintf(smallbuffer, 85, "CONTENT-LENGTH: %d\r\n" , strlen(content));
  strcat(mainbuffer, smallbuffer);
  memset(&smallbuffer, 0, sizeof(smallbuffer));
  
  // add the content type field
  snprintf(smallbuffer, 85, "CONTENT-TYPE: %s\r\n" , contentType);
  strcat(mainbuffer, smallbuffer);
  memset(&smallbuffer, 0, sizeof(smallbuffer));
  
  // optionally add the userHeader1  
  if (strlen(userHeader1)>0){
    strcat(mainbuffer, userHeader1);
    strcat(mainbuffer,  "\r\n");    
  }

  // optionally add the userHeader2
  if (strlen(userHeader2)>0){
    strcat(mainbuffer, userHeader2);
    strcat(mainbuffer,  "\r\n");    
  }

  // end of header
  // one additional line break
  strcat(mainbuffer,  "\r\n");
  
  // add the content
  strcat(mainbuffer, content);
  
  // actually send the stored request
  // return the return value of the send method
  return sendRequest(host, port, response, responseSize);
  
}




// additional methods to parse responses
// xml parser:

// # simple parsing of the responses ------------------------------
//
// find <element>X</element> and return X as a String
//
String getXMLElementContent(String input, String element)
{
    // make open and close string
    String elementOpen = "<" + element + ">";
    String elementClose = "</" + element + ">";
    
    // get positions of open and close
    int openPos = input.indexOf(elementOpen);
    int closePos = input.indexOf(elementClose);
    
    #ifdef SERIALDEBUGPARSER
        Serial.print("[INF] Input length: ");
        Serial.println(input.length());
        Serial.println("[INF] Elements:");
        Serial.println(elementOpen);
        Serial.println(elementClose);
        Serial.println("[INF] Positions:");
        Serial.println(openPos);
        Serial.println(closePos);
    #endif // SERIALDEBUG
    
	
    // verify if open and close can be found in the input
    if (openPos == -1 || closePos == -1) {
		Serial.println("[ERR] can not find element in input");
		return NULL;
	}
    
    // idx: starts at index of element + length of element + tags
    int idx = openPos + elementOpen.length();
 
    // check if length is above 0
    if (closePos - idx > 0) {
        #ifdef SERIALDEBUGPARSER
            Serial.print("[INF] Output length: ");
            Serial.println((input.substring(idx,  closePos)).length());
        #endif
        return input.substring(idx,  closePos);

    } else {
        #ifdef SERIALDEBUGPARSER
            Serial.println("[ERR] idx seems wrong");
        #endif
        return NULL;
        
    }
    
}
