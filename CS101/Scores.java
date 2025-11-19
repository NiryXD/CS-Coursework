class Score {
     public static void main (String[] args){
          char grade = 'B';
          int score = 0;

          switch (grade) {
               case 'A':
                    System.out.println ("Setting new score");
                    score = 100;
                    break;
               case 'B':
                    score = 85;
                    break;
               case 'C':
               case 'c':
                    score = 70;
                    break;
               case 'F':
                    score = 50;
                    break;
               default:
                    score = -1;
                    System.out.println ("Invalid grade");
          }

          System.out.println ("I have pooped " + score + " times");
     }
}