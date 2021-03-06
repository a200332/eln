// Catalog.cpp

#include "Catalog.h"
#include <QDir>
#include <QFileInfo>
#include <QRegExp>

Catalog::Catalog(QString pgdir): pgdir(pgdir) {
  ok = false;
  QDir pages(pgdir);
  if (!pages.exists())
    return;

  QRegExp re1("^(\\d\\d*)-([a-z0-9]+).json");
  QRegExp re0("^(\\d\\d*).json");
  QRegExp ren("^(\\d\\d*)-([a-z0-9]+).notes");
  
  foreach (QFileInfo const &fi, pages.entryInfoList()) {
    if (fi.isFile()) {
      QString fn = fi.fileName();
      if (fn.endsWith(".moved") || fn.endsWith(".THIS")
          || fn.endsWith(".OTHER") || fn.endsWith(".BASE")) 
        err << "Presence of " + fn + " indicates unsuccessful bzr update.";
      if (!fn.endsWith(".json"))
        continue;
      if (re1.exactMatch(fn)) {
        int n = re1.cap(1).toInt();
        pg2file.insert(n, fn);
        filemods[fn] = fi.lastModified();
      } else if (re0.exactMatch(fn)) {
        int n = re0.cap(1).toInt();
        pg2file.insert(n, fn);
        filemods[fn] = fi.lastModified();
      } else {
        err << "Cannot parse " + fn + " as a page file name.";
      }
    } else if (fi.isDir()) {
      QString fn = fi.fileName();
      if (fn.endsWith(".moved") || fn.endsWith(".THIS")
          || fn.endsWith(".OTHER") || fn.endsWith(".BASE")) 
        err << "Presence of " + fn + " indicates unsuccessful bzr update.";
      if (ren.exactMatch(fn)) {
        notemods[fn] = fi.lastModified();
      }
    }
  }  
}

bool Catalog::isClean() const {
  if (!err.isEmpty())
    return false;
  for (int k: pg2file.keys())
    if (pg2file.count(k)!=1)
      return false;
  return true;
}

QDateTime Catalog::fileMod(QString fn) const {
  return filemods.contains(fn) ? filemods[fn] : QDateTime();
}

bool Catalog::hasNotes(QString fn) const {
  return notemods.contains(fn);
}

QDateTime Catalog::noteDirMod(QString fn) const {
  return notemods.contains(fn) ? notemods[fn] : QDateTime();
}
