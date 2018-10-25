/*
    C server socket example
*/

#include <iostream>
#include <string.h> //strlen
#include <fstream>
#include <stdio.h>
#include <unistd.h> //write
#include <arpa/inet.h> //inet_addr
#include <sys/socket.h>

#include <algorithm>
#include <map>
#include <thread>
#include <vector>


using namespace std;

void worker()
{
  while (1)
  {
    cout << "te thread" << endl;
    sleep(1);
  }
}


int main(int argc, char *argv[])
{
  vector<thread> workers;
  map<string, vector<string>> userMap;

  int socket_desc, client_sock, c;
  struct sockaddr_in server;
  string serverPort = argv[1];

  // Creates socket
  socket_desc = socket(AF_INET, SOCK_STREAM, 0);

  if (socket_desc == -1)
    printf("> Could not create socket\n\n");

  puts("> Socket created");

  // Prepares the sockaddr_in structure
  server.sin_family = AF_INET;
  server.sin_addr.s_addr = INADDR_ANY;
  server.sin_port = htons(stoi(serverPort.c_str()));

  // Bind
  if (bind(socket_desc, (struct sockaddr *)&server, sizeof(server)) < 0)
  {
    perror("> Bind failed. Error");   // Print the error message
    return 1;
  }

  puts("> Bind done");

  // Listen for connections
  listen(socket_desc, 3);

  // Accept an incoming connection
  puts(">> Waiting for incoming connections...\n");
  c = sizeof(struct sockaddr_in);

  // Accept connection from an incoming client
  struct sockaddr_in client;

  while (1)
  {
    // Accepts a socket connection and creates a thread with it's ID
    client_sock = 
    accept(socket_desc, (struct sockaddr *)&client, (socklen_t *)&c);

    workers.push_back(thread([&userMap, client_sock, client, socket_desc]()
    {
      int readSize;
      char clientName[20];
      char bufferC[1024];
      char bufferA[1024];
      string clientCommand, clientArg;

      if (client_sock < 0)
      {
        perror(">>> Accept failed\n\n");
        return 1;
      }

      puts(">>> Connection accepted");

      // Lê o nome do cliente do socket
      recv(client_sock, clientName, 20, 0);
      cout << ">>>> Connected to the client: " << clientName << endl << endl;

      while (1)
      {
        readSize = recv(client_sock, bufferC, 1024, 0);

        // Checks for disconnect. If true, flushe and break
        if (readSize == 0)
        {
          cout << ">>>> Client " << clientName << " disconnected!" << endl << endl;
          fflush(stdout);
          break;
        } else if (readSize == -1) {
          perror(">>>> recv failed\n\n");
        }

        // If not disconnected, treats the received command
        clientCommand = bufferC;
        cout << "(" << clientName << ") COMMAND RECEIVED: " << clientCommand
             << endl;

        if (clientCommand.compare("echo") == 0)
        {
          // Reads echo's argument
          recv(client_sock, bufferA, 1024, 0);
          clientArg = bufferA;

          write(client_sock, clientArg.c_str(), clientArg.size()+1);

          memset(bufferC, 0, 1024);
          memset(bufferA, 0, 1024);
        } else if (clientCommand.compare("send") == 0) {
          string nomeDoArquivo;
          ofstream myfile;

          recv(client_sock, bufferA, 1024, 0);
          nomeDoArquivo = bufferA;
          nomeDoArquivo = "SERV" + nomeDoArquivo;

          myfile.open(nomeDoArquivo);

          memset(bufferA, 0, 1024);

          while (1)
          {
            recv(client_sock, bufferA, 1024, 0);

            clientArg = bufferA;
            //cout << clientArg << endl;

            if (clientArg.compare("!EOF!") == 0)
            {
              memset(bufferA, 0, 1024);
              break;
            }

            myfile << clientArg << endl; // Receives the line

            write(client_sock, "ACK\0", 4); // Send ACK
            memset(bufferA, 0, 1024);
          }

          myfile.close();

          string done = "File saved successfully!";

          userMap[clientName].push_back(nomeDoArquivo.substr(4));

          for (float i=0; i<100000; i++)
            ; // delay para mandar o fim do arquivo

          write(client_sock, done.c_str(), done.size()+1);

          memset(bufferC, 0, 1024);
          memset(bufferA, 0, 1024);
        } else if (clientCommand.compare("list") == 0)
        {
          write(client_sock, "============", 15); // List's header
          recv(client_sock, bufferA, 1024,
               0); // Waits for the client's ACK which received the line

          for (auto nomeDoArquivo : userMap[clientName])
          {
            for (float i = 0; i < 100000; i++)
              ; // Delay to send each line of the list

            nomeDoArquivo = "* " + nomeDoArquivo;

            write(client_sock, nomeDoArquivo.c_str(), nomeDoArquivo.size() + 1);
            recv(client_sock, bufferA, 1024,0);  // Waits for the client's ACK which received the line
          }

          write(client_sock, "============", 15); // List's ass
          recv(client_sock, bufferA, 1024, 0); // Waits for the client's ACK which received the line
          string endOF = "!EOF!";

          for (float i=0; i<100000; i++)
            ; // Delay to send the EOF

          write(client_sock, endOF.c_str(), endOF.size()+1);

          string done = "Completed list show!";
          for (float i=0;  i<100000; i++)
            ; // Delay to send the EOF

          write(client_sock, done.c_str(), done.size()+1);

        } else if (clientCommand.compare("get") == 0) {
          string endOF = "!EOF!";
          string nomeDoArquivo;

          recv(client_sock, bufferA, 1024, 0);
          nomeDoArquivo = bufferA;
          nomeDoArquivo = "SERV" + nomeDoArquivo;

          cout << ">>>> File name: " << nomeDoArquivo << endl << endl;

          // Checa se o usuário tem o arquivo na lista dele
          bool temArquivo = false;
          for (auto arquivo : userMap[clientName])
          {
            if (arquivo.compare(nomeDoArquivo.substr(4)) == 0)
              temArquivo = true;
          }

          if (!temArquivo)
          {
            for (float i=0; i<500000; i++)
              ; // Delay to send the EOF

            send(client_sock, endOF.c_str(), endOF.size()+1, 0);

            string done = "You do not own this file!";

            for (float i=0; i<100000; i++)
              ; // Delay to send the EOF

            write(client_sock, done.c_str(), done.size()+1);
            continue;
          }

          ifstream myfile(nomeDoArquivo);

          memset(bufferA, 0, 1024);
          sleep(1); // Delay for the client to be ready to receive the messages

          string line;
          if (myfile.is_open())
          {
            while (getline(myfile, line))
            {
              // cout << "ARQV: " << line << endl;
              line += '\0';
              send(client_sock, line.c_str(), line.size()+1, 0); // Sends one line
              recv(client_sock, bufferA, 1024,0); // Waits the server's ACK that received the line
              memset(bufferA, 0, 1024);
            }

            myfile.close();

            for (float i=0; i<500000; i++)
              ; // Delay to send the EOF

            send(client_sock, endOF.c_str(), endOF.size() + 1, 0);

            string done = "File sent successfully";
            for (float i=0; i<100000; i++)
              ; // Delay to send the EOF

            write(client_sock, done.c_str(), done.size() + 1);
          }
        }
      }
      
    }));
  }

  std::for_each(workers.begin(), workers.end(),
                [](std::thread &t) { t.join(); });

  return 0;
}