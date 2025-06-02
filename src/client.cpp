#define NOMINMAX
#include <iostream>
#include <windows.h>
#include <string>
#include <limits>
#include "common.h"

void clearInputBuffer() {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

void printEmployee(const employee& emp) {
    std::cout << "ID: " << emp.num << "\n";
    std::cout << "Name: " << emp.name << "\n";
    std::cout << "Hours: " << emp.hours << "\n";
}

int main() {
    while (true) {
        std::cout << "\nВыберите операцию:\n";
        std::cout << "1 - Чтение записи\n";
        std::cout << "2 - Модификация записи\n";
        std::cout << "0 - Выход\n";
        std::cout << "> ";

        int choice;
        std::cin >> choice;
        if (choice == 0) break;

        int id;
        std::cout << "Введите ID сотрудника: ";
        std::cin >> id;

        HANDLE pipe = CreateFileA(
            PIPE_NAME, GENERIC_READ | GENERIC_WRITE, 0, nullptr,
            OPEN_EXISTING, 0, nullptr);

        if (pipe == INVALID_HANDLE_VALUE) {
            std::cerr << "Не удалось подключиться к каналу\n";
            system("pause");
            continue;
        }

        Request req;
        req.type = (choice == 1 ? READ : MODIFY);
        req.employeeId = id;

        DWORD bytesWritten;
        WriteFile(pipe, &req, sizeof(req), &bytesWritten, nullptr);

        Response res;
        DWORD bytesRead;
        ReadFile(pipe, &res, sizeof(res), &bytesRead, nullptr);

        if (!res.found) {
            std::cout << "Сотрудник с ID " << id << " не найден.\n";
            CloseHandle(pipe);
            continue;
        }

        std::cout << "Запись получена:\n";
        printEmployee(res.emp);

        if (req.type == MODIFY) {
            employee modified = res.emp;

            clearInputBuffer();
            std::string newName;
            std::cout << "Новое имя: ";
            std::getline(std::cin, newName);
            strncpy(modified.name, newName.c_str(), sizeof(modified.name) - 1);
            modified.name[sizeof(modified.name) - 1] = '\0';

            std::cout << "Новые часы: ";
            std::cin >> modified.hours;

            WriteFile(pipe, &modified, sizeof(modified), &bytesWritten, nullptr);
            std::cout << "Изменения отправлены серверу.\n";
        }

        CloseHandle(pipe);
    }

    std::cout << "Клиент завершает работу.\n";
    return 0;
}
