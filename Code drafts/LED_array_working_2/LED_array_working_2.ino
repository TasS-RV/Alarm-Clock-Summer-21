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

int segA = 2; int segB = 3; int segC = 4; int segD = 5; int segE = 6; int segF = 7; int segG = 8;
int decimal = 9;
int digit1 = 13; int digit2 = 12; int digit3 = 11; int digit4 = 10;
//Ordered form 0-9
int num_array[][7] = {{1,1,1,1,1,1,0},{0,1,1,0,0,0,0}, {1,1,0,1,1,0,1}, {1,1,1,1,0,0,1},{0,1,1,0,0,1,1},{1,0,1,1,0,1,1},
{1,0,1,1,1,1,1},{1,1,1,0,0,0,0},{1,1,1,1,1,1,1},{1,1,1,1,0,1,1}};

int pin_array[] = {2,3,4,5,6,7,8}; //Pins to which segments are assigned
int digit_array[] = {13,12,11,10}; //Pins to which each digit is assigned


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
}

int num_value; int seg; int digit;
int num_1; int num_2; int num_3; int num_4;

void loop() {

num_1 = 6; num_2 = 2; num_3 = 5;num_4 = 7;
int digit_values[] = {num_1, num_2, num_3, num_4}; //List to be updated each Minute of RTC reading
 
//Note: can use a blink_without_delay() style structure to blink the Decimal points once/sec
  digitalWrite(decimal, 1);



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
}
