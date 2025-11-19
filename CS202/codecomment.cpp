#include "code_processor.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// Hash function to generate a hash value for a given string using djb2 algorithm
unsigned int djbhash(const string &s) {
  unsigned int h = 5381;
  for (size_t i = 0; i < s.size(); i++) {
    h = (h << 5) + h + s[i];  // h = h * 33 + s[i]
  }
  return h;
}

// Function to add a new prize to the system
// Parameters: id (unique identifier for the prize), description, points required, and quantity available
bool Code_Processor::New_Prize(const string &id, const string &description, 
  int points, int quantity) {
  // Check if the prize already exists or if the points or quantity are invalid
  if (Prizes.find(id) != Prizes.end()) return false;
  if (points <= 0 || quantity <= 0) return false;

  // Create a new Prize object and populate it with the provided information
  Prize *p = new Prize;
  p->id = id;
  p->description = description;
  p->points = points;
  p->quantity = quantity;
  Prizes[id] = p;  // Add the prize to the Prizes map
  
  return true;
}

// Function to add a new user to the system
// Parameters: username (unique identifier for the user), realname, and starting points
bool Code_Processor::New_User(const string &username, const string &realname, 
                            int starting_points) {
  // Check if the user already exists or if the starting points are negative
  if (Names.find(username) != Names.end()) return false;
  if (starting_points < 0) return false;

  // Create a new User object and populate it with the provided information
  User *u = new User;
  u->username = username;
  u->realname = realname;
  u->points = starting_points;
  Names[username] = u;  // Add the user to the Names map
  
  return true;
}

// Function to get the balance (points) of a user
// Parameter: username (identifier of the user)
int Code_Processor::Balance(const string &username) const {
  // Find the user by username; if not found, return -1
  unordered_map<string, User*>::const_iterator it = Names.find(username);
  if (it == Names.end()) return -1;
  return it->second->points;  // Return the user's points
}

// Function to add a phone number to a user
// Parameters: username and phone number to be added
bool Code_Processor::Add_Phone(const string &username, const string &phone) {
  // Find the user by username; if not found, return false
  unordered_map<string, User*>::iterator user_it = Names.find(username);
  if (user_it == Names.end()) return false;
  
  // Check if the phone number already exists; if so, return false
  if (Phones.find(phone) != Phones.end()) return false;
  
  // Add the phone number to the user's set of phone numbers
  user_it->second->phone_numbers.insert(phone);
  Phones[phone] = user_it->second;  // Add the phone number to the Phones map
  
  return true;
}

// Function to show all phone numbers associated with a user
// Parameter: username (identifier of the user)
string Code_Processor::Show_Phones(const string &username) const {
  // Find the user by username; if not found, return "BAD USER"
  unordered_map<string, User*>::const_iterator it = Names.find(username);
  if (it == Names.end()) return "BAD USER";
  
  // Concatenate all phone numbers into a single string with each number on a new line
  string result;
  set<string>::const_iterator phone_it;
  for (phone_it = it->second->phone_numbers.begin(); 
       phone_it != it->second->phone_numbers.end(); ++phone_it) {
    result += *phone_it + "\n";
  }
  return result;
}

// Function to delete a user from the system
// Parameter: username (identifier of the user to be deleted)
bool Code_Processor::Delete_User(const string &username) {
    // Find the user by username; if not found, return false
    unordered_map<string, User*>::iterator it = Names.find(username);
    if (it == Names.end()) return false;
    
    User *user = it->second;
    
    // Remove all phone numbers associated with the user from the Phones map
    set<string>::iterator phone_it;
    for (phone_it = user->phone_numbers.begin(); 
         phone_it != user->phone_numbers.end(); ++phone_it) {
        Phones.erase(*phone_it);
    }
    
    // Remove the user from the Names map and delete the user object
    Names.erase(username);
    delete user;
    
    return true;
}

// Function to remove a phone number from a user
// Parameters: username and phone number to be removed
bool Code_Processor::Remove_Phone(const string &username, const string &phone) {
    // Find the user by username; if not found, return false
    unordered_map<string, User*>::iterator user_it = Names.find(username);
    if (user_it == Names.end()) return false;
    
    // Find the phone number; if not found or not associated with the user, return false
    unordered_map<string, User*>::iterator phone_it = Phones.find(phone);
    if (phone_it == Phones.end()) return false;
    
    if (phone_it->second != user_it->second) return false;
    
    // Remove the phone number from the user's set and from the Phones map
    user_it->second->phone_numbers.erase(phone);
    Phones.erase(phone);
    
    return true;
}

// Function to enter a code for a user and receive points
// Parameters: username and code to be entered
int Code_Processor::Enter_Code(const string &username, const string &code) {
    // Find the user by username; if not found, return -1
    unordered_map<string, User*>::iterator it = Names.find(username);
    if (it == Names.end()) return -1;
    
    // Check if the code has already been used; if so, return -1
    if (Codes.find(code) != Codes.end()) return -1;
    
    // Compute the hash value for the code
    unsigned int hash = djbhash(code);
    int points = 0;
    
    // Determine points based on hash value (multiples of 17 or 13)
    if (hash % 17 == 0) {
        points = 10;
    } else if (hash % 13 == 0) {
        points = 3;
    }
    
    // If the code is valid, add points to the user's balance and mark the code as used
    if (points > 0) {
        it->second->points += points;
        Codes.insert(code);
    }
    
    return points;
}

// Function to enter a code using a phone number
// Parameters: phone number and code to be entered
int Code_Processor::Text_Code(const string &phone, const string &code) {
    // Find the user by phone number; if not found, return -1
    unordered_map<string, User*>::iterator it = Phones.find(phone);
    if (it == Phones.end()) return -1;
    
    // Call Enter_Code with the user's username
    return Enter_Code(it->second->username, code);
}

// Function to mark a code as used without adding points
// Parameter: code to be marked as used
bool Code_Processor::Mark_Code_Used(const string &code) {
    // Check if the code has already been used; if so, return false
    if (Codes.find(code) != Codes.end()) return false;
    
    // Compute the hash value for the code and check if it is a valid code
    unsigned int hash = djbhash(code);
    if (hash % 17 != 0 && hash % 13 != 0) return false;
    
    // Mark the code as used
    Codes.insert(code);
    return true;
}

// Function to redeem a prize for a user
// Parameters: username and prize identifier
bool Code_Processor::Redeem_Prize(const string &username, const string &prize) {
    // Find the user by username; if not found, return false
    unordered_map<string, User*>::iterator user_it = Names.find(username);
    if (user_it == Names.end()) return false;
    
    // Find the prize by id; if not found, return false
    unordered_map<string, Prize*>::iterator prize_it = Prizes.find(prize);
    if (prize_it == Prizes.end()) return false;
    
    User *user = user_it->second;
    Prize *p = prize_it->second;
    
    // Check if the user has enough points to redeem the prize
    if (user->points < p->points) return false;
    
    // Deduct the points and reduce the prize quantity
    user->points -= p->points;
    p->quantity--;
    
    // If the prize quantity reaches zero, remove the prize from the system
    if (p->quantity == 0) {
        Prizes.erase(prize);
        delete p;
    }
    
    return true;
}

// Function to write the current state of the system to a file
// Parameter: filename to write to
bool Code_Processor::Write(const string &filename) const {
  ofstream out(filename);
  if (!out) return false;
  
  // Write all prizes to the file
  unordered_map<string, Prize*>::const_iterator prize_it;
  for (prize_it = Prizes.begin(); prize_it != Prizes.end(); ++prize_it) {
    const Prize *p = prize_it->second;
    out << "PRIZE " << p->id << " " << p->points << " " 
        << p->quantity << " " << p->description << "\n";
  }
  
  // Write all users and their phone numbers to the file
  unordered_map<string, User*>::const_iterator user_it;
  for (user_it = Names.begin(); user_it != Names.end(); ++user_it) {
    const User *u = user_it->second;
    out << "ADD_USER " << u->username << " " << u->points 
        << " " << u->realname << "\n";
        
    set<string>::const_iterator phone_it;
    for (phone_it = u->phone_numbers.begin(); 
         phone_it != u->phone_numbers.end(); ++phone_it) {
      out << "ADD_PHONE " << u->username << " " << *phone_it << "\n";
    }
  }
  
  // Write all used codes to the file
  unordered_set<string>::const_iterator code_it;
  for (code_it = Codes.begin(); code_it != Codes.end(); ++code_it) {
    out << "MARK_USED " << *code_it << "\n";
  }
  
  return true;
}

// Destructor for Code_Processor to clean up allocated memory
Code_Processor::~Code_Processor() {
  // Delete all user objects
  unordered_map<string, User*>::iterator user_it;
  for (user_it = Names.begin(); user_it != Names.end(); ++user_it) {
    delete user_it->second;
  }
  
  // Delete all prize objects
  unordered_map<string, Prize*>::iterator prize_it;
  for (prize_it = Prizes.begin(); prize_it != Prizes.end(); ++prize_it) {
    delete prize_it->second;
  }
}
