class While {
    public static void main (String [] args){
     int x = 1;
     int y = 3;

     while ( x < 10 ) {
          System.out.println("Inside the while loop");
          x = x + y; // or x+=y
          System.out.println ("x now equals " + x);
        }
        System.out.println ("Outisde the loop");
        do {
               System.out.println ("Inside the do-while loop");
               y--;
        } while (y > 0);
        }
    }
//Ctrl+C to stop the infinate loop