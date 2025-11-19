public class SuperheroTester {
    public static void main(String[] args) {
        Power speed = new Power("Super Speed", 10);

        Superhero flash = new Superhero("The Flash", speed);
        Superhero shazam = new Superhero("Shazam", speed);

        System.out.println("Shazam's strength: " + shazam.getStrength());
        System.out.println("The Flash's strength: " + flash.getStrength());

        //Update the Flash's strength to 15
        System.out.println("Updating The Flash's strength to 15.");
        flash.setStrength(15);

        System.out.println("Shazam's strength: " + shazam.getStrength());
        System.out.println("The Flash's strength: " + flash.getStrength());
        
    }
}