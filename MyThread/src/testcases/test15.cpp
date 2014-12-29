#include <bits/stdc++.h>
#include "MyThread.h"

using namespace std;
typedef vector<int> vi;
typedef vector<bool> vb;
typedef vector<vi> vvi;
typedef long long int64;
typedef unsigned long long uint64;

#define rep(i,n) for(int i=0;i<(n);++i)
#define all(X) (X).begin(),(X).end()

int N1, M;
vvi g;
vi tids;
vi res;

void dfs(int u, vi &d) {
	for(int i=0; i<(int)g[u].size(); i++) {
		int v = g[u][i];
		if(d[v]==-1) d[v] = d[u]+1, dfs(v, d);
	}
}

void target1() {
	int src = getID();
	vi d = vi(N1, -1); d[src] = 0;
	dfs(src, d);
	res[src] = *max_element(all(d));
}

void* target2(void *arg) {
	int src = *(int *)(arg);
	vi d = vi(N1, -1); d[src] = 0;
	dfs(src, d);
	res[src] = *max_element(all(d));
}

void f(){
freopen("inp14", "r", stdin);
	cin >> N1 >> M;
	g = vvi(N1, vi()); int u, v;
	tids = res = vi(N1, -1);
	rep(i, M) {
		cin >> u >> v; u--; v--;
		g[u].push_back(v); g[v].push_back(v);
	}
	int arr[N1];
	for(int i=0; i<N1; i++) {
		/* UNCOMMENT ONE OF THE FOLLOWING LINES DEPENDING ON WHICH ONE IS WORKING FOR THE CURRENT GROUP*/
		//tids[i] = create(target1);
		arr[i]=i; tids[i] = createWithArgs(target2, &arr[i]);
		run(tids[i]);
		//target2(i); //DO NOT USE THIS
	}
	for(int i=0; i<N1; i++) {
		JOIN(tids[i]);
	}

	cout << *max_element(all(res)) << endl;
	while(1);
}

int main() {
	create(f);
	start();
	return 0;
}
