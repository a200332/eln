// BackgroundVC.cpp

#include "BackgroundVC.H"
#include <QDebug>
#include <QProcess>
#include <QTimer>

#include "DataFile.H"
#include "DFBlocker.H"
#include "Assert.H"

#ifdef Q_OS_LINUX
#include <sys/types.h>
#include <signal.h>
#endif

BackgroundVC::BackgroundVC(QObject *parent): QObject(parent) {
  vc = 0;
  guard = 0;
  maxt_s = 300;
  block = 0;
  step = -1;
}
  
BackgroundVC::~BackgroundVC() {
  if (vc) {
    qDebug() << "CAUTION! BackgroundVC deleted while still running!";
  }
}

void BackgroundVC::setTimeout(int s) {
  maxt_s = s;
}

bool BackgroundVC::commit(QString path1, QString program1) {
  if (program1!="bzr" && program1!="git") {
    qDebug() << "BackgroundVC can only do bzr and git";
    return false;
  }
  if (vc) {
    qDebug() << "BackgroundVC can only do one task at once";
    return false;
  }
  path = path1;
  program = program1;

  if (!guard) {
    guard = new QTimer(this);
    connect(guard, SIGNAL(timeout()), SLOT(timeout()));
  }
  
  guard->setSingleShot(true);
  guard->setInterval(maxt_s*1000);
  guard->start();

  block = new DFBlocker(this);

  vc = new QProcess(this);
  step = 0;
  vc->setWorkingDirectory(path);
  connect(vc, SIGNAL(finished(int, QProcess::ExitStatus)),
          SLOT(processFinished()));
  connect(vc, SIGNAL(readyReadStandardError()),
          SLOT(processStderr()));
  connect(vc, SIGNAL(readyReadStandardOutput()),
          SLOT(processStdout()));
  if (program=="bzr") 
    vc->start("bzr", QStringList() << "add");
 else if (program=="git")
   vc->start("git", QStringList() << "add" << "-A");
 else
   qDebug() << "BackgroundVC: WHATVC!?!?" << program;
  vc->closeWriteChannel();
  qDebug() << "BackgroundVC: started vc add";
  return true;
}

void BackgroundVC::processStderr() {
  if (vc)
    qDebug() << "BackgroundVC: (stderr) "
             << QString(vc->readAllStandardError());
}

void BackgroundVC::processStdout() {
  if (vc)
    qDebug() << "BackgroundVC: (stdout) "
             << QString(vc->readAllStandardOutput());
}

void BackgroundVC::timeout() {
  if (!vc)
    return;

  qDebug() << "BackgroundVC: timeout";

#ifdef Q_OS_LINUX
  ::kill(vc->pid(), SIGINT);
  // Killing vc with INT produces cleaner exit than with TERM...
#else
  vc->kill();
  // ... but if we don't have POSIX, we have no choice.
#endif

  cleanup(false);
}

void BackgroundVC::cleanup(bool ok) {
  ASSERT(guard);
  ASSERT(block);
  ASSERT(vc);
  guard->stop();
  block->deleteLater();
  block = 0;
  vc->deleteLater();
  vc = 0;
  qDebug() << "BackgroundVC: done " << ok;
  emit(done(ok));
}

void BackgroundVC::processFinished() {
  if (!vc)
    return;

  if (vc->exitCode()) {
    qDebug() << "BackgroundVC: process exited with code " << vc->exitCode();
    cleanup(false);
    return;
  } else if (vc->exitStatus()!=QProcess::NormalExit) {
    qDebug() << "BackgroundVC: process exited with abnormal status "
             << vc->exitStatus();
    cleanup(false);
    return;
  }

  // so we're OK
  if (step==0) {
    // "add" step completed; let's commit (same for bzr and git)
    step = 1;
    vc->start(program, QStringList() << "commit" << "-mautocommit");
    vc->closeWriteChannel();
    qDebug() << "BackgroundVC: started vc commit";
  } else if (step==1 && program=="git") {
    step = 2;
    vc->start(program, QStringList() << "push");
    vc->closeWriteChannel();
    qDebug() << "BackgroundVC: started vc push";
  } else {
    // final step completed. hurray!
    cleanup(true);
  }
}

bool BackgroundVC::isBusy() const {
  return vc!=0;
}
