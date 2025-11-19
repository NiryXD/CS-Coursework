/* Program Name: Equation
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Asks for inputs, and when given will fill out the outlined equation  */
import java.util.Scanner;
class Equation {
   public static void main (String[] args) {
      double a;
      double b;
      double c;
      double d;
      
      Scanner s = new Scanner (System.in);
      System.out.println ("Enter four doubles (a * b - c + d): ");
      a = s.nextDouble();
      b = s.nextDouble();
      c = s.nextDouble();
      d = s.nextDouble();
      s.close();

      double result =  a * b - c + d;
      System.out.format ("The first number is %.2f and the second number is %.2f\n", a, b );
   
      System.out.format("%.2f * %.2f - %.2f + %.2f = %.2f\n", a, b, c, d, result);
   }
}