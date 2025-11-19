import java.util.Scanner;
class SmellyCat {
    public static void main (String [] args){
        // code here for now
        int favNum = 5;  //declaring and initalizing an integer
        double favDec = 6.9; //favDec is in CamelCase; my-scanner is what they use in C++ called SnakeCase
        Scanner MeinScanner = new Scanner (System.in); //This process creates a Scanner
        String Animal, ingVerb, Noun1, Noun2, Noun3, Noun4;
        System.out.println("Hello this is the SmellyCat program");
       
        /* We will use the Scanner to capture the answers to the prompts */
        System.out.println("Type an animal name, you filthy animal");
        Animal = MeinScanner.nextLine();
        System.out.println ("You typed " + Animal + " :D");
        System.out.println ("Type a verd ending in -ing, or else");
        ingVerb = MeinScanner.nextLine();

        //Four Nouns Next
        System.out.println("Give me an occupation");
        Noun1 = MeinScanner.nextLine();
        System.out.println ("Give me a noun");
        Noun2 = MeinScanner.nextLine();
         System.out.println("Give me two rhyming words");
        Noun3 = MeinScanner.nextLine();
        System.out.println ("Second ryhming word");
        Noun4 = MeinScanner.nextLine();

        System.out.println();
        System.out.println("Smelly " + Animal + ", smelly " + Animal  );
        System.out.println ("What are they " + ingVerb + " you");
        System.out.println("Smelly " + Animal + ", smelly " + Animal  );    
        System.out.println ("It's not your fault.");
        System.out.println();  
        System.out.println ("They won't take you to the " + Noun1 );   
        System.out.println ("You're obviously not their favorite " + Noun2);
        System.out.println("Smelly " + Animal + ", smelly " + Animal  );    
        System.out.println ("It's not your fault.");
        System.out.println(); 
        System.out.println("You may not be a bed of " + Noun3);
        System.out.println ("You're not friend to those with " + Noun4);
        System.out.println("Smelly " + Animal + ", smelly " + Animal  );    
        System.out.println ("It's not your fault.");

       }
}