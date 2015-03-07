#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QDirIterator>
#include <QFileDialog>
#include <QMainWindow>
#include <QMap>
#include <QMessageBox>
#include <QTime>
#include <QtXml/QDomDocument>
#include <QXmlStreamReader>

namespace Ui {
class MainWindow;
}

struct Xml_Search_Results {
    QMap<QString, QStringList> tag_values_map;

    int num_xml_docs;
    int num_tags;
    int num_values;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_actionSet_XML_Root_Path_triggered();
    void on_actionAbout_triggered();
    void on_actionQuit_triggered();

    void on_tags_listwidget_activated(const QModelIndex &index);

private:
    Ui::MainWindow *ui_;
    Xml_Search_Results results_;
    int current_row_;

    Xml_Search_Results Create_Xml_Search_Results_();
    QStringList Scan_For_Useful_Tags_(QXmlStreamReader &reader);
    void Status_(const QString &status_text);
    void Xml_Parse_ (QFile &file,
                     Xml_Search_Results &results,
                     QStringList &useful_tags = QStringList());
    void Xml_Search_(const QDir &dir, Xml_Search_Results &results);
    bool Is_Useful_Tag_(const QString &tag);

};

#endif // MAINWINDOW_H
