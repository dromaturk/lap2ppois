#include "computer.h"

#include <cassert>
#include <iostream>
#include <type_traits>

template <class Ex, class F>
bool throwsEx(F f) {
    try { f(); } catch (const Ex&) { return true; } catch (...) { return false; }
    return false;
}

std::shared_ptr<Motherboard> makeBoard() {
    auto board = std::make_shared<Motherboard>("B550", "MSI", 150, "AM4", "DDR4", "ATX", 4);
    board->installProcessor(std::make_shared<Processor>("Ryzen 5 5600X", "AMD", 200, 6, 3.7, "AM4"));
    board->installRam(RamModule("Vengeance", "Corsair", 60, 16384, 3200, "DDR4"));
    return board;
}

std::shared_ptr<Gpu> makeGpu() { return std::make_shared<Gpu>("RTX 3060", "NVIDIA", 350, 12288, 3584, 1800); }

int main() {
    // --- Виртуальное наследование: единственный Identifiable в ромбе.
    Processor cpu1("A", "AMD", 100, 4, 3.0, "AM4");
    Processor cpu2("B", "AMD", 100, 4, 3.0, "AM4");
    assert(cpu1.id() != cpu2.id());
    static_assert(std::is_convertible<Processor*, Identifiable*>::value, "Identifiable должен быть доступен");

    // --- Совместимость комплектующих и исключения.
    Motherboard board("B550", "MSI", 150, "AM4", "DDR4", "ATX", 1);
    assert(throwsEx<IncompatibleComponentException>([&] { board.installProcessor(std::make_shared<Processor>("X", "Intel", 300, 8, 4.0, "LGA1700")); }));
    assert(throwsEx<IncompatibleComponentException>([&] { board.installRam(RamModule("R", "K", 50, 8192, 5200, "DDR5")); }));
    board.installRam(RamModule("R", "K", 50, 8192, 3200, "DDR4"));
    assert(throwsEx<IncompatibleComponentException>([&] { board.installRam(RamModule("R2", "K", 50, 8192, 3200, "DDR4")); }));
    assert(board.freeSlots() == 0 && board.totalRamMb() == 8192);
    assert(throwsEx<InvalidComponentDataException>([] { Processor p("Bad", "X", 1, 0, 1.0, "AM4"); }));
    assert(throwsEx<BootFailureException>([&] { board.processor(); }));

    // --- Температура и охлаждение.
    Processor hot("Hot", "AMD", 100, 8, 4.0, "AM4");
    assert(throwsEx<OverheatException>([&] { hot.setTemperature(150); }));
    CoolingSystem cooler("Fan", "Noctua", 80, 1500, false);
    hot.coolDown(0);
    cooler.cool(hot);
    assert(hot.temperature() < 150);
    assert(throwsEx<OverheatException>([&] { hot.overclock(50); }));

    // --- Накопители: виртуальные методы.
    std::shared_ptr<StorageDevice> ssd = std::make_shared<Ssd>("970", "Samsung", 100, 500, 3500);
    std::shared_ptr<StorageDevice> hdd = std::make_shared<Hdd>("Barracuda", "Seagate", 50, 1000, 7200);
    assert(ssd->readSpeedMbps() == 3500 && hdd->readSpeedMbps() == 144);
    ssd->write(100);
    assert(ssd->freeGb() == 400);
    assert(throwsEx<DiskFullException>([&] { ssd->write(1000); }));
    assert(static_cast<Ssd&>(*ssd).wear() > 0.0);

    // --- Периферия: множественное наследование и using для status().
    Keyboard kb("K70", "Corsair", 120, false);
    assert(throwsEx<DeviceNotConnectedException>([&] { kb.press('a'); }));
    kb.connect();
    for (char c : std::string("hi all")) kb.press(c);
    assert(kb.wordCount() == 2 && kb.statusFull() == "включено, подключено");
    assert(kb.status() == "включено");
    assert(kb.Connectable::status() == "подключено");
    Mouse mouse("G502", "Logitech", 60, true);
    assert(throwsEx<InvalidComponentDataException>([&] { mouse.setDpi(5); }));
    Display display("U2720", "Dell", 500, 3840, 2160);
    assert(display.isHd() && display.interfaceName() == "HDMI");
    Printer printer("LaserJet", "HP", 150);
    assert(throwsEx<DeviceNotConnectedException>([&] { printer.print(1); }));

    // --- Сеть.
    assert(IpAddress::isValid("192.168.1.10") && !IpAddress::isValid("300.1.1.1") && !IpAddress::isValid("1.2.3"));
    assert(throwsEx<InvalidIpAddressException>([] { IpAddress bad("abc"); }));
    Router router("AX3000", "TP-Link", 90, "HomeNet", 1);
    WifiAdapter wifi("AX200", "Intel", 30, "AA:BB:CC:DD:EE:FF");
    assert(throwsEx<NetworkUnavailableException>([&] { wifi.connect(); }));
    IpAddress leased = router.lease(wifi);
    assert(leased.isPrivate() && leased.sameSubnet(router.gateway()));
    wifi.connect("HomeNet");           // перегрузка + using NetworkCard::connect
    assert(wifi.isConnected() && wifi.ssid() == "HomeNet");
    NetworkCard other("Eth", "Realtek", 10, "11:22:33:44:55:66", 1000);
    assert(throwsEx<NetworkUnavailableException>([&] { router.lease(other); }));
    assert(router.release(leased) && router.clientCount() == 0);

    // --- Файлы и права.
    File file("notes.txt", "root", Permissions("r--"));
    assert(throwsEx<PermissionDeniedException>([&] { file.write("x"); }));
    file.chmod(Permissions("rw-"));
    file.write("virus payload");
    assert(file.read() == "virus payload");
    Directory dir("home");
    dir.addFile(file);
    Directory sub("docs");
    sub.addFile(File("a.txt", "root"));
    dir.addSubdirectory(sub);
    assert(dir.fileCount() == 2 && throwsEx<FileNotFoundException>([&] { dir.findFile("nope"); }));
    Antivirus av("Defender", "1.0", 100);
    av.addSignature("virus");
    auto infected = av.scan(dir);
    assert(infected.size() == 1 && infected[0] == "notes.txt" && av.scannedFiles() == 2);
    FileSystem fs(10);
    fs.createFile("big", "root", 8);
    assert(throwsEx<DiskFullException>([&] { fs.createFile("big2", "root", 8); }));
    assert(throwsEx<FileNotFoundException>([&] { fs.remove("none"); }));

    // --- Память и процессы.
    MemoryManager mem(1000);
    ProcessManager pm;
    int pid = pm.spawn("calc", 400, mem);
    assert(mem.freeMb() == 600 && throwsEx<InsufficientMemoryException>([&] { pm.spawn("big", 700, mem); }));
    Scheduler sched(5);
    sched.enqueue(pm.find(pid));
    assert(sched.next() == pid && sched.waitEstimateMs() == 5);
    pm.kill(pid, mem);
    assert(mem.freeMb() == 1000 && throwsEx<ProcessNotFoundException>([&] { pm.find(pid); }));
    assert(throwsEx<ProcessNotFoundException>([&] { Scheduler empty; empty.next(); }));

    // --- Пользователи.
    assert(throwsEx<AuthenticationException>([] { User u("bob", "weak"); }));
    User admin("root", "Passw0rdX", true);
    assert(admin.checkPassword("Passw0rdX") && !admin.checkPassword("x"));
    assert(throwsEx<AuthenticationException>([&] { Session s(admin, "wrong", 0); }));
    Session session(admin, "Passw0rdX", 10);
    assert(session.isActive() && session.duration(25) == 15);
    session.logout();
    assert(!session.isActive());

    // --- Сборка полного компьютера, виртуальные powerOn/powerOff.
    Desktop pc("Office PC");
    assert(throwsEx<BootFailureException>([&] { pc.powerOn(); }));
    pc.setMotherboard(makeBoard());
    pc.setCase(std::make_shared<ComputerCase>("Define", "ATX", 4));
    pc.addStorage(ssd);
    pc.installGpu(makeGpu());
    pc.setCooling(std::make_shared<CoolingSystem>("Fan", "Noctua", 80, 1500, false));
    pc.setPowerSupply(std::make_shared<PowerSupply>("Weak", "X", 30, 200, 0.8));
    assert(throwsEx<PowerException>([&] { pc.powerOn(); }));
    pc.setPowerSupply(std::make_shared<PowerSupply>("RM750", "Corsair", 130, 750, 0.9));
    OperatingSystem& os = pc.installOperatingSystem("Linux", "6.1");
    assert(throwsEx<ComputerException>([&] { os.requireBooted(); }));
    pc.powerOn();
    assert(pc.isOn() && os.isBooted() && pc.bootCount() == 1);
    assert(pc.describe() == "Office PC [включено]");
    assert(pc.runBenchmark().total() > 0);
    pc.attachDisplay(std::make_shared<Display>("D", "Dell", 200, 1920, 1080));
    pc.attachKeyboard(std::make_shared<Keyboard>("K", "X", 10, false));
    pc.attachMouse(std::make_shared<Mouse>("M", "X", 10, false));
    assert(pc.displayCount() == 1 && pc.hasInputDevices());

    // --- Приложения: виртуальный run().
    os.addUser(admin);
    assert(throwsEx<ComputerException>([&] { os.addUser(admin); }));
    Session s2 = os.login("root", "Passw0rdX", 0);
    assert(s2.isAdmin() && throwsEx<AuthenticationException>([&] { os.login("ghost", "Passw0rdX", 0); }));
    Browser browser("Firefox", "120", 2048);
    Game game("Doom", "1.0", 4096, 2048, 4096, 144);
    assert(throwsEx<ComputerException>([&] { os.launch(browser); }));
    os.installApplication(browser);
    os.installApplication(game);
    Application& generic = browser;               // полиморфный вызов
    int bp = os.launch(generic);
    assert(browser.tabCount() == 1 && os.processes().count() == 1);
    os.launch(game);
    assert(game.launches() == 1 && game.canRunOn(*pc.gpu()) && game.estimatedFps(*pc.gpu()) <= 144);
    os.closeProcess(bp);
    assert(os.processes().count() == 1);
    Driver driver("nvidia", "535");
    os.installDriver(driver, *pc.gpu());
    assert(driver.matches(*pc.gpu()) && os.driverCount() == 1);
    os.settings().set("theme", "dark");
    os.settings().lock();
    assert(throwsEx<PermissionDeniedException>([&] { os.settings().set("a", "b"); }));
    assert(throwsEx<ComputerException>([&] { os.clipboard().paste(); }));
    pc.powerOff();
    assert(!pc.isOn() && !os.isBooted());

    // --- Ноутбук.
    Laptop laptop("Book", Battery("Cell", "LG", 80, 5000), Display("Panel", "BOE", 100, 1920, 1080));
    laptop.setMotherboard(makeBoard());
    laptop.setCase(std::make_shared<ComputerCase>("Chassis", "ATX", 2));
    laptop.setPowerSupply(std::make_shared<PowerSupply>("Brick", "X", 20, 300, 0.85));
    laptop.addStorage(std::make_shared<Ssd>("NVMe", "WD", 90, 256, 3000));
    laptop.powerOn();
    laptop.work(10);
    assert(laptop.batteryPercent() < 100);
    laptop.closeLid();
    assert(!laptop.isOn() && throwsEx<PowerException>([&] { laptop.powerOn(); }));
    laptop.openLid();
    assert(throwsEx<PowerException>([&] { laptop.work(1); }));

    // --- Сервер и закрытое наследование.
    SecureServer srv("Prod");
    srv.name();                                       // доступно через using
    assert(throwsEx<PowerException>([&] { srv.requestAccess(IpAddress("10.0.0.1")); }));
    static_assert(!std::is_convertible<SecureServer*, Server*>::value, "private base");
    static_assert(!std::is_convertible<GamingPc*, Computer*>::value, "protected base");
    static_assert(std::is_convertible<Laptop*, Computer*>::value, "public base");
    static_assert(std::is_convertible<Server*, Powered*>::value, "ромб доступен");

    Server plain("Plain");
    plain.setMotherboard(makeBoard());
    plain.setCase(std::make_shared<ComputerCase>("Rack", "ATX", 2));
    plain.setPowerSupply(std::make_shared<PowerSupply>("PSU", "X", 100, 500, 0.9));
    plain.addStorage(hdd);
    assert(throwsEx<PowerException>([&] { plain.startService("ssh"); }));
    plain.powerOn();
    plain.startService("ssh");
    assert(throwsEx<ComputerException>([&] { plain.startService("ssh"); }));
    plain.addUptime(48);
    plain.restart();
    assert(plain.uptimeHours() == 0 && plain.serviceCount() == 0 && plain.bootCount() == 2);

    SecureServer secure("Secure");
    secure.enableFirewall();
    IpAddress bad("203.0.113.5");
    secure.blockIp(bad);
    assert(secure.isBlocked(bad) && secure.blockedCount() == 1);

    // --- Игровой ПК (защищённое наследование).
    GamingPc gaming("Rig");
    gaming.assemble(makeBoard(), std::make_shared<PowerSupply>("PSU", "X", 100, 750, 0.9),
                    std::make_shared<ComputerCase>("Tower", "ATX", 4), makeGpu(),
                    std::make_shared<Ssd>("NVMe", "WD", 90, 1000, 7000));
    gaming.setRgb("rainbow");
    Game heavy("Cyberpunk", "2.0", 1000, 2048, 16000, 60);
    Game light("Tetris", "1.0", 10, 64, 256, 60);
    assert(!gaming.canPlay(heavy) && gaming.canPlay(light));
    assert(gaming.expectedFps(light) <= 60);
    gaming.applyOverclock(5);
    assert(gaming.overclock() == 5 && gaming.benchmarkScore() > 0);
    gaming.powerOn();
    assert(gaming.isOn());

    // --- Обслуживание.
    Technician tech("Иван", "ремонт плат");
    Computer empty("Empty");
    assert(tech.diagnose(empty).size() >= 4);
    RepairTicket ticket("Не включается");
    assert(throwsEx<ComputerException>([&] { ticket.close(); }));
    tech.repair(ticket, cpu1);
    assert(!ticket.isOpen() && ticket.assignedTo() == "Иван" && ticket.cost() > 50 && tech.closedTickets() == 1);

    // --- Прочее.
    Warranty w(12);
    w.consume(12);
    assert(!w.isValid());
    w.extend(6);
    assert(w.isValid() && w.remainingMonths() == 6);
    BenchmarkResult low(100, 100, 100, 100), high(3000, 3000, 3000, 3000);
    assert(high.isBetterThan(low) && high.rating() == "высокий" && low.rating() == "низкий");
    Logger logger(2);
    logger.log("INFO", "a"); logger.log("ERROR", "b"); logger.log("INFO", "c");
    assert(logger.count() == 2 && logger.errorCount() == 1 && logger.last().format() == "[3] INFO: c");
    Battery battery("B", "X", 1, 1000);
    assert(throwsEx<PowerException>([&] { battery.discharge(2000); }) && battery.isEmpty());

    // --- Любое исключение предметной области ловится базовым классом.
    assert(throwsEx<ComputerException>([] { throw DiskFullException("x"); }));

    std::cout << "Все тесты пройдены.\n";
    return 0;
}
