
/*
IoT Plant Monitor
Author: Rishabh Berlia

Thanks to @jimblom  : https://github.com/sparkfun/Photon_Battery_Shield
https://learn.sparkfun.com/tutorials/photon-battery-shield-hookup-guide
Code for the battery shield is taken from this tutorial.

Thanks to @JOEL_E_B :
https://learn.sparkfun.com/tutorials/sparkfun-inventors-kit-for-photon-experiment-guide
Code and explanation to post to phant is taken from this tutorial

*/

//You might want to add this libraries to your app on build.particle.com
// This #include statement was automatically added by the Particle IDE.
#include "SparkFunPhant/SparkFunPhant.h"

// This #include statement was automatically added by the Particle IDE.
#include "spark-dallas-temperature/spark-dallas-temperature.h"

// This #include statement was automatically added by the Particle IDE.
#include "OneWire/OneWire.h"

// This #include statement was automatically added by the Particle IDE.
#include "SparkFunMAX17043/SparkFunMAX17043.h"

#define ONE_WIRE_BUS_PIN 0


// Setup a oneWire instance to be used with D0
OneWire oneWire(ONE_WIRE_BUS_PIN);

// Pass the oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

void printTemperature(DeviceAddress deviceAddress);


// Assign the addresses of your 1-Wire temp sensors.
// Use the Adress Scanner sketch to get the address's of all the One-Wire devices

DeviceAddress Probe01 = { 0x28, 0x2, 0xB1, 0x4B, 0x7, 0x00, 0x00, 0xFC };
DeviceAddress Probe02 = { 0x28, 0x12, 0x4B, 0x4D, 0x7, 0x00, 0x00, 0xB5 };
DeviceAddress Probe03 = { 0x28, 0x46, 0xB4, 0x4B, 0x7, 0x00, 0x00, 0x87 };

double M1,M2,M3 = 0;   //To store the values of Soil Mositure from the SparkFun Soil Moisture Sensor
double T1,T2,T3 = 0; //To store the values of Soil Temperature from the Dallas Temperature Sensors
int plantM1 = A1;   //Declare a variable for the soil moisture sensor
int plantM2 = A2;   //Declare a variable for the soil moisture sensor
int plantM3 = A3;   //Declare a variable for the soil moisture sensor
int plantP1 = D1;   //Variable for Soil moisture power
int plantP2 = D2;   //Variable for Soil moisture Power
int plantP3 = D3;   //Variable for Soil moisture Power


//Rather than powering the sensor through the V-USB or 3.3V pins,
//we'll use a digital pin to power the sensor. This will
//prevent oxidation of the sensor as it sits in the corrosive soil. @Thanks to Ted from Tech Support for pointing this out.

// Phant (data.sparkfun.com) Keys and Server
// These keys are generated when you create a new stream on data.sparkfun.com, just copy and replce them with your keys.

const char server[] = "data.sparkfun.com"; // Phant destination server
const char publicKey[] = "n1rW4K9n1DuloGzNrW44"; // Phant public key
const char privateKey[] = "MogAMYkmo0hyDleokg66"; // Phant private key
Phant phant(server, publicKey, privateKey); // Create a Phant object


//LIPO
double voltage = 0; // Variable to keep track of LiPo voltage
double soc = 0; // Variable to keep track of LiPo state-of-charge (SOC)
//bool alert; // Variable to keep track of whether alert has been triggered


void setup()
{

Serial.begin(9600);   // open serial over USB

pinMode(plantP1, OUTPUT);//Set D0 as an OUTPUT
pinMode(plantP2, OUTPUT);//Set D1 as an OUTPUT
pinMode(plantP3, OUTPUT);//Set D2 as an OUTPUT

digitalWrite(plantP1, LOW);//Set to LOW so no power is flowing through the sensor
digitalWrite(plantP2, LOW);//Set to LOW so no power is flowing through the sensor
digitalWrite(plantP3, LOW);//Set to LOW so no power is flowing through the sensor

//This line creates a variable that is exposed through the cloud.
//You can request its value using a variety of methods
Particle.variable("plantM1", &M1, DOUBLE);
Particle.variable("plantM2", &M2, DOUBLE);
Particle.variable("plantM3", &M3, DOUBLE);
Particle.variable("plantT1", &T1, DOUBLE);
Particle.variable("plantT2", &T2, DOUBLE);
Particle.variable("plantT3", &T3, DOUBLE);

Particle.variable("voltage", &voltage, DOUBLE);
Particle.variable("percentage", &soc, DOUBLE);

// To read the values from a browser, go to:
// http://api.particle.io/v1/devices/{DEVICE_ID}/{VARIABLE}?access_token={ACCESS_TOKEN}

sensors.setResolution(Probe01, 10);
sensors.setResolution(Probe02, 10);
sensors.setResolution(Probe03, 10);

//From the Photon Battery Shield MAX17043_Simple.cpp example.

// Set up the MAX17043 LiPo fuel gauge:
lipo.begin(); // Initialize the MAX17043 LiPo fuel gauge

// Quick start restarts the MAX17043 in hopes of getting a more accurate guess for the SOC.
lipo.quickStart();


}

void loop()
{

delay(1000);

	// lipo.getVoltage() returns a voltage value (e.g. 3.93)
	voltage = lipo.getVoltage();
	// lipo.getSOC() returns the estimated state of charge (e.g. 79%)
	soc = lipo.getSOC();

	// Those variables will update to the Spark Cloud, but we can also print them locally over serial for debugging:
	Serial.print("Voltage: ");
	Serial.print(voltage);  // Print the battery voltage
	Serial.println(" V");

	delay(10);

	Serial.print("Percentage: ");
	Serial.print(soc); // Print the battery state of charge
	Serial.println(" %");
	Serial.println();

	delay(100);


	getMoistures();

	warningLight();

	getTemperatures();

	postToPhant();

	//Save Power

	delay(4000);//stay awake for 40 seconds to allow for App updates. 
	//This is very important, if this is not there, you will not be able to update code on photon easily.

	//Power down between sends to save power, measured in seconds.
	System.sleep(SLEEP_MODE_DEEP, 300);  //for Particle Photon 5 minute (300 seconds) Can be increased to your convienence. 

}

//This is a function used to get the soil moisture content from M1
int readM1()
{
	int m1;
	digitalWrite(plantP1, HIGH);//turn D6 "On"
	delay(10);//wait 10 milliseconds
	m1 = analogRead(plantM1);
	M1 = (((double)m1/1665)*100); //Convert into percentage
	digitalWrite(plantP1, LOW);//turn D6 "Off"
	return M1;
}

//This is a function used to get the soil moisture content from M2
int readM2()
{
	int m2;
	digitalWrite(plantP2, HIGH);//turn D6 "On"
	delay(10);//wait 10 milliseconds
	m2 = analogRead(plantM2);
	M2 = (((double)m2/3350)*100); //Convert into percentage
	digitalWrite(plantP2, LOW);//turn D6 "Off"
	return M2;


}

//This is a function used to get the soil moisture content from M3
int readM3()
{
	int m3;
	digitalWrite(plantP3, HIGH);//turn D6 "On"
	delay(10);//wait 10 milliseconds
	m3 = analogRead(plantM3);
	M3 = (((double)m3/3350)*100); //Convert into percentage
	digitalWrite(plantP3, LOW);//turn D6 "Off"
	return M3;

}

// postToPhant() gathers the sensor data, bundles it into a Phant post, and sends it out to data.sparkfun.com.
int postToPhant(void)
{
	Serial.println("Posting to Phant!"); // Debug statement

	// Use phant.add(<field>, <value>) to add data to each field.
	// Phant requires you to update each and every field before posting,
	// make sure all fields defined in the stream are added here.
	if( int(M1||M2||M3||T1||T2||T3||voltage||soc) < 0)
	Serial.println("Error, Negative Value!"); // Debug statement

	else{
	phant.add("m1", M1); // These first three phant adds set a field value to float variable
	phant.add("m2", M2); // The third parameter -- valid for float variables -- sets the number
	phant.add("m3", M3); // of decimal points after the number.
	phant.add("t1", T1); // of decimal points after the number.
	phant.add("t2", T2); // of decimal points after the number.
	phant.add("t3", T3); // of decimal points after the number.
	phant.add("vbat", voltage); // of decimal points after the number.
	phant.add("vpercentage", soc); // of decimal points after the number.

	// phant.particlePost() performs all of the Phant server connection and HTTP POSTING for you.
	// It'll either return a 1 on success or negative number on fail.
	// It uses the field/value combinations added previously to send Phant its data.
	// MAKE SURE YOU COMMIT A phant.add() FOR EVERY FIELD IN YOUR STREAM BEFORE POSTING!
	return phant.particlePost();
    }
}

void printTemperature(DeviceAddress deviceAddress)
{

float tempF = sensors.getTempF(deviceAddress);

	if (tempF == -127.00)
	{
	Serial.print("Error getting temperature  ");
	}
	else
	{
	Serial.print("F : ");
	Serial.print(tempF);
	}
	delay(10); 
}


void getMoistures()

{
	Serial.print("Soil Moisture from Plant 0 = ");
	//get soil moisture value from the function below and print it
	Serial.println(readM1());
	delay(10);
	Serial.print("Soil Moisture from Plant 1 = ");
	Serial.println(readM2());
	delay(10);
	Serial.print("Soil Moisture from Plant 2 = ");
	Serial.println(readM3());
	delay(10);
}

void warningLight()
{
    
	if(readM1() < 20 || readM2() < 20 || readM3() < 20)
	{
	// take control of the RGB LED
	RGB.control(true);
	RGB.color(255, 0, 0);//set RGB LED to Red
	}
	else
	{
	// resume normal operation
	RGB.control(false);
	}

	delay(100);
}


void getTemperatures()
{
	Serial.print("Getting temperatures... ");
	Serial.println();

	// Command all devices on bus to read temperature
	sensors.requestTemperatures();
	delay(10);


	Serial.print("Probe 01 temperature is:   ");
	printTemperature(Probe01);
	T1 = sensors.getTempF(Probe01);
	Serial.println();
	delay(10);

	Serial.print("Probe 02 temperature is:   ");
	printTemperature(Probe02);
	T2 = sensors.getTempF(Probe02);

	Serial.println();
	delay(10);


	Serial.print("Probe 03 temperature is:   ");
	printTemperature(Probe03);
	T3 = sensors.getTempF(Probe03);
	Serial.println();

	delay(10);

}


