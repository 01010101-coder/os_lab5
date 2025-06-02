
#define NOMINMAX
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <windows.h>
#include <limits>
#include <thread>
#include "common.h"

std::string filename;

void createEmployeeFile() {
    std::ofstream file(filename, std::ios::binary);
    int n;
    std::cout << "Введите количество сотрудников: ";
    std::cin >> n;

    for (int i = 0; i < n; ++i) {
        employee emp;

        std::cout << "ID: ";
        std::cin >> emp.num;

        std::cout << "Имя: ";
        std::string tempName;
        std::cin >> tempName;
        strncpy(emp.name, tempName.c_str(), sizeof(emp.name) - 1);
        emp.name[sizeof(emp.name) - 1] = '\0';

        std::cout << "Часы: ";
        std::cin >> emp.hours;

        file.write(reinterpret_cast<char*>(&emp), sizeof(emp));
    }

    file.close();
}

employee readRecord(int id) {
    std::ifstream file(filename, std::ios::binary);
    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        if (emp.num == id)
            return emp;
    }
    emp.num = -1;
    return emp;
}

void modifyRecord(const employee& newEmp) {
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        if (emp.num == newEmp.num) {
            file.seekp(-static_cast<int>(sizeof(emp)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&newEmp), sizeof(newEmp));
            break;
        }
    }
}

void printFile() {
    std::ifstream file(filename, std::ios::binary);
    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        std::cout << "ID: " << emp.num << " Name: " << emp.name << " Hours: " << emp.hours << "\n";
    }
}

void handleClient(HANDLE pipe) {
    Request req;
    DWORD bytesRead;
    ReadFile(pipe, &req, sizeof(req), &bytesRead, nullptr);

    Response res{};
    employee emp = readRecord(req.employeeId);
    res.found = (emp.num != -1);
    res.emp = emp;

    DWORD bytesWritten;
    WriteFile(pipe, &res, sizeof(res), &bytesWritten, nullptr);

    if (req.type == MODIFY && res.found) {
        employee modified;
        ReadFile(pipe, &modified, sizeof(modified), &bytesRead, nullptr);
        modifyRecord(modified);
    }

    CloseHandle(pipe);
}

void serverLoop() {
    while (true) {
        HANDLE pipe = CreateNamedPipeA(
            PIPE_NAME, PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
            PIPE_UNLIMITED_INSTANCES, 0, 0, 0, nullptr);

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Ошибка создания канала\n";
            return;
        }

        if (ConnectNamedPipe(pipe, nullptr) || GetLastError() == ERROR_PIPE_CONNECTED) {
            std::thread(handleClient, pipe).detach(); 
        }
    }
}

int main() {
    std::cout << "Введите имя файла: ";
    std::cin >> filename;

    createEmployeeFile();
    std::cout << "\nФайл создан:\n";
    printFile();

    int clientCount;
    std::cout << "\nВведите количество клиентов: ";
    std::cin >> clientCount;

    for (int i = 0; i < clientCount; ++i) {
        system("start Client.exe");
    }

    std::cout << "\nСервер ожидает запросы...\n";

    std::thread(serverLoop).detach();

    std::string cmd;
    while (true) {
        std::cout << "\nВведите 'exit' для выхода";
        std::getline(std::cin >> std::ws, cmd);
        if (cmd == "exit") break;
    }

    std::cout << "\nМодифицированный файл:\n";
    printFile();

    return 0;
}
