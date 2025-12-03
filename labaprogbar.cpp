#include <coroutine>
#include <iostream>
#include <chrono>
#include <thread>
#include <string>
#include <iomanip>
#include <clocale>

// ========== ПУНКТ 1: Простая корутина с выводом числа ==========

struct promise_simple;

struct coroutine_simple : std::coroutine_handle<promise_simple>
{
    using promise_type = ::promise_simple;
};

struct promise_simple
{
    coroutine_simple get_return_object() {
        return { coroutine_simple::from_promise(*this) };
    }
    std::suspend_always initial_suspend() noexcept { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}
};

void run_punkt1() {
    std::cout << "=== ПУНКТ 1: Простая корутина с выводом числа ===" << std::endl;

    coroutine_simple h = [](int i) -> coroutine_simple
        {
            std::cout << "Мое специальное число: " << i << std::endl;
            co_return;
        }(1337);

    h.resume();
    h.destroy();

    std::cout << std::endl;
}

// ========== ПУНКТ 2: Корутина с прогрессбаром ==========

struct promise_type
{
    int current_value = 0;

    auto get_return_object()
    {
        return std::coroutine_handle<promise_type>::from_promise(*this);
    }

    std::suspend_always initial_suspend() { return {}; }
    std::suspend_always final_suspend() noexcept { return {}; }
    void return_void() {}
    void unhandled_exception() {}

    std::suspend_always yield_value(int value)
    {
        current_value = value;
        return {};
    }
};

struct task
{
    std::coroutine_handle<promise_type> handle;

    task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~task() { if (handle) handle.destroy(); }

    void resume() { if (handle && !handle.done()) handle.resume(); }
    bool done() const { return handle.done(); }
    int get_value() const { return handle.promise().current_value; }
};

namespace std
{
    template<>
    struct coroutine_traits<task, int>
    {
        using promise_type = ::promise_type;
    };
}

task long_computation(int steps)
{
    for (int i = 1; i <= steps; ++i)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(30));
        co_yield i;
    }
    co_return;
}

// Простой прогрессбар с именем "ЕГОР" (без анимации)
void print_progress_with_name(int current, int total)
{
    float percent = static_cast<float>(current) / total * 100.f;
    int bar_width = 50;

    std::string bar;
    std::string name = "ЕГОР";

    size_t filled = static_cast<size_t>(percent / 100.f * bar_width);

    // Заполняем прогрессбар именем "ЕГОР"
    for (int i = 0; i < bar_width; ++i) {
        if (i < filled) {
            // Циклически используем символы имени "ЕГОР"
            bar += name[i % name.length()];
        }
        else {
            bar += ' ';
        }
    }

    std::cout << "\r[" << bar << "] "
        << std::fixed << std::setprecision(1) << percent << "%"
        << " [" << current << "/" << total << "]";
    std::cout.flush();
}

void run_punkt2() {
    std::cout << "=== ПУНКТ 2: Корутина с прогрессбаром ===" << std::endl;
    std::cout << "Имя: ЕГОР" << std::endl;
    std::cout << "Запуск имитации долгой операции..." << std::endl;
    std::cout << std::endl;

    constexpr int TOTAL_STEPS = 150;
    std::cout << "Всего шагов: " << TOTAL_STEPS << std::endl;
    std::cout << "Прогресс: ";

    auto coro = long_computation(TOTAL_STEPS);

    while (!coro.done())
    {
        coro.resume();
        int progress = coro.get_value();
        print_progress_with_name(progress, TOTAL_STEPS);
        std::this_thread::yield();
    }

    // Финальное сообщение
    std::cout << "\n\n";
    std::cout << "╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║          ОПЕРАЦИЯ ЗАВЕРШЕНА УСПЕШНО!          ║" << std::endl;
    std::cout << "║                                                ║" << std::endl;
    std::cout << "║          Имя в прогрессбаре: ЕГОР             ║" << std::endl;
    std::cout << "║          Всего выполнено: " << std::setw(3) << TOTAL_STEPS << " шагов          ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
}

// ========== Главная функция ==========

int main()
{
    // Установка русской локали
    setlocale(LC_ALL, "Russian");

    std::cout << "╔════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║      ЛАБОРАТОРНАЯ РАБОТА №6: КОРУТИНЫ         ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════╝" << std::endl;
    std::cout << std::endl;

    // Запуск Пункта 1
    run_punkt1();

    // Пауза между демонстрациями
    std::cout << "Подготовка ко второму пункту..." << std::endl;
    for (int i = 3; i > 0; i--) {
        std::cout << "Старт через " << i << "..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    std::cout << std::endl;

    // Запуск Пункта 2
    run_punkt2();

    std::cout << std::endl;
    std::cout << "==================================================" << std::endl;
    std::cout << "    Лабораторная работа выполнена студентом:     " << std::endl;
    std::cout << "                  Егор                            " << std::endl;
    std::cout << "==================================================" << std::endl;

    return 0;
}