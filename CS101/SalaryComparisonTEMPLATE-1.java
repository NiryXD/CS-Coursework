/* TODO: Header (5 parts) */
/*TODO: import the library for the Scanner */

class SalaryComparison {
    public static void main(String[] args) {
        /*TODO: Create a Scanner called s and instantiate it with System input */
        int salaryInput1, salaryInput2;
        
        /* TODO: Create comment for this block of code */
        System.out.print("Enter the salary for the first job (no commas): $");
        /* TODO: Store the response in salaryInput1 using the Scanner */
        System.out.print("Enter the salary for the second job (no commas): $");
        /* TODO: Store the response in salaryInput2 using the Scanner */
        s.close();

        /* TODO: Create comment for this block of code */
        /* TODO: Create an object called job1 of the class Budget and instantiate it with salaryInput1 (Hint: you will use an equals sign, the "new" keyword, and the constructor) */
        /* TODO: Create an object called job2 of the class Budget and instantiate it with salaryInput2 (Hint: you will use an equals sign, the "new" keyword, and the constructor) */
        /* TODO: Call the calculate Take Home Pay (calculateTHP) method on job1 (it's a void method) */
        /* TODO: Call the calculate Take Home Pay (calculateTHP) method on job2 (it's a void method) */

        /* TODO: Create comment for this block of code */
        double needs, wants, savings;
        needs = /* TODO: Call calculateBudgetCategory with 50 percent on job1 and store the return value in needs */
        wants = /* TODO: Call calculateBudgetCategory with 30 percent on job1 and store the return value in wants */
        savings = /* TODO: Call calculateBudgetCategory with 20 percent on job1 and store the return value in savings */
        System.out.format("Monthly Take Home Salary for Job 1: $%.2f\n", job1.getMonthlyTakeHomePay());
        System.out.format("By the 50/30/20 Budget, spend up to $%.2f on needs (Food, shelter, etc)\n", needs);
        System.out.format("                        spend up to $%.2f on wants (Hobbies, travel, etc)\n", wants);
        System.out.format("                        and up to   $%.2f on savings (retirement, emergency fund)\n", savings);

        /*TODO: Comment this code block */
        needs = /* TODO: Call calculateBudgetCategory with 50 percent on job2 and store the return value in needs */
        wants = /* TODO: Call calculateBudgetCategory with 30 percent on job2 and store the return value in wants */
        savings = /* TODO: Call calculateBudgetCategory with 20 percent on job2 and store the return value in savings */
        System.out.format("Monthly Take Home Salary for Job 2: $%.2f\n", job2.getMonthlyTakeHomePay());
        System.out.format("By the 50/30/20 Budget, spend up to $%.2f on needs (Food, shelter, etc)\n", needs);
        System.out.format("                        spend up to $%.2f on wants (Hobbies, travel, etc)\n", wants);
        System.out.format("                        and up to   $%.2f on savings (retirement, emergency fund)\n", savings);
    }
}