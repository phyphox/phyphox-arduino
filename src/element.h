#ifndef ELEMENT
#define ELEMENT
#include <Arduino.h>

class Element
{
	public:

    	Element() {};                     
    	Element(const Element&) = delete;         
    	Element &operator=(const Element &) = delete;
		~Element() = default;

		int typeID = 0;

		char LABEL[50] = "Default Label";

		void setLabel(char*);
		virtual void getBytes(char*);


	private:


};

#endif
