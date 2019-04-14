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

// Phase control class (used for pump)
#include "phasecontrol.h"

// estimate of lag caused by ZCD circuit in usec
// measured as per http://espresso-for-geeks.kalaf.net/mod-list/#hw-zero-cross
#define ZCD_DELAY 36

// power frequency half-period in usec
// set to 8333 for 60Hz
// set to 10000 for 50Hz
#define HALF_PERIOD_USEC 10000

// the time in usec to wait after zero-cross before switching off
// this eliminates the back-EMF by waiting for current to drop before switching off
#define DELAY_AFTER_STOP (HALF_PERIOD_USEC / 2)

// TODO PRECACULATE 50Hz numbers
// These are precalculated values (usec) of the required phase delays for 0 to 100% power.
// Equation used is: 1000000 usec / 120 * (acos(2*x/100 - 1) / pi)
const uint16_t PhaseControl::_timeouts_usec[101] =
    {0, 531, 753, 924, 1068, 1196, 1313, 1421, 1521, 1616, 
    1707, 1793, 1877, 1957, 2035, 2110, 2183, 2255, 2324,
    2393, 2460, 2525, 2590, 2654, 2716, 2778, 2839, 2899,
    2958, 3017, 3075, 3133, 3190, 3246, 3303, 3358, 3414,
    3469, 3524, 3578, 3633, 3687, 3740, 3794, 3848, 3901,
    3954, 4007, 4061, 4114, 4167, 4220, 4273, 4326, 4379, 
    4432, 4486, 4539, 4593, 4647, 4701, 4755, 4810, 4864,
    4919, 4975, 5031, 5087, 5144, 5201, 5258, 5316, 5375,
    5435, 5495, 5556, 5617, 5680, 5743, 5808, 5874, 5941,
    6009, 6079, 6150, 6223, 6299, 6376, 6457, 6540, 6626,
    6717, 6812, 6913, 7020, 7137, 7265, 7410, 7581, 7802,
    8333};
    
PhaseControl::PhaseControl(PinName pin, PinName pin2) : _zcd(pin), _control_signal(pin2) {
	// if your zcd circuit has it's own pullup, set to None
        _zcd.mode(PullUp);
        _zcd.fall(callback(this, &PhaseControl::delayed_start));
        _zcd.rise(callback(this, &PhaseControl::delayed_stop));
        _level = 75;
        _control_signal = 0;

	// This is a workaround until timeouts for 50Hz are calculated
	if (HALF_PERIOD_USEC == 8333)
		_timeout_usec = HALF_PERIOD_USEC - _timeouts_usec[_level] - ZCD_DELAY; 
	else
		_timeout_usec = HALF_PERIOD_USEC - (_timeouts_usec[_level]*10000/8333) - ZCD_DELAY; 
        
        /* debug info
         _t.start();
        counter = 0;
        */
}

PhaseControl::~PhaseControl()
{
}

void PhaseControl::stop()
{
    /* debug info
    if (counter < 2000)
    {
        times[counter][0] = _t.read_us();
        times[counter][1] = 0;
        //times[counter][2] = _level; 
        counter++;
    }
    */

    _control_signal = 0;
    _control_timeout.detach();
}

void PhaseControl::start()
{
    /* debug info
    if (counter < 2000)
    {
        times[counter][0] = _t.read_us();
        times[counter][1] = 1;
        //times[counter][2] = _level;
        counter++;
    }
    */

    _control_signal = 1;
}

void PhaseControl::delayed_start()
{
    // Level 100 is full power (minus zcd lag)
    if (_level == 100)
        start();
    // Level 0 just stop
    else if (_level == 0)
        stop();
    // otherwise do a delayed start (start after timeout_usec elapses)
    else
        _control_timeout.attach_us(callback(this, &PhaseControl::start), _timeout_usec);
}

// We wait DELAY_AFTER_STOP before stopping to avoid back-EMF
void PhaseControl::delayed_stop()
{
    _test_timeout.attach_us(callback(this, &PhaseControl::stop), DELAY_AFTER_STOP);
}

uint8_t PhaseControl::get_level()
{
    return _level;
}

void PhaseControl::set_level(uint8_t level)
{
    if (level > 100)
        _level = 100;
    else
        _level = level;
    
    // This is a workaround until timeouts for 50Hz are calculated
    if (HALF_PERIOD_USEC == 8333)
	    _timeout_usec = HALF_PERIOD_USEC - _timeouts_usec[_level] - ZCD_DELAY;
    else
	    _timeout_usec = HALF_PERIOD_USEC - (_timeouts_usec[_level]*10000/8333) - ZCD_DELAY;
}

void PhaseControl::level_up(uint8_t value)
{
    if (_level < 100)
         set_level(_level + value);
}

void PhaseControl::level_down(uint8_t value)
{
    if (_level > 0)
        set_level(_level - value);
}
