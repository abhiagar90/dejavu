#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <map>
#include <openssl/evp.h>
#include <math.h>
#include "MyThread.h"
#include "MyHash.h"
#include "MyString.h"
#include "MyHelper.h"

using namespace std;

//----------Constants---------
#define MSG_JOIN "j:"
#define MSG_DUMP "d:"
#define MSG_DUMP_ALL "u:"
#define MSG_QUIT "e:"
#define MSG_NODE_SUCC "s:"
#define MSG_NODE_PRED "p:"
#define MSG_CHNG_SUCC "a:"
#define MSG_CHNG_PRED "b:"
#define MSG_KEY_SUCC "k:"
#define MSG_FINGER "f:"
#define MSG_FIX_FINGER "o:"
#define MSG_PUT "i:"
#define MSG_GET "g:"
#define MSG_ACK "m:"
#define MSG_FINGER_ACK "1:"

#define SERVER_BUSY 'x'
#define SERVER_DIFF_RING "c:"

//----------Globals---------
char ui_data[DATA_SIZE_KILO];
char server_send_data[DATA_SIZE_KILO], server_recv_data[DATA_SIZE_KILO];
char client_send_data[DATA_SIZE_KILO], client_recv_data[DATA_SIZE_KILO];
char ff_client_send_data[DATA_SIZE_KILO], ff_client_recv_data[DATA_SIZE_KILO];
char dd_client_send_data[DATA_SIZE_KILO], dd_client_recv_data[DATA_SIZE_KILO];

unsigned int server_port = 0;
unsigned int remote_port = 0; // port with which to connect to server
char ip2Join[IP_SIZE]; //used by client to join the server

int serverThreadId;
int serverSock;

int isCreated = false;
int isJoined = false;
int retry_count = 5;

int isNodeJoined = false;
int isMeCreator = false;

char creatorIP[IP_SIZE];

int fixFingerCount = 0;

//****************Function Declarations*******************
//-------Helper Functions----------
void runClientAndWaitForResult(int clientThreadID);

void helperHelp();
void helperPort(char* portCmd);
void helperClear();
void helperKeys();
void helperCreate();
void helperJoin(char* joinCmd);
void helperQuit();

void helperPut(char* putCmd);
void helperGet(char* getCmd);

void helperFinger();
void helperSuccessor();
void helperPredecessor();
void helperDump();
void helperDumpAddr(char* dumpAddrCmd);
void helperDumpAll();

void populateFingerTableSelf();
void fillNodeEntries(struct sockaddr_in server_addr);

void connectToRemoteNode(char* ip, unsigned int port);

void processQuit(char *data);
void processSucc();
void processPred();

void processFinger(char *data);
void processChangeSucc(char *addr);
void processChangePred(char *addr);
void processKeySucc(char *keyToSearch);
void processDump();

void changeSuccAndFixFirstFinger(nodeHelper* succ);

void shutMe();

//-----TCP Functions-------
void userInput();
void server();
void client();

//-----CHORD Functions-------
void askSuccToFixFinger();
void fixFingers();
nodeHelper* get_SuccFromRemoteNode(nodeHelper* remoteNode);
nodeHelper* get_PredFromRemoteNode(nodeHelper* remoteNode);
void changeSuccOfRemoteNodeToMyself(nodeHelper* remoteNode);
void changePredOfRemoteNodeToMyself(nodeHelper* remoteNode);
nodeHelper* find_successor(char key[]);
nodeHelper* closest_preceding_finger(char key[]);

void distributeKeys(nodeHelper *myPred);

nodeHelper* getKeySuccFromRemoteNode(nodeHelper* remoteNode, char key[]);

//****************Function Definitions*******************
//-------Helper Functions----------

void runClientAndWaitForResult(int clientThreadID) {
	client_recv_data[0] = '\0';
	run(clientThreadID);
	while (client_recv_data[0] == '\0')
		; //wait until data is received
}

void helperHelp() {
	cout << "Commands supported: " << endl;

	helperHelpNewCmd();
	cout << "help";
	tab(4);
	cout << "==> Provides a list of command and their usage details";

	helperHelpNewCmd();
	cout << "self";
	tab(4);
	cout << "==> prints my ip with port";

	helperHelpNewCmd();
	cout << "clear";
	tab(4);
	cout << "==> clears my screen";

	helperHelpNewCmd();
	cout << "mykeys";
	tab(4);
	cout << "==> prints my key values";

	helperHelpNewCmd();
	cout << "port <x>";
	tab(3);
	cout << "==> sets the port number";
	cout << " (eg- port 1234)";

	helperHelpNewCmd();
	cout << "create";
	tab(4);
	cout << "==> creates a ring";

	helperHelpNewCmd();
	cout << "join <x>";
	tab(3);
	cout << "==> joins the ring with x address";
	cout << " (eg- join 111.111.111.111:1000)";

	helperHelpNewCmd();
	cout << "quit";
	tab(4);
	cout << "==> shuts down the ring";

	helperHelpNewCmd();
	cout << "put <key> <value>";
	tab(2);
	cout << "==> inserts the given <key,value> pair in the ring ";
	cout << " (eg- put 23 654)";

	helperHelpNewCmd();
	cout << "get <key>";
	tab(3);
	cout << "==> returns the value corresponding to the key";
	cout << " (eg- get 23)";

	cout << endl;

	//Bonus Commands
	helperHelpNewCmd();
	cout << "-Yes!!! we have implemented BONUS COMMANDS, mentioned them below-";

	helperHelpNewCmd();
	cout << "finger";
	tab(4);
	cout << "==> prints the list of addresses of nodes on the ring";

	helperHelpNewCmd();
	cout << "successor";
	tab(3);
	cout << "==> prints the address of the next node on the ring";

	helperHelpNewCmd();
	cout << "predecessor";
	tab(3);
	cout << "==> prints the address of the previous node on the ring";

	helperHelpNewCmd();
	cout << "dump";
	tab(4);
	cout << "==> displays all information pertaining to calling node";

	helperHelpNewCmd();
	cout << "dumpaddr <address>";
	tab(2);
	cout << "==> displays information pertaining to node at address";
	cout << " (eg - dumpaddr 111.111.111.111:1000)";

	helperHelpNewCmd();
	cout << "dumpall";
	tab(4);
	cout << "==> displays information of all the nodes";

	cout << endl;
}

void helperSelf() {
if (!checkIfPartOfNw(selfNode)) {
		return;
	}
	cout << "I am " << selfNode->self->ipWithPort << endl;
}

void helperPort(char* portCmd) {
	if (!checkIfNotPartOfNw(selfNode)) {
		return;
	}

	server_port = fetchPortNumber(portCmd, 6);
	if (server_port != 0) {
		cout << "port: set to " << server_port << endl;
	} else {
		cout << "port Number did not set" << endl;
	}
}

void helperClear() {
	system("clear");
}

void helperMyKeys() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}
	printDataValMap(selfNode);
}

void helperCreate() {
	//create a listening socket here
	if (isCreated) {
		cout << "Already in a network, server thread running" << endl;
		return;
	}

	isCreated = true;
	isJoined = true;
	serverThreadId = create(server);
	run(serverThreadId);
}

void helperJoin(char* joinCmd) {
	char* addr = fetchAddress(joinCmd, 6);
	if (addr == NULL) {
		return;
	}

	unsigned int port = fetchPortNumber(joinCmd, indexOf(joinCmd, ':') + 2);
	if (port == 0) {
		//Invalid portNumber
		return;
	}

	strcpy(ip2Join, addr);
	remote_port = port;

	char remoteIpWithPort[IP_SIZE];
	joinIpWithPort(ip2Join, remote_port, remoteIpWithPort);
	nodeHelper* remoteNodeHelper = convertToNodeHelper(remoteIpWithPort);

	if (isJoined) {
		cout << "Already in a network and joined, joined thread running"
				<< endl;
		return;
	}

	cout << "Creating myself!!" << endl;
	helperCreate();

	//Making the connection
	strcpy(client_send_data, MSG_JOIN);
	int clientThreadID = create(client);
	runClientAndWaitForResult(clientThreadID);

	if (client_recv_data[0] == SERVER_BUSY) {
		isJoined = false;
		isCreated = false;
		deleteThread(serverThreadId);
		selfNode->self = NULL;
		close(serverSock);
		return;
	}
	strcpy(creatorIP, client_recv_data);
	cout << "Joined chord network with creatorIp: " << creatorIP << endl;
	retry_count = 9999; //Modifying the retry count for all the future connections

	cout << "Asking the known remote node for my actual successor" << endl;
	changeSuccAndFixFirstFinger(
			getKeySuccFromRemoteNode(remoteNodeHelper, selfNode->self->nodeKey));

	cout << "My actual successor is now: " << selfNode->successor->ipWithPort
			<< endl;

	selfNode->predecessor = get_PredFromRemoteNode(selfNode->successor);
	cout << "My actual predecessor is now: "
			<< selfNode->predecessor->ipWithPort << endl;

	cout << "Changing succ.pred & pred.succ to me, please wait---" << endl;

	changeSuccOfRemoteNodeToMyself(selfNode->predecessor);
	changePredOfRemoteNodeToMyself(selfNode->successor);
}

void putInMyMap(char* dataVal) {

	char dataValArr[2][DATA_SIZE_KILO];
	split(dataVal, ' ', dataValArr);

	char* hexHashKey = (char *) malloc(sizeof(char) * 41);
	data2hexHash(dataValArr[0], hexHashKey);

	char* dataForMap = (char *) malloc(sizeof(char) * 1024);

	strcpy(dataForMap, dataValArr[0]);
	strcat(dataForMap, " ");//changed from =>to ' '
	strcat(dataForMap, dataValArr[1]);
	strcat(dataForMap, "\0");

	insertInKeyMap(&selfNode->dataValMap, hexHashKey, dataForMap);

}

char* getFromMyMap(char* data) {

	char hexHashKey[HASH_HEX_BITS];
	data2hexHash(data, hexHashKey);

	map<const char*, const char*>::iterator it;

	if (!isPresentInKeyMap((selfNode->dataValMap), hexHashKey)) {
		cout << "Data not found for key: " << hexHashKey << endl;
		char* toReturn = (char *) malloc(sizeof(char) * 50);
		strcpy(toReturn, "DATA NOT FOUND!!!");
		//return "DATA NOT FOUND!!!";
		return toReturn;
	}
	return getFromKeyMap(selfNode->dataValMap, hexHashKey);
}

void helperPut(char* putCmd) {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}

	char *dataVal = (char *) malloc(sizeof(char) * 1024);
	char *hexHashKey = (char *) malloc(sizeof(char) * 41);
	strcpy(dataVal, substring(putCmd, 5, strlen(putCmd) - 5));

	char dataValArr[2][DATA_SIZE_KILO];

	split(dataVal, ' ', dataValArr);

	//char hexHashKey[HASH_HEX_BITS];
	data2hexHash(dataValArr[0], hexHashKey);

	nodeHelper* remoteNode = find_successor(hexHashKey);
	if (strcmp(remoteNode->nodeKey, selfNode->self->nodeKey) == 0) {
		putInMyMap(dataVal);
	}

	else {
		strcpy(client_send_data, MSG_PUT);
		strcat(client_send_data, dataVal);

		connectToRemoteNode(remoteNode->ip, remoteNode->port);
	}

	cout << "Data inserted: " << dataVal << " with: " << hexHashKey << ", at: "
			<< remoteNode->ipWithPort << endl;

}

void helperGet(char* getCmd) {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}
	char dataVal[1024];

	char data[DATA_SIZE_KILO];
	strcpy(data, substring(getCmd, 5, strlen(getCmd) - 5));

	char dataArr[1][DATA_SIZE_KILO];

	split(data, ' ', dataArr);

	char hexHashKey[HASH_HEX_BITS];
	data2hexHash(dataArr[0], hexHashKey);

	nodeHelper* remoteNode = find_successor(hexHashKey);

	if (strcmp(remoteNode->nodeKey, selfNode->self->nodeKey) == 0) {
		strcpy(dataVal, getFromMyMap(data));
	}

	else {
		strcpy(client_send_data, MSG_GET);
		strcat(client_send_data, dataArr[0]);

		connectToRemoteNode(remoteNode->ip, remoteNode->port);
		strcpy(dataVal, client_recv_data);
	}

	if (strcmp(dataVal, "DATA NOT FOUND!!!") == 0) {
		cout << dataVal << endl;
		return;
	}

	cout << "Data Found: " << hexHashKey << "\t" << dataVal << endl;
}

void helperFinger() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}

	strcpy(client_send_data, MSG_FINGER);

	int i = 0;
	cout << i++ << " -> " << selfNode->self->ipWithPort << endl;

	nodeHelper* remoteNode = selfNode->successor;
	while (strcmp(remoteNode->ipWithPort, selfNode->self->ipWithPort) != 0) {
		cout << i++ << " -> " << remoteNode->ipWithPort << endl;
		connectToRemoteNode(remoteNode->ip, remoteNode->port);

		remoteNode = convertToNodeHelper(client_recv_data);
	}

	cout << "Done with printing all the fingers" << endl;
}

void helperQuit() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}

	cout
			<< "Asking all the nodes to shut down, Thanks for using chord_DHT, see you again soon :)"
			<< endl;

	strcpy(client_send_data, MSG_QUIT);
	strcat(client_send_data, selfNode->self->ipWithPort);

	nodeHelper* remoteNode = selfNode->successor;
	while (strcmp(remoteNode->ipWithPort, selfNode->self->ipWithPort) != 0) {
		connectToRemoteNode(remoteNode->ip, remoteNode->port);
		remoteNode = convertToNodeHelper(client_recv_data);
	}

	cout << "Got ack from all the nodes" << endl;
	shutMe();
}

void helperSuccessor() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}
	cout << "Successor-> " << selfNode->successor->ipWithPort << endl;
}

void helperPredecessor() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}
	cout << "Predecessor-> " << selfNode->predecessor->ipWithPort << endl;
}

void helperDump() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}

	printNodeDetails(selfNode);
}

void getAndPrintDump(char *addr, unsigned int port) {
	strcpy(client_send_data, MSG_DUMP);

	retry_count = 5; //Modifying the retry count assuming server IP may be incorrect
	connectToRemoteNode(addr, port);
	retry_count = 9999; //Modifying the retry count for all the future connections

	if (client_recv_data[0] == SERVER_BUSY) { //Server busy or does not exist
		return;
	} else if (strcmp(client_recv_data, SERVER_DIFF_RING) == 0) { //Server not in the same chord network
		cout << "Sorry, server unreachable, may belong to diff network" << endl;
		return;
	}

	cout << "Dump Received" << endl;

	printDump(client_recv_data);
}

void helperDumpAddr(char* dumpAddrCmd) {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}

	char* addr;
	addr = fetchAddress(dumpAddrCmd, 10);

	if (addr == NULL) {
		return;
	}

	unsigned int port = fetchPortNumber(dumpAddrCmd,
			indexOf(dumpAddrCmd, ':') + 2);

	if (port == 0) {
		//Invalid portNumber
		return;
	}
	getAndPrintDump(addr, port);
}

void helperDumpAll() {
	if (!checkIfPartOfNw(selfNode)) {
		return;
	}

	strcpy(client_send_data, MSG_DUMP_ALL);

	int i = 0;
	cout << i++ << " :: " << endl;
	printNodeDetails(selfNode, false);

	nodeHelper* remoteNode = selfNode->successor;
	while (strcmp(remoteNode->ipWithPort, selfNode->self->ipWithPort) != 0) {

		connectToRemoteNode(remoteNode->ip, remoteNode->port);
		//findSucc
		int startIndex = indexOf(client_recv_data, '?') + 2;
		int lenSucc = strlen(client_recv_data) - startIndex + 1;
		char* succ = substring(client_recv_data, startIndex, lenSucc);

		cout << i++ << " :: " << endl;
		printDump(substring(client_recv_data, 0, startIndex - 2));

		remoteNode = convertToNodeHelper(succ);
	}
	cout << "Done with printing dump of all the nodes" << endl;
}

//populates finger table with all the self entries - only node in the network
void populateFingerTableSelf() {
	for (int i = 0; i < M; i++) {
		selfNode->fingerNode[i] = selfNode->self;
	}
}

void fillNodeEntries(struct sockaddr_in server_addr) {
	nodeHelper* self = new nodeHelper();

	char ip[IP_SIZE];
	memset(ip, 0, sizeof ip);
	getMyIp(ip);

	if (strlen(ip) == 0) {
		strcpy(ip, "127.0.0.1");
	}

	strcpy(self->ip, ip);

	self->port = getMyPort(serverSock);

	char ipWithPort[IP_SIZE];
	joinIpWithPort(self->ip, self->port, ipWithPort);
	strcpy(self->ipWithPort, ipWithPort);

	char hexHash[HASH_HEX_BITS];
	data2hexHash(self->ipWithPort, hexHash);
	strcpy(self->nodeKey, hexHash);

	selfNode->self = self;
	selfNode->successor = self;
	selfNode->predecessor = self;

	//Fill finger table
	int index = 39;
	for (int i = 0; i < M; i++) {
		if (i != 0 && i % 4 == 0) {
			index--;
		}
		char token[] = "0000000000000000000000000000000000000000";
		int tmp = pow(2, i % 4);
		token[index] = (char) (tmp + 48);
		char hexVal[HASH_HEX_BITS];
		hexAddition(self-> nodeKey, token, hexVal, strlen(self->nodeKey));
		strcpy(selfNode->fingerStart[i], hexVal);
	}
	populateFingerTableSelf();

	if (isMeCreator) {
		strcpy(creatorIP, selfNode->self->ipWithPort);
	}
}

void connectToRemoteNode(char* ip, unsigned int port) {
	memset(client_recv_data, 0, sizeof client_recv_data);
	strcpy(ip2Join, ip);
	remote_port = port;

	//Appending creatorIp in the request in the request
	strcat(client_send_data, "?");
	strcat(client_send_data, creatorIP);

	//cout << "Inside connectToRemoteNode: clientsendData - " << client_send_data
	//<< endl;
	int clientThreadID = create(client);
	runClientAndWaitForResult(clientThreadID);
}

void processJoin() {
	//cout << "Client wants to join" << endl;
	if (isMeCreator && !isNodeJoined) {
		isNodeJoined = true;
		askSuccToFixFinger();
	}
	strcpy(server_send_data, creatorIP);
}

void processSucc() {
	//cout << "Client wants my successor details" << endl;
	strcpy(server_send_data, selfNode->successor->ipWithPort);
}

void processPred() {
	//cout << "Client wants my predecessor details" << endl;
	strcpy(server_send_data, selfNode->predecessor->ipWithPort);
}

void processFinger(char *data) {
	//cout << "Client wants to find the fingers" << endl;
	strcpy(server_send_data, selfNode->successor->ipWithPort);
}

void processQuit(char *data) {
	//cout << "ChordRing shutting down, I need to shut as well" << endl;
	strcpy(server_send_data, selfNode->successor->ipWithPort);
}

void processFixFinger() {
	askSuccToFixFinger();
	strcpy(server_send_data, MSG_FINGER_ACK);
}

void processChangeSucc(char *addr) {
	//cout << "Client wants to change my succ to: " << addr << endl;
	changeSuccAndFixFirstFinger(convertToNodeHelper(addr));
	strcpy(server_send_data, MSG_ACK);
}

void processChangePred(char *addr) {
	//cout << "Client wants to change my pred to: " << addr << endl;
	selfNode->predecessor = convertToNodeHelper(addr);
	distributeKeys(selfNode->predecessor);

	strcpy(server_send_data, MSG_ACK);
}

void processPut(char *dataVal) {
	//cout << "Client wants to put: " << dataVal << endl;
	putInMyMap(dataVal);

	strcpy(server_send_data, MSG_ACK);
}

void processGet(char *data) {
	//cout << "Client wants to get val for: " << data << endl;

	char hexHashKey[HASH_HEX_BITS];
	data2hexHash(data, hexHashKey);
	char dataVal[DATA_SIZE_KILO];

	strcpy(dataVal, getFromMyMap(data));
	strcpy(server_send_data, dataVal);
}

void processKeySucc(char *keyToSearch) {
	/*cout << "Client requests for finding Node successor of this key: "
	 << keyToSearch << endl;*/
	nodeHelper* toReturn = find_successor(keyToSearch);
	strcpy(server_send_data, toReturn->ipWithPort);
}

void processDump() {
	//cout << "Client wants my dump" << endl;

	strcpy(server_send_data, selfNode->self->ipWithPort);
	strcat(server_send_data, ",");
	strcat(server_send_data, selfNode->successor->ipWithPort);
	strcat(server_send_data, ",");
	strcat(server_send_data, selfNode->predecessor->ipWithPort);

	//------Not sending my finger details---

	/*strcat(server_send_data, "|");

	 for (int i = 0; i < M; i++) {
	 strcat(server_send_data, selfNode->fingerStart[i]);
	 strcat(server_send_data, ",");
	 }

	 strcat(server_send_data, "|");

	 for (int i = 0; i < M; i++) {
	 char* nodeIpWithPort = selfNode->fingerNode[i]->ipWithPort;
	 strcat(server_send_data, nodeIpWithPort);
	 strcat(server_send_data, ",");
	 }*/

	strcat(server_send_data, "|");

	map<char*, char*>::iterator it;

	for (map<char*, char*>::iterator it = (selfNode->dataValMap).begin(); it
			!= (selfNode->dataValMap).end(); ++it) {
		strcat(server_send_data, it->first);
		strcat(server_send_data, ":");
		strcat(server_send_data, it->second);
		strcat(server_send_data, ",");
	}
}

void processDumpAll() {
	//cout << "Received DumpAll request" << endl;
	processDump();
	//Adding successor ipWithPort, to be used by client to find my successors dump
	strcat(server_send_data, "?");
	strcat(server_send_data, selfNode->successor->ipWithPort);
}

void changeSuccAndFixFirstFinger(nodeHelper* succ) {
	selfNode->successor = succ;
	selfNode->fingerNode[0] = succ;
}

void shutMe() {
	cout << "Shutting myself after 5 sec" << endl;
	sleep(5);
	close(serverSock);
	clean();
}

//-----TCP Functions-------
void userInput() {
	helperClear();

	while (1) {
		cout << "\n------------------------------" << endl;

		cout << ">>>: ";
		fgets(ui_data, sizeof(ui_data), stdin);

		cout << "<<<: " << ui_data << endl;

		char* cmdType = substring(ui_data, 0, indexOf(ui_data, ' '));

		if (strcmp(cmdType, "help") == 0) {
			helperHelp();
		}

		else if (strcmp(cmdType, "self") == 0) {
			helperSelf();
		}

		else if (strcmp(cmdType, "clear") == 0) {
			helperClear();
		}

		else if (strcmp(cmdType, "mykeys") == 0) {
			helperMyKeys();
		}

		else if (strcmp(cmdType, "port") == 0) {
			helperPort(ui_data);
		}

		else if (strcmp(cmdType, "create") == 0) {
			isMeCreator = true;
			helperCreate();
		}

		else if (strcmp(cmdType, "join") == 0) {
			helperJoin(ui_data);
		}

		else if (strcmp(cmdType, "quit") == 0) {
			helperQuit();
		}

		else if (strcmp(cmdType, "put") == 0) {
			helperPut(ui_data);
		}

		else if (strcmp(cmdType, "get") == 0) {
			helperGet(ui_data);
		}

		else if (strcmp(cmdType, "finger") == 0) {
			helperFinger();
		}

		else if (strcmp(cmdType, "successor") == 0) {
			helperSuccessor();
		}

		else if (strcmp(cmdType, "predecessor") == 0) {
			helperPredecessor();
		}

		else if (strcmp(cmdType, "dump") == 0) {
			helperDump();
		}

		else if (strcmp(cmdType, "dumpaddr") == 0) {
			helperDumpAddr(ui_data);
		}

		else if (strcmp(cmdType, "dumpall") == 0) {
			helperDumpAll();
		}

		else {
			cout
					<< "Sorry!!! It seems like you are new here, please type 'help' for list of commands"
					<< endl;
		}

		fflush(stdout);
	}
}

void server() {
	int sock, connected, trueint = 1;

	struct sockaddr_in server_addr, client_addr;
	unsigned int sin_size;

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}

	serverSock = sock;

	if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &trueint, sizeof(int)) == -1) {
		perror("Setsockopt");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	if (server_port != 0) { //Let the server choose the port itself if not supplied externally
		server_addr.sin_port = htons(server_port);
	}
	server_addr.sin_addr.s_addr = INADDR_ANY;

	bzero(&(server_addr.sin_zero), 8);

	if (bind(sock, (struct sockaddr *) ((&server_addr)),
			sizeof(struct sockaddr)) == -1) {
		perror("Unable to bind");
		exit(1);
	}
	if (listen(sock, QUEUE_LIMIT) == -1) {
		perror("Listen");
		exit(1);
	}
	fillNodeEntries(server_addr);

	cout << "Starting to listen on: " << selfNode->self->ipWithPort << endl;
	cout << ">>>: ";
	fflush(stdout);
	while (1) {
		int bytes_received;
		sin_size = sizeof(struct sockaddr_in);
		connected
				= accept(sock, (struct sockaddr*) ((&client_addr)), &sin_size);

		/*cout << "I got a connection from" << inet_ntoa(client_addr.sin_addr)
		 << ntohs(client_addr.sin_port) << endl;*/

		bytes_received = recv(connected, server_recv_data, DATA_SIZE_KILO, 0);
		server_recv_data[bytes_received] = '\0';

		char* type = substring(server_recv_data, 0, 2);
		char* data = substring(server_recv_data, 3, strlen(server_recv_data));

		char dataValArr[2][DATA_SIZE_KILO];
		split(data, '?', dataValArr);

		//cout << "Got request from: " << dataValArr[1] << endl;

		char* reqData = dataValArr[0];
		if (strcmp(type, MSG_QUIT) == 0) {
			processQuit(reqData);
		}

		else if (strcmp(type, MSG_FIX_FINGER) == 0) {
			processFixFinger();
		}

		else if (strcmp(type, MSG_JOIN) == 0) {
			processJoin();
		}

		else if (strcmp(type, MSG_NODE_SUCC) == 0) {
			processSucc();
		}

		else if (strcmp(type, MSG_NODE_PRED) == 0) {
			processPred();
		}

		else if (strcmp(type, MSG_FINGER) == 0) {
			processFinger(reqData);
		}

		else if (strcmp(type, MSG_KEY_SUCC) == 0) {
			processKeySucc(reqData);
		}

		else if (strcmp(type, MSG_DUMP) == 0) {
			if (strcmp(creatorIP, dataValArr[1]) == 0) { //checking if belonging to same chord network
				processDump();
			} else {
				strcpy(server_send_data, SERVER_DIFF_RING);
			}
		}

		else if (strcmp(type, MSG_DUMP_ALL) == 0) {
			processDumpAll();
		}

		else if (strcmp(type, MSG_CHNG_SUCC) == 0) {
			processChangeSucc(reqData);
		}

		else if (strcmp(type, MSG_CHNG_PRED) == 0) {
			processChangePred(reqData);
		}

		else if (strcmp(type, MSG_PUT) == 0) {
			processPut(reqData);
		}

		else if (strcmp(type, MSG_GET) == 0) {
			processGet(reqData);
		}

		send(connected, server_send_data, strlen(server_send_data), 0);
		//cout << "Done the required task, closing the connection" << endl;
		//cout << "------------------------------\n>>>:";
		fflush(stdout);//may be fatal, adding for UI
		close(connected);

		if (strcmp(type, MSG_QUIT) == 0) {
			shutMe();
		}
	}
	//right now, doesn't reach here
	close(sock);
}

bool connectToServer(int & sock) {
	struct hostent *host;
	struct sockaddr_in server_addr;
	/*cout << "Inside connect to server: " << ip2Join << ":" << remote_port
	 << endl;*/
	host = gethostbyname(ip2Join);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Socket");
		exit(1);
	}
	//cout << "Client socket created" << endl;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr = *((struct in_addr *) host->h_addr);
	server_addr.sin_port = htons(remote_port);
	bzero(&(server_addr.sin_zero), 8);

	int retriedCount = 0;
	while (connect(sock, (struct sockaddr *) &server_addr,
			sizeof(struct sockaddr)) == -1) {

		//trying again assuming the server is busy
		retriedCount++;
		cout << "Server busy --- retrying(" << retriedCount << "/"
				<< retry_count << ")" << endl;
		sleep(1);
		if (retriedCount == retry_count) {
			cout
					<< "Server is not up or not responding, terminating client...please try again"
					<< endl;
			close(sock);
			return false;
		}
	}
	//cout << "Client successfully connected to server" << endl;
	return true;
}

void client() {
	//cout << "\n------------------------------" << endl;
	//cout << "Client started" << endl;

	int sock, bytes_recieved;

	if (!connectToServer(sock)) {
		client_recv_data[0] = SERVER_BUSY; //Inserting this --- to be used in helperJoin
		return;
	}

	//cout << "Client socket ID:" << sock << endl;

	send(sock, client_send_data, strlen(client_send_data), 0);

	bytes_recieved = recv(sock, client_recv_data, DATA_SIZE_KILO, 0);
	//cout << "Data successfully received" << endl;
	client_recv_data[bytes_recieved] = '\0';

	close(sock);
}

void fingersClient() {
	sleep(5);
	int sock, bytes_recieved;

	if (!connectToServer(sock)) {
		ff_client_recv_data[0] = SERVER_BUSY; //Inserting this --- to be used in helperJoin
		return;
	}

	send(sock, ff_client_send_data, strlen(ff_client_send_data), 0);

	bytes_recieved = recv(sock, ff_client_recv_data, DATA_SIZE_KILO, 0);
	ff_client_recv_data[bytes_recieved] = '\0';
	close(sock);
}

void distributeClient() {
	sleep(5);
	int sock, bytes_recieved;

	if (!connectToServer(sock)) {
		dd_client_recv_data[0] = SERVER_BUSY; //Inserting this --- to be used in helperJoin
		return;
	}

	send(sock, dd_client_send_data, strlen(dd_client_send_data), 0);

	bytes_recieved = recv(sock, dd_client_recv_data, DATA_SIZE_KILO, 0);
	dd_client_recv_data[bytes_recieved] = '\0';
	close(sock);
}

//-----------CHORD FUNCTIONS-------
void askSuccToFixFinger() {
	//sleep(2);
	fixFingers(); //fixing my finger table

	strcpy(ff_client_send_data, MSG_FIX_FINGER);

	strcpy(ip2Join, selfNode->successor->ip);
	remote_port = selfNode->successor->port;

	int clientThreadId = create(fingersClient);
	run(clientThreadId); //Non blocking call for remoteSucc fixFnger
}

void fixFingers() {
	if (fixFingerCount % 10 != 0) {
		fixFingerCount++;
		return;
	}

	fixFingerCount++;
	//cout << "stabilizing--- ";
	for (int fixFingerIndex = 1; fixFingerIndex < M; fixFingerIndex++) {
		char* key = selfNode->fingerStart[fixFingerIndex];
		char* me = selfNode->self->nodeKey;
		char* succKey = selfNode->successor->nodeKey;
		char* predKey = selfNode->predecessor->nodeKey;

		if (strcmp(key, succKey) == 0 || keyBelongCheck(me, succKey, key)) {
			selfNode->fingerNode[fixFingerIndex] = selfNode->successor;
		}

		else if (strcmp(key, me) == 0 || keyBelongCheck(predKey, me, key)) {
			selfNode->fingerNode[fixFingerIndex] = selfNode->self;
		}

		else {
			selfNode->fingerNode[fixFingerIndex] = find_successor(key);
		}
	}
	//cout << "stabilized" << endl;
}

//I am going to distributeKeys to my predecessor
void distributeKeys(nodeHelper* myPred) {
	map<char*, char*>::iterator it;
	for (map<char*, char*>::iterator it = (selfNode->dataValMap).begin(); it
			!= (selfNode->dataValMap).end(); ++it) {
		//cout << it->first << " : " << it->second << '\n';
		if (keyBelongCheck(selfNode->predecessor->nodeKey,
				selfNode->self->nodeKey, it->first) or strcmp(it->first,
				selfNode->self->nodeKey) == 0) {

		} else {
			char *dataVal = (char *) malloc(sizeof(char) * 1024);
			strcpy(dataVal, it->second);

			strcpy(dd_client_send_data, MSG_PUT);
			strcat(dd_client_send_data, dataVal);

			strcpy(ip2Join, myPred->ip);
			remote_port = myPred->port;

			int clientThreadId = create(distributeClient);
			run(clientThreadId); //Non blocking call for distributekeys

			cout << "key-val pair " << dataVal << " transferring to pred"
					<< endl;
			selfNode->dataValMap.erase(it->first);//last line
			sleep(5);
		}
	}
}

nodeHelper* get_SuccFromRemoteNode(nodeHelper* remoteNode) {
	strcpy(client_send_data, MSG_NODE_SUCC);

	connectToRemoteNode(remoteNode->ip, remoteNode->port);
	//cout << "Got the successor from remote node: " << client_recv_data << endl;
	return convertToNodeHelper(client_recv_data);
}

nodeHelper* get_PredFromRemoteNode(nodeHelper* remoteNode) {
	strcpy(client_send_data, MSG_NODE_PRED);

	connectToRemoteNode(remoteNode->ip, remoteNode->port);
	//cout << "Got the predecessor from remote node: " << client_recv_data
	//<< endl;
	return convertToNodeHelper(client_recv_data);
}

void changeSuccOfRemoteNodeToMyself(nodeHelper* remoteNode) {
	strcpy(client_send_data, MSG_CHNG_SUCC);
	strcat(client_send_data, selfNode->self->ipWithPort);

	connectToRemoteNode(remoteNode->ip, remoteNode->port);
	cout << "Changed the successor of remote node to myself" << endl;
}

void changePredOfRemoteNodeToMyself(nodeHelper* remoteNode) {
	strcpy(client_send_data, MSG_CHNG_PRED);
	strcat(client_send_data, selfNode->self->ipWithPort);

	connectToRemoteNode(remoteNode->ip, remoteNode->port);
	cout << "Changed the predecessor of remote node to myself" << endl;
}

nodeHelper* find_successor(char key[]) {
	char* nodeKey = selfNode->self->nodeKey;
	char* succKey = selfNode->successor->nodeKey;

	if (strcmp(key, succKey) == 0 || keyBelongCheck(nodeKey, succKey, key)) {
		return selfNode->successor;
	} else {
		nodeHelper* closestPrecedingNode = closest_preceding_finger(key);
		if (strcmp(closestPrecedingNode->nodeKey, selfNode->self->nodeKey) == 0) {
			return selfNode->self;
		} else {
			return getKeySuccFromRemoteNode(closestPrecedingNode, key);
		}
	}
}

nodeHelper* closest_preceding_finger(char key[]) {
	for (int i = M - 1; i >= 0; i--) {
		char* fingerNodeId = selfNode->fingerNode[i]->nodeKey;
		if (keyBelongCheck(selfNode->self->nodeKey, key, fingerNodeId)) {
			return selfNode->fingerNode[i];
		}
	}
	return selfNode->self;
}

nodeHelper* getKeySuccFromRemoteNode(nodeHelper* remoteNode, char key[]) {
	strcpy(client_send_data, MSG_KEY_SUCC); //we will be acting as client to send data
	strcat(client_send_data, key);

	if (strcmp(remoteNode->ipWithPort, selfNode->self->ipWithPort) == 0) { //no need to connect to remote node, if it is me
		return selfNode->self;
	}
	connectToRemoteNode(remoteNode->ip, remoteNode->port);

	//cout << "Data received from remote node " << client_recv_data << endl;
	return convertToNodeHelper(client_recv_data);
}

//-----------MAIN---------------

int main() {
	create(userInput);
	start();
	return 0;
}
