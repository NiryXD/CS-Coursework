#include <iostream>
#include <vector>
using namespace std;

int main() {
    int n, r;
    char d;

    while (cin >> n >> r >> d) {
        vector<int> arr(n);
        for (int i = 0; i < n; i++) {
            cin >> arr[i];
        }

        r %= n; // Reduce unnecessary rotations
        if (d == 'L') {
            // Left rotation: Slice and concatenate
            vector<int> temp(arr.begin() + r, arr.end());
            temp.insert(temp.end(), arr.begin(), arr.begin() + r);
            arr = temp; // Replace the original array with the rotated version
        } else if (d == 'R') {
            // Right rotation: Slice and concatenate
            vector<int> temp(arr.end() - r, arr.end());
            temp.insert(temp.end(), arr.begin(), arr.end() - r);
            arr = temp; // Replace the original array with the rotated version
        }

        for (int i = 0; i < n; i++) {
            if (i > 0) cout << " ";
            cout << arr[i];
        }
        cout << endl;
    }

    return 0;
}
