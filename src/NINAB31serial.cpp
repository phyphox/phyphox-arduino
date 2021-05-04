#if defined(ARDUINO_SAMD_MKR1000)
#include "NINAB31serial.h"

String NINAB31Serial::m_input="";
bool NINAB31Serial::connected=false;


bool NINAB31Serial::begin(){
    Serial3.begin(115200);
    delay(1000);
    if(!checkResponse("ATE0",1000)){
        Serial.println("ERROR: Module not connected");
        return false;
    }
    return true;
}

bool NINAB31Serial::setLocalName(String name){
    if(name.length()>29){
        return false;
    }
    return checkResponse(String("AT+UBTLN=\"")+name+"\"", 1000);
}

bool NINAB31Serial::advertise(){
    return checkResponse(String("AT+UBTDM=3"), 1000);
}

bool NINAB31Serial::stopAdvertise(){
    return checkResponse(String("AT+UBTDM=1"), 1000);
}


bool NINAB31Serial::setConnectionInterval(int minInterval, int maxInterval){
    if(minInterval<32 || minInterval >16384 || maxInterval<32 || maxInterval >16384 || maxInterval<minInterval){
        return false;
    }
    return checkResponse(String("AT+UBTLECFG=1,")+minInterval,1000) && checkResponse(String("AT+UBTLECFG=2,")+maxInterval,1000);
}

bool NINAB31Serial::writeValue(int characteristic, String value){
    if(!connected){
        return false;
    }
    if(value.length()>40){
        return false;
    }
    return checkResponse(String("AT+UBTGSN=0,")+characteristic+","+value, 1000);
}

bool NINAB31Serial::writeValue(int characteristic, uint8_t* value, int len){
    if(!connected){
        return false;
    }
    if(len>20){
        return false;
    }
    auto msg=String("AT+UBTGSN=0,")+characteristic+",";
    for(int i=0;i<len;i++){
        msg+=String((value[i]&0xf0)>>4,HEX);
        msg+=String((value[i]&0xf),HEX);
    }
    return checkResponse(msg, 1000);
}



int NINAB31Serial::parseResponse(String cmd, uint32_t timeout){
  while(Serial3.available()){
    (Serial3.read());
  }
  Serial3.print(cmd);
  Serial3.write('\r');
  Serial3.flush();
  String input="";
  auto starttime=millis();
  while(millis()-starttime<timeout || timeout==0){
    if(Serial3.available()){
      input+=(char)(Serial3.read());
      if(input.endsWith("ERROR\r")){
        //Serial.println(String("error with command ")+cmd);
        return -1;
      }
      if(input.endsWith("OK\r")){
        int colonpos=input.indexOf(':');
        int commapos=input.indexOf(',');
        if(colonpos!=-1){
          if(commapos!=-1){
            return input.substring(colonpos+1,commapos).toInt();
          }else{
            return input.substring(colonpos+1).toInt();
          }
        }
        return -1;
      }
    }
  }
  return -1;
}

bool NINAB31Serial::checkResponse(String msg, uint32_t timeout){
  while(Serial3.available())Serial3.read(); //flush input buffer    
  Serial3.print(msg);
  Serial3.write('\r');
  Serial3.flush();
  String input="";
  auto starttime=millis();
  while(millis()-starttime<timeout || timeout==0){
    if(Serial3.available()){
      input+=(char)(Serial3.read());
      if(input.endsWith("ERROR\r")){
        return false;
      }
      if(input.endsWith("OK\r")){
        return true;
      }
    }
  }
  return false;
    
}

bool NINAB31Serial::checkUnsolicited(){
    if(m_input.indexOf("UUBTACLC:0")!=-1){
        connected=true;
        return true;
    }
    else if(m_input.indexOf("UUBTACLD:0")!=-1){
        connected=false;
        return true;
    }else{
        return false;
    }
}

bool NINAB31Serial::poll(){
    if(Serial3.available()){
      m_input+=(char)(Serial3.read());
      if(m_input.endsWith("\r")){
        if(checkUnsolicited()){
            flushInput();
            return false;
        }
        return true;
      }
    }
    return false;
}

String NINAB31Serial::checkCharWritten(int handle){
    if(m_input.indexOf("UUBTGRW:")!=-1){
        int commapos=m_input.indexOf(',');
        int seccommapos=m_input.indexOf(',',commapos+1);
        int thirdcommapos=m_input.indexOf(',',seccommapos+1);
        if(commapos != -1 && seccommapos != -1 && thirdcommapos != -1 && commapos<seccommapos && thirdcommapos > seccommapos){
            int rhandle=m_input.substring(commapos+1,seccommapos).toInt();
            if(rhandle==handle){
                return m_input.substring(seccommapos+1,thirdcommapos);
            }
        }
        
    }else{
        //Serial.println(m_input);
    }
    return "";
    
}

void NINAB31Serial::flushInput(){
    m_input="";
}

#endif


