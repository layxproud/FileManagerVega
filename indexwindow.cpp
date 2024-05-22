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

    connect(ui->cancelButton, &QPushButton::clicked, this, &IndexWindow::close);
    connect(ui->applyButton, &QPushButton::clicked, this, &IndexWindow::onApplyButtonClicked);
}

IndexWindow::~IndexWindow()
{
    delete ui;
}

void IndexWindow::setFiles(const QStringList &files)
{
    filePaths.clear();
    documentsData.clear();

    filePaths = files;

    DocumentData defaultData
        = {0,
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           0,
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           "aaa",
           "aaa"};

    for (const QString &filePath : filePaths) {
        documentsData[filePath] = defaultData;
    }

    populateListWidget();
}

void IndexWindow::addDocumentData(const QString &filePath, const DocumentData &data)
{
    documentsData[filePath] = data;
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
    data.id = ui->idInput->text().toInt();
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
}

void IndexWindow::loadFormData(const DocumentData &data)
{
    ui->idInput->setText(QString::number(data.id));
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

void IndexWindow::onApplyButtonClicked()
{
    updateDocumentData(currFileName);
    emit indexFiles(documentsData);
}
