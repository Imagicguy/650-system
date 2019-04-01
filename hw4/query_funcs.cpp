#include "query_funcs.h"
using namespace pqxx;
void add_player(connection *C, int team_id, int jersey_num, string first_name,
                string last_name, int mpg, int ppg, int rpg, int apg,
                double spg, double bpg) {
  nontransaction addQuery(*C);
  string player_add =
      "INSERT INTO PLAYER (TEAM_ID,UNIFORM_NUM,FIRST_NAME,"
      "LAST_NAME,MPG,PPG,RPG,APG,SPG,BPG) VALUES (" +
      to_string(team_id) + ", " + to_string(jersey_num) + ", " +
      addQuery.quote(first_name) + ", " + addQuery.quote(last_name) + ", " +
      to_string(mpg) + ", " + to_string(ppg) + ", " + to_string(rpg) + ", " +
      to_string(apg) + ", " + to_string(spg) + ", " + to_string(bpg) + ");";
  cout << player_add << endl;
  addQuery.exec(player_add);
  addQuery.commit();
}

void add_team(connection *C, string name, int state_id, int color_id, int wins,
              int losses) {
  nontransaction addQuery(*C);
  string team_add = "INSERT INTO TEAM (NAME,STATE_ID,COLOR_ID,WINS,LOSSES) "
                    "VALUES (" +
                    addQuery.quote(name) + ", " + to_string(state_id) + ", " +
                    to_string(color_id) + ", " + to_string(wins) + ", " +
                    to_string(losses) + ");";
  addQuery.exec(team_add);
  addQuery.commit();
}

void add_state(connection *C, string name) {
  nontransaction addQuery(*C);
  string state_add = "INSERT INTO STATE (NAME) "
                     "VALUES (" +
                     addQuery.quote(name) + ");";
  addQuery.exec(state_add);
  addQuery.commit();
}

void add_color(connection *C, string name) {
  nontransaction addQuery(*C);
  string color_add = "INSERT INTO COLOR (NAME) "
                     "VALUES (" +
                     addQuery.quote(name) + ");";
  addQuery.exec(color_add);
  addQuery.commit();
}

void query1(connection *C, int use_mpg, int min_mpg, int max_mpg, int use_ppg,
            int min_ppg, int max_ppg, int use_rpg, int min_rpg, int max_rpg,
            int use_apg, int min_apg, int max_apg, int use_spg, double min_spg,
            double max_spg, int use_bpg, double min_bpg, double max_bpg) {
  work query1(*C);
  string sql = "SELECT * FROM PLAYER ";
  if (use_mpg || use_apg || use_bpg || use_ppg || use_rpg || use_spg) {
    sql += "WHERE ";
  }
  bool isFirst = true;
  if (use_mpg) {
    isFirst = false;
    sql += "(MPG >= " + to_string(min_mpg) +
           " AND MPG <= " + to_string(max_mpg) + ") ";
  }
  if (use_ppg) {
    if (!isFirst) {
      sql += "AND (PPG >= " + to_string(min_ppg) +
             " AND PPG <= " + to_string(max_ppg) + ") ";
    } else {
      sql += "(PPG >= " + to_string(min_ppg) +
             " AND PPG <= " + to_string(max_ppg) + ") ";
    }
    isFirst = false;
  }
  if (use_rpg) {
    if (!isFirst) {
      sql += "AND (RPG >= " + to_string(min_rpg) +
             " AND RPG <= " + to_string(max_rpg) + ") ";
    } else {
      sql += "(RPG >= " + to_string(min_rpg) +
             " AND RPG <= " + to_string(max_rpg) + ") ";
    }
    isFirst = false;
  }
  if (use_apg) {
    if (!isFirst) {
      sql += "AND (APG >= " + to_string(min_apg) +
             " AND APG <= " + to_string(max_apg) + ") ";
    } else {
      sql += "(APG >= " + to_string(min_apg) +
             " AND APG <= " + to_string(max_apg) + ") ";
    }
    isFirst = false;
  }
  if (use_spg) {
    if (!isFirst) {
      sql += "AND (SPG >= " + to_string(min_spg) +
             " AND SPG <= " + to_string(max_spg) + ") ";
    } else {
      sql += "(SPG >= " + to_string(min_spg) +
             " AND SPG <= " + to_string(max_spg) + ") ";
    }
    isFirst = false;
  }
  if (use_bpg) {
    if (!isFirst) {
      sql += "AND (BPG >= " + to_string(min_bpg) +
             " AND BPG <= " + to_string(max_bpg) + ")";
    } else {
      sql += "(BPG >= " + to_string(min_bpg) +
             " AND BPG <= " + to_string(max_bpg) + ")";
    }
  }

  sql += ";";

  result read(query1.exec(sql));
  cout << "PLAYER_ID TEAM_ID UNIFORM_NUM FIRST_NAME LAST_NAME MPG PPG RPG APG "
          "SPG BPG"
       << endl;

  for (result::const_iterator it = read.begin(); it != read.end(); ++it) {
    cout << it[0].as<int>() << " " << it[1].as<int>() << " " << it[2].as<int>()
         << " " << it[3].as<string>() << " " << it[4].as<string>() << " "
         << it[5].as<int>() << " " << it[6].as<int>() << " " << it[7].as<int>()
         << " " << it[8].as<int>() << " " << it[9].as<double>() << " "
         << it[10].as<double>() << endl;
  }
}

void query2(connection *C, string team_color) {
  work query2(*C);
  string sql = "SELECT TEAM.NAME FROM TEAM, COLOR "
               "WHERE TEAM.COLOR_ID=COLOR.COLOR_ID AND COLOR.NAME=" +
               query2.quote(team_color) + ";";

  result read(query2.exec(sql));

  cout << "NAME" << endl;

  for (result::const_iterator it = read.begin(); it != read.end(); ++it) {
    cout << it[0].as<string>() << endl;
  }
}

void query3(connection *C, string team_name) {
  work query3(*C);

  string sql = "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME FROM PLAYER, TEAM "
               "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.NAME = " +
               query3.quote(team_name) + "ORDER BY PPG DESC;";
  result r(query3.exec(sql));

  cout << "FIRST_NAME LAST_NAME" << endl;

  for (result::const_iterator it = r.begin(); it != r.end(); ++it) {
    cout << it[0].as<string>() << " " << it[1].as<string>() << endl;
  }
}

void query4(connection *C, string team_state, string team_color) {
  work query4(*C);

  string sql = "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME, PLAYER.UNIFORM_NUM "
               "FROM PLAYER, TEAM, "
               "STATE, COLOR WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND "
               "TEAM.STATE_ID = STATE.STATE_ID AND "
               "TEAM.COLOR_ID = COLOR.COLOR_ID AND COLOR.NAME = " +
               query4.quote(team_color) +
               " AND "
               "STATE.NAME = " +
               query4.quote(team_state) + ";";
  result r(query4.exec(sql));

  cout << "FIRST_NAME LAST_NAME UNIFORM_NUM" << endl;

  for (result::const_iterator it = r.begin(); it != r.end(); ++it) {
    cout << it[0].as<string>() << " " << it[1].as<string>() << " "
         << it[2].as<int>() << endl;
  }
}

void query5(connection *C, int num_wins) {
  work query5(*C);

  string sql = "SELECT PLAYER.FIRST_NAME, PLAYER.LAST_NAME, TEAM.NAME, "
               "TEAM.WINS FROM PLAYER, TEAM "
               "WHERE PLAYER.TEAM_ID = TEAM.TEAM_ID AND TEAM.WINS > " +
               to_string(num_wins) + ";";

  result r(query5.exec(sql));

  cout << "FIRST_NAME LAST_NAME NAME WINS" << endl;

  for (result::const_iterator it = r.begin(); it != r.end(); ++it) {
    cout << it[0].as<string>() << " " << it[1].as<string>() << " "
         << it[2].as<string>() << " " << it[3].as<int>() << endl;
  }
}
