#include <iostream>
#include <curl/curl.h>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <string>
#include <ctime>
#include "tools.h"


using namespace std;

struct Var {
	string name;
	string value;
};

size_t WriteCallbackgo(void *contents, size_t size, size_t nmemb, string *response) {
	size_t totalSize = size * nmemb;
	response->append((char *)contents, totalSize);
	return totalSize;
}

string getweb(string url) {
	CURL *curl = curl_easy_init();
	if (curl) {
		string response;
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallbackgo);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);

		struct curl_slist *headers = NULL;
		headers = curl_slist_append(headers, "Accept-Charset: ANSI");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		CURLcode res = curl_easy_perform(curl);
		if (res != CURLE_OK) {
			cerr << "Failed to perform request: " << curl_easy_strerror(res) << endl;
			response = "";
		}
		curl_easy_cleanup(curl);
		curl_slist_free_all(headers);
		return response;
	} else {
		cerr << "Failed to initialize libcurl" << endl;
		return "";
	}
}

struct VarList {
	Var var[100];
	int varNum;
	void setValue(string aName, string aValue) {
		// �ж�aName���Ƿ������urltext|��
		string v9 = "V9";
		string str1;
		string bname = aName;
		if (aName == v9) {
			cout << aName << endl;
		}

		if (aName.find("URLTEXT|") != string::npos) {
			// ��ȡ��ַ
			string url = aValue;
			// ͨ��get�����ȡ��ҳ����
			str1 = getweb(url);
			// ��ȡaName��%rultext|���ұߵ�������Ϊname���浽var��
			bname = aName.substr(aName.find("URLTEXT|") + 8);
			// ��ӻ��޸ı���ֵ
			//cout << bname << endl;

		}

		// Ѱ�ұ����Ƿ���ڣ���Сд������
		int find = -1;
		for (int i = 0; i < varNum; i++) {
			if (upCase(bname) == upCase(var[i].name)) {
				find = i;
				break;
			}
		}

		if (aValue[0] == '%') {
			str1 = getValue(aValue.substr(1));
		} else {
			if (str1 == "") {
				str1 = aValue;

			}
		}
		if (find >= 0) {
			// ����ҵ���ֱ���޸�ֵ
			var[find].value = str1;
		} else {
			// ������±����������
			var[varNum].name = bname;
			var[varNum].value = str1;
			varNum++;
		}


	}

	string getValue(string aName) {
		size_t pos = 0;
		string originalString = aName;
		bool is_pos = false;
		while ((pos = originalString.find('%', pos)) != string::npos) {
			is_pos = true;
			originalString.replace(pos, 1, ""); // �� "%" �滻Ϊ ""
			pos += 1; // ֱ���ƶ�����һ��λ�ã���Ϊ�滻��û���ַ�����
		}
		// Ѱ�ұ����Ƿ���ڣ���Сд������
		if (is_pos) {
			//cout << "getValue" << originalString << endl;

		}
		int find = -1;
		for (int i = 0; i < varNum; i++) {
			if (upCase(originalString) == upCase(var[i].name)) {
				find = i;
				break;
			}
		}
		if (find >= 0) {
			// ����ҵ�
			return var[find].value;
		} else {
			// ����ǲ�����
			return "";
		}
	}
	// �����������ʽ��Ŀǰֻ֧����򵥵ģ�
	bool calcRelaExp(string aExp) {
		for (int i = 0; i < varNum; i++) {
			//cout << var[i].name << " : " << var[i].value << endl;
		}
		//cout << aExp << endl;
		string str1, str2;
		split2(aExp, '=', str1, str2);
		string vst1, vst2;
		if (str1 != "") {
			if (str1 == "%time") {
				time_t now = time(0);
				tm *localTime = localtime(&now);
				int currentHour = localTime->tm_hour;
				return (str2int(str2) == currentHour);
			}
			if (str1[0] == '%') {
				vst1 = getValue(str1.substr(1));
			} else {
				vst1 = str1;
			}
			//cout << "a1: " << str1 << endl;
			//cout << "a2: " << str2 << endl;
			//cout << "b1: " << vst1 << endl;
			if (str2 == "") {
				return str2int(vst1);
			} else {
				if (str2[0] == '%') {
					vst2 = getValue(str2.substr(1));
				} else {
					vst2 = str2;
				}
				//cout << "b2: " << vst2 << endl;
				return (vst2 == vst1);
			}
		} else {
			return false;
		}
	}
};

struct Pos {
	int x, y;
	void setValue(string aStr) {
		string str1, str2;
		split2(aStr, ',', str1, str2);
		x = str2int(str1);
		y = str2int(str2);
	}
};

struct ActionList;
struct Event;

struct World {
	VarList varList;
	int CurrentMap;
	Pos hero;
	bool menuOn;
	virtual void changeMap(string aName, int x, int y) {}
	virtual void showMap() {}
	virtual void showMapPart(int x, int y, int w, int h) {}
	virtual void showMenu(string aName, int x, int y, int aLockMode) {}
	virtual void showSpecMenu(string aName, int x, int y, ActionList *aCode) {}
	virtual void talk(string Who, string Words, int aWait) {}
	virtual void save() {}
	virtual void load(string fName) {}
	virtual int fightOn(string aTeam, ActionList *win, ActionList *lose) {}
	virtual void callBackFunc() {}
	virtual void Info() {}
	virtual void InitVar() {}
	virtual void updateNPC() {}
	virtual void hideNPC() {}
	virtual void hideEvent(Event *evt) {}
};

struct Action {
	string cmd;
	Event *event;
	World *world;
	virtual void execute() {
		if (upCase(cmd) == "EXIT") {
			exit(0);
		} else if (upCase(cmd) == "SAVE") {
			world->save();
		} else if (upCase(cmd) == "INIT") {
			world->InitVar();
		} else if (upCase(cmd) == "HIDESELF") {
			world->hideEvent(event);
		} else if (upCase(cmd) == "INFO") {
			world->Info();
		} else if (upCase(cmd) == "CALLBACK") {
			world->callBackFunc();
		} else if (upCase(cmd) == "UPDATENPC") {
			world->updateNPC();
		} else if (upCase(cmd) == "HIDENPC") {
			world->hideNPC();
		}
	}
	virtual void show() {
		cout << "cmd: " << cmd << endl;
	}
};

// �����������ط��õ����ƵĽṹ����ϣ����һ������ͳһ����
// �ʽ����ṹ
struct ActionList {
	// ���������ָ�룬����ָ����̳ж���
	Action *cmd[50];
	int cmdNum;
	void show() {
		cout << "ָ������ " << cmdNum  << " {" << endl;
		for (int i = 0; i < cmdNum; i++) {
			cmd[i]->show();
		}
		cout << "}" << endl;
	}
	void execute() {
		for (int i = 0; i < cmdNum; i++) {
			gotoxyaa(0, 31);
			cmd[i]->execute();
		}
	}
};

struct CLoad: Action {
	string Name;
	void show() {
		cout << "load from file " << Name << endl;
	}
	void execute() {
		//cout << Name << endl;
		world->load(world->varList.getValue(Name));
	}
};

struct CJump: Action {
	string mapName;
	Pos newp;
	void show() {
		cout << "jump to " << mapName << ":" << newp.x << "-" << newp.y << endl;
	}
	void execute() {
		world->changeMap(mapName, newp.x, newp.y);
	}
};

struct CTalk: Action {
	string Name[20];
	string Speach[20];
	int sentNum;
	void execute() {
		for (int i = 0; i < sentNum; i++) {
			world->talk(Name[i], Speach[i], 0);
		}
		world->showMap();
	}
	void show() {
		cout << "Talk of " << sentNum << endl;
		for (int i = 0; i < sentNum; i++) {
			cout << "    sent " << Name[i] << "��" << Speach[i] << endl;
		}
	}
};

struct CVar: Action {
	string Name;
	string Value;
	void show() {
		cout << "������ֵ " << Name << "," << Value << endl;
	}
	void execute() {
		world->varList.setValue(Name, Value);
	}
};

struct CMenu: Action {
	string Name;
	Pos mpos;
	int Mode;   // ����ģʽ
	void show() {
		cout << "�򿪲˵� " << Name << endl;
	}
	void execute() {
		world->showMenu(Name, mpos.x, mpos.y, Mode);
	}
};

// ����˵�
struct CSpecMenu: Action {
	string Name;    // ����
	Pos mpos;
	ActionList onClick;
	void show() {
		cout << "������˵� " << Name << endl;
	}
	void execute() {
		//onClick.show();
		world->showSpecMenu(Name, mpos.x, mpos.y, &onClick);
	}
};

struct CIf: Action {
	string exp;
	ActionList block;
	ActionList elseblock;
	void show() {
		cout << "if (" << exp << ")" << endl;
		block.show();
		cout << "else" << endl;
		elseblock.show();
	}
	void execute() {
		//cout << exp;
		if (world->varList.calcRelaExp(exp)) {
			block.execute();
		} else {
			elseblock.execute();
		}
	}
};

struct CFight: Action {
	string team;
	ActionList win;
	ActionList lose;
	void show() {
		cout << "fight win with (" << team << ")" << endl;
		win.show();
		cout << "lose" << endl;
		lose.show();
	}
	void execute() {
		//cout << exp;
		world->fightOn(team, &win, &lose);
	}
};

struct Event {
	int trigMode;        // 1=�ߵ�����
	// 2=̽����ײ��������
	// 3=����߶���̽������
	// 4=�Զ�����
	// 5=������ߵĽӴ�����
	Pos pos[20];         // ����λ�ã�֧�ֶ��λ��
	int posNum;		     // λ������
	ActionList block;
	string chart;       // NPCͼ��
	int color;			// ͼ�����ɫ
	bool enabled;        // Ĭ���Ǵ򿪵ģ����Թرգ�������
	// ͨ��һ���ַ���������һ��λ�õ�
	void addPos(string aStr) {
		pos[posNum].setValue(aStr);
		posNum++;
	}
	void addPosList(string aStr) {
		string str1, str2;
		while (true) {
			split2(aStr, ';', str1, str2);
			addPos(str1);
			if (aStr != str1) {
				aStr = str2;
			} else {
				break;
			}
		}
	}
	//
	bool check(int aMode, int x, int y) {
		if (trigMode == aMode) {
			for (int i = 0; i < posNum; i++) {
				if (pos[i].x == x && pos[i].y == y) {
					return true;
				}
			}
		}
		return false;
	}
	//
	bool isNear(int aMode, int x, int y) {
		if (trigMode == aMode) {
			for (int i = 0; i < posNum; i++) {
				if ((pos[i].x == x + 2 && pos[i].y == y) ||
				        (pos[i].x == x - 2 && pos[i].y == y) ||
				        (pos[i].x == x && pos[i].y == y + 1) ||
				        (pos[i].x == x && pos[i].y == y - 1) ) {
					//
					//gotoxy(70, 8);
					//cout << x << "," << y << "     ";
					return true;
				}
			}
		}
		return false;
	}
	//
	void show() {
		block.show();
	}
};

// ����һ��ͨ�ú���������ֵ����Ϊ�˰���if...else...endif�����ж�
int readActions(ifstream &ff, 	ActionList &block, string aStop, World *wd, Event *evt) {
	block.cmdNum = 0;
	while (!ff.eof()) {
		string str1, str2;
		getline(ff, str1);
		if (str1 == "") {
			continue;
		} else if (str1[0] == ';') {
			// �ֺſ�ͷ����ע��
			continue;
		} else if (upCase(str1) == upCase(aStop)) {
			break;
		} else if (str1[0] == '@') {
			// һ���µ�ָ��Ŀ�ʼ
			str1 = str1.substr(1);
			if (upCase(str1) == "JUMP") {
				//cout << "jump" << endl;
				CJump *jump = new CJump();
				block.cmd[block.cmdNum] = jump;
				jump->world = wd;
				getline(ff, str2);
				jump->mapName = str2;
				getline(ff, str2);
				jump->newp.setValue(str2);
			} else if (upCase(str1) == "EXIT" || upCase(str1) == "SAVE" ||
			           upCase(str1) == "INIT" || upCase(str1) == "HIDESELF" ||
			           upCase(str1) == "INFO" || upCase(str1) == "UPDATENPC" ||
			           upCase(str1) == "HIDENPC" ) {
				Action *act = new Action();
				block.cmd[block.cmdNum] = act;
				act->cmd = upCase(str1);
				act->world = wd;
			} else if (upCase(str1) == "LOAD") {
				CLoad *cld = new CLoad();
				cld->world = wd;
				block.cmd[block.cmdNum] = cld;
				getline(ff, str2);
				cld->Name = str2;
			} else if (upCase(str1) == "MENU" || upCase(str1) == "LOCKMENU") {
				CMenu *cmn = new CMenu();
				cmn->world = wd;
				block.cmd[block.cmdNum] = cmn;
				getline(ff, str2);
				cmn->Name = str2;
				getline(ff, str2);
				cmn->mpos.setValue(str2);
				if (upCase(str1) == "LOCKMENU") {
					cmn->Mode = 1;     // ����ģʽ�Ĳ˵����ɹرգ������˵�
				} else {
					cmn->Mode = 0;    // ��ͨģʽ��������
				}
			} else if (upCase(str1) == "SPECMENU") {
				CSpecMenu *cmn = new CSpecMenu();
				cmn->world = wd;
				block.cmd[block.cmdNum] = cmn;
				getline(ff, str2);
				cmn->Name = str2;
				getline(ff, str2);
				cmn->mpos.setValue(str2);
				readActions(ff, cmn->onClick, "@endspec", wd, evt);
			} else if (upCase(str1) == "TALK") {
				//cout << "talk" << endl;
				CTalk *talk = new CTalk();
				talk->world = wd;
				block.cmd[block.cmdNum] = talk;
				talk->sentNum = 0;
				do {
					getline(ff, str2);
					if (str2[0] != '@') {
						split2(str2, ',', talk->Name[talk->sentNum], talk->Speach[talk->sentNum]);
						string speach = talk->Speach[talk->sentNum];
						size_t start = speach.find("<%");
						if (talk->Speach[talk->sentNum] != "" && start != string::npos) {
							while (start != string::npos) {
								size_t end = speach.find(">", start);
								if (end != string::npos) {
									string varName = speach.substr(start + 2, end - start - 2);
									string varValue = talk->world->varList.getValue(varName);
									speach.replace(start, end - start + 1, varValue);
								}
								start = speach.find("<%", start + 1);
							}
							talk->Speach[talk->sentNum] = speach;
						}
						talk->sentNum++;
					} else {
						break;
					}
				} while (1);
			} else if (upCase(str1) == "VAR") {

				CVar *cvar = new CVar();
				cvar->world = wd;
				block.cmd[block.cmdNum] = cvar;
				getline(ff, str2);
				split2(str2, '=', cvar->Name, cvar->Value);
			} else if (upCase(str1) == "IF") {
				CIf *cif = new CIf();
				cif->world = wd;
				block.cmd[block.cmdNum] = cif;
				getline(ff, str2);
				cif->exp = str2;
				int a = readActions(ff, cif->block, "@endif", wd, evt);
				if (a == 1) {
					// ˵���Ķ���@ELSE�����ģ�������
					readActions(ff, cif->elseblock, "@endif", wd, evt);
				}
			} else if (upCase(str1) == "FIGHT") {
				CFight *cfit = new CFight();
				cfit->world = wd;
				block.cmd[block.cmdNum] = cfit;
				getline(ff, str2);
				cfit->team = str2;
				int a = readActions(ff, cfit->win, "@endfight", wd, evt);
				if (a == 2) {
					// ˵���Ķ���@ELSE�����ģ�������
					readActions(ff, cfit->lose, "@endfight", wd, evt);
				}
			} else if (upCase(str1) == "ELSE") {
				// ����else����������﷨����
				//cout << "else"<< endl;
				if (upCase(aStop) == "@ENDIF") {
					// �����������﷨��ǰһ��block���
					// cout << "else2"<< endl;
					return 1;
				} else if (upCase(aStop) == "@ENDFIGHT") {
					return 2;
				} else {
					cout << "else ����" << str1;
					throw 1;
				}
			} else {
				cout << "δָ֪�" + str1 << endl;
				throw 2;
			}
			block.cmd[block.cmdNum]->world = wd;
			block.cmd[block.cmdNum]->event = evt;
			block.cmdNum++;
		} else {
			// ����Ҳ�Ǳ���
			cout << "ָ�����" << str1 << endl;
			throw 3;
		}
	}
	return 0;
}

struct EventList {
	Event event[50];
	int evtNum;

	void readEvent(string aName, World *wd) {
		//cout << "Event file: " << aName << endl;
		//system("pause");
		ifstream fin(aName.c_str());
		if (fin) {
			while (!fin.eof()) {
				string str1, str2;
				getline(fin, str1);
				if (str1 == "") {
					continue;
				} else if (str1[0] == '#') {
					// һ���µ��¼���ʼ
					event[evtNum].trigMode = (str1[1] - 'A') + 1;
					event[evtNum].enabled = true;
					//
					if (str1.size() > 6) {
						int a = str1.find(":");
						if (a > 0) {
							string str2 = getBetween(str1, "(", ")");
							string str3, str4;
							split2(str2, ',', str3, str4);
							event[evtNum].chart = str3;
							event[evtNum].color = str2int(str4);
						} else {
							event[evtNum].chart = "Ů";
							event[evtNum].color = 10;
						}
					} else {
						event[evtNum].chart = "��";
						event[evtNum].color = 12;
					}
					// �����Ȼ��λ�ö���
					getline(fin, str2);
					event[evtNum].addPosList(str2);
					readActions(fin, event[evtNum].block, "#END", wd, &event[evtNum]);
					//event[evtNum].show();
					evtNum++;
				}
			}
		}
		fin.close();
		//system("pause");
	}
	// ���Եıر�����
	void show() {
		for (int i = 0; i < evtNum; i++) {
			cout << "Event " << i << endl;
			event[i].show();
		}
	}
	// �������꣬��λevent
	Event *findEvent(int aMode, int x, int y) {
		for (int i = 0; i < evtNum; i++) {
			if (event[i].check(aMode, x, y)) {
				return &event[i];
			}
		}
		return 0;
	}
	Event *nearEvent(int aMode, int x, int y) {
		for (int i = 0; i < evtNum; i++) {
			if (event[i].isNear(aMode, x, y)) {
				return &event[i];
			}
		}
		return 0;
	}
	// ֱ������
	void runEvent(int aMode, int x, int y) {
		Event *evt;
		evt = findEvent(aMode, x, y);
		if (evt) {
			if (evt->enabled) {
				evt->block.execute();
			}
		} else if (aMode == 5) {
			evt = nearEvent(aMode, x, y);
			if (evt) {
				if (evt->enabled) {
					evt->block.execute();
				}
			}
		}
	}
	// ���������NPCˢ��
	void updateEvent(int aMode) {
		for (int i = 0; i < evtNum; i++) {
			if (event[i].trigMode == aMode) {
				event[i].enabled = true;
			}
		}
	}
	// ��������Ĳ�������������NPC
	void hideNPC(int aMode) {
		for (int i = 0; i < evtNum; i++) {
			if (event[i].trigMode == aMode) {
				event[i].enabled = false;
			}
		}
	}
};

