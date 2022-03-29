
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
*/
#include <avr/wdt.h> //WatchDog Timer
#include<Wire.h> //Required for I2C communication with RTC
#include <DS3231.h>

DS3231 rtc(SDA,SCL);

int segA = 9; int segB = 3; int segC = 4; int segD = 5; int segE = 6; int segF = 7; int segG = 8;
#define decimal A2
int digit1 = 13; int digit2 = 12; int digit3 = 11; int digit4 = 10;
//Ordered form 0-9
int num_array[][7] = {{1,1,1,1,1,1,0},{0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},
{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};

int pin_array[] = {9,3,4,5,6,7,8}; //Pins to which segments are assigned
int digit_array[] = {13,12,11,10}; //Pins to which each digit is assigned

//For Blink-without delay system: to blink the decimal points:
long current_millis = millis(); long previous_millis = 0;
#define interrupt_pin 2//////////////////////////////////////////////////////////////////
volatile String toggle_state = "C";


//Switch-press creates a signal 'rise' - lifting causes a 'fall'. So pressing the switch calls interrupts twice, resetting the state
//This was pointless: so we used attachInterrupt(pin, ISR, RISING); so only called ISR once on the button push-down
void mode_switch(){   
  if (toggle_state == "C"){
    toggle_state = "S";
  }
  else if (toggle_state == "S"){
    toggle_state = "C";}
  }


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
attachInterrupt(digitalPinToInterrupt(interrupt_pin), mode_switch ,RISING); 
}


//Non-pin variables created:
int num_value; int seg; int digit;
int num_1; int num_2; int num_3; int num_4;

int next_sec; int prev_sec;
int prev_state = 0; 
int current_state = 1;

//Button press controls:
int pauseplay; int control = A0;
int run_time; //The total runtime of the program in milli seconds

 
////////////////  Function to  display Real-Time data  ///////////////////////////////////    
void clock_function(){                          
  String time_data = rtc.getTimeStr();  //Extracts time digits from the rtc 'Time' data 

num_1 = time_data.substring(0,1).toInt(); num_2 = time_data.substring(1,2).toInt();
num_3 = time_data.substring(3,4).toInt(); num_4 = time_data.substring(4,5).toInt();

int digit_values[] = {num_1, num_2, num_3, num_4}; //List to be updated each Minute of RTC reading
 

//Multiplexing with a delay of 1ms between each 7 segment array (net refresh rate ~250Hz)

for(digit = 0; digit<=3; digit+=1){ //'digit' is index
  
  //Number to be shown on the (digit+1)th digit - this list is updated by RTC information
  num_value = digit_values[digit];
  
  digitalWrite(digit_array[digit],0);  //Grounding to allow LEDs to light up
  
  //num_n is the number to be written on the nth digit on the LED array
  for (seg = 0; seg<= 6; seg += 1){
  digitalWrite(pin_array[seg], num_array[num_value][seg]);}
  
  delay(1); //>5ms is too obvious (stuttering)
  digitalWrite(digit_array[digit],1); //Switching OFF digit segment
}

current_millis = millis();

if (current_millis - previous_millis >= (1000-run_time)){
  previous_millis = current_millis; //1 Second delay to alternately blink the decimal points
  
  if (current_state == 1 && prev_state == 0){
    current_state = prev_state;
    prev_state = 1;
    digitalWrite(decimal, current_state);}
    
  else if (current_state == 0 && prev_state == 1){
    current_state = prev_state;
    prev_state = 0;
    digitalWrite(decimal, current_state);}
}

if (current_millis > 2000000000){ //Watch will likely be unplugged before the millis() overflows it's 32 bit buffer, however as a contingency:
  //will force reset the arduino after 2b secs to avoid exceeding 2.15b sec 'long' number limit
  wdt_enable(WDTO_250MS);
  while(1){};}
//Serial.println(rtc.getDOWStr()); Serial.print(rtc.getDateStr()); Serial.print(rtc.getTimeStr());
}
/////////////////////////////////////////////////////////////////////////////////
int minutes = 0; int seconds = 0; unsigned long currenttime1; long previoustime1;
unsigned long currenttime2; long previoustime2; //Time keeping for stopwatch


//////////////////////// Functions to operate the Stopwatch ////////////////////////////////////

void countdown(int num1, int num2, int num3, int num4, bool switch_state){
  pauseplay = analogRead(A1);
  Serial.println(pauseplay); 
  Serial.println(switch_state);
  
  }



//Increments or Decrements minute value when rotating Potentiometer knob
void min_increment(int c_delay){//Within parenthesis of declaring a 'void' function, can't use (variab;e) - must use (type variable) and declare it's type
  if (minutes > 95){minutes = 95;}
  else if (minutes < 0){minutes = 0;}
  else {minutes = abs(minutes + c_delay);}
  }


int ppswitch_new = 0; int ppswitch_old = 1; bool count_switch = true;


//Main Stopwatch loop
void stopwatch_function(){
  
num_3 = 0; num_4 = 0;

int digit_values[] = {num_1, num_2, num_3, num_4}; //List to be updated each Minute of RTC reading
 

//Multiplexing with a delay of 1ms between each 7 segment array (net refresh rate ~250Hz)

for(digit = 0; digit<=3; digit+=1){ //'digit' is index
  
  //Number to be shown on the (digit+1)th digit - this list is updated by RTC information
  num_value = digit_values[digit];
  
  digitalWrite(digit_array[digit],0);  //Grounding to allow LEDs to light up
  
  //num_n is the number to be written on the nth digit on the LED array
  for (seg = 0; seg<= 6; seg += 1){
  digitalWrite(pin_array[seg], num_array[num_value][seg]);}
  
  delay(1); //>5ms is too obvious (stuttering)
  digitalWrite(digit_array[digit],1); //Switching OFF digit segment
}
current_millis = millis(); //current_millis Must be used exclusively for the seconds timekeeping

if (current_millis - previous_millis >=(1000-run_time)){
  previous_millis = current_millis; //1 Second delay to alternately blink the decimal points
  
  if (current_state == 1 && prev_state == 0){
    current_state = prev_state;
    prev_state = 1;
    digitalWrite(decimal, current_state);}
    
  else if (current_state == 0 && prev_state == 1){
    current_state = prev_state;
    prev_state = 0;
    digitalWrite(decimal, current_state);}
}

//Serial.println(analogRead(control));
control = analogRead(A0);
int count_state; float count_delay;

if (control >= 996 && control <= 1011){
  count_state = 0;
  count_delay = 0;
}
else if (control >1011){
  count_state = 1;
  if (control >= 1022){count_delay = 12.0;} //Fast count up or down
  else if (control <1022 && control > 1011){count_delay = round((int)(400.0-(( float(control) -1011.0)/12.0)*200.0));}
 
}
else if (control < 996 ){
  count_state = -1;
  if (control <= 657){count_delay = 12.0;} 
  else if (control <996 && control > 657){count_delay = round((int)(400.0-((996.0 - float(control))/336.0)*200.0));}
}
int countdelay = round(count_delay); 

if (count_state == 0){minutes = minutes;}
else if (count_state != 0){
  if ((currenttime1 - previoustime1) >= countdelay){
    previoustime1 = currenttime1; min_increment(count_state);}  //If count_state is +1 then it will increment the minute values 
  } //If count_state is -1 then it will decrement the minute values


//num_1 and num_2 used to display 'minutes' digits: these can be adjusted using the knob
if (minutes < 10){num_1 = 0; num_2 = minutes;}
else if (minutes >= 10){String min_str = String(minutes);
num_1 = (min_str.substring(0,1)).toInt(); num_2 = (min_str.substring(1)).toInt();}





//NOTE! pauseplay value hits 1023 when button is pressed: otherwise normally idles between 100-300 range (but is a decisive digital jump)


ppswitch_new = analogRead(A1);

if (ppswitch_new > 1020 && ppswitch_old < 1000){ //Button push down
  ppswitch_old = ppswitch_new;

  if (count_switch == true){count_switch = false;}
  else if (count_switch == false){count_switch = true;} //Toggles state: play is true, false is pause
  }
if (ppswitch_new < 1000 && ppswitch_old > 1020){ //Button lift up
  ppswitch_old = ppswitch_new; //Resets the 'old' switch state upon lift-up to detect a push down
  }

countdown(num_1, num_2, num_3, num_4, count_switch);





//1TOGGLE IS WORKING CORRECTLY
//Tasks to complete: write a countdown function which iterates every 60 seconds: and each complete iteration will decrement the minutes value
//This will depend on the toggle state of count_switch









//Serial.println(minutes);
}




//Main loop repeated executed to check the mode of the clock: 
void loop() {  
int start_ = millis(); currenttime1 = millis();
if (toggle_state == "C"){
  clock_function();
  }
  
else if (toggle_state == "S"){
  
  stopwatch_function();
//int hello;
  }
int end_ = millis(); run_time = end_-start_; //Measures the program runtime for accurate timekeeping



//Serial.println(run_time);

}   

/*The pause_button function:
 * 
 *void{
 *previous_state = digital_read(A0);
 *}
 * 
 * 
 * 
 * 
 * Stopwatch function can check for button press - depending on analog signal (approximate range)
 * Button press must itself be a single function - which takes in the input of 'which' button (or range of signals) it
 * is detecting the 'press' for
 * 
 * if countdown_state == true then this function will continue onwards....
 * 
 * for (n = 59; n>=0, n-=1){1
 * if(st_current and st_prev: if st_current-st_previous > (1000-program runtime)){
 * st_prev = st_current;
 * String n_ = String(n); ####
 * if length(n_) == 0{
 * n_ = "0"+n_;}    
 * else string n_ = n;
 * 
 * digit 3 = n_.subString(0,1).toInt(); digit 4 = n_.subString(1).toInt() - the 3rd and 4th digits will update with the  number of seconds counting down
 * //Starts from 0 - 59: therefore still will keep 60 seconds per count down cycle
 * 
 * function_call(pause_button)
 * 
 * update bool = true; }1
 * 
 * //For loop ends, and the minute value updates after 60 second countdown to degrement minutes left by 1
 * 
 * if update bool == true{2
 * minute -= 1}
 * 
 * //Apply similar syntax to the minute writing, and update digits 1 and 2 to minute
 * //Syntax from #### for the buffer size to at least be 2
 * update_bool = false
 *}2
 *
 *If countdown state == false then this countdown process will cease to continue, and num1-4 MUST hold their value
 *this is if:
 *
 *if digitalread(button_state)<100{
 *and 450<prev_button state <550}
 *
 //Then toggle the countdown state: the same button is used to pause/ play the timer
 //This detects a 'lift' of the button - rather than it being pushed down (so user can keep it held down for as long
 as they want, and only upon release the timer will be stopped.
 *
 *
 *IN THE MAIN 'VOID LOOP' ANALOG READ THE SIGNALS FROM A POTENTIOMETER TO ADJUST THE MINUTE THRESHOLDS: if centred, iwll not change, if dialed
 *one way will increase, and decrease the time when dialed the other direciton.
 *
 *This way, the 'minute' value can technically be increased at any time: do this by using an 'if' loop:
 *'while' loop could be used but is too risky...alternatively could use the while loop for the 'countdown state' to play/pause the timer
 */
