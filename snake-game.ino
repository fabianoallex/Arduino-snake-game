#include <LiquidCrystal.h>

/*************************************************************************************************************
*******************************BIT ARRAY *********************************************************************
**************************************************************************************************************
mais informações aqui: http://fabianoallex.blogspot.com.br/2015/09/arduino-array-de-bits.html
**************************************************************************************************************/
class BitArray{
  private:
    int _num_bits;   //quantidade de bits a serem gerenciados
    int _num_bytes;  //quantidade de bytes utilizados para armazenar os bits a serem gerenciados
    byte * _bytes;   //array de bytes onde estarão armazenados os bits
  public:
    BitArray(int num_bits){
      _num_bits  = num_bits;
      _num_bytes = _num_bits/8 + (_num_bits%8 ? 1 : 0) + 1;
      _bytes = (byte *)(malloc( _num_bytes * sizeof(byte) ) );
    }
    void write(int index, byte value) {
      byte b = _bytes[ index/8 + (index%8 ? 1 : 0) ];
      unsigned int bit = index%8;
      if (value) { b |= (1 << bit); } else { b &= ~(1 << bit);  }
      _bytes[ index/8 + (index%8 ? 1 : 0) ] = b;
    }
    void write(byte value) { for(int j=0; j<_num_bytes;j++) { _bytes[j] = value ? B11111111 : B00000000;  }  }
    int read(int index) {
      byte b = _bytes[ index/8 + (index%8 ? 1 : 0) ];
      unsigned int bit = index%8;
      return (b & (1 << bit)) != 0;
    }
    ~BitArray(){ free ( _bytes ); }
};

class BitArray2D {
  private:
    unsigned int _rows;
    unsigned int _columns;
    unsigned int _cols_array; //pra cada 8 colunas, 1 byte é usado 
    byte**       _bits;
  public:
    BitArray2D(unsigned int rows, unsigned int columns){
      _rows       = rows;
      _columns    = columns;
      _cols_array = columns/8 + (_columns%8 ? 1 : 0) + 1; //divide por 8 o número de colunas
      _bits = (byte **)malloc(_rows * sizeof(byte *));
      for(int i=0;i<_rows;i++){ _bits[i] = (byte *)malloc(  _cols_array  *  sizeof(byte)); } //cria varios arrays
      clear();
    }
    unsigned int rows(){ return _rows; }
    unsigned int columns(){ return _columns; }
     
    void clear() { 
      for(int i=0;i<_rows;i++){      
        for(int j=0; j<_cols_array;j++) { _bits[i][j] = B00000000; }       
      }   
    }
   
    void write(unsigned int row, unsigned int column, int value){
      byte b = _bits[row][ column/8 + (column%8 ? 1 : 0) ];
      unsigned int bit = column%8;
       
      if (value) { b |= (1 << bit); } else { b &= ~(1 << bit);  }
       
      _bits[row][ column/8 + (column%8 ? 1 : 0) ] = b;
    }
     
    void write(byte value){
      for(int i=0;i<_rows;i++){      
        for(int j=0; j<_cols_array;j++) {      
          _bits[i][j] = value ? B11111111 : B00000000;     
        }       
      }  
    }
     
    int read(unsigned int row, unsigned int column){
      byte b = _bits[row][ column/8 + (column%8 ? 1 : 0) ];
      unsigned int bit = column%8;
       
      return (b & (1 << bit)) != 0;
    }
     
    void toggle(unsigned int row, unsigned int column){ write(row, column, !read(row, column)); }
    void toggle(){ for(int i=0;i<_rows;i++){      for(int j=0; j<_columns;j++) {      toggle(i,j);   }   }   }
};
 
/*************************************************************************************************************
*******************************FIM BIT ARRAY *****************************************************************
**************************************************************************************************************/

/*************************************************************************************************************
************************************CLASSE UNIQUE RANDOM******************************************************
**************************************************************************************************************
mais informações aqui: http://fabianoallex.blogspot.com.br/2015/09/arduino-numeros-aleatorio-repetidos-e.html
*************************************************************************************************************/
  
class UniqueRandom{
  private:
    int _index;
    int _min;
    int _max;
    int _size;
    int* _list;
    void _init(int min, int max) {
      _list = 0; 
      if (min < max) { _min = min; _max = max; } else { _min = max; _max = min; }
      _size = _max - _min; 
      _index = 0;
    }    
  public:
    UniqueRandom(int max)           { _init(0,   max); randomize(); } //construtor com 1 parametro
    UniqueRandom(int min, int max)  { _init(min, max); randomize(); } //construtor com 2 parametros
      
    void randomize() {
      _index = 0;
       
      if (_list == 0) { _list = (int*) malloc(size() * sizeof(int)); }  
      for (int i=0; i<size(); i++) {   _list[i] = _min+i;  }   //preenche a lista do menor ao maior valor
        
      //embaralha a lista
      for (int i=0; i<size(); i++) {  
        int r = random(0, size());     //sorteia uma posição qualquer
        int aux = _list[i];               
        _list[i] = _list[r];
        _list[r] = aux;
      }
    }
      
    int next() {                                  //retorna o proximo numero da lista
      int n = _list[_index++];
      if (_index >= size() ) { _index = 0;} //após recuper o ultimo numero, recomeça na posicao 0
      return n;
    }
      
    int size() { return _size; }
      
    ~UniqueRandom(){ free ( _list ); }  //destrutor
};
/*************************************************************************************************************
************************************FIM CLASSE UNIQUE RANDOM**************************************************
*************************************************************************************************************/

/*************************************************************************************************************
*******************************CLASSE SNAKE GAME**************************************************************
**************************************************************************************************************/
struct Position {
  int lin;
  int col;
};

const int SNAKE_MAX_LEN   = 30;  //tamanho maximo da cobra
const int SNAKE_TIME_INIT = 500; //tempo entre deslocamento da cobra (velocidade)
const int SNAKE_TIME_INC  = 15;  //incremento da velocidade
 
enum Direction { DIR_STOP, DIR_TOP, DIR_LEFT, DIR_BOTTOM, DIR_RIGHT};
enum SnakeStatus { SNAKE_GAME_ON, SNAKE_GAME_OVER };
 
class SnakeGame{
  private:
    BitArray2D * _display;
    Position _snake_positions[SNAKE_MAX_LEN];
    Position _apple;
    int _length;
    Direction _direction;
    unsigned long _last_millis;
    int _time;
    int _score;
    SnakeStatus _snakeStatus;
     
    UniqueRandom * _ur;  //utilizado no game over
     
    void _generateApple() {
      int lin, col;
      boolean random_ok = false;
       
      while (!random_ok) {
        random_ok = true;
        lin = random(0, _display->rows()-1);
        col = random(0, _display->columns()-1);
         
        for (int p=0; p<_length; p++){
          if (_snake_positions[p].col==col && _snake_positions[p].lin==lin){ //verifica se gerou em um local que não seja a cobra
            random_ok = false;
            break;
          }
        }
      }
      _apple.lin = lin;
      _apple.col = col;
    }
     
    void _gameOver(){ 
      _snakeStatus = SNAKE_GAME_OVER; 
      _direction   = DIR_STOP;
      _time = 20;
    }
     
    void _inc_length(){
      _length++; _score++;
      _time -= SNAKE_TIME_INC;
    }
     
    void _runGameOver(){
      int r = _ur->next();
      int lin = (r / _display->columns());
      int col = (r % _display->columns());
       
      _display->write(lin, col, HIGH );
       
      //if ( r>=(_ur->size()-1) || _direction != DIR_STOP ) {  
      if ( r>=(_ur->size()-1) ) {  
        _ur->randomize();
        start(); 
      }
    }
     
    void _run(){
      for (int i=_length-1; i>0; i--){
        _snake_positions[i].lin = _snake_positions[i-1].lin;
        _snake_positions[i].col = _snake_positions[i-1].col;
      }
       
      if (_direction == DIR_TOP )    { _snake_positions[0].lin--;  }
      if (_direction == DIR_BOTTOM ) { _snake_positions[0].lin++;  }
      if (_direction == DIR_LEFT )   { _snake_positions[0].col--;  }
      if (_direction == DIR_RIGHT )  { _snake_positions[0].col++;  }
       
      //verifica se ultrapassou o limite do display
      if (_snake_positions[0].lin < 0)                     { _gameOver(); }
      if (_snake_positions[0].lin >= _display->rows() )    { _gameOver(); }
      if (_snake_positions[0].col < 0)                     { _gameOver(); }
      if (_snake_positions[0].col >= _display->columns() ) { _gameOver(); }
       
      //verifica se colidiu na cobra
      for (int i=_length-1; i>0; i--){
        if (_snake_positions[i].lin == _snake_positions[0].lin && _snake_positions[i].col == _snake_positions[0].col) {
          _gameOver();
        }  
      }
       
      //verifica se comeu a maça
      if (_snake_positions[0].col == _apple.col && _snake_positions[0].lin == _apple.lin){
        _inc_length();
         
        if (_length > SNAKE_MAX_LEN) { _length = SNAKE_MAX_LEN; } else {
          _snake_positions[_length-1].lin = _snake_positions[_length-2].lin;
          _snake_positions[_length-1].col = _snake_positions[_length-2].col;
        }
        _generateApple();
      }
       
      //update display
      for (int lin=0; lin<_display->rows(); lin++) {
        for (int col=0; col<_display->columns(); col++) {
          for (int p=0; p<_length; p++){
            boolean val = _snake_positions[p].col==col && _snake_positions[p].lin==lin;
            _display->write( lin, col,  val );
            if (val) {break;}
          }
        }
      }
      _display->write(_apple.lin, _apple.col, HIGH);
      //--
    }
     
  public:
    SnakeGame(BitArray2D * display){ 
      _display = display;
      _ur = new UniqueRandom( _display->rows() * _display->columns() );
      start();
    }
     
    void start(){
      _length = 1;
      _score  = 0;
      _time = SNAKE_TIME_INIT;
      _last_millis = 0;
      _snake_positions[0].lin = _display->rows() / 2;
      _snake_positions[0].col = _display->columns() / 2;
      _direction = DIR_STOP;
       
      _snakeStatus = SNAKE_GAME_ON;
       
      _generateApple();
    }
     
    void left()   { if (_direction == DIR_RIGHT)  return; _direction = DIR_LEFT;   }
    void right()  { if (_direction == DIR_LEFT)   return; _direction = DIR_RIGHT;  }
    void top()    { if (_direction == DIR_BOTTOM) return; _direction = DIR_TOP;    }
    void bottom() { if (_direction == DIR_TOP)    return; _direction = DIR_BOTTOM; }
     
    int getScore(){ return _score; }
    
    Direction getDirection(){ return _direction; }
     
    int update(){
      int r = false;
       
      if (millis() - _last_millis > _time) {
        r = true;
        _last_millis = millis();
       
        if (_snakeStatus == SNAKE_GAME_ON)   { _run();         } 
        if (_snakeStatus == SNAKE_GAME_OVER) { _runGameOver(); }
      }
       
      return r; //r-->indica se houve mudança no display
    }
};
 
/*************************************************************************************************************
*******************************FIM CLASSE SNAKE GAME**********************************************************
**************************************************************************************************************/
 
/*************************************************************************************************************
************************************CLASSE ROTARY ENCODER*****************************************************
mais informações: http://fabianoallex.blogspot.com.br/2016/05/arduino-rotary-encoder.html
*************************************************************************************************************/
#define ROTARY_NO_BUTTON     255
    
struct RotaryEncoderLimits{
  int min;
  int max;
};
    
class RotaryEncoder {
  private:
    byte _pin_clk;
    byte _pin_dt;
    byte _pin_sw;
    volatile byte _num_results;
    volatile int _result;
    volatile int * _results;
    byte _index_result;
    RotaryEncoderLimits * _limits;
        
    boolean _a;
    boolean _b;
  public:
    RotaryEncoder(byte pin_clk, byte pin_dt, byte pin_sw = ROTARY_NO_BUTTON, byte num_results=1, RotaryEncoderLimits * limits=0){   //parametro do botao opcional
      _pin_clk = pin_clk;
      _pin_dt = pin_dt;
      _pin_sw = pin_sw;
      pinMode(_pin_clk, INPUT);
      pinMode(_pin_dt, INPUT);
      if (_pin_sw != ROTARY_NO_BUTTON){ 
        pinMode(_pin_sw, INPUT); 
        digitalWrite(_pin_sw, HIGH);
      }
      if (num_results == 0) { num_results = 1; }
      _num_results = num_results;
      _results = new int[_num_results];
      for (int i; i<_num_results; i++){ _results[i] = (limits) ? limits[i].min : 0; }
      _index_result = 0;
      _limits = limits;
      _a = false;
      _b = false;
    }
    byte getIndex() { return _index_result; }
    void next()     { _index_result++; if (_index_result >= _num_results) { _index_result = 0; } } 
    void update_a() {
      _result = 0;
      delay (1);
      if( digitalRead(_pin_clk) != _a ) { 
        _a = !_a;
        if ( _a && !_b ) { _result = -1; }
      }
      if (_results[_index_result]+_result >= _limits[_index_result].min && 
          _results[_index_result]+_result <= _limits[_index_result].max ) {
        _results[_index_result] += _result;
      }
    }
    void update_b() {
      _result = 0;
      delay (1);
      if( digitalRead(_pin_dt) != _b ) {  
        _b = !_b;
        if ( _b && !_a ) { _result = +1; }
      }
      if (_results[_index_result]+_result >= _limits[_index_result].min && 
          _results[_index_result]+_result <= _limits[_index_result].max ) {
        _results[_index_result] += _result;
      }
    }
    int read(){ return _result; }                                        //retorn -1, 0 ou 1.
    int getValue(int index=-1) {                                         //retorna o valor da variável corrente ou a passada como parametro
      if (index < 0 ){ return _results[_index_result]; }
      return _results[index];
    }
    void setValue(int value, int index=-1){ _results[ (index==-1) ? _index_result : index] = value; }         //caso a variável inicializa em determinado valor diferente de zero, utilizar esse método.
    int buttonRead(){ return (_pin_sw == ROTARY_NO_BUTTON) ? LOW : digitalRead(_pin_sw); }
};
/*************************************************************************************************************
************************************FIM CLASSE ROTARY ENCODER*************************************************
*************************************************************************************************************/
  
  
/*************************************************************************************************************
*******************************CLASSE SNAKE LCD***************************************************************
**************************************************************************************************************/
/*
  0        1        2        3        4        5        6
 ***                        ***      ***               ***                 
 ***                        ***      ***               ***          
                                                            
 ***      ***               ***               ***             
 ***      ***               ***               ***             
                                                              
 ***      ***      ***               ***                        
 ***      ***      ***               ***                            
*/
byte c0[8] = {B11111,  B11111,  B00000,  B11111,  B11111,  B00000,  B11111,  B11111 };
byte c1[8] = {B00000,  B00000,  B00000,  B11111,  B11111,  B00000,  B11111,  B11111 };
byte c2[8] = {B00000,  B00000,  B00000,  B00000,  B00000,  B00000,  B11111,  B11111 };
byte c3[8] = {B11111,  B11111,  B00000,  B11111,  B11111,  B00000,  B00000,  B00000 };
byte c4[8] = {B11111,  B11111,  B00000,  B00000,  B00000,  B00000,  B11111,  B11111 };
byte c5[8] = {B00000,  B00000,  B00000,  B11111,  B11111,  B00000,  B00000,  B00000 };
byte c6[8] = {B11111,  B11111,  B00000,  B00000,  B00000,  B00000,  B00000,  B00000 };

class SnakeLCD {
  private:
    LiquidCrystal * _lcd;  //ponteiro para um objeto lcd
  public:
    void createChars() { 
      _lcd->createChar(0, c0);  
      _lcd->createChar(1, c1);  
      _lcd->createChar(2, c2); 
      _lcd->createChar(3, c3);  
      _lcd->createChar(4, c4);
      _lcd->createChar(5, c5);
      _lcd->createChar(6, c6);
    }
    SnakeLCD(LiquidCrystal * lcd) {  _lcd = lcd;  }
    
    void write(byte col, byte row, byte val){
      _lcd->setCursor(col, row);
      if (val == B000) { _lcd->print(" ");  }
      if (val == B111) { _lcd->write((byte)0);  }
      if (val == B011) { _lcd->write((byte)1);  }
      if (val == B001) { _lcd->write((byte)2);  }
      if (val == B110) { _lcd->write((byte)3);  }
      if (val == B101) { _lcd->write((byte)4);  }
      if (val == B010) { _lcd->write((byte)5);  }
      if (val == B100) { _lcd->write((byte)6);  }
    }
};
/*************************************************************************************************************
*******************************FIM CLASSE SNAKE LCD***********************************************************
**************************************************************************************************************/

 
/*************************************************************************************************************
*******************************DECLARACAO DOS OBJETOS*********************************************************
**************************************************************************************************************/

/*

  tamanho display real

 * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * *
 * * * * * * * * * * * * * * * *
*/

const int LINHAS  = 6;
const int COLUNAS = 16;
 
LiquidCrystal lcd(12, 11, 10, 9, 8, 7);
SnakeLCD snakeLcd(&lcd);
 
BitArray2D ba(LINHAS, COLUNAS); //8 linhas e 20 colunas, usada para armazenar o estado do display
SnakeGame snake(&ba);

RotaryEncoderLimits lim[] = { {-1000,1000} };  //limites máximos e mínimos que as variaveis podem atingir
RotaryEncoder       re(A0, A1, 4, 1, lim);  //pino clk, pino dt, pino sw, variaveis, limites

/*************************************************************************************************************
*******************************FIM DECLARACAO DOS OBJETOS*****************************************************
**************************************************************************************************************/
  
/*************************************************************************************************************
*******************************TRATAMENTO DAS INTERRUPÇÕES****************************************************
**************************************************************************************************************/
//interrupções dos pinos A0 e A1 via Pin Change Interrupt
ISR(PCINT1_vect) {
  volatile static byte lastVal_a0 = LOW;
  volatile static byte lastVal_a1 = LOW;
  byte val_a0 = digitalRead(A0);
  byte val_a1 = digitalRead(A1);
  if (lastVal_a0 != val_a0){ re.update_a(); lastVal_a0 = val_a0; }
  if (lastVal_a1 != val_a1){ re.update_b(); lastVal_a1 = val_a1; }
}
 
void setup_interrupts(){
  //-----PCI - Pin Change Interrupt ----
  pinMode(A0,INPUT);   // set Pin as Input (default)
  digitalWrite(A0,HIGH);  // enable pullup resistor
  pinMode(A1,INPUT);   // set Pin as Input (default)
  digitalWrite(A1,HIGH);  // enable pullup resistor
  cli();
  PCICR |= 0b00000010; // habilita a porta C - Pin Change Interrupts
  PCMSK1 |= 0b00000011; // habilita interrupção da porta c nos pinos: PCINT8 (A0) e PCINT9(A1)
  sei();
}
 
/*************************************************************************************************************
*******************************FIM TRATAMENTO DAS INTERRUPÇÕES****************************************************
**************************************************************************************************************/
   
void update_display() {
  for (int col=0; col<ba.columns(); col++) {
    byte lin_lcd = 0;
    byte cont = 0;
    byte val = 0;
    
    for (int lin=0; lin<ba.rows(); lin++) {
      if (cont == 0){ val = 0; }
      val = val << 1; 
      val = val | ba.read(lin, col);
      cont++;
      if (cont == 3) {
        snakeLcd.write(col, lin_lcd, val) ;
        cont = 0;
        lin_lcd++;
      }
    }
  }
}
   
void setup() { 
  Serial.begin(9600);
  setup_interrupts();
  lcd.begin(16, 2);
  snakeLcd.createChars();
  randomSeed(analogRead(A2));
  re.setValue(0, 0);  //inicializa o rotary em 0
}
   
void loop() {
  static int val_encoder = 0;
  static boolean change = false;
  
  if (re.getValue(0) < val_encoder && !change){ 
    if (snake.getDirection() == DIR_STOP)  {snake.right(); } else
    if (snake.getDirection() == DIR_TOP) { snake.left();   } else
    if (snake.getDirection() == DIR_RIGHT) { snake.top();  } else
    if (snake.getDirection() == DIR_LEFT) { snake.bottom();  } else
    if (snake.getDirection() == DIR_BOTTOM) { snake.right();   } 
    
    change = true;
  }
  
  if (re.getValue(0) > val_encoder && !change ){
    if (snake.getDirection() == DIR_STOP)  {snake.right();   }  else
    if (snake.getDirection() == DIR_TOP) { snake.right();    } else
    if (snake.getDirection() == DIR_RIGHT) { snake.bottom(); } else
    if (snake.getDirection() == DIR_LEFT) { snake.top();     } else
    if (snake.getDirection() == DIR_BOTTOM) { snake.left();  } 
    
    change = true;
  }  
  
  val_encoder = re.getValue(0);
  if ( snake.update() ) { 
    change = false;
    update_display(); 
    Serial.println(val_encoder);
  }  
  
  //controla o click do botao do enconder
  static byte b = HIGH; //pra ler apenas uma vez o botao ao pressionar
  if( re.buttonRead() == LOW && b != re.buttonRead() ) {
    re.next();           //passa para a próxima variável (index)
    delay(200);          //debounce meia boca
  }
  b = re.buttonRead();
}
