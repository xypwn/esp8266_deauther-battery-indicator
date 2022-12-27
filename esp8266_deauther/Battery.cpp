/* This software is licensed under the MIT License: https://github.com/spacehuhntech/esp8266_deauther */

#include "Battery.h"

#include "A_config.h"
#include "language.h"

struct DischargeCurve {
    // Battery discharge can be approximated by an asymmetrical sigmoidal curve: y = d + (a-d)/(1 + (x/c)^b)^m
    struct SigmoidParams {
        double a, b, c, d, m;

        double apply(double x) const {
            return d + (a - d) / pow(1 + pow(x / c, b), m);
        }
    };
    // At high or low percentage, the curve approximation is changed to a linear approximation (typically in the range of approx. 95-100% and 0-5%)
    // x1 MUST be < than x2 and y1 < y2
    struct LineParams {
        double x1, x2; // Voltage
        double y1, y2; // Battery percentage

        double apply(double x) const {
            double m = (y2 - y1) / (x2 - x1);
            return m * (x - x1) + y1;
        }
    };
    SigmoidParams sigmoid; // Main part of the curve
    LineParams low;  // Low linear threshold / low linear part
    LineParams high; // High linear threshold / high linear part

    double apply(double x) const {
        // Low linear part
        if (x < low.x2) {
            if (x < low.x1)
                return 0.0;
            return low.apply(x);
        }

        // High linear part
        if (x > high.x1) {
            if (x > high.x2)
                return 100.0;
            return high.apply(x);
        }

        // Sigmoidal curve
        return sigmoid.apply(x);
    }
};

static const DischargeCurve dischargeCurve = BATTERY_DISCHARGE_CURVE;

static double analogReadAverage(size_t samples) {
    double avgValue = 0.0;
    for (size_t i = 0; i < samples; i++)
        avgValue += (double)analogRead(A0);
    avgValue /= samples;
    return avgValue;
}

double battery::calibrate(double voltage, size_t samples) {
    return 5.0 / (double)analogReadAverage(samples);
}

double battery::getVoltage(double calibrationFactor, size_t samples) {
    return analogReadAverage(samples) * calibrationFactor + (double)(BATTERY_VOLTAGE_ADJUST);
}

double battery::getPercentage(double calibrationFactor, size_t samples) {
    return dischargeCurve.apply(getVoltage(calibrationFactor, samples));
}

String battery::getStatusJSON(double calibrationFactor, size_t samples) {
    double voltage = getVoltage(calibrationFactor, samples);
    double percentage = getPercentage(calibrationFactor, samples);

    String json = String(OPEN_CURLY_BRACKET);                                                                                              // {
    json += String(DOUBLEQUOTES) + str(B_PERCENTAGE) + String(DOUBLEQUOTES) + String(DOUBLEPOINT) + String(percentage, 0) + String(COMMA); // "percentage": 100,
    json += String(DOUBLEQUOTES) + str(B_VOLTAGE)    + String(DOUBLEQUOTES) + String(DOUBLEPOINT) + String(voltage, 2);                    // "voltage": 3.72
    json += CLOSE_CURLY_BRACKET;                                                                                                           // }
    return json;
}
