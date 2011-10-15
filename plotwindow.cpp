#include "plotwindow.h"
#include "datalog.h"
#include "googlemap.h"
#include "datastatisticsview.h"

#include <qwt_plot_picker.h>
#include <qwt_plot_zoomer.h>
#include <qwt_plot_panner.h>
#include <qwt_picker_machine.h>
#include <qwt_painter.h>
#include <qwt_scale_widget.h>
#include <qwt_scale_engine.h>
#include <qtgui/qcombobox>
#include <qtgui/qvboxlayout>
#include <qtgui/qhboxlayout>
#include <qtgui/qcheckbox>
#include <qtgui/qlabel>
#include <iostream>
#include <sstream>

// Define colour of plot curves
#define HR_COLOUR Qt::darkRed
#define ALT_COLOUR Qt::darkGreen
#define CADENCE_COLOUR Qt::darkBlue
#define SPEED_COLOUR Qt::yellow

// A custom plot zoomer, which defines a better rubber band, and zooms on x-axis only
class QwtPlotCustomZoomer : public QwtPlotZoomer
{
public:
	QwtPlotCustomZoomer(int x_axis, int y_axis, QwtPlotCanvas* canvas, bool do_replot=true):
		QwtPlotZoomer(x_axis,y_axis,canvas,do_replot)
	{}

	// Override user selected points to only take their x coord selection
	bool accept(QPolygon& p) const
	{
		if ( p.count() < 2 )
			return true;

		// Set the zoom rect to be top to bottm, irrespective of what the user selects in y axis
		p[0].setY(0);
		p[1].setY(200);
		return true;
	}

	// Draw a nice rubber band
	void drawRubberBand(QPainter* painter) const
	{
		if ( rubberBand() < UserRubberBand )
			QwtPlotPicker::drawRubberBand( painter );
		else
		{
			if ( !isActive() || rubberBandPen().style() == Qt::NoPen )
				return;

			QPolygon p = selection();

			if ( p.count() < 2 )
				return;

			const QPoint& pt1 = p[0];
			const QPoint& pt2 = p[1];

			const int end = 250;
			painter->drawLine(pt1.x(), 0, pt1.x(), end);
			painter->drawLine(pt2.x(), 0, pt2.x(), end);
			painter->fillRect(QRect(pt1.x(), 0, pt2.x() - pt1.x(), end), QBrush(QColor("black"), Qt::Dense7Pattern));
		}
	}

};

// A custom plot picker to highligh the value of curve closest to the pointer
class QwtPlotCustomPicker : public QwtPlotPicker
{
public:
	QwtPlotCustomPicker(int x_axis, int y_axis, DataLog* data_log, QwtPlotCanvas* canvas):
		QwtPlotPicker(x_axis,y_axis,QwtPlotPicker::UserRubberBand, QwtPicker::AlwaysOn, canvas),
		_data_log(data_log)
	{}

	// Set the data log for this picker
	void setDataLog(DataLog* data_log)
	{
		_data_log = data_log;
	}

	// Surpress default tracker drawing
	void drawTracker(QPainter* painter) const
	{
		return;
	}

	// Draw curve numerical values and highligh curve points
	void drawRubberBand(QPainter* painter) const
	{
		if ( rubberBand() < UserRubberBand )
			QwtPlotPicker::drawRubberBand( painter );
		else
		{
			if ( !isActive() || rubberBandPen().style() == Qt::NoPen )
				return;

			QPolygon p = selection();

			if ( p.count() != 1 )
				return;

			const QPoint& pt1 = p[0];
			const double time = plot()->invTransform(QwtPlot::xBottom,pt1.x());

			const double hr = _data_log->heartRate(_data_log->indexFromTime(time));
			const QPoint pt1_hr(pt1.x(),plot()->transform(QwtPlot::yLeft,hr));

			const double speed = _data_log->speed(_data_log->indexFromTime(time));
			const QPoint pt1_speed(pt1.x(),plot()->transform(QwtPlot::yLeft,speed));

			const double alt = _data_log->alt(_data_log->indexFromTime(time));
			const QPoint pt1_alt(pt1.x(),plot()->transform(QwtPlot::yRight,alt));

			const double cadence = _data_log->cadence(_data_log->indexFromTime(time));
			const QPoint pt1_cadence(pt1.x(),plot()->transform(QwtPlot::yLeft,cadence));

			const QPoint offset(8,-5);
			const QwtPlotItemList item_list = plot()->itemList(QwtPlotCurve::Rtti_PlotCurve);
			for (int i = 0; i < item_list.size(); ++i) 
			{
				if (item_list.at(i)->title().text() == "Heart Rate" && item_list.at(i)->isVisible())
				{
					painter->drawEllipse(pt1_hr,3,3);
					painter->drawText(pt1_hr + offset, QString::number(hr,'g',3));
				}
				if (item_list.at(i)->title().text() == "Speed" && item_list.at(i)->isVisible())
				{
					painter->drawEllipse(pt1_speed,3,3);
					painter->drawText(pt1_speed + offset, QString::number(speed,'g',3));
				}
				if (item_list.at(i)->title().text() == "Elevation" && item_list.at(i)->isVisible())
				{
					painter->drawEllipse(pt1_alt,3,3);
					painter->drawText(pt1_alt + offset, QString::number(alt,'g',4));
				}
				if (item_list.at(i)->title().text() == "Cadence" && item_list.at(i)->isVisible())
				{
					painter->drawEllipse(pt1_cadence,3,3);
					painter->drawText(pt1_cadence + offset, QString::number(cadence,'g',3));
				}
			}
			painter->drawText(QPoint(pt1.x(), 10), "time: " + QString::number(time,'g',4));
		}
	}

private:
	DataLog* _data_log;
};

/******************************************************/
PlotWindow::PlotWindow()
{
	//_data_log = new DataLog();

	_plot = new QwtPlot();
	_plot->enableAxis(QwtPlot::yRight,true);
	_plot->setAxisAutoScale(QwtPlot::xBottom,true);

	QColor c;

	_curve_hr = new QwtPlotCurve("Heart Rate");
	c = HR_COLOUR;
	_curve_hr->setPen(c);
	_curve_hr->setYAxis(QwtPlot::yLeft);

	_curve_cadence = new QwtPlotCurve("Cadence");
	c = CADENCE_COLOUR;
	_curve_cadence->setPen(c);
	_curve_cadence->setYAxis(QwtPlot::yLeft);

	_curve_speed = new QwtPlotCurve("Speed");
	c = SPEED_COLOUR;
	_curve_speed->setPen(c);
	_curve_speed->setYAxis(QwtPlot::yLeft);

	_curve_alt = new QwtPlotCurve("Elevation");
	_curve_alt->setRenderHint(QwtPlotItem::RenderAntialiased);
	c = ALT_COLOUR;
	c.setAlpha(150);
	_curve_alt->setPen(c);
    _curve_alt->setBrush(c);
	_curve_alt->setYAxis(QwtPlot::yRight);

	_curve_alt->attach(_plot);
	_curve_speed->attach(_plot);
	_curve_cadence->attach(_plot);
	_curve_hr->attach(_plot);

	// Plot picker for numerical display
	_plot_picker1 = 
		new QwtPlotCustomPicker(QwtPlot::xBottom, QwtPlot::yLeft, _data_log, _plot->canvas());
	_plot_picker1->setRubberBandPen(QColor(Qt::black));
    _plot_picker1->setTrackerPen(QColor(Qt::black));
	_plot_picker1->setStateMachine(new QwtPickerTrackerMachine());
	connect(_plot_picker1, SIGNAL(moved(const QPointF&)), this, SLOT(setMarkerPosition(const QPointF&)));
	
	// Plot picker for drawing user selection
	_plot_picker2 = 
		new QwtPlotPicker(QwtPlot::xBottom, QwtPlot::yLeft, QwtPlotPicker::NoRubberBand, QwtPicker::AlwaysOff, _plot->canvas());
	_plot_picker2->setStateMachine(new QwtPickerDragPointMachine());
	connect(_plot_picker2, SIGNAL(appended(const QPointF&)), this, SLOT(beginSelection(const QPointF&)));
	connect(_plot_picker2, SIGNAL(moved(const QPointF&)), this, SLOT(endSelection(const QPointF&)));

	// Plot zoomer
	_plot_zoomer = new QwtPlotCustomZoomer(QwtPlot::xBottom, QwtPlot::yLeft, _plot->canvas());
	_plot_zoomer->setRubberBand(QwtPicker::UserRubberBand);
    _plot_zoomer->setRubberBandPen(QColor(Qt::white));
    _plot_zoomer->setTrackerMode(QwtPicker::AlwaysOff);
    _plot_zoomer->setMousePattern(QwtEventPattern::MouseSelect2,Qt::RightButton, Qt::ControlModifier);
    _plot_zoomer->setMousePattern(QwtEventPattern::MouseSelect3,Qt::RightButton);
	connect(_plot_zoomer, SIGNAL(zoomed(const QRectF&)), this, SLOT(zoomSelection(const QRectF&)));

	// Plot panner
	_plot_panner = new QwtPlotPanner(_plot->canvas());
	_plot_panner->setMouseButton(Qt::MidButton);
	connect(_plot_panner, SIGNAL(moved(int, int)), this, SLOT(panSelection(int, int)));
	connect(_plot_panner, SIGNAL(panned(int, int)), this, SLOT(panAndHoldSelection(int, int)));

	// Selection for x-axis measurement
	_x_axis_measurement = new QComboBox();
	_x_axis_measurement->insertItem(0,"Time");
	_x_axis_measurement->insertItem(1,"Distance");
	connect(_x_axis_measurement,SIGNAL(currentIndexChanged(int)), this, SLOT(xAxisUnitsChanged(int)));

	// Selection for graph plots
	_hr_cb = new QCheckBox("Heart Rate");
	_speed_cb = new QCheckBox("Speed");
	_alt_cb = new QCheckBox("Elevation");
	_cadence_cb = new QCheckBox("Cadence");
	_hr_cb->setChecked(true);
	_speed_cb->setChecked(true);
	_alt_cb->setChecked(true);
	_cadence_cb->setChecked(true);
	connect(_hr_cb, SIGNAL(stateChanged(int)),this,SLOT(curveSelectionChanged()));
	connect(_speed_cb, SIGNAL(stateChanged(int)),this,SLOT(curveSelectionChanged()));
	connect(_alt_cb, SIGNAL(stateChanged(int)),this,SLOT(curveSelectionChanged()));
	connect(_cadence_cb, SIGNAL(stateChanged(int)),this,SLOT(curveSelectionChanged()));
	
	// Layout the GUI
	QPalette plt;
	plt.setColor(QPalette::WindowText, HR_COLOUR);
	_hr_cb->setPalette(plt);
	plt.setColor(QPalette::WindowText, SPEED_COLOUR);
	_speed_cb->setPalette(plt);
	plt.setColor(QPalette::WindowText, ALT_COLOUR);
	_alt_cb->setPalette(plt);
	plt.setColor(QPalette::WindowText, CADENCE_COLOUR);
	_cadence_cb->setPalette(plt);

	QLabel* x_axis_measurement_label = new QLabel("X Axis:");
	x_axis_measurement_label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
	x_axis_measurement_label->setIndent(0);
	
	QWidget* plot_options_widget = new QWidget;
	QHBoxLayout* hlayout1 = new QHBoxLayout(plot_options_widget);
	hlayout1->addWidget(x_axis_measurement_label);
	hlayout1->addWidget(_x_axis_measurement);
	hlayout1->addSpacing(20);
	hlayout1->addWidget(_hr_cb);
	hlayout1->addWidget(_speed_cb);
	hlayout1->addWidget(_alt_cb);
	hlayout1->addWidget(_cadence_cb);

	QWidget* plot_widget = new QWidget;
	QHBoxLayout* hlayout2 = new QHBoxLayout(plot_widget);
	hlayout2->addWidget(_plot);
	
	QVBoxLayout* vlayout = new QVBoxLayout(this);
	vlayout->addWidget(plot_widget);
	vlayout->addWidget(plot_options_widget);
	resize(700,270);
}

/******************************************************/
PlotWindow::~PlotWindow()
{

}

/******************************************************/
void PlotWindow::displayRide(DataLog* data_log, GoogleMap* google_map, DataStatisticsView* stats_view)
{
	_data_log = data_log;
	_plot_picker1->setDataLog(_data_log);

	drawGraphs();
	show();
	
	// Connect this window to the google map
	connect(this, SIGNAL(setMarkerPosition(int)), google_map, SLOT(setMarkerPosition(int)));
	connect(this, SIGNAL(beginSelection(int)), google_map, SLOT(beginSelection(int)));
	connect(this, SIGNAL(endSelection(int)), google_map, SLOT(endSelection(int)));
	connect(this, SIGNAL(zoomSelection(int,int)), google_map, SLOT(zoomSelection(int,int)));
	connect(this, SIGNAL(deleteSelection()), google_map, SLOT(deleteSelection()));
	connect(this, SIGNAL(panSelection(int)), google_map, SLOT(moveSelection(int)));
	connect(this, SIGNAL(panAndHoldSelection(int)), google_map, SLOT(moveAndHoldSelection(int)));

	// Connect this window to the statistical viewer
	connect(this, SIGNAL(zoomSelection(int,int)), stats_view, SLOT(setSelection(int,int)));
}

/******************************************************/
void PlotWindow::drawGraphs()
{
	if (_x_axis_measurement->currentIndex() == 0) // time
	{
		_curve_hr->setRawSamples(&_data_log->time(0), &_data_log->heartRate(0), _data_log->numPoints());
		_curve_speed->setRawSamples(&_data_log->time(0), &_data_log->speed(0), _data_log->numPoints());
		_curve_cadence->setRawSamples(&_data_log->time(0), &_data_log->cadence(0), _data_log->numPoints());
		_curve_alt->setRawSamples(&_data_log->time(0), &_data_log->alt(0), _data_log->numPoints());
		
		_plot->setAxisScale(QwtPlot::xBottom, 0, _data_log->totalTime());
	}
	else // distance
	{
		_curve_hr->setRawSamples(&_data_log->dist(0), &_data_log->heartRate(0), _data_log->numPoints());
		_curve_speed->setRawSamples(&_data_log->dist(0), &_data_log->speed(0), _data_log->numPoints());
		_curve_cadence->setRawSamples(&_data_log->dist(0), &_data_log->cadence(0), _data_log->numPoints());
		_curve_alt->setRawSamples(&_data_log->dist(0), &_data_log->alt(0), _data_log->numPoints());
		
		_plot->setAxisScale(QwtPlot::xBottom, 0, _data_log->totalDist());
	}
	_plot->replot();
	_plot_zoomer->setZoomBase();
}

/******************************************************/
void PlotWindow::setMarkerPosition(const QPointF& point)
{
	if (_x_axis_measurement->currentIndex() == 0) // time
		emit setMarkerPosition(_data_log->indexFromTime(point.x()));
	else // distance
		emit setMarkerPosition(_data_log->indexFromDist(point.x()));
}		

/******************************************************/
void PlotWindow::beginSelection(const QPointF& point)
{
	_plot_picker1->setEnabled(false);
	if (_x_axis_measurement->currentIndex() == 0) // time
		emit beginSelection(_data_log->indexFromTime(point.x()));
	else // distance
		emit beginSelection(_data_log->indexFromDist(point.x()));
}

/******************************************************/
void PlotWindow::endSelection(const QPointF& point)
{
	if (_x_axis_measurement->currentIndex() == 0) // time
		emit endSelection(_data_log->indexFromTime(point.x()));
	else // distance
		emit endSelection(_data_log->indexFromDist(point.x()));
}

/******************************************************/
void PlotWindow::zoomSelection(const QRectF& rect)
{
	_plot_picker1->setEnabled(true);

	if (_plot_zoomer->zoomRectIndex() == 0) // if fully zoomed out
	{
		emit deleteSelection();
	}
	else // regular zoom
	{
		if (_x_axis_measurement->currentIndex() == 0) // time
			emit zoomSelection(_data_log->indexFromTime(rect.left()), _data_log->indexFromTime(rect.right()));
		else // distance
			emit zoomSelection(_data_log->indexFromDist(rect.left()), _data_log->indexFromDist(rect.right()));
	}
}

/******************************************************/
void PlotWindow::panSelection(int x, int y)
{
	emit panSelection(_plot->invTransform(QwtPlot::xBottom,x)-_plot->invTransform(QwtPlot::xBottom,0));
}

/******************************************************/
void PlotWindow::panAndHoldSelection(int x, int y)
{
	emit panAndHoldSelection(_plot->invTransform(QwtPlot::xBottom,x)-_plot->invTransform(QwtPlot::xBottom,0));
}

/******************************************************/
void PlotWindow::xAxisUnitsChanged(int idx)
{
	emit deleteSelection();
	drawGraphs();
}
/******************************************************/
void PlotWindow::curveSelectionChanged()
{
	if (_hr_cb->isChecked()) _curve_hr->show(); else _curve_hr->hide();
	if (_alt_cb->isChecked()) _curve_alt->show(); else _curve_alt->hide();
	if (_cadence_cb->isChecked()) _curve_cadence->show(); else _curve_cadence->hide();
	if (_speed_cb->isChecked()) _curve_speed->show(); else _curve_speed->hide();

	_plot->replot();
}
