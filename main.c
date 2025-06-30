#include "NXP/iolpc2138.h"
#include <stdint.h>

// LCD Pin Definitions (8-bit mode)
#define LCD_DATA_MASK   0xFF       // P0.0 to P0.7 for LCD data
#define LCD_RS          (1 << 10)  // P0.10
#define LCD_EN          (1 << 11)  // P0.11

// Motor control
#define MOTOR_PIN1      (1 << 8)   // P0.8
#define MOTOR_PIN2      (1 << 9)   // P0.9
#define MOTOR_ENABLE    (1 << 21)  // P0.21

// LEDs
#define RED_LED         (1 << 12)  // P0.12 for wrong password
#define GREEN_LED       (1 << 13)  // P0.13 for access granted

// Buzzer
#define BUZZER_PIN      (1 << 31)  // P0.31

// Keypad connections
#define KEYPAD_ROW1     (1 << 16)  // P1.16
#define KEYPAD_ROW2     (1 << 17)  // P1.17
#define KEYPAD_ROW3     (1 << 18)  // P1.18
#define KEYPAD_ROW4     (1 << 19)  // P1.19
#define KEYPAD_COL1     (1 << 20)  // P1.20
#define KEYPAD_COL2     (1 << 21)  // P1.21
#define KEYPAD_COL3     (1 << 22)  // P1.22

// Password configuration
#define PASSWORD_LENGTH 4
#define MAX_WRONG_ATTEMPTS 3
const char correctPassword[PASSWORD_LENGTH] = {'1', '2', '3', '4'};
char enteredPassword[PASSWORD_LENGTH];
uint8_t passwordIndex = 0;
uint8_t wrongAttempts = 0;

// Door state
enum DoorState { DOOR_CLOSED, DOOR_OPEN };
enum DoorState currentDoorState = DOOR_CLOSED;

// Alarm state
volatile uint8_t alarmActive = 0;

void delay_ms(uint32_t ms);
void LCD_Init(void);
void LCD_Command(uint8_t cmd);
void LCD_Data(uint8_t data);
void LCD_String(const char *str);
void LCD_Clear(void);
void LCD_SetCursor(uint8_t row, uint8_t column);
char Keypad_GetKey(void);
char Keypad_Scan(void);
void Motor_Clockwise(void);
void Motor_AntiClockwise(void);
void Motor_Stop(void);
void Buzzer_Beep(uint32_t duration_ms, uint32_t frequency_hz);
void Alert_System(uint8_t mode);
void System_Init(void);
void ProcessPassword(void);
void DisplayWelcomeMessage(void);
void DisplayEnterPassword(void);
void DisplayAccessGranted(void);
void DisplayAccessDenied(void);
void DisplayTooManyAttempts(void);

int main(void) {
    System_Init();
    DisplayWelcomeMessage();

    while (1) {
        if (wrongAttempts >= MAX_WRONG_ATTEMPTS) {
            DisplayTooManyAttempts();
            Alert_System(3); 
            wrongAttempts = 0; 
            Buzzer_Beep(500, 500);
            delay_ms(1000); 
            IO0CLR = RED_LED | BUZZER_PIN; 
        }
        DisplayEnterPassword();
        ProcessPassword();
        if (alarmActive) {
            Alert_System(0); 
        }
    }
}

void System_Init(void) {
    // Initialize LCD pins
    IO0DIR |= LCD_DATA_MASK | LCD_RS | LCD_EN;
    
    // Initialize motor pins
    IO0DIR |= MOTOR_PIN1 | MOTOR_PIN2 | MOTOR_ENABLE;
    Motor_Stop();
    
    // Initialize LEDs
    IO0DIR |= RED_LED | GREEN_LED;
    IO0CLR = RED_LED | GREEN_LED; 
    
    // Initialize buzzer
    IO0DIR |= BUZZER_PIN;
    
    // Initialize keypad
    IO1DIR |= KEYPAD_ROW1 | KEYPAD_ROW2 | KEYPAD_ROW3 | KEYPAD_ROW4;
    IO1DIR &= ~(KEYPAD_COL1 | KEYPAD_COL2 | KEYPAD_COL3);
    
    LCD_Init();
}

void Alert_System(uint8_t mode) {
    switch (mode) {
        case 0: 
            if (!alarmActive) {
                alarmActive = 1;
                IO0SET = RED_LED; 
                Buzzer_Beep(200, 500);
                IO0CLR = RED_LED; 
                alarmActive = 0; 
            }
            break;
            
        case 1: // Correct password feedback
            IO0SET = GREEN_LED; 
            Buzzer_Beep(500, 1000); 
            delay_ms(200); 
            IO0CLR = GREEN_LED; 
            alarmActive = 0; 
            break;
            
        case 2: // Keypress feedback
            Buzzer_Beep(500, 5000); 
            break;
            
        case 3: // Lockout alarm (red LED and buzzer)
            IO0SET = RED_LED | BUZZER_PIN; 
            break;
            
        default:
            break;
    }
}

void LCD_Init(void) {
    delay_ms(20); // Reduced power-on delay
    
    // 8-bit initialization
    LCD_Command(0x38); 
    LCD_Command(0x0C); 
    LCD_Command(0x06); 
    LCD_Command(0x01); 
    delay_ms(2);
}

void LCD_Command(uint8_t cmd) {
    IO0CLR = LCD_RS;    
    IO0CLR = LCD_DATA_MASK;
    IO0SET = cmd;      
    
    IO0SET = LCD_EN;   
    delay_ms(1);
    IO0CLR = LCD_EN;
    delay_ms(1);
}

void LCD_Data(uint8_t data) {
    IO0SET = LCD_RS;    
    IO0CLR = LCD_DATA_MASK;
    IO0SET = data;      
    
    IO0SET = LCD_EN;    
    delay_ms(1);
    IO0CLR = LCD_EN;
    delay_ms(1);
}

void LCD_String(const char *str) {
    while (*str) {
        LCD_Data(*str++);
    }
}

void LCD_Clear(void) {
    LCD_Command(0x01);
    delay_ms(2);
}

void LCD_SetCursor(uint8_t row, uint8_t column) {
    uint8_t addr = (row == 0) ? 0x80 + column : 0xC0 + column;
    LCD_Command(addr);
}

char Keypad_GetKey(void) {
    char key = 'a';
    while (key == 'a') {
        key = Keypad_Scan();
    }
    return key;
}

char Keypad_Scan(void) {
    // Set all columns high (inactive)
    IO1SET = KEYPAD_COL1 | KEYPAD_COL2 | KEYPAD_COL3;
    
    // Scan each row
    for (uint8_t row = 0; row < 4; row++) {
        // Activate current row (set low)
        IO1CLR = KEYPAD_ROW1 | KEYPAD_ROW2 | KEYPAD_ROW3 | KEYPAD_ROW4;
        IO1SET = (KEYPAD_ROW1 << row) ^ (KEYPAD_ROW1 | KEYPAD_ROW2 | KEYPAD_ROW3 | KEYPAD_ROW4);
        delay_ms(1);
        
        // Check columns
        if (!(IO1PIN & KEYPAD_COL1)) {
            LCD_Data('*');
            delay_ms(10);
            return (row == 0) ? '1' : 
                   (row == 1) ? '4' : 
                   (row == 2) ? '7' : '*';
        }
        if (!(IO1PIN & KEYPAD_COL2)) {
            LCD_Data('*');
            delay_ms(10);
            return (row == 0) ? '2' : 
                   (row == 1) ? '5' : 
                   (row == 2) ? '8' : '0';
        }
        if (!(IO1PIN & KEYPAD_COL3)) {
            LCD_Data('*');
            delay_ms(10);
            return (row == 0) ? '3' : 
                   (row == 1) ? '6' : 
                   (row == 2) ? '9' : '#';
        }
    }
    return 'a';
}

void Motor_Clockwise(void) {
    IO0SET = MOTOR_PIN1;
    IO0CLR = MOTOR_PIN2;
    IO0SET = MOTOR_ENABLE;
}

void Motor_AntiClockwise(void) {
    IO0CLR = MOTOR_PIN1;
    IO0SET = MOTOR_PIN2;
    IO0SET = MOTOR_ENABLE;
}

void Motor_Stop(void) {
    IO0CLR = MOTOR_ENABLE;
    IO0CLR = MOTOR_PIN1 | MOTOR_PIN2;
}

void Buzzer_Beep(uint32_t duration_ms, uint32_t frequency_hz) {
    uint32_t half_period_ms = 1000 / (2 * frequency_hz); 
    uint32_t cycles = duration_ms / (2 * half_period_ms);
    for (uint32_t i = 0; i < cycles; i++) {
        IO0SET = BUZZER_PIN;
        delay_ms(half_period_ms);
        IO0CLR = BUZZER_PIN;
        delay_ms(half_period_ms);
    }
}

void ProcessPassword(void) {
    passwordIndex = 0;
    char key;
    
    while (passwordIndex < PASSWORD_LENGTH) {
        key = Keypad_GetKey();
        if (key == '#') {
            // Clear password and reset
            passwordIndex = 0;
            for (uint8_t i = 0; i < PASSWORD_LENGTH; i++) {
                enteredPassword[i] = 0;
            }
            DisplayEnterPassword();
            continue;
        }
        if (key != 0 && key != 'a') {
            enteredPassword[passwordIndex++] = key;
            Alert_System(2); 
            delay_ms(100); 
        }
    }
    
    // Check password
    int correct = 1;
    for (int i = 0; i < PASSWORD_LENGTH; i++) {
        if (enteredPassword[i] != correctPassword[i]) {
            correct = 0;
            break;
        }
    }
    
    if (correct) {
        wrongAttempts = 0; 
        DisplayAccessGranted();
        Alert_System(1); 
        
        if (currentDoorState == DOOR_CLOSED) {
            Motor_AntiClockwise();
            delay_ms(300);
            Motor_Stop();
            currentDoorState = DOOR_OPEN;
        } else {
            Motor_Clockwise();
            delay_ms(300);
            Motor_Stop();
            currentDoorState = DOOR_CLOSED;
        }
    } else {
        wrongAttempts++;
        DisplayAccessDenied();
        Alert_System(0); 
    }
    delay_ms(100); 
}

void DisplayWelcomeMessage(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String(" SMART DOOR LOCK");
    LCD_SetCursor(1, 0);
    LCD_String("  SYSTEM READY  ");
    delay_ms(100); 
}

void DisplayEnterPassword(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("ENTER PASSWORD:");
    LCD_SetCursor(1, 0);
}

void DisplayAccessGranted(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String(" ACCESS GRANTED ");
    LCD_SetCursor(1, 0);
    LCD_String(currentDoorState == DOOR_CLOSED ? "  DOOR OPENING  " : "  DOOR CLOSING  ");
}

void DisplayAccessDenied(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String(" ACCESS DENIED ");
    LCD_SetCursor(1, 0);
    LCD_String("WRONG PASSWORD");
}

void DisplayTooManyAttempts(void) {
    LCD_Clear();
    LCD_SetCursor(0, 0);
    LCD_String("TOO MANY TRIES ");
    LCD_SetCursor(1, 0);
    LCD_String("PLEASE WAIT ...");
    
}

void delay_ms(uint32_t ms) {
    for(uint32_t i = 0; i < ms; i++) {
        for(uint32_t j = 0; j < 6000; j++); 
    }
}