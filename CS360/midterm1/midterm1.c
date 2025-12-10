/*
 * Program Description:
 * ---------------------
 * This program reads a file containing 32-bit signed integers 
 * (one number per line). It calculates and prints:
 *   - The sum of all numbers
 *   - The average (mean)
 *   - The smallest number (min)
 *   - The largest number (max)
 *   - The median (middle value after sorting)
 *
 * How it works:
 *   1. The program takes the file name as a command-line argument.
 *   2. It opens the file and reads numbers one by one.
 *   3. It stores numbers in an array that grows dynamically:
 *        - It starts with a small amount of memory (malloc).
 *        - If more space is needed, it resizes the array (realloc).
 *   4. While reading, it keeps track of the sum, min, and max.
 *   5. After reading, it sorts the array using insertion sort.
 *   6. It calculates average = sum / count, and median from the middle values.
 *   7. Finally, it prints the results.
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    // Check if the user gave a filename
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;  // Exit if no filename given
    }

    // Try to open the file for reading
    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]); // Print error if it can't open the file
        return 1;
    }

    // Start with space for 1024 numbers
    size_t capacity = 1024; 
    size_t count = 0;        // How many numbers we’ve actually read so far

    // malloc = "memory allocate"
    // It reserves a block of memory big enough to hold 'capacity' integers
    int32_t *values = malloc(capacity * sizeof *values);
    if(!values) {
        perror("malloc"); // If malloc fails, print an error
        fclose(f);
        return 1;
    }

    int32_t currentValue;       // Temporary holder for the number being read
    long long TotalSum = 0;     // Keep running total of all numbers
    int hasMinMax = 0;          // Flag so we know when to set min/max the first time
    int32_t minValue = 0;       // Smallest number
    int32_t maxValue = 0;       // Largest number

    // Read integers from the file one by one
    while (fscanf(f, "%d", &currentValue) == 1) {
        // If we run out of space, we need to grow the array
        if (count == capacity) {
            size_t nCapacity = capacity * 2; // Double the capacity

            // realloc = "reallocate memory"
            // It tries to expand the old memory block to fit nCapacity
            int32_t *increase = realloc(values, nCapacity * sizeof *values);

            if (!increase) { // If realloc fails, we must free memory and quit
                free(values);
                fclose(f);
                return 1;
            }

            // If realloc succeeds, point values to the new bigger block
            values = increase;
            capacity = nCapacity;
        }

        // Save the new number into the array
        values[count++] = currentValue;

        // Update the running sum
        TotalSum += currentValue;

        // Update min and max values
        if (!hasMinMax) { // First number sets both min and max
            minValue = maxValue = currentValue;
            hasMinMax = 1;
        } 
        else {
            if (currentValue < minValue) 
                minValue = currentValue;
            if (currentValue > maxValue)
                maxValue = currentValue;
        }
    }
    fclose(f); // Always close the file when done

    // Sorting step:
    // We use "Insertion Sort" here.
    // How it works:
    //   - Start from the second element
    //   - Compare it to previous elements
    //   - Move larger elements to the right until the correct spot is found
    //   - Insert the current element into its correct spot
    // This repeats for all numbers, leaving the array sorted.
    for (size_t i = 1; i < count; ++i) {
        int32_t key = values[i];   // Current number to be inserted
        size_t j = i;
        // Keep shifting numbers that are bigger than 'key' to the right
        while (j > 0 && values[j-1] > key){
            values[j] = values[j-1];
            j--;
        }
        // Place 'key' into its correct position
        values[j] = key;
    }

    double average = 0.0;
    double median = 0.0;

    // If there were no numbers, default min/max to 0
    if (count == 0) {
        minValue = maxValue = 0;
    }
    else {
        // Average = total sum divided by how many numbers we read
        average = (double)TotalSum / (double)count;

        // Median depends on whether count is odd or even:
        // If odd, it's the middle element
        // If even, it's the average of the two middle elements
        size_t middleLeft = (count - 1) / 2;
        size_t middleRight = (count / 2);
        double Nmedian = 0.0;

        // Add the two middle elements
        for (size_t i = 0; i < count; ++i) {
            if (i == middleLeft || i == middleRight) 
                Nmedian += values[i];
        }

        median = Nmedian / 2.0; // Divide by 2 to get the middle
    }

    // Print results in nice formatted style
    printf("%-10s = %lld\n", "Sum", TotalSum);
    printf("%-10s = %.2f\n", "Average", average);
    printf("%-10s = %d\n", "Min", minValue);
    printf("%-10s = %d\n", "Max", maxValue);
    printf("%-10s = %.2f\n", "Median", median);

    // Free the dynamically allocated memory to avoid memory leaks
    free(values);
    return 0;
}
