import java.util.Scanner;

class NeoCalc {
    private final static int VECTOR_SIZE = 3;
    private final static int MATRIX_SIZE = VECTOR_SIZE * VECTOR_SIZE;

    public static void main(String[] args) {
        // Create a new array of doubles called matrix that can store MATRIX_SIZE number of values.
        double[] matrix = new double[MATRIX_SIZE];

        // Create a new array of doubles called vector that can store VECTOR_SIZE number of values.
        double[] vector = new double[VECTOR_SIZE];

        Scanner s = new Scanner(System.in);

        // Input values for matrix using a for loop.
        System.out.format("Enter %d matrix values: ", MATRIX_SIZE);
        for (int i = 0; i < MATRIX_SIZE; i++) {
            matrix[i] = s.nextDouble();
        }

        // Input values for vector using a for loop.
        System.out.format("Enter %d vector values: ", VECTOR_SIZE);
        for (int i = 0; i < VECTOR_SIZE; i++) {
            vector[i] = s.nextDouble();
        }

        s.close();

        // Declare the result array outside of the main method.
        double[] result = multiply(matrix, vector);

        // Print the result vector.
        System.out.print("Result = [");
        printVector(result);
        System.out.println("]");
    }

    private static void printVector(double[] arr) {
        // Print the array 'arr' with spaces between values.
        // Take note that there are spaces between each value and that there is NO space after the last value printed!
        for (int i = 0; i < arr.length; i++) {
            System.out.print(arr[i]);
            if (i < arr.length - 1) {
                System.out.print(" ");
            }
        }
    }

    private static double[] multiply(double[] matrix, double[] vector) {
        // Create a new array called result that stores VECTOR_SIZE number of values.
        double[] result = new double[VECTOR_SIZE];

        // Calculate result. There are VECTOR_SIZE number of values in the result.
        // You will need TWO for loops. The *outer* for loop will loop through each row,
        // whereas the *inner* for loop will loop through each column.
        for (int i = 0; i < VECTOR_SIZE; i++) {
            for (int j = 0; j < VECTOR_SIZE; j++) {
                result[i] += matrix[i * VECTOR_SIZE + j] * vector[j];
            }
        }

        // Return the result array out of the method.
        return result;
    }
}
