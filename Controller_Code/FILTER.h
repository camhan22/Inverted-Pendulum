class Filter
{
  private:
    int ts;
    int fs;

    int numerator[1] = {1};// Just need to make sure the number of elements is the same as the number after numerator declaration
    int denominator[3] = {1, 1, 1 };// Just need to make sure the number of elements is the same as the number after denominator declaration
    int previous_outputs[sizeof(denominator)/sizeof(int)];
    int previous_inputs[sizeof(numerator)/sizeof(int)];
    
    
  public:
  
};
