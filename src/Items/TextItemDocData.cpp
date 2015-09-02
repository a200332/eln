// Items/TextItemDocData.cpp - This file is part of eln

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

// TextItemDocData.cpp

#include "TextItemDocData.h"
#include "MarkupEdges.h"
#include <QDebug>

TextItemDocData::TextItemDocData(TextData *text): text(text) {
  indent = 0;
  width = 1000; // hmm
  leftmargin = rightmargin = 0;
  lineheight = 15; // hmm
  y0 = 4;
  writable = false;
}

void TextItemDocData::setBaseFont(QFont const &f) {
  forgetWidths();
  baseFont = f;
  fv.setBase(f);
  ascent = fv.metrics(MarkupStyles())->ascent();
  xheight = fv.metrics(MarkupStyles())->xHeight();
  descent = fv.metrics(MarkupStyles())->descent();
}

void TextItemDocData::setCharWidths(QVector<double> const &cw) {
  charwidths = cw;
}

QVector<double> const &TextItemDocData::charWidths() const {
  if (charwidths.isEmpty()) 
    recalcSomeWidths(0, -1);
  return charwidths;
}

void TextItemDocData::recalcSomeWidths(int start, int end) const {
  /* Calculates widths for every character in range. */
  /* If we currently don't have _any_ widths, we calculate whole doc. */
  /* Currently does not yet do italics correction, but it will. */
  
  if (charwidths.isEmpty()) {
    start = 0;
    end = -1;
  }
  if (start>0)
    --start;

  MarkupStyles current;
  MarkupEdges edges(text->markups());
  foreach (int k, edges.keys()) 
    if (k<start)
      current = edges[k];
    else
      break;
  
  QFontMetricsF const *fm = fv.metrics(current);
  
  QString txt = text->text();
  int N = txt.size();
  charwidths.resize(N);
  if (end<0)
    end = N;

  for (int n=start; n<end; n++) {
    QChar c = txt[n];
    if (edges.contains(n)) {
      current = edges[n];
      fm = fv.metrics(current);
    }
    if (edges.contains(n+1) || n+1>=N) {
      // simple, no kerning across edges
      charwidths[n] = fm->width(c);
      if (edges.contains(n+1) && current.contains(MarkupData::Italic)
	  && !edges[n+1].contains(MarkupData::Italic))
	charwidths[n] += italicCorrection(current);
    } else {
      QChar d = txt[n+1];
      charwidths[n] = fm->width(QString(c) + QString(d)) - fm->width(d);
    }
  }
}

double TextItemDocData::italicCorrection(MarkupStyles const &sty) const {
  QFontMetricsF const *fm = fv.metrics(sty);
  return fm->width(" ")*0.4;
}
