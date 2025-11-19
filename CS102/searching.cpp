#include <iostream>
#include <vector>
using namespace std;
 
/* Binary search function that returns the 
   location of x `in given a vector arr[l..r] if present,
   otherwise -1
*/
int binarySearch(vector <int> arr, int x, int l, int r)
{
    
        int m = l + (r - l) / 2;
 
        // Check if x is present at midpoint
        if (arr[m] == x)
            return m;
 
        // If x greater, ignore left half of vector
		else if (l != r) {
			if (arr[m] < x) {
				l = m+ 1;
			
				}
			else {
				r = m -1;
			
			}
			return binarySearch(arr, x, l ,r);
		}
		else{
			return -1;
		}
            
 
        // If x is smaller, ignore right half of vector

    
 
    /* Element was not in the vector. */

}
 
int main(void)
{
    vector <int> data = { 2, 3, 4, 10, 40 };  // Remember this line requires C++11
    int x = 10;
	int l = data[0]; 
	int r = data[4];
    /* Hint: to make it recursive, adding parameters... specifically 2 parameters... will help. */
    int result = binarySearch(data, x, l, r);
    (result == -1) ? cout << "Element is not here"
                   : cout << "Element is here" << result;
    cout << endl;
    return 0;
}



