import java.util.Scanner;
class AdvancedCalc {
   public static void main(String[] args) {
      int left, right;
      int result;
      String operator;
      Scanner s = new Scanner(System.in);

     /*Above imports the scanners and introduces the variables we're going to use. Left, right and result arfe all integers since the user
      * would be asked to input whole numbers. The operator is classified as String because it takes symbols like '+' and '-'. Scanner is called
      's' so I'd ask for inputs by typing 's.next();'. Below me is where I put the Scanner to use and ask the user what operation they 
      want to use and the numbers too. Afterwards, closing the Scanner.
      */

      System.out.print("Enter <left> <op> <right>: ");
          left = s.nextInt();
          operator = s.next();
          right = s.nextInt();
      s.close();

      switch (operator) {
          case "+":
             result = left + right;
             break;
          case "-":
             result = left - right;
             break;
          case "*":
             result = left * right;
             break;
          case "%":
               result = left % right;
               break;
          case "^":
               result = (int) Math.pow(left, right);
               break;
          case "/":
             result = left / right;
             break;
          default:
             System.out.println("Invalid operator!");
             return;
       }
 /*Above I utilize switch, which enables me to tailor the problem to the clients needs. The right and left stay the same, but the 
  * bread and butter of this program is what operation the user wants to use. All the operators are the same, but to use the 
  exponential function we need to use 'Math.pow' since '^' can't be used. Dividing by zero will cause an error, but there's an 
  assumption that the user will use whole numbers. Of course, if the wrong characters are given, it will give the user the 
  message 'Invalid operator!'. Below prints out what the user will see. Which is the result of what they asked the program.
  */
       System.out.println("Result: " + result);
    }
 }