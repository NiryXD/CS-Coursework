class carExample {
     public static void main (String[] args) {
          /* Make an object of type Car
           * Making an instance of the Car class
           */
          Car carObj1 = new Car (19000, 2012, "Honda", "Accord");

          /*You can split this up into two lines */
          Car carObj2;
          carObj2 = new Car (20000, 1992, "Accura", "Integra");

          //carObj1.printInfo();

          Car carObj3 = new Car (25000, 2007); //making a car with 25k price and 2007 year

          System.out.println (carObj1.getBrand());

          Car carObj4 = new Car ("Ferrari");
          System.out.println (carObj4.getPrice());
          carObj4.setPrice (100000);
          System.out.println (carObj4.getPrice());

          carObj.changeAmount (-200);
     }
}