#include <avr/interrupt.h>
#include <avr/io.h>
#include <avr/pgmspace.h>

// ----------------------------------- for synthesis ------------------------------------------

#ifndef cbi
#define cbi(sfr, bit) (_SFR_BYTE(sfr) &= ~_BV(bit))
#endif
#ifndef sbi
#define sbi(sfr, bit) (_SFR_BYTE(sfr) |= _BV(bit))
#endif
// Standard Arduino Pins
#define digitalPinToPortReg(P) \
        (((P) >= 0 && (P) <= 7) ? &PORTD : (((P) >= 8 && (P) <= 13) ? &PORTB : &PORTC))
#define digitalPinToDDRReg(P) \
        (((P) >= 0 && (P) <= 7) ? &DDRD : (((P) >= 8 && (P) <= 13) ? &DDRB : &DDRC))
#define digitalPinToPINReg(P) \
        (((P) >= 0 && (P) <= 7) ? &PIND : (((P) >= 8 && (P) <= 13) ? &PINB : &PINC))
#define digitalPinToBit(P) \
        (((P) >= 0 && (P) <= 7) ? (P) : (((P) >= 8 && (P) <= 13) ? (P) -8 : (P) -14))
#define digitalReadFast(P) bitRead(*digitalPinToPINReg(P), digitalPinToBit(P))
#define digitalWriteFast(P, V) bitWrite(*digitalPinToPortReg(P), digitalPinToBit(P), (V))
const unsigned char PS_2 = (1 << ADPS0);;
const unsigned char PS_4 = (1 << ADPS1);
const unsigned char PS_8 = (1 << ADPS1) | (1 << ADPS0);
const unsigned char PS_16 = (1 << ADPS2);
const unsigned char PS_32 = (1 << ADPS2) | (1 << ADPS0);
const unsigned char PS_64 = (1 << ADPS2) | (1 << ADPS1);
const unsigned char PS_128 = (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
uint32_t NOTES[12]={208065>>2,220472>>2,233516>>2,247514>>2,262149>>2,277738>>2,
  294281>>2,311779>>2,330390>>2,349956>>2,370794>>2,392746>>2};
const uint8_t sinetable[256] PROGMEM = {
        127,130,133,136,139,143,146,149,152,155,158,161,164,167,170,173,176,178,
        181,184,187,190,192,195,198,200,203,205,208,210,212,215,217,219,221,
        223,225,227,229,231,233,234,236,238,239,240,
        242,243,244,245,247,248,249,249,250,251,252,252,253,253,253,254,254,254,
        254,254,254,254,253,253,253,252,252,251,250,249,249,248,247,245,244,
        243,242,240,239,238,236,234,233,231,229,227,225,223,
        221,219,217,215,212,210,208,205,203,200,198,195,192,190,187,184,181,178,
        176,173,170,167,164,161,158,155,152,149,146,143,139,136,133,130,127,
        124,121,118,115,111,108,105,102,99,96,93,90,87,84,81,78,
        76,73,70,67,64,62,59,56,54,51,49,46,44,42,39,37,35,33,31,29,27,25,23,21,
        20,18,16,15,14,12,11,10,9,7,6,5,5,4,3,2,2,1,1,1,0,0,0,0,0,0,0,1,1,1,
        2,2,3,4,5,5,6,7,9,10,11,12,14,15,16,18,20,21,23,25,27,29,31,
        33,35,37,39,42,44,46,49,51,54,56,59,62,64,67,70,73,76,78,81,84,87,90,93,
        96,99,102,105,108,111,115,118,121,124
};
const uint8_t ATTrates[32]={
        1,2,3,4,5,8,12,20,32,37,43,51,64,85,128,255,0xFF,0xFF,0xFF,0xFF,0xFF,
        0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF
};
const uint8_t RELrates[32]={
        1,2,3,4,5,8,12,20,32,37,43,51,64,85,128,255,255,128,85,64,51,43,37,32,
        20,12,8,5,4,3,2,1
};
volatile uint8_t lfocounter = 0;
volatile uint8_t lfocounter2 = 0;
volatile uint16_t lfoval = 0;
volatile uint16_t lfoval2 = 0;
volatile uint8_t GATED=1;
uint8_t OSCNOTES[4];
int16_t volume=0;
int16_t volume_max = 255;
uint8_t ENVsmoothing;
uint8_t envcnt=10;
//-------- Synth parameters --------------
uint32_t FREQ[16]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};  //DCO pitch
volatile uint32_t DETUNE=0;  //Osc spread or detune
volatile uint8_t CUTOFF=0;  //freq 0-255
volatile uint8_t RESONANCE=0;  //resonance=0-255
volatile uint16_t LFO=32;  //Lfo rate 0-255
volatile uint8_t VCA=255;  //VCA level 0-255
volatile uint8_t ATTACK=1;  // ENV Attack rate 0-255
volatile uint8_t RELEASE=1;  // ENV Release rate 0-255
volatile uint8_t ENVELOPE=0;  // ENV Shape
volatile uint8_t TRIG=0;  //MIDItrig 1=note ON
volatile int16_t BEND;  //Pitchbend
volatile int16_t MOD;  //MODwheel
//-----------------------------------------
volatile int16_t BENDoffset;  //Pitchbend center
volatile uint32_t olddetune;
uint32_t DCOPH[16];
uint8_t integrators[16];
uint8_t delayline[256];
volatile uint8_t writepointer;
volatile uint8_t PHASERMIX;
uint8_t DCO;
int16_t DCF;
int16_t ENV;
int16_t M0;
int16_t M1;
int16_t M2;
int16_t M3;
int16_t M4;
int16_t M5;
int16_t M6;
int16_t MX1;
int16_t MX2;
int8_t coefficient;
volatile uint8_t MIDISTATE=0;
volatile uint8_t MIDIRUNNINGSTATUS=0;
volatile uint8_t MIDINOTE;
volatile uint8_t MIDIVEL;

uint32_t MIDI2FREQ(uint8_t note) {
    uint8_t key=note%12;
    if (note<36) return (NOTES[key]>>(1+(35-note)/12));
    if (note>47) return (NOTES[key]<<((note-36)/12));
    return NOTES[key];
}

// ----------------------------------- for control ------------------------------------------
#define PROCESS_NUM         2
#define TRIG_PRESSED_KEY    0
#define TRIG_RELEASED_KEY   1

#define LONGPRESS_WAITTIME  100
#define SEQ_LENGTH          16
#define BANK_NUM            4

#define KEY_NUM             40

volatile uint32_t timer_count=0, timer_count_prev=0;

//プロセス（機能）管理用
byte trigger_flag[PROCESS_NUM];
bool isSettingMode=false;

//ボタン管理用
int8_t keytable[KEY_NUM], oldkeytable[40];
uint8_t keyscan_num=0;
volatile byte key_status[KEY_NUM];      //Released(0), Pressed(2), Latched(4), LongPressed(8)
uint32_t key_pressingtime[KEY_NUM];
byte key_longpressed = 0;

//スケール選択・チューニング選択・オクターブシフト・キー範囲オフセット
uint8_t scale_select=0;
uint8_t key_select=0;
uint8_t key_oct=1;
uint8_t key_offset = 0;

byte key2midi_normal[KEY_NUM*2]={
    36, 38, 40, 41, 43, 45, 47, 48,    50, 52, 53, 55, 57, 59, 60, 62,
    43, 45, 47, 48, 50, 52, 53, 55,    57, 59, 60, 62, 64, 65, 67, 69,
    48, 50, 52, 53, 55, 57, 59, 60,    62, 64, 65, 67, 69, 71, 72, 74,
    52, 54, 56, 57, 59, 61, 63, 64,    66, 68, 69, 71, 73, 75, 76, 78,
    52, 53, 55, 57, 59, 60, 62, 64,    65, 67, 69, 71, 72, 74, 76, 77};

byte key2midi_minBlues[KEY_NUM*2]={
    43, 45, 48, 50, 52, 55, 57, 60,    62, 64, 67, 69, 72, 74, 76, 79,
    43, 45, 48, 50, 52, 55, 57, 60,    62, 64, 67, 69, 72, 74, 76, 79,
    43, 45, 48, 50, 52, 55, 57, 60,    62, 64, 67, 69, 72, 74, 76, 79,
    43, 45, 48, 50, 52, 55, 57, 60,    62, 64, 67, 69, 72, 74, 76, 79,
    43, 45, 48, 50, 52, 55, 57, 60,    62, 64, 67, 69, 72, 74, 76, 79};

uint8_t KEYNUM2MIDI_bytune(uint8_t _key_num){
    int a = _key_num / 8;
    int b = _key_num % 8;
    int oct_additive = -24 + (key_oct*12);
    switch(scale_select){
        case 0:
            return key2midi_normal[(a*16)+b+key_offset] + oct_additive + key_select;
        case 1:
            return key2midi_minBlues[(a*16)+b+key_offset] + oct_additive + key_select;
        default:
            return key2midi_normal[(a*16)+b+key_offset] + oct_additive + key_select;
    }
}

//AD-input用
int8_t MUX=5;


//--------------------------------------------------------------------
//---------------- Handle Notes---------------------------------------
void handleMIDINOTE(uint8_t status,uint8_t _key_num,uint8_t vel) {
    uint8_t i;
    uint32_t freq;
    if ((!vel)&&(status==0x90)) status=0x80;
    if (status==0x80) {
        for (i=0; i<4; i++) {
            if (OSCNOTES[i]==_key_num) {
                if (!GATED) {
                    FREQ[i<<1]=0;
                    FREQ[(i<<1)|1]=0;
                }
                OSCNOTES[i]=0;
            }
        }
        if (!(OSCNOTES[0]|OSCNOTES[1]|OSCNOTES[2]|OSCNOTES[3])) TRIG=0;
        return;
    }
    if (status==0x90) {
        if ((!TRIG)&&(GATED)) {
            for (i=0; i<8; i++) {
                FREQ[i]=0;
            }
        }
        i=0;
        while (i<4) {
            if (!OSCNOTES[i]) {
                freq=MIDI2FREQ(KEYNUM2MIDI_bytune(_key_num));
                FREQ[i<<1]=freq;
                FREQ[(i<<1)|1]=FREQ[i<<1]+(((FREQ[i<<1]/50)>>0)*DETUNE/127);
                OSCNOTES[i]=_key_num;
                if (!TRIG) {
                    TRIG=1;
                }
                return;
            }
            i++;
        }
    }
}




void setup() {
    //PWM outputs
    pinMode(11, OUTPUT);
    Serial.begin(115200);

    //Keyscanner inputs
    pinMode(2,INPUT_PULLUP);
    pinMode(3,INPUT_PULLUP);
    pinMode(4,INPUT_PULLUP);
    pinMode(5,INPUT_PULLUP);
    pinMode(6,INPUT_PULLUP);
    pinMode(7,INPUT_PULLUP);
    pinMode(8,INPUT_PULLUP);
    pinMode(9,INPUT_PULLUP);
    //Keyscanner outputs
    pinMode(14, OUTPUT);
    pinMode(15, OUTPUT);
    pinMode(16, OUTPUT);
    pinMode(17, OUTPUT);
    pinMode(18, OUTPUT);

    //Init some parameters ------------------------------------------------------------------------
    for(int i=0;i<PROCESS_NUM;i++){
        trigger_flag[i] = false;
    }
    for(int i=0;i<KEY_NUM;i++){
        key_pressingtime[i] = 0;
    }

    cli();
    
    // Timer1 calc signal
    TCCR1B = (TCCR1B & ~_BV(WGM13)) | _BV(WGM12);   // Set CTC mode
    TCCR1A = TCCR1A & ~(_BV(WGM11) | _BV(WGM10));
    TCCR1B = (TCCR1B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);   // No prescaler
    OCR1A = 758;    //F_CPU / SAMPLE_RATE;          // Set the compare register (OCR1A).
    TIMSK1 |= _BV(OCIE1A);                          // Enable interrupt when TCNT1 == OCR1A

    //Timer0 interrupt at xxHz ENV and Sequence
    TCCR0A = 0;     // set entire TCCR0A register to 0
    TCCR0B = 0;     // same for TCCR0B
    TCNT0 = 0;      //initialize counter value to 0
    OCR0A = 31;    // = xxHz
    TCCR0A |= (1 << WGM01);         // turn on CTC mode
    TCCR0B |= (1 << CS02) | (0 << CS01) | (0 << CS00); //256 prescaler
    TIMSK0 |= (1 << OCIE0A);        // enable timer compare interrupt

    sei();

    // Set baud rate to 31,250. Requires modification if clock speed is not 16MHz.
    UBRR0H = ((F_CPU / 16 + 31250 / 2) / 31250 - 1) >> 8;
    UBRR0L = ((F_CPU / 16 + 31250 / 2) / 31250 - 1);
    // Set frame format to 8 data bits, no parity, 1 stop bit
    UCSR0C |= (1<<UCSZ01)|(1<<UCSZ00);
    // enable rx
    UCSR0B |= _BV(RXEN0);
    // USART RX interrupt enable bit on
    UCSR0B |= _BV(RXCIE0);

    // Timer2 PWM output
    ASSR &= ~(_BV(EXCLK) | _BV(AS2));   // Use internal clock (datasheet p.160)
    TCCR2A |= _BV(WGM21) | _BV(WGM20);  // Set fast PWM mode (p.157)
    TCCR2B &= ~_BV(WGM22);
    TCCR2A = (TCCR2A | _BV(COM2A1)) & ~_BV(COM2A0);     // Do non-inverting PWM on pin OC2A(pin11) (p.155)
    TCCR2A &= ~(_BV(COM2B1) | _BV(COM2B0));
    TCCR2B = (TCCR2B & ~(_BV(CS12) | _BV(CS11))) | _BV(CS10);   // No prescaler (p.158)
    OCR2A = 128;        // Set initial pulse width to the first sample.

    
    // set up the ADC
//    BENDoffset=analogRead(7);
//    BENDoffset=0;
    ADCSRA &= ~PS_128; // remove bits set by Arduino library
    // you can choose a prescaler from above.
    // PS_16, PS_32, PS_64 or PS_128
    ADCSRA |= PS_128; // set our own prescaler to 16
    ADMUX = 69;
    sbi(ADCSRA, ADSC);
}

ISR(TIMER1_COMPA_vect) {
        //-------------------- 8 DCO block ------------------------------------------
        DCO=0;
        for (uint8_t i=0; i<8; i++) {
                if (integrators[i]) integrators[i]--; //Decrement integrators
                DCOPH[i] += FREQ[i]; //Add freq to phaseacc's
                if (DCOPH[i]&0x800000) { //Check for integrator reset
                        DCOPH[i]&=0x7FFFFF; //Trim NCO
                        integrators[i]=28; //Reset integrator
                }
                DCO+=integrators[i];
        }
        writepointer++;
        delayline[writepointer]=DCO;
        DCO+=(delayline[(writepointer-lfoval2)&255]*PHASERMIX)>>8;
        //-----------------------------------------------------------------
        //------------------ VCA block ------------------------------------
 #define M(MX, MX1, MX2) \
        asm volatile ( \
                "clr r26 \n\t" \
                "mulsu %B1, %A2 \n\t" \
                "movw %A0, r0 \n\t" \
                "mul %A1, %A2 \n\t" \
                "add %A0, r1 \n\t" \
                "adc %B0, r26 \n\t" \
                "clr r1 \n\t" \
                : \
                "=&r" (MX) \
                : \
                "a" (MX1), \
                "a" (MX2) \
                : \
                "r26" \
                )
        if ((ATTACK==volume_max)&&(TRIG==1)) VCA=volume_max;
        if (!(envcnt--)) {
                envcnt=20;
                if (VCA<volume) VCA++;
                if (VCA>volume) VCA--;
        }
        M(ENV, (int16_t)DCO, VCA);
        OCR2A = ENV;
        //-----------------------------------------------------------------
        //-------------- Calc Sample freq ---------------------------------
        OCR1A = 758-lfoval;
        //-----------------------------------------------------------------
}

//サブ処理用のタイマー
ISR(TIMER0_COMPA_vect) {

//1. -------------------------------------------- 音源のLFO制御 -------------------------------------
    timer_count++;
    if(timer_count%32 == 0){

        //------------------------------ LFO Block -----------------------
//        lfocounter+=LFO;
//        lfoval=(pgm_read_byte_near( sinetable + lfocounter ) * MOD)>>10; //LFO for pitch
        lfoval2=pgm_read_byte_near( sinetable + (lfocounter2++) ); //LFO for the Phaser
        //-----------------------------------------------------------------
        //--------------------- ENV block ---------------------------------
        if ((TRIG==1)&&(volume<volume_max)) {
                volume+=ATTACK;
                if (volume>volume_max) volume=volume_max;
        }
        if ((TRIG==0)&&(volume>0)) {
                volume-=RELEASE;
                if (volume<0) volume=0;
        }

//2. -------------------------------------------- キーの長押し判定 -------------------------------------
        for(int i=0;i<KEY_NUM;i++){
            if(!(key_status[i]&0b00000001)) continue;
            key_pressingtime[i]++;
            if(key_pressingtime[i] > LONGPRESS_WAITTIME && !(key_status[i]&0b00001000)){
                key_status[i]|=0b00001000;
                key_longpressed++;
                if(key_longpressed >= 6){     //6key以上同時押しされたら、
                    //発音を止める
                    for(int j=0;j<KEY_NUM;j++){
                        if(!(key_status[j]&0b00000001)) continue;
                        handleMIDINOTE(0x80, j, 0);
                    }
                    isSettingMode = !isSettingMode;     //設定モード反転
                    if(isSettingMode){
                        key_oct = 2;
                        DETUNE = 0;
                        PHASERMIX = 0;
                        ATTACK=0xFF;
                        RELEASE=32;
                    }
                }
            }
        }
    }
}

void keyPressHandler(uint8_t _keyscan_num){

//共通処理
    //キーステータス更新
    key_status[_keyscan_num] |= 0b00000001;     //Pressed  Flag ON
    key_status[_keyscan_num] &= 0b11111101;     //Released Flag OFF
    if(!(key_status[_keyscan_num]&0b00000100)) key_status[_keyscan_num] |= 0b00000100;    //Latch ON
    else key_status[_keyscan_num] &= 0b11111011;     //Latch OFF

}

void keyReleaseHandler(uint8_t _keyscan_num){
//共通の処理

    //キーステータス更新
    key_status[_keyscan_num] &= 0b11111110;     //Pressed  Flag OFF
    key_status[_keyscan_num] |= 0b00000010;     //Released Flag ON
    if(key_status[_keyscan_num]&0b00001000){
        key_status[_keyscan_num] &= 0b11110111;     //Long Pressed Flag OFF
        key_longpressed--;
    }
    key_pressingtime[_keyscan_num] = 0;         //initialize key_pressingtime

}



void loop() {
    //------------------ Key scanner -----------------------------
    PORTC|=0x1F;
    if ((keyscan_num&0x38)==(0x00<<3)) PORTC&=B11111110;
    if ((keyscan_num&0x38)==(0x01<<3)) PORTC&=B11111101;
    if ((keyscan_num&0x38)==(0x02<<3)) PORTC&=B11111011;
    if ((keyscan_num&0x38)==(0x03<<3)) PORTC&=B11110111;
    if ((keyscan_num&0x38)==(0x04<<3)) PORTC&=B11101111;
    keytable[keyscan_num]=digitalReadFast((keyscan_num&7)+2);
    if (oldkeytable[keyscan_num]!=keytable[keyscan_num]) { //Handle keyevent
        oldkeytable[keyscan_num]=keytable[keyscan_num];

        if (keytable[keyscan_num]==0){
            handleMIDINOTE(0x90, keyscan_num, 127);
            keyPressHandler(keyscan_num);

        }
        else{
            handleMIDINOTE(0x80, keyscan_num, 0);
            keyReleaseHandler(keyscan_num);
        }
    }
    keyscan_num++;
    if(keyscan_num==KEY_NUM) keyscan_num=0;

    //------------------ Knob read -----------------------------
    while (bit_is_set(ADCSRA, ADSC)) ; //Wait for ADC EOC

    if(!isSettingMode){
        if (MUX==7) DETUNE=((ADCL+(ADCH<<8))>>3);
        if (MUX==7) PHASERMIX=((ADCL+(ADCH<<8))>>2);
        //    if (MUX==7) MOD=((ADCL+(ADCH<<8))>>2);
        //    if (MUX==6) PHASERMIX=((ADCL+(ADCH<<8))>>2);
        if (MUX==6) key_oct=((ADCL+(ADCH<<8))>>8);
        if (MUX==5) ENVELOPE=((ADCL+(ADCH<<8))>>5);
        if (MUX==5) ATTACK=ATTrates[ENVELOPE];
        if (MUX==5) RELEASE=RELrates[ENVELOPE];
        if (RELEASE==255) GATED=0;
        if (RELEASE!=255) GATED=1;
        if (DETUNE!=olddetune) {
            olddetune=DETUNE;
            for (uint8_t i=0; i<4; i++) {
                if (FREQ[i<<1]) FREQ[(i<<1)|1]=FREQ[i<<1]+(((FREQ[i<<1]/50)>>0)*DETUNE/127);
            }
        }
    }else{
        if (MUX==5) key_offset=((ADCL+(ADCH<<8))>>7);   //volume setting
        if (MUX==6) key_select=((ADCL+(ADCH<<8))>>6);     //key select
        if (MUX==7) scale_select=((ADCL+(ADCH<<8))>>9);     //scale select
    }
    MUX++;
    if (MUX>7) MUX=5;
    ADMUX = 64 | MUX; //Select MUX
    sbi(ADCSRA, ADSC); //start next conversation
    //--------------------------------------------------------------------
}


