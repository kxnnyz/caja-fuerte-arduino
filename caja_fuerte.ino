#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Keypad.h>
#include <Servo.h>

const byte FILAS = 4;
const byte COLUMNAS = 4;

char mapaTeclas[FILAS][COLUMNAS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte pinesFilas[FILAS] = {9, 8, 7, 6};
byte pinesColumnas[COLUMNAS] = {5, 4, 3, 2};

Keypad teclado = Keypad(makeKeymap(mapaTeclas), pinesFilas, pinesColumnas, FILAS, COLUMNAS);
LiquidCrystal_I2C pantalla(0x27, 16, 2);
Servo cerradura;

const int pinServo = 10;
const int longitudMaxima = 8;
const int maxIntentos = 3;
const int anguloAbierto = 90;
const int anguloCerrado = 0;

String claveCorrecta = "1234";
String claveIngresada = "";
char tecla;
int intentosFallidos = 0;
int segundosRestantes = 0;
unsigned long tiempoBloqueo = 30000UL;
bool cajaAbierta = false;
bool sistemaBloqueado = false;
bool claveCorrectaIngresada = false;

void mostrarMensaje(String linea1, String linea2) {
  pantalla.clear();
  pantalla.setCursor(0, 0);
  pantalla.print(linea1);
  pantalla.setCursor(0, 1);
  pantalla.print(linea2);
}

void mostrarAsteriscos(int cantidad) {
  pantalla.setCursor(0, 1);
  pantalla.print("                ");
  pantalla.setCursor(0, 1);
  for (int i = 0; i < cantidad; i++) {
    pantalla.print('*');
  }
}

void reiniciarEntrada() {
  claveIngresada = "";
  mostrarMensaje("Ingrese clave:", "");
}

char leerTecla() {
  return teclado.getKey();
}

bool verificarClave(String claveIngresadaParametro) {
  return claveIngresadaParametro == claveCorrecta;
}

void abrirCaja() {
  cerradura.write(anguloAbierto);
  cajaAbierta = true;
  mostrarMensaje("Acceso concedido", "# para cerrar");
}

void cerrarCaja() {
  cerradura.write(anguloCerrado);
  cajaAbierta = false;
  mostrarMensaje("Caja cerrada", "");
  delay(1500);
  reiniciarEntrada();
}

int registrarIntentoFallido() {
  intentosFallidos++;
  return intentosFallidos;
}

void bloquearSistema(int segundos) {
  sistemaBloqueado = true;
  segundosRestantes = segundos;
  while (segundosRestantes > 0) {
    mostrarMensaje("Sistema bloqueado", "Espere: " + String(segundosRestantes) + " s");
    delay(1000);
    segundosRestantes -= 1;
  }
  sistemaBloqueado = false;
  intentosFallidos = 0;
  reiniciarEntrada();
}

void procesarTecla(char teclaPulsada) {
  if (teclaPulsada >= '0' && teclaPulsada <= '9') {
    if (claveIngresada.length() < longitudMaxima) {
      claveIngresada += teclaPulsada;
      mostrarAsteriscos(claveIngresada.length());
    }
  } else if (teclaPulsada == '*') {
    reiniciarEntrada();
  } else if (teclaPulsada == '#') {
    claveCorrectaIngresada = verificarClave(claveIngresada);
    if (claveCorrectaIngresada && !sistemaBloqueado) {
      intentosFallidos = 0;
      abrirCaja();
      claveIngresada = "";
    } else {
      registrarIntentoFallido();
      mostrarMensaje("Clave incorrecta", "Intento " + String(intentosFallidos) + "/" + String(maxIntentos));
      delay(1500);
      if (intentosFallidos >= maxIntentos) {
        bloquearSistema(tiempoBloqueo / 1000);
      } else {
        reiniciarEntrada();
      }
    }
  }
}

void setup() {
  pantalla.init();
  pantalla.backlight();
  cerradura.attach(pinServo);
  cerradura.write(anguloCerrado);
  cajaAbierta = false;
  sistemaBloqueado = false;
  intentosFallidos = 0;
  reiniciarEntrada();
}

void loop() {
  tecla = leerTecla();
  if (tecla != NO_KEY && !sistemaBloqueado) {
    if (cajaAbierta) {
      if (tecla == '#') {
        cerrarCaja();
      }
    } else {
      procesarTecla(tecla);
    }
  }
}
