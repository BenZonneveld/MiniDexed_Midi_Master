// Arduino Moving Average Library
// https://github.com/JChristensen/movingAvg
// Copyright (C) 2018 by Jack Christensen and licensed under
// GNU GPL v3.0, https://www.gnu.org/licenses/gpl.html

#ifndef MOVINGAVG_H_INCLUDED
#define MOVINGAVG_H_INCLUDED

#ifdef __cplusplus
class movingAvg
{
public:
    movingAvg(uint16_t interval)
        : m_interval(interval), m_nbrReadings(0), m_sum(0), m_next(0) {}
    void begin();
    uint16_t reading(uint16_t newReading);
    uint16_t getAvg();
    uint16_t getCount() { return m_nbrReadings; }
    void reset();
    uint16_t* getReadings() { return m_readings; }

private:
    uint16_t m_interval;     // number of data points for the moving average
    uint16_t m_nbrReadings;  // number of readings
    long m_sum;         // sum of the m_readings array
    uint16_t m_next;         // index to the next reading
    uint16_t* m_readings;    // pointer to the dynamically allocated interval array
};
#endif
#endif