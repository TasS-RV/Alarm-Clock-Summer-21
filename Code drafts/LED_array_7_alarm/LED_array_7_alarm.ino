
/* !This is a COMMON Cathode 7-segment array: so Digit Pins are 'ground': must write 0 to activate LED
   !The digit pins are also in reverse order (to RoboJax video diagram)
   Watch: https://www.youtube.com/watch?v=f3GqRMQ8jYs&t=908s at 2:26

   !Serial.print() SHOULD NOT BE USED in Multiplexing 'for' loop due to interrupts inteferring with very high refresh rate

   digitalWrite HIGH signal for Digit Pins: behave as Anode, writing HIGH connects to + voltage
   digitalWrite LOW signal for pins A-G effectively 'grounds' the LED: in a Common Cathode (pins A-G behave as negative terminal)
   In a Common Cathode 7-segment LED: the Digit pins behave as negative- and A-G as positive terminal, so digitalWrite
   signals would have to be reversed.

  Arduino arrays:
  type_stored array_name[][n]= {{n elements},{n elements},{n elements}};
  Number of square double-brackets after array_name indicates the dimension of the array: the number inside indicates
  the number of elements to be stored in each sub-array.
  NOTE! This is a 'bound', so more than n elements stored will cause an error. ONLY the first layer of the array does not need
  a set limit for the number of 'sub-arrays' or values it stores.

  Conversions:
  type variable1 = value;
  variable1 = (new_type) variable1; Is NOT valid format - cannot redefine the type except particular cases
  Particular cases include: String to Int - string_name.toint();

  Otherwise must (cast) the variable into a NEW one of suitable type:
  new_type variable2;
  variable2 = (new_type) variable1; This is valid, as we are not re-defining the 'type' of the object
*/
#include <avr/wdt.h> //WatchDog Timer
#include<Wire.h> //Required for I2C communication with RTC
#include <DS3231.h>

DS3231 rtc(SDA, SCL);

int segA = 9; int segB = 3; int segC = 4; int segD = 5; int segE = 6; int segF = 7; int segG = 8;
#define decimal A2
int digit1 = 13; int digit2 = 12; int digit3 = 11; int digit4 = 10;
//Ordered form 0-9
int num_array[][7] = {{1, 1, 1, 1, 1, 1, 0}, {0, 1, 1, 0, 0, 0, 0}, {1, 1, 0, 1, 1, 0, 1}, {1, 1, 1, 1, 0, 0, 1}, {0, 1, 1, 0, 0, 1, 1}, {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1}, {1, 1, 1, 0, 0, 0, 0}, {1, 1, 1, 1, 1, 1, 1}, {1, 1, 1, 1, 0, 1, 1}
};

int pin_array[] = {9, 3, 4, 5, 6, 7, 8}; //Pins to which segments are assigned
int digit_array[] = {13, 12, 11, 10}; //Pins to which each digit is assigned

//For Blink-without delay system: to blink the decimal points:
long current_millis = millis(); long previous_millis = 0;
#define interrupt_pin 2//////////////////////////////////////////////////////////////////
volatile int tog1 = 0; volatile int tog2 = 0;
volatile String toggle_state = "C";


//Switch-press creates a signal 'rise' - lifting causes a 'fall'. So pressing the switch calls interrupts twice, resetting the state
//This was pointless: so we used attachInterrupt(pin, ISR, RISING); so only called ISR once on the button push-down
void mode_switch() {
  if (tog1 == 0 && tog2 == 0) {
    tog2 = 1; toggle_state = "S";
  }
  else if (tog1 == 0 && tog2 == 1) {
    tog1 = 1; toggle_state = "A";
  }
  else if (tog1 == 1 && tog2 == 1) {
    tog1 = 0; tog2 = 0; toggle_state = "C";
  }
}
/*
   Cycling through the different modes:
  tog1, tog2 = (0,0) displays current time only (this mode deactivates alarm)
  tog1, tog2 = (0,1) displays countdown timer
  tog1, tog2  = (1,1) displays current time - can long press button to adjust Alarm time
*/


//Variables required for the main clock loop
int next_sec; int prev_sec;
int prev_state = 0;
int current_state = 1;


void setup() {
  Serial.begin(9600);
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);

  pinMode(decimal, OUTPUT); //Will blink every 1 second period

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  pinMode(digit4, OUTPUT);

  rtc.begin();
  wdt_disable();
  //Note: those should only be set-up once, or when the CR2032 battery is being replaced -
  //should not be run everytime at start-up as it will store the Time data
  //  rtc.setDOW(WEDNESDAY);     // Set Day-of-Week
  // rtc.setTime(16,12,07);     // Set the time to 12:00:00
  //rtc.setDate(9, 8, 2021);   // Set the date MM/DD/YY

  pinMode(A0, INPUT); //Knob for Adjusting minutes
  pinMode(A1, INPUT); //Pause/Play button for Timer countdown
  pinMode(interrupt_pin, INPUT); 
  attachInterrupt(digitalPinToInterrupt(interrupt_pin), mode_switch , RISING);
  pinMode(A3, OUTPUT); pinMode(A4, OUTPUT); //Pins for alarm LED indicator and alarm buzzer (respectively)
}


//Non-pin variables created:
int num_value; int seg; int digit;
int num_1; int num_2; int num_3; int num_4;

//Button press controls:
int pauseplay; int control = A0;
int run_time; //The total runtime of the program in milli seconds


void decimal_blink() { //Regardless of running mode: will blink decimal every Second period
  if (current_state == 1 && prev_state == 0) {
    current_state = prev_state;
    prev_state = 1;
    digitalWrite(decimal, current_state);
  }

  else if (current_state == 0 && prev_state == 1) {
    current_state = prev_state;
    prev_state = 0;
    digitalWrite(decimal, current_state);
  }
}

//Variables for alarm time
int hour_ = 0; int minute_ = 0;
      

////////////////  Function to  display Real-Time data  ///////////////////////////////////
void clock_function(bool alarm_state) {
  int digit_values[] = {num_1, num_2, num_3, num_4}; //List to be updated each Minute of RTC reading


  //Multiplexing with a delay of 1ms between each 7 segment array (net refresh rate ~250Hz)

  for (digit = 0; digit <= 3; digit += 1) { //'digit' is index

    //Number to be shown on the (digit+1)th digit - this list is updated by RTC information
    num_value = digit_values[digit];

    digitalWrite(digit_array[digit], 0); //Grounding to allow LEDs to light up

    //num_n is the number to be written on the nth digit on the LED array
    for (seg = 0; seg <= 6; seg += 1) {
      digitalWrite(pin_array[seg], num_array[num_value][seg]);
    }

    delay(1); //>5ms is too obvious (stuttering)
    digitalWrite(digit_array[digit], 1); //Switching OFF digit segment
  }

  current_millis = micros();

  if (current_millis - previous_millis >= (1000000 - run_time)) {
    previous_millis = current_millis; //1 Second delay to alternately blink the decimal points
    decimal_blink();
  }

  if (current_millis > 2000000000) { //Watch will likely be unplugged before the millis() overflows it's 32 bit buffer, however as a contingency:
    //will force reset the arduino after 2b secs to avoid exceeding 2.15b sec 'long' number limit
    wdt_enable(WDTO_250MS);
    while (1) {};
  }
  //Serial.println(rtc.getDOWStr()); Serial.print(rtc.getDateStr()); Serial.print(rtc.getTimeStr());



if (alarm_state == false){}

else if (alarm_state == true) {
  //Write an alarm loop that will iterate for a whole minute before stopping - which will comtinue to run for the whole minute duration
  


  }






  
}

/////////////////////////////////////////////////////////////////////////////////
int minutes = 0; int seconds = 60; unsigned long currenttime1; long previoustime1; //Global - so actively updated in all time-keeping loops
unsigned long currenttime2; long previoustime2; //Time keeping for stopwatch


//////////////////////// Functions to operate the Stopwatch ////////////////////////////////////
int ppswitch_new = 0; int ppswitch_old = 1; bool count_switch = false; //'false' state prevents timer from auto-starting


void countdown(int num1, int num2, int num3, int num4, bool switch_state) {
  currenttime2 = micros();
  if (switch_state == false) {} //Do nothing
  else if (switch_state == true  && ((currenttime2 - previoustime2) >= (1000000 - run_time))) { //Using Microseconds more accurate that millis - lose a lower fraction of a second per hour
    decimal_blink(); //Blinks decimal in sync with seconds count

    String sec_str = (String) seconds; String min_str = (String) minutes; Serial.println(min_str + ":" + sec_str); //Debugging - to read values
    previoustime2 = currenttime2;
    seconds -= 1; //Decrements seconds value each cycle of the loop
    if (seconds == -1) {
      seconds = 59;
    }
    if (seconds == 59) {
      minutes -= 1;  //MUST GO AFTER seconds -= 1 otherwise minutes becomes -1: invalid, if minutes starts at 0
    }

    // Formatting digits 3 and 4 correctly to display the seconds value
    if (seconds < 10) {
      num_3 = 0;
      num_4 = seconds;
    }
    else if (seconds >= 10) {
      sec_str = (String) seconds;
      num_3 = (sec_str.substring(0, 1)).toInt();
      num_4 = (sec_str.substring(1)).toInt();
    }

    if (minutes == 0 && seconds == 0) {
      count_switch = false; //Halts the counting sequence when timer runs to 0
    }

    //CONTINGENCY: If programs runs for a 'very' long time - in case of buffer overflow
    if (currenttime2 > 2100000000) {
      wdt_enable(WDTO_250MS);
      while (1);
    }
  }
}


//Increments or Decrements minute value when rotating Potentiometer knob
void min_increment(int c_delay) { //Within parenthesis of declaring a 'void' function, can't use (variab;e) - must use (type variable) and declare it's type
  if (minutes > 95) {
    minutes = 95;
  }
  else if (minutes < 0) {
    minutes = 0;
  }
  else {
    minutes = abs(minutes + c_delay);
  }
}


//Main Stopwatch loop
void stopwatch_function() {

  int digit_values[] = {num_1, num_2, num_3, num_4}; //List to be updated each cycle (per second) of Stopwatch loop

  //Multiplexing with a delay of 1ms between each 7 segment array (net refresh rate ~250Hz)

  for (digit = 0; digit <= 3; digit += 1) { //'digit' is index

    //Number to be shown on the (digit+1)th digit - this list is updated by RTC information
    num_value = digit_values[digit];

    digitalWrite(digit_array[digit], 0); //Grounding to allow LEDs to light up

    //num_n is the number to be written on the nth digit on the LED array
    for (seg = 0; seg <= 6; seg += 1) {
      digitalWrite(pin_array[seg], num_array[num_value][seg]);
    }

    delay(1); //>5ms is too obvious (stuttering)
    digitalWrite(digit_array[digit], 1); //Switching OFF digit segment
  }


  //Serial.println(analogRead(control));
  control = analogRead(A0);
  int count_state; float count_delay;

  if (control >= 996 && control <= 1011) {
    count_state = 0;
    count_delay = 0;
  }
  else if (control > 1011) {
    count_state = 1;
    if (control >= 1022) {
      count_delay = 12.0; //Fast count up or down
    }
    else if (control < 1022 && control > 1011) {
      count_delay = round((int)(400.0 - (( float(control) - 1011.0) / 12.0) * 200.0));
    }

  }
  else if (control < 996 ) {
    count_state = -1;
    if (control <= 657) {
      count_delay = 12.0;
    }
    else if (control < 996 && control > 657) {
      count_delay = round((int)(400.0 - ((996.0 - float(control)) / 336.0) * 200.0));
    }
  }
  int countdelay = round(count_delay);

  if (count_state == 0) {
    minutes = minutes;
  }
  else if (count_state != 0) {
    if ((currenttime1 - previoustime1) >= countdelay) {
      previoustime1 = currenttime1; min_increment(count_state);
    }  //If count_state is +1 then it will increment the minute values
  } //If count_state is -1 then it will decrement the minute values


  //num_1 and num_2 used to display 'minutes' digits: these can be adjusted using the knob
  if (minutes < 10) {
    num_1 = 0;
    num_2 = minutes;
  }
  else if (minutes >= 10) {
    String min_str = String(minutes);
    num_1 = (min_str.substring(0, 1)).toInt(); num_2 = (min_str.substring(1)).toInt();
  }


  //NOTE! pauseplay analog value hits 1023 when button is pressed: otherwise normally idles between 100-300 range (but is a decisive digital jump)
  ppswitch_new = analogRead(A1);

  if (ppswitch_new > 1020 && ppswitch_old < 1000) { //Button push down
    ppswitch_old = ppswitch_new;

    if (count_switch == true) {
      count_switch = false;
    }
    else if (count_switch == false) {
      count_switch = true;  //Toggles state: play is true, false is pause. Resets seconds value to initial.
      seconds = 60;
    }
  }
  if (ppswitch_new < 1000 && ppswitch_old > 1020) { //Button lift up
    ppswitch_old = ppswitch_new; //Resets the 'old' switch state upon lift-up to detect a push down
  }

  countdown(num_1, num_2, num_3, num_4, count_switch);
}

//////////////// Functions for Alarm activation loop //////////////////

int A_switchold = 0; int A_switchnew = 0; unsigned long time_down; 
String edit_term = "HOUR"; String edit_value; int display_state = 1;

//This is multiplex to actually display the values onto the 7-SEGMENT display



void alarm_edit(int countstate, String editterm){
 if (editterm == "MINUTE"){minute_ += countstate;}
 else if (editterm == "HOUR"){hour_ += countstate;}
}  //Now must implement modulus into this so it does not go into negatives and does not exceed the set time limits




//Main tasks!
//Implement modulus into minute and hour adjustments
//MUST recheck if the RTC can store 24 hour format or not - RTC does in fact support it
//Write a blink without delay style tune for the alarm that will run continuously for a whole minute for the alarm
//Make LED blink when adjusting alarm
//Solder all components onto the PCB

























//Alternates between current-time and alarm-time, allows setting of time for alarm
void alarm_activation() {

  A_switchnew = analogRead(A1);
  //Button push-down
  if (A_switchnew > 1000 && A_switchold < 1000){time_down = millis(); A_switchold = A_switchnew;
  
  }
  //Button pull-up
  else if (A_switchnew < 1000 && A_switchold > 1000){int period = (millis() - time_down); A_switchold = A_switchnew;
   
    //Nested loops - different conditions depending on how long switch was pressed down for
    
    //Long press: alternates between showing the alarm time and the current time
    if (period >400){ 
      if  (display_state == 0){display_state = 1;} //Displays alarm time
      else if (display_state == 1){ display_state = 0;}} //Displays the current time - alarm activated in BG
      
    //Short press: to alternate whether to change hour or minute
    else if (period <= 250){
   if (edit_term == "HOUR"){edit_term = "MINUTE";}
   else if (edit_term == "MINUTE"){ edit_term = "HOUR";}
     }
   }


//Long-press can toggle between showing the alarm time and the current time:

if (display_state == 1){String time_data = rtc.getTimeStr();  //Extracts time digits from the rtc 'Time' data
        num_1 = time_data.substring(0, 1).toInt(); num_2 = time_data.substring(1, 2).toInt();
        num_3 = time_data.substring(3, 4).toInt(); num_4 = time_data.substring(4, 5).toInt();
        clock_function(true);}  
          
else if(display_state == 0){
  
  char hourbuffer[3]; char minutebuffer[3];
  sprintf(hourbuffer, "%02d", hour_); sprintf(minutebuffer, "%02d", minute_); 
//^This reduces need for 'if' loop to check if value is 1 or 2 characters long: FORMATS to have preceding 0 when only 1 digit long
//Necessary as any number on display must be 2 characters in length.
  
  String _hour = (String) hourbuffer; String _minute = (String) minutebuffer; 
  num_1 = _hour.substring(0,1).toInt(); num_2 = _hour.substring(1).toInt(); 
  num_3 = _minute.substring(0,1).toInt(); num_4 = _minute.substring(1).toInt();
  clock_function(true);}
              

//Potentiometer knob to adjust hours/ minutes for Alarm 
control = analogRead(A0); int count_state; float count_delay;
  
  if (control >= 996 && control <= 1011) {
    count_state = 0;
    count_delay = 0;}
  else if (control > 1011) {
    count_state = 1;
    if (control >= 1022) {
      count_delay = 12.0;} //Fast count up or down}
    else if (control < 1022 && control > 1011) {
      count_delay = round((int)(400.0 - (( float(control) - 1011.0) / 12.0) * 200.0));}
      }
 
  else if (control < 996 ) {
    count_state = -1;
    if (control <= 657) {
      count_delay = 12.0;}
    else if (control < 996 && control > 657) {
      count_delay = round((int)(400.0 - ((996.0 - float(control)) / 336.0) * 200.0));}
  }
int countdelay = round(count_delay);


if ((currenttime1 - previoustime1) >= countdelay) {
      previoustime1 = currenttime1; alarm_edit(count_state, edit_term); //edit_term toggles which value is adjusted
    }  //If count_state is +1 then it will increment edit_value, otherwise decrement if count_state = -1
  } 






//Main loop repeated executed to check the mode of the clock:
void loop() {
  int start_ = micros(); currenttime1 = millis();
  if (toggle_state == "C") {    
    String time_data = rtc.getTimeStr();  //Extracts time digits from the rtc 'Time' data
    num_1 = time_data.substring(0, 1).toInt(); num_2 = time_data.substring(1, 2).toInt();
    num_3 = time_data.substring(3, 4).toInt(); num_4 = time_data.substring(4, 5).toInt();
    clock_function(false); //False - so alarm will be Deactivated

    digitalWrite(A3, LOW);
  }

  else if (toggle_state == "S") {
    stopwatch_function(); digitalWrite(A3, LOW);
  }
  else if (toggle_state == "A") {
    alarm_activation(); digitalWrite(A3, HIGH);
  }

  int end_ = micros(); run_time = end_ - start_; //Measures the program runtime for accurate timekeeping

  //Serial.println(run_time); //Debugging - if unusually high program processing time
}

/*Key Notes: _______________1)Char arrays into Strings  2)Formatting using sprintf______________________
 * 
 * char buffer_name[size] defines the 'char' array - this can be an integer or string
 * But each sequential character is stores within a list
 * Note!: 'buffer' is an in-built variable that can be used in the place of an actual buffer name - this is a temporarily
 * stores character array usually used for Serial prints.
 * 
 * 1) Conversion to string: String variable = (String) buffer_name 
 * HOWEVER! We must add a size+1 into the buffer size to accomodate a '\0' at the end.
 * This is the 'null terminator' that differentiates any normal sequence of characters from a String - which must have 
 * a numm terminator.
 * 
 * 2)sprintf(buffer_name, "String %placeholder rest of String", variables)  
 * ---> Similar to python formatting without the .format(list of vairables) at the end.
 * Placeholders can be:
 * %d for signed integer %s for string - for FLOATS MUST CONVERT INTO A STRING AS THEY CANNOT BE FORMATTED BY %d
 * 
 * Padding with 0s: %0nd as the formatting placeholder, where n is the number allocated length of the number. It will 
 * pad the number with PRECEDING 0s if it is any shorter than the allocated length.
 */
