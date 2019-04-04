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

#ifndef WIDGET_H
#define WIDGET_H

#include <QColor>
#include <QVector>
#include <QLabel>
#include <QWidget>

namespace Ui {
class ColorPickerWidget;
}

class ScreenWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ScreenWidget(QWidget *parent = nullptr);
    ~ScreenWidget();
};

class MagnifierWidget : public QLabel
{
    Q_OBJECT

public:
    explicit MagnifierWidget(QWidget *parent = nullptr);
    ~MagnifierWidget();

    void update(QScreen *screen,
                const QPoint &cursorPosition,
                const QColor &pixelColor);
};

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = nullptr);
    ~Widget();

    bool eventFilter(QObject *watched, QEvent *event) Q_DECL_OVERRIDE;
    QColor grabPixelColor(QScreen *screen, const QPoint &position);
    void updatePixelColor(QWidget *screenWidget);
    void pickColor(QScreen *screen, const QPoint &position);
    void updateClipboard(const QString &colorName);
    void deleteScreenWidgets();

private slots:
    void on_pushButton_activate_clicked();
    void on_pushButton_lighter_clicked();
    void on_pushButton_main_clicked();
    void on_pushButton_darker_clicked();

private:
    Ui::ColorPickerWidget *ui;
    MagnifierWidget *m_magnifier;
    QVector<QWidget*> m_screenWidgets;
    QColor m_color;
};

#endif // WIDGET_H
