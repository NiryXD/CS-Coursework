import java.util.Scanner;

class testes {
     public static void main(String[] args) {
          Scanner sc = new Scanner(System.in); // Renamed 's' to 'sc' for clarity
          String input = sc.next(); // Renamed 's' to 'input' for clarity
          int i;
          for (i = input.length() - 1; i >= 0; i--) { // Changed 's.lenght()' to 'input.length()'. Also, replaced 'i -= 1' with 'i--' for simplicity.
               System.out.print(input.charAt(i)); // Changed 's' to 'input' for clarity
          }
          sc.close();
     }
}
