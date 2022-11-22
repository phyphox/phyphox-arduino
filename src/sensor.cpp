#include "phyphoxBleExperiment.h"
#include "copyToMem.h"
#include <sstream>

void PhyphoxBleExperiment::Sensor::setType(const char *t)
{
    ERROR = ERROR.MESSAGE == NULL ? err_checkSensor(t, "setType") : ERROR;
	copyToMem(&TYPE, (std::string(t)).c_str());
}

void PhyphoxBleExperiment::Sensor::mapChannel(const char *comp, int ch)
{
    for(int i=0;i<5;i++){
        if (COMPONENT[i]==NULL)
        {
            ERROR = ERROR.MESSAGE == NULL ? err_checkComponent(comp, "routeData") : ERROR;
            ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(ch,5, "routeData") : ERROR;
            copyToMem(&COMPONENT[i], (std::string(comp)).c_str());
            std::ostringstream s;
            s << ch;
            std::string temp = s.str();
            copyToMem(&CHANNEL[i], ("CB"+temp).c_str());
            break;
        }
    }
}
void PhyphoxBleExperiment::Sensor::setAverage(bool b)
{
    if(b){
        copyToMem(&AVERAGE, " average=\"true\"");
    }else{
        copyToMem(&AVERAGE, " average=\"false\"");
    }
}

void PhyphoxBleExperiment::Sensor::setRate(int r){
    ERROR = ERROR.MESSAGE == NULL ? err_checkUpper(r,100, "setRate") : ERROR;
	std::ostringstream s;
    s << r;
    std::string temp = s.str();
    copyToMem(&RATE, (" rate=\""+temp+"\"").c_str());
}

void PhyphoxBleExperiment::Sensor::setXMLAttribute(const char *xml){
	ERROR = ERROR.MESSAGE == NULL ? err_checkLength(xml, 98, "setXMLAttribute") : ERROR;
	copyToMem(&XMLAttribute, (" " + std::string(xml)).c_str());
}

void PhyphoxBleExperiment::Sensor::getBytes(char *buffArray)
{
    if(ERROR.MESSAGE == NULL){
        strcat(buffArray,"\t<sensor type=\"");
        strcat(buffArray,TYPE);
        strcat(buffArray,"\"");
        if (!RATE)  {strcat(buffArray," rate=\"80\"");} else {strcat(buffArray,RATE);}
        if (!AVERAGE)  {strcat(buffArray," average=\"true\"");} else {strcat(buffArray,AVERAGE);}
        if (XMLAttribute) {strcat(buffArray,XMLAttribute);}
        strcat(buffArray,">\n");
        //route components
        for (int i = 0; i < 5; i++){
            if(CHANNEL[i]!=NULL && COMPONENT[i]!=NULL){
                strcat(buffArray,"\t\t<output");
                strcat(buffArray," component=\"");
                strcat(buffArray,COMPONENT[i]);
                strcat(buffArray,"\"");
                strcat(buffArray,">");
                strcat(buffArray,CHANNEL[i]);
                strcat(buffArray,"</output>\n");
            }
        }
        strcat(buffArray,"\t</sensor>\n");
    }
    
}