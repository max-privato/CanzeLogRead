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
  setAcceptDrops(true);
  ui->label_4->setText("");
  initialLabel3Text=ui->okLabel->text();
  updateClickedState();
}

MainWindow::~MainWindow() {
  delete ui;
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
  event->acceptProposedAction();//mette il puntatore in posizione di accettazione
                                // puts the pointer in the accept position
}

void MainWindow::dropEvent(QDropEvent *event)
{
  /* Funzione per il caricamento dei files che vengono "droppati" sulla finestra*/
  /* Function for loading files that are "dropped" on the window */

  const QMimeData *mimeData = event->mimeData();
  inFileName= mimeData->urls().at(0).path();
  inFileName.remove(0,1);
  ui->label_4->setText(inFileName);
  ui->okLabel->setText(initialLabel3Text);
}

void MainWindow::on_okButton_clicked(){
  QList <QByteArray> fields, names;
  QList <QByteArray> inLines, outLines; //stringhe contenenti le righe del file di ingresso e di uscita
  QFile inFile(inFileName);
  QByteArray line;
  lineCount=0;

  if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
     ui->okLabel->setText("Unable to open file for reading!");
     return;
  }

  //Set the fields to be searched:
  if(ui->batVoltCB->isChecked()){
    fields.append("7ec.623203.24");
    names.append("vBatHV");
  }
  if(ui->batCurrCB->isChecked()){
    fields.append("7ec.623204.24");
    names.append("iBatHV");
  }
  if(ui->eleTorqueCB->isChecked()){
    fields.append("1f8.28");
    names.append("eleBrakeTorque");
  }
  if(ui->eleTorqueCB->isChecked()){
    fields.append("18a.27");
    names.append("Coasting Torque");
  }

  // Each field must give rise to a different ADF file:
  outFiles=new QFile[names.count()];
  QStringList outFileNames;

  QStringList shortOutFNames;
  foreach(QString varName, names){
    QString fileName=inFileName;
    fileName.chop(4);
    fileName=fileName+"_"+varName+".ADF";
    outFileNames.append(fileName);
    int pos=fileName.lastIndexOf('/');
    QString shortName=fileName.mid(pos+1,fileName.count()-pos);
    shortOutFNames.append(shortName);
  }

  QTextStream * outStreams;
  outStreams=new QTextStream[names.count()];
  for(int i=0; i<names.count(); i++){
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
  QByteArray header1;
  header1=" //"+inLines[0].mid(0,4)+"-"+inLines[0].mid(4,2)+"-"+inLines[0].mid(6,2) +
             "; "+inLines[0].mid(8,2)+":"+inLines[0].mid(10,2);

  //Determining seconds corresponding to intial time to be substracted to all outputed seconds:
  double iniSecs;
  QByteArray hh=inLines[0].mid(8,2), mm=inLines[0].mid(10,2), ss=inLines[0].mid(12,5);
  ss.insert(2,".");
  //Compute seconds:
  iniSecs=ss.toDouble();
  //add minutes:
  iniSecs+=60.*mm.toDouble();
  //add hours:
  iniSecs+=3600.*hh.toDouble();

  int fieldNum=0,sampleCount=0;
  //Writing header lines
  for (int i=0; i<names.count(); i++){
    outStreams[i]<<header1<<"\n";
    outStreams[i]<<"t\t"<<names[i]<<"\n";
  }
  foreach (QByteArray field,fields){
    sampleCount=0;
    foreach(QByteArray line1,inLines){
      if(line1.contains(field)){
        QString outLine=processLine(line1,iniSecs);
        outStreams[fieldNum]<<outLine<<"\n";
        sampleCount++;
      }
    }
    fieldNum++;
  }
  for(int i=0; i<names.count(); i++){
    outFiles[i].close();
  }
  ui->okLabel->setText("Output files correctly created! ");
  delete[] outFiles;
  delete[] outStreams;
}

QByteArray MainWindow::processLine(QByteArray line_, double iniSecs_){
  /* Questa funzione analizza la riga in ingresso, letta dall'output dello Yokogawa
   * e determina la stringa contenente la corrispondente riga da scrivere poi sul file
   * di uscita.
   * In pratica decodifico la timestamp e il valore numerico.
   * Per il timestamp mantengo solo i secondi (fino ai ms);  la data la metto solo
   * nell'intestazione del file
  */
  QByteArray strSec, strVal, ret="";
  // per prima cosa prendo la stringa che contiene l'orario fino ai secondi, con tre dcimali:
  QByteArray hh=line_.mid(8,2), mm=line_.mid(10,2), ss=line_.mid(12,5), sValue;
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

void MainWindow::on_batCurrCB_clicked(){
  updateClickedState();
}

void MainWindow::on_batVoltCB_clicked(){
    updateClickedState();
}

void MainWindow::on_eleTorqueCB_clicked(){
    updateClickedState();
}

void MainWindow::on_coastTorqueCB_clicked(){
    updateClickedState();
}


void MainWindow::updateClickedState(){
  if(ui->batCurrCB->isChecked()||ui->batVoltCB->isChecked()||
    ui->eleTorqueCB->isChecked()||ui->coastTorqueCB->isChecked() ){
    ui->okButton->setEnabled(true);
    ui->okLabel->setEnabled(true);
  }else{
    ui->okButton->setEnabled(false);
    ui->okLabel->setEnabled(false );
  }
}


