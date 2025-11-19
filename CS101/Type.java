import java.util.Scanner;

public class Type {
    public static void main(String[] args) {
        Scanner s = new Scanner(System.in);
        String yn;

        do {
            System.out.println("Are you a guy? (y/n)");
            String answer = s.nextLine();

            if (answer.equals("y")) {
                System.out.println("Do you like dudes? (y/n)");
                String likeDudes = s.nextLine();

                if (likeDudes.equals("y")) {
                    System.out.println("lol kys fag");
                } else if (likeDudes.equals("n")) {
                    System.out.println("das swag");
                } else {
                    System.out.println("y or n only.");
                }

            } else if (answer.equals("n")) {
                System.out.println("Ew, woman");

            } else {
                System.out.println("y or n only.");
            }

            System.out.println("Play again? y/n");
            yn = s.nextLine();
            if (yn.equals("n")) {
                System.out.println("kys");
            }

        } while (yn.equals("y"));

        s.close();
    }
}
