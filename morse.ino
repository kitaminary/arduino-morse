#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);

const int BTN_PIN  = 2;
const int BUZZ_PIN = 3;
const int FREQ     = 3030;
const int DOT_MAX  = 200;
const int CHAR_GAP = 600;
const int WORD_GAP = 1500;

struct MorseEntry { char letter; const char* code; };

const MorseEntry MORSE[] = {
  { 'A', ".-"   }, { 'B', "-..." }, { 'C', "-.-." }, { 'D', "-.."  }, { 'E', "."    },
  { 'F', "..-." }, { 'G', "--."  }, { 'H', "...." }, { 'I', ".."   }, { 'J', ".---" },
  { 'K', "-.-"  }, { 'L', ".-.." }, { 'M', "--"   }, { 'N', "-."   }, { 'O', "---"  },
  { 'P', ".--." }, { 'Q', "--.-" }, { 'R', ".-."  }, { 'S', "..."  }, { 'T', "-"    },
  { 'U', "..-"  }, { 'V', "...-" }, { 'W', ".--"  }, { 'X', "-..-" }, { 'Y', "-.--" },
  { 'Z', "--.." }
};

const MorseEntry DIGITS[] = {
  { '0', "-----" }, { '1', ".----" }, { '2', "..---" }, { '3', "...--" }, { '4', "....-" },
  { '5', "....." }, { '6', "-...." }, { '7', "--..." }, { '8', "---.." }, { '9', "----." }
};

String code = "";
String currentWord = "";
String line1 = "";  // верхняя строка экрана — накопленный текст
unsigned long lastRelease = 0;
unsigned long lastDebounce = 0;
bool wasDown = false;
bool btnState = false;
unsigned long pressStart = 0;

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZ_PIN, OUTPUT);
  Serial.begin(9600);

  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Morse ready!");
  Serial.println("Готово!");
  delay(1000);
  lcd.clear();
}

char decode(String c) {
  for (auto& e : MORSE)
    if (c == e.code) return e.letter;
  for (auto& e : DIGITS)
    if (c == e.code) return e.letter;
  return '?';
}

void lcdPrintLine(int row, String text) {
  lcd.setCursor(0, row);
  lcd.print("                ");
  lcd.setCursor(0, row);
  lcd.print(text);
}

void commitChar() {
  if (code.length() == 0) return;
  char c = decode(code);
  currentWord += c;
  Serial.print(c);

  // строка 0 — точки/тире очищаем
  lcdPrintLine(0, "");
  // строка 1 — текущее слово
  lcdPrintLine(1, currentWord);

  code = "";
}

void commitWord() {
  if (currentWord.length() == 0) return;

  Serial.println();
  Serial.print("Слово: "); Serial.println(currentWord);

  // новое слово влезает в строку 1 — просто добавляем
  if (line1.length() + 1 + currentWord.length() <= 16) {
    if (line1.length() > 0) line1 += " ";
    line1 += currentWord;
  } else {
    // не влезает — сдвигаем: строка 1 становится строкой 0, новое слово в строку 1
    lcdPrintLine(0, line1);
    line1 = currentWord;
  }

  lcdPrintLine(0, "");       // очищаем верхнюю (точки/тире там уже нет)
  lcdPrintLine(1, line1);    // показываем накопленное

  // если строка 0 должна показывать предыдущую строку — перерисуем
  // находим что было на строке 0 до сдвига
  // проще: храним две строки
  Serial.print("Текст: "); Serial.println(line1);

  currentWord = "";
}

void loop() {
  bool reading = digitalRead(BTN_PIN) == LOW;
  unsigned long now = millis();

  if (reading != btnState) {
    lastDebounce = now;
    btnState = reading;
  }

  if ((now - lastDebounce) > 0) {
    bool down = btnState;

    if (down && !wasDown) {
      pressStart = now;
      wasDown = true;
      tone(BUZZ_PIN, FREQ);
    }

    if (!down && wasDown) {
      noTone(BUZZ_PIN);
      wasDown = false;
      lastRelease = now;
      long dur = now - pressStart;
      if (dur < DOT_MAX) {
        code += ".";
        Serial.print(".");
      } else {
        code += "-";
        Serial.print("-");
      }
      lcdPrintLine(0, code);
    }
  }

  unsigned long silence = now - lastRelease;

  if (!wasDown && code.length() > 0 && silence > CHAR_GAP)
    commitChar();

  if (!wasDown && currentWord.length() > 0 && code.length() == 0 && silence > WORD_GAP)
    commitWord();
}
