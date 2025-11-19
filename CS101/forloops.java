    /* Program Name: First Submission
     * Student Name: Ar-Raniry Ar-Rasyid
     * Student ID: 000-66-3921
     * NetID: jzr266
     * Description: Gives you sum or product of multiple numbers */
import java.util.Scanner;

class forloops {
    public static void main(String[] args) {
        String input;
        int n;
        Scanner s = new Scanner(System.in);
/*Above introduces the Scanner and variables input and n. Whatever the user types in is categorized as input and 'n' is used later on
 * for the if else statements. Below I create a continous loops through the use of a 'while' statement. The user is promted to the options
 * sum or product and is also provided the alternative route of quitting. Quitting breaks the sequence, while the other two options continue
 * to run along.
 */
        while (true) {
            System.out.print("Sum or product (type 'quit' to quit): ");
            input = s.next();
            if (input.equals("quit")) {
                break;
            } else if (!input.equals("sum") && !input.equals("product")) {
                System.out.println("Invalid operation, try again.");
                continue;
            }
            System.out.print("How many values? ");
            n = s.nextInt();
            if (input.equals("sum")) {
                double sum = 0;
                for (int i = 0; i < n; i++) {
                    System.out.print("Enter number " + (i + 1) + ": ");
                    sum += s.nextDouble();
/*Above is asking how many values the user is going to input using a if else statement to confirm the user selected sum. How this
 * stores inputs is it checks for 'n' which is how many the terms the user wants and compares it to another variable called 'i'.
 * 'i' must be under 'n' since it starts counting at zero. A double 'sum' is also introduced to add the existing and following numbers
 * that the user inputs. Below uses the same process for products, but instead of addition, it multiplies the terms. 
 */
                  }
                System.out.printf("Sum: %.3f\n", sum);
            } else {
                double product = 1;
                for (int i = 0; i < n; i++) {
                    System.out.print("Enter number " + (i + 1) + ": ");
                    product *= s.nextDouble();
                }
                System.out.format("Product: %.3f\n", product);
            }
/*To finish the program I need to show the user what their results were and this is done through shortening their result into
 * bite sized information by rounding it to the nearest hundreths place using float instead of print in System.out. Then the final step
 * was to close out the Scanner because there's no longer a use for it, unless the user decides to continue the loop by not typing in quit
 */
        }
        s.close();
    }
}
