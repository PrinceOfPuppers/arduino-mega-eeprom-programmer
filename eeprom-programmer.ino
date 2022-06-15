#define DATA_START_PIN 22
#define DATA_NUM_PINS 8
#define ADDR_START_PIN 38
#define ADDR_NUM_PINS 15

#define WRITE_PIN 2
#define OUTPUT_ENABLE_PIN 3

static uint8_t eeprom_write_data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
static int eeprom_data_len = sizeof(eeprom_write_data) / sizeof(eeprom_write_data[0]);


////////////////////////////
// pin parallel interface //
////////////////////////////

void write_val_pins(int val, int start_pin, int num_pins){
    int bit = 0;
    for (int i = 0; i < num_pins; i++){
        bit = (val & 1<<i)!=0;
        digitalWrite( start_pin + i, bit );
    }
}

uint8_t read_byte_pins(int start_pin){
    uint8_t val = 0;

    for (int i = 0; i < 8; i++){
        val |= ( digitalRead( start_pin + i ) << i );
    }
    return val;
}




//////////////////
// eeprom setup //
//////////////////

void setup_eeprom_writer(){
    delay(100);
    for (int i = 0; i < ADDR_NUM_PINS; i++){
        digitalWrite(ADDR_START_PIN + i, LOW);
        pinMode(ADDR_START_PIN + i, OUTPUT);
    }

    for (int i = 0; i < DATA_NUM_PINS; i++){
        digitalWrite(DATA_START_PIN + i, LOW);
        pinMode(DATA_START_PIN + i, OUTPUT);
    }

    pinMode(WRITE_PIN, OUTPUT);
    digitalWrite(WRITE_PIN, HIGH); // active low

    pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
    digitalWrite(OUTPUT_ENABLE_PIN, HIGH); // active low

    delay(100);
}

void setup_eeprom_reader(){
    delay(100);
    for (int i = 0; i < ADDR_NUM_PINS; i++){
        digitalWrite(ADDR_START_PIN + i, LOW);
        pinMode(ADDR_START_PIN + i, OUTPUT);
    }

    for (int i = 0; i < DATA_NUM_PINS; i++){
        digitalWrite(DATA_START_PIN + i, LOW);
        pinMode(DATA_START_PIN + i, INPUT);
    }

    digitalWrite(WRITE_PIN, HIGH); // active low
    pinMode(WRITE_PIN, OUTPUT);

    digitalWrite(OUTPUT_ENABLE_PIN, LOW); // active low
    pinMode(OUTPUT_ENABLE_PIN, OUTPUT);
    delay(100);
}





////////////////////
// eeprom byte rw //
////////////////////

uint8_t eeprom_read_byte(int address){
    //delay(10);
    write_val_pins(address, ADDR_START_PIN, ADDR_NUM_PINS);
    uint8_t x = read_byte_pins(DATA_START_PIN);
    return x;
}

void eeprom_write_byte(int address, uint8_t byte){
    //delay(10);
    write_val_pins(address, ADDR_START_PIN, ADDR_NUM_PINS);
    write_val_pins(byte, DATA_START_PIN, DATA_NUM_PINS);
    digitalWrite(WRITE_PIN, LOW);
    digitalWrite(WRITE_PIN, HIGH);
}




///////////////////////
// eeprom block read //
///////////////////////

void eeprom_dump_to_buff(uint8_t *buff, int read_len, int offset){
    Serial.print("reading... \n");
    for (int i = 0; i < read_len; i++){
        buff[i] = eeprom_read_byte(i+offset);
    }
}

void eeprom_dump_to_serial(int read_len, int offset){
    char buff[5];
    for(int i = 0; i < read_len; i++){
        if (i%(2*16) == 0){
            Serial.print("\n");
            sprintf(buff, "%04x", i+offset);
            Serial.print(buff);
            Serial.print(":");
        }
        else if (i%16 == 0){
            Serial.print(" ");
        }

        Serial.print(" ");

        sprintf(buff, "%02x", eeprom_read_byte(i+offset));
        Serial.print(buff);
    }
    Serial.print("\n");
}




////////////////////////
// eeprom block write //
////////////////////////

void eeprom_write(uint8_t *data, int data_len){
    Serial.print("writing...\n");
    for (int i = 0; i < data_len; i++){
        eeprom_write_byte(i, eeprom_write_data[i]);
    }
}
int eeprom_verify_write(uint8_t *written_data, int data_len){
    Serial.print("verifying write...\n");
    setup_eeprom_reader();

    for(int i = 0; i < data_len; i++){
        if(written_data[i] != eeprom_read_byte(i)){
            return 1;
        }
    }
    return 0;
}




static int write_err = 0;

void setup() {
    Serial.begin(9600);

    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);

    setup_eeprom_writer();
    eeprom_write(eeprom_write_data, eeprom_data_len);
    write_err = eeprom_verify_write(eeprom_write_data, eeprom_data_len);

    if(write_err){
        Serial.print("write error! \ndumping written:\n");
        eeprom_dump_to_serial(eeprom_data_len, 0);
        return;
    }
    Serial.print("write successful!\n");

    eeprom_dump_to_serial(255, 0);
    digitalWrite(LED_BUILTIN, HIGH);
}

void loop() {
    if(write_err){
        digitalWrite(LED_BUILTIN, HIGH);
        delay(1000);
        digitalWrite(LED_BUILTIN, LOW);
    }
    delay(1000);
}
