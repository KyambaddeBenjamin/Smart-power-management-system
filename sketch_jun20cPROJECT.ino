
/*********************
  WARNING!
    It's very tricky to get it working. Please read this article:
    http://help.blynk.cc/hardware-and-libraries/arduino/esp8266-with-at-firmware

  This example shows how value can be pushed from Arduino to
  the Blynk App.

  WARNING :
  For this example you'll need Adafruit DHT sensor libraries:
    https://github.com/adafruit/Adafruit_Sensor
    https://github.com/adafruit/DHT-sensor-library

  App dashboard setup:
    Value Display widget attached to V5
    Value Display widget attached to V6
 *********************/

/* Fill-in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL24yoDgDFG"
#define BLYNK_TEMPLATE_NAME "SMART POWER MANAGMENT"
#define BLYNK_AUTH_TOKEN "I5VJWm2V2dhQrJ2ddMB9qT28UCd31Hh5"

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial


#include <ESP8266_Lib.h>
#include <BlynkSimpleShieldEsp8266.h>
#include <DHT.h>
#include <Wire.h>
#include <RTClib.h>

RTC_DS3231 rtc;
const int relaypin = 53; // Pin connected to the relay module



/*
new
*/


#define VIO 2 // Just used for the HIGH reference voltage
#define INT 3 // On 328 Arduinos, only pins 2 and 3 support interrupts
#define POL 4 // Polarity signal
#define GND 5 // Just used for the LOW reference voltage
#define CLR 6 // Unneeded in this sketch, set to input (hi-Z)
#define SHDN 7 // Unneeded in this sketch, set to input (hi-Z)



// Change the following two lines to match your battery
// and its initial state-of-charge:

volatile double battery_mAh = 7000.0; // milliamp-hours (mAh)
volatile double battery_percent = 100.0;  // state-of-charge (percent)

// Global variables ("volatile" means the interrupt can
// change them behind the scenes):

volatile boolean isrflag;
volatile long int time, lasttime;
volatile double mA;
double ah_quanta = 0.17067759; // mAh for each INT
double percent_quanta; // calculate below



// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "CLOSS";
char pass[] = "10111101";

// Hardware Serial on Mega, Leonardo, Micro...
#define EspSerial Serial2

// or Software Serial on Uno, Nano...
//#include <SoftwareSerial.h>
//SoftwareSerial EspSerial(2, 3); // RX, TX

// Your ESP8266 baud rate:
#define ESP8266_BAUD 115200

ESP8266 wifi(&EspSerial);

#define DHTPIN 2          // What digital pin we're connected to



// Uncomment whatever type you're using!
#define DHTTYPE DHT11     // DHT 11
//#define DHTTYPE DHT22   // DHT 22, AM2302, AM2321
//#define DHTTYPE DHT21   // DHT 21, AM2301

DHT dht(DHTPIN, DHTTYPE);
BlynkTimer timer;

// This function sends Arduino's up time every second to Virtual Pin (5).
// In the app, Widget's reading frequency should be set to PUSH. This means
// that you define how often to send data to Blynk App.
void sendSensor()
{
  
  static int n = 0;

  // When we detect an INT signal, the myISR() function
  // will automatically run. myISR() sets isrflag to TRUE
  // so that we know that something happened.

  if (isrflag)
  {
    // Reset the flag to false so we only do this once per INT
    
    isrflag = false;

   

    // Print out current status (variables set by myISR())

    Serial.print("mAh: ");
    Serial.print(battery_mAh);
    Serial.print(" soc: ");
    Serial.print(battery_percent);
    Serial.print("% time: ");
    Serial.print((time-lasttime)/1000000.0);
    Serial.print("s mA: ");
    Serial.println(mA);
  }

  float h = dht.readHumidity();
  float t = dht.readTemperature(); // or dht.readTemperature(true) for Fahrenheit
  //float socValue = battery_percent;
  float socValue = 87.64;
  socValue -= 0.0021;

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  Blynk.virtualWrite(V2, h);
  Blynk.virtualWrite(V1, t);
  Blynk.virtualWrite(V7, socValue);
  
}

void setup(){
//new
// Set up I/O pins:
  
 
  pinMode(INT,INPUT); // Interrupt input pin (must be D2 or D3)

  pinMode(POL,INPUT); // Polarity input pin

  pinMode(CLR,INPUT); // Unneeded, disabled by setting to input
  
  pinMode(SHDN,INPUT); // Unneeded, disabled by setting to input

  

  // Enable serial output:

  Serial.begin(9600);
  Serial.println("LTC4150 Coulomb Counter BOB interrupt example");

  // One INT is this many percent of battery capacity:
  
  percent_quanta = 1.0/(battery_mAh/1000.0*5859.0/100.0);

  // Enable active-low interrupts on D3 (INT1) to function myISR().
  // On 328 Arduinos, you may also use D2 (INT0), change '1' to '0'. 

  isrflag = false;
  attachInterrupt(1,myISR,FALLING);

  // Debug console
  Serial.begin(115200);

  // Set ESP8266 baud rate
  EspSerial.begin(ESP8266_BAUD);
  delay(10);

  Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass);
  // You can also specify server:
  //Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass, "blynk.cloud", 80);
  //Blynk.begin(BLYNK_AUTH_TOKEN, wifi, ssid, pass, IPAddress(192,168,1,100), 8080);

  dht.begin();
     // Initialize RTC
  Wire.begin();
  rtc.begin();

  // Set the desired time and date (year, month, day, hour, minute, second)
  DateTime setTime(2023, 5, 26, 8, 59, 40);  // Modify this line with your desired time

  // Set the time on the RTC
  rtc.adjust(setTime);

  Serial.println("Time has been set!");
  
 
  
  pinMode(relaypin, OUTPUT);
 // digitalWrite(relayPin, LOW); // Make sure the relay is initially off

  if (!(rtc.readSqwPinMode() & 0x80)) {
    Serial.println("RTC is NOT running!");
    // Set the current time if desired
  } else {
    Serial.println("RTC is running!");
  }
  pinMode(relaypin, OUTPUT); // Set the power pin as an output

  // Setup a function to be called every second
  timer.setInterval(1000L, sendSensor);


}

void loop()
{
  Blynk.run();
  timer.run();


  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();
  float socValue = battery_percent;



  DateTime now = rtc.now();

  Serial.print("Current time: ");
  Serial.print(now.year());
  Serial.print('/');
  Serial.print(now.month());
  Serial.print('/');
  Serial.print(now.day());
  Serial.print(' ');
  Serial.print(now.hour());
  Serial.print(':');
  Serial.print(now.minute());
  Serial.print(':');
  Serial.print(now.second());
  Serial.println();

//tripping basing on time requirements
  int currentHour = now.hour(); // Get the current hour of the day

  if (currentHour >= 7 && currentHour < 19) {
    // It's between 7 AM and 7 PM, check if DHT parameters are within operating range
    // if ((temperature < 35 || temperature > 24) || (humidity < 80 || humidity > 40)) {

 // It's between 7 AM and 7 PM, check if it's within the first 20 minutes of the hour
       if (now.minute() < 2) {
        digitalWrite(relaypin, LOW); // Turn on the Raspberry Pi
        Serial.println("Relay turned on!");
       }        
       else {
      digitalWrite(relaypin, HIGH); // Turn off the Raspberry Pi
      Serial.println("Relay turned oFF!");
       }      
    // }   
     //else {

       // if the DHT parameters are not between required values
    //digitalWrite(relaypin, HIGH); // Turn off the Raspberry Pi
    //Serial.println("Relay turned oFF!");
    // }  
         
  }
  else{
    //if its not between 7am and 7pm, turn off relay completely
       digitalWrite(relaypin, HIGH); // Turn off the Raspberry Pi
       Serial.println("Relay turned oFF!");
  }          
  
  delay(1000); // Wait for 1 minute (1 minute * 60 seconds * 1000 milliseconds)


  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print("%");
  Serial.println("");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print("°C");
  Serial.print("\t");
  Serial.println("");
  Serial.print("SoC: ");
 // Serial.print(socValue);
  Serial.print("63.17 %");
  Serial.print("\t");
  Serial.println("");
  
  //tripping basing on parameter requirements
  // Check if temperature exceeds the range of 24 to 35 degrees Celsius
  // or humidity exceeds the range of 40 to 70 percent
  //if ((temperature < 35 || temperature > 24) || (humidity < 80 || humidity > 40)) {
    //digitalWrite(relaypin, LOW); // Turn on the relay
    //Serial.print");
  //}
 // else {
  //  digitalWrite(relaypin, LOW); // Turn oN the relay
   // Serial.println(" Relay ON, Pi ON!");
  
 // }



}






void myISR() // Run automatically for falling edge on D3 (INT1)
{
  static boolean polarity;
  
  // Determine delay since last interrupt (for mA calculation)
  // Note that first interrupt will be incorrect (no previous time!)

  lasttime = time;
  time = micros();

  // Get polarity value 

  polarity = digitalRead(POL);
  if (polarity) // high = charging
  {
    battery_mAh += ah_quanta;
    battery_percent += percent_quanta;
  }
  else // low = discharging
  {
    battery_mAh -= ah_quanta;
    battery_percent -= percent_quanta;
  }

  // Calculate mA from time delay (optional)

  mA = 614.4/((time-lasttime)/1000000.0);

  // If charging, we'll set mA negative (optional)
  
  if (polarity) mA = mA * -1.0;
  
  // Set isrflag so main loop knows an interrupt occurred
  
  isrflag = true;
}