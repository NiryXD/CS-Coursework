#include <iostream>
#include <vector>
#include <iomanip>

using namespace std;

void printVector(vector <int> vec ){
    int size = vec.size();
    for(int i = 0; i < size; i++){
        cout << " " << vec[i];
    }
    cout << endl;
}
void swat(vector <int> &vec , int i1, int i2){
    int temp = vec[i1] ;
    vec[i1] = vec[i2];
    vec[i2] = temp;
}
void bubbleSort(vector <int> &vec){
    bool swapCall = true;
    while(swapCall){
        swapCall = false;
    int i;
    for (int i = 0; i < vec.size() - 1; i++){
        if(vec[i] > vec[i+1]){
            int a = vec[i];
            int b = vec[i+1];
            vec[i] = b;
            vec[i+1] = a;
            swapCall = true;
            }
        }
    }

}
int main(){
    vector<int> data = {5, 1, 4, 2, 8,};

    cout <<"First Vector:\n";
    printVector(data);

    bubbleSort(data);

    cout << "Sorted Vector in Acending Order:\n";
    printVector(data);
}