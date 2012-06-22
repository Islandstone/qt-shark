#include <QtGui>
#include <QWidget>

#define MOVE_THRESHOLD 3

class GroovesharkWidget : public QWidget 
{
private:
    QImage *m_pImage;
    bool m_bMouseButtonDown;
    QPoint m_vStartPoint;
    QPoint m_vClickOffset;

public:
    GroovesharkWidget() : QWidget(NULL, Qt::FramelessWindowHint) {
        resize(128, 128);
        m_pImage = new QImage("grooveshark128.png");
        setStyleSheet("background:transparent;");
        setAttribute(Qt::WA_TranslucentBackground);
    }

    ~GroovesharkWidget() {
        delete m_pImage;
    }

signals:

    void mouseClickEvent() {
        qDebug() << "Click";
    }

public:

    void paintEvent(QPaintEvent * /*event*/) {
        QPainter painter(this);
        painter.drawImage(0,0, *m_pImage);
    }

    void mousePressEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            m_bMouseButtonDown = true;
            m_vClickOffset = event->globalPos() - this->pos();
            m_vStartPoint = event->globalPos();
        }
    }

    void mouseMoveEvent(QMouseEvent* event)
    {
        if ( event->buttons().testFlag(Qt::LeftButton) && m_bMouseButtonDown)
        {
            QPoint newPos = m_vStartPoint - event->globalPos();

            if ( newPos.manhattanLength() < MOVE_THRESHOLD )
                return;

            this->move(event->globalPos() - m_vClickOffset);
        }
    }

    void mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton)
        {
            m_bMouseButtonDown = false;

            QPoint newPos = m_vStartPoint - event->globalPos();

            if ( newPos.manhattanLength() < MOVE_THRESHOLD )
                mouseClickEvent();
        }
    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    GroovesharkWidget window;
    window.show();

    return app.exec();
}
