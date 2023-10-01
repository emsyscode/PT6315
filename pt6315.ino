/****************************************************/
/* This is only one example of code structure       */
/* of course this code can be optimized, but        */
/* the idea is let it so simple to be easy catch    */
/* where can do changes and look to the results     */
/****************************************************/

#define VFD_in 8// If 0 write LCD, if 1 read of LCD
#define VFD_clk 9 // if 0 is a command, if 1 is a data0
#define VFD_stb 10 // Must be pulsed to LCD fetch data of bus

unsigned int segments[] ={
  // Here I'm forced to use the "0" as 10, because the 7 segments start in "1"
  // This table is inverted
  // The PT6315 use 3 bytes to each grid, 23HU is maximum address of block RAM to 12 digits
  //    This not respect the table for 7 or 14 segments like "abcdefgh"  //
 //     11122222   22222333
 //     78901234   56789012 
      0b11000100,0b10001100,0b00000000,//0
      0b01000000,0b00000100,0b00000000,//1 
      0b01000100,0b10001011,0b00000000,//2 
      0b01000100,0b10000111,0b00000000,//3 
      0b11000000,0b00000111,0b00000000,//4
      0b10000100,0b10000111,0b00000000,//5 
      0b10000000,0b10001111,0b00000000,//6
      0b01000100,0b00000100,0b00000000,//7 
      0b11000100,0b10001111,0b00000000,//8
      0b11000100,0b00000111,0b00000000,//9 
      0b00000000,0b00000000,0b00000000//10 // empty display
 };
void pt6315_init(void){
  //Note: The number of GRIDs on VFD on this example is 6, because
  //the left and rhigth grid's are the same... have less of 8 segments each, they
  //use the same grid!
  delayMicroseconds(200); //power_up delay
  // Note: Allways the first byte in the input data after the STB go to LOW is interpret as command!!!
  // Configure VFD display (grids)
  cmd_with_stb(0b00001111);//  (0b01000000)    cmd1 12 grids 16 segm
  delayMicroseconds(10);
  // Write to memory display, increment address, normal operation
  cmd_with_stb(0b01000000);//(BIN(01000000));
  delayMicroseconds(10);
  // Address 00H - 23H 
  cmd_with_stb(0b11000000);//(BIN(01100110)); 
  delayMicroseconds(10);
  // set DIMM/PWM to value
  cmd_with_stb((0b10001000) | 7);//0 min - 7 max  )(0b01010000)
  delayMicroseconds(10);
}
void cmd_without_stb(unsigned char a){
  // send without stb
  unsigned char transmit = 7; //define our transmit pin
  unsigned char data = 170; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  //This don't send the strobe signal, to be used in burst data send
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_in, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_in, LOW);
     }
    delayMicroseconds(5);
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(5);
   }
   //digitalWrite(VFD_clk, LOW);
}
void cmd_with_stb(unsigned char a){
  // send with stb
  unsigned char data = 0x00; //value to transmit, binary 10101010
  unsigned char mask = 1; //our bitmask
  
  data=a;
  
  //This send the strobe signal
  //Note: The first byte input at in after the STB go LOW is interpreted as a command!!!
  digitalWrite(VFD_stb, LOW);
  delayMicroseconds(1);
   for (mask = 00000001; mask>0; mask <<= 1) { //iterate through bit mask
     digitalWrite(VFD_clk, LOW);
     delayMicroseconds(1);
     if (data & mask){ // if bitwise AND resolves to true
        digitalWrite(VFD_in, HIGH);
     }
     else{ //if bitwise and resolves to false
       digitalWrite(VFD_in, LOW);
     }
    digitalWrite(VFD_clk, HIGH);
    delayMicroseconds(1);
   }
   digitalWrite(VFD_stb, HIGH);
   delayMicroseconds(1);
}
void test_VFD(void){
  /* 
  Here do a test for all segments of 12 grids
  each grid is controlled by a group of 3 bytes
  by these reason I'm send a burst of 3 bytes of
  data. The cycle for do a increment of 3 bytes on 
  the variable "i" on each test cycle of FOR.
  */
  // to test 12 grids is 12*3=36, for example, the 6 grids will result in 6*3=18.
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 23H)
        
         for (int i = 0; i < 12 ; i++){ // test base to 16 segm and 12 grids
         cmd_without_stb(0b11111111); // Data to fill table 
         cmd_without_stb(0b11111111); // Data to fill table 
         cmd_without_stb(0b11111111); // Data to fill table 
         }
    
      cmd_with_stb((0b10001000) | 7); //cmd 4
      digitalWrite(VFD_stb, HIGH);
      delay(1);
}
void clear_VFD(void){
  /*
  Here I clean all registers 
  Could be done only on the number of grid
  to be more fast. The 12 * 3 bytes = 36 registers
  */
      for (uint8_t n=0x00; n < 0x24; n=n+3){  // 
        //cmd_with_stb(0b00001000); //       cmd 1 // 12 Grids & 16 Segments
        cmd_with_stb(0b01000000); //       cmd 2 //Normal operation; Set pulse as 1/16
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
            cmd_without_stb((0b11000000) | n); // cmd 3 //wich define the start address (00H to 23H)
            cmd_without_stb(0b00000000);  //8-1 Data to fill table RAM of 12 grids * 16 segm 
            cmd_without_stb(0b00000000);  //16-9 Data to fill table RAM of 12 grids * 16 segm
            cmd_without_stb(0b00000000);  //24-17 Data to fill table RAM of 12 grids * 16 segm
            //
            digitalWrite(VFD_stb, HIGH);
            delay(1);  //This timer here is only to debug, can use 1uSec
           //cmd_with_stb(0b00001000); //       cmd 1 // 12 Grids & 16 Segments
            cmd_with_stb((0b10001000) | 7); //cmd 4
            delay(1);
     }
}

void readButtons(){
//Take special attention to the initialize digital pin LED_BUILTIN as an output.
//
int ledPin = 13;   // LED connected to digital pin 13
int inPin = 8;     
int val = 0;       // variable to store the read value
int dataIn=0;
byte array[8] = {0,0,0,0,0,0,0,0};
byte together = 0;
unsigned char receive = 8; //define our transmit pin
unsigned char data = 0; //value to transmit, binary 10101010
unsigned char mask = 1; //our bitmask
array[0] = 1;
unsigned char btn1 = 0x41;
      digitalWrite(VFD_stb, LOW);
        delayMicroseconds(2);
      cmd_without_stb(0b01000010); // cmd 2 //Read Keys;Normal operation; Set pulse as 1/16
       // cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 23H)
     // send without stb
  
  pinMode(inPin, INPUT);  // Important this point! Here I'm changing the direction of the pin to INPUT data.
  delayMicroseconds(2);
  //PORTD != B01010100; // this will set only the pins you want and leave the rest alone at
  //their current value (0 or 1), be careful setting an input pin though as you may turn 
  //on or off the pull up resistor  
  //This don't send the strobe signal, to be used in burst data send
         for (int z = 0; z < 3; z++){
             //for (mask=00000001; mask > 0; mask <<= 1) { //iterate through bit mask
                   for (int h =8; h > 0; h--) {
                      digitalWrite(VFD_clk, HIGH);  // Remember wich the read data happen when the clk go from LOW to HIGH! Reverse from write data to out.
                      delayMicroseconds(2);
                     val = digitalRead(inPin);
                      //digitalWrite(ledPin, val);    // sets the LED to the button's value
                           if (val & mask){ // if bitwise AND resolves to true
                             //Serial.print(val);
                            //data =data | (1 << mask);
                            array[h] = 1;
                           }
                           else{ //if bitwise and resolves to false
                            //Serial.print(val);
                           // data = data | (1 << mask);
                           array[h] = 0;
                           }
                    digitalWrite(VFD_clk, LOW);
                    delayMicroseconds(2);
                    
                   } 
             
              Serial.print(z);  // All the lines of print is only used to debug, comment it, please!
              Serial.print(" - " );
                        
                                  for (int bits = 7 ; bits > -1; bits--) {
                                      Serial.print(array[bits]);
                                   }
                        
                        if (z==1){
                          if(array[6] == 1){
                           
                          }
                        }
                          if (z==0){
                          if(array[2] == 1){
                           
                          }
                          }
                          if (z==0){
                          if(array[6] == 1){
                           
                          }
                        }
                        if (z==0){
                          if(array[7] == 1){
                           
                          }
                        }
                        if (z==0){
                          if(array[3] == 1){
                           
                          }
                        }
                          if (z==1){
                            if(array[7] == 1){
                             
                            }
                          }
                         
                  Serial.println();
          }  // End of "for" of "z"
      Serial.println();  // This line is only used to debug, please comment it!
 digitalWrite(VFD_stb, HIGH);
 delayMicroseconds(2);
 cmd_with_stb((0b10001000) | 7); //cmd 4
 delayMicroseconds(2);
 pinMode(inPin, OUTPUT);  // Important this point!  // Important this point! Here I'm changing the direction of the pin to OUTPUT data.
 delay(1); 
}
void msgFolks(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11000000)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b10000100);   cmd_without_stb(0b10000111);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          cmd_without_stb(0b10100000);   cmd_without_stb(0b01001010);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (K)
          cmd_without_stb(0b10000000);   cmd_without_stb(0b10001000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (L)
          cmd_without_stb(0b11000100);   cmd_without_stb(0b10001100);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (O)
          cmd_without_stb(0b10000100);   cmd_without_stb(0b00001011);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (F)
          cmd_without_stb(0b00000000);   cmd_without_stb(0b00000000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (space)
          cmd_without_stb(0b00010000);   cmd_without_stb(0b00100000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (I)
          cmd_without_stb(0b11000000);   cmd_without_stb(0b00001111);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (H)
          cmd_without_stb(0b00000000);   cmd_without_stb(0b00000000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (space)
          cmd_without_stb(0b00000000);   cmd_without_stb(0b00000000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (space)
          cmd_without_stb(0b00000000);   cmd_without_stb(0b00000000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (space)
          cmd_without_stb(0b00000000);   cmd_without_stb(0b00000000);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (space)

         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void msgWheelsA(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11100001)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b00001000);   cmd_without_stb(0b00000010);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          
         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void msgWheelsB(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11100001)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b00010000);   cmd_without_stb(0b00000110);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          
         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void msgWheelsC(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11100001)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b00100000);   cmd_without_stb(0b00000010);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          
         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void msgWheelsD(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11100001)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b01000000);   cmd_without_stb(0b00000110);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          
         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void msgWheelsE(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11100001)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b10000000);   cmd_without_stb(0b00000010);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          
         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void msgWheelsF(void){
      digitalWrite(VFD_stb, LOW);
      delayMicroseconds(1);
      cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Set pulse as 1/16
      
        digitalWrite(VFD_stb, LOW);
        delayMicroseconds(1);
        cmd_without_stb((0b11100001)); //cmd 3 wich define the start address (00H to 24H)
        
          cmd_without_stb(0b00000000);   cmd_without_stb(0b00000111);  cmd_without_stb(0b00000000);// 8-1    16-9  24-17 (S)
          
         digitalWrite(VFD_stb, HIGH);
        delay(3);
        cmd_with_stb((0b10001000) | 7); //cmd 4
        delay(3);
}
void numbers(){
  clear_VFD();
     for(uint8_t n=0x00; n<0x24; n=n+3){   //Here is a loop to go through the 12 grid's (each group have 3 bytes) 3*12 = 36, start at 0, means 36-1;
          for(byte s=0; s<31; s=s+3){ //Here you go through the number of digits from "0" to "9" and also the "blank" digit
            digitalWrite(VFD_stb, LOW);
              delayMicroseconds(1);
              cmd_with_stb(0b01000000); // cmd 2 //Normal operation; Write to display, write to LED's, read keys.
              //
                digitalWrite(VFD_stb, LOW);
                delayMicroseconds(1);
                cmd_without_stb((0b11000000 | n)); //cmd 3 wich define the start address (00H to 15H)
                Serial.print("Grid: ");Serial.print((0b11000000 | n), HEX);Serial.print("; Digit: ");Serial.println(s, HEX);  //uncomment to debug!
                cmd_without_stb(segments[s]); 
                cmd_without_stb(segments[s+1]); 
                cmd_without_stb(segments[s+2]);
              //
            digitalWrite(VFD_stb, HIGH);
            //cmd_with_stb((0b10001000) | 7); //cmd 4
            delay(300);
           }
     }
/*
* 00HL 00HU 01HL 01HU 02HL 02HU DIG1 
* 03HL 03HU 04HL 04HU 05HL 05HU DIG2 
* 06HL 06HU 07HL 07HU 08HL 08HU DIG3 
* 09HL 09HU 0AHL 0AHU 0BHL 0BHU DIG4 
* 0CHL 0CHU 0DHL 0DHU 0EHL 0EHU DIG5 
* 0FHL 0FHU 10HL 10HU 11HL 11HU DIG6 
* 12HL 12HU 13HL 13HU 14HL 14HU DIG7 
* 15HL 15HU 16HL 16HU 17HL 17HU DIG8 
* 18HL 18HU 19HL 19HU 1AHL 1AHU DIG9 
* 1BHL 1BHU 1CHL 1CHU 1DHL 1DHU DIG10
* 1EHL 1EHU 1FHL 1FHU 20HL 20HU DIG11
* 21HL 21HU 22HL 22HU 23HL 23HU DIG12
*/
}
void writeLED(){
  for(int led=0; led<4; led++){
              digitalWrite(VFD_stb, LOW);
              delayMicroseconds(1);
              //cmd_with_stb(0b00000010); // cmd 1 // 6 Grids & 16 Segments
              //
                digitalWrite(VFD_stb, LOW);
                delayMicroseconds(1);
                cmd_without_stb(0b01000001);
                switch (led){
                  case '0': cmd_without_stb(0b00001110); break; //LED's 0,1,2,3 is reverse mode 0=On 1=Off
                  case '1': cmd_without_stb(0b00001101); break;
                  case '2': cmd_without_stb(0b00001011); break;
                  case '3': cmd_without_stb(0b00000111); break;
                }
              //
              digitalWrite(VFD_stb, HIGH);
              delay(500);
  }
}
void setup() {
// put your setup code here, to run once:
// initialize digital pin LED_BUILTIN as an output.
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

pinMode(VFD_in, OUTPUT);
pinMode(VFD_clk, OUTPUT);
pinMode(VFD_stb, OUTPUT);

pt6315_init();
}
void loop() {
  // You can comment the function to avoid it running and observe only that you want run
  pt6315_init();
  clear_VFD();
     while(1){
        clear_VFD();
        numbers();
    //  writeLED();
    //  readButtons();  //Maybe implement a "interrupt" to call the read buttons function!
    //
           for (int i=0; i<3; i++){
            //test_VFD();      
            msgFolks();
           //
           msgWheelsA();delay(200);
           msgWheelsB();delay(200);
           msgWheelsC();delay(200);
           msgWheelsD();delay(200);
           msgWheelsE();delay(200);
           msgWheelsF();delay(200);
           }
      }
}
