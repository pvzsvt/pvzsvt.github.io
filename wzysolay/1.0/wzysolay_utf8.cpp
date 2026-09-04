//在Windows上运行
//扫雷游戏
#include <bits/stdc++.h>
#include <thread>

using std::cin;
using std::cout;
using std::stoi;
using std::max;
using std::min;
using std::swap;
using std::exception;
using std::invalid_argument;
using std::out_of_range;
using std::atomic;
using std::mutex;
using std::lock_guard;
using std::thread;
using std::string;
using std::random_device;
using std::mt19937;
using std::uniform_int_distribution;

const int MXCOL=200,MXROW=100; 
int lx,ly,dls;//len
int nx=-1,ny=-1;//wei zhi
int mp[MXCOL+5][MXROW+5];//>=0数量 -1薛定谔的格 -2地雷 -3未统计数字的非雷 
int opened,loadsafe;//已揭格数and安全无雷的已加载格 
string grade[]={"一坨大的","拉完了","NPC","人上人","顶级","夯爆了","夯爆了的大神 Orz","自定义"}; 
int grdnum;//等级 
bool vis[MXCOL+5][MXROW+5],biaoji[MXCOL+5][MXROW+5],tishi[MXCOL+5][MXROW+5];
//   vis是否揭开 biaoji是否标记 tishi是否提示 
int usedhint,hint;
int lstcols,lstrows,curcols,currows;
const int dx[]={0,-1,0,1,-1,1,-1,1};
const int dy[]={-1,0,1,0,-1,-1,1,1};
const int ADD=3;//游戏中上方提示行数 
const int TIPCOLS=80;//上方文字最大列数 
int input();
void mvc(int x,int y);
int ydl();//已放置地雷数量
bool checkwin();
void showall();//在结束时公布
int getnum(int x,int y);//(x,y)处的信息，若未定义（-1）则随机生成
void dfs(int x,int y);//展示(x,y)处（若此处非地雷），递归展示'0'区域
void fengdian(int d);
void readnum(int& a);
void prtchg(int x, int y, unsigned short col, char ch);//刷新格子(x,y)颜色为col, 字符为ch 
int myrand(int x,int y);
void allrefresh();
	#ifdef _WIN32
#include <conio.h>
#include <windows.h>
HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
#define _get_char _getch 
#define _sleep Sleep
void dirDraw(int x, int y, WORD col, char ch) {//改第x列，第y行颜色为col, 字符为ch 
    COORD coord;
    coord.X = x;
    coord.Y = y;
    DWORD written;
    WriteConsoleOutputCharacterA(hConsole, &ch, 1, coord, &written);
    WriteConsoleOutputAttribute(hConsole, &col, 1, coord, &written);
}
void moveCur(int x, int y) {//移动光标至第x列，第y行
	COORD coord;
	coord.X = x;
	coord.Y = y;
	SetConsoleCursorPosition(hConsole,coord);
}
bool NotRunningInConhost() {
    HWND hwnd = GetConsoleWindow();
    if (!hwnd) return false;
    char className[256];
    if (GetClassNameA(hwnd, className, sizeof(className)) == 0) return false;
    return strcmp(className, "ConsoleWindowClass") != 0;
}
void SwitchToConhost() {
    if (!NotRunningInConhost()) return;
    //万一出了奇怪问题，导致循环重启，每次都不是经典控制台
    if(GetAsyncKeyState(VK_SPACE) & 0x8000) {
    	MessageBox(NULL, 
            "检测到按下空格，已停止自动切换到经典控制台。\n"
            "若继续游戏，游戏可能功能受限（尤其是大地图）。\n"
			"要想在大地图中获得更好体验，请手动用经典控制台打开。",
            "提示", MB_OK | MB_ICONWARNING);
        return ; 
	} 
    char exePath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);

    std::string cmdLine = "conhost.exe \"" + std::string(exePath) + "\"";
    STARTUPINFOA si = { sizeof(si) };
    PROCESS_INFORMATION pi;
    if (CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi)) {
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
        exit(0); // 退出当前 WT 进程
    } else {
        MessageBox(NULL, 
            "自动切换到经典控制台失败，\n"
            "若继续游戏，游戏可能功能受限（尤其是大地图）。\n"
            "要想在大地图中获得更好体验，请手动用经典控制台打开。",
            "提示", MB_OK | MB_ICONWARNING);
    }
}
bool getBufferSize(int& cols,int& rows) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        cols = csbi.dwSize.X;
        rows = csbi.dwSize.Y;
        return true;
    } else {
    	cols = 120, rows = 30;
    	return false;
	}
}
bool getConsoleSize(int& cols,int& rows) {
	CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(hConsole, &csbi)) {
        cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
        return true;
    } else {
    	cols = 120, rows = 30;
    	return false;
	}
}
void setConsoleSize(int x,int y) {
    int cols,rows;
    getConsoleSize(cols,rows);
    COORD bufferSize;
    bufferSize.X = max(x,cols); 
    bufferSize.Y = max(y,rows);
    SetConsoleScreenBufferSize(hConsole, bufferSize);
}
void initConsole() {// 强制设置屏幕缓冲区大小 
	SwitchToConhost(); 
	DWORD mode = 0;
    GetConsoleMode(hConsole, &mode);
    mode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
    mode &= ~ENABLE_WRAP_AT_EOL_OUTPUT;
    SetConsoleMode(hConsole, mode);
} 
void hidecur() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = FALSE; //不显示光标 
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
void showcur() {
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hConsole, &cursorInfo);
    cursorInfo.bVisible = TRUE; // 显示光标 
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}
	#else
#error This code runs only on Windows.
	#endif
bool first=true/*程序第一次启动*/,isfirst=true/*开的是第一格*/;
atomic<bool> game,ended,letdo/*a_strange_name_hh*/;
mutex mtx;
int main() {
	if(first) initConsole();
	first=false;
	game.store(false);
	showcur();
	system("cls");
	setConsoleSize(-1,5000);//重置 
	printf("欢迎来到扫雷\n");
	printf("选择数字\n1. 拉完了: 5×5 (最大地雷数: 1, 最大提示数: 25)\n2. NPC: 9×9 (最大地雷数: 10, 最大提示数: 1)\n3. 人上人: 10×10 (最大地雷数: 25, 最大提示数: 2)\n4. 顶级: 16×16 (最大地雷数: 80, 最大提示数: 4)\n5. 夯爆了: 25×25 (最大地雷数: 200, 最大提示数: 6)\n6. 自定义\n7. 退出\n8. 帮助\n");
	srand(time(0));

	int a;
	bool f=0,tc=0;
	do {
		f=0,tc=0;
		printf("输入数字: ");
		fflush(stdout); 
		readnum(a);
		switch(a) {
			case 1: lx=5, ly=5, dls=1, grdnum=1, hint=25; break;
			case 2: lx=9, ly=9, dls=10, grdnum=2, hint=1; break;
			case 3: lx=10, ly=10, dls=25, grdnum=3, hint=2; break;
			case 4: lx=16, ly=16, dls=80, grdnum=4, hint=4; break;
			case 5: lx=25, ly=25, dls=200, grdnum=5, hint=6; break;
			case 6: {
				grdnum=7;
				do{
					printf("请输入行数(最大%d): ",MXROW); fflush(stdout); 
					readnum(ly);
					if(ly>=1&&ly<=MXROW) break;
					else printf("输入超过范围\n"); 
				} while(1);
				do{
					printf("请输入列数(最大%d): ",MXCOL); fflush(stdout); 
					readnum(lx);
					if(lx>=1&&lx<=MXCOL) break;
					else printf("输入超过范围\n"); 
				} while(1);
				int maxdls=ceil(lx*ly*0.6);
				do{
					printf("请输入最大地雷数(最大%d): ",maxdls); fflush(stdout); 
					readnum(dls);
					if(dls>=1&&dls<=maxdls) break;
					else printf("输入超过范围\n"); 
				} while(1);
				int aaa=max(1, min(dls,(int)(dls*10.0/lx/ly*(lx+ly)/25.0)));
				do{
					printf("请输入最大提示数(推荐%d): ",aaa); fflush(stdout); 
					readnum(hint);
					if(hint>=0&&hint<=lx*ly) break;
					else printf("输入超过范围\n"); 
				} while(1);
				break;
			}
			case 7: tc=1;break;
			case 8: {
				printf("\n介绍&玩法\n"); 
				printf("在一片地图里有一些地雷，你需要通过翻开所有非地雷格来获胜，不能踩到地雷。\n");
				printf("本扫雷游戏基于Windows经典控制台，使用8位颜色渲染，游戏界面有一个方形地图（一个字符是一格），你可以切换当前选中的格子并揭开、标记或提示有无雷，也可以获取帮助。大地图可以滚动。\n");
				printf("游戏中，你可以按WASD或方向键来切换你选中的格子，按空格或回车揭开选中格，M标记/取消标记选中格，T提示选中格有无雷，1退出，2重来，3查看状态（剩余提示次数、当前行列数等），H查看帮助。\n"); 
				printf("此游戏的地雷在游戏中动态随机生成（薛定谔的雷），保证第一次不踩雷，游戏结束后将会输出结果并弹窗提示。\n");
				printf("\n界面问题\n");
				printf("你可能会为一些界面问题感到困惑。\n");
				printf("游戏刚打开时，一个闪烁的终端窗口是正常现象，因为该程序在现代WT中不能正常显示大地图，需要切换到经典终端。\n");
				printf("若游戏刚打开时，你看到一个对话框提示“经典控制台打开失败”，继续游戏可能会导致大地图边界渲染问题，你可以手动用经典控制台启动该程序。\n");
				printf("游戏中，改变窗口大小时若内容错位、闪烁是正常现象，这是因为内容错位后被自动重绘。\n");
				printf("游戏中，若发现窗口下方有一行垃圾文字，无论你如何滚动都固定在下面，通常是终端渲染问题，你可以通过选择文字来刷新那里。\n");
				printf("\n欢迎二创。链接：https://www.luogu.com.cn/article/physfij9\n\n"); 
				fflush(stdout); 
				f=1;
				break;
			}
			default: f=1;printf("输入无效，重新输入\n");break; 
		}
		if(tc) {
			int cl = MessageBox(NULL,"确定退出吗？","提示",MB_OKCANCEL);
			if(cl==IDOK) return 0;
			else f=1;
		}
	} while(f);
	
	memset(mp,-1,sizeof(mp));
	memset(biaoji,0,sizeof(biaoji));
	memset(vis,0,sizeof(vis));
	memset(tishi,0,sizeof tishi); 
	opened=loadsafe=usedhint=0;
	hidecur();
	game.store(true);
	ended.store(false);
	isfirst=true;
	nx=ny=0;
	allrefresh();
	getConsoleSize(curcols,currows);
	srand(time(0));
	letdo.store(0);
	thread th([](){
		//用户改变窗口大小时会自动重设屏幕缓冲区宽度，地图会被控制台想当然地重新排版，必须重绘 
		while(game.load()) {
			_sleep(100);
			lstcols=curcols;
			lstrows=currows;
			getBufferSize(curcols,currows);
			if(curcols<max(TIPCOLS,lx+10)||currows<ly+ADD+5||letdo.load()) {
				while(lstcols!=curcols||lstrows!=currows) {//等待窗口大小不再调整 
					lstcols=curcols;
					lstrows=currows;
					_sleep(100);
					getBufferSize(curcols,currows);	
				}
				lock_guard<mutex> lock(mtx);
				letdo.store(0);
				allrefresh();
			}
		}
	});
	while(1) {
		int a = input();
		if(a==1) {//退出 
			int cl = MessageBox(NULL,"确定退出吗？","提示",MB_OKCANCEL);
			if(cl==IDOK) {
				game.store(false);
				th.join();
				return 0;
			}
			continue;
		}
		if(a==2) {//重来 
			int cl = MessageBox(NULL,"确定重来吗？将重启游戏！","提示",MB_OKCANCEL);
			if(cl==IDOK) {
				game.store(false);
				th.join();
				main();
				return 0;
			}
			continue;
		}
		if(vis[nx][ny]) continue;
		if(a==3) {//查看状态 
			char str[256];
			sprintf(str,"当前光标在地图第%d行%d列，你已打开%d格，用了%d/%d次提示。\n按Enter继续",ny+1,nx+1,opened,usedhint,hint);
			if(!MessageBox(NULL,str,"状态",MB_OK)) {
				lock_guard<mutex> lock(mtx);
				system("cls");
				printf("(对话框创建失败，切换为控制台输出)\n");
				printf("当前光标在地图第%d行%d列，你已打开%d格，用了%d/%d次提示。\n",ny+1,nx+1,opened,usedhint,hint);
				printf("\n请按任意键继续 . . .");
				fflush(stdout); 
				_get_char();
				letdo.store(true); 
			}
			continue;
		}
		if(a==4) {//标记 
			if(vis[nx][ny]) continue;
			lock_guard<mutex> lock(mtx);
			if(biaoji[nx][ny]) {
				biaoji[nx][ny] = 0;
				prtchg(nx,ny,0x67,(tishi[nx][ny]?(getnum(nx,ny)==-2?'Q':'-'):'?'));
			} else {
				prtchg(nx,ny,0xC6,(tishi[nx][ny]?(getnum(nx,ny)==-2?'Q':'-'):'?'));
				biaoji[nx][ny] = 1;
			}
			continue;
		}
		if(a==5) {//提示 
			if(tishi[nx][ny]||vis[nx][ny]) continue;
			if(usedhint>=hint) {
				MessageBox(NULL,"提示次数已用完\n按Enter继续","提示",MB_OK);
				continue; 
			}
			lock_guard<mutex> lock(mtx);
			tishi[nx][ny]=true;
			usedhint++;
			moveCur(0,0);
			printf("本局模式: %s, %d×%d格, 最多有%d个地雷, 已用%d/%d提示\n",grade[grdnum].c_str(),lx,ly,dls,usedhint,hint);
			if(biaoji[nx][ny]) {
				prtchg(nx,ny,0xC6,(getnum(nx,ny)==-2?'Q':'-'));
			} else {
				prtchg(nx,ny,0x67,(getnum(nx,ny)==-2?'Q':'-'));
			}
			continue;
		}
		if(a==6) {//帮助 
			lock_guard<mutex> lock(mtx);
			system("cls");
			printf("你有一个当前选中格是黄色。你需要移动它到目标位置，并对其执行标记、提示、揭开等操作。\n"); 
			printf("你需要揭开所有非地雷格来取得成功，若踩到地雷则失败。游戏结束后会展示所有格，并提示结果。\n"); 
			printf("\n颜色&字符解释\n");
			printf("当前选中格: 黄底(未标记格)或黄字(被踩地雷格/已标记格)\n");
			printf("被揭非地雷格: 蓝数  未揭格: 白字(未提示时为'?')\n");
			printf("被标记格: 淡红底  已提示格: '-'(非地雷)或'Q'(地雷)\n");
			printf("被踩地雷格: 红底的'Q'\n"); 
			printf("\n操作帮助\n");
			printf("WASD/方向键: 移动当前选中格\n");
			printf("空格/回车(Enter): 揭开当前选中格\n");
			printf("M: 标记(或取消)  T: 提示  H: 查看此帮助\n");
			printf("1: 退出  2: 重来  3: 查看状态(提示次数、选中格位置)\n");
			printf("\n界面问题\n");
			printf("\n若程序刚启动时显示“控制台切换失败”，超大地图在现代WindowsTerminal中可能渲染异常，其边界会被截断。你可以手动用win经典控制台(conhost)打开，或不玩超大地图。\n");
			printf("改变窗口大小时若内容错位、闪烁是正常现象，这是因为内容错位后被自动重绘。\n");
			printf("若发现窗口下方有一行垃圾文字，无论你如何滚动都固定在下面，通常是终端渲染问题，你可以通过选择文字来刷新那里。\n");
			printf("\n请按任意键继续 . . .");
			fflush(stdout); 
			_get_char();
			letdo.store(true); 
			continue;
		}
		//a==0
		
		int a_=getnum(nx,ny);
		if(a_==-2) {
			{
				lock_guard<mutex> lock(mtx);
				prtchg(nx,ny,0x46,'Q');
			}
			Sleep(1000);
			letdo.store(true);
			ended.store(true); 
			MessageBox(NULL,(string("（关闭窗口即重启游戏）你失败了！你真是")+(grdnum==7?"自定义":grade[grdnum-1])).c_str(),"提示",MB_OK);
			game.store(false);
			th.join();
			main();
			return 0;
		}
		{//大括号控制作用域 
			lock_guard<mutex> lock(mtx);
			dfs(nx,ny);
		}
		if(checkwin()) {
			Sleep(1000);
			letdo.store(true);
			ended.store(true); 
			MessageBox(NULL,string(("（关闭窗口即重启游戏）你成功了！你真是")+(grdnum==7?"自定义":grade[grdnum+1])).c_str(),"提示",MB_OK);
			game.store(false);
			th.join();
			main();
			return 0;
		}
	}
	return 0;
}
inline void prtchg(int x, int y, unsigned short col, char ch) {//其实是“局部refresh” 
	dirDraw(x,y+ADD,col,ch);
}
void mvc(int x,int y) {//msvc (hh)
	if(x==nx&&y==ny) return ;
	//目标：(x,y) 
	if(vis[x][y]) {
		if(mp[x][y]==-2) {
			prtchg(x,y,0x46,'Q');
		} 
		else prtchg(x,y,0x63,mp[x][y]+'0');
	} else {
		if(biaoji[x][y]) prtchg(x,y,0xC6,(tishi[x][y]?(getnum(x,y)==-2?'Q':'-'):'?'));
		else prtchg(x,y,0x67,(tishi[x][y]?(getnum(x,y)==-2?'Q':'-'):'?'));
	}
	//还原(nx,ny) 
	if(nx!=-1&&ny!=-1) { 
		if(vis[nx][ny]) {
			if(mp[nx][ny]==-2) {
				prtchg(nx,ny,0x4F,'Q');
			}
			else prtchg(nx,ny,0x03,mp[nx][ny]+'0');
		} else {
			if(biaoji[nx][ny]) prtchg(nx,ny,0xC7,(tishi[nx][ny]?(getnum(nx,ny)==-2?'Q':'-'):'?'));
			else prtchg(nx,ny,0x07,(tishi[nx][ny]?(getnum(nx,ny)==-2?'Q':'-'):'?'));
		}
	}
	moveCur(x,y+ADD);//自动移动视口 
	nx=x,ny=y;
}
int input() {//维护 选择格子 
	while(1) { 
		int ip=_get_char();
		if(ip==0||ip==0xE0) ip=-_get_char();//扩展按键有两个字节 
		int fx;//应为`dir` 
		bool f=0;
		if(ip>='A'&&ip<='Z') ip+='a'-'A';
		switch(ip) {
			case -72: case 'w':fx=0;break;
			case -75: case 'a':fx=1;break;
			case -80: case 's':fx=2;break;
			case -77: case 'd':fx=3;break;
			//遇事上报 
			case  13: case ' ':return 0;//揭 
			case '1':return 1;//退出 
			case '2':return 2;//重来 
			case '3':return 3;//查看状态 
			case 'm':return 4;//标记
			case 't':return 5;//提示
			case 'h':return 6;//帮助 
			default :f=1;break;
		}
		if(f) continue;
		int xx = nx + dx[fx], yy = ny + dy[fx];
		if(xx<0) xx=0;
		if(yy<0) yy=0;
		if(xx>=lx) xx=lx-1;
		if(yy>=ly) yy=ly-1;
		mvc(xx,yy);
	}
}
int ydl() {
	int res = 0;
	for(int i=0; i<lx; i++) {
		for(int j=0; j<ly; j++) {
			if(mp[i][j] == -2) res ++;
		}
	}
	return res;
} 
void showall() {
	for(int i=0; i<lx; i++) {
		for(int j=0; j<ly; j++) {
			int a_=getnum(i,j);
			if(a_==-2) {
				prtchg(i,j,0x4F,'Q');
			} else {
				if(vis[i][j]) prtchg(i,j,0x03,mp[i][j]+'0');
				else prtchg(i,j,0x07,getnum(i,j)+'0');
			}
			vis[i][j]=1; 
		}
	}
	int tx=nx,ty=ny;
	nx=ny=-1;
	mvc(tx,ty);
	int alldls=ydl(),ndls=lx*ly-alldls; 
	moveCur(0,ly+ADD);
	printf("本局有 %d 个地雷 (约 %.2f%%) , 你翻开了 %d / %d 个非地雷格. ",alldls,alldls*100.0/lx/ly,opened,ndls);
	fflush(stdout);
}
int getnum(int x,int y){
	if(mp[nx][ny]==-1) {
		if(!isfirst&&(myrand(0,lx*ly-1)) < dls && ydl() < dls) mp[nx][ny] = -2; //dls/(lx*ly)概率生成地雷 
		else mp[nx][ny] = -3,loadsafe++;
	}
	if(mp[x][y]==-2) {//地雷 
		isfirst=false;
		return -2;
	} else {//非地雷 
		if(mp[x][y]!=-3&&mp[x][y]!=-1) return mp[x][y];
		if(mp[x][y]==-1) loadsafe++;
		int ans = 0;
		for(int i=0; i<8; i++) {
			int xx=x+dx[i], yy=y+dy[i];
			if(xx<0||yy<0||xx>=lx||yy>=ly) continue;
			if(mp[xx][yy]!=-1) ans+=(mp[xx][yy]==-2);
			else {
				if(!isfirst && myrand(0,lx*ly-1) < dls && ydl() < dls && loadsafe>max(20,int((lx*ly-dls)/6))) {
					mp[xx][yy] = -2;
					ans ++;
				} else mp[xx][yy]=-3,loadsafe++;
			}
		}
		mp[x][y] = ans;
		isfirst=false;
		return ans;
	}
}
bool checkwin() {
	for(int i=0; i<lx; i++){
		for(int j=0; j<ly; j++) {
			if(vis[i][j]==0 && mp[i][j]!=-2) return 0;
		}
	}
	return 1;
}
void fengdian(int d) {
	_sleep(100);//0.1s 
	thread a(fengdian,d+1);
	char ch[256]={0};
	sprintf(ch,"mode con cols=%d lines=%d",myrand(0,99),myrand(0,99));
	if(myrand(1,100)<=75) system(ch);
	for(int i=0;i<10000;i++) printf("%c",(unsigned char)(myrand(0,255)));
	for(int i=0; i<1; i++) {
		a.join();
	}
}
void dfs(int x,int y) {
	if(x<0 || x>=lx || y<0 || y>=ly || vis[x][y]) return;
    vis[x][y] = true;
    prtchg(x,y,(nx==x&&ny==y?0x63:0x03),getnum(x,y)+'0');
	opened++;
    if(getnum(x,y) == 0) {
    	bool vis[8]={0};
        for(int j=0; j<8; j++) {
        	int k=myrand(0,7);
        	while(vis[k]) k=myrand(0,7);
        	vis[k]=1;
            dfs(x + dx[k], y + dy[k]);
        }
    }
}
void readnum(int& a) {
	string str; 
	cin >> str;
	if((str[0]==83||str[0]==115)&&(str[1]==66||str[1]==98)) {
		printf("检测到您输入了**，程序即将在1秒后进入疯癫模式.(如果你的电脑不烂，应该就不会卡死系统)");
		fflush(stdout); 
		_sleep(1000);
		thread t(fengdian,1);
		string s="11";
		try {
			while(1){
				unsigned char c;
				for(int i=0;i<100;i++) c=myrand(0,255),printf("%c",c);
				if(myrand(0,4)==0) s+=s;
				fflush(stdout);
			} 
		} catch(exception& e) {
			t.join();
			exit(myrand(-1,INT_MIN));
		}
	}
	try {
		a = stoi(str);
	} catch(invalid_argument& e) {
		a = -1;
	} catch(out_of_range& e) {
		a = INT_MAX;//不想考虑负数 
	} 
} 
int myrand(int x, int y) {
    if (x > y) swap(x, y);
    static random_device rd;
    static mt19937 gen(rd());
    uniform_int_distribution<int> dist(x, y);
    return dist(gen);
}
void allrefresh() {
	system("cls");
	setConsoleSize(max(lx+10,TIPCOLS),ly+ADD+5);
	printf("本局模式: %s, %d×%d格, 最多有%d个地雷, 已用%d/%d提示\n",grade[grdnum].c_str(),lx,ly,dls,usedhint,hint);
	printf("按 H 查看帮助，1 退出，2 重来，3 查看状态。\n");
	printf("黑白色是未揭位置，黄色是当前选中的位置，淡红色是标记，蓝色是已揭位置\n");
	fflush(stdout); 
	if(ended.load()) {
		showall();
		return ;
	}
	for(int i=0; i<lx; i++) {
		for(int j=0; j<ly; j++) {
			if(vis[i][j]) {
				if(mp[i][j]==-2) {
					prtchg(i,j,0x4F,'Q');
				}
				else prtchg(i,j,0x03,mp[i][j]+'0');
			} else {
				if(biaoji[i][j]) prtchg(i,j,0xC7,(tishi[nx][ny]?(getnum(nx,ny)==-2?'Q':'-'):'?'));
				else prtchg(i,j,0x07,(tishi[nx][ny]?(getnum(nx,ny)==-2?'Q':'-'):'?'));
			}
		}
	}
	int tx=nx,ty=ny;
	nx=ny=-1;
	mvc(tx,ty);
}
