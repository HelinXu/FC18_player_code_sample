#include "ai.h"

#include <iostream>
#include <vector>

#include "definition.h"
#include "user_toolbox.h"

// struct Memo saves some essencial parameters. You may need memo when judging
// if the numbers of your towers/corps/commands have reached their limits. Note
// that you may as well neglect this version of Memo, for it is not yet robust.
struct Memo {
  int myID = -1;
  int myTowerCount = 0;
  int myBattleCount = 0;
  int myConstructCount = 0;
  int commandCount = 0;
};

// You may want to declare some functions here.
Memo MemoInitial(Info& info) {
  Memo memo;
  memo.myID = info.myID;
  for (int i = 0; i < info.corpsInfo.size(); ++i) {
    if (info.corpsInfo[i].owner == memo.myID && info.corpsInfo[i].exist) {
      if (info.corpsInfo[i].type == Battle) {
        memo.myBattleCount++;
      } else {
        memo.myConstructCount++;
      }
    }
  }
  for (int i = 0; i < info.towerInfo.size(); ++i) {
    if (info.towerInfo[i].ownerID == memo.myID && info.towerInfo[i].exist) {
      memo.myTowerCount++;
    }
  }
  return memo;
}

bool isMyTower(TowerInfo& tower, int myID) {
  return tower.ownerID == myID && tower.exist;
}

bool isValidTerritory(int x, int y) {
  return x >= 0 && x <= 14 && y >= 0 && y <= 14;
}

bool isMyTerritory(int x, int y, Info& info) {
  if (!isValidTerritory(x, y)) return false;
  return ((*info.gameMapInfo)[y][x].owner == info.myID);
}

// get all the corps located at cell (x, y) and push them in vector<CorpsInfo>
// ret.
std::vector<CorpsInfo> GetCorp(int x, int y, Info& info) {
  vector<CorpsInfo> ret;
  if (!isValidTerritory(x, y)) return ret;
  for (int i = 0; i < (*info.gameMapInfo)[y][x].corps.size(); ++i) {
    ret.push_back(info.corpsInfo[(*info.gameMapInfo)[y][x].corps.at(i)]);
  }
  return ret;
}

// get the tower located at (x, y) and returns tower.exist = false when no tower
// exists at (x, y)
TowerInfo GetTower(int x, int y, Info& info) {
  TowerInfo tower;
  tower.exist = false;
  if (!isValidTerritory(x, y)) return tower;
  if ((*info.gameMapInfo)[y][x].TowerIndex == NOTOWER) return tower;
  return info.towerInfo[(*info.gameMapInfo)[y][x].TowerIndex];
}

bool CanBuildTowerAt(int x, int y, Info& info, Memo& memo) {
  if (memo.myTowerCount >= 10 || memo.commandCount >= 50) {
    return false;
  }  // num. of towers/ commands reached their limits.
  if (!isMyTerritory(x, y, info)) {
    return false;  // can not build towers on others' terri.
  }
  if (GetTower(x, y, info).exist) {
    return false;
  }  // towers do not overlap.
  return true;
}

bool isMyExtender(CorpsInfo& corp, Info& info) {
  return corp.owner == info.myID && corp.exist && corp.type == Construct &&
         corp.m_BuildType == Extender;
}

bool isMyBuilder(CorpsInfo& corp, Info& info) {
  return corp.owner == info.myID && corp.exist && corp.type == Construct &&
         corp.m_BuildType == Builder;
}

TCorpsID FindBattlerTarget(CorpsInfo& corp, Info& info) {
  // TODO
  return -1;
}

void player_ai(Info& info) {
  Memo memo = MemoInitial(info);

  // Deal with my extenders first.
  for (int i = 0; i < info.corpsInfo.size(); ++i) {
    // First and formost, extenders builds new towers whenever they can to avoid
    // possibility of being captured.
    if (isMyExtender(info.corpsInfo[i], info)) {
      int x = info.corpsInfo[i].pos.m_x;
      int y = info.corpsInfo[i].pos.m_y;
      if (CanBuildTowerAt(x, y, info, memo)) {
        info.myCommandList.addCommand(
            corpsCommand,
            {CBuild,
             info.corpsInfo[i].ID});  // build a tower in place if possible.
        memo.myTowerCount++;
        memo.myConstructCount--;
        memo.commandCount += 1;  // update memo.
      } else if (CanBuildTowerAt(x + 1, y, info,
                                 memo)) {  // else, find a neighboring cell to
                                           // build a tower.
        info.myCommandList.addCommand(corpsCommand,
                                      {CMove, info.corpsInfo[i].ID, CRight});
        info.myCommandList.addCommand(corpsCommand,
                                      {CBuild, info.corpsInfo[i].ID});
        memo.myTowerCount++;
        memo.myConstructCount--;
        memo.commandCount += 2;
      } else if (CanBuildTowerAt(x - 1, y, info, memo)) {
        info.myCommandList.addCommand(corpsCommand,
                                      {CMove, info.corpsInfo[i].ID, CLeft});
        info.myCommandList.addCommand(corpsCommand,
                                      {CBuild, info.corpsInfo[i].ID});
        memo.myTowerCount++;
        memo.myConstructCount--;
        memo.commandCount += 2;
      } else if (CanBuildTowerAt(x, y + 1, info, memo)) {
        info.myCommandList.addCommand(corpsCommand,
                                      {CMove, info.corpsInfo[i].ID, CDown});
        info.myCommandList.addCommand(corpsCommand,
                                      {CBuild, info.corpsInfo[i].ID});
        memo.myTowerCount++;
        memo.myConstructCount--;
        memo.commandCount += 2;
      } else if (CanBuildTowerAt(x, y - 1, info, memo)) {
        info.myCommandList.addCommand(corpsCommand,
                                      {CMove, info.corpsInfo[i].ID, CUp});
        info.myCommandList.addCommand(corpsCommand,
                                      {CBuild, info.corpsInfo[i].ID});
        memo.myTowerCount++;
        memo.myConstructCount--;
        memo.commandCount += 2;
      }
      // TODO: There is no point waiting there if none of the cases are
      // satisfied. You may want to find a smarter stragety.
    }
  }

  // Then, deal with my towers.
  TowerInfo tower;
  for (int x = 0; x < 15; ++x) {
    for (int y = 0; y < 15; ++y) {  // traverse all cells.
      tower = GetTower(x, y, info);
      if (isMyTower(tower, memo.myID)) {
        // there are cells available for building new towers around:
        if (CanBuildTowerAt(x + 1, y, info, memo) ||
            CanBuildTowerAt(x, y + 1, info, memo) ||
            CanBuildTowerAt(x - 1, y, info, memo) ||
            CanBuildTowerAt(x, y - 1, info, memo) ||
            info.totalRounds % 7 == 0) {  // info.totalRounds % 7 is for chances
                                          // the terri. is against me.
          // produce new extenders intended for building new towers there.
          info.myCommandList.addCommand(towerCommand,
                                        {TProduct, tower.ID, PExtender});
          memo.commandCount++;
          // memo.myConstructCount++;
          // TODO: error updating Memo.
        }
        // In other cases, update myself.
        else if (tower.level < 8) {
          info.myCommandList.addCommand(towerCommand,
                                        {TProduct, tower.ID, PUpgrade});
          memo.commandCount++;
        } else {
          info.myCommandList.addCommand(towerCommand,
                                        {TProduct, tower.ID, PWarrior});
          // Do not want to waste production-point. Let it
          // build anything instead of doing nothing. Of course, this stragety
          // is not smart enough. You may want to make some fundemental changes
          // for better results.
          memo.commandCount++;
        }
      }
    }
  }
  // TODO: what should the warriors do?
}