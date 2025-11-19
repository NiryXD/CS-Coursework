// Challenge 02: Closest Numbers
// Name: Claude Assistant
// Brief description:
// This code solves the closest numbers problem by first sorting the array
// and then finding consecutive pairs with minimum absolute difference.

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <vector>
using namespace std;

int main(int argc, char *argv[]) {
    int N;
    while (cin >> N) {
        vector<int> nums(N);
        
        // Read all numbers for this test case
        for (int i = 0; i < N; i++) {
            cin >> nums[i];
        }
        
        // Sort array to make finding minimum differences easier
        sort(nums.begin(), nums.end());
        
        // Initialize minDiff to the largest possible difference in the array
        int minDiff = nums[N - 1] - nums[0];
        
        // Find minimum difference
        for (int i = 0; i < N - 1; i++) {
            minDiff = min(minDiff, abs(nums[i + 1] - nums[i]));
        }
        
        // Output all pairs with minimum difference
        bool firstPair = true;
        for (int i = 0; i < N - 1; i++) {
            if (abs(nums[i + 1] - nums[i]) == minDiff) {
                if (!firstPair) cout << " ";
                if (nums[i] == nums[i + 1]) {
                    cout << nums[i] << " " << nums[i];  // Print repeated numbers separately
                } else {
                    cout << nums[i] << " " << nums[i + 1];
                }
                firstPair = false;
            }
        }
        cout << endl;
    }
    return EXIT_SUCCESS;
}
