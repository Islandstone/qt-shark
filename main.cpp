#include <QtGui>
#include <QWidget>
#include <QFile>

#define MOVE_THRESHOLD 3
#define WIDGET_SIZE 128
#define BUTTON_FRACTION 0.25f

#define DEFAULT_WINDOW_FLAGS (Qt::FramelessWindowHint)

QString getShortcutFilePath() {
	// Win7
	QString appdata = QDesktopServices::storageLocation(QDesktopServices::DataLocation).remove("Local") + "Roaming"; // + "/GroovesharkDesktop.7F9BF17D6D9CB2159C78A6A6AB076EA0B1E0497C.1/Local Store/shortcutAction.txt";
	
	QDir dir(appdata);
	dir.setFilter(QDir::Dirs);

	QString groovesharkFolder = "";

    QFileInfoList list = dir.entryInfoList();
    for (int i = 0; i < list.size(); ++i) {
        QFileInfo fileInfo = list.at(i);

		if (fileInfo.fileName().contains("GroovesharkDesktop")) {
			groovesharkFolder = fileInfo.fileName();
		}
    }

	return appdata + "/" + groovesharkFolder + "/Local Store/shortcutAction.txt";
}

class GroovesharkWidget : public QWidget 
{
private:
    QImage *m_pImage;
	QImage *m_pPlayPause;
	QImage *m_pNext;
	QImage *m_pPrev;
	QImage *m_pUp;
	QImage *m_pDown;
	QImage *m_pMove;

    bool m_bMouseButtonDown;
	bool m_bAlwaysOnTop;
	bool m_bIsMoving;
    QPoint m_vStartPoint;
    QPoint m_vClickOffset;
	QFile *m_pFile;
	QTextStream *out;
	bool hasFocus;

public:
    GroovesharkWidget() : QWidget(NULL, DEFAULT_WINDOW_FLAGS) {
        resize(128, 128);

        m_pImage = new QImage("grooveshark128.png" );
		m_pPlayPause = new QImage("playpause.png");
		m_pNext = new QImage("next.png");
		m_pPrev = new QImage("prev.png");
		m_pUp = new QImage("up.png");
		m_pDown = new QImage("down.png");
		m_pMove = new QImage("move.png");

        setStyleSheet("background:transparent;");
        setAttribute(Qt::WA_TranslucentBackground);
		qDebug() << getShortcutFilePath();
        m_pFile = new QFile(getShortcutFilePath());
		out = new QTextStream(m_pFile);
		m_bMouseButtonDown = false;
		m_bAlwaysOnTop = false;
		m_bIsMoving = false;
		setFocusPolicy(Qt::StrongFocus);

		hasFocus = false;

		QObject::startTimer(60);
		
    }

    ~GroovesharkWidget() {
		//m_pFile->close();
		delete out;
		delete m_pFile;
        delete m_pImage;
    }

signals:

    void mouseClickEvent(QPoint pos) {
		if (!m_pFile->open(QIODevice::WriteOnly | QIODevice::Text)) {
			qDebug() << "Failed to open file";
			return;
		}

		if ( pos.x() <= WIDGET_SIZE * BUTTON_FRACTION ) {
			// Back
			*out << "previous" << "\n";
		} else if ( pos.x() >= WIDGET_SIZE - (WIDGET_SIZE * BUTTON_FRACTION) ) {
			// Forward
			*out << "next" << "\n";
		} else if ( pos.y() <= WIDGET_SIZE * BUTTON_FRACTION ) {
			// Volume up
			*out << "volumeup" << "\n";
		} else if ( pos.y() >= WIDGET_SIZE - (WIDGET_SIZE * BUTTON_FRACTION) ) {
			// Volume down
			*out << "volumedown" << "\n";
		} else {
			// Play/pause
			*out << "playpause" << "\n";
		}
	
		if (!m_pFile->flush()) {
			qDebug() << "Failed to write file";
		}		

		m_pFile->close();
    }

	double getOpacity() {
		if (!m_bAlwaysOnTop) return 1.0;

		QPoint pos = QCursor::pos();
		QPoint center = this->pos() + QPoint(WIDGET_SIZE / 2, WIDGET_SIZE / 2);
		QPoint vec = center - pos;

		double dist = sqrt( double( (vec.x()*vec.x()) + (vec.y()*vec.y()) ) );

		if (dist <= 64.0) return 1.0;
		return 1.0 - ((1.0/350.0) * dist);
	}

public:

    void paintEvent(QPaintEvent * /*event*/) {
        QPainter painter(this);
		painter.setOpacity( getOpacity() );
		//qDebug() << getOpacity();
        painter.drawImage(0,0, *m_pImage);

		if (m_bIsMoving) {
			painter.drawImage(32, 32, *m_pMove);
			return;
		}

		QPoint pos = this->mapFromGlobal(QCursor::pos());

		if ( pos.x() > 0 && pos.x() < WIDGET_SIZE && pos.y() > 0 && pos.y() < WIDGET_SIZE ) {
			if ( pos.x() <= WIDGET_SIZE * BUTTON_FRACTION ) {
				// Back
				painter.drawImage(32, 32, *m_pPrev);
			} else if ( pos.x() >= WIDGET_SIZE - (WIDGET_SIZE * BUTTON_FRACTION) ) {
				// Forward
				painter.drawImage(32, 32, *m_pNext);
			} else if ( pos.y() <= WIDGET_SIZE * BUTTON_FRACTION ) {
				// Volume up
				painter.drawImage(32, 32, *m_pUp);
			} else if ( pos.y() >= WIDGET_SIZE - (WIDGET_SIZE * BUTTON_FRACTION) ) {
				// Volume down
				painter.drawImage(32, 32, *m_pDown);
			} else  {
				// Play/pause
				painter.drawImage(32, 32, *m_pPlayPause);
			}
		}

		
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

			m_bIsMoving = true;

            this->move(event->globalPos() - m_vClickOffset);
        }
    }

	void timerEvent(QTimerEvent* event) {
		repaint();
	}

	void focusInEvent(QFocusEvent* event) {
		hasFocus = true;
		repaint();
	}

	void focusOutEvent(QFocusEvent* event) {
		hasFocus = false;
		repaint();
	}	

    void mouseReleaseEvent(QMouseEvent* event)
    {
        if (event->button() == Qt::LeftButton) {
            m_bMouseButtonDown = false;
			m_bIsMoving = false;

            QPoint newPos = m_vStartPoint - event->globalPos();

            if ( newPos.manhattanLength() < MOVE_THRESHOLD )
                mouseClickEvent(event->pos());
		} else if (event->button() == Qt::RightButton) {
			if (m_bAlwaysOnTop) {
				this->setWindowFlags(DEFAULT_WINDOW_FLAGS);
				this->show();
				m_bAlwaysOnTop = false;
			} else {
				this->setWindowFlags(DEFAULT_WINDOW_FLAGS | Qt::WindowStaysOnTopHint);
				this->show();
				m_bAlwaysOnTop = true;
			}
		} else if (event->button() == Qt::MiddleButton) {
			this->close();
		}

    }
};


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
	app.setQuitOnLastWindowClosed(true);

    GroovesharkWidget window;
    window.show();

    return app.exec();
}
