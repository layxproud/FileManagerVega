#include "indexwindow.h"
#include "ui_indexwindow.h"

IndexWindow::IndexWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::IndexWindow)
{
    ui->setupUi(this);
    connect(ui->listWidget, &QListWidget::itemClicked, this, &IndexWindow::onFileSelected);

    currFileName = "";
    prevFileName = "";

    filePaths.clear();
    documentsData.clear();

    connect(ui->cancelButton, &QPushButton::clicked, this, &IndexWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &IndexWindow::onApplyButtonClicked);
    connect(ui->removeButton, &QPushButton::clicked, this, &IndexWindow::onRemoveButtonClicked);
    connect(ui->addButton, &QPushButton::clicked, this, &IndexWindow::addFilesSignal);
}

IndexWindow::~IndexWindow()
{
    delete ui;
}

void IndexWindow::setFiles(const QStringList &files)
{
    for (auto &file : files) {
        if (!filePaths.contains(file)) {
            filePaths.append(file);
            addDefaultIndexParams(file);
        }
    }

    populateListWidget();
}

void IndexWindow::setDbName(const QString &name)
{
    dbName = name;
    ui->dbNameInput->setText(dbName);
}

void IndexWindow::closeEvent(QCloseEvent *event)
{
    deleteLater();
    event->accept();
}

void IndexWindow::addDefaultIndexParams(const QString &filePath)
{
    DocumentData defaultData
        = {"0",
           "аа ",
           "1,2,3",
           "1,2,3",
           "<сойдет>",
           "4",
           "<desc_header>",
           "<desc_body>",
           "Новожилов А. А.",
           3,
           "<не знаю>",
           "<не знаю>",
           "",
           "https://link",
           "2022",
           "https://link",
           QList<int>{},
           3,
           "12000"};

    documentsData[filePath] = defaultData;
}

void IndexWindow::populateListWidget()
{
    ui->listWidget->clear();
    for (const QString &filePath : filePaths) {
        ui->listWidget->addItem(filePath);
    }
}

void IndexWindow::updateDocumentData(const QString &fileName)
{
    if (fileName == "") {
        return;
    }

    if (!documentsData.contains(fileName)) {
        return;
    }

    DocumentData &data = documentsData[fileName];
    data.id = ui->idInput->text();
    data.title = ui->titleInput->text();
    data.disciplines = ui->disciplinesInput->text();
    data.themes = ui->themesInput->text();
    data.comment = ui->commentInput->text();
    data.mark = ui->markInput->text();
    data.descHeader = ui->descHeaderInput->text();
    data.descBody = ui->descBodyInput->text();
    data.authors = ui->authorsInput->text();
    data.typeDocId = ui->typeDocIdInput->text().toInt();
    data.udk = ui->udkInput->text();
    data.bbk = ui->bbkInput->text();
    data.code = ui->codeInput->text();
    data.url = ui->urlInput->text();
    data.publishYear = ui->publishYearInput->text();
    data.publisher = ui->publisherInput->text();
    QString partsInputText = ui->partsInput->text();
    QStringList partsStringList = partsInputText.split(",", QString::SkipEmptyParts);
    QList<int> parts;
    for (const QString &partString : partsStringList) {
        bool ok;
        int part = partString.toInt(&ok);
        if (ok) {
            parts.append(part);
        } else {
            qCritical() << "Не удалось обработать части речи";
        }
    }
    data.parts = parts;
    data.shingle_length = ui->shingleLengthInput->text().toInt();
    data.collection_id = ui->collectionIdInput->text();
}

void IndexWindow::clearScrollArea()
{
    QWidget *scrollAreaWidget = ui->scrollArea->widget();
    if (!scrollAreaWidget)
        return;

    QList<QLineEdit *> lineEdits = scrollAreaWidget->findChildren<QLineEdit *>();
    foreach (QLineEdit *lineEdit, lineEdits) {
        lineEdit->clear();
    }
}

void IndexWindow::loadFormData(const DocumentData &data)
{
    ui->idInput->setText(data.id);
    ui->titleInput->setText(data.title);
    ui->disciplinesInput->setText(data.disciplines);
    ui->themesInput->setText(data.themes);
    ui->commentInput->setText(data.comment);
    ui->markInput->setText(data.mark);
    ui->descHeaderInput->setText(data.descHeader);
    ui->descBodyInput->setText(data.descBody);
    ui->authorsInput->setText(data.authors);
    ui->typeDocIdInput->setText(QString::number(data.typeDocId));
    ui->udkInput->setText(data.udk);
    ui->bbkInput->setText(data.bbk);
    ui->codeInput->setText(data.code);
    ui->urlInput->setText(data.url);
    ui->publishYearInput->setText(data.publishYear);
    ui->publisherInput->setText(data.publisher);
    QStringList stringList;
    for (int value : data.parts) {
        stringList.append(QString::number(value));
    }
    QString partsString = stringList.join(",");
    ui->partsInput->setText(partsString);
    ui->shingleLengthInput->setText(QString::number(data.shingle_length));
    ui->collectionIdInput->setText(data.collection_id);
}

void IndexWindow::onFileSelected(QListWidgetItem *item)
{
    prevFileName = currFileName;
    currFileName = item->text();
    updateDocumentData(prevFileName);

    QString filePath = item->text();
    if (documentsData.contains(filePath)) {
        loadFormData(documentsData[filePath]);
    }
}

void IndexWindow::onRemoveButtonClicked()
{
    QList<QListWidgetItem *> selectedItems = ui->listWidget->selectedItems();
    foreach (QListWidgetItem *item, selectedItems) {
        QString fileName = item->text();
        if (filePaths.contains(fileName)) {
            filePaths.removeOne(fileName);
            documentsData.remove(fileName);
        }
        delete ui->listWidget->takeItem(ui->listWidget->row(item));
    }
    clearScrollArea();
    ui->listWidget->clearSelection();
}

void IndexWindow::onApplyButtonClicked()
{
    updateDocumentData(currFileName);
    bool calcWeightSim = bool(ui->weightSimCombo->currentIndex());
    emit indexFiles(dbName, calcWeightSim, documentsData);
}
