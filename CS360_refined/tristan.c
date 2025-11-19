#include <stdio.h>
#include <stdlib.h>

// function for qsort to compare integers
// return negative if b is greater than a, or positive
// if a is greater than b. zero if equal 
int check(const void a, const voidb) {
    int x = (const int)a;
    int y = (const int)b;
    return (x > y) - (x < y);
}

// check to see if usage is correct with filename 
int main(int argc, char argv[])
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }


    FILEfp = fopen(argv[1], "r"); // file is opened 
    if (!fp) {
    perror("Error Opening"); // error check to see if it opens 
    return 1;
}
    int capacity = 10; // create default array 
    int arr = malloc(capacity sizeof(int)); // allocate memory
    if (!arr) {
        perror("Memory problem"); // error check to see if memory leak
        fclose(fp);
        return 1;
    }

    int sum = 0; // sum value 
    int amount = 0; // mean value 
    int min; // minimum value 
    int max; // maximum value 
    int value;  // current value
while (fscanf(fp, "%d", &value) == 1) { // files read in from fp (file)
        if (amount == capacity) { // if the amount of values is same as capacity, capacity is doubled 
            capacity = 2;
            inttemp = realloc(arr, capacity * sizeof(int)); // new allocation to adjust new capacity 
            if (!temp) {
                perror("Memory problem 2"); // error checks second array 
                free (arr);
                fclose(fp);
                return 1;
            }
            arr = temp; // array is udpated to new memory 
        }
        arr[amount++] = value; // current value is stored 
        sum += value;  // value is added to the sum 
        if(amount == 1) {
            min = max = value; // updates min and max values 
        } else {
            if (value < min) min = value; // if value is less than min, than min is updated 
            if (value > max) max = value; // if value is greater than max, the max is upadted 
        }
    }
    fclose(fp); // file closed 
    if (amount == 0) {
        printf("no amount"); // if numbers are 0 then throw error 
        free(arr);
        return 1; 
    }
    double mean = (double)sum / amount; // mean = sum / number amount 
    qsort(arr, amount, sizeof(int), check); // goes through array to find median
    double median;
    if (amount % 2 == 1) { 
        median = arr[amount / 2]; // odd number of elements takes middle one
    } else {
        median = (arr[amount / 2 - 1] + arr[amount / 2]) / 2.0; // even number of elements finds average of two middle ones
    }
    // results are printed in left justified 10 character field
    // (%-10s=) %d = int, %.2f = double
    printf("%-11s= %d\n", "Sum", sum);
    printf("%-11s= %.2f\n", "Average", mean);
    printf("%-11s= %d\n", "Min", min);
    printf("%-11s= %d\n", "Max", max);
    printf("%-11s= %.2f\n", "Median", median);
    free(arr); // free memory 
    return 0;

}
