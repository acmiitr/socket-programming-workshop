#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <bits/stdc++.h>

using namespace std;

char game_board[3][3];
vector<int> possible_positions = {1, 2, 3, 4, 5, 6, 7, 8, 9};
bool isGameComplete = false;

int check(int cl){
    for(int i=0; i<possible_positions.size(); i++){
        if(cl==possible_positions[i])return i;
    }
    return -1;
}

bool checkForWin(char c){
    
    for (int i = 0; i < 3; i++)
    {
        if(game_board[i][0]==c && game_board[i][0]==game_board[i][1] && game_board[i][1]==game_board[i][2]) return 1;
    }
    for (int i = 0; i < 3; i++)
    {
        if(game_board[0][i]==c && game_board[0][i]==game_board[1][i] && game_board[1][i]==game_board[2][i]) return 1;
    }

    if(game_board[0][0]==c && game_board[0][0]==game_board[1][1] && game_board[1][1]==game_board[2][2]) return 1;
    if(game_board[0][2]==c && game_board[0][2]==game_board[1][1] && game_board[1][1]==game_board[2][0]) return 1;
    
    return 0;
}

string board_representation(){
    string rep="";
    string line_ = "-------------\n";
    rep+=line_;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            rep+="| ";
            rep.push_back(game_board[i][j]);
            rep.push_back(' ');
        }
        rep+="|\n";
        rep+=line_;
    }
    return rep;
}

string playGame(int cl){
    string currMessage="";
    string line_ = "-------------\n";

    int idx = check(cl);
    if(idx==-1){
        currMessage +="Invalid Move\n";
        return currMessage;
    }
    int r = (cl-1)/3, c=(cl-1)%3;
    game_board[r][c]='X';
    possible_positions.erase(possible_positions.begin()+idx);
    bool result = checkForWin('X');
    if(result){
        currMessage+="Congratulations! you won!!\n";
        currMessage+=board_representation();
        isGameComplete=1;
        return currMessage;
    }
    if(possible_positions.size()==0){
        currMessage+="Draw!!\n";
        currMessage+=board_representation();
        isGameComplete=1;
        return currMessage;
    }

    int n = possible_positions.size();
    int random_index = rand()%n;
    int random_pos = possible_positions[random_index];
    currMessage+="Server chose " + to_string(random_pos)+"\n";

    int rs = (random_pos-1)/3, cs=(random_pos-1)%3;
    game_board[rs][cs]='O';
    possible_positions.erase(possible_positions.begin()+random_index);
    currMessage+=board_representation();

    result = checkForWin('O');
    if(result){
        currMessage+="Server Won!! Better luck next time!";
        isGameComplete=1;
        return currMessage;
    }
    else
    {
        currMessage+="Enter a position number where you want to place \'X\'";
        return currMessage;
    }
    
}

string help_message(){
    string currMessage="-----------Tic Tac Toe-----------\nYou are given \'X\', server will use \'O\' and for blank places \'.\' will be used.\n";
    string line_ = "-------------------\n";
    currMessage+=line_;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            currMessage+="| (" + to_string(3*i+j+1) + ") ";
        }
        currMessage+="|\n";
        currMessage+=line_;
    }
    currMessage+= "Type \"help\" for help and \"exit\" to leave the game.\n";
    return currMessage;
}

string init_from_server(){
    isGameComplete=0;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 0; j < 3; j++)
        {
            game_board[i][j]='.';
        }
    }
    
    string currMessage=help_message();
    currMessage+="Enter a position number where you want to place \'X\'";
    return currMessage;
}

int main(int argc, char const *argv[])
{
    srand(time(0));
    if (argc!=2)
    {
        cerr << "Missing port number\n";
        exit(0);
    }

    int port = atoi(argv[1]);
    char message[1500];


    //------- Code for welcome socket -------
    sockaddr_in serverSocket;
    bzero((char *)&serverSocket, sizeof(serverSocket));
    serverSocket.sin_family = AF_INET;
    serverSocket.sin_port = htons(port);
    serverSocket.sin_addr.s_addr = htonl(INADDR_ANY);

    int serverSocketID = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocketID<0)
    {
        cerr<<"Unsuccessful socket()\n";
        exit(0);
    }

    int bindStatus =  bind(serverSocketID, (struct sockaddr *)&serverSocket, sizeof(serverSocket));
    if (bindStatus<0)
    {
        cerr<<"Unsuccessful bind()\n";
        exit(0);
    }
    
    cout<<"Waiting for the client to connect..."<<"\n";

    listen(serverSocketID, 1);

    //------- Code for connection socket -------
    sockaddr_in newSocket;
    socklen_t newSocketLen = sizeof(newSocket);

    int newSocketID = accept(serverSocketID, (struct sockaddr *)&newSocket, &newSocketLen);

    if (newSocketID<0)
    {
        cerr<<"Unsuccessful accept()\n";
        exit(0);
    }
    cout<<"Client connected successfully"<<"\n";
    string currMessage = init_from_server();
    strcpy(message, currMessage.c_str());
    send(newSocketID, (char *)&message, sizeof(message), 0);
    
    while (true)
    {
        cout<<"Client: ";

        recv(newSocketID, (char *)&message, sizeof(message), 0);
        
        if (!strcmp(message, "exit"))
        {
            cout<<"Session terminated"<<"\n";
            break;
        }
        cout<<message<<"\n";
        
        cout<<"Server: ";
        string data;
        if(!strcmp(message, "help")){
            data = help_message();
            data+="Enter a position number where you want to place \'X\'";
        } 
        else data = playGame((int)(message[0]-'0'));
        // cin>>data;
        // getline(cin, data);

        strcpy(message, data.c_str());

        send(newSocketID, (char *)&message, sizeof(message), 0);
        if (isGameComplete)
        {
            data="exit";
            strcpy(message, data.c_str());
            send(newSocketID, (char *)&message, sizeof(message), 0);
        }
        
        if (data=="exit")
        {
            cout<<"Session terminated"<<"\n";
            break;
        }

    }
    
    close(newSocketID);
    close(serverSocketID);

    return 0;
}