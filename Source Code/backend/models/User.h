#ifndef USER_H
#define USER_H

#include <string>
#include <iostream>
using namespace std;

// Abstract base class representing a user in the vehicle rental system
class User {
protected:
    int id;
    string userName;
    string passWord;
    string role;

public:
    // Constructor
    User(int userId, const string& usernameVal, const string& passwordVal, const string& roleVal)
        : id(userId), userName(usernameVal), passWord(passwordVal), role(roleVal) {}

    // Virtual destructor
    virtual ~User() = default;

    // Pure virtual method for polymorphic display
    virtual void displayInfo() = 0;

    // Getters
    int getId() const { return id; }
    string getRole() const { return role; }
    string getUsername() const { return userName; }
    string getPassword() const { return passWord; }

    // Setters
    void setId(int userId) { id = userId; }
    void setUsername(const string& usernameVal) { userName = usernameVal; }
    void setPassword(const string& passwordVal) { passWord = passwordVal; }
    void setRole(const string& roleVal) { role = roleVal; }
};

#endif // USER_H
