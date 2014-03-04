#include <macd_sell_signal/macd_indicator.h>

macd_indicator::macd_indicator(){
  this->short_ = 0;
}

macd_indicator::macd_indicator(int short_len, int long_len, int sig_len, int period_len, int spread_window_len){
  this->short_ = short_len;
  this->long_ = long_len;
  this->sig_ = sig_len;
  this->period_ = period_len;
  
  //alpha for EMA calculated with N points, the last N datum points in an EMA represent about 86% of the total weight in the calculation (this can be changed, but i don't really see any reason to)
  this->short_alpha_ = 2.0/((this->short_ + 1) * this->period_);  
  this->long_alpha_ = 2.0/((this->long_ + 1) * this->period_);
  this->sig_alpha_ = 2.0/((this->sig_ + 1) * this->period_);
  
  this->short_emaf_line_ = 0;
  this->long_emaf_line_ = 0;
  this->macd_line_ = 0;
  this->signal_line_ = 0;
  this->macd_hist_ = 0;

}
/*short_emaf = exponential_moving_average(data,(2/((short+1)*delta_t)));
    long_emaf = exponential_moving_average(data,(2/((long+1)*delta_t)));
    macd_line = short_emaf - long_emaf;
    signal_line = exponential_moving_average(macd_line,(2/((signal+1)*delta_t)));
    macd = macd_line - signal_line;*/

int macd_indicator::update(double new_data, double &short_emaf, double &long_emaf, double &macd, double &signal, double &macd_hist){

  short_emaf = (this->short_alpha*new_data) + ((1-this->short_alpha_)*this->short_emaf_line_);
  long_emaf = (this->long_alpha*new_data) + ((1-this->long_alpha_)*this->long_emaf_line_);
  macd = short_emaf-long_emaf;
  signal = (this->sig_alpha_ * macd) + ((1-this->sig_alpha_) * this->signal_line_);
  macd_hist = macd - signal;

  //update objects local values with new information
  this->short_emaf_line_ = short_emaf;
  this->long_emaf_line_ = long_emaf;
  this->macd_line_ = macd;
  this->signal_line_ = signal;
  this->macd_hist_ = macd_hist;

  return 0;
}
