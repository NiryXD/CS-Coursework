public class Superhero {
    private String name;
    private Power superPower;
    private int level;

    public Superhero(String heroName, Power heroPower) {
        name = heroName;
        // superPower = heroPower; //can't do this!
        
        superPower = new Power(heroPower.getName(), heroPower.getStrength());
    }

    public String getName() {
        return name;
    }

    public void setName(String newName) {
        name = newName;
    }

    public int getStrength() {
        return superPower.getStrength();
    }

    public void setStrength(int newStrength) {
        superPower.setStrength(newStrength);
    }

    public int getLevel () { 
        return level;
    }

    public void setLevel (int newLevel) {
        if (newLevel < 1) System.out.print("Cannot set level less than one.");
        else level = newLevel; 
    }


}

//all attributes private, and methods public