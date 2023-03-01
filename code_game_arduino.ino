#include <LiquidCrystal.h>
#define CAR 1
#define ROAD 0
#define CHARACTER 2
#define BASE_SPEED 100

//BUTTONS INIT

// Button's pins
const int btnLeftPin = A0;
const int btnRightPin = A1;

// Button's parameters
int btnLeft = 0, btnRight = 0;
bool btnLeftPressed = false, btnRightPressed = false;

// CUSTOM CHARACTERS INIT
byte heart[8] = {
  0b00000,
  0b01010,
  0b11110,
  0b11111,
  0b11111,
  0b01110,
  0b00100,
  0b00000
};
byte car[8] = {
  0b11111,
  0b00000,
  0b10001,
  0b01110,
  0b01110,
  0b10001,
  0b00000,
  0b11111
};
byte road[8] = {
  0b11111,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b11111
};
byte character[8] = {
  0b11111,
  0b00000,
  0b00100,
  0b01110,
  0b00100,
  0b01010,
  0b00000,
  0b11111
};

//LCD INIT
const int LCD_NUMBER_CHARACTERS = 16;
const int LCD_NUMBER_LINES = 2;
// initialisation, on définit les ports pour RS, E et D4 à D7
LiquidCrystal lcd(12, 11, 4, 5, 6, 7);

//GAME INIT
//Game Map
int game_map[LCD_NUMBER_LINES][LCD_NUMBER_CHARACTERS] = {0};
int player_pos = 0;
//Time Details
int time_after_new_block = 0;
int time_before_placing_new_block = 0;
long int saved_time = -BASE_SPEED;
int time_block_speed = BASE_SPEED;

//Cars generation
int max_space_between_cars = 6;
int min_space_between_cars = 3;

//Game state
enum game_state {GAME_IS_RUNNING, SHOW_END_MESSAGE, END};
enum game_state current_game_state = GAME_IS_RUNNING;
int score = 0;

void setup() {
    // indique le nombre de caractères (16) et de lignes (2)
  	lcd.begin(LCD_NUMBER_CHARACTERS, LCD_NUMBER_LINES);
	//Create custom characters
  	lcd.createChar(CHARACTER, character);
  	lcd.createChar(ROAD, road);
  	lcd.createChar(CAR, car);  

    //Init btn
 	pinMode(btnLeftPin, INPUT_PULLUP);
  	pinMode(btnRightPin, INPUT_PULLUP);
   
}
void initialize(){
  lcd.clear();
  score = 0;
  time_block_speed = BASE_SPEED;
  current_game_state = GAME_IS_RUNNING;
  memset(game_map, 0, sizeof(game_map));
  player_pos = 0;
}
void loop() {
  testPushBtn();
  switch (current_game_state){
    case GAME_IS_RUNNING :
    	game_logic();
   		break;
    case SHOW_END_MESSAGE :
    	show_end_message();
    	break;
  }
}

void game_logic(){
  	long int current_time = millis();
  	if ( current_time - time_block_speed > saved_time ){
    	saved_time = current_time;
  		time_after_new_block++;
    	actualize_grid();
      	test_player_position();
    	update_screen();
  	}	
}
void test_player_position(){
  	if(game_map[player_pos][0] == CAR){
      	current_game_state = SHOW_END_MESSAGE;
  	}
    else if (game_map[1 - player_pos][0] == CAR){
    	score+=1;
        time_block_speed = BASE_SPEED - (score * 2);
    }
}
void show_end_message(){
  	lcd.clear();
   	lcd.setCursor(0, 0);
	//lcd.print("Game Over");
  	lcd.print("score: ");
  	lcd.print(score);
    lcd.setCursor(4, 1);
  	lcd.print("R to restart");
    current_game_state = END;
}

void actualize_grid(){
  for(int i = 0; i < LCD_NUMBER_CHARACTERS - 1; i++){
    	game_map[0][i] = game_map[0][i + 1];
    	game_map[1][i] = game_map[1][i + 1];
  	}
    
  	game_map[0][LCD_NUMBER_CHARACTERS - 1] = ROAD;
  	game_map[1][LCD_NUMBER_CHARACTERS - 1] = ROAD;
    
  	if(time_after_new_block >= time_before_placing_new_block){
    	time_after_new_block = 0;
    	time_before_placing_new_block = random(min_space_between_cars, max_space_between_cars);
    	int block_position = random(0,LCD_NUMBER_LINES);
    	game_map[block_position][LCD_NUMBER_CHARACTERS - 1] = CAR;               
  	}
    
}
  
void update_screen(){
  	lcd.setCursor(0, 0);
 	print_line(0);
  	lcd.setCursor(0, 1);
  	print_line(1);
    lcd.setCursor(0, player_pos);
}
  

void print_line(int line){
   for(int i = 0; i < LCD_NUMBER_CHARACTERS; i++){
     if(i == 0 && player_pos == line){
       lcd.write(byte(CHARACTER));
     }
     else if(game_map[line][i] == CAR){
       lcd.write(byte(CAR));  
     }
     else{
       lcd.write(byte(ROAD));  	
     }
    //lcd.print(game_map[line][i]);
  }
}

void testPushBtn() {
  // Read pushbutton
  btnLeft = analogRead(btnLeftPin);
  btnRight = analogRead(btnRightPin);
  
  btnLeftPressed = (btnLeft < 200);
  btnRightPressed = (btnRight < 200);
  
  if(current_game_state == GAME_IS_RUNNING){
  	if (btnLeftPressed){
    	player_pos = 0;
      	test_player_position();
    	update_screen();
  	} else if (btnRightPressed) {
    	player_pos = 1;
      	test_player_position();
    	update_screen();
  	}
  }
  else if(current_game_state == END){
    if (btnRightPressed) {
    	initialize();
    	update_screen();
  	}
  }
}