#include <Keypad.h>     // Para manejo del teclado
#include <Wire.h>        // Puerto I2C
#include <RTClib.h>      // Reloj tiempo real
#include <Adafruit_GFX.h> // Hardware-specific library
#include <MCUFRIEND_kbv.h>
#include <EEPROM.h>      // Librería para manejar EEPROM

RTC_DS3231 rtc;
MCUFRIEND_kbv tft;

#define LCD_CS A3   // Pines del LCD
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
#define LCD_RESET 1

#define BLACK     0x0000
#define BLUE      0x001F
#define BLUE2     0x051F
#define RED       0xF800
#define GREEN     0x07E0
#define CYAN      0x07FF
#define MAGENTA   0xF81F
#define YELLOW    0xFFE0
#define WHITE     0xFFFF
#define DARKGREY  0x7BEF
#define ORANGE    0xFD20

#define tiempo 3000

bool entradas[5] = {false, false, false, false, false};
char tiemposEntrada[5][20] = {"", "", "", "", ""};
char tiemposSalida[5][20] = {"", "", "", "", ""};

const byte FILAS = 4;
const byte COLUMNAS = 3;
char keys[FILAS][COLUMNAS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte pinesFilas[FILAS] = {22, 23, 24, 25};
byte pinesColumnas[COLUMNAS] = {26, 27, 28};

Keypad teclado = Keypad(makeKeymap(keys), pinesFilas, pinesColumnas, FILAS, COLUMNAS);

char TECLA;
char CLAVE[3];
char CLAVE_ENTRAR[5][3] = {"1*", "2*", "3*", "4*", "5*"};
char CLAVE_SALIR[5][3] = {"1#", "2#", "3#", "4#", "5#"};
char CLAVE_CALENDARIO[3] = "*#";  // Clave para acceder al calendario
char CLAVE_PRINCIPAL[3] = "#*";   // Clave para regresar a la pantalla principal

byte INDICE = 0;

int currentYear, currentMonth, currentDay;
bool shouldUpdate = true;
bool pantallaCalendario = false;

// Funciones para manejar la EEPROM
void guardarEstadoEEPROM() {
  for (int i = 0; i < 5; i++) {
    EEPROM.put(i * 10, entradas[i]);
    EEPROM.put(50 + i * 20, tiemposEntrada[i]);
    EEPROM.put(150 + i * 20, tiemposSalida[i]);
  }
}

void cargarEstadoEEPROM() {
  for (int i = 0; i < 5; i++) {
    EEPROM.get(i * 10, entradas[i]);
    EEPROM.get(50 + i * 20, tiemposEntrada[i]);
    EEPROM.get(150 + i * 20, tiemposSalida[i]);
  }
}

void setup() {
  tft.begin(0x9341);
  Serial.begin(9600);

  if (!rtc.begin()) {
    Serial.println("Modulo RTC no encontrado !");
    while (1);
  }

  tft.setRotation(3);
  tft.fillScreen(DARKGREY);

  cargarEstadoEEPROM();
  mostrarHabitaciones();

  DateTime now = rtc.now();
  currentYear = now.year();
  currentMonth = now.month();
  currentDay = now.day();
}

void mostrarHabitaciones() {
  tft.fillScreen(DARKGREY); // Limpia la pantalla completamente antes de redibujar

  for (int i = 0; i < 5; i++) {
    int y = 10 + (i * 42);
    tft.drawRoundRect(5, y, 48, 40, 5, BLUE);
    tft.drawRoundRect(55, y, 170, 40, 5, BLUE);
    tft.drawRoundRect(227, y, 90, 40, 5, BLUE);
    tft.fillRoundRect(6, y + 1, 46, 38, 5, BLACK);

    tft.setTextSize(2); // Restablece el tamaño de texto
    if (entradas[i]) {
      tft.fillRoundRect(56, y + 1, 168, 38, 5, BLACK);
      tft.fillRoundRect(228, y + 1, 88, 38, 5, YELLOW);
      tft.setCursor(231, y + 13);
      tft.setTextColor(RED);
      tft.print("OCUPADO");
      tft.setCursor(56, y + 5);
      tft.setTextColor(GREEN);
      tft.print(tiemposEntrada[i]);
    } else {
      tft.fillRoundRect(56, y + 1, 168, 38, 5, WHITE);
      tft.fillRoundRect(228, y + 1, 88, 38, 5, GREEN);
      tft.setCursor(243, y + 13);
      tft.setTextColor(BLUE);
      tft.print("LIBRE");
      tft.setCursor(56, y + 5);
      //tft.print(tiemposSalida[i]);
    }

    tft.setTextColor(WHITE);
    tft.setTextSize(4); // Restablece el tamaño de texto para los números de habitación
    tft.setCursor(20, y + 5);
    tft.print(i + 1);
  }
}

void actualizarHabitacion(int hab, const char* tipo, uint16_t colorTexto, int linea) {
  int y = 10 + (hab * 42);

  DateTime fecha = rtc.now();
  char buffer[20];
  sprintf(buffer, "%02d/%02d-%02d:%02d:%02d", fecha.day(), fecha.month(), fecha.hour(), fecha.minute(), fecha.second());

  int offsetY = (linea == 0) ? y + 5 : y + 22; // Primera línea para entrada, segunda para salida

  tft.setTextColor(colorTexto);
  tft.setTextSize(2);
  tft.setCursor(56, offsetY);

  if (strcmp(tipo, "Entrada") == 0) {
    strcpy(tiemposEntrada[hab], buffer);
    tft.fillRoundRect(56, y + 1, 168, 38, 5, BLACK);
    tft.print(buffer);
    tft.fillRoundRect(228, y + 1, 88, 38, 5, YELLOW);
    tft.setCursor(231, y + 13);
    tft.setTextColor(RED);
    tft.print("OCUPADO");
  } else if (strcmp(tipo, "Salida") == 0) {
    strcpy(tiemposSalida[hab], buffer);
    tft.print(buffer);
    delay(tiempo); // Espera 3 segundos
    tft.fillRoundRect(56, y + 1, 168, 38, 5, WHITE);
    tft.fillRoundRect(228, y + 1, 88, 38, 5, GREEN);
    tft.setCursor(243, y + 13);
    tft.setTextColor(BLUE);
    tft.print("LIBRE");
  }

  guardarEstadoEEPROM(); // Guardar cambios en la EEPROM

  Serial.print(tipo);
  Serial.print(" Hab ");
  Serial.print(hab + 1);
  Serial.print(": ");
  Serial.println(buffer);
}

void dibujarCalendario(int year, int month, int day) {
  tft.fillScreen(BLACK); // Limpia completamente la pantalla antes de dibujar el calendario

  // Fondo Blanco
  tft.drawRoundRect(15, 8, 290, 220, 8, BLUE);
  tft.fillRoundRect(16, 9, 288, 218, 8, WHITE);

  // Fondo de Título del mes
  tft.drawRoundRect(15, 8, 290, 30, 8, ORANGE);
  tft.fillRoundRect(16, 9, 288, 30, 8, BLACK);

  // Fondo de los días
  tft.drawRoundRect(15, 44, 290, 25, 8, ORANGE);
  tft.fillRoundRect(16, 45, 288, 25, 8, DARKGREY);

  // Color de los números
  tft.setTextColor(BLACK);
  tft.setTextSize(2);

  const char* meses[] = {
    "Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio",
    "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"
  };

  String mesNombre = meses[month - 1];
  tft.setCursor(30, 13);
  tft.setTextSize(3);
  tft.setTextColor(ORANGE);
  tft.println(mesNombre + "  " + String(year));

  const char* days[] = {"D", "L", "M", "M", "J", "V", "S"};
  tft.setTextSize(2);
  for (int i = 0; i < 7; i++) {
    tft.setTextColor(BLUE);
    tft.setCursor(30 + i * 40, 50);
    tft.print(days[i]);
  }

  for (int i = 0; i < 31; i++) {
    int x = 30 + (i % 7) * 40;
    int y = 80 + (i / 7) * 32;
    tft.setCursor(x, y);
    if (i + 1 == day) {
      tft.setTextColor(RED);
    } else {
      tft.setTextColor(BLACK);
    }
    tft.print(i + 1);
  }
}

void loop() {
//////////Detecta la tecla//////////

  TECLA = teclado.getKey();
  if (TECLA) {
    CLAVE[INDICE] = TECLA;
    INDICE++;
    Serial.print(TECLA);
  }


//////////Reconocimiento de las claves//////////

  if (INDICE == 2) {
    CLAVE[INDICE] = '\0';
    Serial.print("\nClave ingresada: ");
    Serial.println(CLAVE);

    if (strcmp(CLAVE, CLAVE_CALENDARIO) == 0) {
      pantallaCalendario = true;
      dibujarCalendario(currentYear, currentMonth, currentDay);
    } else if (strcmp(CLAVE, CLAVE_PRINCIPAL) == 0) {
      pantallaCalendario = false;
      mostrarHabitaciones();
    } else {
      for (int i = 0; i < 5; i++) {
        if (!strcmp(CLAVE, CLAVE_ENTRAR[i])) {
          entradas[i] = true;
          if (!pantallaCalendario) {
            actualizarHabitacion(i, "Entrada", GREEN, 0);
          } else {
            DateTime fecha = rtc.now();
            char buffer[20];
            sprintf(buffer, "%02d/%02d-%02d:%02d:%02d", fecha.day(), fecha.month(), fecha.hour(), fecha.minute(), fecha.second());
            strcpy(tiemposEntrada[i], buffer);
            guardarEstadoEEPROM(); // Asegura que se almacene aunque no se muestre
          }
        } else if (entradas[i] && !strcmp(CLAVE, CLAVE_SALIR[i])) {
          entradas[i] = false;
          if (!pantallaCalendario) {
            actualizarHabitacion(i, "Salida", RED, 1);
          } else {
            DateTime fecha = rtc.now();
            char buffer[20];
            sprintf(buffer, "%02d/%02d-%02d:%02d:%02d", fecha.day(), fecha.month(), fecha.hour(), fecha.minute(), fecha.second());
            strcpy(tiemposSalida[i], buffer);
            guardarEstadoEEPROM(); // Asegura que se almacene aunque no se muestre
          }
        }
      }
    }

    INDICE = 0; // Reinicia el índice después de procesar la clave
  }

  // Actualización condicional de la pantalla si es necesario
  if (pantallaCalendario && shouldUpdate) {
    dibujarCalendario(currentYear, currentMonth, currentDay);
    shouldUpdate = false; // Evita actualizaciones innecesarias
  }
}
