
/* Program Name: First Submission
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Exponential Loop program */

import java.util.Scanner;

class Loops {
    public static void main(String[] args) {
        double exponent;
        Scanner s = new Scanner(System.in);
        String quit;
/*Above is a pretty basic introduction to the program. I introduced variables exponent and quit. 'Exponent' to store the user's input,
 * and 'quit' is used to quit the program. I'm sure theres a more efficent way, but this is the only way I know how. The scanner is 
 * introduced in order to ask for what double the user wants. Below is the Do-While statements requireed by the assignment. First in the
 * inital do brackets we have '(s.hasNextDouble())' in order to see if what was inputted was really a integer or not. And if it was 
 * a number the program will use 'Math.exp(exponent)' in order to exponent it. 'System.out.format' is used to shorten the long number
 * down to four decimal points. */
        do {
            System.out.print("Enter exponent: ");
            if (s.hasNextDouble()) {
                exponent = s.nextDouble();
                System.out.format("Result = %.4f%n", Math.exp(exponent));
            } 
            else {
                quit = s.next();
                if (quit.equals("quit")) {
                    break;
            } 
            else {
                    System.out.println("Invalid input.");
            }
            }
        } while (true);
/* If the user didn't input a number the else statement takes care of that. Like if the user types in 'quit' that'll awaken the 
 * quit variable which will 'break' the program. If that doesn't trigger, the responsibility will fall upon the final 'else' statement
 * which will have a general answer of 'Invalid input', this response goes for symbols and letters. Or maybe even Chinese characters. The 
 * final statement of the program is the 's.close();' line, which closes the scanner the user has used previously.
 */
        s.close();
    }
}