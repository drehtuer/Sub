#ifndef EVAL_EVAL_H
#define EVAL_EVAL_H

#include <string>
#include <sstream>
#include <vector>
#include <iomanip>
#include <boost/random.hpp>
#include "ui_MainWindow.h"
#include <QMutex>

#define DLLE __declspec(dllexport)
#define NOMINMAX
#undef min
#undef max

namespace Ui {

	template <class Type> std::string num2str(const Type t, const int precision = 12) {
		std::stringstream ws;
		ws << std::setprecision(precision) << t;
		return ws.str();
	};

	struct DLLE Img {
		Img() : Filename(""), shown(0) {};
		std::string Filename;
		uint shown;
	};

	struct DLLE ImgCompare {
		ImgCompare() : shown(0), choice(-1) {};
		std::vector<Img> Images;
		uint shown;
        int choice;
	};

	struct DLLE ImageGroup {
		ImageGroup() : shown(0) {};
		std::string Name;
		std::vector<ImgCompare> Compares;
		std::vector<std::vector<std::string> > Directories;
		uint shown;
	};

    struct DLLE Question {
        std::string Q;
        std::vector<std::string> A; // if A is empty, the answer is freestyle
        std::string Answer;
    };

    struct DLLE Questionaire {
        std::vector<Question> Questions;
    };


    // taken from http://stackoverflow.com/questions/147391/using-boostrandom-as-the-rng-for-stdrandom-shuffle
	struct DLLE RNG : std::unary_function<unsigned , unsigned> {
	public:
		RNG(boost::mt19937 &MT) : m_MersenneTwister(MT) {};
		unsigned operator()(unsigned i) {
			boost::uniform_int<> rng(0, i-1);
			return rng(m_MersenneTwister);
		}
	private:
		boost::mt19937 &m_MersenneTwister;
	};




	class DLLE Eval : public QMainWindow, private MainWindow {
		Q_OBJECT
	public:
		Eval(QWidget *Parent = NULL);
		~Eval();

	signals:
		void changeStack(int index);
		void changeImage();

	protected:
		void keyPressEvent(QKeyEvent *E);
		void mouseReleaseEvent(QMouseEvent *E);

	protected slots:
		void unlockKeyLock();
        void choiceFirstImage();
		void choiceSecondImage();
		void completeQuestionaire();
        void displayNext();

	private:
		bool loadConfig(const std::string &Filename);
		void initSets(const bool useCounterbalancing);
		void displayImage();
        void blankDisplay();
		bool saveResults();
		std::string generateRandomStringSequence(const uint length);
        bool getKeyLockStatus() const;
        void setKeyLock(const bool status);

		QTimer *m_BlockTimer, *m_BlankTimer;
		QMutex *m_Mutex;
		boost::mt19937 m_RNGState;
		std::string m_ResultsFolder;
		std::vector<ImageGroup> m_Groups;
        std::vector<ImgCompare> m_Compares;
        std::vector<QLineEdit*> m_Textfields;
        std::vector<QButtonGroup*> m_ButtonGroups;
        Questionaire m_Questionaire;
		size_t m_ComparisonIndex, m_ComparisonId;
		uint m_width, m_height, m_blankTime, m_blockInputTime;
        uint m_status;
        bool m_parallelComparison, m_fullscreen, m_inputBlocked;
	};
}

#endif
