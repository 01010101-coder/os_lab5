#pragma once

#define PIPE_NAME R"(\\.\pipe\MyNamedPipe)"

enum RequestType {
    READ,
    MODIFY
};

struct employee {
    int num;
    char name[20];
    double hours;
};

struct Request {
    RequestType type;
    int employeeId;
};

struct Response {
    bool found;
    employee emp;
};
