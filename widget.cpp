/*******************************************************************************
 * MIT License
 *
 * This file is part of the SmallColorPicker project:
 * https://github.com/wxmaper/SmallColorPicker
 *
 * Copyright (c) 2019 Aleksandr Kazantsev (https://wxmaper.ru)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "widget.h"
#include "ui_widget.h"

#include <QMouseEvent>
#include <QDesktopWidget>
#include <QScreen>
#include <QClipboard>
#include <QPainter>

ScreenWidget::ScreenWidget(QWidget *parent)
    : QWidget(parent)
{
    static const QCursor DEFAULT_CURSOR(QPixmap(":/assets/cursor.png"), 0, 0);

    setWindowFlags(Qt::Tool
                   | Qt::FramelessWindowHint
                   | Qt::NoDropShadowWindowHint
                   | Qt::WindowStaysOnTopHint);
    setWindowOpacity(0.005);
    setMouseTracking(true);
    setCursor(DEFAULT_CURSOR);
}

ScreenWidget::~ScreenWidget()
{}

MagnifierWidget::MagnifierWidget(QWidget *parent)
    : QLabel(parent)
{
    setWindowFlags(Qt::ToolTip
                   | Qt::FramelessWindowHint
                   | Qt::NoDropShadowWindowHint
                   | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);
    resize(90, 90);
}

MagnifierWidget::~MagnifierWidget()
{}

void MagnifierWidget::update(QScreen *screen,
                             const QPoint &cursorPosition,
                             const QColor &pixelColor)
{
    static const QPoint POSITION_OFFSET(30, 8);
    static const QPoint REGION_OFFSET(40, 40);
    static const QSize REGION_SIZE(80, 80);
    static const QPixmap MAGNIFIER_PIXMAP(":/assets/magnifier.png");

    move(cursorPosition + POSITION_OFFSET);

    // grab region
    QPixmap regionPixmap;
    if (screen != nullptr) {
        const QPoint regionPoint = cursorPosition - REGION_OFFSET;
        regionPixmap = screen->grabWindow(0, regionPoint.x(), regionPoint.y(),
                                          REGION_SIZE.width(), REGION_SIZE.height());
        QPixmap scaledRegion = regionPixmap.scaled(REGION_SIZE.width()*2, REGION_SIZE.height()*2,
                                                   Qt::IgnoreAspectRatio, Qt::FastTransformation);
        regionPixmap = scaledRegion.copy(QRect(REGION_OFFSET, REGION_SIZE));
    }

    // update cursor color
    QPixmap coloredBigCursorPixmap(":/assets/big-cursor.png");
    QPainter coloredBigCursorPainter(&coloredBigCursorPixmap);
    coloredBigCursorPainter.setCompositionMode(QPainter::RasterOp_SourceAndDestination);
    coloredBigCursorPainter.fillRect(coloredBigCursorPixmap.rect(), pixelColor);
    coloredBigCursorPainter.end();

    // update magnifier scaled picture
    QPixmap magnifier(90, 90);
    magnifier.fill(Qt::transparent);

    QPainter magnifierPainter(&magnifier);
    magnifierPainter.setPen(Qt::transparent);
    magnifierPainter.setBrush(regionPixmap.toImage());
    magnifierPainter.drawRoundedRect(6, 6, 78, 78, 39, 39);

    magnifierPainter.drawPixmap(QRect(REGION_OFFSET, coloredBigCursorPixmap.size()),
                                coloredBigCursorPixmap);
    magnifierPainter.drawPixmap(QRect(QPoint(0, 0), MAGNIFIER_PIXMAP.size()),
                                MAGNIFIER_PIXMAP);
    magnifierPainter.end();

    setPixmap(magnifier);
}

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColorPickerWidget),
    m_magnifier(new MagnifierWidget)
{
    ui->setupUi(this);
    setFixedSize(size());
    setStyleSheet("#ColorPickerWidget { background: white; }");
}

Widget::~Widget()
{
    delete ui;
    delete m_magnifier;
}

bool Widget::eventFilter(QObject *watched, QEvent *event)
{
    ScreenWidget *screenWidget = qobject_cast<ScreenWidget*>(watched);

    if (screenWidget != nullptr) {
        QMouseEvent *mouseEvent = nullptr;

        switch (event->type()) {
        case QEvent::MouseButtonPress:
            mouseEvent = static_cast<QMouseEvent*>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                updatePixelColor(screenWidget);
            }
            else deleteScreenWidgets();
            return true;

        case QEvent::MouseButtonRelease:
            mouseEvent = static_cast<QMouseEvent*>(event);
            updatePixelColor(screenWidget);

            if (mouseEvent->button() == Qt::LeftButton) {
                const int screenNumber = QApplication::desktop()->screenNumber(screenWidget);
                QScreen *screen = QApplication::screens()[screenNumber];
                const QPoint cursorPosition = screenWidget->mapFromGlobal(QCursor::pos());
                deleteScreenWidgets(); // delete before final pick
                pickColor(screen, cursorPosition);
            }
            else {
                deleteScreenWidgets();
            }

            return true;

        case QEvent::MouseMove:
            updatePixelColor(screenWidget);
            break;

        default:
            /* nothing */
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

QColor Widget::grabPixelColor(QScreen *screen, const QPoint &position)
{
    const QPixmap pixmap = screen->grabWindow(0, position.x(), position.y(), 1, 1);
    const QRgb pixelValue = pixmap.toImage().pixel(0, 0);
    return QColor(pixelValue);
}

void Widget::deleteScreenWidgets()
{
    while (m_screenWidgets.size()) {
        QWidget *w = m_screenWidgets.takeFirst();
        w->hide();
        w->deleteLater();
    }

    m_magnifier->hide();
}

void Widget::updatePixelColor(QWidget *screenWidget)
{
    static const QString PCW_STYLESHEET("background:%1;");
    static const QPixmap CURSOR_PIXMAP(":/assets/cursor.png");

    const int screenNumber = QApplication::desktop()->screenNumber(screenWidget);
    QScreen *screen = QApplication::screens()[screenNumber];

    const QPoint cursorPosition = screenWidget->mapFromGlobal(QCursor::pos());
    m_color = grabPixelColor(screen, cursorPosition);

    // Repaint magnifier
    if (ui->magnifierCheckBox->isChecked()) {
        m_magnifier->update(screen, cursorPosition, m_color);
    }

    // Update cursor
    QPixmap coloredCursorPixmap(CURSOR_PIXMAP.size());
    coloredCursorPixmap.fill(Qt::transparent);

    QPainter coloredCursorPainter(&coloredCursorPixmap);
    coloredCursorPainter.setRenderHint(QPainter::Antialiasing);
    coloredCursorPainter.fillRect(2, 10, 22, 14, m_color);
    coloredCursorPainter.drawPixmap(0, 0, CURSOR_PIXMAP);
    coloredCursorPainter.end();

    screenWidget->setCursor(QCursor(coloredCursorPixmap, 0, 0));
}

void Widget::pickColor(QScreen *screen, const QPoint &position)
{
    static const QString BUTTON_STYLE(
                "QPushButton { border:1px solid rgba(0,0,0,0.5); background:%1; } "
                "QPushButton:hover { border:1px solid rgba(0,0,0,0.2); background:%2; } "
                "QPushButton:pressed { border:1px solid rgba(0,0,0,0.5); background:%3; } ");

    m_color = grabPixelColor(screen, position);

    int r = m_color.red();
    int g = m_color.green();
    int b = m_color.blue();

    int lr = qRound(double(255 - r) * 0.5 + double(r));
    int lg = qRound(double(255 - g) * 0.5 + double(g));
    int lb = qRound(double(255 - b) * 0.5 + double(b));
    QColor lighter = QColor::fromRgb(lr, lg, lb);
    if (lighter == m_color) lighter = m_color.lighter();

    int dr = qRound(double(0 + r) * 1.7 - double(r));
    int dg = qRound(double(0 + g) * 1.7 - double(g));
    int db = qRound(double(0 + b) * 1.7 - double(b));
    QColor darker = QColor::fromRgb(dr, dg, db);
    if (darker == m_color) darker = m_color.darker();

    ui->pushButton_main->setStyleSheet(BUTTON_STYLE.arg(m_color.name())
                                       .arg(m_color.lighter(110).name())
                                       .arg(m_color.darker(115).name()));
    ui->pushButton_main->setText(m_color.name());

    ui->pushButton_lighter->setStyleSheet(BUTTON_STYLE.arg(lighter.name())
                                          .arg(lighter.lighter(110).name())
                                          .arg(lighter.darker(115).name()));
    ui->pushButton_lighter->setText(lighter.name());

    ui->pushButton_darker->setStyleSheet(BUTTON_STYLE.arg(darker.name())
                                         .arg(darker.lighter(110).name())
                                         .arg(darker.darker(115).name()));
    ui->pushButton_darker->setText(darker.name());

    ui->lineEdit_main->setText(m_color.name());
    ui->lineEdit_lighter->setText(lighter.name());
    ui->lineEdit_darker->setText(darker.name());

    updateClipboard(m_color.name());
}

void Widget::updateClipboard(const QString &colorName)
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(colorName);
}

void Widget::on_pushButton_activate_clicked()
{
    const QList<QScreen *> screens = QGuiApplication::screens();

    foreach (QScreen *screen, screens) {
        ScreenWidget *screenWidget = new ScreenWidget();
        screenWidget->move(screen->availableGeometry().topLeft());
        screenWidget->resize(screen->size());
        screenWidget->installEventFilter(this);
        screenWidget->show();
        m_screenWidgets << screenWidget;
    }

    if (ui->magnifierCheckBox->isChecked()) {
        m_magnifier->update(nullptr, QCursor::pos(), Qt::white);
        m_magnifier->show();
    }
}

void Widget::on_pushButton_lighter_clicked()
{
    updateClipboard(ui->pushButton_lighter->text());
}

void Widget::on_pushButton_main_clicked()
{
    updateClipboard(ui->pushButton_main->text());
}

void Widget::on_pushButton_darker_clicked()
{
    updateClipboard(ui->pushButton_darker->text());
}
