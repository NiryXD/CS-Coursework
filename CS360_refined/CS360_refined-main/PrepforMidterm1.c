#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

int main(int argc, char *argv[])
{
    if (argc < 2) {
        printf("Usage: %s <file>\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "r");
    if (!f) {
        perror(argv[1]);
        return 1;
    }

    size_t capacity = 1024;
    size_t count = 0;
    int32_t *values = malloc(capacity * sizeof *values);
    if(!values) {
        perror("malloc");
        fclose(f);
        return 1;
    }

    int32_t currentValue;
    long long TotalSum = 0;
    int hasMinMax = 0;
    int32_t minValue = 0;
    int32_t maxValue = 0;

    while (fscanf(f, "%d", &currentValue) == 1) {
        if (count == capacity) {
            size_t nCapacity = capacity * 2;
            int32_t *increase = realloc(values, nCapacity * sizeof *values);
            if (!increase) {
                free(values);
                fclose(f);
                return 1;
            }
            values = increase;
            capacity = nCapacity;
        }

        values[count++] = currentValue;
        TotalSum += currentValue;

        if (!hasMinMax) {
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
fclose(f);


for (size_t i = 1; i < count; ++i) {
    int32_t key = values[i];
    size_t j = i;
    while (j > 0 && values[j-1] > key){
        values[j] = values[j-1];
        j--;
    }
    values[j] = key;
}

double average = 0.0;
double median = 0.0;

if (count == 0) {
    minValue = maxValue = 0;
}
else {
    average = (double)TotalSum / (double)count;

    size_t middleLeft = (count - 1) / 2;
    size_t middleRight = (count / 2);
    double Nmedian = 0.0;

    for (size_t i = 0; i < count; ++i) {
        if ( i == middleLeft || i == middleRight) 
            Nmedian += values[i];
        
    }

    median = Nmedian / 2.0;
}

printf("%-10s = %lld\n", "Sum", TotalSum);
printf("%-10s = %.2f\n", "Average", average);
printf("%-10s = %d\n", "Min", minValue);
printf("%-10s = %d\n", "Max", maxValue);
printf("%-10s = %.2f\n", "Median", median);

free(values);
return 0;

}



