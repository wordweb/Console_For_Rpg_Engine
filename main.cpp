#include <curl/curl.h>
#include <iostream>
#include <fstream>
#include "windows.h"
#include <conio.h>
#include <sstream>
#include "tools.cpp"
#include "audio_clip.cpp"
#include "actions.cpp"
#include <stack>
#include <vector>
#include <algorithm>

using namespace std;

string config = ".\\game.ini";

void drawWindow(int x, int y, int w, int h) {
	string up = "�x";
	string left = "��";
	string right = "��";
	string down = "��";
	// �ϱ�Ե����
	for (int i = 1; i < w / 2 - 1; i++) {
		gotoxyaa(x + i * 2, y);
		cout << up;
		gotoxyaa(x + i * 2, y + h - 1);
		cout << down;
	}
	for (int i = 1; i < h - 1; i++) {
		gotoxyaa(x, y + i);
		cout << left;
		gotoxyaa(x + w - 2, y + i);
		cout << right;
	}
	// �ڲ����
	for (int i = 1; i < w / 2 - 1; i++) {
		for (int j = 1; j < h - 1; j++) {
			gotoxyaa(x + i * 2, j + y);
			cout << "��";
		}
	}
}

void textOut(int x, int y, string aText, int aWidth, int aWait) {
	int a, b;
	a = 0;
	b = 0;
	for (int i = 0; i < aText.size() / 2; i++) {
		gotoxyaa(x + a, y);
		cout << aText.substr(i * 2, 2);
		Sleep(aWait);
		a += 2;
		if (a > aWidth) {
			b++;
			a = 0;
		}
	}
}


struct MenuItem {
	string text;
	ActionList *code;
};

struct Menu {
	string Name;               // ���ڱ�ʶ��λ��
	MenuItem item[10];
	int itemNum;
	int wx, wy, ww, wh;
	int left, top, bottom;     // ����λ��
	int yy;                    // ��ǰλ��
	World *world;
	string mark = "��";
	// ����ģʽ
	int LockMode;
	// ��Ӳ˵���
	void addItem(string aName, ActionList *onClick) {
		item[itemNum].text = aName;
		item[itemNum].code = onClick;
		itemNum++;
	}
	void clear() {
		itemNum = 0;
	}
	void loadFromFile(string aName, World *wd) {
		world = wd;
		ifstream fin(aName.c_str());
		// cout << aName << endl;
		if (fin) {
			//cout << "have file" << endl;
			itemNum = 0;
			while (!fin.eof()) {
				string str1, str2;
				getline(fin, str1);
				if (str1 == "") {
					continue;
				} else if (str1[0] == '#') {
					// һ���µĲ˵���Ŀ�ʼ
					item[itemNum].text = str1.substr(1);
					//cout << item[itemNum].text << endl;
					// �����Ǵ���
					ActionList *temp = new ActionList();
					readActions(fin, *temp, "#END", wd, 0);

					item[itemNum].code = temp;
					itemNum++;
				}
			}
		}
		fin.close();
		//getch();
	}
	void showDot(bool aShow) {
		setColor(15);
		gotoxyaa(left, yy);
		if (aShow) {
			cout << mark;
		} else {
			cout << "  ";
		}
	}
	void showMenu(int x, int y, int aLockMode) {
		// �����Զ����㴰�ڵĿ��
		int w = 2, h = itemNum;
		for (int i = 0; i < itemNum; i++) {
			if (item[i].text.size() > w) {
				w = item[i].text.size();
			}
		}
		// ���ݿ�ߣ������ڣ�������ߣ���ѡ���
		setColor(15);
		drawWindow(x, y, w + 8, h + 4);
		// �رմ����õ���Ϣ
		wx = x;
		wy = y;
		ww = w + 8;
		wh = h + 4;
		// ��ʾ����
		for (int i = 0; i < itemNum; i++) {
			textOut(x + 4, y + 2 + i, item[i].text, w + 6, 0);
		}
		// ��ʾѡ���
		left = x + 2;
		top = y + 2;
		bottom = y + 2 + itemNum - 1;
		yy = top;
		showDot(1);
		// �˵�ģʽ��
		world->menuOn = true;
		// �Ƿ�������ʽ
		LockMode = aLockMode;
	}
	void hideMenu() {
		world->menuOn = false;
		world->showMapPart(wx / 2, wy, ww / 2 + 2, wh);
	}
};

struct Map {
	string Name;
	string map[100];
	string mapmove[100];
	string mapcolor[50];
	int Map_Height, Map_Width;
	int Map_Color_Num;
	// ��ͼ��������
	AudioClip sound;
	// �¼���Ϣ
	EventList eventList;
	// �����������磬���˽�Ӣ�۵�λ��
	World *world;
	// �����Ǻ���
	int getColor(string aCode) {
		string str1 = "", str_else;
		for (int i = 0; i < Map_Color_Num; i++) {
			int a = mapcolor[i].find("else");
			if (a >= 0) {
				str_else = mapcolor[i];
				break;
			}
		}
		for (int i = 0; i < Map_Color_Num; i++) {
			int a = mapcolor[i].find(aCode);
			if (a >= 0) {
				str1 = mapcolor[i];
				break;
			}
		}
		//cout << found << endl;
		int cc;
		if (str1 != "") {
			int b = str1.find("=");
			string str2 = str1.substr(b + 1, 2);
			cc = str2int(str2);
		} else {
			if (str_else != "") {
				int b = str_else.find("=");
				string str2 = str_else.substr(b + 1, 2);
				cc = str2int(str2);
			} else {
				cc = 15;
			}
		}
		return cc;
	}
	bool canMove(int x, int y) {
		if (mapmove[y][x] == '1') {
			Event *ev = eventList.findEvent(3, x * 2, y);
			if (ev && ev->enabled) {
				return 0;
			}
			ev = eventList.findEvent(5, x * 2, y);
			if (ev && ev->enabled) {
				return 0;
			}
			if (x == world->hero.x / 2 && y == world->hero.y) {
				return 0;
			}
			return 1;
		} else {
			return 0;
		}
	}

	void moveNPC() {
		for (int i = 0; i < eventList.evtNum; i++) {
			if ((eventList.event[i].trigMode == 5 || eventList.event[i].trigMode == 3)
			        && eventList.event[i].enabled) {
				// ����ƶ������ߺ����ƶ�����������
				int a = rand() % 2;
				int b = rand() % 3 - 1;
				// ����NPCֻ���ǵ���
				int x = eventList.event[i].pos[0].x;
				int y = eventList.event[i].pos[0].y;
				int x1, y1;
				if (a == 0) {
					x1 = x + b * 2;
					y1 = y;
				} else {
					x1 = x;
					y1 = y + b;
				}
				if (canMove(x1 / 2, y1)) {
					gotoxyaa(x, y);
					setColor(getColor(map[y].substr(x, 2)));
					cout << map[y].substr(x, 2);
					//
					eventList.event[i].pos[0].x = x1;
					eventList.event[i].pos[0].y = y1;
					//
					gotoxyaa(x1, y1);
					setColor(eventList.event[i].color);
					cout << eventList.event[i].chart;
				}
			}
		}
	}
	void showNPC() {
		for (int i = 0; i < eventList.evtNum; i++) {
			if ((eventList.event[i].trigMode == 5 || eventList.event[i].trigMode == 3)) {
				if (eventList.event[i].enabled) {
					// ��ʾ����
					// ����NPCֻ���ǵ���
					int x = eventList.event[i].pos[0].x;
					int y = eventList.event[i].pos[0].y;
					//
					gotoxyaa(x, y);
					setColor(eventList.event[i].color);
					cout << eventList.event[i].chart;
				} else {
					// ����ʾ
				}
			}
		}
	};
	void hideNPC() {
		for (int i = 0; i < eventList.evtNum; i++) {
			if ((eventList.event[i].trigMode == 5 || eventList.event[i].trigMode == 3)) {
				if (eventList.event[i].enabled) {
					// ��������ʾ��NPC��������
				} else {
					// ����ʾ��NPCֱ������
					// ����NPCֻ���ǵ���
					int x = eventList.event[i].pos[0].x;
					int y = eventList.event[i].pos[0].y;
					//
					gotoxyaa(x, y);
					string str1 = map[y].substr(x, 2);
					setColor(getColor(str1));
					cout << str1;
				}
			}
		}
	};
	void showMap() {
		cout << Map_Height << "," << Map_Width << endl;
		//system("pause");
		//return ;
		for (int i = 0; i < Map_Height; i++) {
			for (int j = 0; j < Map_Width; j++) {
				gotoxyaa(j * 2, i);
				string str1;
				str1 = map[i].substr(j * 2, 2);
				setColor(getColor(str1));
				cout << str1;
			}
		}
		showNPC();
	}

	void showMapPart(int x, int y, int w, int h) {
		//cout << Map_Height << "," << Map_Width << endl;
		//return ;
		int xx, yy;
		for (int i = 0; i < h; i++) {
			for (int j = 0; j < w; j++) {
				xx = x + j;
				yy = y + i;
				gotoxyaa(xx * 2, yy);
				string str1;
				str1 = map[yy].substr(xx * 2, 2);
				setColor(getColor(str1));
				cout << str1;
			}
		}
		showNPC();
	}

	void loadMap(string aDataPath, string aSndPath, string FName, World *wd) {
		string str1;
		// ���Ȼ�ȡ���ļ�����Ϊ��ͼ��
		Name = FName;
		world = wd;
		//cout << aDataPath << endl;
		//cout << aSndPath << endl;
		//cout << FName << endl;
		// ������Ƶ
		char ctemp[100];
		string temp;
		GetPrivateProfileString("map", FName.c_str(), "", ctemp, 100, config.c_str());
		temp = string(ctemp);
		str1 = aSndPath + temp;
		//cout << str1 << endl;
		sound.load(str1);
		//system("pause");
		// �����ͼ
		str1 = aDataPath + FName + ".map";
		//cout << str1 << endl;
		ifstream fin(str1.c_str());
		int i = 0;
		Map_Height = 0;
		if (fin) {
			//cout << "load map" << endl;
			while (!fin.eof()) {
				getline(fin, str1);
				// ͬʱ���浽map��
				if (i == 0) {
					Map_Width = str1.size();
				}
				if (str1.size() < Map_Width) {
					//Map_Height++;
					str1 = str1 + manyChar(" ", Map_Width - str1.size()) ;
				}
				map[i] = str1;
				Map_Height++;
				i++;
			}
			fin.close();
			Map_Width = Map_Width / 2;
		}
		// ������ɫ����
		str1 = aDataPath + FName + "_color.txt";
		//cout << str1 << endl;
		fin.open(str1.c_str());
		i = 0;
		if (fin) {
			//cout << "load color map" << endl;
			while (!fin.eof()) {
				getline(fin, str1);
				// ͬʱ���浽map��
				mapcolor[i] = str1;
				i++;
			}
			Map_Color_Num = i;
			fin.close();
			//cout << Map_Color_Num << endl;
		}
		// ����ͨ�ж���Ϣ
		str1 = aDataPath + FName + "_move.map";
		//cout << str1 << endl;
		fin.open(str1.c_str());
		i = 0;
		if (fin) {
			//cout << "load move map" << endl;
			while (!fin.eof()) {
				getline(fin, str1);
				// ͬʱ���浽map��
				mapmove[i] = str1;
				i++;
			}
			fin.close();
		} else {
			// δ����ͨ�жȣ�����ȫ������
			for (int i = 0; i < Map_Height; i++) {
				mapmove[i] = manyChar("1", Map_Width * 2);
			}
		}
		// �����ͼ�¼�
		str1 = aDataPath + FName + "_event.txt";
		//cout << str1 << endl;
		eventList.readEvent(str1, wd);
		//eventList.show();
		// ��һ����Կ��������ص�Ч��
		//system("pause");
	}
};

// ս����ɫ��Ϣ
struct ActorInfo {
	string Name;
	string Face;
	int Lev, att, def, hpMax;
	int hp, exp;
	void showInfo(int x, int y, int aWait) {
		vector <string> info;
		info.push_back(fitStr("��" + Name + "��"));
		info.push_back(fitStr("�ȼ���" + int2str(Lev)));
		info.push_back("������" + fitStr(int2str(att)));
		info.push_back("������" + fitStr(int2str(def)));
		info.push_back("������" + fitStr(int2str(hp) + "/" + int2str(hpMax)));
		info.push_back("���飺" + fitStr(int2str(exp) + "/100"));
		//
		setColor(15);
		int a = 4;
		int b = 2;
		int w = 22;
		int h = 10;
		drawWindow(x, y, w, h);
		for (int i = 0; i < info.size(); i++) {
			textOut(x + a, y + b + i, info.at(i), w, 0);
		}
		// ��ͣһ��
		if (aWait >= 0) {
			// ��ʾֹͣ��ʾ
			gotoxyaa(x + w - 4, y + h - 2);
			cout << "��" ;
			// ֻ��ѡ���ȡ���������ܼ��������������
			while (1) {
				if (kbhit()) {
					char ch1 = getch();
					if (ch1 == 27 || ch1 == 32 || ch1 == 13) {
						break;
					}
				}
				//onTime();
			}
		}
	}
};

// һ��ս������
struct Fight {
	ActorInfo *left;          // ���ͨ��Ϊ���
	ActorInfo *right;	      // �ұ�ͨ��ΪNPC
	int rnd, half;            // �غ������볡��
	int result;				  // ս����� 0=ս���� 1=���ʤ��2=�ұ�ʤ
	ActionList *win;		  // ʤ���ű�
	ActionList *lose;         // ʧ�ܽű�
	void fightRound() {
		if (result != 0) {
			// �Ѿ��ֳ�ʤ������ս��
			return;
		}
		// �������볡�ֱ���
		if (half == 0) {
			int a;
			a = right->hp - (left->att - right->def);
			if (a <= 0) {
				a = 0;
				result = 1;
			}
			right->hp = a;
			half = 1;
		} else {
			int a;
			a = left->hp - (right->att - left->def);
			if (a <= 0) {
				a = 0;
				result = 2;
			}
			left->hp = a;
			half = 0;
			rnd++;
		}
		if (result == 1) {
			// ִ��ʤ���ű�
		} else if (result == 2) {
			// ִ��ʧ�ܽű�
		}
	}
	void init(ActorInfo *a1, ActorInfo *a2, ActionList *s1, ActionList *s2) {
		left = a1;
		right = a2;
		win = s1;
		lose = s2;
		rnd = 1;
		half = 0;
		result = 0;
	}
	void showScene() {
		// ���ڴ��
		drawWindow(0, 1, 60, 12);
		// ��ʾ��Ϣ
		left->showInfo(2, 2, -1);
		right->showInfo(36, 2, -1);
		// ��ʾ�غ������볡��
		//drawWindow(24, 6, 12, 4);
		string str1;
		str1 = "��" + fitStr(int2str(rnd)) + "�غ�";
		textOut(26, 7, fitStr(str1), 10, 0);
		if (half == 0) {
			str1 = "  �ҷ�";
		} else {
			str1 = "  �з�";
		}
		textOut(26, 8, fitStr(str1), 20, 0);
	}
};

struct MapWorld: World {
	Map mapList[20];
	int mapNum;
	string currentMapName;
	string dataPath;
	string savePath;
	string sndPath;
	// Ӣ�ۻ�����Ϣ
	ActorInfo info;
	ActorInfo enemy;
	// ս������
	Fight fight;
	ActionList callBack;
	// �˵��б�
	Menu menu[50];
	int menuNum;
	int currentMenu;
	stack <int> menuStack;
	// ����ģʽ����ʾ������Ϣ������
	bool debugOn;
	// ��ʱ��
	bool stopTimer;
	int pre_time;
	// ���ֲ�����
	AudioClip sound;
	// ��ʾ������Ϣ
	void Info() {
		loadInfoFromVar();
		info.showInfo(2, 2, 0);
		showMap();
	}
	// ���Ŷ���Ч�ĺ���
	void playSound(string aName) {
		char ctemp[100];
		string temp, str1, str2;
		GetPrivateProfileString("sound", aName.c_str(), "", ctemp, 100, config.c_str());
		temp = string(ctemp);

		sound.load(sndPath + temp);
		sound.play();
	}
	// ս������
	int fightOn(string aTeam, ActionList *win, ActionList *lose) {
		// ս����ʼ������ת��
		mapList[CurrentMap].sound.stop();
		playSound("fight");
		// ս��ʱ����������Ϣ
		char ctemp[100];
		string temp, str1, str2;
		GetPrivateProfileString("team", aTeam.c_str(), "", ctemp, 100, config.c_str());
		temp = string(ctemp);
		//
		split2(temp, ',', str1, str2);
		enemy.Name = str1;
		temp = str2;
		split2(temp, ',', str1, str2);
		enemy.Lev = str2int(str1);
		temp = str2;
		split2(temp, ',', str1, str2);
		enemy.att = str2int(str1);
		temp = str2;
		split2(temp, ',', str1, str2);
		enemy.def = str2int(str1);
		temp = str2;
		split2(temp, ',', str1, str2);
		enemy.hpMax = str2int(str1);
		enemy.hp = enemy.hpMax;
		enemy.exp = 0;
		loadInfoFromVar();
		fight.init(&info, &enemy, win, lose);
		fight.showScene();
		showSpecMenu("fight", 24, 13, &callBack);
		// ֻ��ѡ���ȡ���������ܼ��������������
	}
	// ս���˵��ص�����
	void callBackFunc() {
		// �ӱ����л�øղ�ѡ��Ĵ���
		string s1 = varList.getValue("Fight_Id");
		if (s1 == "0") {
			// ����
			Sleep(50);
			playSound("att1");
			fight.fightRound();
			// ���ݱ仯����
			saveInfoToVar();
			fight.showScene();
			Sleep(1000);
			if (fight.result == 1) {
				// ʤ��������£����Ӿ���ֵ
				sound.stop();
				playSound("win");
				fight.left->exp += ((fight.right->Lev + 1) * 40 / (fight.left->Lev + 1));
				if (fight.left->exp >= 100) {
					Sleep(1000);
					sound.stop();
					playSound("upLevel");
				}
				while (fight.left->exp >= 100) {
					fight.left->Lev += 1;
					fight.left->att += 2;
					fight.left->def += 1;
					fight.left->hpMax += 5;
					fight.left->exp -= 100;
				}
				saveInfoToVar();
				fight.win->execute();
				// ִ�д���󻭵�ͼ��ս�ܵ�NPC����ʧ
				showMap();
				sound.stop();
				mapList[CurrentMap].sound.play();
			} else {
				playSound("att2");
				fight.fightRound();
				fight.showScene();
				if (fight.result == 2) {
					showMap();
					sound.stop();
					mapList[CurrentMap].sound.play();
					fight.lose->execute();
				} else {
					// �ٴ���ʾ�˵������봫�ص��ű����ʴ˿�������0
					showSpecMenu("fight", 24, 13, 0);
				}
			}
		} else if (s1 == "1") {
			// ���ܣ�ֱ���˳�
			sound.stop();
			mapList[CurrentMap].sound.play();
			showMap();
		}
	}
	//
	virtual void updateNPC() {
		// ��Ȼ��ˢ�µ�ǰ��ͼ
		mapList[CurrentMap].eventList.updateEvent(5);
	}
	virtual void hideNPC() {
		// ͬʱˢ�µ�ǰ��ͼ
		mapList[CurrentMap].eventList.hideNPC(5);
		mapList[CurrentMap].hideNPC();
	}
	//
	void savetoFile(string aName) {
		ofstream fout(aName.c_str());
		// ���豣�����Ϣ
		// ��ǰ���ڵ�ͼ
		fout << mapList[CurrentMap].Name << endl;
		// ��ǰλ��
		fout << hero.x << "," << hero.y << endl;
		// ��ǰȫ���Ѷ���ı���
		fout << varList.varNum << endl;
		for (int i = 0; i < varList.varNum; i++) {
			fout << varList.var[i].name << "=" << varList.var[i].value << endl;
		}
		// Ӣ�����ݱ���
		// ���ﲻ���������� saveInfoToVar();
		// �ر�
		fout.close();
	}
	void save() {
		// ��������ڣ��򴴽��ļ���
		//cout << getExePath()+ "\\"+ savePath << endl;
		makeDir(getExePath() + "\\" + savePath);
		// ��ȡ�ļ���
		string str1 = getTimeName();
		// ������λ�ַ�����ȷ����ʾ����
		str1 = str1 + manyChar(" ", 15 - str1.size());
		str1 = str1 + "@" + mapList[CurrentMap].Name;
		//
		// cout << str1 << endl;
		// ����
		savetoFile(savePath + str1 + ".sav");
	}
	void loadFromFile(string aName) {
		ifstream fin(aName.c_str());
		// ��ǰ���ڵ�ͼ
		fin >> currentMapName;
		// ��ǰλ��
		string str1;
		fin >> str1;
		hero.setValue(str1);
		// ��ǰȫ���Ѷ���ı���
		fin >> varList.varNum;
		for (int i = 0; i < varList.varNum; i++) {
			fin >> str1;
			string str2, str3;
			split2(str1, '=', str2, str3);
			varList.var[i].name = str2;
			varList.var[i].value = str3;
		}
		// Ӣ�����ݶ�ȡ
		loadInfoFromVar();
		// �ر�
		fin.close();
	}
	void load(string fName) {
		//cout << fName;
		loadFromFile(savePath + fName + ".sav");
		changeMap(currentMapName, hero.x, hero.y);
	}
	void loadData() {
		// �漰�˻����ܹ���ֱ�ӳ���������������������
		string str1, str2, str3;
		char cpath[100];
		GetPrivateProfileString("system", "datapath", ".\\data\\", cpath, 100, config.c_str());
		dataPath = string(cpath);
		GetPrivateProfileString("system", "savepath", ".\\save\\", cpath, 100, config.c_str());
		savePath = string(cpath);
		GetPrivateProfileString("system", "soundpath", ".\\sound\\", cpath, 100, config.c_str());
		sndPath = string(cpath);
		// ����ģʽ
		debugOn = GetPrivateProfileInt("system", "debug", 1, config.c_str());
		// �޸Ĵ������ƣ��޸Ĵ��ڴ�С
		char ctemp[100];
		string temp;
		GetPrivateProfileString("windows", "title", "", ctemp, 100, config.c_str());
		temp = string(ctemp);
		setConsoleTitle(temp);
		//
		int ww = GetPrivateProfileInt("windows", "width", 30, config.c_str());
		int hh = GetPrivateProfileInt("windows", "height", 30, config.c_str());
		setConsoleSize(ww, hh);
		// �����ͼ���˵���Ϣ
		//GetPrivateProfileString("system", "mainMenu", "", ctemp, 100, config.c_str());
		//temp= string(ctemp);
		// ��ʼ��Ϣ
		GetPrivateProfileString("system", "enter", "", ctemp, 100, config.c_str());
		currentMapName = string(ctemp);
		//CurrentMap= GetPrivateProfileInt("system", "enter", 0, config.c_str());
		GetPrivateProfileString("system", "initPos", "", ctemp, 100, config.c_str());
		temp = string(ctemp);
		hero.setValue(temp);
		//mainMenu.loadFromFile(path+temp, this);
		//system("pause");
	}
	void hideEvent(Event *evt) {
		evt->enabled = false;
	}
	void loadInfoFromVar() {
		info.Face = varList.getValue("_Face");
		info.Name = varList.getValue("_Name");
		info.att = str2int(varList.getValue("_att"));
		info.def = str2int(varList.getValue("_def"));
		info.hp = str2int(varList.getValue("_hp"));
		info.hpMax = str2int(varList.getValue("_hpMax"));
		info.Lev = str2int(varList.getValue("_Lev"));
		info.exp = str2int(varList.getValue("_Exp"));
	}
	void saveInfoToVar() {
		varList.setValue("_Face", info.Face);
		varList.setValue("_Name", info.Name);
		varList.setValue("_Att", int2str(info.att));
		varList.setValue("_def", int2str(info.def));
		varList.setValue("_hp", int2str(info.hp));
		varList.setValue("_hpMax", int2str(info.hpMax));
		varList.setValue("_Lev", int2str(info.Lev));
		varList.setValue("_Exp", int2str(info.exp));
	}
	void InitVar() {
		varList.varNum = 0;
		char ctemp[100];
		string temp;
		GetPrivateProfileString("hero", "name", "", ctemp, 100, config.c_str());
		info.Name = string(ctemp);
		GetPrivateProfileString("hero", "Face", "", ctemp, 100, config.c_str());
		info.Face = string(ctemp);
		info.att = GetPrivateProfileInt("hero", "att", 1, config.c_str());
		info.def = GetPrivateProfileInt("hero", "def", 1, config.c_str());
		info.Lev = GetPrivateProfileInt("hero", "Lev", 1, config.c_str());
		info.hpMax = GetPrivateProfileInt("hero", "hp", 1, config.c_str());
		info.hp = info.hpMax;
		info.exp = 0;
		// ͬʱ���浽����ϵͳ��
		saveInfoToVar();
		//system("pause");
	}
	void init() {
		CurrentMap = -1;
		HideCursor();
		pre_time = clock();
		// ʵ�ֻص�����
		callBack.cmd[0] = new Action();
		callBack.cmd[0]->cmd = "CALLBACK";
		callBack.cmd[0]->world = this;
		callBack.cmdNum = 1;
	}
	void changeMap(string aName, int x, int y) {
		// ����ֹͣ������
		if (CurrentMap >= 0) {
			mapList[CurrentMap].sound.stop();
		}
		// ���ҵ�ͼ
		bool found = false;
		for (int i = 0; i < mapNum; i++) {
			if (mapList[i].Name == aName) {
				CurrentMap = i;
				found = true;
				break;
			}
		}
		if (!found) {
			// δ�ҵ������
			mapList[mapNum].loadMap(dataPath, sndPath, aName, this);
			CurrentMap = mapNum;
			mapNum++;
		}
		// ����ͼ��������
		mapList[CurrentMap].sound.play();
		//cout << str1;
		//system("pause");
		//
		hero.x = x;
		hero.y = y;
		// ִ��ָ��ǰ��ʾ��ͼ�������Զ�ָ�������ص�NPC���ºۼ�
		showMap();
		// ִ���Զ�ָ��
		Event *evt;
		evt = mapList[CurrentMap].eventList.findEvent(4, -1, -1);
		if (evt) {
			//system("pause");
			evt->block.execute();
		}
		// ִ��ָ�������ʾ��ͼ�����谭ָ���е����Ĳ˵�
		// showMap();
	}
	void talk(string Who, string Words, int aWait) {
		int x, y, w, h;
		x = 2;
		w = 60;
		h = 10;
		// ����λ�ã�ѡ�������ﻭ����
		setColor(15);
		if (hero.y < 15) {
			drawWindow(0, 20, w, h);
			y = 22;
		} else {
			drawWindow(0, 0, w, h);
			y = 2;
		}
		for (int i = 0; i < Who.size() / 2; i++) {
			gotoxyaa(x + 2 + i * 2, y);
			cout << Who.substr(i * 2, 2);
			Sleep(50);
		}
		int b = 0;
		int a = 2;
		for (int i = 0; i < Words.size() / 2; i++) {
			gotoxyaa(x + a, y + 2 + b);
			cout << Words.substr(i * 2, 2);
			Sleep(50);
			a += 2;
			if (a > 50) {
				b++;
				a = 2;
			}
		}
		if (aWait >= 0) {
			// ˵һ��֮����ͣһ��
			// ��ʾһ����������Ϊ��ʾ
			gotoxyaa(x + w - 8, y + h - 5);
			cout << "��" ;
			// ֻ��ѡ���ȡ���������ܼ��������������
			while (1) {
				if (kbhit()) {
					char ch1 = getch();
					if (ch1 == 27 || ch1 == 32 || ch1 == 13) {
						break;
					}
				}
				//onTime();
			}
		}
	}
	bool checkSpeak(int x, int y) {
		// 2 �ǹ̶������̽����
		mapList[CurrentMap].eventList.runEvent(2, x, y);
		// 3 �ǿ������ƶ���NPC
		mapList[CurrentMap].eventList.runEvent(3, x, y);
		// 5 �ǿ������ƶ��ĵ��ˣ�ײ��ͬ������
		mapList[CurrentMap].eventList.runEvent(5, x, y);
	}
	void showSpecMenu(string aName, int x, int y, ActionList *aCode) {
		// �����ҵ�����˵��Ƿ��Ѿ�ռλ
		int find = -1;
		int lock;
		for (int i = 0; i < menuNum; i++) {
			// ����˵�
			if (menu[i].Name == aName) {
				find = i;
			}
		}
		if (find >= 0) {
			currentMenu = find;
		} else {
			currentMenu = menuNum;
			menu[currentMenu].world = this;
			menu[menuNum].Name = aName;
			menuNum++;
			// �½��˵�
			if (upCase(aName) == "FIGHT") {
				menu[currentMenu].addItem("����", aCode);
				menu[currentMenu].addItem("����", aCode);
			}
			lock = 1;
		}
		// ����˵��������Ѿ��򿪵����⣬������������
		if (upCase(aName) == "SAVE") {
			vector <string> files;
			getFiles(savePath, "sav", files);
			menu[currentMenu].clear();
			sort(files.begin(), files.end());
			int fsum = files.size();
			for (int i = 1; i <= fsum && i <= 5; i++) {
				string str1 = files.at(fsum - i);
				int a = str1.find_last_of(".");
				string str2 = str1.substr(0, a);

				//cout << str1 << endl;
				//aCode->show();
				menu[currentMenu].addItem(str2, aCode);
			}
			lock = 0;
		}
		//
		menu[currentMenu].showMenu(x, y, lock);
		menuOn = true;
	}
	void showMenu(string aName, int x, int y, int aLockMode) {
		// ����´򿪵Ĳ˵����������Ĳ˵�����¼֮
		// ����˵��Ѿ����룬��ֱ�����У�����������
		int find = -1;
		for (int i = 0; i < menuNum; i++) {
			if (menu[i].Name == aName) {
				find = i;
			}
		}
		if (find >= 0) {
			currentMenu = find;
		} else {
			menu[menuNum].loadFromFile(dataPath + aName + ".txt", this);
			menu[menuNum].Name = aName;
			currentMenu = menuNum;
			menuNum++;
		}
		menu[currentMenu].showMenu(x, y, aLockMode);
		menuOn = true;
	}
	void showHero() {
		gotoxyaa(hero.x, hero.y);
		setColor(mapList[CurrentMap].getColor(info.Face));
		cout << info.Face;
	}
	void moveHero(int x, int y) {
		// �����ж��Ƿ������ߵ���һ����
		if (debugOn) {
			gotoxyaa(70, 4);
			cout << mapList[CurrentMap].Map_Width << "-" << mapList[CurrentMap].Map_Height;
			gotoxyaa(70, 5);
			cout << x << "," << y << "    ";
			gotoxyaa(70, 6);
			cout << hero.x << "," << hero.y << "    ";
		}
		if (x < 0 || y < 0 || x / 2 >= mapList[CurrentMap].Map_Width || y >= mapList[CurrentMap].Map_Height) {
			// ���ߣ�Ҳ������
			if (!checkSpeak(x, y)) {
				Beep(200, 100);
			}
			return;
		}
		if (mapList[CurrentMap].canMove(x / 2, y)) {
			//
			string under;
			under = mapList[CurrentMap].map[hero.y].substr(hero.x, 2);
			gotoxyaa(hero.x, hero.y);
			setColor(mapList[CurrentMap].getColor(under));
			cout << under;
			hero.x = x;
			hero.y = y;
			showHero();
			// ʵ�ʷ����ƶ����Ŵ����ƶ��¼�
			mapList[CurrentMap].eventList.runEvent(1, hero.x, hero.y);
		} else {
			// ��������ʾ
			if (!checkSpeak(x, y)) {
				Beep(200, 100);
			}
		}
	}
	void showMap() {
		system("cls");
		mapList[CurrentMap].showMap();
		showHero();
	}
	void showMapPart(int x, int y, int w, int h) {
		mapList[CurrentMap].showMapPart(x, y, w, h);
		showHero();
	}
	void mapControl(char ch1) {
		// ��һ��������������ֵ����һ����224
		switch (ch1) {
			case 72:
				// cout << "�����ƶ�" << endl;
				moveHero(hero.x, hero.y - 1);
				break;
			case 80:
				//cout << "�����ƶ�" << endl;
				moveHero(hero.x, hero.y + 1);
				break;
			case 75:
				//cout << "�����ƶ�" << endl;
				moveHero(hero.x - 2, hero.y);
				break;
			case 77:
				//cout << "�����ƶ�" << endl;
				moveHero(hero.x + 2, hero.y);
				break;
			case 27:
				//cout << "ȡ��" << endl;
				showMenu("mainMenu", 5, 5, 0);
			case 13:
			case 32:
				//cout << "̽��" << endl;
				break;
		}
	}
	void menuControl(char ch1) {
		switch (ch1) {
			case 72:
				// cout << "�����ƶ�" << endl;
				menu[currentMenu].showDot(0);
				if (menu[currentMenu].yy > menu[currentMenu].top ) {
					menu[currentMenu].yy = menu[currentMenu].yy - 1;
				} else {
					menu[currentMenu].yy = menu[currentMenu].bottom;
				}
				menu[currentMenu].showDot(1);
				break;
			case 80:
				//cout << "�����ƶ�" << endl;
				menu[currentMenu].showDot(0);
				if (menu[currentMenu].yy < menu[currentMenu].bottom) {
					menu[currentMenu].yy = menu[currentMenu].yy + 1;
				} else {
					menu[currentMenu].yy = menu[currentMenu].top;
				}
				menu[currentMenu].showDot(1);
				break;
			case 27:
				// �˵�ģʽ�ر�
				if (!menu[currentMenu].LockMode) {
					menu[currentMenu].hideMenu();
					// ����˵�ջ����
					// ��ʱӦ����������ǰһ���˵�
					if (!menuStack.empty()) {
						currentMenu = menuStack.top();
						menuStack.pop();
						// ��ԭ�˵��ĵ�ǰѡ���
						int a = menu[currentMenu].yy;
						menu[currentMenu].showMenu(menu[currentMenu].wx, menu[currentMenu].wy, menu[currentMenu].LockMode);
						menuOn = true;
						// ͬʱ������ǰһ�˵���ѡ��λ��
						menu[currentMenu].showDot(0);
						menu[currentMenu].yy = a;
						menu[currentMenu].showDot(1);
					}
				}
				break;
			case 32:
			case 13:
				//ѡ����ѡ��
				int a = menu[currentMenu].yy - menu[currentMenu].top;
				// ��ѡ������ݺ�ID�ż�¼�ڱ����У��Ա�ű�ʹ��
				varList.setValue(menu[currentMenu].Name + "_id", int2str(a));
				varList.setValue(menu[currentMenu].Name + "_name", menu[currentMenu].item[a].text);
				//gotoxyaa(0, 0);
				//setColor(15);
				//cout << menu[currentMenu].Name+"_id" << endl;
				// ��ʱ��¼��ǰ�Ĳ˵���
				int m1 = currentMenu;
				//gotoxyaa(70, 9);
				//cout << "execute" << endl;
				//item[a].code.show();
				// ���menuOn�Ŀ��ر������ִ��ָ��֮ǰ
				// ����ִ�е�ָ������л��������򿪲˵��Ĳ��������쳣
				menu[currentMenu].hideMenu();
				menu[currentMenu].item[a].code->execute();
				// ������й��󣬲˵��ŷ����仯������ֲ˵��ݽ������⡣
				if (currentMenu != m1) {
					menuStack.push(m1);
				} else {
					// û�д򿪲˵��������в˵����н����
					while (!menuStack.empty()) {
						menuStack.pop();
					}
				}
				break;
		}
	}
	void onTime() {
		if (stopTimer || menuOn) {
			return;
		}
		int x = clock();
		if (x - pre_time > CLOCKS_PER_SEC) {
			mapList[CurrentMap].moveNPC();
			// NPC�ƶ�������������������
			mapList[CurrentMap].eventList.runEvent(5, hero.x, hero.y);
			pre_time = x;
		}
	}
	void run() {
		if (CurrentMap >= 0) {
			showMap();
			// ִ���Զ�ָ��
			mapList[CurrentMap].eventList.runEvent(4, -1, -1);
		} else {
			changeMap(currentMapName, hero.x, hero.y);
		}
		unsigned char ch1;
		while (true) {
			if (kbhit()) {
				ch1 = getch();
				if (debugOn) {
					gotoxyaa(70, 7);
					cout << int(ch1) << "," << menuOn << "    ";
				}
				if (menuOn) {
					menuControl(ch1);
				} else {
					mapControl(ch1);
				}
			} else {
				onTime();
			}
		}
	}
};

MapWorld world;

int main() {
	world.loadData();
	world.init();
	//system("pause");
	world.run();
	/*
	int a= menuMove("��ʼ����Ϸ,�������,�˳���Ϸ");
	if (a== 0){
		mapMove(hero);
	}
	*/
}

