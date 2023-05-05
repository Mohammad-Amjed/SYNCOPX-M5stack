#include <M5Core2.h>
#include <WiFi.h>
#include <EMailSender.h>

//the network to connect to
const String ssid = "wifi network name";
const String password = "wifi natwork password";

//teh server that we will send and receive email messages through it and its port
const char* smtp_server = "smtp.gmail.com";
const int smtp_port = 587;

//the email of the m5stack and its password
const char* email_username = "email address that will be used to send the email";
const char* email_password = "its password";

//using the library EMailSender to assign email_username as the sender
EMailSender emailSend(email_username, email_password);

//the email of the recipient
const char* recipient = "recipient email";

// Set up variables for gyroscope and accelerometer readings
float gyroX, gyroY, gyroZ;
float accX, accY, accZ;
float pitch, roll, yaw;

// Set up variables for detecting fainting events
bool isFainting = false;
unsigned long lastFaintingTime = 0; //unsigned long is used because it doesn't take negative values
unsigned long faintingDuration = 0;
unsigned long startTime = 0;

// FUNCTIONS:

//Vibration function should receive the time needed for vibration as a parameter
void vibration(int t) {
  M5.Axp.SetLDOEnable(3, true);
  delay(t);
  M5.Axp.SetLDOEnable(3, false);
}

//The sendEmails function uses the object "EmailSender" created and specify the message sent
void sendEmails(){
  M5.Lcd.clear();
  M5.Lcd.fillScreen(TFT_BLACK); // clear the screen
  M5.Lcd.setTextWrap(true, true);
  M5.Lcd.drawString("Email is being sent!", 10, (int)M5.Lcd.height() / 2, 2); 
  EMailSender::EMailMessage message;
  message.subject = "Your relative has Fainted!";
  message.message = "I am sending this email to inform you that your relative who has diabetes has Fainted. Move as soon as possible to help him/her!";
  EMailSender::Response resp = emailSend.send(recipient, message);
}

//check if the faint is real or not
bool falseAlarm(){
  isFainting = true; 
  startTime = millis();
  while (millis() - startTime < 5000) { //give the user 5 seconds to responce
    if (isFainting == true) {
      vibration(200);
      M5.Lcd.clear();
      M5.Lcd.setTextSize(2);  //Set the font size. 
      M5.Lcd.setTextDatum(ML_DATUM); 
      M5.Lcd.setCursor(10, 10);  //Move the cursor position to (x,y). 
      M5.Lcd.printf("Faint Detected!");  
      M5.Lcd.setCursor(10, 10+25);  //Move the cursor position to (x,y). 
      M5.Lcd.printf("False Alarm?");  
      M5.Lcd.setCursor(10, 10+50);  //Move the cursor position to (x,y). 
      M5.Lcd.printf("Press Button 'A'");  
      M5.update();    
    }
    if (M5.BtnA.wasPressed()) { //if the user pressed button A, cancel the alarm
      isFainting = false;
      M5.Lcd.clear();
      M5.Lcd.fillScreen(TFT_BLACK); // clear the screen
      M5.Lcd.setTextWrap(true, true);
      M5.Lcd.setCursor(10, 10);  //Move the cursor position to (x,y). 
      M5.Lcd.printf("Canceling Alarm System");  
      M5.Lcd.setCursor(10, 10+25);  //Move the cursor position to (x,y). 
      M5.Lcd.printf("False Alarm");   
      delay(2000);    
    }
  }
  M5.Lcd.clear();
  return isFainting;
}

// prints the instructions to the people around the user and play a DingDong voice to grab their attention to the user
void alarm_and_instructionos(){
  M5.Lcd.setTextSize(2);  //Set the font size. 
  M5.Lcd.fillScreen(TFT_BLACK); // clear the screen
  M5.Lcd.setCursor(10, 10);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("Follow the instructions:");  
  M5.Lcd.setCursor(10, 10+30);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("1- Lay user flat");  
  M5.Lcd.setCursor(10, 40+25);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("->if pregrnant:");  
  M5.Lcd.setCursor(10, 40+50);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("-->Lay on side");  
  M5.Lcd.setCursor(10, 40+75);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("2-Check breathing & pulse");  
  M5.Lcd.setCursor(10, 40+100);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("3- Raise user's legs");  
  M5.Lcd.setCursor(10, 40+125);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("4- No responce after 1 min");
  M5.Lcd.setCursor(10, 40+150);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("->Call 999");  


  M5.Spk.DingDong(); //Play DingDong
  M5.update();
}

//if the 5 seconds has passed without any responce, the email is sent and the instructions are printed
void alert() {
  sendEmails();
  while (isFainting == true) {
    alarm_and_instructionos();
    if (M5.BtnB.wasPressed()) {
      isFainting = false;
      faintingDuration = millis() - startTime;
      M5.Lcd.clear();
      M5.Lcd.setTextSize(2);  //Set the font size. 
      M5.Lcd.fillScreen(TFT_BLACK); // clear the screen
      M5.Lcd.drawString("Fainting duration is: " + String(faintingDuration/1000), 10, (int) M5.Lcd.height() / 2); 
      delay(2000);    
      M5.Lcd.clear();
      M5.Lcd.setTextSize(2);  //Set the font size. 
      M5.Lcd.fillScreen(TFT_BLACK); // clear the screen
      M5.Lcd.drawString("Restarting..", 10, (int) M5.Lcd.height() / 2); 
      delay(2000); 
      M5.Lcd.clear();

      }
  }
}

void setup() {
  // put your setup code here, to run once:
  M5.begin();
  M5.Spk.begin();     // Initialize the speaker. 
  Serial.begin(9600);
  M5.Lcd.clear();

  // Connect to WiFi
  
  M5.Lcd.print("Connecting to ");
  M5.Lcd.print(ssid);
  delay(2000);
  WiFi.begin(ssid.c_str(), password.c_str());
  
  //if the m5stack doesn't connect to wifi it will print dots
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    M5.Lcd.print(".");
  }
  
  M5.Lcd.println("Connected to WiFi!");
  delay(2000);
  
  M5.IMU.Init();
  M5.Lcd.fillScreen(BLACK);  //Set the screen background color to black. 
  M5.Lcd.setTextColor(WHITE, BLACK);  //Sets the foreground color and background color of the displayed text.  
  M5.Lcd.setTextSize(2);  //Set the font size. 


}

void loop() {
  // put your main code here, to run repeatedly:
  m5.update();

  M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
  M5.IMU.getAccelData(&accX, &accY, &accZ);  //Stores the triaxial accelerometer.
  M5.IMU.getAhrsData(&pitch, &roll, &yaw);  //Stores the inertial sensor attitude.

  // printing the readings on the screen
  M5.Lcd.setCursor(0, 20);  //Move the cursor position to (x,y). 
  M5.Lcd.printf("gyroX,  gyroY, gyroZ");  //Screen printingformatted string. 
  M5.Lcd.setCursor(0, 42);
  M5.Lcd.printf("%6.2f %6.2f%6.2f degree/s", gyroX, gyroY, gyroZ);

  M5.Lcd.setCursor(0, 70);
  M5.Lcd.printf("accX,   accY,  accZ");
  M5.Lcd.setCursor(0, 92);
  M5.Lcd.printf("%5.2f  %5.2f  %5.2f G", accX, accY, accZ);

  M5.Lcd.setCursor(0, 120);
  M5.Lcd.printf("pitch,  roll,  yaw");
  M5.Lcd.setCursor(0, 142);
  M5.Lcd.printf("%5.2f  %5.2f  %5.2f deg", pitch, roll, yaw); 

  //check for potential fainting 
  if (abs(gyroX) > 200 && 70 < abs(pitch) < 90  || abs(gyroZ) > 200 && 0 < abs(roll) < 20  || abs(gyroZ) > 200 && 160 < abs(roll) < 180) {
    if (falseAlarm() == true){
      alert();
    }
  }
  delay(100);
}