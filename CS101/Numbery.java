import java.util.Scanner;

class Numbery {
    public static void main(String[] args) {
        boolean isrunning = true;
        Staty mystats = new Staty();
        Scanner s = new Scanner(System.in);
        do {
            do {
                System.out.print("Enter an integer (\"quit\" to quit): ");
                if (!s.hasNextInt()) {
                    if (s.next().equals("quit")) {
                        isrunning = false;
                        break;
                    }
                    System.out.println("You did not enter an integer, try again.");
                } else {
                    int value = s.nextInt();
                    mystats.addValue(value);
                    break;
                }
            } while (true);
        } while (isrunning);
        s.close();

        System.out.format("Number of values entered = %d%n", mystats.getNumValues());
        System.out.format("Sum of all values        = %d%n", mystats.getSum());
        System.out.format("Product of all values    = %d%n", mystats.getProduct());
        System.out.format("Biggest value            = %d%n", mystats.getMax());
        System.out.format("Smallest value           = %d%n", mystats.getMin());
        System.out.format("Average of all values    = %.2f%n", mystats.getAverage());
    }
}
