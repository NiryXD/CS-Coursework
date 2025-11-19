/* Program Name: First Submission
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Compare two salaries, and budget accordingly. */
import java.util.Scanner;

class SalaryComparison {
    public static void main(String[] args) {
     Scanner s = new Scanner(System.in);
        int salaryInput1, salaryInput2;
        
        /* The code below asks the user to enter the salaries of the jobs they want to compare */
        System.out.print("Enter the salary for the first job (no commas): $");
        salaryInput1 = s.nextInt();
        System.out.print("Enter the salary for the second job (no commas): $");
        salaryInput2 = s.nextInt();
        s.close();

        /* What I'm doing here is initalizing job1 and job2. I assign the previously inputed data of incomes to either job1 or job2
         * THP is Take Home Pay which isn't the gross amount of money, it's what you have after taxes. The numbers are ran through
         * that so we can calculate what the user can actually use.*/
        Budget job1 = new Budget(salaryInput1);
        Budget job2 = new Budget (salaryInput2);
        job1.calculateTHP();
        job2.calculateTHP();

        /* This takes the percentage of the inputted income after THP. 50, 30, and 20 for their respective category of importance.
         * The following block of code is basically the same but with job2. */
        double needs, wants, savings;
        needs = job1.calculateBudgetCategory(50);
        wants = job1.calculateBudgetCategory(30);
        savings = job1.calculateBudgetCategory(20);
        System.out.format("Monthly Take Home Salary for Job 1: $%.2f\n", job1.getMonthlyTakeHomePay());
        System.out.format("By the 50/30/20 Budget, spend up to $%.2f on needs (Food, shelter, etc)\n", needs);
        System.out.format("                        spend up to $%.2f on wants (Hobbies, travel, etc)\n", wants);
        System.out.format("                        and up to   $%.2f on savings (retirement, emergency fund)\n", savings);

        /*This is the last block, analyzing the second income. One thing I'd like to do is move the THP here so it'd be alongside
        all the other factors. This way it is symetrical to the code that prints out what the user can see. Just a quality of life
        change.*/
        needs = job2.calculateBudgetCategory(50);
        wants = job2.calculateBudgetCategory(30);
        savings = job2.calculateBudgetCategory(20);
        System.out.format("Monthly Take Home Salary for Job 2: $%.2f\n", job2.getMonthlyTakeHomePay());
        System.out.format("By the 50/30/20 Budget, spend up to $%.2f on needs (Food, shelter, etc)\n", needs);
        System.out.format("                        spend up to $%.2f on wants (Hobbies, travel, etc)\n", wants);
        System.out.format("                        and up to   $%.2f on savings (retirement, emergency fund)\n", savings);
    }
}