/* !This is a COMMON Cathode 7-segment array: so Digit Pins are 'ground': must write 0 to activate LED
   !The digit pins are also in reverse order (to RoboJax video diagram)
   Watch: https://www.youtube.com/watch?v=f3GqRMQ8jYs&t=908s at 2:26

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

void setup() {
  Serial.begin(9600);
  pinMode(segA, OUTPUT);
  pinMode(segB, OUTPUT);
  pinMode(segC, OUTPUT);
  pinMode(segD, OUTPUT);
  pinMode(segE, OUTPUT);
  pinMode(segF, OUTPUT);
  pinMode(segG, OUTPUT);

  pinMode(decimal, OUTPUT);

  pinMode(digit1, OUTPUT);
  pinMode(digit2, OUTPUT);
  pinMode(digit3, OUTPUT);
  pinMode(digit4, OUTPUT);
}

int num1;

void loop() {
int seg; num1 = 9;
  digitalWrite(digit1, 0);
  digitalWrite(digit2, 0);
  digitalWrite(digit3, 1);
  digitalWrite(digit4, 1);

for (seg = 0; seg<= 6; seg += 1){
 //num_n is the number to be written on the nth digit on the LED array
  digitalWrite(pin_array[seg], num_array[num1][seg]); 
}

//Note: can use a blink_without_delay() style structure to blink the Decimal points once/sec
  digitalWrite(decimal, 1);





}

/*For loop: must declare the variable initially: can 'reset' the value in the for loop

  Let variable = n (must be an integer)
  int n;
  for(reset n-value; limit of n-value; update of n-value){
  Some operation;
  }

  Typically: n-value is incremented until it reaches the limit: when the for loop resets
  its value.

  What if we want to iterate over the items of a list?
  Unlike python, we cannot directly do this: for loops can only be counters, so the updated
  n-value can be used as an Index of the list.

  Each iteration, n-value is incremented, to move onto the next value.

  int arr[] = {value1, value2...}; //list must store the same object 'type'

  sizeof arr = //The number of elements in the array. Therefore, limit of n-value can be
  set to the length of the list (to iterate and read each element incrementally, by index).

*/
