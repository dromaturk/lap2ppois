# Лабораторная работа №2. Крупный ООП-проект

**Предметная область:** компьютер (комплектующие, периферия, сеть, операционная система, сборка и обслуживание ПК). Язык: C++17.

## Структура репозитория

- `include/computer.h` — вся объектная модель (классы и исключения);
- `src/main.cpp` — консольное меню для работы с моделью;
- `tests/tests.cpp` — тесты на `assert` (проверяют поведения, исключения и виды наследования);
- `.vscode/tasks.json` — задача сборки.

Сборка и запуск:
```
g++ -std=c++17 -Iinclude src/main.cpp -o main && ./main
g++ -std=c++17 -Iinclude tests/tests.cpp -o tests && ./tests
```

## Применённые концепции ООП (по заданию)

| Концепция | Где применена |
|---|---|
| Открытое наследование | `Component : public Named, public Powered`, `Processor`, `Gpu`, `Ssd`, `Hdd`, `Keyboard`, `Mouse`, `Desktop`, `Laptop`, `Server`, `Game`, `Browser` и др. |
| Защищённое наследование | `GamingPc : protected Desktop` — снаружи `GamingPc` не является `Computer`; нужный интерфейс открыт через `using` |
| Закрытое наследование | `SecureServer : private Server` — интерфейс `Server` скрыт, наружу выведены только `startService`, `uptimeHours`, `powerOn` и др. |
| Виртуальные методы | `Identifiable::describe`, `Powered::powerOn/powerOff/status`, `Connectable::connect/disconnect`, `StorageDevice::write/readSpeedMbps` (чисто виртуальный), `Application::run`, `Computer::powerOn/powerOff` |
| Разрешение имён с помощью `using` | `InputDevice`: `using Powered::status` (конфликт с `Connectable::status`); `WifiAdapter`: `using NetworkCard::connect` (перегрузка скрывала бы метод базы); `SecureServer` и `GamingPc`: `using Server::...`, `using Desktop::...` |
| Множественное наследование | `Component : Named, Powered`; `InputDevice : Component, Connectable`; `Display`, `Printer`, `NetworkCard : Component, Connectable`; `Processor`, `Gpu`, `Hdd` с миксином `Monitorable`; `Computer : Named, Powered` |
| Виртуальное наследование | `Named` и `Powered` наследуют `virtual public Identifiable` — «ромб», общий `id` у `Component`, `Computer` и всех потомков хранится один раз |

## Классы

Формат строки: `Класс | число полей | число методов | связанные классы`.

| Класс | Поля | Методы | Ассоциации (связанные классы) |
|---|---|---|---|
| Antivirus | 2 | 5 | Directory |
| Application | 4 | 6 | FileSystem, MemoryManager, ProcessManager |
| Battery | 3 | 6 | — |
| BenchmarkResult | 4 | 4 | — |
| Bios | 3 | 8 | Motherboard, PowerSupply |
| Browser | 2 | 6 | MemoryManager, ProcessManager |
| Clipboard | 2 | 4 | — |
| Component | 2 | 4 | — |
| Computer | 10 | 23 | Gpu, StorageDevice, Motherboard, PowerSupply, CoolingSystem, ComputerCase, Bios, OperatingSystem, Warranty, BenchmarkResult |
| ComputerCase | 3 | 5 | UsbPort, Motherboard |
| Connectable | 1 | 6 | — |
| CoolingSystem | 2 | 3 | Monitorable |
| Desktop | 4 | 8 | RamModule, Keyboard, Mouse, Display |
| Directory | 2 | 8 | File |
| Display | 4 | 6 | — |
| Driver | 3 | 6 | Component |
| File | 4 | 9 | Permissions |
| FileSystem | 3 | 8 | File, Directory |
| Game | 3 | 4 | Gpu, MemoryManager, ProcessManager |
| GamingPc | 2 | 7 | Gpu, StorageDevice, Motherboard, PowerSupply, ComputerCase, Game |
| Gpu | 3 | 4 | — |
| Hdd | 2 | 4 | — |
| Identifiable | 1 | 3 | — |
| InputDevice | 2 | 5 | — |
| IpAddress | 2 | 6 | — |
| Keyboard | 3 | 5 | — |
| Laptop | 4 | 9 | Display, Battery |
| LogEntry | 3 | 3 | — |
| Logger | 3 | 6 | LogEntry |
| MemoryManager | 2 | 5 | — |
| Monitorable | 2 | 4 | — |
| Motherboard | 6 | 9 | Processor, RamModule |
| Mouse | 2 | 4 | — |
| Named | 1 | 3 | — |
| NetworkCard | 4 | 7 | IpAddress |
| OperatingSystem | 11 | 19 | Component, FileSystem, MemoryManager, ProcessManager, Scheduler, Driver, Application, Bios, User, Session, Logger, Settings, Clipboard |
| Permissions | 3 | 5 | — |
| PowerSupply | 2 | 4 | — |
| Powered | 2 | 6 | — |
| Printer | 3 | 5 | — |
| Process | 3 | 8 | — |
| ProcessManager | 2 | 7 | Process, MemoryManager |
| Processor | 3 | 7 | — |
| RamModule | 3 | 5 | — |
| RepairTicket | 3 | 6 | Component, Technician |
| Router | 4 | 5 | IpAddress, NetworkCard |
| Scheduler | 3 | 7 | Process |
| SecureServer | 2 | 5 | IpAddress |
| Server | 2 | 7 | — |
| Session | 3 | 5 | User |
| Settings | 2 | 7 | — |
| Ssd | 2 | 5 | — |
| StorageDevice | 2 | 8 | — |
| Technician | 2 | 4 | Component, Computer, RepairTicket |
| UsbPort | 2 | 4 | — |
| User | 3 | 6 | — |
| Warranty | 2 | 4 | — |
| WifiAdapter | 2 | 5 | — |

### Исключения (14)

Базовое `ComputerException : std::runtime_error`; от него наследуются:

- `PowerException` — нехватка питания, разряженная батарея, выключенное устройство
- `OverheatException` — превышение допустимой температуры
- `IncompatibleComponentException` — несовместимые комплектующие (сокет, тип памяти, корпус)
- `InsufficientMemoryException` — не хватает оперативной памяти
- `DiskFullException` — нет места на накопителе или в файловой системе
- `FileNotFoundException` — файл не найден
- `PermissionDeniedException` — нет прав на операцию
- `ProcessNotFoundException` — процесс не найден / пустая очередь планировщика
- `InvalidIpAddressException` — некорректный IP-адрес
- `NetworkUnavailableException` — нет сети, адреса или пул DHCP исчерпан
- `BootFailureException` — ошибка POST / не хватает комплектующих
- `DeviceNotConnectedException` — устройство не подключено
- `AuthenticationException` — неверный или слабый пароль, нет пользователя
- `InvalidComponentDataException` — некорректные параметры компонента

## Итоговая статистика

| Показатель | Значение | Требование |
|---|---|---|
| Классы (без исключений) | 58 | ≥ 50 |
| Классы исключений | 14 (+ базовый `ComputerException`) | ≥ 12 |
| Поля | 169 | ≥ 150 |
| Методы всего | 357 | — |
| Поведения (методы без простых геттеров) | 281 | ≥ 100 |
| Ассоциации | 70 | ≥ 30 |
