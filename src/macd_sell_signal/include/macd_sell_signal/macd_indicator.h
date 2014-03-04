#include <iostream>
#include <vector>
#include <deque>

class macd_indicator{
private:
  double short_emaf_line_;
  double long_emaf_line_;
  double macd_line_;
  double signal_line_;
  double macd_hist_;

  int short_, long_, sig_, period_;
  double short_alpha_, long_alpha_, signal_alpha_;

public:
  inline macd_indicator(){}
  macd_indicator(int short_len, int long_len, int sig_len, int period_len);
  int update(double new_data, double &short_emaf, double &long_emaf, double &macd, double &signal, double &macd_hist);
  
};
