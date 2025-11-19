import java.util.ArrayList;
class LearnAL {
public static void main (String [] args) {
    ArrayList<Integer> scores = new ArrayList<Integer>();
    scores.add(98);
    scores.add(88);
    scores.add(1, 91); // this inserts 91 in between 98 and 88

    System.out.println("The size of the AL is " + scores.size()); 
    for (int i= 0; i < scores.size(); i++) {
        System.out.println("Element " + i + " is " + scores.get(i));
        }

        scores.set(1, 51); //changes the element at index 1 to 51
    for (int i= 0; i < scores.size(); i++) {
        System.out.println("Element " + i + " is " + scores.get(i));
        }

        scores.remove(0); //removes the element at index 0
     }
}