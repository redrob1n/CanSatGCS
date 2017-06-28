#include "plot.h"
#include <qglobal.h>
#include <qtimer.h>
#include <qwt_plot_grid.h>
#include <qwt_plot_canvas.h>
#include <qwt_plot_layout.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_draw.h>
#include "scrollzoomer.h"

const unsigned int c_rangeMax = 1000;

class Zoomer: public ScrollZoomer
{
public:
    Zoomer( QWidget *canvas ):
        ScrollZoomer( canvas )
    {
#if 0
        setRubberBandPen( QPen( Qt::red, 2, Qt::DotLine ) );
#else
        setRubberBandPen( QPen( Qt::red ) );
#endif
    }

    virtual QwtText trackerTextF( const QPointF &pos ) const
    {
        QColor bg( Qt::white );

        QwtText text = QwtPlotZoomer::trackerTextF( pos );
        text.setBackgroundBrush( QBrush( bg ) );
        return text;
    }

    virtual void rescale()
    {
        QwtScaleWidget *scaleWidget = plot()->axisWidget( yAxis() );
        QwtScaleDraw *sd = scaleWidget->scaleDraw();

        double minExtent = 0.0;
        if ( zoomRectIndex() > 0 )
        {
            // When scrolling in vertical direction
            // the plot is jumping in horizontal direction
            // because of the different widths of the labels
            // So we better use a fixed extent.

            minExtent = sd->spacing() + sd->maxTickLength() + 1;
            minExtent += sd->labelSize(
                scaleWidget->font(), c_rangeMax ).width();
        }

        sd->setMinimumExtent( minExtent );

        ScrollZoomer::rescale();
    }
};


Plot::Plot(QWidget * parent ):
    IncrementalPlot( parent ),
    d_xInterval(0.0, 5000.0),
    d_yInterval(0.0, 500000.0)
{
    setFrameStyle(QFrame::NoFrame);

    QwtPlotGrid * grid = new QwtPlotGrid;
    grid->setMajorPen(Qt::gray, 0, Qt::DotLine);
    grid->attach(this);

    setCanvasBackground(Qt::white);
    //setAxisScale(QwtPlot::xBottom, d_xInterval.minValue(), d_xInterval.maxValue());
    //setAxisScale(QwtPlot::yLeft, d_yInterval.minValue(), d_yInterval.maxValue());
    //setAxisAutoScale(QwtPlot::xBottom);
    //setAxisAutoScale(QwtPlot::yLeft);
    replot();

    (void) new Zoomer( canvas() );
}

void Plot::append(int x, int y)
{    
    //setXAxisIntervalLength((double) x);
    //setYAxisIntervalLength((double) y);
    IncrementalPlot::appendPoint( QPointF(x, y));
}

void Plot::append(int x, float y)
{
    //setXAxisIntervalLength((double) x);
    //setYAxisIntervalLength((double) y);
    IncrementalPlot::appendPoint( QPointF(x, y));
}

void Plot::setXAxisIntervalLength(double interval)
//Sets the scale of the xBottom axis in real time
//This axis is time so no other scaling computations are necessary
{
    if ( interval > 0.0 && interval != d_xInterval.width() )
    {
        d_xInterval.setMaxValue( d_xInterval.minValue() + interval + 20.0 );
        setAxisScale( QwtPlot::xBottom,
            d_xInterval.minValue(), d_xInterval.maxValue() );

        replot();
    }
}

void Plot::setYAxisIntervalLength(double interval)
//Sets the scale of the yLeft axis in real time
{
    if(d_yInterval.maxValue() < 50.0)
    {
        d_yInterval.setMaxValue(interval + 5.0);
        setAxisScale( QwtPlot::yLeft,
            d_yInterval.minValue(), d_yInterval.maxValue() );
    }
    if ( (interval > 0.0 && interval != d_yInterval.width()) && (interval >= d_yInterval.maxValue()))
    {
        if(interval >= 20000.0)
        {
            d_yInterval.setMaxValue( d_yInterval.minValue() + interval + 20000.0 );
            setAxisScale( QwtPlot::yLeft,
                d_yInterval.minValue(), d_yInterval.maxValue() );

        }
        else if(interval >= 1000)
        {
            d_yInterval.setMaxValue( d_yInterval.minValue() + interval + 2000.0 );
            setAxisScale( QwtPlot::yLeft,
                d_yInterval.minValue(), d_yInterval.maxValue() );
        }
        else
        {
            d_yInterval.setMaxValue( d_yInterval.minValue() + interval + 20.0 );
            setAxisScale( QwtPlot::yLeft,
                d_yInterval.minValue(), d_yInterval.maxValue() );

        }

        replot();
    }
}
