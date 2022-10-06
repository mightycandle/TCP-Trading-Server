#include <bits/stdc++.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
using namespace std;

int port = 5555;
char IP[] = "127.0.0.1";

const int MAX = 1025;
char buffer[MAX];

socklen_t addrlen;
int server_sock, new_sock;
int sd=0,max_sd=0,num_clients=0,id_cntr=0;
int clientSock[MAX];
int clientID[MAX];

struct sockaddr_in address;


class Input{ public:
	int client_sock;
	int client_id;
	string text = "";
	vector<string> text_split = {""};
	string type = "";
	Input(int client_sock,int client_id,string text){
		this->client_sock=client_sock;
		this->client_id = client_id;
		this->text=text;
		for(auto x:text){
			if(x==' '){
				text_split.push_back("");
			}
			else{
				text_split.back()+=x;
			}
		}
		type = text_split[0];
	}
	string Type(){
		return text_split[0];
	}
};


class Order{ public:
	int order_id=0;
	int client_id=0;
	int stock_id=0;

	int qty;
	int price;
	
	bool is_buy = 0;
	bool is_sell = 0;
	
	string stock = "";
	string text = "";
	string type = "";

	Order(){}
	Order(int order_id, Input input){
		this->order_id=order_id;
		client_id = input.client_id;

		text = input.text;
		type = input.text_split[0];
		(type == "BUY") ? is_buy=1:is_sell=1;
		stock = input.text_split[1];
		qty=stoi(input.text_split[2]);
		price=stoi(input.text_split[3]);
	}
	void setStockID(int stock_id){
		this->stock_id = stock_id;
	}

	string show(){
		string output = "orderID:"+to_string(order_id) + " | ";
		output+=text;
		while(output.back()=='\n'){
			output.pop_back();
		}
		output += " --> from Client - " + to_string(client_id) + "\n";
		return output;
	}
};

class Stock{public:
	int id = 0;
	int order_cnt = 0;
	string name = "";

	vector<Order> orders;
	vector<string> logs = {""};

	Stock(int id=0,string name=""){
		this->id=id;
		this->name=name;
	}

	void Log(Order order){
		logs.push_back(order.show());
	}
	
	string List(){
		string log_str = "\nLIST orders for Stock: " + name + "\n";
		for(int i=0;i<logs.size();i++){
			log_str += logs[i];
		}
		return log_str;
	}
};

class TradingServer{ public:
	int order_cnt = 1;
	int stock_cnt = 1;
	
	vector<Order> orders;
	vector<Stock> stocks;
	
	map<string,int> stock_ids;

	TradingServer(){}

	int GetStockIndex(string stock){
		return stock_ids[stock]-1;
	}

	int GetOrderIndex(Order order){
		for(int i=0;i<orders.size();i++){
			if(orders[i].order_id == order.order_id){
				return i;
			}
		}
		return -1;
	}

	static bool higherPrice(Order a,Order b){
		return a.price > b.price;
	}
	static bool lowerPrice(Order a,Order b){
		return a.price < b.price;
	}
	static bool lowerOrder(Order a,Order b){
		return a.order_id < b.order_id;
	}

	string Operate(Input input){
		if(input.type=="BUY" or input.type=="SELL"){

			Order order = Order(order_cnt,input);
			order_cnt++;

			if(stock_ids[order.stock]==0){
				stock_ids[order.stock] = stock_cnt;
				stocks.push_back(Stock(stock_cnt,order.stock));
				order.setStockID(stock_cnt);
				stock_cnt++;
			}

			int stock_index = GetStockIndex(order.stock);
			stocks[stock_index].Log(order);

			orders.push_back(order);

			string output = "\n";
			if(order.is_buy){
				vector<Order> bought = BuyOrder(order);
				output += "Executing buy order ...\n";
				if(bought.empty()){
					output += "\tNo orders matched.\n";
				}
				else{
					for(auto bought_order: bought){
						output += "\t";
						output+= bought_order.show();
					}
				}
				output += "Remaining: "+to_string(order.qty)+" shares to be bought.\n";
			}
			else{
				vector<Order> sold = SellOrder(order);
				output = "Executing sell order ...\n";
				if(sold.empty()){
					output+="\tNo orders matched.\n";
				}
				else{
					for(auto sold_order : sold){
						output += "\t";
						output += sold_order.show();
					}
				}
				output += "Remaining: "+ to_string(order.qty)+" shares to be sold.\n";
			}
			return output;
		}
		else if(input.type == "LIST"){
			string stock_name = input.text_split[1];
			int index = stock_ids[stock_name];
			return stocks[index].List();
		}
		return "Error. No matching command found.\n";
	}
	/*
	 * for every buy order, match it with least sell price order
	 * for each sell order, match it with highest buy order
	 * */
	vector<Order> BuyOrder(Order &buy_order){
		vector<Order> bought, new_orders;
		sort(orders.begin(),orders.end(),lowerPrice);

		for(int i=0;i<orders.size();i++){
			bool should_check = true;
			if(orders[i].is_buy or orders[i].price > buy_order.price){
				should_check = false;
			}
			if(buy_order.client_id == orders[i].client_id){
				should_check = false;
			}
			if(buy_order.qty <= 0){
				should_check = false;
			}

			if(should_check){
				Order order_took = orders[i];
				order_took.qty = min(orders[i].qty, buy_order.qty);

				orders[i].qty -= order_took.qty;
				buy_order.qty -= order_took.qty;

				bought.push_back(order_took);
			}
			if(orders[i].qty > 0){
				new_orders.push_back(orders[i]);
			}
		}

		orders = new_orders;
		sort(bought.begin(),bought.end(),lowerOrder);
		return bought;
	}


	vector<Order> SellOrder(Order &sell_order){
		vector<Order> sold, new_orders;
		sort(orders.begin(),orders.end(),higherPrice);

		for(int i=0;i<orders.size();i++){
			bool should_check = true;
			if(orders[i].is_sell or orders[i].price < sell_order.price){
				should_check = false;
			}
			if(sell_order.client_id == orders[i].client_id){
				should_check = false;
			}
			if(sell_order.qty <= 0){
				should_check = false;
			}

			if(should_check){
				Order order_took = orders[i];
				order_took.qty = min(orders[i].qty, sell_order.qty);

				orders[i].qty -= order_took.qty;
				sell_order.qty -= order_took.qty;

				sold.push_back(order_took);
			}
			if(orders[i].qty > 0){
				new_orders.push_back(orders[i]);
			}
		}

		orders = new_orders;
		sort(sold.begin(),sold.end(),lowerOrder);
		return sold;
	}

};


int main(){
	fd_set readfds;
	server_sock = socket(AF_INET, SOCK_STREAM, 0);

	if (server_sock < 0) {
		cout << "[-] Socket error." << "\n";
		exit(1);
	}
	cout << "[+] TCP Server Socket created." << "\n";

	memset(&address, '\0', sizeof(address));
	address.sin_family = AF_INET;
	address.sin_port = port;
	address.sin_addr.s_addr = inet_addr(IP);

	bzero(buffer,MAX);
	if (bind(server_sock, (struct sockaddr *)&address, sizeof(address)) < 0) {
		cout << "[-] Bind Error." << "\n";
		exit(1);
	}
	cout << "[+] Bind to the port number: " << port << " sucessful.\n";

	addrlen = sizeof(address);
	listen(server_sock, 4);
	
	cout << "Listening..." << "\n";

	memset(clientSock,0,MAX);
	memset(clientID,0,MAX);

	TradingServer Server;

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(server_sock,&readfds);
		max_sd=server_sock;

		for(int i=0;i<MAX;i++){
			sd=clientSock[i];
			if(sd>0){
				FD_SET(sd,&readfds);
			}
			max_sd=max(max_sd,sd);
		}

		struct timeval timeout;
		timeout.tv_sec=0;
		timeout.tv_usec=0;
		int activity = select(max_sd+1,&readfds,NULL,NULL,&timeout);
		
		if(FD_ISSET(server_sock,&readfds)){
			new_sock = accept(server_sock,(struct sockaddr *)&address,(socklen_t*)&addrlen);
			if(new_sock < 0){
				cout << "[-] Client hasn't accepted anything." << "\n";
				exit(1);
			}

			cout << "[+] New connection - Socket fd : " << new_sock << " | " << "IP : " << inet_ntoa(address.sin_addr) << " | Port : " << ntohs(address.sin_port) << "\n\n";

			num_clients++;
			for(int i=0;i<MAX;i++){
				if(clientSock[i] == 0){
					clientSock[i]=new_sock;
					clientID[i]=++id_cntr;
					break;
				}
			}
		}
		for(int i=0;i<MAX;i++){
			sd=clientSock[i];
			if(FD_ISSET(sd,&readfds)){
				int status = read(sd,buffer,MAX);
				if(status!=0){
					cout << "Client " << clientID[i] << ": " << buffer << "\n";

					string input_text(buffer);
					Input input(clientSock[i],clientID[i],input_text);

					string result = Server.Operate(input) + "\n";

					bzero(buffer,MAX);
					strcpy(buffer,result.c_str());

					send(clientSock[i],buffer,strlen(buffer),0);
					bzero(buffer,MAX);
				}
				else{
					getpeername(sd,(struct sockaddr*)&address,(socklen_t*)&addrlen);
					cout << "[.] Host disconnected - " << "IP: " << inet_ntoa(address.sin_addr) << " | Port: " << ntohs(address.sin_port) << "\n\n";

					num_clients-=1;

					bzero(buffer,MAX);
					close(sd);
					clientSock[i]=0;
				}
			}
		}
	}
}

