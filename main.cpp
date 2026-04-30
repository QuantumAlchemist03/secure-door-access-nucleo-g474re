#include "mbed.h"

DigitalOut led(PA_5);
DigitalIn  button(PC_13, PullUp);
BufferedSerial pc(USBTX, USBRX, 9600);
PwmOut buzzer(PC_7);

DigitalOut rows[4] = {
    DigitalOut(PB_0),
    DigitalOut(PA_4),
    DigitalOut(PA_1),
    DigitalOut(PA_0)
};

DigitalIn cols[4] = {
    DigitalIn(PC_1, PullDown),
    DigitalIn(PC_0, PullDown),
    DigitalIn(PB_5, PullDown),
    DigitalIn(PA_6, PullDown)
};

const char keyMap[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};

bool doorOpen      = false;
bool authenticated = false;
int  failCount     = 0;
bool systemLocked  = false;
int  lastButton    = 1;

const char* correctPIN = "2580";
char enteredPIN[5];
int  pinIndex = 0;

void printMsg(const char* msg) {
    pc.write(msg, strlen(msg));
}

void beepSuccess() {
    buzzer.period(1.0f / 2000.0f);
    buzzer = 0.5f;
    ThisThread::sleep_for(300ms);
    buzzer = 0.0f;
}

void beepFail() {
    buzzer.period(1.0f / 400.0f);
    buzzer = 0.5f;
    ThisThread::sleep_for(600ms);
    buzzer = 0.0f;
}

void beepDoorbell() {
    buzzer.period(1.0f / 1500.0f);
    buzzer = 0.5f;
    ThisThread::sleep_for(150ms);
    buzzer = 0.0f;
    ThisThread::sleep_for(100ms);
    buzzer = 0.5f;
    ThisThread::sleep_for(150ms);
    buzzer = 0.0f;
}

void beepLockout() {
    for (int i = 0; i < 3; i++) {
        buzzer.period(1.0f / 300.0f);
        buzzer = 0.5f;
        ThisThread::sleep_for(400ms);
        buzzer = 0.0f;
        ThisThread::sleep_for(100ms);
    }
}

void printStatus() {
    printMsg("\r\n  +----------------------------------+\r\n");
    if (authenticated)
        printMsg("  | AUTH:  [**GRANTED**]             |\r\n");
    else
        printMsg("  | AUTH:  [--LOCKED-- ]             |\r\n");
    if (doorOpen)
        printMsg("  | DOOR:  [** OPEN  **]             |\r\n");
    else
        printMsg("  | DOOR:  [-- CLOSED--]             |\r\n");
    char buf[48];
    snprintf(buf, sizeof(buf),
        "  | ATTEMPTS: %d / 3                  |\r\n", failCount);
    printMsg(buf);
    printMsg("  | SYSTEM: NUCLEO-G474RE ONLINE     |\r\n");
    printMsg("  +----------------------------------+\r\n");
}

void bootSequence() {
    buzzer = 0.0f;
    printMsg("\r\n\r\n");
    printMsg("  initialising secure system...\r\n");
    ThisThread::sleep_for(400ms);
    printMsg("  loading encryption module...\r\n");
    ThisThread::sleep_for(400ms);
    printMsg("  establishing secure channel...\r\n");
    ThisThread::sleep_for(400ms);
    printMsg("  hardware check... OK\r\n");
    ThisThread::sleep_for(300ms);
    printMsg("  keypad... OK\r\n");
    ThisThread::sleep_for(300ms);
    printMsg("  buzzer... OK\r\n");
    ThisThread::sleep_for(300ms);
    printMsg("\r\n");
    printMsg("  +==================================+\r\n");
    printMsg("  |   SECURE DOOR ACCESS SYSTEM     |\r\n");
    printMsg("  |   NUCLEO-G474RE  |  Mbed OS 6   |\r\n");
    printMsg("  |          CLASSIFIED              |\r\n");
    printMsg("  +==================================+\r\n");
    printMsg("\r\n");
    printMsg("  AUTHENTICATION REQUIRED\r\n");
    printMsg("\r\n");
}

void showMenu() {
    printMsg("\r\n  +==================================+\r\n");
    printMsg("  |  [O] Open Door                  |\r\n");
    printMsg("  |  [C] Close Door                 |\r\n");
    printMsg("  |  [S] Door Status                |\r\n");
    printMsg("  |  [L] Logout                     |\r\n");
    printMsg("  |  [#] Logout via keypad          |\r\n");
    printMsg("  +==================================+\r\n");
    printStatus();
    printMsg("\r\n  Enter command: ");
}

void promptPIN() {
    printMsg("\r\n  Enter 4-digit PIN on keypad: ");
    pinIndex = 0;
    memset(enteredPIN, 0, sizeof(enteredPIN));
}

char scanKeypad() {
    for (int r = 0; r < 4; r++) {
        for (int i = 0; i < 4; i++) rows[i] = 0;
        rows[r] = 1;
        ThisThread::sleep_for(5ms);
        for (int c = 0; c < 4; c++) {
            if (cols[c].read() == 1) {
                while (cols[c].read() == 1) ThisThread::sleep_for(10ms);
                return keyMap[r][c];
            }
        }
    }
    return '\0';
}

void handlePINDigit(char digit) {
    if (digit >= '0' && digit <= '9' && pinIndex < 4) {
        enteredPIN[pinIndex] = digit;
        pinIndex++;
        printMsg("*");
    }
    if (pinIndex == 4) {
        enteredPIN[4] = '\0';
        ThisThread::sleep_for(200ms);
        if (strcmp(enteredPIN, correctPIN) == 0) {
            failCount = 0;
            authenticated = true;
            beepSuccess();
            printMsg("\r\n\r\n");
            printMsg("  +==================================+\r\n");
            printMsg("  |   ACCESS GRANTED -- Welcome!    |\r\n");
            printMsg("  +==================================+\r\n");
            led = 0;
            showMenu();
        } else {
            failCount++;
            beepFail();
            if (failCount >= 3) {
                systemLocked = true;
                beepLockout();
                printMsg("\r\n\r\n");
                printMsg("  +==================================+\r\n");
                printMsg("  |  !! SYSTEM LOCKOUT !!           |\r\n");
                printMsg("  |  Too many failed attempts.      |\r\n");
                printMsg("  |  Reset board to try again.      |\r\n");
                printMsg("  +==================================+\r\n");
                led = 0;
                return;
            }
            char buf[64];
            snprintf(buf, sizeof(buf),
                "\r\n  Wrong PIN! %d attempt(s) remaining.\r\n",
                3 - failCount);
            printMsg(buf);
            promptPIN();
        }
    }
}

int main() {
    led = 0;
    buzzer = 0.0f;
    ThisThread::sleep_for(500ms);
    bootSequence();
    promptPIN();

    while (1) {
        if (systemLocked) {
            ThisThread::sleep_for(1s);
            continue;
        }

        char key = scanKeypad();
        if (key != '\0') {
            if (!authenticated) {
                handlePINDigit(key);
            } else {
                if (key == '#') {
                    authenticated = false;
                    led = 0;
                    doorOpen = false;
                    beepFail();
                    printMsg("\r\n  Logged out. Door locked.\r\n");
                    promptPIN();
                } else if (key == '*') {
                    printStatus();
                    showMenu();
                }
            }
        }

        if (pc.readable()) {
            char rxChar;
            pc.read(&rxChar, 1);
            if (!authenticated) {
                if (rxChar >= '0' && rxChar <= '9') {
                    handlePINDigit(rxChar);
                }
            } else {
                switch (rxChar) {
                    case 'O': case 'o':
                        led = 1;
                        doorOpen = true;
                        beepSuccess();
                        printMsg("\r\n  Door OPENED -- LED ON\r\n");
                        showMenu();
                        break;
                    case 'C': case 'c':
                        led = 0;
                        doorOpen = false;
                        beepFail();
                        printMsg("\r\n  Door CLOSED -- LED OFF\r\n");
                        showMenu();
                        break;
                    case 'S': case 's':
                        printStatus();
                        showMenu();
                        break;
                    case 'L': case 'l':
                        authenticated = false;
                        led = 0;
                        doorOpen = false;
                        beepFail();
                        printMsg("\r\n  Logged out. Door locked.\r\n");
                        promptPIN();
                        break;
                    default:
                        printMsg("\r\n  Invalid command!\r\n");
                        showMenu();
                        break;
                }
            }
        }

        int currentButton = button.read();
        if (currentButton == 0 && lastButton == 1) {
            beepDoorbell();
            printMsg("\r\n  ** DOORBELL: Visitor at the door! **\r\n");
            if (authenticated) showMenu();
            else promptPIN();
            ThisThread::sleep_for(200ms);
        }
        lastButton = currentButton;
        ThisThread::sleep_for(10ms);
    }
}