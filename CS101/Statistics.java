class Statistics {
    private int mNumValues;
    private int mSumOfValues;
    private int mProductOfValues;
    private int mMaximumValue;
    private int mMinimumValue;

    public Statistics() {
        mNumValues = 0;
        mSumOfValues = 0;
        mProductOfValues = 1;
        mMaximumValue = 0;
        mMinimumValue = 0;
    }
/* Above I have initalized the private ints provided before hand. I created my own names before realizing that names were provided, so 
 * I'll keep that in mind for the future.
  */
    public int getSum() {
        return mSumOfValues;
    }

    public int getNumValues() {
        return mNumValues;
    }

    public int getProduct() {
        return mProductOfValues;
    }

    public int getMin() {
        return mMinimumValue;
    }

    public int getMax() {
        return mMaximumValue;
    }
    public void addValue(int val) {
        mNumValues++;
        mSumOfValues += val;
        mProductOfValues *= val;
        if (val > mMaximumValue) {
            mMaximumValue = val;
        }
        if (val < mMinimumValue) {
            mMinimumValue = val;
        }
        /* Above I kept track by how many values the user inputted by adding to NumValues each time. This way I can output the final 
        amount of terms shown when the user types in 'quit'. Using '+=' I can add the inputted values to the variable 'SumOfValues', 
        the same goes for the product counterpart, but using '*=' instead. The if-statements are made to filter which values are the greatest
        and smallest, which is one of the outputs given to the user. Below is the code for getting the average, the comment left there
        told me to return '0' when there aren't any values. This is done by a if-else statement. My code wasn't fucntioning correctly
        but I learned about a 'return' function that aided my code. Also to make variables that were initalized as ints, I included
        the keyword double to counter act that.
        */
    }

    public double getAverage() {
        if (mNumValues == 0) {
            return 0.0;
        } else {
            return (double) mSumOfValues / mNumValues;
        }
    }
    }