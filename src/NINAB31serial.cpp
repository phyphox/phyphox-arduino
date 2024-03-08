#if defined(ARDUINO_SAMD_MKR1000) || defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
#include "NINAB31serial.h"

#if defined(ARDUINO_SAMD_MKR1000)
  #define SerialBLE Serial3
#elif defined(ARDUINO_SENSEBOX_MCU_ESP32S2)
  #define SerialBLE Serial1
#endif


String NINAB31Serial::m_input="";
bool NINAB31Serial::connected=false;

bool NINAB31Serial::configModule(){
    SerialBLE.print("AT+UMRS=115200,2,8,1,1\r"); //115200, no flow control, 8 data bits, 1 stop bit, no parity
    SerialBLE.print("AT&W0\r"); //write configuration to nonvolatile memory
    SerialBLE.print("AT+CPWROFF\r"); //restart module into new configuration
    SerialBLE.flush();
    delay(2000); //wait for module to come up
    /*digitalWrite(22,HIGH); //hardware power cycle - avoid if possible
    delay(500);
    digitalWrite(22,LOW);
    delay(1000);
    */
    return checkResponse("ATE0",500); //send command (echo off) and check if module is responding correctly
}

bool NINAB31Serial::begin(){
    SerialBLE.begin(115200);
    delay(500);
    checkResponse("AT",500); //throwaway command in case buffer gets reinitialized because SerialBLE.begin was already called
    for(int i=0;i<3;i++){ //try to send a command, in case it fails configure device and restart it
        if(checkResponse("ATE0",500)){
            return true;
        }else{
            if(configModule()){
                return true;
            }
        }
    }
    return false;
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
  while(SerialBLE.available()){
    (SerialBLE.read());
  }
  SerialBLE.print(cmd);
  SerialBLE.write('\r');
  SerialBLE.flush();
  String input="";
  auto starttime=millis();
  while(millis()-starttime<timeout || timeout==0){
    if(SerialBLE.available()){
      input+=(char)(SerialBLE.read());
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
  while(SerialBLE.available())SerialBLE.read(); //flush input buffer
  SerialBLE.print(msg);
  SerialBLE.write('\r');
  SerialBLE.flush();
  String input="";
  auto starttime=millis();
  while(millis()-starttime<timeout || timeout==0){
    if(SerialBLE.available()){
      input+=(char)(SerialBLE.read());
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
    if(SerialBLE.available()){
      m_input+=(char)(SerialBLE.read());
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


