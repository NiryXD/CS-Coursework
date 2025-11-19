import java.util.Scanner;
class funeeName {
     public static void main (String[] args) {
          String Title, Middle, Bev, Street, newName, yn;
          Scanner s = new Scanner (System.in);

          System.out.println("Funny name generator");
          do {
               System.out.println("What is your title?");
               Title = s.nextLine();
               System.out.println("What is your Middle name?");
               Middle = s.nextLine();
               System.out.println("What is the last bev you had? We out here yurrrrrrrr");
               Bev = s.nextLine();
               System.out.println("What is the name of the street you live on?");
               Street = s.nextLine();

newName = Title + " " + Middle + " " + (Bev + "ton") + " of " + (Street + "shire");
System.out.println("Your Funny name is " + newName);

System.out.println("Play again. y/n");
yn = s.nextLine();
          } while (yn.equals("y"));
     }
}