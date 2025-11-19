/* I am going to create rectangles and triangles in this main class
 * using the Rectangle and Triangle blueprints (those blueprints are
 * the class definitions).
 */
class ExampleClass {
    public static void main(String[] args) {
        System.out.println("cccccc");
        Rectangle yard = new Rectangle(10, 20);     //created an object of type Rectangle called "yard"
        Rectangle brownie = new Rectangle(.01, .02); //created another object called "brownie"
        Rectangle table = new Rectangle (3.3, 2.0);

        System.out.println ("My table is " + table.getHeight() + " feet high.");
        System.out.println ("The area of my table is " + table.getArea());

         
        System.out.println("My yard is " + yard.getWidth() + " feet wide.");
        System.out.println("My brownie is " + brownie.getHeight() + " feet tall.");
        System.out.println("The area of my yard is " + yard.getArea() + ".");

        table.printInfo();
        yard.printInfo();
        brownie.printInfo();

        Triangle pyramid = new Triangle(3, 7);
        pyramid.printInfo();
        double twoBases = pyramid.getBase() + pyramid.getBase();   

    }
}