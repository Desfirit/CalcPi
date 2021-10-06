#include <iostream>
#include <random>
#include <algorithm>
#include <time.h>
#include <windows.h>
#include <iomanip>
#include <iterator>
#include <math.h>
#include <limits>
using namespace std;


static void showProgress(long long cur, long long end)
{
    const string desc = "Progress: ";

    static HANDLE output = GetStdHandle(STD_OUTPUT_HANDLE);

    if(!cur)
        cout << desc;

    SetConsoleCursorPosition(output, {(short)desc.size(),0});
    cout << (double)(cur+1)*100/end;
}

class Model
{

    mt19937 gen;
    pair<double,double> circle_point;
    double radius;
    uniform_real_distribution<> dis{0,1};


    template<typename Fs>
    auto scaled_point(Fs scaler1, Fs scaler2)
    {
        return [=](double a, double b){
            return pair{scaler1(a), scaler2(b)};
        };
    }

    auto scaler(double min, double max)
    {
        return [=](double val){
            return (max - min) * val + min;
        };
    }

    auto isInCircle(pair<double,double> point){
        
        auto [xp,yp] = point;
        auto [x0,y0] = circle_point;

        return pow((xp - x0),2) + pow((yp-y0),2) - pow(radius,2) < numeric_limits<double>::epsilon();
    }

public:
    Model(mt19937 generator, pair<double,double> circle_point, double radius): gen{generator}, circle_point{circle_point}, radius{radius}{}

    double calc_Pi(long long selection){
        long long inCircle = 0;


        auto scalerX(scaler(circle_point.first, circle_point.first + radius));
        auto scalerY(scaler(circle_point.second, circle_point.second + radius));

        auto point_maker(scaled_point(scalerX,scalerY));

        cout << fixed << setprecision(1);
        for(long long i = 0; i < selection; ++i)
        {
            if(isInCircle(point_maker(dis(gen),dis(gen))))
                ++inCircle;

            if(i % (selection/1000) == 0)
                showProgress(i,selection);
        }
        cout << setprecision(10) << '\n';
        return (long double)inCircle / selection * 4;
    }
};


template<typename Model>
class Tester
{
    Model model;
public:
    Tester(Model model): model{model} {}

    double make_test(const long long selection)
    {
        return model(selection);
    }

    vector<double> make_tests(const vector<long long>& inputs)
    {
        vector<double> res;
        res.reserve(inputs.size());

        transform(begin(inputs), end(inputs), back_inserter(res), [&](auto val){
            return model(val);
        });
        return res;
    }
};

void print_serias(vector<vector<double>>& serias)
{
    cout << "---------------------------" << '\n';
    for(int i = 0; i < serias.size(); ++i)
    {
        cout << "Seria" << i << ":\n";
        copy(begin(serias[i]), end(serias[i]), ostream_iterator<double>(cout, "\n"));
        cout << "---------------------------" << '\n';
    }
}

void print_eps(vector<double>& eps)
{
    copy(begin(eps), end(eps), ostream_iterator<double>(cout, "\n"));
    cout << "---------------------------" << '\n';
}

double calc_eps(const vector<double>& seria, const double abs_val)
{
    return accumulate(begin(seria),end(seria),(double)0, [abs_val](double sum, double val){
        return sum + abs((val - abs_val) / abs_val);
    });
}

auto calc_serias_eps(const vector<vector<double>>& serias, vector<double>& output, const double abs_val)
{
    return transform(begin(serias),end(serias), begin(output), [abs_val](auto& seria){
        return calc_eps(seria, abs_val);
    });
}

//(xp - x0)^2 + (yp - y0)^2 - ro^2 < eps
//(xp - x0)^2 + (yp - y0)^2  < ro^2
int main()
{
    //При 100.000.000 Pi ~ 3.141
    //При 1.000.000.000 Pi ~ 3.1415
    //При 10.000.000.000 Pi ~ 3.1415766704

    const double x0 = 0;
    const double y0 = 2;
    const double r0 = 5;

    const int serias_num = 5;

    const vector<long long> selections{10'000,100'000, 1'000'000, 10'000'000, 100'000'000};

    mt19937 gen(time(0));

    Model model(gen, {x0,y0}, r0);
    Tester tester([&model](auto val){
        return model.calc_Pi(val);
    });

    vector<vector<double>> serias(serias_num);
    transform(serias.begin(),serias.end(),serias.begin(), [&tester,selections](auto& c){
        return vector<double>(tester.make_tests(selections));
    });

    print_serias(serias);

    vector<double> eps(serias_num);
    calc_serias_eps(serias, eps, M_PI);

    cout << "Eps: " << '\n';
    print_eps(eps);

    vector<double> avg_eps(serias_num,0);
    for(int i = 0; i < serias_num; ++i)
        for(int j = 0; j < serias[i].size(); ++j)
            avg_eps[j] += serias[i][j];


    transform(begin(avg_eps),end(avg_eps), begin(avg_eps), [](double val){
        return abs(((val/serias_num) - M_PI) / M_PI);
    });

    cout << "Average eps: " << '\n';
    print_eps(avg_eps);

    return 0;
}
