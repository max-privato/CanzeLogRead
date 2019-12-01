#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDate>
#include <QMimeData>

/* Il formato del log contiene campioni delle varie grandezze a differenti istanti.
 * Per questa ragione un unico adf non p adatto. Di consegunenza per ogni filelog creerò
 * tanti adf, ognuno ad una sola variabile, quante sono le variabili presenti nel log
*/

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) ,
    ui(new Ui::MainWindow) {
  ui->setupUi(this);
  move(90,0);
  initialLabel3Text=ui->okLabel->text();
  setAcceptDrops(true);

  //Prepare qstringlist to be inserted into tree.codeNameTable
  treeCodes.append("7ec.623203.24"); treeNames.append("vBatHV");
  treeCodes.append("7ec.623204.24"); treeNames.append("iBatHV");
  treeCodes.append("7ec.622002.24"); treeNames.append("batSOC");
  treeCodes.append("7bb.6103.192"); treeNames.append("batSOCreal");
  treeCodes.append("18a.27"); treeNames.append("Coasting Torque");
  treeCodes.append("1f8.28"); treeNames.append("eleBrakeTorque1");     //ElecBrakeWheelsTorqueApplied:
  treeCodes.append("1f8.16"); treeNames.append("eleBrakeTorque2");    //TotalPotentialResistiveWheelsTorque;
  treeCodes.append("793.62504a.24"); treeNames.append("pMains");
  treeCodes.append("5d7.0"); treeNames.append("vhSpeed");
  // I codici per le tensioni di celle sono molti e li stabilisco programmaticamente:
  for (int i=16; i<993; i+=16){
     QString iStr, codeStr, nameStr;
     iStr.setNum(i);
     codeStr="7bb.6141."+iStr;
     int cell=i/16;
     if(cell<10){
       iStr.setNum(cell);
       iStr="0"+iStr;
     }  else
       iStr.setNum(cell);
     nameStr="vCell"+iStr;
     treeCodes.append(codeStr); treeNames.append(nameStr);
  }
  treeCodes.append("42e.20"); treeNames.append("fanSpeed");
  treeCodes.append("42e.38"); treeNames.append("iPilotCharge");
  treeCodes.append("e.44"); treeNames.append("batTemp");
  treeCodes.append("e.56"); treeNames.append("pCharge_kW");
  // I codici per le temperature di celle sono molti e li stabilisco programmaticamente:
  for (int i=32; i<997; i+=24){
     QString iStr, codeStr, nameStr;
     iStr.setNum(i);
     codeStr="7bb.6104."+iStr;
     int cell=i/24;
     if(cell<10){
       iStr.setNum(cell);
       iStr="0"+iStr;
     }  else
       iStr.setNum(cell);
     nameStr="tempCell"+iStr;
     treeCodes.append(codeStr); treeNames.append(nameStr);
  }


  /*
  // Prepare listTree:
  ui->treeWidget->setColumnCount(2);
  QStringList HeaderLabels;
  HeaderLabels.append("code");
  HeaderLabels.append("out varName");
  ui->treeWidget->setHeaderLabels(HeaderLabels);
  ui->treeWidget->setAlternatingRowColors(true);
  QStringList list;
  for(int  i=0; i<treeCodes.count(); i++){
    list.append(treeCodes[i]);
    list.append(treeNames[i]);
    QTreeWidgetItem treeItem;

    treeItems.append(new QTreeWidgetItem((QTreeWidget*)0,list));
    treeItems[treeItems.count()-1]->setCheckState(0,Qt::Checked);
    list.clear();
  }
   ui->treeWidget->insertTopLevelItems(0, treeItems);
   ui->treeWidget->resizeColumnToContents(0);
   ui->treeWidget->resizeColumnToContents(1);
   */
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event){
  event->acceptProposedAction();//mette il puntatore in posizione di accettazione
                                // puts the pointer in the accept position
}

void MainWindow::dropEvent(QDropEvent *event)
{
  /* Funzione per il caricamento dei files che vengono "droppati" sulla finestra*/
  /* Function for loading dropped files */

  const QMimeData *mimeData = event->mimeData();
  inFileName= mimeData->urls().at(0).path();
  inFileName.remove(0,1);
  ui->label_4->setText(inFileName);
  ui->okLabel->setText(initialLabel3Text);
  ui->okLabel->setEnabled(true);
  ui->okButton->setEnabled(true);

  // Ora procedo con l'individuazione dei codici presenti nel file
  QStringList inLines;
  QFile inFile(inFileName);
  if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
     ui->okLabel->setText("Unable to open file for reading!");
     return;
  }
   //per ragioni di efficienza mi limito a ricercare i codici nelle sole prime 1000 righe. Ridengo praticamente impossibile che oltre le prime 1000 righe visi posano trovare codici non già incontrati.
  int iRow=0;
  while (!inFile.atEnd()) {
    iRow++;
    if(iRow>1000)
      break;
    inLines.append(inFile.readLine());
  }

  QTreeWidgetItem treeItem;
  treeItems.clear();
  ui->treeWidget->setColumnCount(2);
  for (int i=0; i<treeCodes.count(); i++){
    bool codeFound=false;
    foreach(QString line1,inLines){
      if(!codeFound && line1.contains(treeCodes[i])){
        codeFound=true;
        QStringList list;
        list.append(treeCodes[i]+"   ");
        list.append(treeNames[i]);
        treeItems.append(new QTreeWidgetItem((QTreeWidget*)0,list));
        treeItems[treeItems.count()-1]->setCheckState(0,Qt::Checked);
      }
    }
  }
  ui->treeWidget->clear();
  ui->treeWidget->insertTopLevelItems(0, treeItems);
  ui->treeWidget->resizeColumnToContents(0);
  ui->treeWidget->resizeColumnToContents(1);
}

void MainWindow::on_okButton_clicked(){
  QStringList  inLines, outLines; //stringhe contenenti le righe del file di ingresso e di uscita
  QFile inFile(inFileName);
  QByteArray line;
  lineCount=0;

  if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
     ui->okLabel->setText("Unable to open file for reading!");
     return;
  }

  // Each field must give rise to a different ADF file:
  outFiles=new QFile[treeNames.count()];
  QStringList outFileNames;

  QStringList shortOutFNames;
  foreach(QString varName, treeNames){
    QString fileName=inFileName;
    fileName.chop(4);
    fileName=fileName+"_"+varName+".ADF";
    outFileNames.append(fileName);
    int pos=fileName.lastIndexOf('/');
    QString shortName=fileName.mid(pos+1,fileName.count()-pos);
    shortOutFNames.append(shortName);
  }

  QTextStream * outStreams;
  outStreams=new QTextStream[treeNames.count()];


  for(int i=0; i<treeNames.count(); i++){
    if(!treeItems[i]->checkState(0))
       continue;
      outFiles[i].setFileName(outFileNames[i]);
    if(!outFiles[i].open(QIODevice::WriteOnly | QIODevice::Text)){
      ui->okLabel->setText("Unable to open file "+shortOutFNames[i]+" for writing!");
      return;
    }
    outStreams[i].setDevice(&outFiles[i]);
  }


  //Bypassing header line:
  line = inFile.readLine();

  while (!inFile.atEnd()) {
    inLines.append(inFile.readLine());
  }

  //Creating output Header1 taking date-time info from the first line:
  QString header1;
  header1=" //"+inLines[0].mid(0,4)+"-"+inLines[0].mid(4,2)+"-"+inLines[0].mid(6,2) +
             "; "+inLines[0].mid(8,2)+":"+inLines[0].mid(10,2);

  //Determining seconds corresponding to intial time to be substracted to all outputed seconds:
  double iniSecs;
  QString hh=inLines[0].mid(8,2), mm=inLines[0].mid(10,2), ss=inLines[0].mid(12,5);
  ss.insert(2,".");
  //Compute seconds:
  iniSecs=ss.toDouble();
  //add minutes:
  iniSecs+=60.*mm.toDouble();
  //add hours:
  iniSecs+=3600.*hh.toDouble();

  int fieldNum=0,sampleCount=0;
  //Writing header lines
  for (int i=0; i<treeNames.count(); i++){
    outStreams[i]<<header1<<"\n";
    outStreams[i]<<"t\t"<<treeNames[i]<<"\n";
  }

  foreach (QString code,treeCodes){
    sampleCount=0;
    foreach(QString line1,inLines){
      if(line1.contains(code)){
        QString outLine=processLine(line1,iniSecs);
        outStreams[fieldNum]<<outLine<<"\n";
        sampleCount++;
      }
    }
    fieldNum++;
  }
  for(int i=0; i<treeNames.count(); i++){
    outFiles[i].close();
  }
  ui->okLabel->setText("Output files correctly created! ");
  delete[] outFiles;
  delete[] outStreams;
}

QString MainWindow::processLine(QString line_, double iniSecs_){
  /* Questa funzione analizza la riga in ingresso, letta dall'output dello Yokogawa
   * e determina la stringa contenente la corrispondente riga da scrivere poi sul file
   * di uscita.
   * In pratica decodifico la timestamp e il valore numerico.
   * Per il timestamp mantengo solo i secondi (fino ai ms);  la data la metto solo
   * nell'intestazione del file
  */
  QString strSec, strVal, ret="";
  // per prima cosa prendo la stringa che contiene l'orario fino ai secondi, con tre dcimali:
  QString hh=line_.mid(8,2), mm=line_.mid(10,2), ss=line_.mid(12,5), sValue;
  ss.insert(2,".");
  //Compute seconds:
  double timeS=ss.toDouble();
  //add minutes:
  timeS+=60.*mm.toDouble();
  //add hours:
  timeS+=3600.*hh.toDouble();

  timeS-=iniSecs_;
  // find value:
  int valueEndPos=line_.lastIndexOf(',');
  // Eliminate comma to allow next serch:
  line_[valueEndPos]=' ';
  //now last comma is just before the value:
  int valueStartPos=line_.lastIndexOf(',')+1;
  sValue=line_.mid(valueStartPos,valueEndPos-valueStartPos);

  ret.setNum(timeS);
  ret+="\t";
  ret+=sValue;
  return ret;
}

void MainWindow::on_selectBtn_clicked() {
  for(int i=0; i<treeNames.count(); i++)
    treeItems[i]->setCheckState(0,Qt::Checked);
}

void MainWindow::on_unselectBtn_clicked() {
    for(int i=0; i<treeNames.count(); i++)
      treeItems[i]->setCheckState(0,Qt::Unchecked);
}
