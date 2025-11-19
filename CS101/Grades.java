import java.util.Scanner;

class Grades {
    public static void main(String[] args) {
        Scanner s = new Scanner(System.in);
        System.out.println("Print yo score out homie");
        if (s.hasNextInt()) {
            int score = s.nextInt(); // Corrected variable name and moved it inside the if block
            char letterGrade = 'Z'; // Initialized the variable letterGrade

            if (score >= 90) {
                letterGrade = 'A';
            } else if (score >= 80) {
                letterGrade = 'B';
            } else {
                letterGrade = 'F';
            }

            System.out.println("Your letter grade is " + letterGrade);
        } else {
            System.out.println("Fuck you I asked for a number"); // Corrected the print statement
        }
        s.close(); // Close the scanner
    }
}
