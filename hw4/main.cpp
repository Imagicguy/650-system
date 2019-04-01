#include "exerciser.h"
#include <bits/stdc++.h>
#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <pqxx/pqxx>

using namespace std;
using namespace pqxx;

void resetTable(connection *C) {
  if (C->is_open()) {
    work dropTable(*C);
    dropTable.exec("DROP TABLE IF EXISTS PLAYER;");
    dropTable.exec("DROP TABLE IF EXISTS TEAM;");
    dropTable.exec("DROP TABLE IF EXISTS STATE;");
    dropTable.exec("DROP TABLE IF EXISTS COLOR;");
    dropTable.commit();
    cout << "resetTable succeed!" << endl;
  }

  std::string playerTable, teamTable, colorTable, stateTable;
  teamTable = "CREATE TABLE TEAM("
              "TEAM_ID  SERIAL PRIMARY KEY                  NOT NULL,"
              "NAME     TEXT                             NOT NULL,"
              "STATE_ID INT REFERENCES STATE(STATE_ID)   NOT NULL,"
              "COLOR_ID INT REFERENCES COLOR(COLOR_ID)   NOT NULL,"
              "WINS     INT                              NOT NULL,"
              "LOSSES   INT                              NOT NULL);";
  playerTable = "CREATE TABLE PLAYER("
                "PLAYER_ID   SERIAL PRIMARY KEY               NOT NULL,"
                "TEAM_ID     INT REFERENCES TEAM(TEAM_ID)  NOT NULL,"
                "UNIFORM_NUM INT                           NOT NULL,"
                "FIRST_NAME  TEXT                          NOT NULL,"
                "LAST_NAME   TEXT                          NOT NULL,"
                "MPG         INT                           NOT NULL,"
                "PPG         INT                           NOT NULL,"
                "RPG         INT                           NOT NULL,"
                "APG         INT                           NOT NULL,"
                "SPG         DOUBLE PRECISION              NOT NULL,"
                "BPG         DOUBLE PRECISION              NOT NULL);";
  stateTable = "CREATE TABLE STATE("
               "STATE_ID SERIAL PRIMARY KEY                  NOT NULL,"
               "NAME                 TEXT                 NOT NULL);";
  colorTable = "CREATE TABLE COLOR("
               "COLOR_ID SERIAL PRIMARY KEY                  NOT NULL,"
               "NAME                 TEXT                 NOT NULL);";

  work createTable(*C);
  createTable.exec(stateTable);
  createTable.exec(colorTable);
  createTable.exec(teamTable);
  createTable.exec(playerTable);
  createTable.commit();
  cout << "createTable succeed!" << endl;
}
void playerInit(connection *C) {
  ifstream myfile;
  string line;
  work addQuery(*C);
  myfile.open("player.txt");
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      vector<string> playerInfo;
      boost::split(playerInfo, line, boost::is_any_of(" "));
      string player_add = "INSERT INTO PLAYER (TEAM_ID,UNIFORM_NUM,FIRST_NAME,"
                          "LAST_NAME,MPG,PPG,RPG,APG,SPG,BPG) VALUES (" +
                          playerInfo[1] + ", " + playerInfo[2] + ", " +
                          addQuery.quote(playerInfo[3]) + ", " +
                          addQuery.quote(playerInfo[4]) + ", " + playerInfo[5] +
                          ", " + playerInfo[6] + ", " + playerInfo[7] + ", " +
                          playerInfo[8] + ", " + playerInfo[9] + ", " +
                          playerInfo[10] + ");";
      addQuery.exec(player_add);
    }
    addQuery.commit();
    myfile.close();
  }
}

void teamInit(connection *C) {
  ifstream myfile;
  string line;
  work addQuery(*C);
  myfile.open("team.txt");
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      vector<string> teamInfo;
      boost::split(teamInfo, line, boost::is_any_of(" "));
      string team_add = "INSERT INTO TEAM (NAME,STATE_ID,COLOR_ID,WINS,LOSSES) "
                        "VALUES (" +
                        addQuery.quote(teamInfo[1]) + ", " + teamInfo[2] +
                        ", " + teamInfo[3] + ", " + teamInfo[4] + ", " +
                        teamInfo[5] + ");";
      addQuery.exec(team_add);
    }
    addQuery.commit();
    myfile.close();
  }
}

void stateInit(connection *C) {
  ifstream myfile;
  string line;
  work addQuery(*C);
  myfile.open("state.txt");
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      vector<string> stateInfo;
      boost::split(stateInfo, line, boost::is_any_of(" "));
      string state_add = "INSERT INTO STATE (NAME) "
                         "VALUES (" +
                         addQuery.quote(stateInfo[1]) + ");";
      addQuery.exec(state_add);
    }
    addQuery.commit();
    myfile.close();
  }
}

void colorInit(connection *C) {
  ifstream myfile;
  string line;
  work addQuery(*C);
  myfile.open("color.txt");
  if (myfile.is_open()) {
    while (getline(myfile, line)) {
      vector<string> colorInfo;
      boost::split(colorInfo, line, boost::is_any_of(" "));
      string color_add = "INSERT INTO COLOR (NAME) "
                         "VALUES (" +
                         addQuery.quote(colorInfo[1]) + ");";
      addQuery.exec(color_add);
    }
    addQuery.commit();
    myfile.close();
  }
}

int main(int argc, char *argv[]) {

  // Allocate & initialize a Postgres connection object
  connection *C;

  try {
    // Establish a connection to the database
    // Parameters: database name, user name, user password
    C = new connection("dbname=acc_bball user=postgres password=passw0rd");
    if (C->is_open()) {
      cout << "Opened database successfully: " << C->dbname() << endl;
    } else {
      cout << "Can't open database" << endl;
      return 1;
    }
    resetTable(C);
    stateInit(C);

    colorInit(C);

    teamInit(C);

    playerInit(C);

  } catch (const std::exception &e) {
    cerr << e.what() << std::endl;
    return 1;
  }

  // TODO: create PLAYER, TEAM, STATE, and COLOR tables in the ACC_BBALL
  // database
  //      load each table with rows from the provided source txt files

  exercise(C);

  // Close database connection
  C->disconnect();

  return 0;
}
