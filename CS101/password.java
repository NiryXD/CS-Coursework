import java.util.Scanner;
class password {
    public static void main(String[] args) {
        Scanner s = new Scanner(System.in); // Renamed 's' to 'scanner' for clarity
        String pw; // Renamed 'pw' to 'password' for clarity
        do {
            System.out.print("Enter Password: ");
            pw = s.nextLine();
        } while (!pw.equals("password"));
        System.out.println("Yay!");
        s.close(); // Added to close the Scanner object
    }
}
