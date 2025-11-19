// Noah Cash
// ncash3
// Description: recursively sorts a list
// using partition, concatenate, and qsort
// functions
// quick.cpp

#include "volsort.h"

#include <iostream>

// Prototypes

Node *qsort(Node *head, bool numeric);
void  partition(Node *head, Node *pivot, Node *&left, Node *&right, bool numeric);
Node *concatenate(Node *left, Node *right);

// Implementations

// point to sorted version of the list
void quick_sort(List &l, bool numeric) {
	if (l.size == 0){
		return;
	}
	l.head = qsort(l.head, numeric);
}

Node *qsort(Node *head, bool numeric) {
	

	// base case : check if list is empty or one element
	if((!head) || (!head->next)){
		return head;
	}

	// make the pivot at the head
	Node *pivot = head;

	Node *left = nullptr;
	Node *right = nullptr;

	// parition around the pivot
	partition(head->next, pivot, left, right, numeric);

	// recursively sort left and right subparts
	if (left){
		left = qsort(left, numeric);
	}
	if (right){
		right = qsort(right, numeric);
	}
	
	// concatenate pivot w/ right list
//	Node *temp = concatenate(pivot, right);
	pivot->next = right;	
	// concatenate the left list w/ the right
	return concatenate(left, pivot);


}

void partition(Node *head, Node *pivot, Node *&left, Node *&right, bool numeric) {

	while(head){
		
		bool lessThan = false;
		
		// determines if value is less than pivot or not
		if (numeric) { // for numbers
			lessThan = (head->number) < (pivot->number);
		} else { // for letters
			lessThan = head->string.compare(pivot->string) < 0;
		}

		// store current node and move pointer over
		Node *temp = head;
		head = head->next;
		
		// assigns node to either left or right
		// depending if its less or greater than pivot
		if (lessThan) {
			temp->next = left;  
			left = temp;
		} else {
			temp->next = right; 
			right = temp;
		}
		
	}
}

Node *concatenate(Node *left, Node *right) {
	
	// for when left node is empty
	if(!left){
		return right;
	}

	// go to the end of the left list
	Node *temp = left;
	while (temp->next){
		temp = temp->next;
	}

	// the concatenation
	temp->next = right;
	return left;

}
