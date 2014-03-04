#include <iostream>
#include <vector>
#include <deque>

class macd_indicator{
private:
  std::deque<double> short_emaf_line_;
  std::deque<double> long_emaf_line_;
  std::deque<double> macd_line_;
  std::deque<double> signal_line_;
  std::deque<double> macd_hist_;

  int short_, long_, sig_, period_, spread_window_, size_backlog_;
  double short_alpha_, long_alpha_, signal_alpha_;
public:
  macd_indicator(){}
  macd_indicator(int short_len, int long_len, int sig_len, int period_len, int spread_window_len);
  int update(double new_data);
  
  std::deque<double> get_short_emaf_line();
  std::deque<double> get_long_emaf_line();
  std::deque<double> get_macd_line();
  std::deque<double> get_signal_line();
  std::deque<double> get_macd_hist_();
};
