import java.util.Scanner;
class NumbersMain {
   public static void main(String[] args) {
      boolean isrunning = true;
      Statistics mystats = new Statistics();
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
            }
            else {
               int value = s.nextInt();
               mystats.addValue(value);
               break;
            }
/*Above I used an else statement to process the integers inputted by the user. It first runs though a predetermined code of whether
 * if it said 'quit' and after that made sure it was an actual proccesable input. If it doesn't meet the prior requirements then it's 
 * allowed to be ran through the things within the 'mystat' value. 
 */
         } while (true);
      } while (isrunning);
      s.close();
      
      /* All the print statements were done, all I needed to do was to fill in the names of the variables. I just had to fit the 
       * rubric for what the final program should look like.
       */
               System.out.format("Number of values entered = %d%n", mystats.getNumValues());
               System.out.format("Sum of all values        = %d%n", mystats.getSum());
               System.out.format("Product of all values    = %d%n", mystats.getProduct());
               System.out.format("Biggest value            = %d%n", mystats.getMax());
               System.out.format("Smallest value           = %d%n", mystats.getMin());
               System.out.format("Average of all values    = %.2f%n", mystats.getAverage());
   }
}