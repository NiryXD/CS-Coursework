#include "volsort.h"
#include <bits/stdc++.h>

using namespace std;

// Initalizes a empty linked list
List::List()
{
  head = nullptr;
  // Set head to nothing
  size = 0;
  // Size is 0 because there's nothing yet
}

// Cleans the linked list
List::~List()
{
  while (head)
  {
    // Runs when there's elements
    Node *temporary = head;
    // Store head in temp var
    head = head->next;
    // Increment the head
    delete temporary;
    // Delete prev head
  }
}

void List::push_front(const std::string &s)
{

  // Creates a new node
  Node *neoNode = new Node();

  // Convert the node to string
  neoNode->string = s;

  // Flag to check if num (number in spanish for flare :D)
  bool numero = true;
  // loop through each char in da string
  for (char neoChar : s)
  {
    if (!isdigit(neoChar))
    {
      // if it aint a number mark it down as not a number
      numero = false;
      break;
    }
  }

  if (numero && !s.empty())
  {
    neoNode->number = stoi(s);
    //  If string are numbers and not empty, stoi it
  }
  else
  {
    neoNode->number = 0;
    // else put in default integer (zer0)
  }

  neoNode->next = head;
  // next equal head (move it)
  head = neoNode;
  // Update head pointer to neoNode
  size++;
  // Increment size that was initalized earlier
}
