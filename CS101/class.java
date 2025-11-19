import java.util.Scanner;
class test{
     public static void main(String[] args) {
          Scanner s = new Scanner(System.in);
          String word = s.next();
          System.out.println(word.substring(3,5));
          System.out.println(word.substring(2));
          System.out.println((int)word.charAt(3));
          char myell = 108;

          System.out.println(myell);
          int p = (int) 0.25;
//Narrowing
          for (p = 0; p < 4; p++) {
               System.out.println("Hewwo");
 //Post fix increment, p++              
          }
          System.out.println(p);
//iteration - One run of the forLoops body
     }
}