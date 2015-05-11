// App/main.cpp - This file is part of eln

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

// main.C

#include "PageEditor.h"
#include "SceneBank.h"
#include <QApplication>
#include <QFile>
#include <QDir>
#include "Notebook.h"
#include "RecentBooks.h"
#include "App.h"
#include "Fonts.h"
#include <QDesktopWidget>
#include <QDebug>
#include <stdlib.h>
#include "Assert.h"
#include "SplashScene.h"
#include "AlreadyOpen.h"
#include "DefaultSize.h"

int main(int argc, char **argv) {
  App app(argc, argv);
  eln_grabsignals();
  app.setWindowIcon(QIcon(":/eln.png"));
  Fonts fonts;
  
  Notebook *nb = 0;
  if (argc==1) {
    nb = SplashScene::openNotebook();
    if (!nb)
      return 0;
  } else if (argc==2) {
    if (AlreadyOpen::check(argv[1]))
      return 0;
    nb = Notebook::load(argv[1]);
  } else if (argc==3 && argv[1]==QString("-new")) {
    if (QDir(argv[2]).exists()) {
      qDebug() << "eln: Cannot create new notebook '" << argv[2]
	       << "': found existing notebook";
      return 1;
    }
    nb = Notebook::create(argv[2]);
  } else {
    qDebug() << "Usage: eln";
    qDebug() << "Usage: eln notebook";
    qDebug() << "Usage: eln -new notebook";
    return 1;
  }
  if (!nb) {
    qDebug() << "Notebook not found";
    return 1;
  }

  QObject::connect(&app, SIGNAL(aboutToQuit()), nb, SLOT(commitNow()));

  SceneBank *bank = new SceneBank(nb);
  PageEditor *editor = new PageEditor(bank);

  editor->resize(DefaultSize::onScreenSize(editor->sizeHint()));
  new AlreadyOpen(nb->dirPath(), editor);
  editor->show();
  int r = app.exec();
  delete bank;
  delete nb;
  delete RecentBooks::instance();
  return r;
}
