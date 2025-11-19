/* Program Name: First Submission
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Mod & Integer Division Program */
import java.util.Scanner;
class ElapsedSeconds {
   public static void main(String[] args) {
Scanner s = new Scanner (System.in);
`
      System.out.print("Enter the elapsed time in seconds: ");
          int dataInSeconds;
  dataInSeconds = s.nextInt();
  int Hours = dataInSeconds / 3600;
  int remainderSecond = dataInSeconds % 3600;
  int Minute = remainderSecond / 60;
  int remainderSecond = remainderSecond % 60;


      s.close();
    }
}
