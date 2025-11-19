import java.util.Scanner;

class MiniGolf {
    public static void main (String[] args) {
        Scanner s = new Scanner(System.in);
        final int numOplayers = 2;
        final int numOholes = 3;
        int score1 = 0;

        System.out.println("Welcome to mini golf!");

        for (int p = 1; p <= numOplayers; p++) {
            System.out.print("What is your name?");
            String name1 = s.next();
            score1 = 0;
            System.out.println(name1);
            for (int i = 1; i <= numOholes; i++) {
                System.out.print("  What is your score for hole " + i + "? ");
                score1 += s.nextInt();
            }
            System.out.println ( name1 +"'s score is " + score1);
        }
        s.close();
    }

}
