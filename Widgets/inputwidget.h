#ifndef INPUTWIDGET_H
#define INPUTWIDGET_H

#include "componentwidget.h"

namespace Ui{
    class Input;
}

/** Widget for inputcomponents for the editor.
 * @brief Widget for directionallightcomponents for the editor.
 */
class InputWidget : public ComponentWidget
{
    Q_OBJECT

public:
    InputWidget(MainWindow *mainWindow, QWidget* parent = nullptr);

public slots:
    void on_checkBox_IsBeingControlled_toggled(bool checked);

protected slots:
    void Remove() override;

private:
    Ui::Input* ui;
};

#endif // INPUTWIDGET_H
