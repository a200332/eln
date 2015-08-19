// File/VersionControl.cpp - This file is part of eln

/* eln is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   eln is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with eln.  If not, see <http://www.gnu.org/licenses/>.
*/

// VersionControl.C

#include "App.h"
#include "VersionControl.h"
#include <QStringList>
#include <QMessageBox>
#include <QEventLoop>
#include <QProcess>
#include <QDebug>
#include <QDir>
#include <QPushButton>
#include "Process.h"

#ifdef Q_OS_LINUX
#include <sys/types.h>
#include <signal.h>
#endif

#define VC_TIMEOUT 300 // seconds

namespace VersionControl {
bool runVC(QString vccmd, QString subcmd, QStringList args, QString label,
             QString *stdo=0, QString *stde=0) {
  Process proc;
  proc.setWindowCaption(label);
  args.push_front(subcmd);
  proc.setCommandAndArgs(vccmd, args);
  bool ok = proc.exec();
  if (stdo)
    *stdo = proc.stdout();
  if (stde)
    *stde = proc.stderr();

  return ok;
}


bool runBzr(QString cmd, QStringList args, QString label,
              QString *stdo=0, QString *stde=0) {
  return runVC("bzr", cmd, args, label, stdo, stde);
}

bool runGit(QString cmd, QStringList args, QString label,
              QString *stdo=0, QString *stde=0) {
  return runVC("git", cmd, args, label, stdo, stde);
}
  
bool update(QString path, QString program) {
  if (program == "")
    return true;

  QString cwd = QDir::currentPath();
  QString se;

  QDir::setCurrent(path);
  bool success = false;
  if (program == "bzr") 
    success = runBzr("update", QStringList(), "Updating with bzr...", 0, &se);
  else if (program == "git")
    success = runGit("pull", QStringList(), "Updating with git...", 0, &se);
  QDir::setCurrent(cwd);

  if (success)
    return true;
  
  if (se.isEmpty())
    se = "(no message)";

  QMessageBox mb(QMessageBox::Warning, "eln version control",
		 "Update of '" + path + "' failed.", 0);
  QPushButton *editb
    = mb.addButton("Edit anyway", QMessageBox::DestructiveRole);
  QPushButton *rob
    = mb.addButton("Open read-only", QMessageBox::AcceptRole);
  QPushButton *qb = mb.addButton("Quit", QMessageBox::RejectRole);
  mb.setDefaultButton(rob);
  mb.setEscapeButton(qb);
  mb.setDetailedText(se);
  mb.exec();
  QAbstractButton *r = mb.clickedButton();
  if (r==editb) {
    // edit anyway
    return true;
  } else if (r==rob) {
    // open read only
    return false;
  } else {
    exit(1); // immediate and total abort
  }
}

bool commit(QString path, QString program) {
  bool success = false;
  QString cwd = QDir::currentPath();
  QString se;
  QDir::setCurrent(path);
  if (program == "") {
    return true;
  } else if (program == "bzr") {
    /* The logic is:
       (1) we run bzr status
       (2) if that returns any text at all, we first do "add", then "commit".
       (3) errors at any stage cause us to give up immediately
    */
    QString out;
    success = runBzr("status", QStringList(), "Checking bzr status...",
		     &out, &se);
    bool need = !out.isEmpty();
    if (need && success) 
      success = runBzr("add", QStringList(), "Adding files to bzr...", 0, &se);
    if (need && success) {
      QStringList args;
      args << "-meln autocommit";
      success = runBzr("commit", args, "Committing with bzr...", 0, &se);
    }
  } else if (program == "git") {
    /* The logic is:
       (1) we run git status -s
       (2) if that returns any text at all, we first do "add", then "commit",
           and finally "push".
       (3) errors at any stage cause us to give up immediately
     */
    QString out;
    success = runGit("status", QStringList() << "-s",
                     "Checking git status...", &out, &se);
    bool need = !out.isEmpty();
    if (need && success)
      success = runGit("add" , QStringList() << "-A",
                       "Adding files to git...", 0, &se);
    if (need && success)
      success = runGit("commit", QStringList() << "-meln elnautocommit",
                       "Committing locally using git...", 0, &se);
    if (need && success)
      success = runGit("push", QStringList(),
                       "Pushing changes using git...", 0, &se);
  }
  QDir::setCurrent(cwd);
  if (se.isEmpty())
    se = "(no message)";
  if (!success) {
    QMessageBox mb(QMessageBox::Warning, "eln version control",
		   "Commit of '" + path + "' failed.",
		   QMessageBox::Ignore);
    mb.setDetailedText(se);
    mb.exec();
  }
  return success;
}
};
