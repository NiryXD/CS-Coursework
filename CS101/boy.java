import java.util.Scanner;

class boy {
    /* Program Name: First Submission
     * Student Name: Ar-Raniry Ar-Rasyid
     * Student ID: 000-66-3921
     * NetID: jzr266
     * Description: Gives you sum or product of multiple numbers */
    public static void main(String[] args) {
        String input;
        int n;
        Scanner s = new Scanner(System.in);

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
                }
                System.out.printf("Sum: %.3f\n", sum);
            } else {
                double product = 1;
                for (int i = 0; i < n; i++) {
                    System.out.print("Enter number " + (i + 1) + ": ");
                    product *= s.nextDouble();
                }
                System.out.printf("Product: %.3f\n", product);
            }
        }
        s.close();
    }
}
