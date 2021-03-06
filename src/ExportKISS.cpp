/*
Copyright (C) 2000,2001 Stefan Duffner

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <iostream>
#include <qregexp.h>

#include "ExportKISS.h"
#include "Machine.h"
#include "TransitionInfo.h"
//#include "AppInfo.h"
#include "IOInfo.h"

// using namespace std;

ExportKISS::ExportKISS(Options *opt) : Export(opt) {}

/// Writes all the relevant data into the tdf file.
void ExportKISS::doExport() {
  writeHeader("#", "");
  writeName();
  writeIO();
  //  writeVariables();
  writeMain();
}

QString ExportKISS::fileFilter() { return "KISS (*.kiss)"; }
QString ExportKISS::defaultExtension() { return "kiss"; }

/*
/// Writes the header (some comments) into the file.
void ExportKISS::writeHeader()
{
  // Write some comment
  QWidget* wm = qApp->mainWidget();
  AppInfo ai(wm);

  *out << "% This AHDL file was generated by	%" << endl;
  *out << "% Qfsm Version " << ai.getVersionMajor() << "."
       << ai.getVersionMinor() << "			%" << endl;
  *out << "% (C) " << ai.getAuthor() << "			%" << endl <<
endl;
}
*/

/// Writes '#' followed by the machine name to the output stream
void ExportKISS::writeName() {
  using namespace std;

  QString n;
  //  int found, count=0;

  n = machine->getName();

  /*
  found = n.find(" ", count);
  while (found!=-1)
  {
    n[found]='_';
    count=found+1;
    found = n.find(" ", count);
  }

  *out << "SUBDESIGN " << n << endl;
  */
  *out << "# " << n.latin1() << endl << endl;
}

/// Writes the inputs/outputs to the output stream
void ExportKISS::writeIO() {
  using namespace std;

  QString on;
  *out << ".i " << machine->getNumInputs() << endl;
  *out << ".o " << machine->getNumOutputs() << endl;
  *out << ".ilb " << machine->getMealyInputNames(" ").latin1() << endl;

  on = machine->getMealyOutputNames(" ");
  if (!on.isEmpty())
    *out << ".ob " << on.latin1() << endl;

  *out << endl;
}

/*
void ExportKISS::writeVariables()
{
  QString sn, c;
  GState* s;
  bool first=true;

  *out << "VARIABLE" << endl;
  *out << "\tfsm\t:\tMACHINE OF BITS(" << machine->getMooreOutputNames() << ")"
<< endl; *out << "\t\t\t\tWITH STATES (";

  QListIterator<GState> i(machine->getSList());

  // Write initial state
  while (i.current() && !i.current()->isDeleted() &&
i.current()!=machine->getInitialState())
    ++i;
  if (i.current() && !i.current()->isDeleted() &&
i.current()==machine->getInitialState())
  {
    s = i.current();

    sn = s->getName();
    sn.replace(QRegExp(" "), "_");
    c = s->getCodeStr(Binary);

    if (!first)
      *out << ",";
    *out << endl << "\t\t\t\t\t";
    *out << sn << " = B\"" << c << "\"";
    first=false;
  }
  i.toFirst();

  // Write other states
  for(; i.current(); ++i)
  {
    s = i.current();
    if (!s->isDeleted() && s != machine->getInitialState())
    {
      sn = s->getName();
      sn.replace(QRegExp(" "), "_");
      c = s->getCodeStr(Binary);

      if (!first)
        *out << ",";
      *out << endl << "\t\t\t\t\t";
      *out << sn << " = B\"" << c << "\"";
      first=false;
    }
  }
  *out << ");" << endl << endl;
}
*/

/// Writes the reset state and the transitions to the output stream
void ExportKISS::writeMain() {
  using namespace std;

  GState *reset = NULL;

  reset = machine->getInitialState();
  if (reset)
    *out << ".r " << reset->getStateName().latin1() << endl << endl;

  writeTransitions();
}

/// Writes the transitions to the output stream
void ExportKISS::writeTransitions() {
  using namespace std;

  GState *s;
  GTransition *t;
  QString tinfoi, tinfoo, sn1, sn2;
  State *stmp;
  TransitionInfo *tinfo;
  IOInfo *iosingle;
  IOInfo *tioinfo;

  QMutableListIterator<GState *> is(machine->getSList());

  while (is.hasNext()) {
    s = is.next();
    if (s->isDeleted())
      continue;

    sn1 = s->getStateName();
    sn1.replace(QRegExp(" "), "_");

    QMutableListIterator<GTransition *> it(s->tlist);

    while (it.hasNext()) {
      t = it.next();
      tinfo = t->getInfo();
      tioinfo = tinfo->getInputInfo();

      if (!t->isDeleted() && t->getEnd()) {
        //   	QList<IOInfo> iolist;
        QList<IOInfo *> iolist;
        iolist = tioinfo->getSinglesInversion();
        //   	iolist.setAutoDelete(true);

        //  	QListIterator<IOInfo> ioit(iolist);
        QListIterator<IOInfo *> ioit(iolist);

        while (ioit.hasNext()) {
          iosingle = ioit.next();
          tinfoi = iosingle->convertToBinStr();
          stmp = t->getEnd();
          tinfoo = tinfo->getOutputsStrBin();

          if (!tinfoi.isEmpty() && stmp) {
            int slen = tinfoi.length();
            int numin = machine->getNumInputs();
            for (int k = slen; k < numin; k++)
              *out << "0";

            tinfoi.replace(QRegExp("x"), "-");
            *out << tinfoi.latin1() << " "; // << "\" ";

            sn2 = stmp->getStateName();
            sn2.replace(QRegExp(" "), "_");

            *out << sn1.latin1() << " " << sn2.latin1() << " ";

            slen = tinfoo.length();
            int numout = machine->getNumOutputs();
            for (int l = slen; l < numout; l++)
              *out << "0";

            *out << tinfoo.latin1() << endl;
          }
        }
      }
    }
  }
}
