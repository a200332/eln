// Book/Index.h - This file is part of eln

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

// Index.H

#ifndef INDEX_H

#define INDEX_H

#include <QObject>
#include <QMap>

class Index: public QObject {
  Q_OBJECT;
public:
  Index(QString rootDir, class TOC *toc, QObject *parent);
  virtual ~Index();
  void watchEntry(class EntryData *);
  void unwatchEntry(class EntryData *);
  void deleteEntry(class EntryData *);
  void flush();
  class WordIndex *words() const;
private:
  bool flush(int pgno);
private:
  class WordIndex *widx;
  QMap<int, class WordSet *> pgs;
  QString rootdir;
};

#endif
