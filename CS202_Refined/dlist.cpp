/* Program Name: Dlist
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: This program can manipulate a doubley linked list */

#include "dlist.hpp"
#include <iostream>

using namespace std;

// initalize a double linked lisk wif a sentinel node
Dlist::Dlist()
{
    sentinel = new Dnode;       // initalize sentinel node here
    sentinel->flink = sentinel; // link in front points to itself
    sentinel->blink = sentinel; // link in da back points to itself
    size = 0;                   // initalize size to zero
}

// removes all the notes
Dlist::~Dlist()
{
    Clear();         // Remove all nodes
    delete sentinel; // remove sentinel
}

// create a dupe of an existing list
Dlist::Dlist(const Dlist &d)
{
    sentinel = new Dnode;       // initalize a new sentinel node
    sentinel->flink = sentinel; // Same things as before
    sentinel->blink = sentinel;
    size = 0;
    *this = d;
    // So *this = d copies all elements from d to the new list, using the code to duplicate it.
}

// assigns contents to another
Dlist &Dlist::operator=(const Dlist &d)
{
    if (this != &d)
    {            // complete opposiste of what is done above, thsi aviods duping itself
        Clear(); // clears the list
        for (Dnode *current = d.Begin(); current != d.End(); current = current->Next())
        {
            Push_Back(current->s); // add each element to the lsit
        }
    }
    return *this; // return the current list
}

// removes all nodes except the sentinel
void Dlist::Clear()
{
    Dnode *current = sentinel->flink; // start from the first
    while (current != sentinel)
    {                             // travels to the sentinel
        Dnode *temp = current;    // store the current node
        current = current->flink; // move forward
        delete temp;              // delete current node
    }
    sentinel->flink = sentinel; // Reset to beginning position
    sentinel->blink = sentinel;
    size = 0;
}

// check to see if it's empty
bool Dlist::Empty() const
{
    return size == 0; // it's empty if it's zero, then return true
}

// gives how many elements are in the list
size_t Dlist::Size() const
{
    return size; // give back the size
}

// puts a new node with the string input
void Dlist::Push_Front(const string &s)
{
    Insert_After(s, sentinel); // puts it after the sentinel
}

// put new node with the string input in the back this time
void Dlist::Push_Back(const string &s)
{
    Insert_Before(s, sentinel); // puts it before the sentinel
}

// Removes elemenet from the front
string Dlist::Pop_Front()
{
    if (Empty())
    {                                                               // check for empty
        throw underflow_error("Pop_Front called on an empty list"); // error message
    }
    Dnode *node = sentinel->flink; // first node
    string value = node->s;        // store first node
    Erase(node);                   // remove first node
    return value;
}

// remove element from the back
string Dlist::Pop_Back()
{
    if (Empty())
    {                                                              // check for empty
        throw underflow_error("Pop_Back called on an empty list"); // error message
    }
    Dnode *node = sentinel->blink; // Basically the same from pop_front
    string value = node->s;
    Erase(node);
    return value;
}

// pointer to the first element
Dnode *Dlist::Begin() const
{
    return sentinel->flink; // node after sentinel
}

// pointe to sentinel node
Dnode *Dlist::End() const
{
    return sentinel; // goes to sentinel (end of list)
}

// pointer to very last element
Dnode *Dlist::Rbegin() const
{
    return sentinel->blink; // node before sentinel
}

// pointer to sentinel node
Dnode *Dlist::Rend() const
{
    return sentinel; // goes to sentinel
}

// new node with specific string inserted in a particular way
void Dlist::Insert_Before(const string &s, Dnode *n)
{
    Dnode *newNode = new Dnode; // new node
    newNode->s = s;             // set value of said node
    newNode->flink = n;         // next link of the new node to the input node
    newNode->blink = n->blink;  // behind link of the new node to the previous node
    n->blink->flink = newNode;  // change the next  link of the previous node
    n->blink = newNode;         // change the backward link of the input node
    ++size;                     // increase list size
}

// basically the same as before but using behind before next
void Dlist::Insert_After(const string &s, Dnode *n)
{
    Dnode *newNode = new Dnode;
    newNode->s = s;
    newNode->flink = n->flink;
    newNode->blink = n;
    n->flink->blink = newNode;
    n->flink = newNode;
    ++size;
}

// removes a certain element
void Dlist::Erase(Dnode *n)
{
    if (n == sentinel)
    {                                                         // sentinel check
        throw invalid_argument("Cannot erase sentinel node"); // error message
    }
    n->blink->flink = n->flink; // change the next link of the previous node
    n->flink->blink = n->blink; // change the behind link of the next node
    delete n;                   // delete the node
    --size;                     // decrease list size
}

// pointer to the next node in the list
Dnode *Dnode::Next()
{
    return flink; // goes to next node
}

// pointer to the behind node in the list
Dnode *Dnode::Prev()
{
    return blink; // goes to behind node
}
