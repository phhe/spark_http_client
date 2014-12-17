#ifndef __HTTPCLIENT_H
#define __HTTPCLIENT_H

//#include "allduino.h"
//#include <stdlib.h>
#include "spark_wiring_tcpclient.h"
#include "application.h"


/*----------------------------------------------------------------------*/
/* Macros and constants */
/*----------------------------------------------------------------------*/



class HTTPClient 
{

public:
	HTTPClient();
  virtual int makeRequest(unsigned short type, 
                  const char* url, 
                  byte* host, 
                  unsigned short port,  
                  boolean keepAlive, 
                  const char* contentType, 
                  const char* userHeader1, 
                  const char* userHeader2, 
                  const char* content,
                  char* response,
                  unsigned short responseSize, 
                  bool storeResponseHeader);
};


#endif /* __HTTPCLIENT_H */