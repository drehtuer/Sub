#include "Eval.h"
#include <iostream>
#include <algorithm>
#include <ctime>
#include <cstring>
#include <boost/filesystem.hpp>
#include <QtXml>
#include <QtCore>
#include <QTimer>
#include <QtGui>

using namespace Ui;

#define SCALEIMAGE 0
#define QUESTIONAIREWIDGET 0
#define IMAGEWIDGET 1
#define CHOICEWIDGET 2
#define BLANKWIDGET 3

Eval::Eval(QWidget *Parent)
	: QMainWindow(Parent),
      m_ComparisonIndex(0),
      m_ComparisonId(0),
      m_inputBlocked(true),
      m_status(0)
    {
	// setup
	m_Mutex = new QMutex();
	m_RNGState.seed(std::time(NULL));
	setupUi(this);
	setWindowState(Qt::WindowFullScreen);

	// load config
	if(!loadConfig("../../config/Eval.xml")) {
		std::cerr<<"config not loaded successfully, exiting ..."<<std::endl;
		exit(-1);
	}

	m_BlockTimer = new QTimer(this);
	m_BlockTimer->setSingleShot(true);
	m_BlockTimer->setInterval(m_blockInputTime);
	connect(m_BlockTimer, SIGNAL(timeout()), this, SLOT(unlockKeyLock()));

	m_BlankTimer = new QTimer(this);
	m_BlankTimer->setSingleShot(true);
	m_BlankTimer->setInterval(m_blankTime);
	connect(m_BlankTimer, SIGNAL(timeout()), this, SLOT(displayNext()));

	connect(this, SIGNAL(changeStack(int)), StackedWidget, SLOT(setCurrentIndex(int)));
	connect(this, SIGNAL(changeImage()), this, SLOT(displayNext()));

	emit changeStack(QUESTIONAIREWIDGET);
}

Eval::~Eval() {
	delete m_Mutex;
}

bool Eval::loadConfig(const std::string &Filename) {
	QDomDocument Doc("config");
	QFile File(Filename.c_str());
	if(!File.open(QIODevice::ReadOnly)) {
		std::cerr<<"Could not open config file"<<std::endl;
		QMessageBox::critical(this, "Read error", "Could not open config file");
		return false;
	}
	if(!Doc.setContent(&File)) {
		File.close();
		std::cerr<<"Could not parse config file"<<std::endl;
		QMessageBox::critical(this, "Read error", "Could not parse config file");
		return false;
	}
	File.close();
	
	QDomElement Root = Doc.documentElement();

	m_blankTime = Root.elementsByTagName("BlankTime").item(0).toElement().text().toUInt();
    m_blockInputTime = Root.elementsByTagName("BlockInput").item(0).toElement().text().toUInt();
    m_parallelComparison = Root.elementsByTagName("ParallelComparison").item(0).toElement().text().toInt();
	m_width =  Root.elementsByTagName("Window").item(0).toElement().elementsByTagName("Width").item(0).toElement().text().toUInt();
	m_height = Root.elementsByTagName("Window").item(0).toElement().elementsByTagName("Height").item(0).toElement().text().toUInt();
	m_fullscreen = Root.elementsByTagName("Window").item(0).toElement().elementsByTagName("Fullscreen").item(0).toElement().text().toUInt();
	
	m_ResultsFolder = Root.elementsByTagName("Results").item(0).toElement().elementsByTagName("Folder").item(0).toElement().text().toStdString();

    // comparisons
	// get data for initPairs();
	bool cb = Root.elementsByTagName("UseCounterbalancing").item(0).toElement().text().toInt();
	// ComparisonGroups
	QDomNodeList CGroups = Root.elementsByTagName("ComparisonGroups");
	for(uint cg=0; cg<CGroups.length(); ++cg) {
		// group
		QDomNodeList Group = CGroups.at(cg).toElement().elementsByTagName("Group");
		for(uint g=0; g<Group.length(); ++g) {
			std::string GroupName = Group.at(g).toElement().attribute("name").toStdString();
			//std::cout<<"Group '"<<GroupName<<"'"<<std::endl;
			ImageGroup IG;
			IG.Name = GroupName;
            // folder
			QDomNodeList Folder = Group.at(g).toElement().elementsByTagName("Folder");
			for(uint f=0; f<Folder.length(); ++f) {
                uint type = Folder.at(f).toElement().attribute("type").toUInt();
				std::string FolderName = Folder.at(f).toElement().text().toStdString();
				//std::cout<<"\t"<<type<<": "<<FolderName<<std::endl;

                if(IG.Directories.size() < type+1)
                    IG.Directories.resize(type+1);
				IG.Directories[type].push_back(FolderName);
			}
			m_Groups.push_back(IG);
		}		
	}

	initSets(cb);

	return true;
}

void Eval::completeQuestionaire() {
	if(AgeEdit->text() == "")
		return;
	if(!MaleRadioButton->isChecked() && !FemaleRadioButton->isChecked())
		return;
	if(!ConsumerRadioButton->isChecked() && !BasicRadioButton->isChecked() && !AdvancedRadioButton->isChecked() && !ProfessionalRadioButton->isChecked())
		return;

	Question Quest;
	Quest.Q = AgeLabel->text().toStdString();
	Quest.Answer = AgeEdit->text().toStdString();
	m_Questionaire.Questions.push_back(Quest);

	Quest.Q = SexLabel->text().toStdString();
	if(MaleRadioButton->isChecked())
		Quest.Answer = "m";
	if(FemaleRadioButton->isChecked())
		Quest.Answer = "f";
	m_Questionaire.Questions.push_back(Quest);

	Quest.Q = ExperienceLabel->text().toStdString();
	if(ConsumerRadioButton->isChecked())
		Quest.Answer = "consumer";
	if(BasicRadioButton->isChecked())
		Quest.Answer = "basic";
	if(AdvancedRadioButton->isChecked())
		Quest.Answer = "advanced";
	if(ProfessionalRadioButton->isChecked())
		Quest.Answer = "professional";
	m_Questionaire.Questions.push_back(Quest);

	blankDisplay();
}

void Eval::choiceFirstImage() {
	m_Compares[m_ComparisonIndex-1].choice = 0;
	blankDisplay();
}

void Eval::choiceSecondImage() {
	m_Compares[m_ComparisonIndex-1].choice = 1;
	blankDisplay();
}

void Eval::displayImage() {
	setKeyLock(true);
	if(m_ComparisonIndex >= m_Compares.size()) {
		if(saveResults())
			close();
		return;
	}

	m_BlockTimer->start();

	QImage Img;
	if(!Img.load(m_Compares[m_ComparisonIndex].Images[m_ComparisonId].Filename.c_str())) {
		QMessageBox::critical(this, tr("Image not found"), tr(("Image '"+m_Compares[m_ComparisonIndex].Images[m_ComparisonId].Filename+"' not found!").c_str()));
	}
    #if SCALEIMAGE
	    Img = Img.scaled(width(), height()-50, Qt::KeepAspectRatio);
    #endif
	ImageLabel->setPixmap(QPixmap::fromImage(Img));
	
	switch(m_ComparisonId) {
    case 0:
        if(m_parallelComparison)
            m_ComparisonIndex++; // next comparison
        else
            m_ComparisonId = 1; // second part
        break;

    case 1:
        m_ComparisonId = 0; // first part
        m_ComparisonIndex++; // next comparison
        break;

    default:
        std::cerr<<"Unknown comparisonId: "<<m_ComparisonId<<std::endl;
        break;
    }

	emit changeStack(IMAGEWIDGET);

	// wait for user to press the space bar
}

void Eval::blankDisplay() {
	setKeyLock(true);
	emit changeStack(BLANKWIDGET);
	m_BlankTimer->start();
}

void Eval::displayNext() {
    switch(m_status) {
	// first image
    case 0:
        m_status = 1;
        displayImage();
        break;

	// second image
    case 1:
        m_status = 2;
        displayImage();
        break;

	// choice
    case 2:
        m_status = 0;
		ButtonFirst->clearFocus();
		ButtonSecond->clearFocus();
		emit changeStack(CHOICEWIDGET);
        break;

    default:
        std::cerr<<"Unknwon status"<<std::endl;
        break;
    }
}

void Eval::initSets(const bool useCounterbalancing) {
	// load image filenames
    for(size_t g=0; g<m_Groups.size(); ++g) { // group
        for(size_t t=0; t<m_Groups[g].Directories.size(); ++t) { // type (0 is reference)
            for(size_t d=0; d<m_Groups[g].Directories[t].size(); ++d) { // folder
                boost::filesystem::path dir(m_Groups[g].Directories[t][d]); // directory

                try {
                    if(boost::filesystem::exists(dir)) { // directory exists
                        std::vector<boost::filesystem::path> files;
						
						// read directory content
                        std::copy(boost::filesystem::directory_iterator(dir), boost::filesystem::directory_iterator(), std::back_inserter(files));

                        ImgCompare IC;
                        //IC.Images.resize(files.size());
                        for(size_t f=0; f<files.size(); ++f) {
                            if(boost::filesystem::is_regular_file(files[f])) {
								if(files[f].extension() == ".png") {
									Img I;
									I.Filename = files[f].string();
									IC.Images.push_back(I);
									//std::cout << "\t\tFound file '"<<files[f]<<"'"<<std::endl;
								}
                            }
                        }
                        m_Groups[g].Compares.push_back(IC);
                    }
                } catch(const boost::filesystem::filesystem_error &E) {
                    std::cerr<<E.what()<<std::endl;
                }
            }
        }
    }

	// prepare RNG
	RNG rand(m_RNGState);

    // building pairs
	for(size_t g=0; g<m_Groups.size(); ++g) { // group
		for(size_t c=1; c<m_Groups[g].Compares.size(); ++c) { // compares
			for(size_t p=0; p<m_Groups[g].Compares[c].Images.size(); ++p) {  // compares, first is reference
				ImgCompare IC;
				IC.Images.resize(2);
				IC.Images[0].Filename = m_Groups[g].Compares[0].Images[0].Filename;
				IC.Images[1].Filename = m_Groups[g].Compares[c].Images[p].Filename;
				
				// counterbalancing -> duplicate & switch
				if(useCounterbalancing) {
					m_Compares.push_back(IC);
					std::swap(IC.Images[0].Filename, IC.Images[1].Filename);
					m_Compares.push_back(IC);
				} else { // random switch
					if(rand(10) < 5)
						std::swap(IC.Images[0].Filename, IC.Images[1].Filename);
					m_Compares.push_back(IC);
				}
			}
		}
	}
	//std::cout<<"Number of compares: "<<m_Compares.size()<<std::endl;

	// shuffle image pairs
	std::random_shuffle(m_Compares.begin(), m_Compares.end(), rand);
	std::random_shuffle(m_Compares.begin(), m_Compares.end(), rand); // because i can
	std::random_shuffle(m_Compares.begin(), m_Compares.end(), rand);
}

void Eval::keyPressEvent(QKeyEvent *E) {
    switch(E->key()) {
	case Qt::Key_Space:
        E->accept();
        if(!getKeyLockStatus()) { // can we proceed?
            setKeyLock(true);
            blankDisplay();      
        }
		break;

	// easier than alt-f4, no one needs to know about this
	//case Qt::Key_Escape:
 //       E->accept();
 //       close();
	//	break;

	default:
		break;
	}
}

void Eval::mouseReleaseEvent(QMouseEvent *E) {
	QKeyEvent *KE = NULL;
	switch(E->button()) {
		case Qt::LeftButton:
			if(!getKeyLockStatus()) {
				E->accept();
				KE = new QKeyEvent(QEvent::KeyPress, Qt::Key_Space, Qt::NoModifier);
				QApplication::postEvent(this, KE);
			}
			break;

		default:
			break;
	}
}

void Eval::unlockKeyLock() {
    setKeyLock(false);
}

bool Eval::saveResults() {
	QDomDocument Doc("Results");
	QDomElement Root = Doc.createElement("Results");
	Doc.appendChild(Root);
	QDomElement IdElement = Doc.createElement("Id");
	Root.appendChild(IdElement);

    // questionaire
    QDomElement Questionaire = Doc.createElement("Questionaire");
    Root.appendChild(Questionaire);
    for(size_t q=0; q<m_Questionaire.Questions.size(); ++q) {
        QDomElement Answer = Doc.createElement("Answer");
        Answer.setAttribute("id", q);
        Answer.appendChild(Doc.createTextNode(m_Questionaire.Questions[q].Answer.c_str()));
        Questionaire.appendChild(Answer);
    }

    // comparisons
	for(size_t c=0; c<m_Compares.size(); ++c) {
		QDomElement Comparison = Doc.createElement("Comparison");
		Comparison.setAttribute("order", num2str(c).c_str());
		
		QDomElement FirstImage = Doc.createElement("First");
		FirstImage.appendChild(Doc.createTextNode(m_Compares[c].Images[0].Filename.c_str()));
		QDomElement SecondImage = Doc.createElement("Second");
		SecondImage.appendChild(Doc.createTextNode(m_Compares[c].Images[1].Filename.c_str()));
        QDomElement Choice = Doc.createElement("Choice");
        Choice.appendChild(Doc.createTextNode(num2str(m_Compares[c].choice).c_str()));

		Comparison.appendChild(FirstImage);
		Comparison.appendChild(SecondImage);
        Comparison.appendChild(Choice);
		
		Root.appendChild(Comparison);
	}

	// save
	std::string Filename, key;
	bool isNewFile = true;
	do {
		key = generateRandomStringSequence(4);
		Filename = m_ResultsFolder + "/" + key + ".xml";
		boost::filesystem::path f(Filename);
		if(boost::filesystem::exists(f))
			isNewFile = false;
	} while(!isNewFile);

	// save unique key
	IdElement.setAttribute("key", num2str(key).c_str());
	

	QFile File(Filename.c_str());
	if(!File.open(QIODevice::WriteOnly)) {
		QMessageBox::critical(this, "Write error", "Could not save results!");
		std::cerr<<"Could not open config file"<<std::endl;
		return false;
	}
	QTextStream OutStream(&File);
	Doc.save(OutStream, 0);
	File.close();

	std::cout << "Thank you for participating in this user study."<<std::endl;
  	return true;
}

std::string Eval::generateRandomStringSequence(const uint length) {
	char *sequence = new char[length+1];
	std::memset(sequence, 0, (length+1)*sizeof(char));
	static const char validChars[] = "0123456789abcdefghijklmnopqrstuvwxyz";
	uint tableSize = sizeof(validChars)/sizeof(char) - 1; // don't count terminator

	RNG rand(m_RNGState);
	for(uint i=0; i<length; ++i) {
		sequence[i] = validChars[rand(tableSize)];
	}
	sequence[length] = 0;
	std::string s = std::string(sequence);
	delete [] sequence;
	return s;
}

bool Eval::getKeyLockStatus() const {
	bool result;
	m_Mutex->lock();
    result = m_inputBlocked;
	m_Mutex->unlock();
	return result;
}

void Eval::setKeyLock(const bool status) {
	m_Mutex->lock();
    m_inputBlocked = status;
	m_Mutex->unlock();
}