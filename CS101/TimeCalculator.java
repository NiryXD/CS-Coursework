import java.util.Scanner;

public class TimeCalculator {
    public static void main(String[] args) {
        Scanner input = new Scanner(System.in);
        al number of seconds: ");
        int totalSeconds = input.nextInt();
        
        // Calculate hours, minutes, and remaining seconds using modulus
        // 
        // Hours
        // Prompt the user for the total number of seconds
        System.out.print("Enter the tot
        int hours = totalSeconds / 3600;           // 1 hour = 3600 seconds
        //Minutes
        int remainingSeconds = totalSeconds % 3600; // Remaining seconds after extracting hours
        int minutes = remainingSeconds / 60;        // 1 minute = 60 seconds
        //Second
        remainingSeconds = remainingSeconds % 60;    // Remaining seconds after extracting minutes
        
        // Display the result
        System.out.println("Hours: " + hours);
        System.out.println("Minutes: " + minutes);
        System.out.println("Seconds: " + remainingSeconds);
        
        // Close the scanner
        input.close();
    }
}