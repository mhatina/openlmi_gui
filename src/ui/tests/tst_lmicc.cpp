/* ***** BEGIN LICENSE BLOCK *****
 *   Copyright (C) 2013-2014, Dominika Hoďovská <dominika.hodovska@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or
 *   modify it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2.1 of
 *   the License, or (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 * ***** END LICENSE BLOCK ***** */

/*
 * notes:
 * discover button won't be tested
 */

/* TODO
 * try to run app as root
 * check systems presence after running/debugging tests
 * destrukcia potomkov!!
 * dal by sa urobit z testsuits jeden include? (jeden priecinok)
 * remove (delete passwd, remove from tree) system if exist example: remove 127.0.0.1, then add 127.0.0.1
 * po zadani nespravneho hesla daj remove password a klikni znovu na system.. mala by sa vykonat autenizacia
 * maybe use electric fence?
 * check layout
 * sdd system, don't authentificate (press cancel), try authenficitation again
 * beginning of test - is pegasus running? what happens when it is not active
 * otestovat vsetky close buttony
 */

/*
 * BUGS:
 * adding user of same name
 * try to have two systems active, add string to account in filter and switch to account
 *      tab of another system
 *
 * numbers in domain name??
 * in options turned off automatic refreshing, reload plugins in file menu doesn't work
 */

//beh testov na pozadi
//systemctl list-units | grep service
//
#include "tst_lmicc.h"
#include "kernel.h"

int prepareConfFiles() {
    std::string pathComp = QDir::homePath().toStdString() + "/.config/lmicc_computers.xml";
    std::string tmpPathComp = QDir::homePath().toStdString() + "/.config/lmicc_computers_tmp.xml";
    std::string pathSet = QDir::homePath().toStdString() + "/.config/lmicc_settings.xml";
    std::string tmpPathSet = QDir::homePath().toStdString() + "/.config/lmicc_settings_tmp.xml";

    QFile confSet(pathSet.c_str());
    QFile tmpSet(tmpPathSet.c_str());
    QFile confComp(pathComp.c_str());
    QFile tmpComp(tmpPathComp.c_str());
    tmpComp.remove();
    tmpSet.remove();

    if (confSet.exists() && !confSet.copy(tmpPathSet.c_str()) ) {
        std::cerr << "Error while copying file " << pathSet << " to " << tmpPathSet << ": \n";
        std::cerr << strerror(errno) << std::endl;

        return -1;
    }

    if (confComp.exists() && !confComp.copy(tmpPathComp.c_str()) ) {
        std::cerr << "Error while copying file " << pathComp << " to " << tmpPathComp << ": \n";
        std::cerr << strerror(errno) << std::endl;

        return -1;
    }

    confComp.remove();
    confSet.remove();

    return 0;
}

void cleanConfFiles() {
    std::string pathComp = QDir::homePath().toStdString() + "/.config/lmicc_computers.xml";
    std::string tmpPathComp = QDir::homePath().toStdString() + "/.config/lmicc_computers_tmp.xml";
    std::string pathSet = QDir::homePath().toStdString() + "/.config/lmicc_settings.xml";
    std::string tmpPathSet = QDir::homePath().toStdString() + "/.config/lmicc_settings_tmp.xml";

    QFile confSet(pathSet.c_str());
    QFile tmpSet(tmpPathSet.c_str());
    QFile confComp(pathComp.c_str());
    QFile tmpComp(tmpPathComp.c_str());

    confComp.remove();
    confSet.remove();

    if (!confSet.exists() && !tmpSet.copy(pathSet.c_str())) {
        std::cerr << "Warning: Error while copying file " <<  tmpPathSet << " to " << pathSet << ": \n";
        std::cerr << strerror(errno) << std::endl;
    }

    if (!confComp.exists() && !tmpComp.copy(pathComp.c_str())) {
        std::cerr << "Warning: Error while copying file " <<  tmpPathComp << " to " << pathComp << ": \n";
        std::cerr << strerror(errno) << std::endl;
    }

    tmpComp.remove();
    tmpSet.remove();
}

//not ready - add info about crash
void signalHandler (int signum) {
    std::cerr << "Program crashed: received signal: " << strsignal(signum) << std::endl;
    system("secret-tool clear server localhost user root");
    cleanConfFiles();
    exit(signum);
}

//ulozi sa po prvom spusteni config file aj ked nie su pridane ziadne systemy? - ano
int Tst_LMICC::main_for_tests ()
{
    signal(SIGABRT, signalHandler);
    signal(SIGPIPE, signalHandler);
    signal(SIGSEGV, signalHandler);

    AbstractTestSuite* tst;
    int result = 0;
    QStringList testCmd;
    testCmd<<" "<<"-v1";

    if (prepareConfFiles() == -1) {
        cleanConfFiles();
        return -1;
    }

    //reload options set
//    tst = new StartupTestSuite();
//    result += QTest::qExec(tst, testCmd);
//    delete tst;

    //testOptionsAction
//    tst = new MenuBarTestSuite();
//    result += QTest::qExec(tst, testCmd);
//    delete tst;

    //runs normally
//    tst = new ToolbarTestSuite();
//    result += QTest::qExec(tst, testCmd);
//    delete tst;

    //ready?
//    tst = new PCTreeWidgetTestSuite();
//    result += QTest::qExec(tst, testCmd);
//    delete tst;

//    tst = new AccountTestSuite();
//    result += QTest::qExec(tst, testCmd);
//    delete tst;

    //no reload
//    tst = new HardwarePluginTestSuite();
//    result += QTest::qExec(tst, testCmd);
//    delete tst;

    cleanConfFiles();
    return result;
}
