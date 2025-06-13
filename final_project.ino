const String BCCode[] = {"*","??*","?*<","*?","?","?*?","?**","**","??","*??","*<","??<","?*","<*","<?","?<","**<","?<*","?<?","<","<<","***","**?","*<?","?<<","*?<","<??*","<??<","<?*?","<?**","<?*<","<?<?","<?<*","<?<<","<*??","<???","<??","<?*","<*?","<**","<*<","<<?","<<*","<<<","<?<","<**<"};
const char AsciiCharacters[] = {'A','B','C','D','E','F','G','H','I','J','K','L','M','N','O','P','Q','R','S','T','U','V','W','X','Y','Z','1','2','3','4','5','6','7','8','9','0','.',',','\'','?','!','+','-','*','/','='};
void setup() {
  Serial.begin(9600); // the code begins here
  readInput();
}

//Receive input from serial port and determines if it is BCCode or ASCII
void readInput(void){
  String inputString;
  Serial.println("Please enter the text you want to translate: ");
  while (!Serial.available()){ //Waits until theres an input from the SerialPort 
    ;
  }
  inputString = Serial.readString(); //Sets the input given by the user to a string
  inputString.toUpperCase(); //Makes that string upper case so comparisons against the AsciiCharacters array can work irregardless of case
  int startOfText; //will hold the position of the first character that isnt a space
  for (int i = 0;i < inputString.length();i++){ //Goes through the characters in the input until it finds one that isnt a space
    if (inputString.charAt(i) == " "){
      ; //Moves on to next character because this one is a space
    }else{ 
      startOfText = i; //Sets to the first non-space character
      break;
    }
  }
  String firstRealCharacter = inputString.substring(startOfText,(startOfText + 1)); // Stores what the first non-space character is

  if (firstRealCharacter == "*" || firstRealCharacter == "?" || firstRealCharacter == "<"){ //checks if the character is one used in BCCode 
    BCCodeToAscii(inputString); //The BCCode translation function is called
  }else if (firstRealCharacter == ""){ // checks if theres any real characters if not nothing happens 
    ;
  }else{ //if its not a character used in BCCode and the input was valid the text is presumed to be Ascii
    AsciiToBCCode(inputString,"True"); //The Ascii translation function is called
  }
}

//BCCode to ASCII

String BCCodeToAscii(String originalText){
  String endText; //The final message decoded from morse will be stored here
  String currentCharacter; //The current character that is being examined 
  String currentLetter; //The full morse letter that will be translated is stored here

  for (int i = 0; i < (originalText.length()); i++){ // Goes through every character of the original input given
    currentCharacter = String(originalText.charAt(i)); // Stores the current character 
    if (currentCharacter == "~"){ // when the end of a letter is signalled 
      endText += BCCodeToChar(currentLetter); // the translation of the morse letter is added to the final message
      currentLetter = ""; //resets current letter
    }else if (currentCharacter == " "){ // when theres a space between words
      endText += BCCodeToChar(currentLetter); // Also end of letter so translation of the morse letter is added to the final message
      endText += " "; // space also added to final message 
      currentLetter = ""; //resets current letter
    }else{ //if its not a new word or new letter
      currentLetter += currentCharacter; //add character to the characters in the current letter
    }
  }
  
  endText += BCCodeToChar(currentLetter); //the last letter if there is one is added to the final text
  Serial.println(endText); //outputs the completed translation
  LEDTransmissions(originalText,11); // Sends the BCCode to be converted to LED flashes

  commandResponse(endText); //finds if the text given is a command 

  readInput(); // Looks for a new input now translation is complete 
}

String BCCodeToChar(String currentLetter){
  for (int i = 0; i < 46;i++){ //checks through all the items in our BCCode array to see if it matches one of them
    if (currentLetter == (String(BCCode[i]))){
      return (String(AsciiCharacters[i])); // returns the corresponding Ascii character 
    }
  }
  return(""); // if it cant find a corresponding character nothing is returned
}


// checks if BCCode input is a command
void commandResponse(String asciiText){
  int numberStore; //Multiple commands require a number to be stored temporarily so this is multi-functional
  // the red LED is defined here because it is used in both BCxxx and BBxxxx so is more efficent to do beforehand
  #define redLED 5
  pinMode(redLED,OUTPUT);

  //BxB
  if ((asciiText.length() == 3) && (asciiText.charAt(0) == 'B') && (asciiText.charAt(2) == 'B') && isDigit(asciiText.charAt(1))) { 
    String stringOutput; // where the 4 digit potentiometer reading will be stored
    numberStore = (asciiText.charAt(1) - '0'); //Stores the number given as 'x' in BxB
    if ((numberStore % 2) == 0 && numberStore != 0){ //checks if 'x' is fully divisable by 2 
      //set up of the potentiometer for yse
      #define potentiometer A0
      pinMode (potentiometer,INPUT);
      int potentiometerReading = (analogRead(potentiometer)); //reads value of potentiometer and stores it
      
      //the following 7 lines of code ensures the potentiomiter reading is always a 4 digit number by adding zeros before the number if necessary
      if (potentiometerReading < 10){
        stringOutput = ("000" + String(potentiometerReading));
      }else if(potentiometerReading < 100){
        stringOutput = ("00" + String(potentiometerReading));
      }else if(potentiometerReading < 1000){
        stringOutput = ("0" + String(potentiometerReading));
      }

      AsciiToBCCode(stringOutput,"False"); //translates the potentiometer reading to BCCode without being shown in LED signals
    }

  }
  //PBxxx
  if ((asciiText.length() == 5) && (asciiText.substring(0,2) == "PB") && isDigit(asciiText.charAt(2)) && isDigit(asciiText.charAt(3)) && isDigit(asciiText.charAt(4))) {
    //following 3 lines of code stores the numbers that are given seperatly
    int num1 = (asciiText.charAt(2) - '0');
    int num2 = (asciiText.charAt(3) - '0');
    int num3 = (asciiText.charAt(4) - '0');
    
    numberStore = (num1 + num2 +num3); //adds up the numbers received for this command

    if (numberStore < 10){ //if the numbers added up is less than 10
      // sets up Infra-red receiver for use
      #define IRreceiver 2
      pinMode (IRreceiver,INPUT);
      
      if (digitalRead(IRreceiver) == 1){ //if the IR receiver value is read as 'high' its printed
        Serial.println("HIGH");
      }else if (digitalRead(IRreceiver) == 0){ //otherwise 'low' will be printed
        Serial.println("LOW");
      }

    }else{
      Serial.println("UNKNOWN");// unknown is displayed when the 3 numbers given adds up to 10 or more
    }
    
  }
  //BCxxx
  if ((asciiText.length() == 5) && (asciiText.substring(0,2) == "BC") && isDigit(asciiText.charAt(2)) && isDigit(asciiText.charAt(3)) && isDigit(asciiText.charAt(4))){
    numberStore = (asciiText.substring(2)).toInt(); // The three numbers are stored as a single integer
    if (numberStore <= 255){ //if that integer is less than or equal to 255
      analogWrite(redLED,numberStore); //the leds brightness is set to the value of numberStore 
      delay(250);
      analogWrite(redLED,0); //after 250ms the LED turns off 
    }
    
  }
  //BBxxxx
  if ((asciiText.length() == 6) && (asciiText.substring(0,2) == "BB") && isDigit(asciiText.charAt(2)) && isDigit(asciiText.charAt(3)) && isDigit(asciiText.charAt(4)) && isDigit(asciiText.charAt(5))){
    //Yellow, green, and orange LED's set up for use.
    #define yellowLED 9
    pinMode(yellowLED,OUTPUT);
    #define greenLED 10
    pinMode(greenLED,OUTPUT);
    #define orangeLED 6
    pinMode(orangeLED,OUTPUT);
    uint8_t LEDs[] = {yellowLED,redLED,greenLED,orangeLED}; // The defined names of LEDS in a unsigned integer array
    int LEDvalues[] = {(asciiText.charAt(2) - '0'),(asciiText.charAt(3) - '0'),(asciiText.charAt(4) - '0'),(asciiText.charAt(5) - '0')}; //the numbers given are each addded individually to this array and converted to a string
    // The arrays correspond with eachother
    for (int i =0;i<4;i++){ //goes through every LED turning them on if their corresponding value is bigger than 0
      if (LEDvalues[i] > 0){
        digitalWrite(LEDs[i],5);
      }
    }
    delay(100); //100 ms delay

    for (int i = 2; i <11;i++){ //Goes through every LED turning them off if their corresponding value is less than i
      for (int j = 0; j<4;j++){
        if (LEDvalues[j] < i ){
          digitalWrite(LEDs[j],0);
        }
      }
      delay(100); // 100ms delay
    }
    
  }
}


//ASCII to BCCode

void AsciiToBCCode(String originalText, String continueToLed){
  String endBCCode; //The final message decoded from BCCode will be stored here
  for (int i = 0; i < (originalText.length());i++) {// Goes through every character in the string
    endBCCode += (charToBCCode(originalText.substring(i,i+2),i,(originalText.length() -1))); //adds the translation for that character to the end string
  }
  Serial.println(endBCCode); //putputs the final translation

  if (continueToLed == "True"){ 
    LEDTransmissions(endBCCode,10); // Sends the BCCode to be converted to LED flashes
  }
  readInput(); //Looks for a new input now translation is complete 
}

String charToBCCode(String inputtedChar,int pointInArray,int lengthOfArray){
  String currentCharacter = (inputtedChar.substring(0,1)); // set as what the current character being translated is 
  String nextCharacter = (inputtedChar.substring(1,2)); //  set as what the next character to be translated is 
  String translation; // Holds the value to be returned

  if (pointInArray == lengthOfArray){ //Checks if current character is the last character in the string
    nextCharacter = ""; //if it is then sets the next character as nothing
  }
  for (int i = 0; i < 46;i++){ //checks all the Ascii characters for the one that matches
    if (currentCharacter == String(AsciiCharacters[i])){ //If the character is found
      translation = (BCCode[i]); //sets as the BCCode translation for the character
      break; // breaks loop once translation is found
    }
  }
  if (currentCharacter == " " || nextCharacter == ""){ //if this is the last character in the message or a space then add nothing to the end of it
    return translation;
  }else if (nextCharacter == " "){ //if the character is the last one in a word then add a space after 
    return (translation + " ");
  }else{
    return(translation + "~"); // if the character is not the last letter in the word then add a ~ after it
  }
}

//BCCode to LED
void LEDTransmissions(String BCCodeString, int pinType){
  //Set up of pin and potentiomiter
  #define ledPin pinType //sets the led to green or blue depending on the input 
  #define speedControl A0
  pinMode(ledPin, OUTPUT);
  pinMode (speedControl,INPUT);
  int potentiometerReading = (analogRead(speedControl)); //reads what the potentiomiter is set as
  int timeUnit = (map(potentiometerReading,0,1023,20,400)); //sets the dot length between 20-400ms depending on the position
  char currentCharacter;
  for (int i = 0; i < BCCodeString.length();i++){ //goes through every character in the BBCode
    currentCharacter = BCCodeString.charAt(i); //sets the variable to the current character
    if (currentCharacter == '?'){ //if its a question mark then the LED stays on for one time unit
      digitalWrite(pinType,5);
      delay(timeUnit);
      digitalWrite(pinType,0);
    }else if (currentCharacter == '*'){ //if its a asterisk then the LED stays on for two time units
      digitalWrite(pinType,5);
      delay(2 * timeUnit);
      digitalWrite(pinType,0);
    }else if (currentCharacter == '<'){ //if its a less than symbol then the LED stays on for 3 time units
      digitalWrite(pinType,5);
      delay(3 * timeUnit);
      digitalWrite(pinType,0);
    }else if (currentCharacter == '~'){ //if it is a space between letters 
      ; //the delay lasts 2 time units (including the constant delay which is applied twice )
    }else if (currentCharacter == ' '){
      delay(3 * timeUnit); //the delay lasts 5 time units in total (including the constant delay)
    }
    delay(timeUnit); // this 1 timeUnit delay is between every character and adds an extra time Unit to other delays
  }
}


void loop() {
  

}
