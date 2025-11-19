#include <iostream>
using namespace std;

    class Movie {
        private:
            string name;
            string mpaa;
            int count1; //terrible
            int count2; //bad
            int count3; //ok
            int count4; //good
            int count5; //great

        public:
            //constructor
            Movie (string n, string r) {
                name = n;
                mpaa = r;
                // TODO count 1-5 all need to start at zero
            }
            //getter and setter for name and mpaa
            string get_name() { return name; }
            string get_mpaa() { return mpaa; }
            void set_name(string newName) {name = newName; }
            void set_mpaa (string newMpaa) {mpaa = newMpaa; }
            // addRating
            void addRating(int rating ) {
                // TODO switch on the rating and if it doens't match 1-5 then default
                
                /* if (rating >= 1 && rating <= 5) {
                    // good rating
                } */
            }
            //getAverage
            double getAverage () {
                // TODO return the average of all the ratings
            }

    };

  int main () {
    cout << "abc" << endl;
  }