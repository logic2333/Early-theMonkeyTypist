/******************
猴子随机地敲击键盘上ABCD四个字母，敲击1024次，请问生成了多少个ABAB序列？（ABABAB算两个）
输入模拟次数，模拟敲击，生成随机的字符串
输入统计算法，统计这些字符串中ABAB出现的平均个数，与理论期望比较
三种算法：逐四位统计、归并、折减
折减算法给出第一个AB出现位置的分布情况
比较三种算法的时间效率

结果发现归并算法慢了太多，毕竟要递归。折减算法略快于逐位统计，约快10%
*******************/

#include <iostream>
#include <conio.h>
#include <ctime>
#include <vector>
#include <map>
#include <string>
using namespace std;

const double expec = 1021 / 256.0;		//期望的理论值
unsigned num;							//输入的模拟次数

int random(int low, int high) {			//产生一个[low, high]的随机整数，用于生成字符串
	return low + rand() % (high - low + 1);
}

//用于归并算法，考察左右两个子串分界线附近有无ABAB
unsigned merge_addition(string& left, string& right) {
	unsigned res = 0;
	//分界线附近有如下三种情况会存在ABAB
	//左串后三位ABA，右串首位B
	if (left.substr(left.length() - 3) == "ABA" && right[0] == 'B') res++;
	//左串末位A，右串前三位BAB
	if (left.back() == 'A' && right.substr(0, 3) == "BAB") res++;
	//左串后两位AB，右串前两位AB
	if (left.substr(left.length() - 2) == "AB" && right.substr(0, 2) == "AB") res++;
	return res;
}

//用于归并算法，递归地统计左右子串，类似于二叉树的后序遍历
unsigned count_merge_iter(string& str) {
	if (str.length() == 4) {	//递归终点是子串长度为4
		if (str == "ABAB") return 1;
		else return 0;
	}
	else {
		unsigned res = 0;
		string& left = str.substr(0, str.length() / 2);
		string& right = str.substr(str.length() / 2);
		res += count_merge_iter(left);			//统计左子串
		res += count_merge_iter(right);			//统计右子串
		res += merge_addition(left, right);		//统计分界线（即归并）
		return res;
	}
}

//每一个字符串都是一个样本
class sample_type {
	friend class total_result_type;
private:
	string str;
	unsigned count_1, count_2, count_3;			//三种算法对同一个字符串的统计结果（如果程序正确它们应该相等）
	double runtime_1, runtime_2, runtime_3, genertime;	//三种算法在同一个字符串上各自的运行时间，及生成该字符串消耗的时间
	unsigned firstAB;							//第一个AB在该字符串中出现的位置（记其中A的位置）
public:
	sample_type() {								//构造函数生成一个字符串
		clock_t start, stop;
		start = clock();
		for (unsigned j = 0; j < 1024; j++)
			str += 'A' + random(0, 3);
		stop = clock();
		genertime = double(stop - start) / CLK_TCK;
	}
	//逐四位统计算法
	void count_every_four_bits() {
		count_1 = 0;
		clock_t start, stop;
		start = clock();
		for (unsigned i = 0; i < str.length() - 3; i++) {
			if (str.substr(i, 4) == "ABAB") count_1++;
		}
		stop = clock();
		runtime_1 = double(stop - start) / CLK_TCK;
	}
	
	void count_merge() {						//归并算法
		clock_t start, stop;
		start = clock();
		count_2 = count_merge_iter(str);
		stop = clock();
		runtime_2 = double(stop - start) / CLK_TCK;
	}
	
	void count_reduce() {						//折减算法
		count_3 = 0; firstAB = 1024;			//firstAB赋初值1024，当其值为1024时表明尚未遇到第一个AB
		clock_t start, stop;
		start = clock();
		for (unsigned i = 0; i < str.length() - 3; i++) {
			if (str.substr(i, 2) == "AB") {		//找到AB
				if (firstAB == 1024) firstAB = i;//当firstAB已经不是1024时，表明这次遇到的AB已经不是第一个AB了，不记
				i += 2;							//观察这个AB的后两位
				while (str.substr(i, 2) == "AB") {//如果这个AB的后两位是AB，记一个ABAB，继续观察它的后两位，直到不是AB为止
					count_3++; i += 2;
				}
			}
		}
		stop = clock();
		runtime_3 = double(stop - start) / CLK_TCK;
	}
};

vector<sample_type> samples;

//将本实验所有结果整合为一个类，对所有字符串进行总体统计
class total_result_type {
private:
	//各算法对num个字符串统计消耗的总时间，生成num个字符串消耗的总时间
	double runtime_1, runtime_2, runtime_3, genertime;
	//各算法统计的num个字符串中ABAB的平均个数（如果程序正确它们应该相等）
	double aver_count_1, aver_count_2, aver_count_3;
	map<unsigned, unsigned> firstAB;		//<位置，次数> 0-1020每一个位置上都有可能出现第一个AB，位置号和出现第一个AB的次数一一对应
	char mode;								//所采用的算法
public:
	total_result_type() {}
	//c代表运行的算法种类，对于折减算法要额外统计第一个AB出现位置的分布
	total_result_type(char& c) {
		mode = c;
		switch (mode) {
		case '1': {
			for (auto i : samples) {
				runtime_1 += i.runtime_1; aver_count_1 += i.count_1;
				genertime += i.genertime;
			}
			break;
		}
		case '2': {
			for (auto i : samples) {
				runtime_2 += i.runtime_2; aver_count_2 += i.count_2;
				genertime += i.genertime;
			}
			break;
		}
		case '3': {
			for (auto i : samples) {
				runtime_3 += i.runtime_3; aver_count_3 += i.count_3;
				genertime += i.genertime;
				firstAB[i.firstAB]++;	//统计第一个AB在各个位置出现的频数
			}
			break;
		}
		}
		aver_count_1 /= double(num); aver_count_2 /= double(num); aver_count_3 /= double(num);
	}
	void print() {
		cout << endl;
		cout << "生成" << num << "个字符串总共耗时" << genertime << 's' << endl;
		cout << "统计结果：平均每个字符串中含"; 
		switch (mode)
		{
		case '1': {
			cout << aver_count_1 << "个ABAB；理论值：" << expec << endl;
			cout << "逐四位统计算法耗时" << runtime_1 << 's' << endl; 
			break; 
		}
		case '2': { 
			cout << aver_count_2 << "个ABAB；理论值：" << expec << endl;
			cout << "归并算法耗时" << runtime_2 << 's' << endl; 
			break;
		}
		case '3': {
			cout << aver_count_3 << "个ABAB；理论值：" << expec << endl;
			cout << "折减算法耗时" << runtime_3 << 's' << endl;
			cout << "按a查看AB首次出现位置的分布情况，按其他键返回...";
			if (_getche() == 'a') {
				cout << endl;
				for (auto i : firstAB)
					cout << i.first << " 频数 = " << i.second << endl;
			}
			else cout << endl;
			break;
		}
		}
		cout << endl;
	}
};

int main()
{
	srand(time(0));				//随机数产生器初始化
	cout << "请输入模拟次数："; cin >> num;
	for (unsigned i = 0; i < num; i++)	//生成num个样本，放入一个vector中
		samples.emplace_back();
	cout << "字符串产生完毕！" << endl << endl;
	while (true) {
		cout << "请输入统计算法（1 - 逐四位统计  2 - 归并  3 - 折减  其他 - 退出程序）：";
		char mode = _getche(); cout << endl;
		cout << "统计中...";
		switch (mode) {
			//以下循环不能用迭代器！！！
		case '1':{
			for (unsigned i = 0; i < num; i++)
				samples[i].count_every_four_bits();
			break;
		}
		case '2':{
			for (unsigned i = 0; i < num; i++)
				samples[i].count_merge();
			break;
		}
		case '3':{
			for (unsigned i = 0; i < num; i++)
				samples[i].count_reduce();
			break;
		}
		default:
			exit(0);
		}
		total_result_type total_result(mode);
		cout << "统计结束！" << endl;
		total_result.print();
	}
	return 0;
}
