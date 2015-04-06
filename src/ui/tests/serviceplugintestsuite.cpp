#include "serviceplugintestsuite.h"
#include "../../plugin/service/actionbox.h"

ServicePluginTestSuite::ServicePluginTestSuite() :
    AbstractPluginTestSuite(SERVICE)
{
}

void ServicePluginTestSuite::prepareServicePlugin() {
    preparePlugin("Service");
}

//services from templates not tested (@.service)

void ServicePluginTestSuite::testServiceList() {

    QSKIP("Ready", SkipAll);
    prepareServicePlugin();

    QTableWidget* servicesTable = kernel->widget<QTableWidget*>("services_table");
    QVERIFY2(servicesTable, "Failed to get services_table widget");

    std::vector<std::string>* services = getLocalServices();
    std::vector<std::string>* lmiServices = new std::vector<std::string>();

    int k = 0;
    for(int j = 0; j < servicesTable->rowCount(); j++) {
        std::stringstream buff("");
        for(int i = 0; i < servicesTable->columnCount()-1; i++) {
            if(servicesTable->item(j,i)) {
                if (i == 0 && servicesTable->item(j,i)->text().toStdString().find('@') != std::string::npos) {
                    break;
                }
                buff << servicesTable->item(j,i)->text().toStdString();
                if (i != servicesTable->columnCount()-2)
                    buff << " ";
            }
        }
        if (buff.str() == "") {
            continue;
        }
        if (services->size() <= (unsigned)j) {
            break;
        }
        if (buff.str() != services->at(k)) {
            std::cerr << "Next services are different:\n"
                      << "OpenLmiCC - " << buff.str() << "(" << j << ") AND\n"
                      << "systemctl - " << services->at(k) << "(" << k << ")\n";
            std::cerr.flush();
            QFAIL("");
        }
        lmiServices->push_back(buff.str());
        k++;

    }

    delete services;
    services = NULL;
    delete lmiServices;
    lmiServices = NULL;

}

std::vector<std::string>* ServicePluginTestSuite::getLocalServices() {

    using namespace std;
    vector<string>* services = new vector<string>();

    string str("rm -rf service_status.out");
    system(str.c_str());

    str = "for s in `systemctl list-unit-files | "
                    "grep \\.service | grep -v \\@\\.service |"
                    "awk {'print $1'} | sort`; "
                    "do "
                    "echo '~' $s >> service_status.out;"
                    "systemctl status $s >> service_status.out; done";
    system(str.c_str());

    FILE* loc_serv;
    loc_serv = fopen("service_status.out", "r");
    if (loc_serv == NULL) {
        qDebug("Failed to open service_status.out for reading.");
        return NULL;
    }

    int c;
    int i = 0;
    while(1) {
        string temp;
        while ((c = fgetc(loc_serv)) != '~') {
            if (c == -1) {
                sort(services->begin(), services->end(), compareByFirstWord);
                return services;
            }
        }
        char buff[200];

        //name
        fgetc(loc_serv);
        fgets(buff, 200, loc_serv);
        buff[strlen(buff)-1] = '\0';
        temp.append(buff);

        for(int j = 0; j < 5; j++, fgetc(loc_serv));

        //name passing
        fscanf(loc_serv, "%s ", buff);

        //FL trash passing
        for(int j = 0; j < 2; j++, fgetc(loc_serv));

        //description
        fgets(buff, 200, loc_serv);
        buff[strlen(buff)-1] = '\0';
        temp.append(" ");
        temp.append(buff);

        //bordel passing
        fgets(buff, 200, loc_serv);

        fscanf(loc_serv, "%s ", buff);
        while (strcmp(buff,"Active:"))
            fscanf(loc_serv, "%s ", buff);
        //status
        fscanf(loc_serv, "%s ", buff);

        services->push_back(temp);

        statusTranslation(services,buff);
        i++;
    }
}

void ServicePluginTestSuite::statusTranslation(std::vector<std::string> *services, char status[]) {
    if (strcmp(status, "active") == 0) {
        services->rbegin()->append(" {OK}");
        services->rbegin()->append(" OK");
    } else if (strcmp(status, "inactive") == 0) {
        services->rbegin()->append(" {Completed, OK}");
        services->rbegin()->append(" Stopped");
    } else if (strcmp(status, "failed") == 0) {
        services->rbegin()->append(" {Completed, Error}");
        services->rbegin()->append(" Stopped");
    } else if (strcmp(status, "activating") == 0) {
        services->rbegin()->append(" {Starting}");
        services->rbegin()->append(" Stopped");
    } else if (strcmp(status, "deactivating") == 0) {
        services->rbegin()->append(" {Stopping}");
        services->rbegin()->append(" OK");
    }
}

void ServicePluginTestSuite::testColumnNames()
{
    QSKIP("Ready", SkipAll);
    QFETCH(int, column_index);
    QFETCH(QString, column_name);

    QTableWidget *servicesTable = kernel->widget<QTableWidget *>("services_table");
    QVERIFY2(servicesTable, "Failed to get user_table widget");

    QString name = servicesTable->horizontalHeaderItem(column_index)->text();
    QVERIFY(name == column_name);
}

void ServicePluginTestSuite::testColumnNames_data()
{
    using namespace QTest;
    addColumn<int>("column_index");
    addColumn<QString>("column_name");

    newRow("Name")                  << 0 << "Name";
    newRow("Caption")               << 1 << "Caption";
    newRow("Operational status")    << 2 << "Operational status";
    newRow("Status")                << 3 << "Status";
    newRow("Action")                << 4 << "Action";
}

/*
void ServicePluginTestSuite::testActionBox() {
    prepareServicePlugin();

    QComboBox* actionBox = kernel->widget<QComboBox*>("action_box");
    QVERIFY2(actionBox,"Failed to get action_box.");

    QList<QAction*> actionsList = actionBox->actions();

    QVERIFY(actionsList.at(0) == "-");
}
*/

void ServicePluginTestSuite::testReload() {
    prepareServicePlugin();

    QTableWidget *servicesTable = kernel->widget<QTableWidget *>("services_table");
    QVERIFY2(servicesTable, "Failed to get user_table widget");

    ActionBox* actionBox = (ActionBox*)servicesTable->cellWidget(0,4);
    QVERIFY2(actionBox,"Failed to get action_box.");

//    QTest::mouseClick(actionBox,Qt::LeftButton);
//    actionBox->changeAction("Reload");
//    QTest::qWait(2000);
}
