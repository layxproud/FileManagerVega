#ifndef INDEXWINDOW_H
#define INDEXWINDOW_H

#include "trmlshell.h"
#include <QCloseEvent>
#include <QDialog>
#include <QListWidget>

namespace Ui {
class IndexWindow;
}

class IndexWindow : public QDialog
{
    Q_OBJECT

    QStringList filePaths;
    QMap<QString, DocumentData> documentsData;
    QString dbName;
    QString currFileName;
    QString prevFileName;

public:
    explicit IndexWindow(QWidget *parent = nullptr);
    ~IndexWindow();

    void setFiles(const QStringList &files);
    void setDbName(const QString &name);

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    Ui::IndexWindow *ui;

    void addDefaultIndexParams(const QString &filePath);
    void loadFormData(const DocumentData &data);
    void populateListWidget();
    void updateDocumentData(const QString &fileName);
    void clearScrollArea();

signals:
    void addFilesSignal();
    void indexFiles(
        const QString &dbName, bool calcWeightSim, const QMap<QString, DocumentData> &documentsData);

private slots:
    void onFileSelected(QListWidgetItem *item);
    void onRemoveButtonClicked();
    void onApplyButtonClicked();
};

#endif // INDEXWINDOW_H
