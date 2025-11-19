
/* Program Name: First Submission
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: Mod & Integer Division Program */
import java.util.Scanner;

class SecondsRev {
    public static void main(String[] args) {
        int dataInSeconds, hours, minutes, seconds;

        Scanner s = new Scanner(System.in);

        /*
         * The code below is meant for the user to input the seconds they want to get
         * converted.
         * Then it stores it and closes the scanner
         */
        System.out.println("Enter the elapsed time in seconds: ");
        dataInSeconds = s.nextInt();
        s.close();

        /*
         * All the integers were declared in the first few lines of code, except
         * reminaderSecond.
         * The data the user inputed would first go through the hours, the remanding
         * seconds would go
         * into remainderSecond which will be used to find the minutes. When minutes are
         * found the
         * remainder trickles down to seconds.
         */
        hours = dataInSeconds / 3600;
        int remainderSecond = dataInSeconds % 3600;
        minutes = remainderSecond / 60;
        seconds = remainderSecond % 60;

        /*
         * TODO: Output the data as
         * "___ seconds is ___ hours, ___ minutes, ___ seconds."
         * Use System.out.format OR System.out.println. See example for output.
         */

        System.out.println(
                dataInSeconds + " seconds is " + hours + " hours, " + minutes + " minutes, " + seconds + " seconds.");
    }
}