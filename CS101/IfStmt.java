import java.util.Scanner;
class IfStmt {
     public static void main (String[] args) {
          int height = 15;
          int age = 40;
          boolean Season = true;

          if (height >= 48 && age >= 12 && Season) {
               System.out.println ("Yo wassup");
          }
          if (height <= 20 && age >= 30 ) {
               System.out.println ("lol Short");
          }
          else {
               System.out.println("Fuck outta here");
          }

          String x = "abc";
          String y;
          System.out.println ("Type 'abc'");
          Scanner s = new Scanner(System.in);
          y = s.next();
          s.close();

          if (x.equals(y)) {
               System.out.println ("String are the same");
          }
          else {
               System.out.println ("They aren't the same dummy");
          }
          System.out.println(x);
          System.out.println(y);
          }
    }
