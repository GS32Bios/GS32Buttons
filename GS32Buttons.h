#ifndef GS32_BUTTONS_H
#define GS32_BUTTONS_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <memory>

// Класс ОДНОЙ кнопки
class GS32Button {
public:
    using ButtonCallback = std::function<void(GS32Button&)>;
    using StateCallback  = std::function<void(GS32Button&, bool pressed, uint32_t duration)>;

    GS32Button(uint8_t pin, bool pullup = true, uint32_t debounceMs = 30)
        : _pin(pin), _pullup(pullup), _debounceMs(debounceMs) {}

    void begin() {
        pinMode(_pin, _pullup ? INPUT_PULLUP : INPUT);
        bool rawState = digitalRead(_pin);
        _isPressed = _pullup ? !rawState : rawState;
        _lastDebounceState = _isPressed;
        _lastStateChangeTime = millis();
        _lastDebounceTime = millis();

        if (_isPressed && _onBootCallback) {
            _bootPressed = true;
            _onBootCallback(*this);
        }
    }

    // Настройки
    void setDoubleClickTime(uint32_t ms) { _doubleClickMs = ms; }
    void setLongPressTime(uint32_t ms)   { _longPressMs = ms; }

    // События
    void onClick(StateCallback cb)        { _onClickCallback = cb; }
    void onSingleClick(ButtonCallback cb) { _onSingleClickCallback = cb; }
    void onDoubleClick(ButtonCallback cb) { _onDoubleClickCallback = cb; }
    void onLongPress(ButtonCallback cb)   { _onLongPressCallback = cb; }
    void onBootPress(ButtonCallback cb)   { _onBootCallback = cb; }

    // Состояние
    uint8_t getPin() const { return _pin; }
    bool isPressed() const { return _isPressed; }

    void tick() {
        uint32_t now = millis();
        bool rawRead = digitalRead(_pin);
        bool currentPressed = _pullup ? !rawRead : rawRead;

        if (currentPressed != _lastDebounceState) {
            _lastDebounceTime = now;
            _lastDebounceState = currentPressed;
        }

        if ((now - _lastDebounceTime) >= _debounceMs) {
            if (currentPressed != _isPressed) {
                uint32_t duration = now - _lastStateChangeTime;
                _isPressed = currentPressed;
                _lastStateChangeTime = now;

                if (_onClickCallback) _onClickCallback(*this, _isPressed, duration);

                if (_isPressed) {
                    _longPressHandled = false;
                    _pressStartTime = now;
                } else {
                    if (!_longPressHandled) {
                        _clickCount++;
                        _lastClickTime = now;
                    }
                }
            }
        }

        if (_isPressed && !_longPressHandled && (now - _pressStartTime >= _longPressMs)) {
            _longPressHandled = true;
            _clickCount = 0;
            if (_onLongPressCallback) _onLongPressCallback(*this);
        }

        if (_clickCount > 0 && !_isPressed) {
            if (_clickCount == 1 && (now - _lastClickTime > _doubleClickMs)) {
                _clickCount = 0;
                if (_onSingleClickCallback) _onSingleClickCallback(*this);
            } else if (_clickCount >= 2) {
                _clickCount = 0;
                if (_onDoubleClickCallback) _onDoubleClickCallback(*this);
            }
        }
    }

private:
    uint8_t _pin;
    bool _pullup;
    uint32_t _debounceMs;
    uint32_t _doubleClickMs = 300;
    uint32_t _longPressMs = 1000;
    bool _isPressed = false;
    bool _lastDebounceState = false;
    bool _bootPressed = false;
    bool _longPressHandled = false;
    uint32_t _lastDebounceTime = 0;
    uint32_t _lastStateChangeTime = 0;
    uint32_t _pressStartTime = 0;
    uint32_t _lastClickTime = 0;
    uint8_t _clickCount = 0;

    StateCallback _onClickCallback = nullptr;
    ButtonCallback _onSingleClickCallback = nullptr;
    ButtonCallback _onDoubleClickCallback = nullptr;
    ButtonCallback _onLongPressCallback = nullptr;
    ButtonCallback _onBootCallback = nullptr;
};

// МЕНЕДЖЕР списка кнопок
class GS32Buttons {
public:
    // Добавить кнопку и вернуть на неё ссылку для настройки
    GS32Button& add(uint8_t pin, bool pullup = true, uint32_t debounceMs = 30) {
        _buttons.emplace_back(new GS32Button(pin, pullup, debounceMs));
        return *_buttons.back();
    }

    // Инициализировать ВСЕ кнопки (вызывать в конце setup)
    void begin() {
        for (auto& btn : _buttons) {
            btn->begin();
        }
    }

    // Опрашивать ВСЕ кнопки (вызывать в loop)
    void tick() {
        for (auto& btn : _buttons) {
            btn->tick();
        }
    }

private:
    std::vector<std::unique_ptr<GS32Button>> _buttons;
};

#endif
