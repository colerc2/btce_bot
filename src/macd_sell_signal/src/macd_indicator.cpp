#include <macd_sell_signal/macd_indicator.h>

macd_indicator::macd_indicator(){
  this->short_ = 0;
}

macd_indicator::macd_indicator(int short_len, int long_len, int sig_len, int period_len, int spread_window_len){
  this->short_ = short_len;
  this->long_ = long_len;
  this->sig_ = sig_len;
  this->period_ = period_len;
  this->spread_window_ = spread_window_len;
  
  //alpha for EMA calculated with N points, the last N datum points in an EMA represent about 86% of the total weight in the calculation (this can be changed, but i don't really see any reason to)
  this->short_alpha_ = 2.0/((this->short_ + 1) * this->period_);  
  this->long_alpha_ = 2.0/((this->long_ + 1) * this->period_);
  this->sig_alpha_ = 2.0/((this->sig_ + 1) * this->period_);

}
/*short_emaf = exponential_moving_average(data,(2/((short+1)*delta_t)));
    long_emaf = exponential_moving_average(data,(2/((long+1)*delta_t)));
    macd_line = short_emaf - long_emaf;
    signal_line = exponential_moving_average(macd_line,(2/((signal+1)*delta_t)));
    macd = macd_line - signal_line;*/

int macd_indicator::update(double new_data){
  
  return 0;
}

std::deque<double> macd_indicator::get_short_emaf_line(){
  return this->short_emaf_line_;
}
std::deque<double> macd_indicator::get_long_emaf_line(){
  return this->long_emaf_line_;
}
std::deque<double> macd_indicator::get_macd_line(){
  return this->macd_line_;
}
std::deque<double> macd_indicator::get_signal_line(){
  return this->signal_line_line_;
}
std::deque<double> macd_indicator::get_macd_hist_(){
  return this->short_emaf_line_;
}
