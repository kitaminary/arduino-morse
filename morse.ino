const int BTN_PIN  = 2;
const int BUZZ_PIN = 8;
const int FREQ     = 3000;
const int DOT_MAX  = 250;
const int CHAR_GAP = 480;
const int WORD_GAP = 1500;

struct MorseEntry { char letter; const char* code; };

const MorseEntry MORSE[] = {
  { 'A', ".-"   }, { 'B', "-..." }, 
  { 'C', "-.-." }, { 'D', "-.."  }, 
  { 'E', "."    }, { 'F', "..-." }, 
  { 'G', "--."  }, { 'H', "...." },
  { 'I', ".."   }, { 'J', ".---" },
  { 'K', "-.-"  }, { 'L', ".-.." },
  { 'M', "--"   }, { 'N', "-."   }, 
  { 'O', "---"  }, { 'P', ".--." },
  { 'Q', "--.-" }, { 'R', ".-."  },
  { 'S', "..."  }, { 'T', "-"    },
  { 'U', "..-"  }, { 'V', "...-" }, 
  { 'W', ".--"  }, { 'X', "-..-" },
  { 'Y', "-.--" }, { 'Z', "--.." }
};

const MorseEntry DIGITS[] = {
  { '0', "-----" }, { '1', ".----" }, 
  { '2', "..---" }, { '3', "...--" }, 
  { '4', "....-" }, { '5', "....." },
  { '6', "-...." }, { '7', "--..." },
  { '8', "---.." }, { '9', "----." }
};

String code = "";
String currentWord = "";
String fullText = "";
unsigned long lastRelease = 0;
unsigned long lastDebounce = 0;
bool wasDown = false;
bool btnState = false;
unsigned long pressStart = 0;
bool charCommitted = false;

void setup() {
  pinMode(BTN_PIN, INPUT_PULLUP);
  pinMode(BUZZ_PIN, OUTPUT);
  Serial.begin(9600);
  Serial.println("Готово! Пауза 0.48с = буква, 1.5с = слово");
}

char decode(String c) {
  for (auto& e : MORSE)
    if (c == e.code) return e.letter;
  for (auto& e : DIGITS)
    if (c == e.code) return e.letter;
  return '?';
}

void commitChar() {
  if (code.length() == 0) return;
  char c = decode(code);
  currentWord += c;
  Serial.print(c);
  code = "";
  charCommitted = true;
}

void commitWord() {
  if (currentWord.length() == 0) return;
  fullText += currentWord + " ";
  Serial.println();
  Serial.print("Слово: "); Serial.println(currentWord);
  Serial.print("Текст: "); Serial.println(fullText);
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
      charCommitted = false;
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
    }
  }

  unsigned long silence = now - lastRelease;

  if (!wasDown && code.length() > 0 && silence > CHAR_GAP)
    commitChar();

  if (!wasDown && currentWord.length() > 0 && code.length() == 0 && silence > WORD_GAP)
    commitWord();
}