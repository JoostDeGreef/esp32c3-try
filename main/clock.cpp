#include <cmath>
#include <numbers>
#include <array>
#include <tuple> 

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
    static uint16_t grey = RGB(200,200,200);
    static uint16_t red = RGB(255,0,0);
    static uint16_t r = 120;

    Painter p = Display::getPainter();
    p.clear();
    // get the time from somewhere real, for now, millis()
    int seconds = esp_timer_get_time() / (1000*1000);
    int minutes = seconds/60;
    int hours = minutes/60;
    minutes %= 60;
    seconds %= 60;
    // digital clock    
    std::string time = Joost::Format("%02i:%02i", hours, minutes);
    int w = p.textWidth(time);
    p.text(r-w/2,150,grey,time);
    // center dot
    p.circle(r,r,5,white);
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
    // draw the hands
    r_inner = 10;
    p.line(static_cast<uint16_t>(r+std::cos(2 * pi * seconds / 60)*r_inner), static_cast<uint16_t>(r+std::sin(2 * pi * seconds / 60)*r_inner), static_cast<uint16_t>(r+std::cos(2 * pi * seconds / 60)*r_outer), static_cast<uint16_t>(r+std::sin(2 * pi * seconds / 60)*r_outer),red);

    Display::flip();
}

