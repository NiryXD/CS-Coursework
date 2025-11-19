import java.util.Scanner;

class Loops2 {
    public static void main(String[] args) {
        double exponent;
        Scanner s = new Scanner(System.in);
        String quit;

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

        s.close();
    }
}