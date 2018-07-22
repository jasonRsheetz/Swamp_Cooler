#include <TempSensor.h>
#include <SoftwareSerial.h>
#include <util/delay.h>
#include <avr/io.h>

//counting variables
uint8_t elapsedSeconds = 0;
uint8_t elapsedMinutes = 0;
uint8_t oneMinute = 60;
uint16_t count = 0;

//temp and humidity variables
uint8_t temperature = 0;
uint8_t humidity = 0;

uint32_t thirteenMinutes = 780000;

//class variables
  //note that the memory-mapped address of portb is passed to the funciton
TempSensor Temp(&PORTB, &DDRB, &PINB, 7);
SoftwareSerial mySerialWifi(5,6);



int main(void)
{
  
//start off with a temp reading
elapsedMinutes = 15;

//allow the wifi module to join the wifi network for the initial temp reading, delay one minute
_delay_ms(60000);

//set up the timer
InitTimer0();

  //coutinue forever
  while(1)
  {

    //if one minutes is reached, increase the minute count
    if(elapsedSeconds == oneMinute)
    {
      elapsedMinutes++;

      //reset the count variables that increase the minute count
      elapsedSeconds = 0;
      count = 0;
    }
    
    //if 5 minutes has passed upload a new data point
    if(elapsedMinutes >= 15)
    {
       
        //reset the minute and second counters
        elapsedSeconds = elapsedMinutes = count = 0;
        
        //get new temp and humidity data
        Temp.Read();
        temperature = Temp.GetTemp();
        humidity = Temp.GetHumidity();
        
        //upload temp 
        UploadData(temperature, 1);
        
        //wait 20 seconds, required interval between thingspeak uploads
        _delay_ms(20);
        
        //upload humidity
        UploadData(humidity, 2);

        //sleep the wifi module for 13 minutes, this will allow enough time for re-up with the wifi network
        mySerialWifi.println("AT+GSLP=780000");
        
    }//end 15 min counter loop 

  }//end while loop
  
}//end main

void UploadData(int _data, int _field)
{

  //establish connection with thingspeak.com
  mySerialWifi.println("AT+CIPSTART=\"TCP\",\"184.106.153.149\",80");
  delay(2000);

  //the length of the web address for thingspeak is 40 characters, plus to for the newline and carriage return characters
  uint8_t webAddressLength = 42;
  //find the length of the data
  String dataToString = String(_data);
  uint8_t dataLength =dataToString.length()+webAddressLength;
  String dataLengthToString = String(dataLength);
  String dataLengthcmd = "AT+CIPSEND=";
  dataLengthcmd += dataLengthToString;
  //send the data length
  mySerialWifi.println(dataLengthcmd);
  delay(2000);

  String dataToSend = "GET /update?key=51BRH7LXM19P8X92&field";
  dataToSend += String(_field);
  dataToSend += "=";
  
  dataToSend += dataToString;
  mySerialWifi.println(dataToSend);
  delay(500);
  mySerialWifi.println("AT+CIPCLOSE");

}
//---------------------END UploadData FUNCTION---------------------//

//---------------------BEGIN initTimer1 FUNCTION---------------------//
//InitTImer0
//purpose: to count in approximatly 1 second intervals
//parameters: none
//returns: nothing
void InitTimer0(void)
{
  //disable interrupts
  cli();
  
  //clear timer registers, timer running in normal mode
  TCCR0A = 0x00;
  TCCR0B = 0x00;
  
  //prescaler of 1024
  TCCR0B |= (1 << CS00);
  TCCR0B |= (1 << CS02);
  
  //enable timer overflow interrupt
  TIMSK |= (1 << TOIE0);
  
  //enable global interrupts
  sei();
}

//---------------------END initTimer1 FUNCTION---------------------//


//---------------------BEGIN initTimer1 ISR FUNCTION---------------------//
ISR(TIMER0_OVF_vect)
{
  //increase the seconds count
    count++; 

    //increase seconds if enough overflows have occured
    if(count == 61)
    {
      elapsedSeconds++;
    }
    
}
//---------------------END initTimer1ISR FUNCTION---------------------//


