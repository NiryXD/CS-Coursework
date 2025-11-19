/* Program Name: First Submission
 * Student Name: Ar-Raniry Ar-Rasyid
 * Student ID: 000-66-3921
 * NetID: jzr266
 * Description: A number of cents is inputted by the user, and the program then 
 * tells the user what coins they could have. This calculates the least amount
 * of coins on hand, because the program has no way of knowing whether the user
 * has a dime or two nickels  */
import java.util.Scanner;

/*Right above I declared the scanner. Below, and me introducing it and naming the class 
 * properly, and by properly I mean that the class is the same name as the 
 * file which would be coinChange.java
 */
class Coins {
    public static void main(String[] args) {
    Scanner s = new Scanner(System.in);

int userInput;
System.out.print("Enter amount of cents you have (0-99): ");
        userInput = s.nextInt();
        s.close();
/*Right above, I asked the user for their input, asking for a range between 0 and 99;
 * and then afterwards closing the scanner. Below that are the initalized values
 * of the coins that are going to be used in my algorithm.
 */
int qValue = 25;
int dValue = 10;
int nValue = 5;
int pValue = 1;

qValue = userInput / 25;
int userRemain = userInput % 25;
dValue = userRemain / 10;
int userRemain2 = userRemain % 10;
nValue = userRemain2 / 5;
int userRemain3 = userRemain2 % 5;
pValue = userRemain3 / 1;
/* How my algorithm works is that it takes the original number from the user
 * and divides that by 25, to symbolize quarters. I made another variable called
 * "userRemain" to take the remainder and then use the remainder to find the amount of 
 * dimes the user has. I think of this as how water is filtered with sand and charcoal. 
 * You get in the unclean water, in this case the orignal number, and then strain
 * it through several layers to get the clean water, in this case pennies. Then, below 
 * shows the results to the viewer, if it didn't print out there wouldn't be any
 * point to the previously made process.
 */
System.out.println ("Number of quarters: " + qValue);
System.out.println ("Number of dimes: " + dValue);
System.out.println ("Number of nickels: " + nValue);
System.out.println("Number of pennies: " + pValue);

    }
}