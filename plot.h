#ifndef PLOT_H
#define PLOT_H

#include "incrementalplot.h"

class Plot: public IncrementalPlot
{
public:
    Plot(QWidget *parent);

public Q_SLOTS:
    void append(int, int); //append is an overloaded function
    void append(int, float);
    void setXAxisIntervalLength(double interval);
    void setYAxisIntervalLength(double interval);

private:
    QwtInterval d_xInterval;
    QwtInterval d_yInterval;


};

#endif // PLOT_H
