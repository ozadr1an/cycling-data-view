#ifndef GOOGLEMAPCOLLAGE_H
#define GOOGLEMAPCOLLAGE_H

#include "latlng.h"

#include <qtxml/qdomdocument>
#include <QWidget.h>
#include <QPoint.h>
#include <QWebView.h>
#include <QMap.h>

#include <iostream>

class DataLog;
class TcxParser;
class FitParser;
class QComboBox;
class ColourBar;

class GoogleMapCollageWindow : public QWidget
{
	Q_OBJECT

 public:
	GoogleMapCollageWindow();
	~GoogleMapCollageWindow();

	// Display all the rides in filenames on a google map
	void displayRides(const std::vector<QString>& filenames);

 private:
	// Create the webpage to display google maps
	void createPage(std::ostringstream& page);

	std::string defineColours();
	std::string defineCoords();

	bool parse(const QString filename, DataLog* data_log);

	TcxParser* _tcx_parser;
	FitParser* _fit_parser;

	// The window to display google maps
	QWebView *_view;

	std::vector<std::pair<LatLng, int> > _accumulated_points; // first=lat,long, second=count
	std::vector<std::pair<int, double> > _accumulated_point_extra_info; // first=ride id, second=time
	ColourBar* _colour_bar;
	int _max_count;
};

#endif // GOOGLEMAPCOLLAGE_H