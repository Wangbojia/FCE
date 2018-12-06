#include <iostream>
#include <vector>
#include <algorithm>


using namespace std;
//set num of task i=1-10,num of core K=3 
int NUM = 10;
//Tikl denote the exe time of task i on the k-th core locally
int Ti1l[10] = { 9,8,6,7,5,7,8,6,5,7 };
int Ti2l[10] = { 7,6,5,5,4,6,5,4,3,4 };
int Ti3l[10] = { 5,5,4,3,2,4,3,2,2,2 };
//minimum local exe time for task vi
int Ti_min[10];
//remote exe time for task i
int Ti_re[10];
//used to mark which core does task i use, local:1,2,3;cloud:0
int Ti_core[10];
//computation cost
float w[10];
//priority of task i
float prior[10];
//the exe order of tasks
int order[10];


vector<vector<int> > graph{ 
	{ 2,3,4,5,6 },
	{ 8,9 },
	{ 7 },
	{ 8,9 },
	{ 9 },
	{ 8 },
	{ 10 },
	{ 10 },
	{ 10 },
	{} 
};

vector<vector<int> > pred{ {},{1},{1},{1},{1},{1},{3},{2,4,6},{2,4,5},{7,8,9} };

//set const val Ts Tc Tr for each task
const int Ts = 3;
const int Tc = 1;
const int Tr = 1;

//ready time for tasks
vector<int> RT_l(10, -1);
vector<int> RT_c(10, -1);
vector<int> RT_ws(10, -1);
vector<int> RT_wr(10, -1);

//finish time for tasks
vector<int> FT_l(10, -1); //finish time on local core
vector<int> FT_c(10, -1); //ft on cloud
vector<int> FT_ws(10, -1); //finish time on wireless sending channel
vector<int> FT_wr(10, -1); //ft on wireless receiving channel



void Print_vec(int a[],int length) {
	for (int i = 0; i < length; i++)
		cout << a[i]<<" ";
	cout << endl;
}
void Print_vec(float a[], int length) {
	for (int i = 0; i < length; i++)
		cout << a[i] << " ";
	cout << endl;
}

void printv(vector<int> a) {
	for (int i = 0; i < a.size(); i++)
		cout << a[i] << " ";
	cout << endl;
}

void printv(vector<float> a) {
	for (int i = 0; i < a.size(); i++)
		cout << a[i] << " ";
	cout << endl;
}

void Primary_assign() {
	//calculate the minimum local exe time Ti_l_min	
	for (int i = 0; i < 10; i++) {
		int mint;
		if (Ti1l[i] > Ti2l[i]) {
			mint = Ti2l[i];
			Ti_core[i] = 2;
		}
		else
		{
			mint = Ti1l[i];
			Ti_core[i] = 1;
		}
		if (Ti3l[i] < mint) {
			mint = Ti3l[i];
			Ti_core[i] = 3;
		}
		Ti_min[i] = mint;
	}
	//calculate the estimated remote exe time
	for (int i = 0; i < 10; i++)
		Ti_re[i] = Ts + Tc + Tr;
	//assign task i to remote or local  0:cloud 1,2,3:local core
	for (int i = 0; i < 10; i++)
		Ti_core[i] = Ti_re[i] < Ti_min[i] ? 0 : Ti_core[i]; //each ele indicates which core generates the minimum time for task i
}

void Task_prior() {
	for (int i = 0; i < 10; i++) {
		if (Ti_core[i] == 0)
			w[i] = Ti_re[i];
		else
			w[i] = (Ti1l[i] + Ti2l[i] + Ti3l[i]) / 3.0;
	}
	prior[9] = w[9];
	for (int i = 8; i >=0;i--) {
		vector<int> tempvec;
		float maxim = 0;
		tempvec = graph[i];
		for (int j = 0; j < tempvec.size(); j++) {
			if (maxim < prior[tempvec[j]-1]) {
				maxim = prior[tempvec[j]-1];
			}
		}		
		prior[i] = w[i] + maxim; //bigger = priorer
	}
}

int Get_max(int l[], int len) {
	int res = 0;
	for (int i = 0; i < len; i++) {
		if (l[i] > l[res]) {
			res = i;
		}
	}
	return res;
}





bool isempty(vector<bool> a) {
	for (int i = 0; i < a.size(); i++) {
		if (a[i] == false)
			return false;
	}
	return true;
}


void Exe_unit_select() {

	vector<int> RC(6, 0); //the available time for 3 local cores, wireless send ,cloud compute,wireless receive

	//find the unit with the highest priority recursively
	vector<bool> usedstatus(10, false); // 0:unused,1:already used
	int thisorder = 0;
	while (isempty(usedstatus) == false) {	
		int maxindex;
		int tempmax=-1;
		int curtask;
		thisorder += 1;
		for (int i = 0; i < 10; i++) {
			if (usedstatus[i] == false) {
				if (prior[i] > tempmax) {
					tempmax = prior[i];
					maxindex = i;
				}
			}
			else {
				continue;
			}
		}
		curtask = maxindex; //the task with the highest current priority
		order[curtask] = thisorder; // record the order of execution
		usedstatus[maxindex] = true; //marked used;
		int c = curtask;
		//now let's play with curtask
		if (Ti_core[c] == 0)
		//means it's a cloud task
		{
			FT_l[c] = 0;
			if (order[c] == 1) 
			{
				RT_ws[c] = 0; //no ready time for the first task 
				FT_ws[c] = RT_ws[c] + Ts;
				RC[3] = FT_ws[c]; // available time for wireless send for next task
				RT_c[c] = FT_ws[c];
				FT_c[c] = RT_c[c] + Tc;
				RC[4] = FT_c[c]; //available time for cloud computing for next task
				RT_wr[c] = FT_c[c];
				FT_wr[c] = FT_c[c] + Tr;
				RC[5] = FT_wr[c]; //available time for wireless receive for next task
			}
			else 
			{
				vector<int> predvec = pred[c]; //the set of immediate predecessors of the current task
				int totalmax = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curmax = max(FT_l[predvec[m] - 1], FT_ws[predvec[m] - 1]);
					totalmax = max(totalmax, curmax);
				}
				RT_ws[c] = totalmax;
				FT_ws[c] = RT_ws[c] + Ts;

				int maxftc=0;
				for (int m = 0; m < predvec.size(); m++) {
					int curftc = FT_c[predvec[m] - 1];
					maxftc = max(maxftc, curftc);
				}
				RT_c[c] = max(FT_ws[c], maxftc);
				FT_c[c] = RT_c[c] + Tc;
				FT_wr[c] = FT_c[c] + Tr;
			}
		}
		else 
		//it can be exe on either local or cloud
		{
			if (order[c] == 1)//executed first
			{
				//choose the fastest core directly(cloud can be ignored because of the prior assignment)
				RT_l[c] = 0; RT_ws[c] = 0;
				FT_l[c] = min(min(Ti1l[c], Ti2l[c]), Ti3l[c]);
				FT_c[c] = 0;
				FT_ws[c] = 0;
				FT_wr[c] = 0;
			}
			else
			{	//find the max finish time of immediate predessors of c
				vector<int> predvec = pred[c]; //the set of immediate predecessors of the current task
				int totalmax = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curmax = max(FT_l[predvec[m] - 1], FT_wr[predvec[m] - 1]);
					totalmax = max(totalmax, curmax);
				}
				RT_l[c] = totalmax;

				//calculate the finish time if the task is excuted on cloud
				int totalmax2 = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curmax2 = max(FT_l[predvec[m] - 1], FT_ws[predvec[m] - 1]);
					totalmax2 = max(totalmax2, curmax2);
				}
				int temp_RT_ws = max(totalmax2,RC[3]); // available time vs ready time
				int temp_FT_ws = temp_RT_ws + Ts;
				//Max finish time on cloud of predecessors
				int maxftc = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curftc = FT_c[predvec[m] - 1];
					maxftc = max(maxftc, curftc);
				}
				int temp_RT_c = max(temp_FT_ws, maxftc); //ready time for cloud is the max of (finishtime wireless send, finish time cloud of pred)

				//modified
				temp_RT_c = max(RC[4], temp_RT_c);

				int temp_FT_c = temp_RT_c + Tc;

				//modified
				int temp_RT_wr = max(RC[5],temp_FT_c);

				int temp_FT_wr = temp_FT_c + Tr;  //the possible finish time if this task is executed on cloud

				//estimate the finish time on each core and compare to that on cloud
				int sum1 = max(RT_l[c], RC[0]) + Ti1l[c]; // compare the ready time for current task and the ready time for a core to execute, get the finish time
				int sum2 = max(RT_l[c], RC[1]) + Ti2l[c];
				int sum3 = max(RT_l[c], RC[2]) + Ti3l[c];

				int Ftime[4] = { temp_FT_wr,sum1,sum2,sum3 };
				int minft = Ftime[0];
				int minindex = 0;
				for (int n = 0; n < 4; n++) {
					if (Ftime[n] < minft) {
						minft = Ftime[n];
						minindex = n;
					}
				}
				Ti_core[c] = minindex;
				FT_l[c] = Ftime[minindex];
				if (Ti_core[c] == 0) {
					RC[3] = temp_FT_ws; // think twice!!!!!!!!!!!!!!!!!!!!!!
					FT_l[c] = 0;
					FT_c[c] = temp_FT_c;
					RC[4] = FT_c[c];
					RT_wr[c] = FT_c[c];
					FT_ws[c] = temp_FT_ws;
					FT_wr[c] = temp_FT_wr;
					RC[5] = FT_wr[c];
					RT_ws[c] = temp_RT_ws;
					RT_c[c] = temp_RT_c;
				}
				else if (Ti_core[c] == 1) {
					RC[0] = sum1;
					FT_l[c] = RC[0];
				}
				else if (Ti_core[c] == 2) {
					RC[1] = sum2;
					FT_l[c] = RC[1];
				}
				else if (Ti_core[c] == 3) {
					RC[2] = sum3;
					FT_l[c] = RC[2];
				}

				if (Ti_core[c] != 0) {
					FT_c[c] = 0;
					FT_ws[c] = 0;
					FT_wr[c] = 0;
				}
			}
		}
	}
}

// Si denote the sequence of tasks excuted on the i'th core 0:cloud
vector<vector<int>> S(4);
void Orig_sched() {

	for (int i = 0; i < 10; i++) {
		int index = Ti_core[i];
		S[index].push_back(i);  //S[0]={1,2} means task2 and task3 exe on cloud
	}
}

//power consumption of cloud and each core
vector<float> PC{ 0.5,1,2,4 };
float Energy_consumption(int core[]) {
	float Etotal = 0;
	for (int i = 0; i < 10; i++) {
		int exe = core[i];
		int Eki;
		if (exe == 1)
			Eki = PC[1] * Ti1l[i];
		else if (exe == 2)
			Eki = PC[2] * Ti2l[i];
		else if (exe == 3)
			Eki = PC[3] * Ti3l[i];
		else if (exe == 0)
			Eki = PC[0] * Ts;
		Etotal += Eki;
	}
	return Etotal;

}




//return time and E
vector<float> Ker_alg(int N, int K)  
//try executing task[N] on Kth core/cloud
//好像不用全部return，记录下N，K就行,执行全部之后用NK求 RT，FT，CORE
//return the new energy consumption and application completion time for comparison(and S?)

{
	vector<int> ready1(10);
	vector<int> ready2(10, -1);
	cout << endl;
	cout << "Runing Ker_org. I'm taking N=" << N << " ,K=" << K << endl;
	//used to store the execution core
	int core[10];
	copy(begin(Ti_core), end(Ti_core), begin(core));
	core[N] = K;
	//cout << "the location of execution is" << endl;
	//Print_vec(core,10);
	//get Sk_new
	vector<vector<int>> Snew = S;
	//erase i from the original S[k] list
	for(int i = 0; i < 4; i++){
		for (int j = 0; j < Snew[i].size(); j++) {
			if (Snew[i][j] == N) {
				Snew[i].erase(Snew[i].begin()+j);
			}
		}
	}
	//insert i to the new S[k], use priority in S[k] to determine location
	Snew[K].push_back(N);
	for (int i = Snew[K].size() - 1; i > 0; i--) {
		if (prior[Snew[K][i]] > prior[Snew[K][i - 1]])
			swap(Snew[K][i], Snew[K][i - 1]);
	}
	//now we have the new sequence sets Snew

	//cout << "new sequence initialized" << endl;


	for (int i = 0; i < 10; i++) {
		ready1[i] = pred[i].size();
		if (ready1[i] == 0)
			ready2[i] = 0;
	}
	//cout << "original ready1" << endl;
	//printv(ready1);
	vector<int> stack;
	vector<int> stackstatus(10, 0); //denote if a task is in the stack; 0:not in; 1:in
	//initialize the stack by pushing task i into it
	for (int i = 0; i < 10; i++) {
		if (ready1[i] == 0 && ready2[i] == 0) {
			stack.push_back(i);
			stackstatus[i] = 1;
		}
	}

	vector<int> schedstatus(10, 0);//schedule status for all tasks, 0 means unscheduled, 1 means scheduled

	vector<int> RC(6, 0); //the available time for 3 cores, wireless sending, cloud computing, wireless receiving.

	//ready time for tasks
	vector<int> RTl(10, -1);
	vector<int> RTc(10, -1);
	vector<int> RTws(10, -1);
	vector<int> RTwr(10, -1);

	//finish time for tasks
	vector<int> FTl(10, -1); //finish time on local core
	vector<int> FTc(10, -1); //ft on cloud
	vector<int> FTws(10, -1); //finish time on wireless sending channel
	vector<int> FTwr(10, -1); //ft on wireless receiving channel

	

	//repeat the steps until no element in stack
	while (stack.size() != 0) {
		//pop a task vi from the stack
		int cur = stack.back();
		//cout << "task " << cur << " exe on core" << core[cur] << endl;
		schedstatus[cur] = 1; //marked scheduled
		//cout << "the stack before pop" << endl;
		//printv(stack);
		stack.pop_back();  //delete the last element
		//cout << "the stack is now" << endl;
		//printv(stack);
		if (core[cur] == 0)
			//means it's a cloud task
		{
			FTl[cur] = 0;
			if (order[cur] == 1)
			{
				RTws[cur] = 0; //no ready time for the first task
				FTws[cur] = RTws[cur] + Ts;
				RC[3] = FTws[cur];
				RTc[cur] = FTws[cur];
				FTc[cur] = RTc[cur] + Tc;
				RC[4] = FTc[cur];
				RTwr[cur] = FTc[cur];
				FTwr[cur] = FTc[cur] + Tr;
				RC[5] = FTwr[cur];
				//cout << "available time after 1st task" << endl;
				//printv(RC);
				
			}
			else
			{
				vector<int> predvec = pred[cur]; //the set of immediate predecessors of the current task
				int totalmax = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curmax = max(FTl[predvec[m] - 1], FTws[predvec[m] - 1]);
					totalmax = max(totalmax, curmax);
				}
				RTws[cur] = max(totalmax,RC[3]);

				FTws[cur] = RTws[cur] + Ts;

				RC[3] = FTws[cur];
				int maxftc = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curftc = FTc[predvec[m] - 1];
					maxftc = max(maxftc, curftc);
				}
				RTc[cur] = max(FTws[cur], maxftc);
				//cout << "finish time for wireless send is " << FTws[cur] << endl;
				//cout << "current readytime for cloud is " << RTc[cur]<<endl;
				//cout << "available time for cloud is " << RC[4]<< endl;
				RTc[cur] = max(RTc[cur], RC[4]);
				//cout << "the final ready time is" << RTc[cur] << endl;
				FTc[cur] = RTc[cur] + Tc;
				RC[4] = FTc[cur];
				RTwr[cur] = max(FTc[cur], RC[5]);
				FTwr[cur] = FTc[cur] + Tr;				
				RC[5] = FTwr[cur];

				//cout << "new available time" << endl;
				//printv(RC);
			}
		}
		else
			//it is executed on local core
		{
			if (order[cur] == 1)//executed first
			{
				//choose the fastest core directly(cloud can be ignored because of the prior assignment)
				RTl[cur] = 0; RTws[cur] = 0;
				vector<int> te{ Ti1l[cur], Ti2l[cur], Ti3l[cur] };
				FTl[cur] = te[core[cur] - 1];
				RC[core[cur]-1] = FTl[cur];
				FTc[cur] = 0;
				FTws[cur] = 0;
				FTwr[cur] = 0;
				//cout << "available time after 1st task" << endl;
				//printv(RC);
			}
			else
			{	//find the max finish time of immediate predessors of c
				vector<int> predvec = pred[cur]; //the set of immediate predecessors of the current task
				int totalmax = 0;
				for (int m = 0; m < predvec.size(); m++) {
					int curmax = max(FTl[predvec[m] - 1], FTwr[predvec[m] - 1]);
					totalmax = max(totalmax, curmax);
				}
				RTl[cur] = max(totalmax,RC[core[cur]-1]);


				if (core[cur] == 1) {
					FTl[cur] = RTl[cur] + Ti1l[cur];
					RC[0] = FTl[cur];

				}
				else if (core[cur] == 2) {
					FTl[cur] = RTl[cur] + Ti2l[cur];
					RC[1] = FTl[cur];
				}
				else if (core[cur] == 3) {
					FTl[cur] = RTl[cur] + Ti3l[cur];
					RC[2] = FTl[cur];
				}

				if (core[cur] != 0) {
					FT_c[cur] = 0;
					FT_ws[cur] = 0;
					FT_wr[cur] = 0;
				}
				//cout << "new available time" << endl;
				//printv(RC);
			}
		}

		vector<int> succ = graph[cur];
		for (int i = 0; i < succ.size(); i++) {
			int asucc=succ[i]-1;
			ready1[asucc] -= 1;
			
		}
		//cout << "updated ready1" << endl;
		//printv(ready1);
		//for all predecessors(i) of cur, if status i is scheduled, ready2[cur]=0;
		//cout << "schedule status" << endl;
		//printv(schedstatus);

		//modified, rewiritten first find S[k] which contains vi, then check if all tasks before vi have been scheduled
		//m denotes the #task
		for (int m = 0; m < 10; m++) {
			int flag = 0;
			vector<int > a= S[core[m]]; //a=S[k] contains m
			for (int n = 0; n < a.size(); n++) {
				if (prior[a[n]] > prior[m]) {
					if (schedstatus[a[n]] == 0)//predecessor have not been scheduled
						flag = 1;
				}
			}
			if (flag == 0)
				ready2[m] = 0;
		}


		//cout << "updated ready2" << endl;
		//printv(ready2);
		//push all the new tasks vj with ready1j=0 and ready2j=0
		for (int i = 0; i < 10; i++) {
			if (ready1[i] == 0 && ready2[i] == 0)
				if (schedstatus[i] == 0&&stackstatus[i]==0) //have not been scheduled yet
				{
					stack.push_back(i);
					stackstatus[i] = 1;
					//cout << "push task " << i << " into the stack" << endl;
					
				}
		}
	}
	cout << "ready time ws" << endl;
	printv(RTws);
	cout << "finish time wireless send" << endl;
	printv(FTws);
	cout << "ready time c" << endl;
	printv(RTc);
	cout << "finish time cloud" << endl;
	printv(FTc);	
	cout << "ready time wr" << endl;
	printv(RTwr);
	cout << "Finish time wireless receive" << endl;
	printv(FTwr);	
	cout << "readytime l" << endl;
	printv(RTl);
	cout << "Finish time local" << endl;
	printv(FTl);
	cout << "execute core" << endl;
	Print_vec(core,10);
	
	float max1 = *max_element(FTl.begin(),FTl.end());
	float max2 = *max_element(FTwr.begin(), FTwr.end());

	float finish_time = max(max1, max2);


	float engergy_cost = Energy_consumption(core);
	vector<float> res{ finish_time,engergy_cost };
	return res;
}


vector<float> Outerloop_once(float Eorg = Energy_consumption(Ti_core), float Torg = max(FT_l[9], FT_wr[9])) 
//go through all choices once
{
	//float Eorg = Energy_consumption(Ti_core);
	//float Torg = max(FT_l[9], FT_wr[9]);
	float delE = 0;
	int besti = -1;
	int bestk = -1;
	//go through all choices
	for(int i=0;i<10;i++)
		for (int k = 0; k < 4; k++) { 
			if (Ti_core[i] == k||Ti_core[i]==0) //skip the same schedule and cloud task don't reschedule as the paper says.
				continue;
			vector<float> thisres = Ker_alg(i, k);
			if(thisres[0]<=Torg)
				if (thisres[1] < Eorg) {
					float thisdelE = Eorg - thisres[1];
					if (thisdelE > delE) {
						delE = thisdelE;
						besti = i;
						bestk = k;
					}
				}
		}
	if (besti == -1)//means the time cannot be decreased,then we select the result in the largest ratio of E reduction to the increase of T
	{
		cout << "running time cannot be deducted" << endl;
		float largest_ratio = 0.0;
		for (int i = 0; i < 10; i++)
			for (int k = 0; k < 4; k++) {
				if (Ti_core[i] == k || Ti_core[i] == 0) //skip the same schedule and cloud task don't reschedule as the paper says.
					continue;
				vector<float> thisres = Ker_alg(i, k);
					if (thisres[1] < Eorg) {
						float thisdelE = Eorg - thisres[1];
						float T_incres = thisres[0] - Torg;
						float thisratio = thisdelE / T_incres;
						if (thisratio > largest_ratio) {
							largest_ratio=thisratio;
							besti = i;
							bestk = k;
						}
					}
			}
	}
	//time and energy consumption
	vector<float>  te = Ker_alg(besti, bestk); 
	cout << "besti=" << besti << " bestk=" << bestk << endl;
	
	vector<float> a = { float(besti),float(bestk),te[0],te[1] };

	if(besti!=-1)
		Ti_core[besti] = bestk; //update the execute location for task(besti)

	//cout << "exe cores" << endl;
	//Print_vec(Ti_core, 10);
	
	return a;
}

vector<float> Outerloop_it() {
	vector<float> res = Outerloop_once();
	float maxtime = 27.0;
	float time = res[2];
	float energy = res[3];
	int count = 0;
	cout << "current core schedule is" << endl;
	Print_vec(Ti_core, 10);
	while (true)
	{
		cout << "I'm in the " << count << " while loop" << endl;
		count += 1;
		cout << "I'm going to the outerloop" << endl;
		vector<float> newres = Outerloop_once(energy, time); //go through all choices and find transfer task i to core k, get finishtime and engergy consump
		cout << "time and cost after run outloop once" << endl;
		printv(newres);
		if (newres[0] == -1) //cannot find a better solution
		{
			cout << "the result cannot be improved QAQ" << endl;
			break;
		}
		if (float(newres[2]) > maxtime || float(newres[3]) >= energy) //////////////////
		{
			cout << "I can't find a better choice" << endl;
			break;
		}
		time = newres[2];
		energy = newres[3];
		cout << "time cost is " << time << " engergy cost is " << energy << endl;
	}
	vector<float> finalres = { time,energy };
	return finalres;
}


void Printtime() {
	Primary_assign();
	Task_prior();
	cout << "task prior" << endl;;
	Print_vec(prior, 10);

	Exe_unit_select();
	cout << "exe order" << endl;
	
	cout << "Readytime_local" << endl;
	printv(RT_l);
	cout << "Readytime_cloud" << endl;
	printv(RT_c);
	cout << "Readytime_wirelesssend" << endl;
	printv(RT_ws);
	cout << "Readytime_wireless_receive" << endl;
	printv(RT_wr);
	cout << "Finish time local" << endl;
	printv(FT_l);
	cout << "Finish time cloud" << endl;
	printv(FT_c);
	cout << "Finish time wireless send" << endl;
	printv(FT_ws);
	cout << "Finish time wireless receive" << endl;
	printv(FT_wr);

	cout << "execute core" << endl;
	Print_vec(Ti_core, 10);
}
int main() {
	Printtime();
	Orig_sched();
	printv(S[0]);
	printv(S[1]);
	printv(S[2]);
	printv(S[3]);
	float E1 = Energy_consumption(Ti_core);
	cout <<"the engergy cost after initial sched is " <<E1<<endl;

	//vector<float> temp = Ker_alg(3, 2);
	//cout << "after kernal alg" << endl;
	//printv(temp);

	vector<float> time_energy = Outerloop_it();
	cout << "final time and energy cost is" << endl;
	printv(time_energy);
	cout << "final arrangement" << endl;
	Print_vec(Ti_core,10);

	return 0;
}
