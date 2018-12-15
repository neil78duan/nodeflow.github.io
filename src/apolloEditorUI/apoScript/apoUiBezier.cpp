/* file apoUiBezier.h
*
* logic parser editor bezier
*
* create by duan
*
* 2017.2.10
*/

#include "apoScript/apoUiParam.h"
#include "apoUiBezier.h"
#include <QPainter>


apoUiBezier::apoUiBezier( QWidget *parent, const QPoint &startPos, const QPoint &endPos):
    m_parent(parent)
{
	m_type = LineParam;
	m_color = Qt::red;
	m_lineWid = 1;

	m_startCtrl = 0;
	m_endCtrl = 0 ;
    initPos(startPos, endPos);
}


apoUiBezier::apoUiBezier(QWidget *parent) : m_parent(parent)
{
	m_type = LineParam;

	m_color = Qt::red;
	m_lineWid = 1;

	m_startCtrl = 0;
	m_endCtrl = 0;

}

apoUiBezier::~apoUiBezier()
{
}

apoUiBezier &apoUiBezier::operator=(const apoUiBezier &r)
{
#define COPY_FROM_R(_a) _a = r._a ; 
	COPY_FROM_R(m_parent);
	COPY_FROM_R(m_start);
	COPY_FROM_R(m_end);

	COPY_FROM_R(m_ctrlPos1);
	COPY_FROM_R(m_ctrlPos2);


	COPY_FROM_R(m_startCtrl);
	COPY_FROM_R(m_endCtrl);

	COPY_FROM_R(m_color);
	COPY_FROM_R(m_lineWid);
	COPY_FROM_R(m_type);

	return *this;
}

void apoUiBezier::setDebug(bool bDebug )
{
	if (bDebug) {

		m_color = Qt::yellow;
	}
	else {

		m_color = Qt::red;
	}

}

void apoUiBezier::setAnchor(QWidget *fromCtrl, QWidget *toCtrl)
{
	m_startCtrl = fromCtrl;
	m_endCtrl = toCtrl;

	initCtrl();
}

void apoUiBezier::resetPath( const QPoint &startPos, const QPoint &endPos)
{
    initPos(startPos, endPos) ;
}

void apoUiBezier::paintTo(const QPoint &endPos)
{
	initPos(m_start, endPos);
}


void apoUiBezier::initCtrl()
{
	QSize size1 = m_startCtrl->size();
	QSize size2 = m_endCtrl->size();


	QPoint pt1 = m_startCtrl->mapToGlobal(QPoint(size1.width(), size1.height() / 2));
	QPoint pt2 = m_endCtrl->mapToGlobal(QPoint(0, size2.height() / 2));

	pt1 = m_parent->mapFromGlobal(pt1);
	pt2 = m_parent->mapFromGlobal(pt2);


	initPos(pt1, pt2);
}
void apoUiBezier::initPos(const QPoint &start,const QPoint &end)
{
    m_start = start ;
    m_end = end ;
	
	m_ctrlPos1.setX(m_start.x() + 50);
	m_ctrlPos1.setY(m_start.y() );

	m_ctrlPos2.setX(end.x() - 50);
	m_ctrlPos2.setY(end.y() );

}

void apoUiBezier::paintEvent()
{
	if (m_startCtrl){
		initCtrl();
	}
	paint();

}


void apoUiBezier::paint()
{
	QPainter painter(m_parent);

	if (m_type == LineGoto) {
		painter.setPen(QPen(Qt::darkYellow, (qreal)m_lineWid));

		QPoint pt2 (m_start.x() + 20, m_start.y());
		painter.drawLine(m_start, pt2);
		
		QPoint pt3(pt2.x() , pt2.y()+100);
		painter.drawLine( pt2, pt3);


		QPoint pt4(m_end.x() - 20, pt3.y());
		painter.drawLine(pt3, pt4);

		QPoint pt5(m_end.x()-20, m_end.y());
		painter.drawLine(pt4, pt5);

 		painter.drawLine( pt5, m_end);
	}
	else {

		painter.setPen(QPen(m_color, (qreal)m_lineWid));

		QPainterPath myPath;
		myPath.moveTo(m_start);
		myPath.cubicTo(m_ctrlPos1, m_ctrlPos2, m_end);
		painter.drawPath(myPath);
	}
}

void apoUiBezier::move(const QPoint &offset)
{
	initPos(m_start + offset, m_end + offset);
}

void apoUiBezier::onRemove()
{
	if (m_startCtrl){
		((apoBaseSlotCtrl *)m_startCtrl)->onDisconnect();
	}

	if (m_endCtrl && m_type != LineGoto){
		((apoBaseSlotCtrl *)m_endCtrl)->onDisconnect();
	}
}