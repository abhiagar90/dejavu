#include <iostream>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>
using namespace std;
/*
 #include <string.h>
 #include <iostream>
 #include <stdio.h>

 #include <stdlib.h>
 using namespace std;

 #include "MyThread.h"
 #include <iostream>
 #define SECOND 1000000

 using namespace std;

 void f() {
 while (1) {
 cout << "f" << endl;
 usleep(SECOND);
 }
 }

 void g() {
 while (1) {
 cout << "g" << endl;
 usleep(SECOND);
 }
 }

 void* k(void* a) {
 while (1) {
 cout << "K" << endl;
 usleep(SECOND);
 //clean();
 }
 void* p;
 return p;
 }


 int main() {
 create(f);
 create(g);

 int a = 5;
 int *p = &a;
 createWithArgs(k, p);

 start();
 return 0;
 }




 int main() {
 while (1) {
 char ui_data[1024];
 fgets(ui_data, sizeof(ui_data), stdin);
 cout << ui_data << endl;
 cout << "outside" << endl;
 if (strcmp(ui_data, "create\n") == 0) {
 cout << "inside";
 }
 }
 return 0;
 }

 */

/*
int main() {
	int m = 160;
	int index = 39;
	for (int i = 0; i < m; i++) {
		if (i != 0 && i % 4 == 0) {
			index--;
		}
		char token[] = "0000000000000000000000000000000000000000";
		int tmp = pow(2, i % 4);
		token[index] = (char) (tmp + 48);
		cout << "i: " << i << " " << token << endl;
	}
	return 0;
}
*/
