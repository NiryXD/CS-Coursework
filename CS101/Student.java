class Student {
     public String firstName;
     public String lastName;
     public double GPA;

     public Student(String first, String last) {
          firstName = first;
          lastName = last;
     }

     public void setGPA(double newGPA){
          GPA = newGPA;

     }

     public String getInitials(){
          String initials;
          initials = firstName.substring(0,1) + "." + lastName.substring(0,1) + ".";
          return initials;
     }
}