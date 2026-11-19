#include "computer.h"

#include <iostream>
#include <limits>
#include <string>

std::shared_ptr<Desktop> buildDesktop() {
    auto pc = std::make_shared<Desktop>("Домашний ПК");
    auto board = std::make_shared<Motherboard>("B550 Tomahawk", "MSI", 150, "AM4", "DDR4", "ATX", 4);
    board->installProcessor(std::make_shared<Processor>("Ryzen 5 5600X", "AMD", 200, 6, 3.7, "AM4"));
    board->installRam(RamModule("Vengeance 16GB", "Corsair", 60, 16384, 3200, "DDR4"));
    pc->setMotherboard(board);
    pc->setCase(std::make_shared<ComputerCase>("Define 7", "ATX", 4));
    pc->setPowerSupply(std::make_shared<PowerSupply>("RM750", "Corsair", 130, 750, 0.9));
    pc->installGpu(std::make_shared<Gpu>("RTX 3060", "NVIDIA", 350, 12288, 3584, 1800));
    pc->setCooling(std::make_shared<CoolingSystem>("NH-D15", "Noctua", 100, 1500, false));
    pc->addStorage(std::make_shared<Ssd>("970 EVO", "Samsung", 100, 500, 3500));
    pc->addStorage(std::make_shared<Hdd>("Barracuda", "Seagate", 50, 2000, 7200));
    pc->installOperatingSystem("Linux", "6.1");
    pc->os().addUser(User("admin", "Passw0rdX", true));
    return pc;
}

void showMenu() {
    std::cout << "\n=== Компьютер ===\n"
              << "1. Включить\n"
              << "2. Выключить\n"
              << "3. Установить и запустить браузер\n"
              << "4. Установить и запустить игру\n"
              << "5. Показать процессы и память\n"
              << "6. Запустить тест производительности\n"
              << "7. Войти в систему (admin)\n"
              << "8. Добавить модуль памяти (DDR4, 8 ГБ)\n"
              << "9. Диагностика мастером\n"
              << "0. Выход\n"
              << "Выбор: ";
}

int readInt() {
    int value;
    while (!(std::cin >> value)) {
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Введите целое число: ";
    }
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    return value;
}

int main() {
    auto pc = buildDesktop();
    Browser browser("Firefox", "120", 2048);
    Game game("Doom", "1.0", 4096, 2048, 4096, 144);
    Technician technician("Иван", "диагностика");
    int choice = -1;

    while (choice != 0) {
        showMenu();
        choice = readInt();
        try {
            if (choice == 1) {
                pc->powerOn();
                std::cout << pc->describe() << '\n';
            } else if (choice == 2) {
                pc->powerOff();
                std::cout << pc->describe() << '\n';
            } else if (choice == 3 || choice == 4) {
                Application& app = (choice == 3) ? static_cast<Application&>(browser) : static_cast<Application&>(game);
                if (!app.isInstalled()) pc->os().installApplication(app);
                int pid = pc->os().launch(app);
                std::cout << app.name() << " запущен, PID " << pid << '\n';
            } else if (choice == 5) {
                auto& os = pc->os();
                for (const auto& name : os.processes().names()) std::cout << " - " << name << '\n';
                std::cout << "Память занята на " << os.memory().usagePercent() << " %\n";
            } else if (choice == 6) {
                std::cout << pc->runBenchmark().summary() << '\n';
            } else if (choice == 7) {
                std::cout << "Пароль: ";
                std::string password;
                std::getline(std::cin, password);
                Session session = pc->os().login("admin", password, 0);
                std::cout << "Вход выполнен: " << session.userName() << '\n';
            } else if (choice == 8) {
                pc->upgradeRam(RamModule("Vengeance 8GB", "Corsair", 35, 8192, 3200, "DDR4"));
                std::cout << "Модуль установлен.\n";
            } else if (choice == 9) {
                auto issues = technician.diagnose(*pc);
                std::cout << (issues.empty() ? "Неисправностей нет.\n" : "Есть проблемы.\n");
            } else if (choice != 0) {
                std::cout << "Неизвестный пункт меню.\n";
            }
        } catch (const ComputerException& error) {
            std::cout << "Ошибка: " << error.what() << '\n';
        }
    }
    std::cout << "Работа завершена.\n";
    return 0;
}
