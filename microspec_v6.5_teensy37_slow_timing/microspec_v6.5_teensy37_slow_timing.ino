#include <elapsedMillis.h>
#include "c12880.h"
#include <SD.h>
#include <Arduino.h>


#define SPEC_TRG         A0
#define SPEC_ST          A1
#define SPEC_CLK         10
#define SPEC_VIDEO       A2

#if defined(CORE_TEENSY)
#define FLASH_TRIGGER    12
IntervalTimer flashTimer;
#endif
String incomingStr,  datastring;
String  fileName,folderName;
 char fileNameType[100]= "test.txt";
 char folderNameType[12]= "";
uint16_t data [C12880_NUM_CHANNELS];
uint16_t dataseries [30][C12880_NUM_CHANNELS];
int timearray [30], numberOfSample;
uint16_t  peakvalue;
int pixeltarget=52;
elapsedMillis timeElapsed;
boolean flashstarted=false;
uint16_t  setintensity =12000;
File myFile;
const int chipSelect =  BUILTIN_SDCARD;

C12880_Class spec(SPEC_TRG,SPEC_ST,SPEC_CLK,SPEC_VIDEO);

//SerialCommand sCmd(Serial);// the SerialCommand parser object
/******************************************************************************/
void setup(){
  #if defined(CORE_TEENSY)
  pinMode(FLASH_TRIGGER,OUTPUT);
  digitalWrite(FLASH_TRIGGER,LOW);
  #endif
  Serial.begin(115200); // Baud Rate set to 115200

  if (!SD.begin(chipSelect)) {
    Serial.println("initialization failed!");
    return;
  }
//  Serial.println("initialization done.");
  

  spec.begin();
  spec.set_integration_time(100);
}

void loop(){


//check if any input from serial port
   if (Serial.available () > 0){
   //send the input to process 
   processIncomingByte (Serial.read ());
   }  


  
}




// Subroutine to parse the input
void processData (String inputdata)
  {



 String inputString = inputdata;
 String timestamp;


    //     Serial.println(inputString.indexOf("IT"));

  if   (inputString.indexOf("IT")>-1)      //
   {         
         String AcqTime= inputString.substring(inputString.indexOf(":")+1 );
         int integ_time=AcqTime.toInt();
     spec.set_integration_time(integ_time);

   //    Serial.print("Acquisition time is set to: ") ;
  //       Serial.println(integ_time);
    }


 else if  (inputString.indexOf("Foldername")>-1)      // 
   {         
   folderName= inputString.substring(inputString.indexOf(":")+1 );
   folderName.toCharArray(folderNameType, sizeof(folderName));

    // see if the directory exists, create it if not.
if( !SD.exists(folderNameType) ) 
{
 SD.mkdir(folderNameType);
 
}

}
 else if  (inputString.indexOf("Filename")>-1)      // 
   {         
   fileName= inputString.substring(inputString.indexOf(":")+1 ) + ".txt";
  folderName += "/";
  fileName= folderName + fileName;
//  Serial.println(fileName);
  fileName.toCharArray(fileNameType, sizeof(fileName)+4);
//  Serial.println(fileNameType);
   myFile = SD.open(fileNameType, FILE_WRITE);
   myFile.print("  ");
//   myFile.close();

   }
  
  else if  (inputString.indexOf("READ")>-1)      // 
   {         
    String sampleNum= inputString.substring(inputString.indexOf(":")+1 );
    int numberOfSample=sampleNum.toInt();
    readspecdata(numberOfSample);
   }

  inputString="";

}  // end of processData
  



void processIncomingByte (char c)
  {
  switch (c)
    {
    case '!':   // end of text
      // terminator reached! process inputLine here ...
      processData (incomingStr);
     delayMicroseconds(10);       
      // reset for next time
      incomingStr="";  
      break;
  
    default:
      // keep adding
      incomingStr += c;
      break;
  
    }  // end of switch
  } // end of processIncomingByte






void readspecdata(int numberOfSample)
{
   char readchar;
//     Serial.print("Reading data..... ") ;
//     Serial.print("Initializing SD card...");




timeElapsed=0;
if (myFile) 
  {    
      for (int j = 0; j < 2000; j++)
      {
    
           spec.read_into(datastring, peakvalue);
            // Serial.println(peakvalue);
           
            if (peakvalue>setintensity && !flashstarted)
            {
              flashstarted=true;
              j=2000-numberOfSample;
             //Serial.println("Flash started");
            }
            
            
            if (flashstarted){
              myFile.print(timeElapsed,DEC);
              myFile.print(":");
              myFile.println(datastring + "!");
            }
      }
}  
  
  // close the file:
myFile.close();


//    Serial.println("Done");
//*******************************************************************************/
  
  // re-open the file for reading:
if (flashstarted)
  {
  myFile = SD.open(fileNameType);
  if (myFile) 
    {
     datastring="";
    // read from the file until there's nothing else in it:
     readchar=myFile.read();
      readchar=myFile.read();
       while (myFile.available()) 
         {
          readchar=myFile.read();
          if (readchar != '!')
          {
            datastring+=readchar;  
          }
          else
          {
            Serial.println(datastring.substring(0,768 + datastring.indexOf(":"))+">");
            while(Serial.read() != '>')
            {
                delayMicroseconds(10);       
            }
            Serial.println(datastring.substring(769 + datastring.indexOf(":"),datastring.length()) + "!");
            datastring="";
            while(Serial.read() != '!')
             {
               delayMicroseconds(10);       
             }
          }
         }
      
    // close the file:
    myFile.close();
    
  }
  else 
    {
    // if the file didn't open, print an error: 
    Serial.print("error opening ");
    Serial.println(fileNameType);
   }
    Serial.println ("*");
    flashstarted=false;
  }
}









 



/******************************************************************************/

