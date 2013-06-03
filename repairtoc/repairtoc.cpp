// repairtoc/repairtoc.cpp - This file is part of eln

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

// repairtoc.C

#include <QFile>
#include <QDir>
#include <QDebug>
#include <QDateTime>

#include "JSONFile.H"

#include <stdio.h>
#include <stdlib.h>

void usage() {
  fprintf(stderr, "Usage: repairtoc -v [notebookdir]\n");
  exit(1);
}

struct BasicTOCEntry {
  QString filename;
  QString title;
  int startPage;
  int sheetCount;
  QDateTime created;
  QDateTime modified;
  bool operator<(BasicTOCEntry const &b) const {
    if (created<b.created)
      return true;
    else if (created>b.created)
      return false;

    if (startPage<b.startPage)
      return true;
    else if (startPage>b.startPage)
      return false;
    
    if (modified<b.modified)
      return true;
    else if (modified>b.modified)
      return false;

    if (filename<b.filename)
      return true;
    else if (filename>b.filename)
      return false;

    if (title<b.title)
      return true;
    else if (title>b.title)
      return false;

    if (sheetCount<b.sheetCount)
      return true;
    else if (sheetCount>b.sheetCount)
      return false;

    return false; // truly equal
  }
};

QString findANotebook() {
  QDir d(QDir::current());
  QStringList flt; flt.append("*.nb");
  QStringList dd = d.entryList(flt, QDir::Dirs, QDir::Name);
  if (dd.isEmpty()) {
    qDebug() << "eln: No notebook found in" << d.absolutePath();
    exit(1);
  } else if (dd.size()>1) {
    qDebug() << "eln: Multiple notebooks found in" << d.absolutePath();
    exit(1);
  }
  qDebug() << "Loading notebook" << dd[0];
  return dd[0];
}

int main(int argc, char **argv) {
  bool verbose = false;
  if (argc>=2 && QString(argv[1]) == "-v") {
    verbose = true;
    argc--;
    argv++;
  }
  if (argc>2)
    usage();
  QDir root(argc==2 ? argv[1] : findANotebook());
  QDir pages(root.filePath("pages"));

  bool originalReadable;
  QVariantMap originalTOCFile = JSONFile::load(root.filePath("toc.json"),
					       &originalReadable);
  QMap<BasicTOCEntry, int> originalTOC;
  if (originalReadable && originalTOCFile.contains("cc")) {
    if (verbose)
      qDebug() << "Original TOC file parsed";
    QVariantList originalEntries = originalTOCFile["cc"].toList();
    foreach (QVariant e0, originalEntries) {
      BasicTOCEntry bte;
      QVariantMap e = e0.toMap();
      bte.startPage = e["startPage"].toInt();
      bte.sheetCount = e["sheetCount"].toInt();
      bte.created = e["cre"].toDateTime();
      bte.modified = e["mod"].toDateTime();
      bte.title = e["title"].toString();
      bte.filename = QString("%1.json").arg(bte.startPage);
      if (pages.exists(bte.filename)) {
	if (verbose)
	  qDebug() << "Original TOC entry for " << bte.filename << " read";
	originalTOC[bte] = 1;
      } else {
	qDebug() << "Original TOC entry for " << bte.filename
		 << " does not correspond to an existing file. Ignored.";
      }
    }
  } else {
    originalReadable = false;
  }

  QList<BasicTOCEntry> actualTOC;
  QStringList nameflt; nameflt << "*.json" << "*.json.*";
  QStringList pagefilenames = pages.entryList(nameflt,
					      QDir::Files | QDir::Readable);
  QStringList renameList; // these files will have to be moved
  foreach (QString fn, pagefilenames) {
    if (fn.endsWith("~"))
      continue;
    bool pageReadable;
    QVariantMap page = JSONFile::load(pages.filePath(fn),
				      &pageReadable);
    if (pageReadable) {
      if (verbose)
	qDebug() << "Parsed page file " << fn;
      BasicTOCEntry bte;
      bte.filename = fn;
      bool startPageOK;
      bte.startPage = page["startPage"].toInt(&startPageOK);
      bte.sheetCount = 0;
      bte.created = page["cre"].toDateTime();
      bte.modified = page["mod"].toDateTime();
      foreach (QVariant c0, page["cc"].toList()) {
	QVariantMap c = c0.toMap();
	if (c["typ"].toString()=="title") {
	  foreach (QVariant c1, c["cc"].toList()) {
	    QVariantMap c = c1.toMap();
	    if (c["typ"].toString()=="text")
	      bte.title = c["text"].toString();
	  }
	} else if (c.contains("sheet")) {
	  int sheet = c["sheet"].toInt();
	  if (sheet>=bte.sheetCount)
	    bte.sheetCount = sheet+1;
	}
      }
      if (startPageOK) {
	if (verbose)
	  qDebug() << "File " << fn
		   << " is titled " << bte.title
		   << ", starts at page " << bte.startPage 
		   << ", and contains " << bte.sheetCount << " sheets.";
	actualTOC.append(bte);
      } else {
	qDebug() << "File " << fn
		 << " does not mention a start page. Ignored";
      }
    } else {
      if (fn.endsWith(".json")) {
	qDebug() << "File " << fn
		 << " cannot be parsed as a page file. Will be renamed.";
	renameList << fn;
      } else {
	qDebug() << "File " << fn
		 << " cannot be parsed as a page file. Ignored.";
      }
    }
  }

  if (!originalReadable)
    qDebug() << "Original TOC file could not be parsed. Ignored.";

  qDebug() << "";

  /* Now we have an "actualTOC" and perhaps an "originalTOC".
     Let's sort the actualTOC.
  */
  qSort(actualTOC);

  // Let's renumber if needed
  int page = 1;
  bool renumbered = false;
  bool perfectmatch = originalReadable;
  for (QList<BasicTOCEntry>::iterator i = actualTOC.begin();
       i!=actualTOC.end(); ++i) {
    BasicTOCEntry &bte(*i);
    if (originalTOC.contains(bte)) {
      originalTOC.remove(bte);
    } else if (originalReadable) {
      perfectmatch = false;
      qDebug() << "No entry in original TOC for " << bte.filename;
    }
    if (bte.startPage!=page) {
      qDebug() << "Renumbering entry " << bte.filename
	       << " to start on page " << page;
      bte.startPage = page;
      renumbered = true;
    }
    page += bte.sheetCount;
  }
  
  if (!originalTOC.isEmpty()) {
    perfectmatch = false;
    foreach (BasicTOCEntry const &bte, originalTOC.keys()) 
      qDebug() << "No file for original TOC entry " << bte.filename;
  }

  if (perfectmatch && !renumbered) {
    qDebug() << "The original TOC matches the directory structure.";
    qDebug() << "No action required.";
    return 0;
  }

  QList<BasicTOCEntry> toBeRenumbered;
  foreach (BasicTOCEntry const &bte, actualTOC) 
    if (bte.filename != QString("%1.json").arg(bte.startPage))
      toBeRenumbered.append(bte);

  qDebug() << "";
  
  qDebug() << "The TOC file will be rebuilt.";
  if (!toBeRenumbered.isEmpty())
    qDebug() << "In addition, " << toBeRenumbered.size()
	     << "file(s) will be renumbered.";
  if (!renameList.isEmpty())
    qDebug() << "In addition, " << renameList.size()
	     << "unparsable file(s) will be renamed.";
    

  qDebug() << "Press Enter to proceed";

  char buffer[1000];
  if (!fgets(buffer, 999, stdin)) {
    qDebug() << "Not confirmed. Terminating without action.";
    return 1;
  }

  foreach (QString fn, renameList) {
    QString nw = fn + ".unparsed~";
    if (pages.rename(fn, nw)) {
      qDebug() << "Renamed " << fn << " as " << nw;
    } else {
      qDebug() << "Unfortunately, " << fn << " could not be renamed as " << nw;
      qDebug() << "Aborting.";
      return 2;
    }      
  }
  

  qDebug() << "Rebuilding TOC file";
  QVariantMap toc;
  toc["typ"] = "toc";
  toc["cre"] = QDateTime::currentDateTime();
  toc["mod"] = QDateTime::currentDateTime();
  QVariantList cc;
  foreach (BasicTOCEntry const &bte, actualTOC) {
    QVariantMap e;
    e["typ"] = "entry";
    e["cre"] = bte.created;
    e["mod"] = bte.modified;
    e["startPage"] = bte.startPage;
    e["sheetCount"] = bte.sheetCount;
    e["title"] = bte.title;
    cc.append(e);
  }
  toc["cc"] = cc;
  if (!JSONFile::save(toc, root.filePath("toc.json"))) {
    qDebug() << "Failed to save new TOC file.";
    root.remove("toc.json");
    return 2;
  }
  qDebug() << "New TOC file saved OK.";

  // Construct a map of new pages organized by original filename
  QMap<QString, QVariantMap> renumberedPages;
   
  // Read all renumberable pages
  foreach (BasicTOCEntry const &bte, toBeRenumbered) {
    bool pageReadable;
    QVariantMap page = JSONFile::load(pages.filePath(bte.filename),
				      &pageReadable);
    if (!pageReadable) {
      qDebug() << "Unfortunately, " << bte.filename << " could not be parsed.";
      qDebug() << "Aborting.";
      return 2;
    }
    page["startPage"] = bte.startPage;
    renumberedPages[bte.filename] = page;
  }

  // Make sure no poorly named ".res" directories exist
  foreach (BasicTOCEntry const &bte, toBeRenumbered) {
    QString res = bte.filename;
    res.replace("json", "res");
    if (pages.exists(res + ".tmp")) {
      qDebug() << "Existence of " << (res+".tmp")
	       << "indicates a problem that repairtoc cannot fix.";
      qDebug() << "Aborting.";
      return 2;
    }
  }

  // Rename all affected original files
  foreach (BasicTOCEntry const &bte, toBeRenumbered) {
    pages.remove(bte.filename + "~");
    if (!pages.rename(bte.filename, bte.filename + "~")) {
      // This should be replaced with "bzr move" if there is a
      // "vc": "bzr" key in style.json.
      qDebug() << "Failed to rename " << bte.filename
	       << " as " << (bte.filename+"~");
      qDebug() << "Aborting.";
      return 2;
    }
    qDebug() << "Renamed " << bte.filename
	     << " as " << (bte.filename+"~");
    QString res = bte.filename;
    res.replace("json", "res");
    if (pages.exists(res)) {
      if (!pages.rename(res, res+".tmp")) {
        // This should be replaced with "bzr move" if there is a
        // "vc": "bzr" key in style.json.
	qDebug() << "Failed to rename " << res
		 << " as " << (res+".tmp");
	qDebug() << "Aborting.";
	return 2;
      }
    }	 
  }

  // Save all renumbered files
  foreach (QString origfn, renumberedPages.keys()) {
    QVariantMap const &pg = renumberedPages[origfn];
    int pgno = pg["startPage"].toInt();
    if (pages.exists(QString("%1.json").arg(pgno))) {
      qDebug() << "File " << QString("%1.json").arg(pgno)
	       << " exists. Cannot renumber " << origfn << " as " << pgno;
      qDebug() << "Aborting.";
      return 2;
    }
    if (JSONFile::save(pg, pages.filePath(QString("%1.json").arg(pgno)))) {
      // We should recall which "~" this came from, and bzr move as appropriate.
      qDebug() << "Renumbered " << origfn << " as " << pgno;
    } else {
      qDebug() << "Failed to renumber " << origfn << " as " << pgno;
      qDebug() << "Aborting.";
      return 2;
    }
    QString res = origfn; res.replace("json", "res");
    if (pages.exists(res + ".tmp")) {
      if (pages.exists((QString("%1.res").arg(pgno)))) {
	qDebug() << "Dir " << QString("%1.res").arg(pgno)
		 << " exists. Cannot renumber " << (res+".tmp");
	qDebug() << "Aborting.";
	return 2;
      }
      if (!pages.rename(res + ".tmp", QString("%1.res").arg(pgno))) {
        // This should be replaced with "bzr move" if there is a
        // "vc": "bzr" key in style.json.
	qDebug() << "Failed to rename " << (res+".tmp")
		 << " to " << QString("%1.res").arg(pgno);
	qDebug() << "Aborting.";
	return 2;
      }
      qDebug() << "Renumbered " << res << " as " << pgno;
    }
  } 

  return 0;
}