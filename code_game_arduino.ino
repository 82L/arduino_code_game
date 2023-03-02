#include <LiquidCrystal.h>
#define BUZZER_PIN 3
#define NOTE_B0  31
#define NOTE_C1  33
#define NOTE_CS1 35
#define NOTE_D1  37
#define NOTE_DS1 39
#define NOTE_E1  41
#define NOTE_F1  44
#define NOTE_FS1 46
#define NOTE_G1  49
#define NOTE_GS1 52
#define NOTE_A1  55
#define NOTE_AS1 58
#define NOTE_B1  62
#define NOTE_C2  65
#define NOTE_CS2 69
#define NOTE_D2  73
#define NOTE_DS2 78
#define NOTE_E2  82
#define NOTE_F2  87
#define NOTE_FS2 93
#define NOTE_G2  98
#define NOTE_GS2 104
#define NOTE_A2  110
#define NOTE_AS2 117
#define NOTE_B2  123
#define NOTE_C3  131
#define NOTE_CS3 139
#define NOTE_D3  147
#define NOTE_DS3 156
#define NOTE_E3  165
#define NOTE_F3  175
#define NOTE_FS3 185
#define NOTE_G3  196
#define NOTE_GS3 208
#define NOTE_A3  220
#define NOTE_AS3 233
#define NOTE_B3  247
#define NOTE_C4  262
#define NOTE_CS4 277
#define NOTE_D4  294
#define NOTE_DS4 311
#define NOTE_E4  330
#define NOTE_F4  349
#define NOTE_FS4 370
#define NOTE_G4  392
#define NOTE_GS4 415
#define NOTE_A4  440
#define NOTE_AS4 466
#define NOTE_B4  494
#define NOTE_C5  523
#define NOTE_CS5 554
#define NOTE_D5  587
#define NOTE_DS5 622
#define NOTE_E5  659
#define NOTE_F5  698
#define NOTE_FS5 740
#define NOTE_G5  784
#define NOTE_GS5 831
#define NOTE_A5  880
#define NOTE_AS5 932
#define NOTE_B5  988
#define NOTE_C6  1047
#define NOTE_CS6 1109
#define NOTE_D6  1175
#define NOTE_DS6 1245
#define NOTE_E6  1319
#define NOTE_F6  1397
#define NOTE_FS6 1480
#define NOTE_G6  1568
#define NOTE_GS6 1661
#define NOTE_A6  1760
#define NOTE_AS6 1865
#define NOTE_B6  1976
#define NOTE_C7  2093
#define NOTE_CS7 2217
#define NOTE_D7  2349
#define NOTE_DS7 2489
#define NOTE_E7  2637
#define NOTE_F7  2794
#define NOTE_FS7 2960
#define NOTE_G7  3136
#define NOTE_GS7 3322
#define NOTE_A7  3520
#define NOTE_AS7 3729
#define NOTE_B7  3951
#define NOTE_C8  4186
#define NOTE_CS8 4435
#define NOTE_D8  4699
#define NOTE_DS8 4978

#define CAR 1
#define ROAD 0
#define CHARACTER 2
#define BASE_DELAY 300




// Button's pins
const int btnLeftPin = A0;
const int btnRightPin = A1;

int btnLeftPressedPrevValue = 0;
int btnRightPressedPrevValue = 0;
bool btnLeftPressed = false;
bool btnRightPressed = false;

// Music Manager
//typedef struct MusicNote MusicNote;
struct MusicNote{
  int note;
  int duration;
  int time_before_note;
} ;
bool music_is_playing = false;
enum Musics {NONE, BUTTON};
Musics current_music = NONE;
int current_note = 0;
int current_wait_time_note = 0;
long int last_time_note_played = 0;

struct MusicNote music_buttons[] = {
  {
    .note = NOTE_A4,
    .duration = 100,
    .time_before_note = 0,
  },
  {
    .note = NOTE_F4,
    .duration = 100,
    .time_before_note = 50,
  }
};


// CUSTOM CHARACTERS INIT
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
LiquidCrystal lcd(4, 6, 10, 11, 12, 13);

//GAME INIT
//Game Map
int game_map[LCD_NUMBER_LINES][LCD_NUMBER_CHARACTERS] = {0};
int player_pos = 0;
//Time Details
int time_after_new_block = 0;
int time_before_placing_new_block = 0;
long int saved_time = -BASE_DELAY;
int time_block_speed = BASE_DELAY;

//Cars generation
int max_space_between_cars = 6;
int min_space_between_cars = 3;

//Game state
enum game_state {GAME_IS_RUNNING, SHOW_END_MESSAGE, END};
game_state current_game_state = GAME_IS_RUNNING;
int score = 0;

void setup() {
  Serial.begin(9600);
  
    // indique le nombre de caractères (16) et de lignes (2)
  	lcd.begin(LCD_NUMBER_CHARACTERS, LCD_NUMBER_LINES);
  
	//Create custom characters
  	lcd.createChar(CHARACTER, character);
  	lcd.createChar(ROAD, road);
  	lcd.createChar(CAR, car);  

    //Init btn
 	pinMode(btnLeftPin, INPUT_PULLUP);
  	pinMode(btnRightPin, INPUT_PULLUP);
  	btnRightPressedPrevValue = analogRead(btnRightPin) ;
    btnLeftPressedPrevValue = analogRead(btnLeftPin);
}

void initialize(){
  lcd.clear();
  score = 0;
  time_block_speed = BASE_DELAY;
  current_game_state = GAME_IS_RUNNING;
  memset(game_map, 0, sizeof(game_map));
  player_pos = 0;
}

void loop() {
  testPushBtn();
  
  switch (current_game_state){
    case GAME_IS_RUNNING :
    	update_player_position();
    	game_logic();
   		break;
    case SHOW_END_MESSAGE :
    	show_end_message();
    	break;
    case END :
    	test_restart_game();
    	break;
  };
 
  play_music();
  
}



void try_to_play_note(struct MusicNote music[]){
  long int current_time = millis();
  if (last_time_note_played + music[current_note].time_before_note > current_time ) {
  	tone(BUZZER_PIN, music[current_note].note, music[current_note].duration);
    current_note++;
    size_t n = sizeof(music)/sizeof(music[0]);
    if (n == current_note){
      current_note = 0;
      current_music = NONE;
    }
  }
  
}

void set_new_music(Musics new_music){
  current_note = 0;
  current_music = new_music;
  last_time_note_played = 0;
}

void play_music(){
  switch (current_music) {
    case NONE :
    	break;
    case BUTTON:
    	try_to_play_note(music_buttons);
    	break;
  };
}




void update_player_position(){
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

void test_restart_game(){
  	if (btnRightPressed) {
    	initialize();
        update_screen();
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
        time_block_speed = BASE_DELAY - (score * 2);
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
  int btnLeft = analogRead(btnLeftPin);
  int btnRight = analogRead(btnRightPin);
  
  btnLeftPressed = (btnLeft < btnLeftPressedPrevValue);
  btnRightPressed = (btnRight < btnRightPressedPrevValue);
  btnLeftPressedPrevValue = btnLeft;
  btnRightPressedPrevValue = btnRight;
  if(btnLeftPressed || btnRightPressed){
    set_new_music(BUTTON);
  }
  
}