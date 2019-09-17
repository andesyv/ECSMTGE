#include "inputwidget.h"
#include "mainwindow.h"
#include "ui_input.h"
#include "entitymanager.h"

InputWidget::InputWidget(MainWindow *mainWindow, QWidget *parent)
    : ComponentWidget(mainWindow, parent), ui(new Ui::Input)
{
    ui->setupUi(this);
}

void InputWidget::updateData()
{
    auto entity = mMainWindow->currentEntitySelected;
    if(entity)
    {
        if(auto input = mMainWindow->getEntityManager()->getComponent<InputComponent>(entity->entityId))
        {
            isUpdating = true;
            auto checkState = input->isCurrentlyControlled ? Qt::CheckState::Checked : Qt::CheckState::Unchecked;
            ui->checkBox_IsBeingControlled->setCheckState(checkState);
            isUpdating = false;
        }
    }
}

void InputWidget::on_checkBox_IsBeingControlled_toggled(bool checked)
{
    if(isUpdating) return;

    auto entity = mMainWindow->currentEntitySelected;
    if(entity)
    {
        if(auto comp = mMainWindow->getEntityManager()->getComponent<InputComponent>(entity->entityId))
        {
            comp->isCurrentlyControlled = checked;
        }
    }
}

void InputWidget::Remove()
{
    auto entity = mMainWindow->currentEntitySelected;
    if(entity)
    {
        if(mMainWindow->getEntityManager()->removeComponent<InputComponent>(entity->entityId))
        {
            widgetRemoved(this);
        }
    }
}
