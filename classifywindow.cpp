#include "classifywindow.h"
#include "ui_classifywindow.h"
#include <QDebug>

ClassifyWindow::ClassifyWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ClassifyWindow)
    , portraitsWidget(new PortraitListWidget(this))
    , classesWidget(new PortraitListWidget(this))
{
    ui->setupUi(this);

    ui->portraitsLayout->addWidget(portraitsWidget);
    ui->classesLayout->addWidget(classesWidget);

    connect(ui->cancelButton, &QPushButton::clicked, this, &ClassifyWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &ClassifyWindow::onApplyButtonClicked);
    connect(
        portraitsWidget,
        &PortraitListWidget::addPortraitsSignal,
        this,
        &ClassifyWindow::addPortraits);
    connect(
        classesWidget, &PortraitListWidget::addPortraitsSignal, this, &ClassifyWindow::addPortraits);
}

ClassifyWindow::~ClassifyWindow()
{
    delete ui;
}

void ClassifyWindow::setPortraits(const QMap<QString, long> &p)
{
    portraitsWidget->setItems(p);
}

void ClassifyWindow::setClasses(const QMap<QString, long> &c)
{
    classesWidget->setItems(c);
}

void ClassifyWindow::setDbName(const QString &name)
{
    dbName = name;
    ui->dbNameInput->setText(dbName);
}

void ClassifyWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();
    event->accept();
}

void ClassifyWindow::addPortraits()
{
    PortraitListWidget *senderWidget = qobject_cast<PortraitListWidget *>(sender());
    if (senderWidget == portraitsWidget) {
        emit getPortraitsSignal();
    } else if (senderWidget == classesWidget) {
        emit getClassesSignal();
    }
}

void ClassifyWindow::onApplyButtonClicked()
{
    if (portraitsWidget->getItems().isEmpty() || classesWidget->getItems().isEmpty()) {
        qWarning() << "Не выбрано ни одного класса или портрета";
        return;
    }

    QList<long> portraitIDs;
    QList<long> classesIDs;

    QMap<QString, long> portraitsMap = portraitsWidget->getItems();
    for (auto it = portraitsMap.begin(); it != portraitsMap.end(); ++it) {
        portraitIDs.append(it.value());
    }

    QMap<QString, long> classesMap = classesWidget->getItems();
    for (auto it = classesMap.begin(); it != classesMap.end(); ++it) {
        classesIDs.append(it.value());
    }

    emit classifyPortraits(portraitIDs, classesIDs);
}
