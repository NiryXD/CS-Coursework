public class Arrays {
    public static void main(String[] args) {
        String[] studentNames = new String[3];
        int[] scores = new int[3];
        char[] finalGrades = new char[3];

        studentNames[0] = "Sarah";
        scores[0] = 97;
        finalGrades[0] = 'A';

        studentNames[1] = "Waldo";
        scores[1] = 83;
        finalGrades[1] = 'B';

        studentNames[2] = "Xavier";
        scores[2] = 91;
        finalGrades[2] = 'A';

        for (int i = 0; i < studentNames.length; i++) {
            System.out.println(studentNames[i]);
            System.out.println(scores[i]);
            System.out.println(finalGrades[i]);
            System.out.println("\n");
        }

        double sum = 0;
        for (int i = 0; i < scores.length; i++) {
            sum += scores[i];
        }

        double average = sum / scores.length;
        System.out.println("Average: " + average);
    }
}
