/*
    C ECHO client example using sockets
*/

#include <iostream>
#include <string>
#include <fstream>
#include <stdio.h>      //printf
#include <string.h>     //strlen
#include <unistd.h>
#include <arpa/inet.h>  //inet_addr
#include <sys/socket.h> //socket

using namespace std;


int main (int argc, char *argv[])
{
  int sock;
  struct sockaddr_in server;
  char buffer[1024];

  string command, arg;

  string serverLocation = argv[1];
  string serverPort = argv[2];
  string userName = argv[3];

  cout << "> Server Location: " << serverLocation << endl;
  cout << "> Server Port: " << serverPort << endl;
  cout << "> Username: " << userName << endl << endl;

  // Creates socket
  sock = socket(AF_INET, SOCK_STREAM, 0);

  if (sock == -1)
    printf(">> Could not create socket\n\n");

  puts(">> Socket created");

  server.sin_addr.s_addr = inet_addr(serverLocation.c_str());
  server.sin_family = AF_INET;
  server.sin_port = htons(stoi(serverPort.c_str() ));

  // Connect to remote server
  if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror(">> Connection failed. Error\n\n");
    return 1;
  }

  cout << ">> Connected to the server!" << endl << endl;

  send(sock, userName.c_str(), userName.size(), 0);

  // Keep communicating with the server
  while (1)
  {
    cout << ">> ";
    cin.clear();

    // Read the command
    cin >> command;

    if (command.compare("exit") == 0)
    {
      cout << ">>> Bye!" << endl << endl;
      close(sock);
      return 0;
    } else if (command.compare("echo") == 0) {
      cin >> arg;

      send(sock, command.c_str(), command.size()+1, 0);

      for (float i=0; i<100000; i++)
        ; // Delay to send second part of message

      send(sock, arg.c_str(), arg.size()+1, 0);

    } else if (command.compare("send") == 0) {
      string endOF = "!EOF!";
      string line;

      cin >> arg;

      ifstream myfile(arg);

      send(sock, command.c_str(), command.size()+1, 0);

      for (float i=0; i<100000; i++)
        ; // Delay

      send(sock, arg.c_str(), arg.size()+1, 0); // Send the file's name

      sleep(1); // Delay so that the server is ready to receive messages

      if (myfile.is_open())
      {
        while (getline(myfile, line))
        {
          line += '\0';
          send(sock, line.c_str(), line.size()+1, 0); // Send one line
          recv(sock, buffer, 1024, 0); // Wait server's ACK that received the line
          memset(buffer, 0, 1024);
        }

        myfile.close();

        for (float i=0; i<500000; i++)
          ; // Delay to send the EOF

        send(sock, endOF.c_str(), endOF.size()+1, 0);
      }

    } else if (command.compare("list") == 0) {
      send(sock, command.c_str(), command.size()+1, 0);

      while (1)
      {
        recv(sock, buffer, 1024, 0);

        arg = buffer;

        if (arg.compare("!EOF!") == 0) // Break if it's the end of the list
        {
          memset(buffer, 0, 1024);
          break;
        }

        cout << arg << endl;

        send(sock, "ACK\0", 4, 0); // Send ACK
        memset(buffer, 0, 1024);
      }
    } else if (command.compare("get") == 0) {
      send(sock, command.c_str(), command.size()+1, 0);

      cin >> arg;

      string nomeDoArquivo;
      ofstream myfile;

      nomeDoArquivo = arg;

      for (float i=0; i<100000; i++)
        ; // Delay

      send(sock, arg.c_str(), arg.size()+1, 0); // Send the file's name

      myfile.open(nomeDoArquivo);

      memset(buffer, 0, 1024);

			int lines = 0;

      while (1)
      {
				lines++;
        recv(sock, buffer, 1024, 0);

        arg = buffer;
        //cout << arg << endl;

        if (arg.compare("!EOF!") == 0)
        {
          memset(buffer, 0, 1024);
          break;
        }

        myfile << arg << endl; // Receives the line

        send(sock, "ACK\0", 4, 0); // Send ACK
        memset(buffer, 0, 1024);
      }

			if(lines == 0)
				remove(nomeDoArquivo.c_str());

      myfile.close();
      memset(buffer, 0, 1024);

    } else {
      cout << ">> Unknown command!\n" << endl;
      continue;
    }

    // Resposta do servidor
    if (recv(sock, buffer, 1024, 0) < 0) {
      puts("recv failed");
      break;
    }

    cout << ">>> SERVER: " << buffer << endl << endl;

    memset(buffer, 0, 1024);
  }

  close(sock);
  return 0;
}