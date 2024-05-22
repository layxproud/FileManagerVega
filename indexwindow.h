#ifndef INDEXWINDOW_H
#define INDEXWINDOW_H

#include "trmlshell.h"
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
    QString currFileName;
    QString prevFileName;

public:
    explicit IndexWindow(QWidget *parent = nullptr);
    ~IndexWindow();

    void setFiles(const QStringList &files);
    void addDocumentData(const QString &filePath, const DocumentData &data);

private:
    Ui::IndexWindow *ui;

    void loadFormData(const DocumentData &data);
    void populateListWidget();
    void updateDocumentData(const QString &fileName);

private slots:
    void onFileSelected(QListWidgetItem *item);
    void onApplyButtonClicked();

signals:
    void indexFiles(const QMap<QString, DocumentData> &documentsData);
};

#endif // INDEXWINDOW_H
