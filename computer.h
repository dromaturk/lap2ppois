#pragma once

#include <algorithm>
#include <cctype>
#include <functional>
#include <map>
#include <memory>
#include <numeric>
#include <stdexcept>
#include <string>
#include <vector>

// ============================================================================
//  Исключения предметной области «Компьютер»
// ============================================================================
class ComputerException : public std::runtime_error {
public:
    explicit ComputerException(const std::string& message) : std::runtime_error(message) {}
};
class PowerException : public ComputerException { public: using ComputerException::ComputerException; };
class OverheatException : public ComputerException { public: using ComputerException::ComputerException; };
class IncompatibleComponentException : public ComputerException { public: using ComputerException::ComputerException; };
class InsufficientMemoryException : public ComputerException { public: using ComputerException::ComputerException; };
class DiskFullException : public ComputerException { public: using ComputerException::ComputerException; };
class FileNotFoundException : public ComputerException { public: using ComputerException::ComputerException; };
class PermissionDeniedException : public ComputerException { public: using ComputerException::ComputerException; };
class ProcessNotFoundException : public ComputerException { public: using ComputerException::ComputerException; };
class InvalidIpAddressException : public ComputerException { public: using ComputerException::ComputerException; };
class NetworkUnavailableException : public ComputerException { public: using ComputerException::ComputerException; };
class BootFailureException : public ComputerException { public: using ComputerException::ComputerException; };
class DeviceNotConnectedException : public ComputerException { public: using ComputerException::ComputerException; };
class AuthenticationException : public ComputerException { public: using ComputerException::ComputerException; };
class InvalidComponentDataException : public ComputerException { public: using ComputerException::ComputerException; };

// ============================================================================
//  Базовые классы: виртуальное и множественное наследование
// ============================================================================

// Виртуальная база: общий для всей иерархии идентификатор (ромбовидное наследование).
class Identifiable {
    static int& counter() { static int value = 0; return value; }
protected:
    int id_;
public:
    Identifiable() : id_(++counter()) {}
    virtual ~Identifiable() = default;
    int id() const { return id_; }
    virtual std::string describe() const { return "Объект #" + std::to_string(id_); }
};

class Named : virtual public Identifiable {
protected:
    std::string name_;
public:
    explicit Named(const std::string& name = "") : name_(name) {}
    const std::string& name() const { return name_; }
    void rename(const std::string& value) {
        if (value.empty()) throw InvalidComponentDataException("Название не может быть пустым");
        name_ = value;
    }
    std::string describe() const override { return name_ + " #" + std::to_string(id_); }
};

class Powered : virtual public Identifiable {
protected:
    bool on_;
    int watts_;
public:
    explicit Powered(int watts = 0) : on_(false), watts_(watts) {}
    virtual void powerOn() { on_ = true; }
    virtual void powerOff() { on_ = false; }
    bool isOn() const { return on_; }
    int watts() const { return on_ ? watts_ : 0; }
    int ratedWatts() const { return watts_; }
    virtual std::string status() const { return on_ ? "включено" : "выключено"; }
};

// Интерфейс подключаемого устройства (чисто виртуальный метод interfaceName).
class Connectable {
protected:
    bool connected_ = false;
public:
    virtual ~Connectable() = default;
    virtual std::string interfaceName() const = 0;
    virtual void connect() { connected_ = true; }
    virtual void disconnect() { connected_ = false; }
    bool isConnected() const { return connected_; }
    virtual std::string status() const { return connected_ ? "подключено" : "отключено"; }
    void requireConnection() const {
        if (!connected_) throw DeviceNotConnectedException("Устройство не подключено");
    }
};

// Миксин контроля температуры.
class Monitorable {
protected:
    double temperature_ = 30.0;
    double maxTemperature_;
public:
    explicit Monitorable(double maxTemperature = 90.0) : maxTemperature_(maxTemperature) {}
    virtual ~Monitorable() = default;
    void setTemperature(double value) {
        temperature_ = value;
        if (value > maxTemperature_) throw OverheatException("Перегрев: " + std::to_string(static_cast<int>(value)) + " °C");
    }
    double temperature() const { return temperature_; }
    bool isOverheated() const { return temperature_ > maxTemperature_; }
    void coolDown(double degrees) { temperature_ = std::max(20.0, temperature_ - degrees); }
};

// Ромб: Component -> (Named, Powered) -> Identifiable (виртуальная база).
class Component : public Named, public Powered {
protected:
    std::string manufacturer_;
    double price_;
public:
    Component(const std::string& name, const std::string& manufacturer, double price, int watts)
        : Named(name), Powered(watts), manufacturer_(manufacturer), price_(price) {
        if (price < 0) throw InvalidComponentDataException("Цена не может быть отрицательной");
    }
    double price() const { return price_; }
    const std::string& manufacturer() const { return manufacturer_; }
    double priceWithDiscount(double percent) const {
        if (percent < 0 || percent > 100) throw InvalidComponentDataException("Скидка должна быть от 0 до 100");
        return price_ * (1.0 - percent / 100.0);
    }
    std::string describe() const override { return name_ + " (" + manufacturer_ + ")"; }
};

// ============================================================================
//  Комплектующие
// ============================================================================
class Processor : public Component, public Monitorable {
    int cores_;
    double frequencyGhz_;
    std::string socket_;
public:
    Processor(const std::string& name, const std::string& maker, double price,
              int cores, double ghz, const std::string& socket)
        : Component(name, maker, price, 65), Monitorable(100.0), cores_(cores), frequencyGhz_(ghz), socket_(socket) {
        if (cores <= 0 || ghz <= 0) throw InvalidComponentDataException("Некорректные параметры процессора");
    }
    int cores() const { return cores_; }
    double frequency() const { return frequencyGhz_; }
    const std::string& socket() const { return socket_; }
    bool fitsSocket(const std::string& socket) const { return socket_ == socket; }
    int benchmarkScore() const { return static_cast<int>(cores_ * frequencyGhz_ * 1000); }
    void setFrequency(double ghz) {
        if (ghz < 0.5 || ghz > 7.0) throw InvalidComponentDataException("Частота вне допустимого диапазона");
        frequencyGhz_ = ghz;
    }
    void overclock(double percent) {
        setFrequency(frequencyGhz_ * (1.0 + percent / 100.0));
        setTemperature(temperature_ + percent * 1.5);
    }
};

class Gpu : public Component, public Monitorable {
    int vramMb_;
    int shaderUnits_;
    int boostMhz_;
public:
    Gpu(const std::string& name, const std::string& maker, double price, int vramMb, int shaderUnits, int boostMhz)
        : Component(name, maker, price, 200), Monitorable(95.0), vramMb_(vramMb), shaderUnits_(shaderUnits), boostMhz_(boostMhz) {
        if (vramMb <= 0 || shaderUnits <= 0 || boostMhz <= 0) throw InvalidComponentDataException("Некорректные параметры видеокарты");
    }
    int vramMb() const { return vramMb_; }
    bool meetsVram(int requiredMb) const { return vramMb_ >= requiredMb; }
    int renderFps(int sceneComplexity) const {
        if (sceneComplexity <= 0) throw InvalidComponentDataException("Сложность сцены должна быть положительной");
        return std::max(1, static_cast<int>(static_cast<long long>(shaderUnits_) * boostMhz_ / (sceneComplexity * 50LL)));
    }
    void boost(int mhz) {
        boostMhz_ += mhz;
        setTemperature(temperature_ + mhz / 20.0);
    }
};

class RamModule : public Component {
    int capacityMb_;
    int speedMhz_;
    std::string type_;
public:
    RamModule(const std::string& name, const std::string& maker, double price, int capacityMb, int speedMhz, const std::string& type)
        : Component(name, maker, price, 5), capacityMb_(capacityMb), speedMhz_(speedMhz), type_(type) {
        if (capacityMb <= 0 || speedMhz <= 0) throw InvalidComponentDataException("Некорректные параметры памяти");
    }
    int capacityMb() const { return capacityMb_; }
    int speedMhz() const { return speedMhz_; }
    const std::string& type() const { return type_; }
    int bandwidthMbps() const { return speedMhz_ * 8; }
    bool isFasterThan(const RamModule& other) const { return speedMhz_ > other.speedMhz_; }
};

// Абстрактный накопитель.
class StorageDevice : public Component {
protected:
    int capacityGb_;
    int usedGb_;
public:
    StorageDevice(const std::string& name, const std::string& maker, double price, int capacityGb, int watts)
        : Component(name, maker, price, watts), capacityGb_(capacityGb), usedGb_(0) {
        if (capacityGb <= 0) throw InvalidComponentDataException("Ёмкость должна быть положительной");
    }
    virtual int readSpeedMbps() const = 0;
    virtual void write(int gb) {
        if (gb < 0) throw InvalidComponentDataException("Нельзя записать отрицательный объём");
        if (usedGb_ + gb > capacityGb_) throw DiskFullException("Недостаточно места на накопителе");
        usedGb_ += gb;
    }
    virtual void erase(int gb) { usedGb_ = std::max(0, usedGb_ - gb); }
    int capacityGb() const { return capacityGb_; }
    int usedGb() const { return usedGb_; }
    int freeGb() const { return capacityGb_ - usedGb_; }
    double fillPercent() const { return 100.0 * usedGb_ / capacityGb_; }
    int readTimeSeconds(int gb) const { return std::max(1, gb * 1024 / readSpeedMbps()); }
};

class Ssd : public StorageDevice {
    int readMbps_;
    double wearPercent_;
public:
    Ssd(const std::string& name, const std::string& maker, double price, int capacityGb, int readMbps)
        : StorageDevice(name, maker, price, capacityGb, 5), readMbps_(readMbps), wearPercent_(0.0) {}
    int readSpeedMbps() const override { return readMbps_; }
    void write(int gb) override {
        StorageDevice::write(gb);
        wearPercent_ += gb * 0.01;
    }
    double wear() const { return wearPercent_; }
    bool needsReplacement() const { return wearPercent_ > 80.0; }
    void trim() { wearPercent_ = std::max(0.0, wearPercent_ - 0.5); }
};

class Hdd : public StorageDevice, public Monitorable {
    int rpm_;
    int badSectors_;
public:
    Hdd(const std::string& name, const std::string& maker, double price, int capacityGb, int rpm)
        : StorageDevice(name, maker, price, capacityGb, 8), Monitorable(70.0), rpm_(rpm), badSectors_(0) {
        if (rpm <= 0) throw InvalidComponentDataException("Скорость вращения должна быть положительной");
    }
    int readSpeedMbps() const override { return rpm_ / 50; }
    void defragment() { setTemperature(temperature_ + 5.0); }
    void markBadSector() { ++badSectors_; }
    bool isHealthy() const { return badSectors_ < 100; }
};

class UsbPort {
    double version_;
    bool occupied_;
public:
    explicit UsbPort(double version = 3.0) : version_(version), occupied_(false) {}
    void plug() {
        if (occupied_) throw ComputerException("USB-порт уже занят");
        occupied_ = true;
    }
    void unplug() { occupied_ = false; }
    bool isFree() const { return !occupied_; }
    int speedMbps() const { return version_ >= 3.0 ? 5000 : 480; }
};

class Motherboard : public Component {
    std::string socket_;
    std::string ramType_;
    std::string formFactor_;
    int ramSlots_;
    std::shared_ptr<Processor> cpu_;
    std::vector<RamModule> ram_;
public:
    Motherboard(const std::string& name, const std::string& maker, double price, const std::string& socket,
                const std::string& ramType, const std::string& formFactor, int ramSlots)
        : Component(name, maker, price, 40), socket_(socket), ramType_(ramType), formFactor_(formFactor), ramSlots_(ramSlots) {
        if (ramSlots <= 0) throw InvalidComponentDataException("Нужен хотя бы один слот памяти");
    }
    void installProcessor(std::shared_ptr<Processor> cpu) {
        if (!cpu) throw InvalidComponentDataException("Процессор не задан");
        if (!cpu->fitsSocket(socket_)) throw IncompatibleComponentException("Сокет процессора не подходит к плате");
        cpu_ = cpu;
    }
    void installRam(const RamModule& module) {
        if (static_cast<int>(ram_.size()) >= ramSlots_) throw IncompatibleComponentException("Нет свободных слотов памяти");
        if (module.type() != ramType_) throw IncompatibleComponentException("Тип памяти не поддерживается платой");
        ram_.push_back(module);
    }
    void removeRam() {
        if (ram_.empty()) throw ComputerException("Модули памяти не установлены");
        ram_.pop_back();
    }
    bool hasProcessor() const { return cpu_ != nullptr; }
    Processor& processor() const {
        if (!cpu_) throw BootFailureException("Процессор не установлен");
        return *cpu_;
    }
    int totalRamMb() const {
        return std::accumulate(ram_.begin(), ram_.end(), 0, [](int sum, const RamModule& m) { return sum + m.capacityMb(); });
    }
    int freeSlots() const { return ramSlots_ - static_cast<int>(ram_.size()); }
    const std::string& socket() const { return socket_; }
    const std::string& formFactor() const { return formFactor_; }
};

class PowerSupply : public Component {
    int wattage_;
    double efficiency_;
public:
    PowerSupply(const std::string& name, const std::string& maker, double price, int wattage, double efficiency)
        : Component(name, maker, price, 0), wattage_(wattage), efficiency_(efficiency) {
        if (wattage <= 0 || efficiency <= 0 || efficiency > 1.0) throw InvalidComponentDataException("Некорректные параметры блока питания");
    }
    int wattage() const { return wattage_; }
    bool canSupply(int load) const { return load <= wattage_; }
    double loadPercent(int load) const { return 100.0 * load / wattage_; }
    double inputPower(int load) const { return load / efficiency_; }
};

class CoolingSystem : public Component {
    int fanRpm_;
    bool liquid_;
public:
    CoolingSystem(const std::string& name, const std::string& maker, double price, int fanRpm, bool liquid)
        : Component(name, maker, price, 6), fanRpm_(fanRpm), liquid_(liquid) {}
    void cool(Monitorable& target) const { target.coolDown(fanRpm_ / 100.0 * (liquid_ ? 1.5 : 1.0)); }
    void setFanSpeed(int rpm) {
        if (rpm < 0 || rpm > 5000) throw InvalidComponentDataException("Скорость вентилятора вне диапазона");
        fanRpm_ = rpm;
    }
    int noiseDb() const { return 15 + fanRpm_ / 100; }
};

class ComputerCase : public Named {
    std::string formFactor_;
    int fans_;
    std::vector<UsbPort> ports_;
public:
    ComputerCase(const std::string& name, const std::string& formFactor, int usbCount, int fans = 2)
        : Named(name), formFactor_(formFactor), fans_(fans), ports_(usbCount) {}
    bool supportsBoard(const Motherboard& board) const {
        return board.formFactor() == formFactor_ || (formFactor_ == "ATX" && board.formFactor() == "mATX");
    }
    int freePorts() const {
        return static_cast<int>(std::count_if(ports_.begin(), ports_.end(), [](const UsbPort& p) { return p.isFree(); }));
    }
    void plugDevice() {
        for (auto& port : ports_) {
            if (port.isFree()) { port.plug(); return; }
        }
        throw ComputerException("Нет свободных USB-портов");
    }
    void unplugAll() { for (auto& port : ports_) port.unplug(); }
    int fanCount() const { return fans_; }
};

// ---------------------------------------------------------------------------
//  Периферия: множественное наследование Component + Connectable.
//  Методы status() есть и в Powered, и в Connectable — конфликт имён
//  разрешается с помощью using-объявления.
// ---------------------------------------------------------------------------
class InputDevice : public Component, public Connectable {
protected:
    bool wireless_;
    int batteryLevel_;
public:
    using Powered::status;
    InputDevice(const std::string& name, const std::string& maker, double price, bool wireless)
        : Component(name, maker, price, 2), wireless_(wireless), batteryLevel_(100) {}
    std::string statusFull() const { return Powered::status() + ", " + Connectable::status(); }
    void connect() override {
        powerOn();
        Connectable::connect();
    }
    void disconnect() override {
        Connectable::disconnect();
        powerOff();
    }
    bool isWireless() const { return wireless_; }
    void drainBattery(int percent) {
        if (!wireless_) return;
        batteryLevel_ = std::max(0, batteryLevel_ - percent);
        if (batteryLevel_ == 0) powerOff();
    }
};

class Keyboard : public InputDevice {
    std::string layout_;
    std::string buffer_;
    int keysPressed_;
public:
    Keyboard(const std::string& name, const std::string& maker, double price, bool wireless, const std::string& layout = "RU")
        : InputDevice(name, maker, price, wireless), layout_(layout), keysPressed_(0) {}
    std::string interfaceName() const override { return wireless_ ? "Bluetooth" : "USB"; }
    void press(char key) {
        requireConnection();
        buffer_ += key;
        ++keysPressed_;
        if (keysPressed_ % 100 == 0) drainBattery(1);
    }
    const std::string& readBuffer() const { return buffer_; }
    void clearBuffer() { buffer_.clear(); }
    int wordCount() const {
        int words = 0;
        bool inWord = false;
        for (char c : buffer_) {
            bool letter = !std::isspace(static_cast<unsigned char>(c));
            if (letter && !inWord) ++words;
            inWord = letter;
        }
        return words;
    }
};

class Mouse : public InputDevice {
    int dpi_;
    int clicks_;
public:
    Mouse(const std::string& name, const std::string& maker, double price, bool wireless, int dpi = 800)
        : InputDevice(name, maker, price, wireless), dpi_(dpi), clicks_(0) {}
    std::string interfaceName() const override { return wireless_ ? "Bluetooth" : "USB"; }
    void setDpi(int value) {
        if (value < 100 || value > 32000) throw InvalidComponentDataException("DPI вне диапазона");
        dpi_ = value;
    }
    void click() {
        requireConnection();
        ++clicks_;
    }
    int clicks() const { return clicks_; }
};

class Display : public Component, public Connectable {
    int widthPx_;
    int heightPx_;
    int refreshHz_;
    int brightness_;
public:
    Display(const std::string& name, const std::string& maker, double price, int w, int h, int hz = 60)
        : Component(name, maker, price, 30), widthPx_(w), heightPx_(h), refreshHz_(hz), brightness_(50) {
        if (w <= 0 || h <= 0 || hz <= 0) throw InvalidComponentDataException("Некорректные параметры дисплея");
    }
    std::string interfaceName() const override { return "HDMI"; }
    double megapixels() const { return widthPx_ * heightPx_ / 1.0e6; }
    bool isHd() const { return widthPx_ >= 1920 && heightPx_ >= 1080; }
    void setRefreshRate(int hz) {
        if (hz < 24 || hz > 360) throw InvalidComponentDataException("Частота обновления вне диапазона");
        refreshHz_ = hz;
    }
    void setBrightness(int percent) { brightness_ = std::clamp(percent, 0, 100); }
    int refreshRate() const { return refreshHz_; }
};

class Printer : public Component, public Connectable {
    int paperSheets_;
    int inkPercent_;
    int printedPages_;
public:
    Printer(const std::string& name, const std::string& maker, double price)
        : Component(name, maker, price, 20), paperSheets_(0), inkPercent_(100), printedPages_(0) {}
    std::string interfaceName() const override { return "USB"; }
    void print(int pages) {
        requireConnection();
        if (!isOn()) throw PowerException("Принтер выключен");
        if (pages > paperSheets_) throw ComputerException("В лотке не хватает бумаги");
        if (inkPercent_ < pages / 10 + 1) throw ComputerException("Закончились чернила");
        paperSheets_ -= pages;
        inkPercent_ -= pages / 10 + 1;
        printedPages_ += pages;
    }
    void refillPaper(int sheets) { paperSheets_ += sheets; }
    void refillInk() { inkPercent_ = 100; }
    int printedPages() const { return printedPages_; }
};

// ---------------------------------------------------------------------------
//  Сеть
// ---------------------------------------------------------------------------
class IpAddress {
    std::string text_;
    std::vector<int> octets_;
public:
    static std::vector<int> parse(const std::string& text) {
        std::vector<int> result;
        std::string part;
        for (std::size_t i = 0; i <= text.size(); ++i) {
            if (i == text.size() || text[i] == '.') {
                if (part.empty() || part.size() > 3) return {};
                int value = std::stoi(part);
                if (value > 255) return {};
                result.push_back(value);
                part.clear();
            } else if (std::isdigit(static_cast<unsigned char>(text[i]))) {
                part += text[i];
            } else {
                return {};
            }
        }
        if (result.size() != 4) return {};
        return result;
    }
    static bool isValid(const std::string& text) { return !parse(text).empty(); }
    explicit IpAddress(const std::string& text) : text_(text), octets_(parse(text)) {
        if (octets_.empty()) throw InvalidIpAddressException("Некорректный IP-адрес: " + text);
    }
    const std::string& toString() const { return text_; }
    int octet(int index) const { return octets_.at(static_cast<std::size_t>(index)); }
    bool isPrivate() const {
        return octets_[0] == 10 || (octets_[0] == 192 && octets_[1] == 168) || (octets_[0] == 172 && octets_[1] >= 16 && octets_[1] <= 31);
    }
    bool sameSubnet(const IpAddress& other) const {
        return octets_[0] == other.octets_[0] && octets_[1] == other.octets_[1] && octets_[2] == other.octets_[2];
    }
};

class NetworkCard : public Component, public Connectable {
    std::string macAddress_;
    int speedMbps_;
    bool hasIp_;
    IpAddress ip_;
public:
    NetworkCard(const std::string& name, const std::string& maker, double price, const std::string& mac, int speedMbps)
        : Component(name, maker, price, 4), macAddress_(mac), speedMbps_(speedMbps), hasIp_(false), ip_("0.0.0.0") {
        if (speedMbps <= 0) throw InvalidComponentDataException("Скорость сети должна быть положительной");
    }
    std::string interfaceName() const override { return "Ethernet"; }
    void connect() override {
        if (!hasIp_) throw NetworkUnavailableException("Сетевому адаптеру не назначен IP-адрес");
        Connectable::connect();
    }
    void assignIp(const IpAddress& address) {
        ip_ = address;
        hasIp_ = true;
    }
    const IpAddress& ip() const { return ip_; }
    bool hasIp() const { return hasIp_; }
    int transferSeconds(int megabytes) const { return (megabytes * 8 + speedMbps_ - 1) / speedMbps_; }
    const std::string& mac() const { return macAddress_; }
};

class WifiAdapter : public NetworkCard {
    std::string ssid_;
    int signalPercent_;
public:
    // Перегрузка connect(ssid) скрыла бы connect() базового класса — возвращаем его using-объявлением.
    using NetworkCard::connect;
    WifiAdapter(const std::string& name, const std::string& maker, double price, const std::string& mac)
        : NetworkCard(name, maker, price, mac, 300), signalPercent_(0) {}
    void connect(const std::string& ssid) {
        if (ssid.empty()) throw NetworkUnavailableException("Не указано имя сети");
        ssid_ = ssid;
        connect();
    }
    std::string interfaceName() const override { return "Wi-Fi"; }
    void setSignal(int percent) { signalPercent_ = std::clamp(percent, 0, 100); }
    bool isStable() const { return signalPercent_ >= 40; }
    const std::string& ssid() const { return ssid_; }
};

class Router : public Component {
    IpAddress gateway_;
    int maxClients_;
    std::string ssid_;
    std::vector<IpAddress> leases_;
public:
    Router(const std::string& name, const std::string& maker, double price, const std::string& ssid, int maxClients)
        : Component(name, maker, price, 12), gateway_("192.168.0.1"), maxClients_(maxClients), ssid_(ssid) {}
    IpAddress lease(NetworkCard& card) {
        if (isFull()) throw NetworkUnavailableException("Нет свободных адресов в пуле DHCP");
        IpAddress address("192.168.0." + std::to_string(leases_.size() + 2));
        leases_.push_back(address);
        card.assignIp(address);
        return address;
    }
    bool release(const IpAddress& address) {
        auto it = std::find_if(leases_.begin(), leases_.end(),
            [&address](const IpAddress& a) { return a.toString() == address.toString(); });
        if (it == leases_.end()) return false;
        leases_.erase(it);
        return true;
    }
    int clientCount() const { return static_cast<int>(leases_.size()); }
    bool isFull() const { return static_cast<int>(leases_.size()) >= maxClients_; }
    const IpAddress& gateway() const { return gateway_; }
};

class Battery : public Component {
    int capacityMah_;
    int chargeMah_;
    int cycles_;
public:
    Battery(const std::string& name, const std::string& maker, double price, int capacityMah)
        : Component(name, maker, price, 0), capacityMah_(capacityMah), chargeMah_(capacityMah), cycles_(0) {
        if (capacityMah <= 0) throw InvalidComponentDataException("Ёмкость батареи должна быть положительной");
    }
    void charge(int mah) {
        chargeMah_ = std::min(capacityMah_, chargeMah_ + mah);
        if (chargeMah_ == capacityMah_) ++cycles_;
    }
    void discharge(int mah) {
        if (mah > chargeMah_) {
            chargeMah_ = 0;
            throw PowerException("Батарея разряжена");
        }
        chargeMah_ -= mah;
    }
    int percent() const { return 100 * chargeMah_ / capacityMah_; }
    int healthPercent() const { return std::max(0, 100 - cycles_ / 5); }
    bool isEmpty() const { return chargeMah_ == 0; }
    int runtimeMinutes(int drawMa) const { return drawMa > 0 ? chargeMah_ * 60 / drawMa : 0; }
};

// ============================================================================
//  Программная часть
// ============================================================================
class Permissions {
    bool read_;
    bool write_;
    bool exec_;
public:
    explicit Permissions(const std::string& mask = "rw-")
        : read_(mask.size() > 0 && mask[0] == 'r'), write_(mask.size() > 1 && mask[1] == 'w'), exec_(mask.size() > 2 && mask[2] == 'x') {}
    bool allows(char operation) const {
        switch (operation) {
            case 'r': return read_;
            case 'w': return write_;
            case 'x': return exec_;
            default: return false;
        }
    }
    std::string toString() const { return std::string(read_ ? "r" : "-") + (write_ ? "w" : "-") + (exec_ ? "x" : "-"); }
    void grant(char operation) { setFlag(operation, true); }
    void revoke(char operation) { setFlag(operation, false); }
private:
    void setFlag(char operation, bool value) {
        if (operation == 'r') read_ = value;
        else if (operation == 'w') write_ = value;
        else if (operation == 'x') exec_ = value;
    }
};

class File : public Named {
    int sizeKb_;
    std::string content_;
    Permissions perms_;
    std::string owner_;
public:
    File(const std::string& name, const std::string& owner, const Permissions& perms = Permissions())
        : Named(name), sizeKb_(0), perms_(perms), owner_(owner) {
        if (name.empty()) throw InvalidComponentDataException("Имя файла не может быть пустым");
    }
    void write(const std::string& text) {
        if (!perms_.allows('w')) throw PermissionDeniedException("Запись в файл запрещена: " + name_);
        content_ = text;
        sizeKb_ = static_cast<int>(text.size() / 1024) + 1;
    }
    void append(const std::string& text) { write(content_ + text); }
    std::string read() const {
        if (!perms_.allows('r')) throw PermissionDeniedException("Чтение файла запрещено: " + name_);
        return content_;
    }
    const std::string& peek() const { return content_; }
    void resize(int kb) { sizeKb_ = std::max(0, kb); }
    void chmod(const Permissions& perms) { perms_ = perms; }
    int sizeKb() const { return sizeKb_; }
    const std::string& owner() const { return owner_; }
    bool isEmpty() const { return content_.empty(); }
};

class Directory : public Named {
    std::vector<File> files_;
    std::vector<Directory> subdirs_;
public:
    explicit Directory(const std::string& name) : Named(name) {}
    void addFile(const File& file) {
        for (const auto& f : files_)
            if (f.name() == file.name()) throw ComputerException("Файл уже существует: " + file.name());
        files_.push_back(file);
    }
    File& findFile(const std::string& fileName) {
        for (auto& f : files_) if (f.name() == fileName) return f;
        for (auto& d : subdirs_) {
            try { return d.findFile(fileName); } catch (const FileNotFoundException&) {}
        }
        throw FileNotFoundException("Файл не найден: " + fileName);
    }
    bool removeFile(const std::string& fileName) {
        auto it = std::remove_if(files_.begin(), files_.end(), [&fileName](const File& f) { return f.name() == fileName; });
        if (it == files_.end()) return false;
        files_.erase(it, files_.end());
        return true;
    }
    void addSubdirectory(const Directory& dir) { subdirs_.push_back(dir); }
    int fileCount() const {
        int total = static_cast<int>(files_.size());
        for (const auto& d : subdirs_) total += d.fileCount();
        return total;
    }
    int totalSizeKb() const {
        int total = 0;
        for (const auto& f : files_) total += f.sizeKb();
        for (const auto& d : subdirs_) total += d.totalSizeKb();
        return total;
    }
    const std::vector<File>& files() const { return files_; }
    const std::vector<Directory>& subdirectories() const { return subdirs_; }
};

class FileSystem {
    int capacityKb_;
    std::string label_;
    Directory root_;
public:
    explicit FileSystem(int capacityKb, const std::string& label = "System")
        : capacityKb_(capacityKb), label_(label), root_("/") {
        if (capacityKb <= 0) throw InvalidComponentDataException("Размер файловой системы должен быть положительным");
    }
    void createFile(const std::string& name, const std::string& owner, int sizeKb) {
        if (usedKb() + sizeKb > capacityKb_) throw DiskFullException("Недостаточно места в файловой системе");
        File file(name, owner);
        file.resize(sizeKb);
        root_.addFile(file);
    }
    void makeDirectory(const std::string& name) { root_.addSubdirectory(Directory(name)); }
    File& find(const std::string& name) { return root_.findFile(name); }
    void remove(const std::string& name) {
        if (!root_.removeFile(name)) throw FileNotFoundException("Файл не найден: " + name);
    }
    void format() { root_ = Directory("/"); }
    int usedKb() const { return root_.totalSizeKb(); }
    int freeKb() const { return capacityKb_ - usedKb(); }
    const Directory& root() const { return root_; }
};

class Process : public Named {
    std::string state_;
    int memoryMb_;
    int priority_;
public:
    Process(const std::string& name, int memoryMb, int priority = 20)
        : Named(name), state_("new"), memoryMb_(memoryMb), priority_(priority) {}
    void start() { state_ = "running"; }
    void suspend() {
        if (state_ != "running") throw ComputerException("Приостановить можно только работающий процесс");
        state_ = "suspended";
    }
    void resume() {
        if (state_ != "suspended") throw ComputerException("Возобновить можно только приостановленный процесс");
        state_ = "running";
    }
    void terminate() { state_ = "terminated"; }
    void setPriority(int value) {
        if (value < 0 || value > 39) throw ComputerException("Приоритет должен быть от 0 до 39");
        priority_ = value;
    }
    bool isRunning() const { return state_ == "running"; }
    int memoryMb() const { return memoryMb_; }
    const std::string& state() const { return state_; }
};

class MemoryManager {
    int totalMb_;
    int usedMb_;
public:
    explicit MemoryManager(int totalMb) : totalMb_(totalMb), usedMb_(0) {}
    bool canAllocate(int mb) const { return mb >= 0 && usedMb_ + mb <= totalMb_; }
    void allocate(int mb) {
        if (!canAllocate(mb)) throw InsufficientMemoryException("Недостаточно оперативной памяти");
        usedMb_ += mb;
    }
    void release(int mb) { usedMb_ = std::max(0, usedMb_ - mb); }
    int freeMb() const { return totalMb_ - usedMb_; }
    double usagePercent() const { return totalMb_ > 0 ? 100.0 * usedMb_ / totalMb_ : 0.0; }
};

class ProcessManager {
    std::vector<Process> processes_;
    int maxProcesses_;
public:
    explicit ProcessManager(int maxProcesses = 256) : maxProcesses_(maxProcesses) {}
    int spawn(const std::string& name, int memoryMb, MemoryManager& memory) {
        if (static_cast<int>(processes_.size()) >= maxProcesses_) throw ComputerException("Достигнут лимит процессов");
        memory.allocate(memoryMb);
        Process process(name, memoryMb);
        process.start();
        processes_.push_back(process);
        return process.id();
    }
    Process& find(int pid) {
        for (auto& p : processes_) if (p.id() == pid) return p;
        throw ProcessNotFoundException("Процесс не найден: " + std::to_string(pid));
    }
    void kill(int pid, MemoryManager& memory) {
        Process& process = find(pid);
        memory.release(process.memoryMb());
        process.terminate();
        processes_.erase(std::remove_if(processes_.begin(), processes_.end(),
            [pid](const Process& p) { return p.id() == pid; }), processes_.end());
    }
    void killAll(MemoryManager& memory) {
        for (auto& p : processes_) memory.release(p.memoryMb());
        processes_.clear();
    }
    int count() const { return static_cast<int>(processes_.size()); }
    int totalMemoryMb() const {
        return std::accumulate(processes_.begin(), processes_.end(), 0, [](int sum, const Process& p) { return sum + p.memoryMb(); });
    }
    std::vector<std::string> names() const {
        std::vector<std::string> result;
        for (const auto& p : processes_) result.push_back(p.name());
        return result;
    }
};

class Scheduler {
    int quantumMs_;
    int ticks_;
    std::vector<int> queue_;
public:
    explicit Scheduler(int quantumMs = 10) : quantumMs_(quantumMs), ticks_(0) {}
    void enqueue(const Process& process) { queue_.push_back(process.id()); }
    int next() {
        if (queue_.empty()) throw ProcessNotFoundException("Очередь планировщика пуста");
        int pid = queue_.front();
        queue_.erase(queue_.begin());
        queue_.push_back(pid);
        ++ticks_;
        return pid;
    }
    bool remove(int pid) {
        auto it = std::find(queue_.begin(), queue_.end(), pid);
        if (it == queue_.end()) return false;
        queue_.erase(it);
        return true;
    }
    void setQuantum(int ms) {
        if (ms <= 0) throw ComputerException("Квант времени должен быть положительным");
        quantumMs_ = ms;
    }
    int waitEstimateMs() const { return static_cast<int>(queue_.size()) * quantumMs_; }
    int size() const { return static_cast<int>(queue_.size()); }
    int ticks() const { return ticks_; }
};

class Driver : public Named {
    std::string version_;
    std::string deviceName_;
    bool installed_;
public:
    Driver(const std::string& name, const std::string& version)
        : Named(name), version_(version), installed_(false) {}
    void install(const Component& device) {
        deviceName_ = device.name();
        installed_ = true;
    }
    void uninstall() {
        deviceName_.clear();
        installed_ = false;
    }
    void update(const std::string& version) {
        if (!installed_) throw ComputerException("Драйвер не установлен");
        version_ = version;
    }
    bool matches(const Component& device) const { return installed_ && deviceName_ == device.name(); }
    bool isInstalled() const { return installed_; }
    const std::string& version() const { return version_; }
};

class Application : public Named {
protected:
    std::string version_;
    int sizeKb_;
    int requiredMb_;
    bool installed_;
public:
    Application(const std::string& name, const std::string& version, int sizeKb, int requiredMb)
        : Named(name), version_(version), sizeKb_(sizeKb), requiredMb_(requiredMb), installed_(false) {}
    void install(FileSystem& fs) {
        if (installed_) throw ComputerException("Приложение уже установлено: " + name_);
        fs.createFile(name_ + ".bin", "system", sizeKb_);
        installed_ = true;
    }
    void uninstall(FileSystem& fs) {
        if (!installed_) throw ComputerException("Приложение не установлено: " + name_);
        fs.remove(name_ + ".bin");
        installed_ = false;
    }
    virtual int run(ProcessManager& processes, MemoryManager& memory) {
        if (!installed_) throw ComputerException("Приложение не установлено: " + name_);
        return processes.spawn(name_, requiredMb_, memory);
    }
    bool isInstalled() const { return installed_; }
    int sizeKb() const { return sizeKb_; }
    const std::string& version() const { return version_; }
};

class Game : public Application {
    int requiredVramMb_;
    int targetFps_;
    int launches_;
public:
    Game(const std::string& name, const std::string& version, int sizeKb, int requiredMb, int vramMb, int fps)
        : Application(name, version, sizeKb, requiredMb), requiredVramMb_(vramMb), targetFps_(fps), launches_(0) {}
    bool canRunOn(const Gpu& gpu) const { return gpu.meetsVram(requiredVramMb_); }
    int estimatedFps(const Gpu& gpu) const { return std::min(targetFps_, gpu.renderFps(2000)); }
    int run(ProcessManager& processes, MemoryManager& memory) override {
        int pid = Application::run(processes, memory);
        ++launches_;
        return pid;
    }
    int launches() const { return launches_; }
};

class Browser : public Application {
    std::vector<std::string> tabs_;
    std::vector<std::string> history_;
public:
    Browser(const std::string& name, const std::string& version, int sizeKb)
        : Application(name, version, sizeKb, 300) {}
    void openTab(const std::string& url) {
        if (url.empty()) throw ComputerException("Пустой адрес вкладки");
        tabs_.push_back(url);
        history_.push_back(url);
    }
    void closeTab(int index) {
        if (index < 0 || index >= static_cast<int>(tabs_.size())) throw ComputerException("Нет вкладки с таким номером");
        tabs_.erase(tabs_.begin() + index);
    }
    int run(ProcessManager& processes, MemoryManager& memory) override {
        int pid = Application::run(processes, memory);
        openTab("about:blank");
        return pid;
    }
    void clearHistory() { history_.clear(); }
    int tabCount() const { return static_cast<int>(tabs_.size()); }
    int historyCount() const { return static_cast<int>(history_.size()); }
};

class Antivirus : public Application {
    std::vector<std::string> signatures_;
    int scannedFiles_;
public:
    Antivirus(const std::string& name, const std::string& version, int sizeKb)
        : Application(name, version, sizeKb, 150), scannedFiles_(0) {}
    void addSignature(const std::string& signature) {
        if (signature.empty()) throw InvalidComponentDataException("Пустая сигнатура");
        signatures_.push_back(signature);
    }
    void updateSignatures(const std::vector<std::string>& fresh) {
        for (const auto& s : fresh) addSignature(s);
    }
    std::vector<std::string> scan(const Directory& directory) {
        std::vector<std::string> infected;
        for (const auto& file : directory.files()) {
            ++scannedFiles_;
            for (const auto& sig : signatures_) {
                if (file.peek().find(sig) != std::string::npos) { infected.push_back(file.name()); break; }
            }
        }
        for (const auto& sub : directory.subdirectories()) {
            auto nested = scan(sub);
            infected.insert(infected.end(), nested.begin(), nested.end());
        }
        return infected;
    }
    int signatureCount() const { return static_cast<int>(signatures_.size()); }
    int scannedFiles() const { return scannedFiles_; }
};

class Bios : public Named {
    std::string version_;
    std::vector<std::string> bootOrder_;
    bool secureBoot_;
public:
    explicit Bios(const std::string& version)
        : Named("BIOS"), version_(version), bootOrder_{"SSD", "HDD", "USB"}, secureBoot_(false) {}
    bool post(const Motherboard& board, const PowerSupply& psu) const {
        if (!board.hasProcessor()) throw BootFailureException("POST: не найден процессор");
        if (board.totalRamMb() == 0) throw BootFailureException("POST: не найдена оперативная память");
        if (psu.wattage() <= 0) throw BootFailureException("POST: нет питания");
        return true;
    }
    void setBootOrder(const std::vector<std::string>& order) {
        if (order.empty()) throw InvalidComponentDataException("Порядок загрузки не может быть пустым");
        bootOrder_ = order;
    }
    void enableSecureBoot() { secureBoot_ = true; }
    void flash(const std::string& version) { version_ = version; }
    const std::string& firstBootDevice() const { return bootOrder_.front(); }
    const std::string& version() const { return version_; }
    bool isSecureBoot() const { return secureBoot_; }
};

class User : public Named {
    std::size_t passwordHash_;
    bool admin_;
    std::string homeDir_;
public:
    static bool isStrongPassword(const std::string& password) {
        bool digit = false, upper = false;
        for (char c : password) {
            digit = digit || std::isdigit(static_cast<unsigned char>(c));
            upper = upper || std::isupper(static_cast<unsigned char>(c));
        }
        return password.size() >= 8 && digit && upper;
    }
    User(const std::string& name, const std::string& password, bool admin = false)
        : Named(name), passwordHash_(0), admin_(admin), homeDir_("/home/" + name) {
        setPassword(password);
    }
    void setPassword(const std::string& password) {
        if (!isStrongPassword(password)) throw AuthenticationException("Пароль слишком слабый");
        passwordHash_ = std::hash<std::string>{}(password);
    }
    bool checkPassword(const std::string& password) const { return passwordHash_ == std::hash<std::string>{}(password); }
    void promote() { admin_ = true; }
    bool isAdmin() const { return admin_; }
    const std::string& homeDir() const { return homeDir_; }
};

class Session {
    User user_;
    int loginMinute_;
    bool active_;
public:
    Session(const User& user, const std::string& password, int nowMinute)
        : user_(user), loginMinute_(nowMinute), active_(false) {
        if (!user.checkPassword(password)) throw AuthenticationException("Неверный пароль");
        active_ = true;
    }
    void logout() { active_ = false; }
    int duration(int nowMinute) const { return active_ ? nowMinute - loginMinute_ : 0; }
    bool isActive() const { return active_; }
    bool isAdmin() const { return user_.isAdmin(); }
    const std::string& userName() const { return user_.name(); }
};

class LogEntry {
    std::string level_;
    std::string message_;
    int timestamp_;
public:
    LogEntry(const std::string& level, const std::string& message, int timestamp)
        : level_(level), message_(message), timestamp_(timestamp) {}
    std::string format() const { return "[" + std::to_string(timestamp_) + "] " + level_ + ": " + message_; }
    bool isError() const { return level_ == "ERROR"; }
    const std::string& level() const { return level_; }
};

class Logger {
    std::vector<LogEntry> entries_;
    std::size_t maxEntries_;
    int clock_;
public:
    explicit Logger(std::size_t maxEntries = 1000) : maxEntries_(maxEntries), clock_(0) {}
    void log(const std::string& level, const std::string& message) {
        if (entries_.size() >= maxEntries_) entries_.erase(entries_.begin());
        entries_.emplace_back(level, message, ++clock_);
    }
    std::vector<LogEntry> filter(const std::string& level) const {
        std::vector<LogEntry> result;
        for (const auto& e : entries_) if (e.level() == level) result.push_back(e);
        return result;
    }
    const LogEntry& last() const {
        if (entries_.empty()) throw ComputerException("Журнал пуст");
        return entries_.back();
    }
    void clear() { entries_.clear(); }
    int count() const { return static_cast<int>(entries_.size()); }
    int errorCount() const {
        return static_cast<int>(std::count_if(entries_.begin(), entries_.end(), [](const LogEntry& e) { return e.isError(); }));
    }
};

class Settings {
    std::map<std::string, std::string> values_;
    bool locked_;
public:
    Settings() : locked_(false) { values_["language"] = "ru"; }
    void set(const std::string& key, const std::string& value) {
        if (locked_) throw PermissionDeniedException("Настройки заблокированы");
        values_[key] = value;
    }
    const std::string& get(const std::string& key) const {
        auto it = values_.find(key);
        if (it == values_.end()) throw ComputerException("Нет настройки: " + key);
        return it->second;
    }
    bool has(const std::string& key) const { return values_.count(key) > 0; }
    bool remove(const std::string& key) { return values_.erase(key) > 0; }
    void lock() { locked_ = true; }
    void unlock() { locked_ = false; }
    int count() const { return static_cast<int>(values_.size()); }
};

class Clipboard {
    std::string content_;
    std::vector<std::string> history_;
public:
    void copy(const std::string& text) {
        content_ = text;
        history_.push_back(text);
    }
    const std::string& paste() const {
        if (content_.empty()) throw ComputerException("Буфер обмена пуст");
        return content_;
    }
    void clear() { content_.clear(); }
    int historySize() const { return static_cast<int>(history_.size()); }
};

class OperatingSystem : public Named {
    std::string version_;
    MemoryManager memory_;
    ProcessManager processes_;
    FileSystem fs_;
    Logger logger_;
    Scheduler scheduler_;
    Settings settings_;
    Clipboard clipboard_;
    std::vector<User> users_;
    std::vector<Driver> drivers_;
    bool booted_;
public:
    OperatingSystem(const std::string& name, const std::string& version, int ramMb, int diskKb)
        : Named(name), version_(version), memory_(ramMb), processes_(), fs_(diskKb), booted_(false) {}
    void boot(const Bios& bios) {
        if (booted_) throw ComputerException("Операционная система уже загружена");
        memory_.allocate(std::min(512, memory_.freeMb()));
        booted_ = true;
        logger_.log("INFO", "Загрузка с " + bios.firstBootDevice());
    }
    void shutdown() {
        requireBooted();
        processes_.killAll(memory_);
        memory_.release(512);
        booted_ = false;
        logger_.log("INFO", "Завершение работы");
    }
    void addUser(const User& user) {
        for (const auto& u : users_)
            if (u.name() == user.name()) throw ComputerException("Пользователь уже существует: " + user.name());
        users_.push_back(user);
    }
    Session login(const std::string& userName, const std::string& password, int nowMinute) {
        requireBooted();
        for (const auto& u : users_) {
            if (u.name() == userName) return Session(u, password, nowMinute);
        }
        throw AuthenticationException("Пользователь не найден: " + userName);
    }
    void installApplication(Application& app) {
        requireBooted();
        app.install(fs_);
        logger_.log("INFO", "Установлено: " + app.name());
    }
    int launch(Application& app) {
        requireBooted();
        int pid = app.run(processes_, memory_);
        scheduler_.enqueue(processes_.find(pid));
        logger_.log("INFO", "Запущено: " + app.name());
        return pid;
    }
    void closeProcess(int pid) {
        requireBooted();
        processes_.kill(pid, memory_);
        scheduler_.remove(pid);
    }
    void installDriver(Driver& driver, const Component& device) {
        requireBooted();
        driver.install(device);
        drivers_.push_back(driver);
    }
    void requireBooted() const {
        if (!booted_) throw ComputerException("Операционная система не загружена");
    }
    bool isBooted() const { return booted_; }
    int driverCount() const { return static_cast<int>(drivers_.size()); }
    const std::string& version() const { return version_; }
    MemoryManager& memory() { return memory_; }
    ProcessManager& processes() { return processes_; }
    FileSystem& fileSystem() { return fs_; }
    Logger& logger() { return logger_; }
    Scheduler& scheduler() { return scheduler_; }
    Settings& settings() { return settings_; }
    Clipboard& clipboard() { return clipboard_; }
};

// ============================================================================
//  Компьютеры и обслуживание
// ============================================================================
class Warranty {
    int months_;
    int usedMonths_;
public:
    explicit Warranty(int months) : months_(months), usedMonths_(0) {}
    bool isValid() const { return usedMonths_ < months_; }
    int remainingMonths() const { return std::max(0, months_ - usedMonths_); }
    void extend(int months) { months_ += months; }
    void consume(int months) { usedMonths_ += months; }
};

class BenchmarkResult {
    int cpuScore_;
    int gpuScore_;
    int memoryScore_;
    int storageScore_;
public:
    BenchmarkResult(int cpu, int gpu, int memory, int storage)
        : cpuScore_(cpu), gpuScore_(gpu), memoryScore_(memory), storageScore_(storage) {}
    int total() const {
        return static_cast<int>(cpuScore_ * 0.4 + gpuScore_ * 0.3 + memoryScore_ * 0.15 + storageScore_ * 0.15);
    }
    std::string rating() const {
        int t = total();
        return t >= 2000 ? "высокий" : (t >= 800 ? "средний" : "низкий");
    }
    bool isBetterThan(const BenchmarkResult& other) const { return total() > other.total(); }
    std::string summary() const { return "Итог: " + std::to_string(total()) + " (" + rating() + ")"; }
};

class Computer : public Named, public Powered {
protected:
    std::shared_ptr<Motherboard> board_;
    std::shared_ptr<PowerSupply> psu_;
    std::shared_ptr<ComputerCase> case_;
    std::shared_ptr<Gpu> gpu_;
    std::shared_ptr<CoolingSystem> cooling_;
    std::vector<std::shared_ptr<StorageDevice>> storage_;
    std::shared_ptr<OperatingSystem> os_;
    Bios bios_;
    Warranty warranty_;
    int bootCount_;
public:
    explicit Computer(const std::string& name, int warrantyMonths = 24)
        : Named(name), Powered(0), bios_("1.0.0"), warranty_(warrantyMonths), bootCount_(0) {}
    void setMotherboard(std::shared_ptr<Motherboard> board) {
        if (!board) throw InvalidComponentDataException("Материнская плата не задана");
        if (case_ && !case_->supportsBoard(*board)) throw IncompatibleComponentException("Плата не помещается в корпус");
        board_ = board;
    }
    void setCase(std::shared_ptr<ComputerCase> pcCase) {
        if (!pcCase) throw InvalidComponentDataException("Корпус не задан");
        if (board_ && !pcCase->supportsBoard(*board_)) throw IncompatibleComponentException("Плата не помещается в корпус");
        case_ = pcCase;
    }
    void setPowerSupply(std::shared_ptr<PowerSupply> psu) {
        if (!psu) throw InvalidComponentDataException("Блок питания не задан");
        psu_ = psu;
    }
    void installGpu(std::shared_ptr<Gpu> gpu) {
        if (!gpu) throw InvalidComponentDataException("Видеокарта не задана");
        gpu_ = gpu;
    }
    void setCooling(std::shared_ptr<CoolingSystem> cooling) { cooling_ = cooling; }
    void addStorage(std::shared_ptr<StorageDevice> device) {
        if (!device) throw InvalidComponentDataException("Накопитель не задан");
        storage_.push_back(device);
    }
    int totalStorageGb() const {
        return std::accumulate(storage_.begin(), storage_.end(), 0, [](int sum, const std::shared_ptr<StorageDevice>& d) { return sum + d->capacityGb(); });
    }
    int totalPowerDraw() const {
        int total = 0;
        if (board_) {
            total += board_->ratedWatts();
            if (board_->hasProcessor()) total += board_->processor().ratedWatts();
        }
        if (gpu_) total += gpu_->ratedWatts();
        if (cooling_) total += cooling_->ratedWatts();
        for (const auto& d : storage_) total += d->ratedWatts();
        return total;
    }
    std::vector<std::string> missingParts() const {
        std::vector<std::string> missing;
        if (!board_) missing.push_back("материнская плата");
        else if (!board_->hasProcessor()) missing.push_back("процессор");
        if (board_ && board_->totalRamMb() == 0) missing.push_back("оперативная память");
        if (!psu_) missing.push_back("блок питания");
        if (!case_) missing.push_back("корпус");
        if (storage_.empty()) missing.push_back("накопитель");
        return missing;
    }
    void validateBuild() const {
        auto missing = missingParts();
        if (!missing.empty()) throw BootFailureException("Не хватает комплектующих: " + missing.front());
        if (!psu_->canSupply(totalPowerDraw())) throw PowerException("Мощности блока питания недостаточно");
    }
    void powerOn() override {
        validateBuild();
        bios_.post(*board_, *psu_);
        Powered::powerOn();
        ++bootCount_;
        if (os_) os_->boot(bios_);
    }
    void powerOff() override {
        if (os_ && os_->isBooted()) os_->shutdown();
        Powered::powerOff();
    }
    OperatingSystem& installOperatingSystem(const std::string& osName, const std::string& version) {
        validateBuild();
        int diskKb = std::min(storage_.front()->capacityGb(), 1000) * 1024 * 1024;
        os_ = std::make_shared<OperatingSystem>(osName, version, board_->totalRamMb(), diskKb);
        return *os_;
    }
    OperatingSystem& os() {
        if (!os_) throw ComputerException("Операционная система не установлена");
        return *os_;
    }
    BenchmarkResult runBenchmark() const {
        validateBuild();
        int cpu = board_->processor().benchmarkScore() / 10;
        int gpu = gpu_ ? gpu_->renderFps(1000) * 10 : 0;
        int memory = board_->totalRamMb() / 10;
        int disk = storage_.front()->readSpeedMbps();
        return BenchmarkResult(cpu, gpu, memory, disk);
    }
    double estimatedPrice() const {
        double total = 0;
        if (board_) total += board_->price() + (board_->hasProcessor() ? board_->processor().price() : 0.0);
        if (psu_) total += psu_->price();
        if (gpu_) total += gpu_->price();
        for (const auto& d : storage_) total += d->price();
        return total;
    }
    void cool() {
        if (!board_ || !cooling_ || !board_->hasProcessor()) return;
        cooling_->cool(board_->processor());
        if (gpu_) cooling_->cool(*gpu_);
    }
    std::string describe() const override { return name_ + " [" + status() + "]"; }
    int bootCount() const { return bootCount_; }
    bool hasOs() const { return os_ != nullptr; }
    Warranty& warranty() { return warranty_; }
    std::shared_ptr<Gpu> gpu() const { return gpu_; }
    std::shared_ptr<Motherboard> board() const { return board_; }
};

class Desktop : public Computer {
    std::vector<std::shared_ptr<Display>> displays_;
    std::shared_ptr<Keyboard> keyboard_;
    std::shared_ptr<Mouse> mouse_;
    int extraFans_;
public:
    explicit Desktop(const std::string& name) : Computer(name), extraFans_(0) {}
    void attachDisplay(std::shared_ptr<Display> display) {
        if (!display) throw InvalidComponentDataException("Дисплей не задан");
        if (case_) case_->plugDevice();
        display->connect();
        displays_.push_back(display);
    }
    void attachKeyboard(std::shared_ptr<Keyboard> keyboard) {
        keyboard->connect();
        keyboard_ = keyboard;
    }
    void attachMouse(std::shared_ptr<Mouse> mouse) {
        mouse->connect();
        mouse_ = mouse;
    }
    void upgradeRam(const RamModule& module) {
        if (!board_) throw BootFailureException("Нет материнской платы");
        board_->installRam(module);
    }
    void addExtraFan() { ++extraFans_; }
    bool hasInputDevices() const { return keyboard_ && mouse_; }
    int displayCount() const { return static_cast<int>(displays_.size()); }
    double totalMegapixels() const {
        double total = 0;
        for (const auto& d : displays_) total += d->megapixels();
        return total;
    }
};

class Laptop : public Computer {
    Battery battery_;
    Display screen_;
    bool lidOpen_;
    bool chargerPlugged_;
public:
    Laptop(const std::string& name, const Battery& battery, const Display& screen)
        : Computer(name), battery_(battery), screen_(screen), lidOpen_(true), chargerPlugged_(false) {}
    void powerOn() override {
        if (!lidOpen_) throw PowerException("Крышка закрыта");
        if (!chargerPlugged_ && battery_.isEmpty()) throw PowerException("Батарея разряжена");
        Computer::powerOn();
    }
    void closeLid() {
        lidOpen_ = false;
        if (isOn()) powerOff();
    }
    void openLid() { lidOpen_ = true; }
    void plugCharger(bool plugged) { chargerPlugged_ = plugged; }
    void work(int minutes) {
        if (!isOn()) throw PowerException("Ноутбук выключен");
        if (!chargerPlugged_) battery_.discharge(minutes * 25);
    }
    void charge(int minutes) { battery_.charge(minutes * 50); }
    int batteryPercent() const { return battery_.percent(); }
    int runtimeMinutes() const { return battery_.runtimeMinutes(1500); }
    bool isLidOpen() const { return lidOpen_; }
};

class Server : public Computer {
    int uptimeHours_;
    std::vector<std::string> services_;
public:
    explicit Server(const std::string& name) : Computer(name, 60), uptimeHours_(0) {}
    void startService(const std::string& service) {
        if (!isOn()) throw PowerException("Сервер выключен");
        if (isServiceRunning(service)) throw ComputerException("Служба уже запущена: " + service);
        services_.push_back(service);
    }
    bool stopService(const std::string& service) {
        auto it = std::find(services_.begin(), services_.end(), service);
        if (it == services_.end()) return false;
        services_.erase(it);
        return true;
    }
    bool isServiceRunning(const std::string& service) const {
        return std::find(services_.begin(), services_.end(), service) != services_.end();
    }
    void addUptime(int hours) { if (hours > 0) uptimeHours_ += hours; }
    void restart() {
        powerOff();
        powerOn();
        uptimeHours_ = 0;
        services_.clear();
    }
    int uptimeHours() const { return uptimeHours_; }
    int serviceCount() const { return static_cast<int>(services_.size()); }
};

// Закрытое наследование: интерфейс Server скрыт, нужное «открывается» using-объявлениями.
class SecureServer : private Server {
    bool firewallEnabled_;
    std::vector<std::string> blockedIps_;
public:
    using Server::name;
    using Server::isOn;
    using Server::powerOn;
    using Server::powerOff;
    using Server::startService;
    using Server::uptimeHours;
    explicit SecureServer(const std::string& name) : Server(name), firewallEnabled_(false) {}
    void enableFirewall() { firewallEnabled_ = true; }
    void blockIp(const IpAddress& address) { blockedIps_.push_back(address.toString()); }
    bool isBlocked(const IpAddress& address) const {
        return std::find(blockedIps_.begin(), blockedIps_.end(), address.toString()) != blockedIps_.end();
    }
    bool requestAccess(const IpAddress& address) {
        if (!isOn()) throw PowerException("Сервер выключен");
        if (firewallEnabled_ && isBlocked(address)) throw PermissionDeniedException("Адрес заблокирован: " + address.toString());
        return true;
    }
    int blockedCount() const { return static_cast<int>(blockedIps_.size()); }
};

// Защищённое наследование: за пределами класса GamingPc не является Computer.
class GamingPc : protected Desktop {
    std::string rgbMode_;
    double overclockPercent_;
public:
    using Desktop::name;
    using Desktop::isOn;
    using Desktop::powerOn;
    using Desktop::powerOff;
    using Desktop::attachDisplay;
    explicit GamingPc(const std::string& name) : Desktop(name), rgbMode_("off"), overclockPercent_(0.0) {}
    void applyOverclock(double percent) {
        if (!board_) throw BootFailureException("Нет материнской платы");
        board_->processor().overclock(percent);
        overclockPercent_ = percent;
    }
    void setRgb(const std::string& mode) { rgbMode_ = mode; }
    bool canPlay(const Game& game) const { return gpu_ && game.canRunOn(*gpu_); }
    int expectedFps(const Game& game) const {
        if (!gpu_) throw ComputerException("Видеокарта не установлена");
        return game.estimatedFps(*gpu_);
    }
    int benchmarkScore() const { return runBenchmark().total(); }
    double overclock() const { return overclockPercent_; }
    // Сборка использует защищённые методы базового класса Desktop/Computer.
    void assemble(std::shared_ptr<Motherboard> b, std::shared_ptr<PowerSupply> p, std::shared_ptr<ComputerCase> c,
                  std::shared_ptr<Gpu> g, std::shared_ptr<StorageDevice> s) {
        setMotherboard(b);
        setCase(c);
        setPowerSupply(p);
        installGpu(g);
        addStorage(s);
    }
};

class Technician;
class RepairTicket : public Named {
    std::string status_;
    double costEstimate_;
    std::string assignedTo_;
public:
    explicit RepairTicket(const std::string& description)
        : Named(description), status_("open"), costEstimate_(0.0) {}
    inline void assign(const Technician& technician);
    void estimate(const Component& broken) { costEstimate_ = broken.price() * 0.15 + 50.0; }
    void close() {
        if (assignedTo_.empty()) throw ComputerException("Заявка не назначена мастеру");
        status_ = "closed";
    }
    bool isOpen() const { return status_ == "open"; }
    double cost() const { return costEstimate_; }
    const std::string& assignedTo() const { return assignedTo_; }
};

class Technician : public Named {
    std::string specialization_;
    int closedTickets_;
public:
    Technician(const std::string& name, const std::string& specialization)
        : Named(name), specialization_(specialization), closedTickets_(0) {}
    std::vector<std::string> diagnose(const Computer& pc) const { return pc.missingParts(); }
    void repair(RepairTicket& ticket, const Component& broken) {
        ticket.assign(*this);
        ticket.estimate(broken);
        ticket.close();
        ++closedTickets_;
    }
    int closedTickets() const { return closedTickets_; }
    const std::string& specialization() const { return specialization_; }
};

inline void RepairTicket::assign(const Technician& technician) { assignedTo_ = technician.name(); }
