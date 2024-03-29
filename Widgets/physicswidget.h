#ifndef PHYSICSWIDGET_H
#define PHYSICSWIDGET_H

#include "componentwidget.h"

#include "GSL/gsl_math.h"
#include "GSL/vector3d.h"

namespace Ui{
    class Physics;
}

/** Widget for physiccomponents in the editor.
 * @brief Widget for physiccomponents in the editor.
 */
class PhysicsWidget : public ComponentWidget
{
    Q_OBJECT

public:
    explicit PhysicsWidget(MainWindow *mainWindow, QWidget* parent = nullptr);

private slots:
    void on_spinBox_Velocity_X_valueChanged(double arg1);

    void on_spinBox_Velocity_Y_valueChanged(double arg1);

    void on_spinBox_Velocity_Z_valueChanged(double arg1);

    void on_spinBox_Acceleration_X_valueChanged(double arg1);

    void on_spinBox_Acceleration_Y_valueChanged(double arg1);

    void on_spinBox_Acceleration_Z_valueChanged(double arg1);

    void on_spinBox_Mass_valueChanged(double arg1);

    void Remove() override;

private:
    Ui::Physics* ui;

    void setVelocity(const gsl::vec3& vel);
    void setAcceleration(const gsl::vec3& acc);
    void setMass(float mass);
};
#endif // PhysicsWIDGET_H
