/* Copyright (c) 2017 Philippe Kalaf, MIT License
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software 
 * and associated documentation files (the "Software"), to deal in the Software without restriction, 
 * including without limitation the rights to use, copy, modify, merge, publish, distribute, 
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or 
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING 
 * BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, 
 * DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/* Phase Control */

#ifndef PhaseControl_h
#define PhaseControl_h


class PhaseControl 
{
public:
    PhaseControl(PinName pin, PinName pin2);
    ~PhaseControl();

    uint8_t get_level();
    void level_up(uint8_t value = 1);
    void level_down(uint8_t value = 1);
    void start();
    void stop();
    void set_level(uint8_t level);

    /* debug info
    int times[2000][2];
    int counter;
    */

private:
    void delayed_start();
    void delayed_stop();
    InterruptIn _zcd;
    uint8_t _level;
    DigitalOut _control_signal;
    Timeout _control_timeout;
    Timeout _test_timeout;
    // Timer _t;   used for debug
    // 0-100% pump power ratios for phase control
    uint16_t _timeout_usec;
    static const uint16_t _timeouts_usec[101];
};

#endif// PhaseControl_h
