create_executable( Eval
  FILES Main.cpp Eval.cpp ../include/Eval.h ../config/Eval.xml
  QTMOC Eval.h
  QTUI ../qt/MainWindow.ui
  LINK ${QT_LIBRARIES} )
