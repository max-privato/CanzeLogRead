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

  //Prepare QStringlist to be inserted into tree.codeNameTable

  // ********** longNames impelementation has just started.
  // ********** by far they do nothingare started to

  // Battery-related quantities (excluding individual cells)
  availableCodes.append("7ec.623203.24");
  availableNames.append("vBatHV");
  longNames.append("HV LBC voltage measure");

  availableCodes.append("7ec.623204.24");
  availableNames.append("iBatHV");
  longNames.append("HV LBC current measure");

  availableCodes.append("7ec.622002.24");
  availableNames.append("batSOC");
  longNames.append("HV Battery voltage");

  availableCodes.append("658.33");
  availableNames.append("batSOE");
  longNames.append("HV Battery voltage");

  availableCodes.append("7bb.6103.192");
  availableNames.append("batSOCreal");
  longNames.append("HV Battery voltage");

  availableCodes.append("e.44");
  availableNames.append("batTemp");
  longNames.append("HV Battery voltage");

  availableCodes.append("7bb.6161.96");
  availableNames.append("batTot_kms");
  longNames.append("HV Battery voltage");

  availableCodes.append("7bb.6161.120");
  longNames.append("HV Battery voltage");
  availableNames.append("batTot_kWh");


  // Propulsion related quantities:
  availableCodes.append("18a.27"); availableNames.append("coastingTorque");
  availableCodes.append("1f8.28"); availableNames.append("eleBrakeTorque1");     //ElecBrakeWheelsTorqueApplied:
  availableCodes.append("1f8.16"); availableNames.append("eleBrakeTorque2");    //TotalPotentialResistiveWheelsTorque;
  availableCodes.append("5d7.0"); availableNames.append("vhSpeed");

  // Mains and charging related quantities:
  availableCodes.append("e.56"); availableNames.append("pCharge_kW");
  availableCodes.append("793.625062.24"); availableNames.append("groundResis");
  availableCodes.append("793.62502c.24"); availableNames.append("vMains1");
  availableCodes.append("793.62502d.24"); availableNames.append("vMains2");
  availableCodes.append("793.62502e.24"); availableNames.append("vMains3");
  availableCodes.append("793.62503f.24"); availableNames.append("vMains12");
  availableCodes.append("793.625041.24"); availableNames.append("vMains23");
  availableCodes.append("793.625042.24"); availableNames.append("vMains31");
  availableCodes.append("793.622001.24"); availableNames.append("iMains1");
  availableCodes.append("793.62503a.24"); availableNames.append("iMains2");
  availableCodes.append("793.62503b.24"); availableNames.append("iMains3");
  availableCodes.append("793.62504a.24"); availableNames.append("pMains");

// Miscellaneous:
  availableCodes.append("42e.20"); availableNames.append("fanSpeed");
  availableCodes.append("42e.38"); availableNames.append("iPilotCharge");
  availableCodes.append("7ec.622005.24"); availableNames.append("vBat12V");

  // Cell voltage codes:
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
     availableCodes.append(codeStr);
     availableNames.append(nameStr);
  }
  // Cel temperature codes::
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
     availableCodes.append(codeStr);
     availableNames.append(nameStr);
  }
  // cell Balancing Switches::
  for (int i=16; i<105; i+=8){
     QString iStr, codeStr, nameStr;
     iStr.setNum(i);
     codeStr="7bb.6107."+iStr;
     int cell=i/8-1;
     if(cell<10){
       iStr.setNum(cell);
       iStr="0"+iStr;
     }  else
       iStr.setNum(cell);
     nameStr="balanceSwitch"+iStr;
     availableCodes.append(codeStr);
     availableNames.append(nameStr);
  }

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

  // Opening input file:
  QStringList inLines;
  QFile inFile(inFileName);
  if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
     ui->okLabel->setText("Unable to open file for reading!");
     return;
  }
   //for efficiency reasons I will limit code search to the first 1000 rows: I suppose that codes that are not there will not be in the remaining rows.
  int iRow=0;
  while (!inFile.atEnd()) {
    iRow++;
    if(iRow>1000)
      break;
    inLines.append(inFile.readLine());
  }

  //Update tree with codes and varialble names with the contents got from the read file:
  QTreeWidgetItem treeItem;
  treeItems.clear();
  ui->treeWidget->setColumnCount(2);
  for (int i=0; i<availableCodes.count(); i++){
    bool codeFound=false;
    foreach(QString line1,inLines){
      if(!codeFound && line1.contains(availableCodes[i])){
        codeFound=true;
        QStringList list;
//        list.append(availableCodes[i]+"   ");
        list.append(availableCodes[i]);
        list.append(availableNames[i]);
        treeItems.append(new QTreeWidgetItem((QTreeWidget*)0,list));
        treeItems[treeItems.count()-1]->setCheckState(0,Qt::Checked);
        treeItemsCol0.append(availableCodes[i]);
        treeItemsCol1.append(availableNames[i]);
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

  // Each checked field must give rise to a different ADF file:
  int checkedFiles=0;
  for(int i=0; i<treeItems.count(); i++){
    if(treeItems[i]->checkState(0))
       checkedFiles++;
  }
  QFile * outFiles;
  outFiles=new QFile[checkedFiles];
  QStringList outFileNames, shortOutFNames;

  for(int i=0; i<treeItems.count(); i++){
    if(!treeItems[i]->checkState(0))
       continue;
    QString fileName=inFileName;
    fileName.chop(4);
    QString str= treeItems[i]->text(1);
    fileName=fileName+"_"+treeItems[i]->text(1)+".ADF";
    outFileNames.append(fileName);
    int pos=fileName.lastIndexOf('/');
    QString shortName=fileName.mid(pos+1,fileName.count()-pos);
    shortOutFNames.append(shortName);
  }

  //*** the size of shortOutFNames and outFileNames MUST be equal to checkedFiles!!

  QTextStream * outStreams;
  outStreams=new QTextStream[checkedFiles];

  int currentField=0;
  // The number of outFiles and outStreams is equal to the checked values in treeItems (i.e. variable checkedFiles). Variable actualFile will count these files.
  for(int i=0; i<treeItems.count(); i++){
    if(!treeItems[i]->checkState(0))
       continue;
    outFiles[currentField].setFileName(outFileNames[currentField]);
    if(!outFiles[currentField].open(QIODevice::WriteOnly | QIODevice::Text)){
      ui->okLabel->setText("Unable to open file "+shortOutFNames[i]+" for writing!");
      return;
    }
    outStreams[currentField].setDevice(&outFiles[currentField]);
    currentField++;
  }

  //***Here currentFiels MUST be equal to checkedFiles!!


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

  int sampleCount=0;

  currentField=0;
  //Writing header lines
  for (int i=0; i<treeItems.count(); i++){
    if(treeItems[i]->checkState(0)){
      outStreams[currentField]<<header1<<"\n";
      outStreams[currentField]<<"t\t"<<treeItems[i]->text(1)<<"\n";
      currentField++;
    }
  }

  currentField=0;
  for (int i=0; i<treeItems.count(); i++){
      // Must add the two commas because othersswise I could search a subcode of a code that is a superset of what I'm seraching.
    QString searchStr= ","+treeItems[i]->text(0)+",";
    if(!treeItems[i]->checkState(0))
       continue;
    sampleCount=0;
    foreach(QString line1,inLines){
      if(line1.contains(searchStr)){
        QString outLine=processLine(line1,iniSecs);
        outStreams[currentField]<<outLine<<"\n";
        sampleCount++;
      }
    }
    currentField++;
  }
  for(int i=0; i<checkedFiles; i++){
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
  for(int i=0; i<treeItems.count(); i++)
    treeItems[i]->setCheckState(0,Qt::Checked);
}

void MainWindow::on_unselectBtn_clicked() {
    for(int i=0; i<treeItems.count(); i++)
      treeItems[i]->setCheckState(0,Qt::Unchecked);
}
