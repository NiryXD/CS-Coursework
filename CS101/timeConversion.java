import java.util.Scanner;
class timeConversion {
    public static void main (String [] args){
Scanner s = new Scanner (System.in);
int mHours, mMins; //military hours and minutes
System.out.println ("Give the hours of military time:");
mHours = s.nextInt();
System.out.println ("Give the minutes of military time:");
mMins = s.nextInt();

int sHours, sMins; //standard hours and minutes
sHours = mHours % 12;
sMins = mMins;
s.close ();

System.out.format ("The military time is %02d:%02d\n", mHours, mMins);
System.out.format("The standard time is %02d:%02d\n",sHours, sMins);
    }
}