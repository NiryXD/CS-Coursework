import java.util.Scanner;
class MatrixCalc {
    private final static int VECTOR_SIZE = 3;
    private final static int MATRIX_SIZE = VECTOR_SIZE * VECTOR_SIZE;
    public static void main(String[] args) {
/*Above was the predetermined values of the variables I introduce below. Utilizing 'substring' I can create variables for
 * 'MATRIX_SIZE' and 'VECTOR_SIZE'. */
        double[] matrix = new double[MATRIX_SIZE];
        double[] vector = new double[VECTOR_SIZE];
        Scanner s = new Scanner(System.in);
        System.out.printf("Enter %d matrix values: ", MATRIX_SIZE);
        for (int p = 0; p < MATRIX_SIZE; p++) {
            matrix[p] = s.nextDouble();
        }
/* Using a loop we can take in what the user types in and store it in a matrix. The aformentioned variables determine this. So p is set 
 * less than MATRIX_SIZE, which would give the user the opportunity to put in nine inputs. Using this method, the user can change the
 *  predetermined number to four or any other digit. Because Matrix_Size is Vector_Size squared.
 * The same thought process is applied to the following line of code, but instead of Matrix_Size, I used Vector_Size. */
        System.out.printf("Enter %d vector values: ", VECTOR_SIZE);  
        for (int p = 0; p < VECTOR_SIZE; p++) {
            vector[p] = s.nextDouble();
        }
        s.close();
/* Above we closed the Scanner because it's no longer in use. Below is what the code computates given the inputs from the user. The 
 * examples specifically show that three decimal points are required and that is reflected in the printf statement and it's use of
 * '%.3f'. */
        double[] result = multiply(matrix, vector);
        System.out.printf("Result = [%.3f %.3f %.3f]%n", result[0], result[1], result[2]);
    }
    private static void printVector(double[] arr) {
        int lastOne = arr.length - 1;
        for (int i = 0; i < lastOne; i++) {
            System.out.printf("%.3f ", arr[i]);
        }
        System.out.printf("%.3f%n", arr[lastOne]);
    }
/* Above is the code for an array that keeps the end value of the overall code. With the added requirement of not allowing a 'if' 
statement, I had to circumvent this problem through the introduction of a variable of my own called 'lastOne' refering to the last
element in array. We can run through the array's index smoothly by starting from -1 and from there it's a basic loop. 
 * Below is the code for taking what is stored in the code above and then do the mathmatical computations required by the assignment. 
 * This is done through nested for loops, each taking care of rows and columns respectively. At the very end of the code it 'returns result;'
 * which is used when printing what the user has to see.
 */
private static double[] multiply(double[] matrix, double[] vector) {
    double[] result = new double[VECTOR_SIZE];
    for (int p = 0; p < VECTOR_SIZE; p++) {
        for (int a = 0; a < VECTOR_SIZE; a++) {
            result[p] += matrix[p * VECTOR_SIZE + a] * vector[a];
        }
    }
    return result;
}
}
