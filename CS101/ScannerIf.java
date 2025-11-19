import java.util.Scanner;
class ScannerIf {
    public static void main(String [] args){
          Scanner s = new Scanner (System.in);
          //These represent the coin values
          int cents, numQ, numD, numN, numP;
          int qValue = 25;
          int nValue = 5;
          int dValue = 10;
          int pValue = 1;

          System.out.println ("Enter the amount of change (0-99): ");
          if (!(s.hasNextInt())); {
               System.out.println ("Need to have an integer input dumbass");
               s.close();
               return;
          }
          cents = s.nextInt();
          if (! (cents >= 0 && cents <= 99)) {
               s.close(); 
               return;
          }
          cents = s.nextInt();
          //Calculate the number of coins each
          qValue = cents / 25;
          int userRemain = cents % 25;
          dValue = userRemain / 10;
          int userRemain2 = userRemain % 10;
          nValue = userRemain2 / 5;
          int userRemain3 = userRemain2 % 5;
          pValue = userRemain3 / 1;

          //Print output
          
          System.out.println ("Number of quarters: " + qValue);
          System.out.println ("Number of dimes: " + dValue);
          System.out.println ("Number of Nickles: " + nValue);
          System.out.println ("Number of Penies: " + pValue);
    }
}