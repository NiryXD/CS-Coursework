import java.util.Scanner;

class ScannerIftwo {
    public static void main(String[] args) {
        Scanner s = new Scanner(System.in);
        // These represent the coin values
        int cents, numQ, numD, numN, numP;
        int qValue = 25;
        int nValue = 5;
        int dValue = 10;
        int pValue = 1;

        System.out.println("Enter the amount of change (0-99): ");
        if (!s.hasNextInt()) { // Remove the semicolon here
            System.out.println("Need to have an integer input dumbass");
            s.close();
            return;
        }
        cents = s.nextInt();
        if (!(cents >= 0 && cents <= 99)) {
            s.close();
            return;
        }

        // Calculate the number of coins
        numQ = cents / qValue;
        int userRemain = cents % qValue;
        numD = userRemain / dValue;
        int userRemain2 = userRemain % dValue;
        numN = userRemain2 / nValue;
        numP = userRemain2 % nValue;

        // Print output
        System.out.println("Number of quarters: " + numQ);
        System.out.println("Number of dimes: " + numD);
        System.out.println("Number of Nickels: " + numN);
        System.out.println("Number of Pennies: " + numP);
        s.close();
    }
}