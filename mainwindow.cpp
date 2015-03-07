#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "QDebug"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow) {
    ui_->setupUi(this);

    current_row_ = -1;
}

MainWindow::~MainWindow() {
    delete ui_;
}

void MainWindow::on_actionSet_XML_Root_Path_triggered() {
    QString path = "";
    this->Status_("Select a Root folder to scan for XML files.");
    path = QFileDialog::getExistingDirectory(this,
                                             "Open XML Directory",
                                             "",
                                             QFileDialog::ShowDirsOnly |
                                             QFileDialog::DontResolveSymlinks);
    QDir dir(path);

    this->Status_(QString("Scaning %1 for XML.").arg(path));
    QTime start_time = QTime::currentTime();
    results_ = Create_Xml_Search_Results_();
    Xml_Search_(dir, results_);
    int total_time = start_time.msecsTo(QTime::currentTime());

    this->Status_(QString("Found %1 XML documents in %2, %3 Tags, "
                          "and %4 Values in %5ms.")
                  .arg(results_.num_xml_docs)
                  .arg(path)
                  .arg(results_.num_tags)
                  .arg(results_.num_values)
                  .arg(total_time));

    ui_->tags_listwidget->addItems(results_.tag_values_map.keys());
}

void MainWindow::on_actionAbout_triggered() {
    QMessageBox mb;
    mb.about(this, "About",
             "XML Tag Value Search\nVersion: 2015-03-06\nBy: @JDBoronkas");
    mb.show();
}

void MainWindow::on_actionQuit_triggered() {
    this->close();
}

Xml_Search_Results MainWindow::Create_Xml_Search_Results_() {
    Xml_Search_Results results;

    results.tag_values_map = QMap<QString, QStringList>();
    results.num_xml_docs = 0;
    results.num_tags = 0;
    results.num_values = 0;

    return results;
}

QStringList MainWindow::Scan_For_Useful_Tags_(QXmlStreamReader &reader) {
    QStringList tags;

    while (reader.atEnd() == false &&
           reader.hasError() == false) {
        reader.readNextStartElement();
        tags.append(reader.name().toString());
    }

    return tags;
}

void MainWindow::Status_(const QString &status_text) {
    ui_->statusBar->showMessage(status_text);
}

void MainWindow::Xml_Search_(const QDir &dir,
                             Xml_Search_Results &results) {
    QFileInfoList entries = dir.entryInfoList();

    foreach (QFileInfo entry, entries) {
        if (entry.fileName() == "." ||
            entry.fileName() == dir.currentPath() ||
            entry.fileName() == "..") {
            continue;
        }

        if (entry.isDir()) {
            QDir next_dir(entry.dir().path() +
                          QDir::separator() +
                          entry.fileName());
            Xml_Search_(next_dir, results);
        } else if (entry.completeSuffix()
                   .compare("xml", Qt::CaseInsensitive) == 0) {
            Xml_Parse_(QFile(entry.filePath()), results);
            results.num_xml_docs += 1;
        }
    }
}

void MainWindow::Xml_Parse_(QFile &file,
                            Xml_Search_Results &results,
                            QStringList &useful_tags) {
    if (file.open(QFile::ReadOnly) == false) {
        return;
    }

    if (useful_tags.count() == 0) {
        QXmlStreamReader pre_reader(file.readAll());
        useful_tags = Scan_For_Useful_Tags_(pre_reader);
        file.reset();
    }

    QXmlStreamReader reader(file.readAll());
    while (reader.atEnd() == false &&
           reader.hasError() == false) {
        reader.readNextStartElement();
        if (useful_tags.contains(reader.name().toString())) {
            QString name = reader.name().toString();

            // TODO if tag is top level, then this parses the rest of the
            //   document and everything in the file is skipped over.
            //   How to tell if tag is a child tag? (parse ahead)
            QString value = reader.readElementText();

            value = value.trimmed();
            if (value.compare("") == 0) {
                useful_tags.removeOne(name);
                return;
            }

            QStringList tmp;
            if (results.tag_values_map.contains(name)) {
                if (results.tag_values_map.value(name).contains(value)) {
                    continue;
                }

                tmp = results.tag_values_map.value(name);
                results.tag_values_map.remove(name);
            } else {
                results.num_tags += 1;
            }

            tmp.append(value);
            results.tag_values_map.insert(name, tmp);

            results.num_values += 1;
        }
    }

    file.close();
}

void MainWindow::on_tags_listwidget_activated(const QModelIndex &index)
{
    if (current_row_ == index.row()) {
        return;
    }

    ui_->values_listwidget->clear();

    current_row_ = index.row();

    QListWidgetItem *item = ui_->tags_listwidget->item(current_row_);
    QString key = item->text();

    ui_->values_listwidget->addItems(results_.tag_values_map.value(key));
}
