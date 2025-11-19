/* Program Name: First Submission
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Takes in current salary, evaluates rating, and then calculates a raise. */
import java.util.Scanner;
class Raise {
    public static void main(String[] args){
        double currentSalary; // current annual salary
        int rating;           // performance rating
        double raise;         // dollar amount of the raise
        Scanner scan = new Scanner(System.in);

        /* So first TODO is put the Scanner to use. We use 'scan' instead of the usual 's' beacuse that's how the scanner was 
        introduced in this program. We follow that up with nextInt and not nextDouble because the examples shown on the 
        assignment page don't utilize cents so we don't need to take that into consideration. Then there's a pre-written sentence
        that prompts the user to rate one's preformance and that also takes an integer so I use the same command.*/  
        System.out.print("Enter the annual salary: $");
        currentSalary = scan.nextInt();
        System.out.print("Enter the performance rating. 1 = Excellent, 2 = Good, 3 = Poor: ");
        rating = scan.nextInt();
        scan.close();
        
        if (currentSalary <= 20000) {
            System.out.println("Salary is too low. Must be over $20,000.");
            return;
        }

        /* Below if the employee got a rating of one, they'd get a 8.8 percent raise represented as 0.088 because if it was 0.88 that would
        represent 88 percent. If the inputted value wasn't 1, the program will keep going down to 2, and then three. The issue I had was
        having two else if statements. This was resloved by having just an else statement at the end. Below the else if statements is
        what the users going to see, which is how much their pay is going to increase by and how much their new total is.
        */ 
        if (rating == 1) {
            raise = currentSalary * 0.088;
        } else if (rating == 2) {
            raise = currentSalary * 0.045;
        } else {
            raise = currentSalary * 0.025;
        } 
        
        System.out.format("Amount of your raise: $%.2f\n", raise);
        System.out.format("Your new salary: $%.2f\n", currentSalary + raise);
    }
}