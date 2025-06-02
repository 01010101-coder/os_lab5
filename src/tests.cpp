#include <gtest/gtest.h>
#include <fstream>
#include <cstdio>
#include <cstring>


struct employee {
    int num;
    char name[10];
    double hours;
};

bool writeEmployeeToFile(const std::string& filename, const employee& emp) {
    std::ofstream file(filename, std::ios::binary | std::ios::app);
    if (!file) return false;
    file.write(reinterpret_cast<const char*>(&emp), sizeof(emp));
    return true;
}

employee readEmployeeFromFile(const std::string& filename, int id) {
    std::ifstream file(filename, std::ios::binary);
    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        if (emp.num == id) return emp;
    }
    emp.num = -1;
    return emp;
}

bool modifyEmployeeInFile(const std::string& filename, const employee& modified) {
    std::fstream file(filename, std::ios::in | std::ios::out | std::ios::binary);
    if (!file) return false;
    employee emp;
    while (file.read(reinterpret_cast<char*>(&emp), sizeof(emp))) {
        if (emp.num == modified.num) {
            file.seekp(-static_cast<int>(sizeof(emp)), std::ios::cur);
            file.write(reinterpret_cast<const char*>(&modified), sizeof(modified));
            return true;
        }
    }
    return false;
}


const std::string testFile = "test_employees.dat";

TEST(EmployeeTests, WriteAndRead) {
    employee emp = {1, "Test", 42.5};
    ASSERT_TRUE(writeEmployeeToFile(testFile, emp));

    employee result = readEmployeeFromFile(testFile, 1);
    EXPECT_EQ(result.num, emp.num);
    EXPECT_STREQ(result.name, emp.name);
    EXPECT_DOUBLE_EQ(result.hours, emp.hours);
}

TEST(EmployeeTests, ModifyEmployee) {
    employee emp = {2, "Second", 30.0};
    writeEmployeeToFile(testFile, emp);

    employee updated = {2, "Updated", 50.0};
    ASSERT_TRUE(modifyEmployeeInFile(testFile, updated));

    employee result = readEmployeeFromFile(testFile, 2);
    EXPECT_EQ(result.num, 2);
    EXPECT_STREQ(result.name, "Updated");
    EXPECT_DOUBLE_EQ(result.hours, 50.0);
}

TEST(EmployeeTests, ReadNonExisting) {
    employee result = readEmployeeFromFile(testFile, 999);
    EXPECT_EQ(result.num, -1);
}

TEST(EmployeeTests, ModifyNonExisting) {
    employee ghost = {999, "Ghost", 10.0};
    EXPECT_FALSE(modifyEmployeeInFile(testFile, ghost));
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    int result = RUN_ALL_TESTS();
    std::remove(testFile.c_str());  
    return result;
}
