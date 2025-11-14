#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

Adafruit_LEDBackpack matrix = Adafruit_LEDBackpack();

// Структура для точки
struct Point {
  int x;
  int y;
};

// Змейка
Point snake[64]; // Максимальная длина змейки
int length = 1;  // Начальная длина
int dirX = 1;    // Начальное направление: вправо
int dirY = 0;

// Еда
Point food;

// Джойстик
const int joyXPin = A2;
const int joyYPin = A1;
const int threshold = 300; // Порог срабатывания джойстика

// Игровые параметры
unsigned long lastMove = 0;
const int moveDelay = 200; // Задержка между движениями (мс)
bool gameOver = false;

void spawnFood() {
  bool onSnake;
  do {
    onSnake = false;
    food.x = random(8);
    food.y = random(8);
    
    // Проверяем, не появится ли еда на змейке
    for (int i = 0; i < length; i++) {
      if (snake[i].x == food.x && snake[i].y == food.y) {
        onSnake = true;
        break;
      }
    }
  } while (onSnake);
}

void resetGame() {
  // Начальная позиция змейки
  snake[0].x = 3;
  snake[0].y = 4;
  length = 1;
  dirX = 1;
  dirY = 0;
  gameOver = false;
  
  // Первая еда
  spawnFood();
  
  // Сброс матрицы
  matrix.clear();
  matrix.writeDisplay();
}

void setup() {
  Serial.begin(9600);
  randomSeed(analogRead(3));
  matrix.begin(0x70);
  matrix.setBrightness(5);
  resetGame();
}

void loop() {
  if (gameOver) {
    // Мигание при завершении игры
    matrix.clear();
    matrix.writeDisplay();
    delay(500);
    
    // Отображаем змейку
    for (int i = 0; i < length; i++) {
      matrix.displaybuffer[snake[i].y] |= 1 << snake[i].x;
    }
    matrix.writeDisplay();
    delay(500);
    return;
  }

  // Управление с джойстика
  int joyX = analogRead(joyXPin);
  int joyY = analogRead(joyYPin);

  // Обработка ввода (с защитой от разворота на 180°)
  if (joyX < threshold && dirX == 0) { // Влево
    dirX = 1;
    dirY = 0;
  } else if (joyX > 1023 - threshold && dirX == 0) { // Вправо
    dirX = -1;
    dirY = 0;
  } else if (joyY < threshold && dirY == 0) { // Вниз
    dirY = 1;
    dirX = 0;
  } else if (joyY > 1023 - threshold && dirY == 0) { // Вверх
    dirY = -1;
    dirX = 0;
  }

  // Движение змейки с заданной скоростью
  if (millis() - lastMove > moveDelay) {
    lastMove = millis();

    // Создаем новую голову
    Point newHead;
    newHead.x = snake[0].x + dirX;
    newHead.y = snake[0].y + dirY;

    // Телепортация через границы
    if (newHead.x < 0) newHead.x = 7;
    else if (newHead.x > 7) newHead.x = 0;
    if (newHead.y < 0) newHead.y = 7;
    else if (newHead.y > 7) newHead.y = 0;

    // Проверка столкновения с собой
    for (int i = 0; i < length; i++) {
      if (newHead.x == snake[i].x && newHead.y == snake[i].y) {
        gameOver = true;
        return;
      }
    }

    // Сдвигаем змейку
    for (int i = length; i > 0; i--) {
      snake[i] = snake[i-1];
    }
    snake[0] = newHead;

    // Проверка съедения еды
    if (snake[0].x == food.x && snake[0].y == food.y) {
      length++;
      if (length > 64) length = 64; // Максимальный размер
      spawnFood();
    }

    // Отрисовка
    matrix.clear();
    
    // Рисуем змейку
    for (int i = 0; i < length; i++) {
      matrix.displaybuffer[snake[i].y] |= 1 << snake[i].x;
    }
    
    // Рисуем еду
    matrix.displaybuffer[food.y] |= 1 << food.x;
    
    matrix.writeDisplay();
  }
}