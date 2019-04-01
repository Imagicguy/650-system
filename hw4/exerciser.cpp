#include "exerciser.h"

void exercise(connection *C) {

  // Incorrect input check
  add_player(C, 2, 3, "Zion", "Willison", 35, 27, 9, 4, 2, 2);
  add_team(C, "Evil'Evil;", 1, 3, 4, 1);
  add_state(C, "dfsde'df");
  add_color(C, "sdfsefsdf'sdfase'aadadfa");
  add_player(C, 1, 2, "sdf'd", "dfsdfse'sdfwefdfsd", 2.2, 0.3, 221, 2.3, 2, 2);
  // add test
  for (int i = 8; i < 88; i++) {
    add_team(C, "Team" + to_string(i), i, i, i, 100 - i);
    add_state(C, "State" + to_string(i));
    add_color(C, "Color" + to_string(i));
  }
  // Queries check
  query1(C, 1, 24, 100, 0, 20, 100, 0, 10, 0, 2, 0, 13, 0, 12, 0, 1, 1, 3);
  query2(C, "LightBlue");
  query2(C, "Orange");
  query3(C, "Duke");
  query3(C, "NCSU");
  query3(C, "BUAAISBEST!");
  query4(C, "NC", "Orange");
  query4(C, "NC", "LightBlue");
  query5(C, 21);
  query5(C, 17);
}
