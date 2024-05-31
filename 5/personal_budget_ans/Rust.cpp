#include <ctime>
#include <optional>
#include <iostream>
#include <algorithm>
#include <list>
#include <memory>
#include <exception>
#include <string>
using namespace std;
class Date{
    public:
    int day_=1;
    int month_=1;
    int year_=1;
    Date() = default;
    Date(int day,int month,int year):
        day_(day),
        month_(month),
        year_(year)
    {
    }
    time_t AsTimestamp() const;
    friend int operator-(const Date& date_to,const Date& date_from){
        const time_t timestamp_to = date_to.AsTimestamp();
        const time_t timestamp_from = date_from.AsTimestamp();
        static const int SECONDS_IN_DAY = 60 * 60 * 24;
        return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
    }
};
time_t Date::AsTimestamp() const {
  std::tm t;
  t.tm_sec   = 0;
  t.tm_min   = 0;
  t.tm_hour  = 0;
  t.tm_mday  = day_;
  t.tm_mon   = month_ - 1;
  t.tm_year  = year_ - 1900;
  t.tm_isdst = 0;
  return mktime(&t);
}
int ComputeDaysDiff(const Date& date_to, const Date& date_from) {
  const time_t timestamp_to = date_to.AsTimestamp();
  const time_t timestamp_from = date_from.AsTimestamp();
  static const int SECONDS_IN_DAY = 60 * 60 * 24;
  return (timestamp_to - timestamp_from) / SECONDS_IN_DAY;
}
enum class Operation{
            Addition,
            Substraction,
            Multiplication
        };

class Node{
    private:
        double proceeds_;
        double spend_;
        int count_;
        size_t border_left_,border_right_;
        shared_ptr<Node> left_;
        shared_ptr<Node> right_;
    public:
        Node():
            proceeds_(0),
            spend_(0),
            count_(1),
            left_(nullptr),
            right_(nullptr)
        {
        }
        Node(int count,double proceeds,double spend ,int border_left){
            count_ = count;
            proceeds_ = proceeds;
            spend_ = spend;
            border_left_ = border_left;
            int border_right = border_left+count;
            border_right_ = border_left+count;
            int left_count = count_/2;
            int right_count = count_- left_count;
            size_t left_border_left =border_left_;
            size_t left_border_right = border_left_+ left_count;
            size_t right_border_left = border_left_+ left_count;
            size_t right_border_right = border_right_;
            double left_proceeds = proceeds_* (1.*left_count/count_);
            double left_spend = spend_* (1.*left_count/count_);
            double right_spend = spend_ - left_spend;
            double right_proceeds = proceeds_ - left_proceeds;
           if (left_count>=1&&count_!=1)
            left_ = make_shared<Node>(left_count , left_proceeds,left_spend,left_border_left);
           if (right_count>=1&&count_!=1)
            right_ = make_shared<Node>(right_count, right_proceeds,right_spend,right_border_left);
        }
        void  Print(ostream& os){
            string left = to_string(border_left_);
            string right = to_string(border_right_); 
            string proc = to_string(proceeds_);
            os<<"["+ left+" "+right+"] "+ " proceeds = "+proc;
            os<<endl;
            if (left_!=nullptr){
                left_->Print(os);
            }
            os<<"  ";
            if (right_!=nullptr){
                right_->Print(os);
            }
        }
        
        double getProceeds(size_t border_left,size_t border_right){
            if(border_left==border_left_&&border_right==border_right_){
                return proceeds_-spend_;
            }
            if (border_left<left_->border_right_ && border_right<=left_->border_right_){
                return left_->getProceeds(border_left,border_right);
            }
            else if (border_left<left_->border_right_ && border_right>left_->border_right_){
                return 1.*left_->getProceeds(border_left,left_->border_right_) + 1.*right_->getProceeds(right_->border_left_,border_right);
            }
            else if (border_left>=right_->border_left_){
                return right_->getProceeds(border_left,border_right);
            }
            else {
                throw runtime_error("invalid intervals");
            }
        }
                /*void UpdateProceeds(double proceeds){
            proceeds_ = proceeds;
            if (left_!= nullptr){
                    left_->UpdateProceeds(proceeds*(1.*left_->count_/(1.*count_)));
                }
            if (right_!=nullptr){
                    right_->UpdateProceeds(proceeds*(1.*right_->count_/(1.*count_)));
            }
        }*/
        void Update(size_t border_left,size_t border_right,double proceeds,double percent,Operation op){
            
            if(border_right_==border_right&&border_left_==border_left){
                 //UpdateProceeds(proceeds);
                 if (op==Operation::Addition)
                    {
                       proceeds_ += proceeds;}
                 else if (op==Operation::Substraction){
                     spend_+=proceeds;
                 }
                 else{
                 
                     proceeds_*=(1-percent);
                 }
                 return;
            }
            if (border_left<left_->border_right_ && border_right<=left_->border_right_){
                left_->Update(border_left,border_right,proceeds,percent,op);
            }
            else if (border_left<left_->border_right_ && border_right>left_->border_right_){
                left_->Update(border_left,left_->border_right_,proceeds*(1.*left_->border_right_-1.*border_left)/(1.* border_right-1.* border_left),percent,op);
                right_->Update(left_->border_right_,border_right,proceeds*(1.*border_right-1.*right_->border_left_)/(1.* border_right-1.* border_left),percent,op);
            }
            else if (border_left>=left_->border_right_){
                right_->Update(border_left,border_right,proceeds,percent,op);
            }
            else {
                throw runtime_error("invalid intervals");
            }
        }
};
istream& operator>>(istream& is,Date& date){
    char d1,d2;
    if (is>>date.year_&&is>>d1&&is>>date.month_&&is>>d2&&is>>date.day_){
        if (d1=='-'&&d2=='-')
            return is;
        else throw runtime_error("invalid_date");
            }
    else 
        throw runtime_error("invalid_date");
}
enum class Order{
    Earn,
    PayTax,
    Spend,
    ComputeIncome
};
struct FullOrder{
    Order order_;
    Date from_;
    Date to_;
    int value = 0;
};
FullOrder ReadAnOrder(istream& is){
    string str;
    Date from,to;
    int value;
    is>>str;
    if (str=="ComputeIncome"){
        is>>from>>to;
        return {Order::ComputeIncome,from,to,0};
    }
    else if (str=="Earn"){
        is>>from>>to>>value;
        return {Order::Earn,from,to,value};    
    }
    else if (str=="Spend"){
        is>>from>>to>>value;
        return {Order::Spend,from,to,value}; 
    }
    else if (str=="PayTax"){
        is>>from>>to>>value;
        return {Order::PayTax,from,to,value};
    }
    else return {Order::Earn,{1,1,2000},{1,1,2000},0};
}
class OrderBuilder{
    private:
    FullOrder fo_;
    Node& node_;
    Date begin_date;
    Date end_date;
    public:
    OrderBuilder(Node& node,Date start,Date stop):
    node_(node),
    begin_date(start),
    end_date(stop)
    {
            fo_ = {Order::Earn,{1,1,2000},{1,1,2000},0};
        }
    OrderBuilder(FullOrder fo,Node& node):fo_(fo),node_(node)
    {
    }
    void GetNewOrder(FullOrder fo){
        fo_ = fo;
    }
    optional<double> Build(){
        int start = fo_.from_-begin_date;
        int stop = (fo_.to_-begin_date)+1;
        if (fo_.order_==Order::ComputeIncome){
            optional<double> d;
            d =node_.getProceeds(start,stop);
            return d;
        }
        else if (fo_.order_==Order::Earn){
            //double new_proceeds = node_.getProceeds(start,stop);
            node_.Update(start,stop,1.*fo_.value,1.,Operation::Addition);
            return nullopt;
        }
        else if (fo_.order_ ==Order::Spend){
            node_.Update(start,stop,1.*fo_.value,1.,Operation::Substraction);
            return nullopt;
        }
        else if (fo_.order_==Order::PayTax){
            //double new_proceeds = node_.getProceeds(start,stop);
            node_.Update(start,stop,-1,1.*fo_.value/100.,Operation::Multiplication);
            return nullopt;
        }
        else return nullopt;
    }
};
int main(){
    int Q;
    cin>>Q;
    
    Date begin_date {1,1,2000};
    Date end_date {1,1,2100};
    //cout<<end_date-begin_date<<endl;
    Node n(end_date-begin_date,0,0,0);
    OrderBuilder OB(n,begin_date,end_date);
    for (int i=0;i<Q;i++){
        OB.GetNewOrder(ReadAnOrder(cin));
        optional<double> d = OB.Build();
        if (d){
            cout.precision(25);
            cout<<d.value()<<'\n';
        }
    }

    return 0;
}