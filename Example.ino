#include "GS32Buttons.h"

// Создаем один менеджер для всех кнопок
GS32Buttons btnGroup;

void setup() {
    Serial.begin(115200);

    // --- Регистрируем кнопку 12 (Кнопка Питания) ---
    auto& powerBtn = btnGroup.add(12);
    powerBtn.onLongPress([](GS32Button& b) {
        Serial.printf("Кнопка на пине %d: ВЫКЛЮЧЕНИЕ системы...\n", b.getPin());
    });
    powerBtn.onBootPress([](GS32Button& b) {
        Serial.println("Система запущена с зажатой кнопкой 12!");
    });

    // --- Регистрируем кнопку 13 (Выбор режима) ---
    auto& modeBtn = btnGroup.add(13);
    modeBtn.onSingleClick([](GS32Button& b) {
        Serial.println("Режим: СЛЕДУЮЩИЙ");
    });
    modeBtn.onDoubleClick([](GS32Button& b) {
        Serial.println("Режим: ПРЕДЫДУЩИЙ");
    });

    // --- Регистрируем кнопку 14 (Универсальная) ---
    btnGroup.add(14).onClick([](GS32Button& b, bool pressed, uint32_t duration) {
        Serial.printf("Кнопка %d %s. Была в прошлом состоянии %u мс\n", 
                      b.getPin(), pressed ? "НАЖАТА" : "ОТПУЩЕНА", duration);
    });

    // Инициализируем все кнопки разом
    btnGroup.begin();

    Serial.println("Все кнопки инициализированы.");
}

void loop() {
    // Один вызов для всех зарегистрированных кнопок
    btnGroup.tick();
}
