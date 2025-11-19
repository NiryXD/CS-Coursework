class Staty {
    private int mNumValues;
    private int mSumOfValues;
    private int mProductOfValues;
    private int mMaximumValue;
    private int mMinimumValue;

    public Staty() {
        mNumValues = 0;
        mSumOfValues = 0;
        mProductOfValues = 1;
        mMaximumValue = Integer.MIN_VALUE;
        mMinimumValue = Integer.MAX_VALUE;
    }
    
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
    }

    public double getAverage() {
        if (mNumValues == 0) {
            return 0.0;
        }
        return (double) mSumOfValues / mNumValues;
    }
}
