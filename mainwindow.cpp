#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFile>
#include <QDate>
#include <QMimeData>

/* Il formato del log contiene campioni delle varie grandezze a differenti istanti.
 * Per questa ragione un unico adf non è adatto. Di consegunenza per ogni filelog creerò
 * tanti adf, ognuno ad una sola variabile, quante sono le variabili presenti nel log
*/

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent) ,
    ui(new Ui::MainWindow) {
  ui->setupUi(this);
  move(90,0);
  initialLabel3Text=ui->okLabel->text();
  setAcceptDrops(true);
  logFileLoaded=false;
  droppedShortNameFileName="";
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
  /* Function for loading dropped files.
   * Dropped log files are:
   * - first searched to find all the available codes
   * - then it is checked whether the We have short names for some of the codes via ShortName.txt
   * - then the tree is created allowingusers to select all or just some of the existing namest
   *   to be converted and saved in individual files
   *
   * the user can drop also the "shortNames.txt" file: this will override the one read
   * when reading the lof file or, in case the latter was not there, is however used to
   * choose short names.
*/
  bool shortNamesNowDropped=false; //initialization done just to avoid a warning
  QString snFileName; //filename of the file defining short names
  const QMimeData *mimeData = event->mimeData();

  ui->shortNamesLbl->setText("File ShortNames.txt not read");
  // If a log file has already been loaded we can drop a txt file to override existing short names
  //We must first see if the dropped file is ShortNames,txt or a ont (in the latter case it is alog file)

  QString droppedName=mimeData->urls().at(0).path();
  int i=droppedName.lastIndexOf('/')+1;
  QString shortedName=droppedName.mid(i,droppedName.count()-i);

  if(shortedName=="ShortNames.txt"){
    if(logFileLoaded){
      snFileName=droppedName.remove(0,1);
      ui->shortNamesLbl->setText("File ShortNames.txt dropped");
      shortNamesNowDropped=true;
    }else
      return;
  }else{
    shortNamesNowDropped=false;
    inFileName=mimeData->urls().at(0).path();  //short names file name
    loadFileAndFillLists(inFileName);
    snFileName=inFileName;
    snFileName.chop(snFileName.count()-snFileName.lastIndexOf('/')-1);
    snFileName=snFileName+"ShortNames.txt";
    snFileName=snFileName.remove(0,1);
    logFileLoaded=true;
  }

  int iRow;

  //Manage manual short names:
  QFile snFile(snFileName);
  bool snFileExists=false;
  if (snFile.open(QIODevice::ReadOnly | QIODevice::Text)){
    snFileExists=true;
  }

  if (snFileExists){
    iRow=0;
    QString line;
    while (!snFile.atEnd()) {
      line = snFile.readLine();
      QString code=line;
      code.truncate(line.indexOf(' '));
      QString shortName=line;
      shortName=shortName.mid(line.indexOf(' '),line.count()-line.indexOf(' ')-1);
      int index=codes.indexOf(code);
      if(index>-1)
        shortNames[index]=shortName;
      iRow++;
    }
    ui->shortNamesLbl->setText("File ShortNames.txt read");
  }
  snFile.close();


  //Fill Codes list and LongNames list:
  treeItems.clear();
  for (int i=0; i<codes.count(); i++){
    QStringList list;
    list.append(codes[i]);
    list.append(shortNames[i]);
    list.append(longNames[i]);
    treeItems.append(new QTreeWidgetItem((QTreeWidget*)0,list));
    treeItems[treeItems.count()-1]->setCheckState(0,Qt::Checked);
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
  QString actualLongName;
  // The number of outFiles and outStreams is equal to the checked values in treeItems (i.e. variable checkedFiles). Variable actualFile will count these files.
  for(int i=0; i<treeItems.count(); i++){
    if(!treeItems[i]->checkState(0))
       continue;
    outFiles[currentField].setFileName(outFileNames[currentField]);
    actualLongName=longNames[i];
    if(!outFiles[currentField].open(QIODevice::WriteOnly | QIODevice::Text)){
      ui->okLabel->setText("Unable to open file "+shortOutFNames[i]+" for writing!");
      return;
    }
    outStreams[currentField].setDevice(&outFiles[currentField]);
    currentField++;
  }

  //***Here currentField MUST be equal to checkedFiles!!

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
      outStreams[currentField]<<header1<<"  Long name: \""<<longNames[i]<<"\""<<"\n";
      outStreams[currentField]<<"t\t"<<treeItems[i]->text(1)<<"\n";
      currentField++;
    }
  }

  currentField=0;
  for (int i=0; i<treeItems.count(); i++){
      // Must add the two commas because othersswise I could search a subcode of a code that is a superset of what I'm searching.
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


void MainWindow:: loadFileAndFillLists(QString inFileName_){
  inFileName_.remove(0,1);
  ui->label_4->setText(inFileName_);
  ui->okLabel->setText(initialLabel3Text);
  ui->okLabel->setEnabled(true);
  ui->okButton->setEnabled(true);
  codes.clear();
  shortNames.clear();
  longNames.clear();


  // Opening input file:
  QStringList inLines;
  QFile inFile(inFileName_);
  if (!inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
     ui->okLabel->setText("Unable to open file for reading!");
     return;
  }
  // Now I search for the available codes.  For efficiency reasons, I will limit code search to the first 1000 rows: I suppose that codes that are not there will not be in the remaining rows.
  int iRow=0;
  while (!inFile.atEnd()) {
    QString line, code, longName;
    iRow++;
    if(iRow>1000)
      break;
    line=inFile.readLine();
    if(iRow==1)
      continue;
    int codeStart=line.indexOf(',');
   //eliminate comma to allow the next search to find the second comma in inLines[i]:
    line[codeStart]=' ';
    int codeStop=line.indexOf(',');
    code=line.mid(codeStart+1,codeStop-codeStart-1);
    if(!codes.contains(code)){
      codes.append(code);
      int endLongName=line.lastIndexOf(',');
      line.truncate(endLongName);
      endLongName=line.lastIndexOf(',');
      line.truncate(endLongName);
      int startLongName=line.lastIndexOf(',');
      longName=line.mid(startLongName+1,endLongName-startLongName-1);
      longNames.append(longName);

     // Initially short names are taken equal to the codes; then they will be possibly overridden from the values in ShortNames.txt.
      shortNames.append(code);
    }
  }


 // Here we define some short names which is impractical to manually put in shortNames.txt (v. Word documentation )

  // Cell voltages:
  for (int i=16; i<=1536; i+=16){
     QString iStr, iStr1, codeStr, nameStr;
     iStr.setNum(i);
     codeStr="7bb.6141."+iStr;
     if(i>992){
       iStr1.setNum(i-992);
       codeStr="7bb.6142."+iStr1;
     }

     int index= codes.indexOf(codeStr);
     int cell=i/16;
     if(cell<10){
       iStr.setNum(cell);
       iStr="0"+iStr;
     }  else
       iStr.setNum(cell);
     nameStr="vCell"+iStr;
     //Avoid segfault for possibly larger batteries of future ZOEs
     if(index<0 ||index>shortNames.count())
       break;
     shortNames[index]=nameStr;
  }

  // Cell temperature codes:
  for (int i=32; i<997; i+=24){
    QString iStr, codeStr, nameStr;
    iStr.setNum(i);
    codeStr="7bb.6104."+iStr;
    int index= codes.indexOf(codeStr);
    if(index<0)
      continue;
    int cell=i/24;
    if(cell<10){
      iStr.setNum(cell);
      iStr="0"+iStr;
    }  else
     iStr.setNum(cell);
     nameStr="tempCell"+iStr;
     shortNames[index]=nameStr;
  }

  // cell Balancing Switches::
  for (int i=16; i<105; i+=8){
    QString iStr, codeStr, nameStr;
    iStr.setNum(i);
    codeStr="7bb.6107."+iStr;
    int index= codes.indexOf(codeStr);
    if(index<0)
        continue;
    int cell=i/8-1;
    if(cell<10){
      iStr.setNum(cell);
      iStr="0"+iStr;
    }  else
      iStr.setNum(cell);
    nameStr="balanceSwitch"+iStr;
    shortNames[index]=nameStr;
  }

  inFile.close();

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
  // Eliminate comma to allow next search:
  line_[valueEndPos]=' ';
  //now last comma is just before the value:
  int valueStartPos=line_.lastIndexOf(',')+1;
  sValue=line_.mid(valueStartPos,valueEndPos-valueStartPos);

  ret.setNum(timeS,'f',3);
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
