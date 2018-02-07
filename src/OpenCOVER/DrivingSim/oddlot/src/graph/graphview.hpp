/* This file is part of COVISE.

   You can use it under the terms of the GNU Lesser General Public License
   version 2.1 or later, see lgpl-2.1.txt.

 * License: LGPL 2+ */

/**************************************************************************
** ODD: OpenDRIVE Designer
**   Frank Naegele (c) 2010
**   <mail@f-naegele.de>
**   04.02.2010
**
**************************************************************************/

#ifndef GRAPHVIEW_HPP
#define GRAPHVIEW_HPP

#include <QGraphicsView>
#include <QRubberBand>
#include <QNetworkAccessManager>
#include <QUrl>

class TopviewGraph;
class GraphScene;
class ZoomTool;

class Ruler;
class ScenerySystemItem;
class GraphViewShapeItem;


class QFile;
class QSslError;
class QAuthenticator;
class QNetworkReply;

class ToolAction;

class GraphView : public QGraphicsView
{
    Q_OBJECT

public:
    enum BoundingBoxStatusId // BoundingBox Button Status
    {
        BBOff, // BoundingBox selection mode not selected
        BBActive, // BoundingBox active
        BBPressed // BoundingBox Button pressed, but not active, e.g. during pan
    };

    enum CircleStatusId // Circle Button Status
    {
        CircleOff, // Circle selection mode not selected
        CircleActive, // Circle active
        CirclePressed // Circle Button pressed, but not active, e.g. during pan
    };

public:
    explicit GraphView(GraphScene *graphScene, TopviewGraph *topviewGraph);
    virtual ~GraphView();

    double getScale() const;
    QPointF getCircleCenter()
    {
        return circleCenter_;
    };
	void deleteCircle();

    void resetViewTransformation();

    //################//
    // EVENTS         //
    //################//

public:
    virtual void resizeEvent(QResizeEvent *event);
    virtual void scrollContentsBy(int dx, int dy);

    virtual void mousePressEvent(QMouseEvent *event);
    virtual void mouseReleaseEvent(QMouseEvent *event);
    virtual void mouseMoveEvent(QMouseEvent *event);

    virtual void wheelEvent(QWheelEvent *event);

    virtual void keyPressEvent(QKeyEvent *event);
    virtual void keyReleaseEvent(QKeyEvent *event);

    virtual void contextMenuEvent(QContextMenuEvent *e);

protected:
	virtual void dragEnterEvent(QDragEnterEvent *event);
	virtual void dragMoveEvent(QDragMoveEvent *event);
	virtual void dropEvent(QDropEvent *event);

    //################//
    // SLOTS          //
    //################//

public slots:
    // Tools, Mouse & Key //
    //
    void toolAction(ToolAction *);
    //	void						mouseAction(MouseAction *);
    //	void						keyAction(KeyAction *);

    // Rulers //
    //
    void rebuildRulers();
    void activateRulers(bool activate);

    // Zoom //
    //
    void zoomTo(const QString &zoomFactor);
    void zoomIn();
    void zoomIn(double zoom);
    void zoomOut();
    void zoomBox();
    void viewSelected();
    void scaleView(qreal sx, qreal sy);
    double getScaling() 
    {
        return scaling_;
    }

    // Background Images //
    //
    void loadMap();
    void loadGoogleMap();
    void loadBingMap();

    void deleteMap();
    void lockMap(bool lock);
    void setMapOpacity(const QString &opacity);
    void setMapX(double x);
    void setMapY(double y);
    void setMapWidth(double width, bool keepRatio);
    void setMapHeight(double height, bool keepRatio);
    void setMap(float x,float y,float width,float height,int xRes,int yRes,const char *buf);

    void setShapeEdit(bool edit)
    {
        doShapeEdit_ = edit;
    }

	void shapeEditing(bool edit);

    void setSplineControlPoints(const QVector<QPointF> &controlPoints)
    {
        splineControlPoints_ = controlPoints;
    }

    QVector<QPointF> getSplineControlPoints()
    {
        return splineControlPoints_;
    } 

    void clearSplineControlPoints()
    {
        splineControlPoints_.clear();
    }

    //################//
    // PROPERTIES     //
    //################//

private:
    TopviewGraph *topviewGraph_;
    GraphScene *graphScene_;
	ZoomTool *zoomTool_;

    bool doPan_;
    bool doKeyPan_;
	bool select_;

    BoundingBoxStatusId doBoxSelect_;
    CircleStatusId doCircleSelect_;
    double radius_;
    QGraphicsPathItem *circleItem_;
    QPointF circleCenter_;

    bool doShapeEdit_;
    GraphViewShapeItem *shapeItem_;

    Ruler *horizontalRuler_;
    Ruler *verticalRuler_;
    bool rulersActive_;

    QPoint mp_;
    QRubberBand *rubberBand_;
    bool additionalSelection_;

    // ScenerySystem //
    //
    ScenerySystemItem *scenerySystemItem_;
    QGraphicsPixmapItem *backgroundItem;

    double scaling_;

    QVector<QPointF> splineControlPoints_;

	/// http wget
	void startRequest(const QUrl &requestedUrl);

	private slots:
	void downloadFile(const QString &fn, const QString &url);
	void cancelDownload();
	void httpFinished();
	void httpReadyRead();
	void slotAuthenticationRequired(QNetworkReply *, QAuthenticator *authenticator);
#ifndef QT_NO_SSL
	void sslErrors(QNetworkReply *, const QList<QSslError> &errors);
#endif
	private:
		QUrl url;
		QNetworkAccessManager qnam;
		QNetworkReply *reply;
		QFile *file;
		bool httpRequestAborted;
		void wgetInit();
		QFile *openFileForWrite(const QString &fileName);

};

#endif // GRAPHVIEW_HPP
