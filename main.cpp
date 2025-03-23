#include <iostream>
#include <string>
#include <map>
#include <ctime>
#include <random>
#include <sstream>
#include <unordered_map>

// Generate UUID-like string
std::string generateUUID(int length) {
    static const char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, sizeof(alphanum) - 2);
    
    std::string uuid;
    for (int i = 0; i < length; ++i) {
        uuid += alphanum[dis(gen)];
    }
    return uuid;
}

// Current date/time as string
std::string getCurrentTime() {
    time_t now = time(0);
    return ctime(&now);
}

// Base User class
class User {
protected:
    std::string id, name;

public:
    User(std::string id, std::string name) : id(id), name(name) {}
    
    virtual ~User() {}
    
    void displayInfo() const {
        std::cout << "ID: " << id << ", Name: " << name << std::endl;
    }
    
    std::string getName() const {
        return name;
    }
    
    std::string getId() const {
        return id;
    }
};

// Forward declarations
class Visitor;

// Employee Class: Handles Visitor Approvals & Pre-Approvals
class Employee : public User {
public:
    Employee(std::string id, std::string name) : User(id, name) {}
    
    bool approveVisitor(Visitor* visitor);
    
    void denyVisitor(Visitor* visitor);
};

// Visitor Class: Stores Visitor Details
class Visitor : public User {
private:
    std::string purpose, visitingEmp, date, checkInTime, checkOutTime, ePass, companyName, contactInfo;
    bool approved;

public:
    Visitor(std::string id, std::string name, std::string purpose, std::string visitingEmp, 
            std::string date, std::string company, std::string contactInfo) 
        : User(id, name), purpose(purpose), visitingEmp(visitingEmp), date(date), 
          companyName(company), contactInfo(contactInfo), approved(false) {}
    
    void approve() {
        this->approved = true;
    }
    
    bool isApproved() const {
        return approved;
    }
    
    std::string getEPass() const {
        return ePass;
    }
    
    void generateEPass() {
        this->ePass = "EPASS-" + generateUUID(5);
    }
    
    void checkIn() {
        if (!approved) {
            std::cout << "Access Denied! Visitor not approved." << std::endl;
            return;
        }
        this->checkInTime = getCurrentTime();
        std::cout << "Visitor " << name << " checked in at " << checkInTime;
    }
    
    void checkOut() {
        if (checkInTime.empty()) {
            std::cout << "Visitor has not checked in yet." << std::endl;
            return;
        }
        this->checkOutTime = getCurrentTime();
        std::cout << "Visitor " << name << " checked out at " << checkOutTime;
    }
};

// Implementing Employee methods that depend on Visitor
bool Employee::approveVisitor(Visitor* visitor) {
    visitor->approve();
    std::cout << "Visitor " << visitor->getName() << " approved by " << name << std::endl;
    return true;
}

void Employee::denyVisitor(Visitor* visitor) {
    std::cout << "Visitor " << visitor->getName() << " denied by " << name << std::endl;
}

// Security Class: Manages Check-ins & Check-outs
class Security : public User {
public:
    Security(std::string id, std::string name) : User(id, name) {}
};

// Admin Class: Controls System Settings
class Admin : public User {
private:
    int preApproveLimit = 5;

public:
    Admin(std::string id, std::string name) : User(id, name) {}
    
    void setPreApproveLimit(int limit) {
        preApproveLimit = limit;
        std::cout << "Pre-approval limit set to " << limit << std::endl;
    }
    
    int getPreApproveLimit() const {
        return preApproveLimit;
    }
};

// Database: Stores Employees, Visitors & Pre-Approvals
class Database {
public:
    static std::map<std::string, Employee*> empDB;
    static std::map<std::string, Visitor*> visitorDB;
    static std::map<std::string, std::string> preApprovedList;
    static std::map<std::string, int> preApprovalCount;
    
    // Cleanup method to prevent memory leaks
    static void cleanup() {
        for (auto& pair : empDB) {
            delete pair.second;
        }
        
        for (auto& pair : visitorDB) {
            delete pair.second;
        }
        
        empDB.clear();
        visitorDB.clear();
        preApprovedList.clear();
        preApprovalCount.clear();
    }
};

// Initialize static members
std::map<std::string, Employee*> Database::empDB;
std::map<std::string, Visitor*> Database::visitorDB;
std::map<std::string, std::string> Database::preApprovedList;
std::map<std::string, int> Database::preApprovalCount;

// Security Service: Handles Visitor Management
class SecurityService {
public:
    void registerVisitor(std::string vName, std::string purpose, std::string empName, 
                         std::string company, std::string contactInfo) {
        if (Database::empDB.find(empName) == Database::empDB.end()) {
            std::cout << "Employee not found." << std::endl;
            return;
        }

        std::string visitorId = generateUUID(8);
        Visitor* visitor = new Visitor(visitorId, vName, purpose, empName, 
                                      getCurrentTime(), company, contactInfo);

        Employee* emp = Database::empDB[empName];
        if (emp->approveVisitor(visitor)) {
            visitor->generateEPass();
            Database::visitorDB[visitor->getEPass()] = visitor;
            std::cout << "Visitor registered with ePass: " << visitor->getEPass() << std::endl;
        }
    }

    void checkInVisitor(std::string ePass) {
        if (Database::visitorDB.find(ePass) == Database::visitorDB.end()) {
            std::cout << "ePass not found." << std::endl;
            return;
        }
        Database::visitorDB[ePass]->checkIn();
    }

    void checkOutVisitor(std::string ePass) {
        if (Database::visitorDB.find(ePass) == Database::visitorDB.end()) {
            std::cout << "ePass not found." << std::endl;
            return;
        }
        Database::visitorDB[ePass]->checkOut();
    }
};

// Employee Service: Handles Approvals & Pre-Approvals
class EmployeeService {
public:
    void preApproveVisitor(Employee* emp, std::string timeSlot) {
        if (Database::preApprovalCount[emp->getId()] >= 5) {
            std::cout << "Pre-approval limit reached." << std::endl;
            return;
        }

        std::string ePass = "EPASS-" + generateUUID(5);
        Database::preApprovedList[ePass] = timeSlot;
        Database::preApprovalCount[emp->getId()] = Database::preApprovalCount[emp->getId()] + 1;

        std::cout << "Pre-approved visitor with ePass: " << ePass << std::endl;
    }
};

// Main Menu: Handles User Interaction
class MainMenu {
private:
    SecurityService securityService;
    EmployeeService employeeService;

public:
    void showMenu() {
        int choice;
        std::string input;
        
        while (true) {
            std::cout << "\n1. Register Visitor" << std::endl;
            std::cout << "2. Check-in Visitor" << std::endl;
            std::cout << "3. Pre-Approve Visitor" << std::endl;
            std::cout << "4. Check-out Visitor" << std::endl;
            std::cout << "5. Exit" << std::endl;
            
            std::cout << "Enter your choice: ";
            std::cin >> choice;
            std::cin.ignore(); // Clear the newline character
            
            switch (choice) {
                case 1:
                    registerVisitor();
                    break;
                case 2:
                    checkInVisitor();
                    break;
                case 3:
                    preApproveVisitor();
                    break;
                case 4:
                    checkOutVisitor();
                    break;
                case 5:
                    return;
                default:
                    std::cout << "Invalid choice." << std::endl;
            }
        }
    }

private:
    void registerVisitor() {
        std::string vName, purpose, empName, company, contactInfo;
        
        std::cout << "Enter Visitor Name: ";
        std::getline(std::cin, vName);
        
        std::cout << "Enter Purpose: ";
        std::getline(std::cin, purpose);
        
        std::cout << "Enter Employee Name: ";
        std::getline(std::cin, empName);
        
        std::cout << "Enter Company Name: ";
        std::getline(std::cin, company);
        
        std::cout << "Enter Contact Info: ";
        std::getline(std::cin, contactInfo);

        securityService.registerVisitor(vName, purpose, empName, company, contactInfo);
    }

    void checkInVisitor() {
        std::string ePass;
        std::cout << "Enter ePass ID for Check-in: ";
        std::getline(std::cin, ePass);
        securityService.checkInVisitor(ePass);
    }

    void checkOutVisitor() {
        std::string ePass;
        std::cout << "Enter ePass ID for Check-out: ";
        std::getline(std::cin, ePass);
        securityService.checkOutVisitor(ePass);
    }

    void preApproveVisitor() {
        std::string empName, timeSlot;
        
        std::cout << "Enter Employee Name: ";
        std::getline(std::cin, empName);
        
        if (Database::empDB.find(empName) == Database::empDB.end()) {
            std::cout << "Employee not found." << std::endl;
            return;
        }
        
        std::cout << "Enter Time Slot: ";
        std::getline(std::cin, timeSlot);
        
        employeeService.preApproveVisitor(Database::empDB[empName], timeSlot);
    }
};

// Main function
int main() {
    // Initialize with one employee
    Database::empDB["Alice"] = new Employee("E001", "Alice");
    
    MainMenu menu;
    menu.showMenu();
    
    // Clean up memory
    Database::cleanup();
    
    return 0;
}