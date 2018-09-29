#define PIN_F 2
#define PIN_25 3
#define PIN_50 4
#define PIN_75 5
#define PIN_100 6
#define HARD_SWITCH 8
#define PIN_P 7

String string = "no data";

volatile int NbTopsFan; //measuring the rising edges of the signal

int Calc = 0; 
int flowCheck = 0;

boolean calcWaterFlowVolume = false;
boolean pumpOnFlag = false;
boolean tankFull = false;
boolean automaticMode = false;
boolean startFor = false;
boolean timeStartStop = false;

// these are the button constants
// Button timing variables
int debounce = 20;          // ms debounce period to prevent flickering when pressing or releasing the button
int DCgap = 250;            // max ms between clicks for a double click event
int holdTime = 1000;        // ms hold period: how long to wait for press+hold event
int longHoldTime = 3000;    // ms long hold period: how long to wait for press+hold event

// Button variables
boolean buttonVal = HIGH;   // value read from button
boolean buttonLast = HIGH;  // buffered value of the button's previous state
boolean DCwaiting = false;  // whether we're waiting for a double click (down)
boolean DConUp = false;     // whether to register a double click on next release, or whether to wait and click
boolean singleOK = true;    // whether it's OK to do a single click
long downTime = -1;         // time the button was pressed down
long upTime = -1;           // time the button was released
boolean ignoreUp = false;   // whether to ignore the button release because the click+hold was triggered
boolean waitForUp = false;        // when held, whether to wait for the up event
boolean holdEventPast = false;    // whether or not the hold event happened already
boolean longHoldEventPast = false;// whether or not the long hold event happened already

//Various functions used
void rpm();
int checkButton();
void clickEvent();
void doubleClickEvent();
void holdEvent();
boolean checkTankFull();
void waterFlowAndControl();
void tankStatusUpdate();
void automaticModeCheck();
int char_to_int(char ch);
void timeStartClickEvent();
void bluetoothDataDecode();
void writeString(String stringData); 
void refreshResponse();
void normalUpdateResponse();
void doTimeDifference();
void timeStartForClickEvent();
void timeStartStopFuction();
void starttimeinsecAssignment(char h1,char h2, char m1, char m2);
void stoptimeinsecAssignment(char h1,char h2, char m1, char m2);
void currAndroidTimeinsecAssignment(char h1,char h2, char m1, char m2);

//String and character to store commands from the android device
String androidString;
char command[31];
char temp;
boolean  flag = false;
int starttime = 0;
int stoptime = 0;
int starttimefor = 0;
int currAndroidTime = 0; //the time at which the request was made

//Variables for Time keeping functions
long startMillis = 0;
long currentMillis = 0;
double differenceInMillis = 0.0;
double differenceInMin = 0.0;
long timePackageTosend = 0;

long starttimeinsec = 0;
long stoptimeinsec = 0;
long currAndroidTimeinsec = 0;

long currMillisForSTST = 0;

void setup() {
  pinMode(PIN_25,INPUT);
  digitalWrite(PIN_25,HIGH);
  pinMode(PIN_50,INPUT);
  digitalWrite(PIN_50,HIGH);
  pinMode(PIN_75,INPUT);
  digitalWrite(PIN_75,HIGH);
  pinMode(PIN_100,INPUT);
  digitalWrite(PIN_100,HIGH);
  pinMode(PIN_F,INPUT);
  pinMode(HARD_SWITCH, INPUT);
  digitalWrite(HARD_SWITCH, HIGH);
  pinMode(PIN_P,OUTPUT);
    
  Serial.begin(9600);

  attachInterrupt(0,rpm,RISING);
}

void loop() {
  sei();
  int b = checkButton();
  if (b == 1) clickEvent();
  if (b == 2) doubleClickEvent();
  if (b == 3) holdEvent();

  bluetoothDataDecode();
   
  tankStatusUpdate();

  waterFlowAndControl();

  timeStartStopFuction();
  
  automaticModeCheck();
}

void bluetoothDataDecode(){
  if(Serial.available()>0){
    androidString = "";
    flag = true;
  }

  while(Serial.available() > 0)
    {
      temp = ((byte)Serial.read());
      
      if(temp == ':')
      {
        break;
      }
      else
      {
        androidString += temp;
      }
      
      delay(5);
    }

    if(flag == true){
      Serial.println(androidString);
      androidString.toCharArray(command,31);
      if(flag == true){
        int startt = 0;
        int stopt = 0;
        int starttf = 0;
        int currAndroidt = 0;
        if(command[0] == '0' && command[1] == '0' && command[2] == '0' && command[3] == '0' && command[4] == '0'){
          refreshResponse();
        } else if(command[0] == '1' && command[1] == '1' && command[2] == '1' && command[3] == '1' && command[4] == '1'){
          clickEvent();
        } else if(command[0] == '2' && command[1] == '2' && command[2] == '2' && command[3] == '2' && command[4] == '2'){
         // arduinoTimeStartStopRequestTime = millis();
          currMillisForSTST = millis();
          startt = (1000*char_to_int(command[10])) + (100*char_to_int(command[11])) + (10*char_to_int(command[12])) + (1*char_to_int(command[13]));
          stopt = (1000*char_to_int(command[18])) + (100*char_to_int(command[19])) + (10*char_to_int(command[20])) + (1*char_to_int(command[21]));
          currAndroidt = (1000*char_to_int(command[26])) + (100*char_to_int(command[27])) + (10*char_to_int(command[28])) + (1*char_to_int(command[29]));
          starttime = startt;
          stoptime = stopt;
          
          currAndroidTime = currAndroidt;

          starttimeinsecAssignment(command[10],command[11],command[12],command[13]);
          stoptimeinsecAssignment(command[18],command[19],command[20],command[21]);
          currAndroidTimeinsecAssignment(command[26],command[27],command[28],command[29]);
          
          timeStartClickEvent();
          Serial.println("starttime ");
          Serial.println(starttime);
          Serial.println("stoptime ");
          Serial.println(stoptime);
          Serial.println("currAndroidTime ");
          Serial.println(currAndroidTime);
          
        } else if(command[0] == '3' && command[1] == '3' && command[2] == '3' && command[3] == '3' && command[4] == '3'){
          turnPumpOn();
          starttf = (1000*char_to_int(command[10])) + (100*char_to_int(command[11])) + (10*char_to_int(command[12])) + (1*char_to_int(command[13]));
          starttimefor = starttf;
          startFor = true;
        }
        flag = false;
      }
    }
}

void starttimeinsecAssignment(char h1,char h2, char m1, char m2){
  int hours = 0;
  int mins = 0;
  hours = 10*char_to_int(h1) + (1*char_to_int(h2));
  mins = 10*char_to_int(m1) + (1*char_to_int(m2));

  starttimeinsec = (hours*3600) + (mins*60);
}

void stoptimeinsecAssignment(char h1,char h2, char m1, char m2){
  int hours = 0;
  int mins = 0;
  hours = 10*char_to_int(h1) + (1*char_to_int(h2));
  mins = 10*char_to_int(m1) + (1*char_to_int(m2));

  stoptimeinsec = (hours*3600) + (mins*60);
}

void currAndroidTimeinsecAssignment(char h1,char h2, char m1, char m2){
  int hours = 0;
  int mins = 0;
  hours = 10*char_to_int(h1) + (1*char_to_int(h2));
  mins = 10*char_to_int(m1) + (1*char_to_int(m2));

  currAndroidTimeinsec = (hours*3600) + (mins*60);
}

void timeStartStopFuction(){
  if(timeStartStop){

    long secPassed = 0;

    secPassed = doTimeDifferenceReturnSec();

    if((currAndroidTimeinsec+secPassed)>86400){
       currAndroidTimeinsec = 0;
    }

    if((currAndroidTimeinsec+secPassed) > (starttimeinsec-5) && (currAndroidTimeinsec+secPassed) < (starttimeinsec+5)){
      turnPumpOn();
    }

    if((currAndroidTimeinsec+secPassed) > (stoptimeinsec-5) && (currAndroidTimeinsec+secPassed) < (stoptimeinsec+5)){
      turnPumpOff();
    }
  }
}

void clickEvent() {
   flowCheck=0;
   pumpOnFlag =!pumpOnFlag;
   digitalWrite(PIN_P,pumpOnFlag);
   calcWaterFlowVolume = pumpOnFlag;
   if(pumpOnFlag){
    startMillis = millis(); 
   } else {
    startMillis = 0;
    holdEvent();
   }
}

void timeStartClickEvent(){
   flowCheck=0;
   timeStartStop = true;
}

void timeStartForClickEvent(){
  clickEvent();
  startFor = false;
}

void doubleClickEvent() {
  automaticMode = !automaticMode;
}

void holdEvent() {
   turnPumpOff();
   automaticMode = false;
   startFor = false;
   timeStartStop = false;
   startMillis = 0;
   starttime = 0;
   stoptime = 0;
   starttimefor = 0;
}

int checkTankStatus(){
  if(digitalRead(PIN_100) == LOW){
    if(digitalRead(PIN_75) == LOW && digitalRead(PIN_50) == LOW && digitalRead(PIN_25) == LOW){
      return 100;
    }
  }
  else if(digitalRead(PIN_75) == LOW){
    if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_50) == LOW && digitalRead(PIN_25) == LOW){
     return 75;
    }
  }
  else if(digitalRead(PIN_50) == LOW){
    if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_75) == HIGH && digitalRead(PIN_25) == LOW){
      return 50;
    }
  }
  else if(digitalRead(PIN_25) == LOW){
    if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_75) == HIGH && digitalRead(PIN_50) == HIGH){
      return 25;
    }
  }
  else if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_75) == HIGH && digitalRead(PIN_50) == HIGH && digitalRead(PIN_25) == HIGH){
    return 0;
  }
  else {
    return -1;
  }
}

void waterFlowAndControl(){
  if(calcWaterFlowVolume){
    NbTopsFan = 0;      //Set NbTops to 0 ready for calculations
    
    sei();            //Enables interrupts
    delay (1000);      //Wait 1 second
    cli();            //Disable interrupts
   
    delay(1000);
    Calc = (NbTopsFan * 60 / 7.5); //(Pulse frequency x 60) / 7.5Q, = flow rate in L/hour 

    normalUpdateResponse();

    doTimeDifference();
    
    if(Calc == 0){
      flowCheck++;
    }else{
      flowCheck = 0;
    }
    
    if(flowCheck>150){
      holdEvent();
    }
    
    if(tankFull == true){
     holdEvent();
    }

    if(startFor){
      long setForTime = (long) starttimefor;
      if(timePackageTosend >= setForTime){
        holdEvent();
      }
    }
  }
}

void tankStatusUpdate(){
  if(digitalRead(PIN_100) == LOW){
    if(digitalRead(PIN_75) == LOW && digitalRead(PIN_50) == LOW && digitalRead(PIN_25) == LOW){
      string = "100";
      tankFull = true;
    }
  }
  else if(digitalRead(PIN_75) == LOW){
    if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_50) == LOW && digitalRead(PIN_25) == LOW){
      string = "75";
      tankFull = false;
    }
  }
  else if(digitalRead(PIN_50) == LOW){
    if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_75) == HIGH && digitalRead(PIN_25) == LOW){
      string = "50";
      tankFull = false;
    }
  }
  else if(digitalRead(PIN_25) == LOW){
    if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_75) == HIGH && digitalRead(PIN_50) == HIGH){
      string = "25";
      tankFull = false;
    }
  }
  else if(digitalRead(PIN_100) == HIGH && digitalRead(PIN_75) == HIGH && digitalRead(PIN_50) == HIGH && digitalRead(PIN_25) == HIGH){
    string = "0";
    tankFull = false;
  }
  else {
    string = "error";
  }
}

void turnPumpOff(){
  pumpOnFlag = false;
  digitalWrite(PIN_P,pumpOnFlag);
  calcWaterFlowVolume = pumpOnFlag;
  startFor = false;
  timeStartStop = false;
  starttimeinsec = 0;
  stoptimeinsec = 0;
  currAndroidTimeinsec = 0;
}

void turnPumpOn(){
  pumpOnFlag = true;
  digitalWrite(PIN_P,pumpOnFlag);
  calcWaterFlowVolume = pumpOnFlag;
}


void rpm(){ //This is the function that the interupt calls 
  NbTopsFan++;  //This function measures the rising and falling edge of the hall effect sensors signal
}

int checkButton() {    
    int event = 0;
    buttonVal = digitalRead(HARD_SWITCH);
    // Button pressed down
    if (buttonVal == LOW && buttonLast == HIGH && (millis() - upTime) > debounce)
    {
        downTime = millis();
        ignoreUp = false;
        waitForUp = false;
        singleOK = true;
        holdEventPast = false;
        //longHoldEventPast = false;
        if ((millis()-upTime) < DCgap && DConUp == false && DCwaiting == true)  DConUp = true;
        else  DConUp = false;
        DCwaiting = false;
    }
    // Button released
    else if (buttonVal == HIGH && buttonLast == LOW && (millis() - downTime) > debounce)
    {        
        if (not ignoreUp)
        {
            upTime = millis();
            if (DConUp == false) DCwaiting = true;
            else
            {
                event = 2;
                DConUp = false;
                DCwaiting = false;
                singleOK = false;
            }
        }
    }
    // Test for normal click event: DCgap expired
    if ( buttonVal == HIGH && (millis()-upTime) >= DCgap && DCwaiting == true && DConUp == false && singleOK == true && event != 2)
    {
        event = 1;
        DCwaiting = false;
    }
    // Test for hold
    if (buttonVal == LOW && (millis() - downTime) >= holdTime) {
        // Trigger "normal" hold
        if (not holdEventPast)
        {
            event = 3;
            waitForUp = true;
            ignoreUp = true;
            DConUp = false;
            DCwaiting = false;
            //downTime = millis();
            holdEventPast = true;
        }
        // Trigger "long" hold
        if ((millis() - downTime) >= longHoldTime)
        {
            if (not longHoldEventPast)
            {
                event = 4;
                longHoldEventPast = true;
            }
        }
    }
    buttonLast = buttonVal;
    return event;
}

void automaticModeCheck(){
  if(automaticMode){
    int currentTankLevel = checkTankStatus();
    switch(currentTankLevel){
      case 100:
      turnPumpOff();
      break;
      case 75:
      break;
      case 50:
      turnPumpOn();
      break;
      case 25:
      turnPumpOn();
      break;
      case 0:
      turnPumpOn();
      break;
      case -1:
      //error();
      break;
      default :
      break;
    }
  }
}

int char_to_int(char ch){
  switch(ch){
    case '0':
      return 0;
      break;
    case '1':
      return 1;
      break;
    case '2':
      return 2;
      break;
    case '3':
      return 3;
      break;
    case '4':
      return 4;
      break;
    case '5':
      return 5;
      break;
    case '6':
      return 6;
      break;
    case '7':
      return 7;
      break;
    case '8':
      return 8;
      break;
    case '9':
      return 9;
      break;
    default:
      return 0;
      break;
  }
}

// Used to serially push out a String with Serial.write()
void writeString(String stringData) {
  for (int i = 0; i < stringData.length(); i++)
  {
    Serial.write(stringData[i]);   // Push each char 1 by 1 on each loop pass
  }
}

void refreshResponse(){
  String messagePart1 = "0:(";
  String messagePart2 = ")<";
  String messagePart3 = ">[";
  String messagePart4 = "]#";
  String message = "";
  message = messagePart1 + Calc + messagePart2 + string + messagePart3 + timePackageTosend + messagePart4;
  writeString(message);
}

void normalUpdateResponse(){
  String messagePart1 = "1:(";
  String messagePart2 = ")<";
  String messagePart3 = ">[";
  String messagePart4 = "]#";
  String message = "";
  message = messagePart1 + Calc + messagePart2 + string + messagePart3 + timePackageTosend + messagePart4;
  writeString(message);
}

void doTimeDifference(){
  currentMillis = millis();
  differenceInMillis = (currentMillis - startMillis)/1000;
  differenceInMin = differenceInMillis/60;
  timePackageTosend = (long) differenceInMin;
}

long doTimeDifferenceReturnSec(){
  currentMillis = millis();
  differenceInMillis = (currentMillis - currMillisForSTST)/1000;
  return differenceInMillis;
}
