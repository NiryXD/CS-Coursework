/* Program Name: Code Processor
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: This is a reward system and the user manipulate it */

#include "code_processor.hpp"
#include <fstream>
#include <sstream>
#include <algorithm>

using namespace std;

// The write up asks me to use djb_hash(), so below is my implementation of it
// The function iterates over each character, using bit shifts and addition to compute a hash value
unsigned int djbhash(const string &input)
{
  unsigned int hashvalue = 5381;
  for (char element : input)
  { // I usually use ':' in ternary operators, but in this instance it goes through the aforementioned string
    hashvalue = ((hashvalue << 5) + hashvalue) + element;
  }
  return hashvalue; // Return the hash value
}
// I just copied everything from the bitmatrix lab :D

// This initalizes a new prize if there isn't one like it yet
bool Code_Processor::New_Prize(const string &prize_id, const string &desc,
                               int cost, int stock)
{
  // See if prize is already there
  if (Prizes.find(prize_id) != Prizes.end())
    return false;
  // See if the number is reasonable
  if (cost <= 0 || stock <= 0)
    return false;

  // create new object and then utilize pointers to add the info
  Prize *reward = new Prize;
  reward->id = prize_id;
  reward->description = desc;
  reward->points = cost;
  reward->quantity = stock;
  Prizes[prize_id] = reward;

  return true;
}

// initalizes new users
bool Code_Processor::New_User(const string &login, const string &fullname,
                              int initial_points)
{
  // Checks if the name is already there
  if (Names.find(login) != Names.end())
    return false;
  if (initial_points < 0)
    return false;

  // Same pointer use from before, but with usernames
  User *member = new User;
  member->username = login;
  member->realname = fullname;
  member->points = initial_points;
  Names[login] = member;

  return true;
}

// Check the monies of a certain user
int Code_Processor::Balance(const string &login) const
{
  // If the system can't find the name return -1
  unordered_map<string, User *>::const_iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return -1;
  return member_iter->second->points; // Give user points balance
}

// Add number to existing user
bool Code_Processor::Add_Phone(const string &login, const string &number)
{
  // Look for the name, if it aint there return false
  unordered_map<string, User *>::iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return false;

  // If number is already there, return false
  if (Phones.find(number) != Phones.end())
    return false;

  // add the number to the user
  member_iter->second->phone_numbers.insert(number);
  Phones[number] = member_iter->second;

  return true;
}

// pull up all number associated with a certain user
string Code_Processor::Show_Phones(const string &login) const
{
  // Look up a name, if it aint there return "BAD USER"
  unordered_map<string, User *>::const_iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return "BAD USER";

  // cout that returns all number
  string output;
  set<string>::const_iterator phone_it;
  for (phone_it = member_iter->second->phone_numbers.begin();
       phone_it != member_iter->second->phone_numbers.end(); ++phone_it)
  {
    output += *phone_it + "\n";
  }
  return output;
}

// remove an existing user
bool Code_Processor::Delete_User(const string &login)
{
  // find name if it aint there return false
  unordered_map<string, User *>::iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return false;

  User *member = member_iter->second;

  // Erase the numbers of the user
  set<string>::iterator phone_it;
  for (phone_it = member->phone_numbers.begin();
       phone_it != member->phone_numbers.end(); ++phone_it)
  {
    Phones.erase(*phone_it);
  }

  Names.erase(login);
  delete member;

  return true;
}

// Only remove the number from a person
bool Code_Processor::Remove_Phone(const string &login, const string &number)
{
  // find name if it aint there return false
  unordered_map<string, User *>::iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return false;

  // find his/her number, if it doesn't exist return false
  unordered_map<string, User *>::iterator phone_iter = Phones.find(number);
  if (phone_iter == Phones.end())
    return false;

  if (phone_iter->second != member_iter->second)
    return false;

  member_iter->second->phone_numbers.erase(number);
  Phones.erase(number);

  return true;
}

// enters in a code to get points
int Code_Processor::Enter_Code(const string &login, const string &reward_code)
{
  // search for the name, and if it aint there return -1
  unordered_map<string, User *>::iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return -1;

  // if code is already used, return -1
  if (Codes.find(reward_code) != Codes.end())
    return -1;

  // find the hash for the code
  unsigned int hashval = djbhash(reward_code);
  int points = 0;

  // See how many monies the guy/girl gets
  if (hashval % 17 == 0)
  {
    points = 10;
  }
  else if (hashval % 13 == 0)
  {
    points = 3;
  }

  // add points to balance
  if (points > 0)
  {
    member_iter->second->points += points;
    Codes.insert(reward_code);
  }

  return points;
}

// Function to enter a code using a phone number
// Parameters: phone number and code to be entered
int Code_Processor::Text_Code(const string &number, const string &reward_code)
{
  // Find the user by phone number; if not found, return -1
  unordered_map<string, User *>::iterator member_iter = Phones.find(number);
  if (member_iter == Names.end())
    return -1;

  // Call Enter_Code with the user's username
  return Enter_Code(member_iter->second->username, reward_code);
}

// just too mark code as used
bool Code_Processor::Mark_Code_Used(const string &reward_code)
{
  // return false if the code is already used
  if (Codes.find(reward_code) != Codes.end())
    return false;

  // hash to see if it's a valid code
  unsigned int hashval = djbhash(reward_code);
  if (hashval % 17 != 0 && hashval % 13 != 0)
    return false;

  Codes.insert(reward_code);
  return true;
}

// use balance to redeem prize for a person
bool Code_Processor::Redeem_Prize(const string &login, const string &reward_id)
{
  // try to find the user, if not found return false
  unordered_map<string, User *>::iterator member_iter = Names.find(login);
  if (member_iter == Names.end())
    return false;

  // try to find the prize, if it aint there return false
  unordered_map<string, Prize *>::iterator prize_iter = Prizes.find(reward_id);
  if (prize_iter == Prizes.end())
    return false;

  User *member = member_iter->second;
  Prize *reward = prize_iter->second;

  // Seee if user has enough points to buy the prize
  if (member->points < reward->points)
    return false;

  // substract from the balance
  member->points -= reward->points;
  // substract from the stock
  reward->quantity--;

  // If there isn't anymore of a certain item, we delete it
  if (reward->quantity == 0)
  {
    Prizes.erase(reward_id);
    delete reward;
  }

  return true;
}

// Just to have the system readable to the current user
bool Code_Processor::Write(const string &fname) const
{
  ofstream outfile(fname);
  if (!outfile)
    return false;

  // Shows all the prizes
  unordered_map<string, Prize *>::const_iterator prize_it;
  for (prize_it = Prizes.begin(); prize_it != Prizes.end(); ++prize_it)
  {
    const Prize *reward = prize_it->second;
    outfile << "PRIZE " << reward->id << " " << reward->points << " "
            << reward->quantity << " " << reward->description << "\n";
  }

  // Shows users
  unordered_map<string, User *>::const_iterator user_it;
  for (user_it = Names.begin(); user_it != Names.end(); ++user_it)
  {
    const User *member = user_it->second;
    outfile << "ADD_USER " << member->username << " " << member->points
            << " " << member->realname << "\n";

    // shows numbers of said users
    set<string>::const_iterator phone_it;
    for (phone_it = member->phone_numbers.begin();
         phone_it != member->phone_numbers.end(); ++phone_it)
    {
      outfile << "ADD_PHONE " << member->username << " " << *phone_it << "\n";
    }
  }

  // shows all used codes
  unordered_set<string>::const_iterator code_it;
  for (code_it = Codes.begin(); code_it != Codes.end(); ++code_it)
  {
    outfile << "MARK_USED " << *code_it << "\n";
  }

  return true;
}

// destructor to clear memory
Code_Processor::~Code_Processor()
{
  // deletes all users
  unordered_map<string, User *>::iterator user_it;
  for (user_it = Names.begin(); user_it != Names.end(); ++user_it)
  {
    delete user_it->second;
  }

  // deletes all prizes
  unordered_map<string, Prize *>::iterator prize_it;
  for (prize_it = Prizes.begin(); prize_it != Prizes.end(); ++prize_it)
  {
    delete prize_it->second;
  }
}