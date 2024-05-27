#ifndef MATCHLEVELWINDOW_H
#define MATCHLEVELWINDOW_H

#include <QCloseEvent>
#include <QDialog>

namespace Ui {
class MatchLevelWindow;
}

class MatchLevelWindow : public QDialog
{
    Q_OBJECT

public:
    explicit MatchLevelWindow(QWidget *parent = nullptr);
    ~MatchLevelWindow();

    void setDbName(const QString &name) { dbName = name; }
    void setPortrait(const QString &name, long id);
    void setMatchPortrait(const QString &name, long id);

protected:
    virtual void closeEvent(QCloseEvent *event) override;

private:
    Ui::MatchLevelWindow *ui;
    QString dbName;
    QString inputPortraitName;
    long inputPortraitID;
    QString matchPortraitName;
    long matchPortraitID;
    QString requestText;

signals:
    void addPortraitsSignal();
    void findMatchLevelSignal(
        const QString &dbName, long inputID, const QString &type, const QString &request);

private slots:
    void onRadioButton();
    void onRemoveButton();
    void onApplyButton();
};

#endif // MATCHLEVELWINDOW_H
