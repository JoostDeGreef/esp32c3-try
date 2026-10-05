#include <cmath>
#include <numbers>
#include <array>
#include <tuple>
#include <ctime>

#include "clock.h"
#include "display.h"
#include "console.h"
#include "joost_timer.h"

constexpr double pi = std::numbers::pi;

class ClockImpl
{
public:
    static ClockImpl & getInstance();

    void Start();
    void Stop();
    void Render();
private:
    ClockImpl();
    ClockImpl(const ClockImpl & other) = delete;
    ClockImpl(ClockImpl && other) = delete;

    std::unique_ptr<Timer> render_timer;
};

ClockImpl & ClockImpl::getInstance()
{
    static ClockImpl instance;
    return instance;
}

ClockImpl::ClockImpl()
{
    Display::configure();
}

void ClockImpl::Start()
{
    int period_ms = 333;
    render_timer = std::unique_ptr<Timer>(new Timer([this]()
    {
        this->Render();
    }, period_ms));
    render_timer->start();
}

void Clock::start()
{
    ClockImpl::getInstance().Render();
    ClockImpl::getInstance().Start();
}

void ClockImpl::Stop()
{
    render_timer.reset();
}

void ClockImpl::Render()
{
    static uint16_t white = RGB(255,255,255);
    static uint16_t grey = RGB(150,150,150);
    static uint16_t babeBlue = RGB(137,207,240);
    static uint16_t red = RGB(255,0,0);
    static uint16_t yellow = RGB(255,255,0);

    static uint16_t r = 120;

    Painter p = Display::getPainter();
    p.clear();
    time_t now = time(nullptr);
    struct tm local_time = {};
    localtime_r(&now, &local_time);
    int seconds = local_time.tm_sec;
    int minutes = local_time.tm_min;
    int hours = local_time.tm_hour;

    // digital clock    
    auto textCenteredAt = [&](int x, int y, uint16_t color, const std::string & text)
    {
        int w = p.textWidth(text);
        p.text(x-w/2,y,color,text);
    };
    std::string time = Joost::Format("%02i:%02i", hours, minutes);
    std::string date = Joost::Format("%04i-%02i-%02i", local_time.tm_year + 1900, local_time.tm_mon + 1, local_time.tm_mday);
    textCenteredAt(r,50,grey,time);
    textCenteredAt(r,140,babeBlue,"Mira"+ std::string(1,(char)255));
    textCenteredAt(r,170,grey,date);
    // ticks
    int r_outer = r;
    int r_middle = r-6;
    int r_inner = r-10;
    const static std::array<std::array<uint16_t,4>, 8> ticks
    {{
        {{static_cast<uint16_t>(std::cos(2 * pi * 0 / 12)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 0 / 12)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 0 / 12)*r_outer), static_cast<uint16_t>(std::sin(2 * pi * 0 / 12)*r_outer)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 1 / 60)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 1 / 60)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 1 / 60)*r_middle), static_cast<uint16_t>(std::sin(2 * pi * 1 / 60)*r_middle)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 2 / 60)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 2 / 60)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 2 / 60)*r_middle), static_cast<uint16_t>(std::sin(2 * pi * 2 / 60)*r_middle)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 3 / 60)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 3 / 60)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 3 / 60)*r_middle), static_cast<uint16_t>(std::sin(2 * pi * 3 / 60)*r_middle)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 4 / 60)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 4 / 60)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 4 / 60)*r_middle), static_cast<uint16_t>(std::sin(2 * pi * 4 / 60)*r_middle)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 1 / 12)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 1 / 12)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 1 / 12)*r_outer), static_cast<uint16_t>(std::sin(2 * pi * 1 / 12)*r_outer)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 6 / 60)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 6 / 60)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 6 / 60)*r_middle), static_cast<uint16_t>(std::sin(2 * pi * 6 / 60)*r_middle)}},
        {{static_cast<uint16_t>(std::cos(2 * pi * 7 / 60)*r_inner), static_cast<uint16_t>(std::sin(2 * pi * 7 / 60)*r_inner), static_cast<uint16_t>(std::cos(2 * pi * 7 / 60)*r_middle), static_cast<uint16_t>(std::sin(2 * pi * 7 / 60)*r_middle)}},
    }};
    for(const auto & tick: ticks)
    {
        p.line(r+tick[0],r+tick[1],r+tick[2],r+tick[3],white);
        p.line(r-tick[0],r-tick[1],r-tick[2],r-tick[3],white);
        p.line(r-tick[0],r+tick[1],r-tick[2],r+tick[3],white);
        p.line(r+tick[0],r-tick[1],r+tick[2],r-tick[3],white);
        p.line(r+tick[1],r+tick[0],r+tick[3],r+tick[2],white);
        p.line(r-tick[1],r-tick[0],r-tick[3],r-tick[2],white);
        p.line(r-tick[1],r+tick[0],r-tick[3],r+tick[2],white);
        p.line(r+tick[1],r-tick[0],r+tick[3],r-tick[2],white);
    }
    // center dot
    p.circle(r,r,5,white);
    // draw the hands
    r_inner = 10;
    //hours
    hours = (hours + 9) % 12;
    r_outer = r*2/3;
    double t = 2 * pi * ((hours + (minutes / 60.0)) / 12.0);
    pos p0 = {static_cast<int16_t>(r+std::cos(t)*r_inner), static_cast<int16_t>(r+std::sin(t)*r_inner)};
    pos p1 = {static_cast<int16_t>(r+std::cos(t)*r_outer), static_cast<int16_t>(r+std::sin(t)*r_outer)};
    p.line(p0,p1,yellow,5);
    // minutes
    r_outer = r*5/6;
    minutes = (minutes + 45) % 60;
    t = 2 * pi * (minutes / 60.0);
    p0 = {static_cast<int16_t>(r+std::cos(t)*r_inner), static_cast<int16_t>(r+std::sin(t)*r_inner)};
    p1 = {static_cast<int16_t>(r+std::cos(t)*r_outer), static_cast<int16_t>(r+std::sin(t)*r_outer)};
    p.line(p0,p1,yellow,4);
    // seconds
    r_outer = r*8/9;
    seconds = (seconds + 45) % 60;
    t = 2 * pi * (seconds / 60.0);
    p0 = {static_cast<int16_t>(r+std::cos(t)*r_inner), static_cast<int16_t>(r+std::sin(t)*r_inner)};
    p1 = {static_cast<int16_t>(r+std::cos(t)*r_outer), static_cast<int16_t>(r+std::sin(t)*r_outer)};
    p.line(p0,p1,red,3);

    // something is very odd with the colors. 
    // do some more testing
    // p.fill(black);
    // for(uint16_t y=0;y<15;++y)
    // {
    //     for(uint16_t x=0;x<15;++x)
    //     {
    //         uint16_t rgb = ~((3<<x) | (3<<y));
    //         p.filled_rectangle(50+x*10,50+y*10,57+x*10,57+y*10,rgb);
    //     }
    // }
// p.fill(black);

// p.filled_rectangle(20, 20, 60, 60, RGB(255, 0, 0));
// p.filled_rectangle(70, 20, 110, 60, RGB(0, 255, 0));
// p.filled_rectangle(120, 20, 160, 60, RGB(0, 0, 255));
// p.filled_rectangle(170, 20, 210, 60, RGB(255, 255, 0));
// p.filled_rectangle(45, 80, 85, 120, RGB(255, 0, 255));
// p.filled_rectangle(95, 80, 135, 120, RGB(0, 255, 255));
// p.filled_rectangle(145, 80, 185, 120, RGB(255, 255, 255));

// printf("\nR     %04X\n", RGB(255,0,0));
// printf("G     %04X\n", RGB(0,255,0));
// printf("B     %04X\n", RGB(0,0,255));
// printf("Y     %04X\n", RGB(255,255,0));
// printf("M     %04X\n", RGB(255,0,255));
// printf("C     %04X\n", RGB(0,255,255));
// printf("W     %04X\n", RGB(255,255,255));
// printf("K     %04X\n\n", RGB(0,0,0));

    Display::flip();
}

